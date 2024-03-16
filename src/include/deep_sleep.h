#include "pico/stdlib.h"

void deep_sleep(uint gpio);
void sleep_init();
void deep_sleep_until(datetime_t *time);