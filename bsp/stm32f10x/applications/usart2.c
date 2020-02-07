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
 * 程序清单：这是一个 串口 设备使用例程
 * 例程导出了 uart_sample 命令到控制终端
 * 命令调用格式：uart_sample uart2
 * 命令解释：命令第二个参数是要使用的串口设备名称，为空则使用默认的串口设备
 * 程序功能：通过串口输出字符串"hello RT-Thread!"，然后错位输出输入的字符
*/
#include <rtthread.h>
#include <stdio.h> /* for putchar */
#include <string.h>

#include "usart2.h"

#define USART2_NAME       "uart2"      /* 串口设备名称 */

rt_uint8_t usart2_stack[ 1024 ];
struct rt_thread usart2_thread;

static char recvBuf[RT_SERIAL_RB_BUFSZ] = {'\0'};
static rt_uint16_t timer_counter = 0;
static rt_uint16_t recvCount = 0;


/* 用于接收消息的信号量 */
static struct rt_semaphore rx_sem;
static rt_device_t usart2;

/* 接收数据回调函数 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    /* 串口接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&rx_sem);
	
    return RT_EOK;
}

/* 官方例程提供的方法进行字符的读取（采用中断方式） */
static char usart2_getchar(void)
{
    char ch;

    RT_ASSERT(usart2 != RT_NULL);
    /* 从串口读取一个字节的数据，没有读取到则等待接收信号量 */
    while (rt_device_read(usart2, -1, &ch, 1) != 1) {
        /* 阻塞等待接收信号量，等到信号量后再次读取数据 */
        rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
    }

    return ch;
}

void recv_Callback(void *parameter)
{
	timer_counter++;
		
	if(timer_counter > 1 && recvBuf[0] != '\0') 	//超时20ms, OS Tick: RT_TICK_PER_SECOND 100(默认值10ms)
	{
		timer_counter = 0;
		recvCount = 0;
		rt_kprintf("\r\n[%d]: %s\r\n", __LINE__, recvBuf);
		memset(recvBuf, '\0', sizeof(recvBuf));

		/*
			情况3：定时器超时表示一帧字符串接收完成，可以添加线程间同步操作
		*/
	}
}


void usart2_thread_entry(void *parameter)
{
	char ch;
	const char str[] = {"\r\n****** Hello RT-Thread! ******\r\n"};

	rt_timer_t timer_recv;

    /* 查找串口设备 */
    usart2 = rt_device_find(USART2_NAME);
	
    if (!usart2)
    {
        rt_kprintf("find %s failed!\n", USART2_NAME);
    }
	
    /* 初始化信号量 */
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
	
    /* 以读写及中断接收方式打开串口设备 */
    rt_device_open(usart2, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
	
    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(usart2, uart_input);
	
    /* 发送字符串 */
    rt_device_write(usart2, 0, str, (sizeof(str) - 1));

	/* 创建周期性定时器 */
	timer_recv = rt_timer_create("timer1", recv_Callback,
									RT_NULL, 10,		/* 定时长度，以OS Tick 为单位，即10 个OS Tick */
									RT_TIMER_FLAG_PERIODIC);
	rt_timer_stop(timer_recv);
		
    while (1)
    {
		memset(recvBuf, '\0', sizeof(recvBuf));
		recvCount = 0;
		
    	while(1)	//循环接收过程
    	{
    		rt_timer_start(timer_recv);		//启动接收定时器
    		timer_counter = 0;
			
    		ch = usart2_getchar();
			
	        /* 读取到的数据通过串口输出 */
	        //ch = ch + 1;
	        //rt_device_write(usart2, 0, &ch, 1);
			
			if (ch == '\r' || ch == '\n') 	//情况1：判断以特定字符作为字符串结尾的
			{
				recvBuf[recvCount] = '\0';	//按需求对结尾字符自行处理
				break;						//跳出接收过程
			} 
			else 
			{
				recvBuf[recvCount++] = ch;	//正常情况下，缓存一个个字符
				if(recvCount >= (sizeof(recvBuf)-1)) 	//情况2：如果超出缓存区，跳出接收过程，但会进行第二次接收，会丢失部分字符
				{
					//recvBuf[--recvCount] = '\0';	//防止越界访问，按需求对结尾字符自行处理
					break;
				}
			}
		}
		
		rt_timer_stop(timer_recv);			
		rt_kprintf("\r\n[%d]: %s\r\n", __LINE__, recvBuf);

		/*
			一帧字符串接收完成，可以添加线程间同步操作
			此处接收完成可能是由于情况1或情况2引起，
			如果是情况3定时器超时，仍会阻塞在等待接收信号量rx_sem
		*/
    }
}




