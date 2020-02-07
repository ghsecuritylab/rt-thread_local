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
 * �����嵥������һ�� PIN �豸ʹ������
 * ���̵����� pin_beep_sample ��������ն�
 * ������ø�ʽ��pin_beep_sample
 * �����ܣ�ͨ���������Ʒ�������Ӧ���ŵĵ�ƽ״̬���Ʒ�����
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
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
    GPIO_Init(key0_gpio, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = key3_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0���ó����룬Ĭ������	
    GPIO_Init(key3_gpio, &GPIO_InitStructure);
}

#endif


#ifdef RT_USING_PIN

#include "beep.h"

void pin_key_init(void)
{
	pin_beep_init();
	
    /* ����0����Ϊ����ģʽ */
    rt_pin_mode(KEY0_PIN_NUM, PIN_MODE_INPUT_PULLUP);
    /* ���жϣ�������ģʽ���ص�������Ϊbeep_on */
    rt_pin_attach_irq(KEY0_PIN_NUM, PIN_IRQ_MODE_FALLING, beep_on, RT_NULL);
    /* ʹ���ж� */
    rt_pin_irq_enable(KEY0_PIN_NUM, PIN_IRQ_ENABLE);
	
    /* ����1����Ϊ����ģʽ */
    rt_pin_mode(KEY1_PIN_NUM, PIN_MODE_INPUT_PULLUP);
    /* ���жϣ�������ģʽ���ص�������Ϊbeep_off */
    rt_pin_attach_irq(KEY1_PIN_NUM, PIN_IRQ_MODE_FALLING, beep_off, RT_NULL);
    /* ʹ���ж� */
    rt_pin_irq_enable(KEY1_PIN_NUM, PIN_IRQ_ENABLE);
}

#endif



