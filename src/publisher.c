#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"

#include <cJSON.h>

#include "publisher.h"
#include "secrets.h"

static mqtt_client_t *client;
static bool message_sent = false;
static bool wifi_connected = false;
static bool client_connected = false;

bool connect_wifi()
{

    if (wifi_connected)
    {
        return true;
    }

    if (cyw43_arch_init())
    {
        printf("failed to initialise\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("failed to connect.\n");
        return false;
    }
    printf("Connected to Wi-Fi\n");
    wifi_connected = true;
    return true;
}

void tear_down_wifi()
{
    printf("Disconnecting from Wi-Fi...\n");
    cyw43_arch_disable_sta_mode();
    cyw43_arch_deinit();
    wifi_connected = false;
}

void mqtt_tear_down()
{
    printf("Disconnecting from MQTT server\n");
    cyw43_arch_lwip_begin();
    mqtt_disconnect(client);
    cyw43_arch_lwip_end();
    mqtt_client_free(client);
    client_connected = false;
}


void publish_callback(void *arg, err_t err)
{
    if (err == ERR_OK)
    {
        printf("Message published\n");
        message_sent = true;
    }
    else
    {
        printf("%d\n", err);
        printf("Failed to publish message\n");
    }
}

bool publish_blocking(char *topic, char *message)
{
    printf("Publishing message\n");
    cyw43_arch_lwip_begin();
    mqtt_publish(client, topic, message, strlen(message), 0, 0, &publish_callback, NULL);
    cyw43_arch_lwip_end();

    while (!message_sent)
    {
        tight_loop_contents();
    }
    return true;
}

void mqtt_connection_callback(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    printf("MQTT connection status: %d\n", status);
    if (status == MQTT_CONNECT_ACCEPTED)
    {
        client_connected = true;
        return;
    }
    client_connected = false;
}

bool connect_mqtt_blocking()
{

    if (client != NULL && client_connected)
    {
        return true;
    }

    client = mqtt_client_new();
    const struct mqtt_connect_client_info_t client_info = {
        .client_id = "pico-mqtt-bridge",
        .client_user = MQTT_USER,
        .client_pass = MQTT_PASSWORD,
        .will_topic = NULL,
        .will_msg = NULL,
        .will_qos = 0,
        .will_retain = 0,
        .keep_alive = 60,
    };

    ip_addr_t ipaddr;
    const char *ip_str = "192.168.5.135";

    if (ipaddr_aton(ip_str, &ipaddr))
    {
        // Conversion succeeded
        printf("IP address: %s converted successfully.\n", ip_str);
        // Here you can use 'ipaddr' with other LwIP functions
    }
    else
    {
        // Conversion failed
        printf("Failed to convert IP address: %s\n", ip_str);
    }

    printf("Connecting to MQTT server\n");
    cyw43_arch_lwip_begin();
    err_t err = mqtt_client_connect(client, &ipaddr, MQTT_SERVER_PORT, mqtt_connection_callback, NULL, &client_info);
    cyw43_arch_lwip_end();
    if (err != ERR_OK)
    {
        printf("Failed to connect to MQTT server\n");
        return false;
    }

    while (!client_connected)
    {
        tight_loop_contents();
    }
    return true;
}

publish_status_t publish_message_blocking(WeatherReadingMessage *message)
{
    if (!connect_wifi())
    {
        return PUBLISH_WIFI_CONNECT_FAILED;
    }

    if (!connect_mqtt_blocking())
    {
        return PUBLISH_MQTT_CONNECT_FAILED;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "current_temperature", message->temperature);
    cJSON_AddNumberToObject(root, "current_humidity", message->humidity);
    cJSON_AddNumberToObject(root, "current_pressure", message->barometric_pressure);
    cJSON_AddNumberToObject(root, "current_rainfall", message->rainfall);
    cJSON_AddNumberToObject(root, "current_wind_speed", message->wind_speed);
    cJSON_AddNumberToObject(root, "current_wind_direction", message->wind_direction);

    char *json_str = cJSON_Print(root);
    printf("%s", json_str);
    message_sent = false;
    bool success = publish_blocking("weather/current", json_str);

    if (!success)
    {
        return PUBLISH_FAILED;
    }

    while (!message_sent)
    {
        tight_loop_contents();
    }

    free(json_str);
    cJSON_Delete(root);
    return PUBLISH_SUCCESS;
}
