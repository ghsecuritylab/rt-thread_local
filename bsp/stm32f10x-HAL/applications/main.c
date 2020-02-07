/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-07-29     Arda.Fu      first implementation
 */
#include <rtthread.h>
#include <stdio.h>
#include <string.h>

#include "led.h"
#include "lcd.h"

#define MAX_THREAD 20

static void led_thread_entry(void* parameter)
{
	unsigned int count=0;

    led_init();

    while (1)
    {
        /* led1 on */
#ifdef RT_USING_FINSH
        rt_kprintf("led on, count : %d\r\n",count);
#endif
        count++;
        led_on(0);
        led_off(1);
        rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.5 second and switch to other thread */

        /* led1 off */
#ifdef RT_USING_FINSH
        rt_kprintf("led off\r\n");
#endif
        led_off(0);
        led_on(1);
        rt_thread_delay( RT_TICK_PER_SECOND/2 );
    }
}

static void lcd_thread_entry(void* parameter)
{
	LCD_Init();

	while(1)
	{
        rt_kprintf("led off\r\n");
		
        rt_thread_delay( RT_TICK_PER_SECOND/2 );
	}
}

//线程初始化入口
static void rt_init_thread_entry(void* parameter)
{
	rt_thread_t led_thread;
	rt_thread_t lcd_thread;

    /* Create led thread */
    led_thread = rt_thread_create(
							"led",							        //线程名称
							led_thread_entry, 						//线程入口函数
							RT_NULL,								//线程入口参数
							256, 									//线程栈大小
							20, 									//线程优先级
							20);									//线程时间片大小
    if(led_thread != RT_NULL)
    	rt_thread_startup(led_thread);	
	
    /* Create led thread */
    lcd_thread = rt_thread_create(
							"led",							        //线程名称
							lcd_thread_entry, 						//线程入口函数
							RT_NULL,								//线程入口参数
							256, 									//线程栈大小
							20, 									//线程优先级
							20);									//线程时间片大小
    if(lcd_thread != RT_NULL)
    	rt_thread_startup(lcd_thread);
}


int main(void)
{
	rt_thread_t init_thread;

#if (RT_THREAD_PRIORITY_MAX == 32)
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   512, 8, 20);
#else
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   512, 80, 20);
#endif

    if(init_thread != RT_NULL)
    	rt_thread_startup(init_thread);
	

    return 0;
}
