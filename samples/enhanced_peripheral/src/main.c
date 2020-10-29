/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <zephyr.h>
#include <debug/ppi_trace.h>
#include <drivers/counter.h>
#include <hal/nrf_rtc.h>
#include <hal/nrf_clock.h>
#include <device.h>
#include <logging/log.h>
#include <hal/nrf_gpiote.h>
#include <hal/nrf_gpio.h>
#include <hal/nrf_spim.h>
#include <hal/nrf_dppi.h>

#define INPUT_CONFIG_0 0
#define INPUT_CONFIG_1 1

#define OUTPUT_CONFIG 2

#define LED_4 0x05

#define BUTTON_1 0x06
#define BUTTON_2 0x07

static uint8_t receive_buffer[1024];

void main(void)
{
    const uint8_t trigger_spi_channel = 0;
    const uint8_t trigger_led_channel = 1;

    // initialize receive buffer to all 0x55
    memset(receive_buffer, 0x55, 1024);

    // Configure BUTTON_1 -> uses CONFIG[0] register in GPIOTE for event
    nrf_gpio_cfg_input(BUTTON_1, NRF_GPIO_PIN_PULLUP);
    nrf_gpiote_event_configure(NRF_GPIOTE, INPUT_CONFIG_0, BUTTON_1, NRF_GPIOTE_POLARITY_LOTOHI);
    nrf_gpiote_event_enable(NRF_GPIOTE, INPUT_CONFIG_0);

    // Configure LED_4 -> uses CONFIG[2] register in GPIOTE (I had a second button previously which used CONFIG[1]
    nrf_gpiote_task_configure(NRF_GPIOTE, OUTPUT_CONFIG, LED_4, NRF_GPIOTE_POLARITY_TOGGLE, 0);
    nrf_gpiote_task_enable(NRF_GPIOTE, OUTPUT_CONFIG);

    // Configure SPIM0 -> it will only read zeros as no GPIO pins are assigned
    nrf_spim_frequency_set(NRF_SPIM0, NRF_SPIM_FREQ_2M);
    nrf_spim_rx_buffer_set(NRF_SPIM0, receive_buffer, 896);
    nrf_spim_enable(NRF_SPIM0);

    // now connect Button_1 event with SPIM0 task on DPPI channel 0
    nrf_gpiote_publish_set(NRF_GPIOTE, NRF_GPIOTE_EVENT_IN_0, trigger_spi_channel);
    nrf_spim_subscribe_set(NRF_SPIM0, NRF_SPIM_TASK_START, trigger_spi_channel);
  
    // and connect SPIM0 transaction end event with LED_4 task on DPPI channel 1
    nrf_spim_publish_set(NRF_SPIM0, NRF_SPIM_EVENT_END, trigger_led_channel);
    nrf_gpiote_subscribe_set(NRF_GPIOTE, NRF_GPIOTE_TASK_OUT_2, trigger_led_channel);

    // and enable DPPI for channel 0 and 1 -> 0x00000003
    nrf_dppi_channels_enable(NRF_DPPIC, 0x00000003);

    /* 
    If this is all set up, start the program in the debugger. Pause the execution and add the 
    receive_buffer to your watch window. Double-check that all entries in the array are 0x55.
    Now press button_1. You will see that LED_4 lights up, although the CPU is still halted.
    Now do a single step, to update the watch window: All entries up to 895 are now 0, from 
    896 on all remained at 0x55 -> that is the magic!!!
    */ 
}

