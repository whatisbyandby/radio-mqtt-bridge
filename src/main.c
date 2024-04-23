#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/spi.h"
#include "hardware/watchdog.h"
#include "pico/binary_info.h"
#include "pico/sleep.h"

#include "rfm69.h"


#include "hardware/rtc.h"

#include "hardware/clocks.h"
#include "hardware/rosc.h"
#include "hardware/structs/scb.h"

#include "secrets.h"
#include "weather_reading.pb.h"
#include "publisher.h"
#include "deep_sleep.h"

#include <pb_decode.h>
#include "weather_reading.pb.h"

#include "interrupts.h"

#include "config.h"


// reset board using watchdog timer
void reset_board()
{
    watchdog_enable(1000, 0);
    while (1)
        tight_loop_contents();
}


bool initalize_pins()
{
    // This example will use SPI0 at 0.5MHz.
    spi_init(spi_default, 1000 * 1000);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    // Make the SPI pins available to picotool
    bi_decl(bi_3pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
    gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
    // Make the CS pin available to picotool
    bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));
}

int main()
{
    stdio_init_all();

    printf("Starting up\n");
    
    initalize_pins();
    attach_interrupts();

    rfm69_t rfm69 = {
        .spi = spi0,
        .cs_pin = PICO_DEFAULT_SPI_CSN_PIN,
        .interrupt_pin = PIN_INTR,
        .reset_pin = PIN_RESET,
    };

    rfm69_error_t err = rfm69_init(&rfm69);

    if (err != RFM69_OK)
    {
        printf("Unable to initalize RFM69 Module");
        while (true)
        {
            tight_loop_contents();
        }
    }

    err = rfm69_set_mode_rx(&rfm69);

    if (err != RFM69_OK)
    {
        printf("Unable to put the RFM69 in RX mode");
        while (true)
        {
            tight_loop_contents();
        }
    }

    while (true)
    {

        printf("Waiting for message...");
        while (!rfm69_is_message_available(&rfm69)) {
            sleep_ms(1000);
            printf(".");
        }
        printf("\n");
       

        message_t new_message;
        rfm69_receive(&rfm69, &new_message);

        WeatherReadingMessage weather_reading_message = WeatherReadingMessage_init_zero;
         bool status;
        

        /* Create a stream that reads from the buffer. */
        pb_istream_t stream = pb_istream_from_buffer(new_message.data, new_message.len);

        /* Now we are ready to decode the message. */
        status = pb_decode(&stream, WeatherReadingMessage_fields, &weather_reading_message);

        /* Check for errors... */
        if (!status)
        {
            printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
        }
        publish_status_t publish_status = publish_message_blocking(&weather_reading_message, new_message.rssi);


        if (publish_status == PUBLISH_SUCCESS)
        {
            printf("Message sent successfully\n");
        }
        else
        {
            printf("Failed to send message\n");
        }
    }
}
