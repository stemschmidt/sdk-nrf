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

    memset(receive_buffer, 0x55, 1024);

    nrf_gpio_cfg_input(BUTTON_1, NRF_GPIO_PIN_PULLUP);
    nrf_gpiote_event_configure(NRF_GPIOTE, INPUT_CONFIG_0, BUTTON_1, NRF_GPIOTE_POLARITY_LOTOHI);
    nrf_gpiote_event_enable(NRF_GPIOTE, INPUT_CONFIG_0);

    nrf_gpiote_task_configure(NRF_GPIOTE, OUTPUT_CONFIG, LED_4, NRF_GPIOTE_POLARITY_TOGGLE, 0);
    nrf_gpiote_task_enable(NRF_GPIOTE, OUTPUT_CONFIG);

    nrf_gpiote_publish_set(NRF_GPIOTE, NRF_GPIOTE_EVENT_IN_0, trigger_spi_channel);
    nrf_gpiote_subscribe_set(NRF_GPIOTE, NRF_GPIOTE_TASK_OUT_2, trigger_led_channel);

    nrf_spim_frequency_set(NRF_SPIM0, NRF_SPIM_FREQ_2M);
    nrf_spim_rx_buffer_set(NRF_SPIM0, receive_buffer, 896);

    nrf_spim_subscribe_set(NRF_SPIM0, NRF_SPIM_TASK_START, trigger_spi_channel);
    nrf_spim_publish_set(NRF_SPIM0, NRF_SPIM_EVENT_END, trigger_led_channel);
    nrf_spim_enable(NRF_SPIM0);

    nrf_dppi_channels_enable(NRF_DPPIC, 0x00000003);
}
