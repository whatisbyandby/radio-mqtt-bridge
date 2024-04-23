#ifndef PUBLISHER_H
#define PUBLISHER_H

#include "weather_reading.pb.h"

#define MQTT_BROKER_ADDR "192.168.5.135"
#define MQTT_SERVER_PORT 1883



typedef enum {
    PUBLISH_SUCCESS = 0,
    PUBLISH_WIFI_CONNECT_FAILED,
    PUBLISH_MQTT_CONNECT_FAILED,
    PUBLISH_FAILED
} publish_status_t;

publish_status_t publish_message_blocking(WeatherReadingMessage *weather_reading_message, uint8_t rssi);

#endif // PUBLISHER_H




