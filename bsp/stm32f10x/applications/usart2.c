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
 * �����嵥������һ�� ���� �豸ʹ������
 * ���̵����� uart_sample ��������ն�
 * ������ø�ʽ��uart_sample uart2
 * ������ͣ�����ڶ���������Ҫʹ�õĴ����豸���ƣ�Ϊ����ʹ��Ĭ�ϵĴ����豸
 * �����ܣ�ͨ����������ַ���"hello RT-Thread!"��Ȼ���λ���������ַ�
*/
#include <rtthread.h>
#include <stdio.h> /* for putchar */
#include <string.h>

#include "usart2.h"

#define USART2_NAME       "uart2"      /* �����豸���� */

rt_uint8_t usart2_stack[ 1024 ];
struct rt_thread usart2_thread;

static char recvBuf[RT_SERIAL_RB_BUFSZ] = {'\0'};
static rt_uint16_t timer_counter = 0;
static rt_uint16_t recvCount = 0;


/* ���ڽ�����Ϣ���ź��� */
static struct rt_semaphore rx_sem;
static rt_device_t usart2;

/* �������ݻص����� */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    /* ���ڽ��յ����ݺ�����жϣ����ô˻ص�������Ȼ���ͽ����ź��� */
    rt_sem_release(&rx_sem);
	
    return RT_EOK;
}

/* �ٷ������ṩ�ķ��������ַ��Ķ�ȡ�������жϷ�ʽ�� */
static char usart2_getchar(void)
{
    char ch;

    RT_ASSERT(usart2 != RT_NULL);
    /* �Ӵ��ڶ�ȡһ���ֽڵ����ݣ�û�ж�ȡ����ȴ������ź��� */
    while (rt_device_read(usart2, -1, &ch, 1) != 1) {
        /* �����ȴ������ź������ȵ��ź������ٴζ�ȡ���� */
        rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
    }

    return ch;
}

void recv_Callback(void *parameter)
{
	timer_counter++;
		
	if(timer_counter > 1 && recvBuf[0] != '\0') 	//��ʱ20ms, OS Tick: RT_TICK_PER_SECOND 100(Ĭ��ֵ10ms)
	{
		timer_counter = 0;
		recvCount = 0;
		rt_kprintf("\r\n[%d]: %s\r\n", __LINE__, recvBuf);
		memset(recvBuf, '\0', sizeof(recvBuf));

		/*
			���3����ʱ����ʱ��ʾһ֡�ַ���������ɣ���������̼߳�ͬ������
		*/
	}
}


void usart2_thread_entry(void *parameter)
{
	char ch;
	const char str[] = {"\r\n****** Hello RT-Thread! ******\r\n"};

	rt_timer_t timer_recv;

    /* ���Ҵ����豸 */
    usart2 = rt_device_find(USART2_NAME);
	
    if (!usart2)
    {
        rt_kprintf("find %s failed!\n", USART2_NAME);
    }
	
    /* ��ʼ���ź��� */
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
	
    /* �Զ�д���жϽ��շ�ʽ�򿪴����豸 */
    rt_device_open(usart2, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
	
    /* ���ý��ջص����� */
    rt_device_set_rx_indicate(usart2, uart_input);
	
    /* �����ַ��� */
    rt_device_write(usart2, 0, str, (sizeof(str) - 1));

	/* ���������Զ�ʱ�� */
	timer_recv = rt_timer_create("timer1", recv_Callback,
									RT_NULL, 10,		/* ��ʱ���ȣ���OS Tick Ϊ��λ����10 ��OS Tick */
									RT_TIMER_FLAG_PERIODIC);
	rt_timer_stop(timer_recv);
		
    while (1)
    {
		memset(recvBuf, '\0', sizeof(recvBuf));
		recvCount = 0;
		
    	while(1)	//ѭ�����չ���
    	{
    		rt_timer_start(timer_recv);		//�������ն�ʱ��
    		timer_counter = 0;
			
    		ch = usart2_getchar();
			
	        /* ��ȡ��������ͨ��������� */
	        //ch = ch + 1;
	        //rt_device_write(usart2, 0, &ch, 1);
			
			if (ch == '\r' || ch == '\n') 	//���1���ж����ض��ַ���Ϊ�ַ�����β��
			{
				recvBuf[recvCount] = '\0';	//������Խ�β�ַ����д���
				break;						//�������չ���
			} 
			else 
			{
				recvBuf[recvCount++] = ch;	//��������£�����һ�����ַ�
				if(recvCount >= (sizeof(recvBuf)-1)) 	//���2������������������������չ��̣�������еڶ��ν��գ��ᶪʧ�����ַ�
				{
					//recvBuf[--recvCount] = '\0';	//��ֹԽ����ʣ�������Խ�β�ַ����д���
					break;
				}
			}
		}
		
		rt_timer_stop(timer_recv);			
		rt_kprintf("\r\n[%d]: %s\r\n", __LINE__, recvBuf);

		/*
			һ֡�ַ���������ɣ���������̼߳�ͬ������
			�˴�������ɿ������������1�����2����
			��������3��ʱ����ʱ���Ի������ڵȴ������ź���rx_sem
		*/
    }
}




