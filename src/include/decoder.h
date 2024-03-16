#ifndef DECODER_H
#define DECODER_H

#include "rfm69.h"
#include "pico/stdlib.h"
#include "weather_reading.pb.h"

bool decode_message(message_t *message, WeatherReadingMessage *weather_reading_message);

#endif // DECODER_H