/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

#ifndef __LED_H__
#define __LED_H__

#include <rtthread.h>

/* 在drv_gpio.c中查看各引脚编号 */
#define LED1_PIN_NUM	135//PB5
#define LED2_PIN_NUM 	4//PE5

void led_init(void);
void led_on(rt_uint8_t led);
void led_off(rt_uint8_t led);

#endif
