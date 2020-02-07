/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version.
 * 2011-11-26     aozima       implementation time.
 * 2015-07-16     FlyM         rename rtc to stm32f1_rtc. remove finsh export function
 */
	
#include <time.h>
#include <rtthread.h>
#include <stm32f10x.h>
#include "stm32f1_rtc.h"

static struct rt_device rtc;

#if 0

#define MINUTE   60
#define HOUR   (60*MINUTE)
#define DAY   (24*HOUR)
#define YEAR   (365*DAY)

static rt_uint32_t month[12] =
{
    0,
    DAY*(31),
    DAY*(31+29),
    DAY*(31+29+31),
    DAY*(31+29+31+30),
    DAY*(31+29+31+30+31),
    DAY*(31+29+31+30+31+30),
    DAY*(31+29+31+30+31+30+31),
    DAY*(31+29+31+30+31+30+31+31),
    DAY*(31+29+31+30+31+30+31+31+30),
    DAY*(31+29+31+30+31+30+31+31+30+31),
    DAY*(31+29+31+30+31+30+31+31+30+31+30)
};


static struct tm init_time;


//�ж��Ƿ������꺯��
//�·�   1  2  3  4  5  6  7  8  9  10 11 12
//����   31 29 31 30 31 30 31 31 30 31 30 31
//������ 31 28 31 30 31 30 31 31 30 31 30 31
//����:���
//���:������ǲ�������.1,��.0,����
rt_uint8_t Is_Leap_Year(rt_uint16_t year)
{			  
	if(year%4==0) //�����ܱ�4����
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//�����00��β,��Ҫ�ܱ�400���� 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}	 


//����ʱ��
//�������ʱ��ת��Ϊ����
//��1970��1��1��Ϊ��׼
//1970~2099��Ϊ�Ϸ����
//����ֵ:0,�ɹ�;����:�������.
//�·����ݱ�											 
rt_uint8_t const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //���������ݱ�	  
//ƽ����·����ڱ�
const rt_uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};

rt_uint8_t rt_setTime(struct tm *tm)
{
	rt_uint16_t t;
	
	rt_uint32_t sec_count=0;
	
	if(tm->tm_year < 1970||tm->tm_year>2099) return 1;	
	
	for(t=1970;t<tm->tm_year;t++)	//��������ݵ��������
	{
		if(Is_Leap_Year(t)) sec_count += 31622400;//�����������
		else sec_count+=31536000;			  //ƽ���������
	}
	
	tm->tm_mon -= 1;
	
	for(t=0; t < tm->tm_mon; t++)	   //��ǰ���·ݵ����������
	{
		sec_count += (rt_uint32_t) mon_table[t] * 86400;//�·����������
		if(Is_Leap_Year(tm->tm_year)&&t==1)sec_count+=86400;//����2�·�����һ���������	   
	}
	
	sec_count += (rt_uint32_t)(tm->tm_mday-1)*86400;//��ǰ�����ڵ���������� 
	sec_count += (rt_uint32_t)tm->tm_hour*3600;//Сʱ������
    sec_count += (rt_uint32_t)tm->tm_min*60;	 //����������
	sec_count += tm->tm_sec;//�������Ӽ���ȥ

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��  
	PWR_BackupAccessCmd(ENABLE);	//ʹ��RTC�ͺ󱸼Ĵ������� 
	RTC_SetCounter(sec_count);	//����RTC��������ֵ

	RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������  	
	
	return 0;	    
}

#endif

static rt_err_t rt_rtc_open(rt_device_t dev, rt_uint16_t oflag)
{
    if (dev->rx_indicate != RT_NULL)
    {
        /* Open Interrupt */
    }

    return RT_EOK;
}

static rt_size_t rt_rtc_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    return 0;
}

static rt_err_t rt_rtc_control(rt_device_t dev, int cmd, void *args)
{
    rt_time_t *time;
    RT_ASSERT(dev != RT_NULL);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_RTC_GET_TIME:
        time = (rt_time_t *)args;
        /* read device */
        *time = RTC_GetCounter();
        break;

    case RT_DEVICE_CTRL_RTC_SET_TIME:
    {
        time = (rt_time_t *)args;

        /* Enable PWR and BKP clocks */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

        /* Allow access to BKP Domain */
        PWR_BackupAccessCmd(ENABLE);	//ʹ�ܺ󱸼Ĵ������� 

        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������

        /* Change the current time */
        RTC_SetCounter(*time);

        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();

        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
    }
    break;
    }

    return RT_EOK;
}

/*******************************************************************************
* Function Name  : RTC_Configuration
* Description    : Configures the RTC.
* Input          : None
* Output         : None
* Return         : 0 reday,-1 error.
*******************************************************************************/
int RTC_Init_Configuration(void)
{
    rt_uint32_t count=0x200000;

    /* Enable PWR and BKP clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);	//ʹ�ܺ󱸼Ĵ�������

    /* Reset Backup Domain */
    BKP_DeInit();	//��λ�������� 

    /* Enable LSE */
    RCC_LSEConfig(RCC_LSE_ON);	//�����ⲿ���پ���(LSE),ʹ��������پ���
    /* Wait till LSE is ready */
    while ( (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (--count) );//���ָ����RCC��־λ�������,�ȴ����پ������
    if ( count == 0 )
    {
        return -1;
    }

    /* Select LSE as RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);	//����RTCʱ��(RTCCLK),ѡ��LSE��ΪRTCʱ��  

    /* Enable RTC Clock */
    RCC_RTCCLKCmd(ENABLE);//ʹ��RTCʱ�� 

    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();	//�ȴ�RTC�Ĵ���ͬ��  

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������

    /* Set RTC prescaler: set RTC period to 1sec *///����RTCԤ��Ƶ��ֵ
    RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

#if 0
	/* ��ʼ����������ʱ���� */
	init_time.tm_year = 2019;
	init_time.tm_mon = 9;
	init_time.tm_mday = 1;
	init_time.tm_hour = 12;
	init_time.tm_min = 0;
	init_time.tm_sec = 0;
	
	rt_setTime(&init_time);  //����ʱ��	
	RTC_ExitConfigMode(); //�˳�����ģʽ  
	BKP_WriteBackupRegister(BKP_DR1, 0X5050);	//��ָ���ĺ󱸼Ĵ�����д���û���������
#endif

    return 0;
}

#ifdef RT_USING_RTC

int rt_hw_rtc_init(void)
{
    rtc.type	= RT_Device_Class_RTC;

    if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
    {
        rt_kprintf("rtc is not configured\n");
        rt_kprintf("please configure with set_date and set_time\n");
        if ( RTC_Init_Configuration() != 0)
        {
            rt_kprintf("rtc configure fail...\r\n");
            return -1;
        }
    }
    else
    {
        /* Wait for RTC registers synchronization */
        RTC_WaitForSynchro();
    }

    /* register rtc device */
    rtc.init 	= RT_NULL;
    rtc.open 	= rt_rtc_open;
    rtc.close	= RT_NULL;
    rtc.read 	= rt_rtc_read;
    rtc.write	= RT_NULL;
    rtc.control = rt_rtc_control;

    /* no private */
    rtc.user_data = RT_NULL;

    rt_device_register(&rtc, "rtc", RT_DEVICE_FLAG_RDWR);

    return 0;
}

INIT_BOARD_EXPORT(rt_hw_rtc_init);

#endif
