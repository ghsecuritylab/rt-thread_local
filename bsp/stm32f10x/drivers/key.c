/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-15     misonyo      first implementation.
 */
/*
 * 程序清单：这是一个 PIN 设备使用例程
 * 例程导出了 pin_beep_sample 命令到控制终端
 * 命令调用格式：pin_beep_sample
 * 程序功能：通过按键控制蜂鸣器对应引脚的电平状态控制蜂鸣器
*/
#include <rtthread.h>
#include <rtdevice.h>

#include "key.h"

#ifndef RT_USING_PIN

void rt_hw_key_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(key0_rcc|key1_rcc|key2_rcc|key3_rcc, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = key0_pin | key1_pin | key2_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
    GPIO_Init(key0_gpio, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = key3_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0设置成输入，默认下拉	
    GPIO_Init(key3_gpio, &GPIO_InitStructure);
}

#endif


#ifdef RT_USING_PIN

#include "beep.h"

void pin_key_init(void)
{
	pin_beep_init();
	
    /* 按键0引脚为输入模式 */
    rt_pin_mode(KEY0_PIN_NUM, PIN_MODE_INPUT_PULLUP);
    /* 绑定中断，上升沿模式，回调函数名为beep_on */
    rt_pin_attach_irq(KEY0_PIN_NUM, PIN_IRQ_MODE_FALLING, beep_on, RT_NULL);
    /* 使能中断 */
    rt_pin_irq_enable(KEY0_PIN_NUM, PIN_IRQ_ENABLE);
	
    /* 按键1引脚为输入模式 */
    rt_pin_mode(KEY1_PIN_NUM, PIN_MODE_INPUT_PULLUP);
    /* 绑定中断，上升沿模式，回调函数名为beep_off */
    rt_pin_attach_irq(KEY1_PIN_NUM, PIN_IRQ_MODE_FALLING, beep_off, RT_NULL);
    /* 使能中断 */
    rt_pin_irq_enable(KEY1_PIN_NUM, PIN_IRQ_ENABLE);
}

#endif



