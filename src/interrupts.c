#include <stdio.h>

#include "config.h"
#include "rfm69.h"
#include "interrupts.h"

void interrupt_handler(uint gpio, uint32_t events) {
    switch (gpio) {
        case PIN_INTR:
            printf("Interrupt received\n");
            rfm69_interrupt_handler();
            break;
        default:
            printf("Unknown interrupt received\n");
            break;
    }
}

void attach_interrupts() {
    // Attach the interrupt handler to the interrupt pin
    gpio_set_irq_enabled_with_callback(PIN_INTR, GPIO_IRQ_EDGE_RISE, true, &interrupt_handler);
}