#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/sleep.h"
#include "hardware/clocks.h"
#include "hardware/rosc.h"
#include "hardware/structs/scb.h"

static uint scb_orig;
static uint clock0_orig;
static uint clock1_orig;
static bool is_initalized = false;

void measure_freqs(void)
{
    uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
    uint f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
    uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
    uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
    uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
    uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
    uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);

    printf("pll_sys  = %dkHz\n", f_pll_sys);
    printf("pll_usb  = %dkHz\n", f_pll_usb);
    printf("rosc     = %dkHz\n", f_rosc);
    printf("clk_sys  = %dkHz\n", f_clk_sys);
    printf("clk_peri = %dkHz\n", f_clk_peri);
    printf("clk_usb  = %dkHz\n", f_clk_usb);
    printf("clk_adc  = %dkHz\n", f_clk_adc);
    printf("clk_rtc  = %dkHz\n", f_clk_rtc);
    uart_default_tx_wait_blocking();
    // Can't measure clk_ref / xosc as it is the ref
}

void sleep_init()
{
    // reset clocks
    clocks_init();

    // save values for after sleep
    scb_orig = scb_hw->scr;
    clock0_orig = clocks_hw->sleep_en0;
    clock1_orig = clocks_hw->sleep_en1;

    measure_freqs();
    is_initalized = true;
}

void recover_from_sleep()
{
    // Re-enable ring Oscillator control
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);

    // save values for later
    scb_orig = scb_hw->scr;
    clock0_orig = clocks_hw->sleep_en0;
    clock1_orig = clocks_hw->sleep_en1;

    clocks_init();
    stdio_init_all();

    measure_freqs();
}

void deep_sleep(uint gpio)
{

    sleep_run_from_xosc();
    // Go to sleep until we see a high edge on GPIO 10
    sleep_goto_dormant_until_edge_high(gpio);

    // Recover from sleep
    recover_from_sleep();
}

void deep_sleep_until(datetime_t *time)
{
    if (!is_initalized)
    {
        sleep_init();
    }
}