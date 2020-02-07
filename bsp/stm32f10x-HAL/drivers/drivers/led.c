/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */
#include <rthw.h>
#include <rtdevice.h>
#include <board.h>
#include <rtthread.h>

#include "led.h"


void led_on(rt_uint8_t n)
{
	switch (n)
    {
	    case 0:
			rt_pin_write(LED1_PIN_NUM, PIN_HIGH);
	        break;
	    case 1:
			rt_pin_write(LED2_PIN_NUM, PIN_HIGH);
	        break;
	    default:
	        break;
    }
}

void led_off(rt_uint8_t n)
{
	switch (n)
    {
	    case 0:
			rt_pin_write(LED1_PIN_NUM, PIN_LOW);
	        break;
	    case 1:
			rt_pin_write(LED2_PIN_NUM, PIN_LOW);
	        break;
	    default:
	        break;
    }
}

void led_init(void)
{
	rt_pin_mode(LED1_PIN_NUM , PIN_MODE_OUTPUT);
	rt_pin_mode(LED2_PIN_NUM , PIN_MODE_OUTPUT);
	
	rt_pin_write(LED1_PIN_NUM , PIN_LOW);
	rt_pin_write(LED2_PIN_NUM , PIN_LOW);

}


#ifdef RT_USING_FINSH

#include <finsh.h>
static rt_uint8_t led_inited = 0;

void led(rt_uint32_t led, rt_uint32_t value)
{
    /* init led configuration if it's not inited. */
    if (!led_inited)
    {
        led_init();
        led_inited = 1;
    }

    if ( led == 0 )
    {
        /* set led status */
        switch (value)
        {
        case 0:
            led_off(0);
            break;
        case 1:
            led_on(0);
            break;
        default:
            break;
        }
    }

    if ( led == 1 )
    {
        /* set led status */
        switch (value)
        {
        case 0:
            led_off(1);
            break;
        case 1:
            led_on(1);
            break;
        default:
            break;
        }
    }
}
MSH_CMD_EXPORT(led, set led[0 - 1] on[1] or off[0].)
#endif

