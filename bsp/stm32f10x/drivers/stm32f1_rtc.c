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


//判断是否是闰年函数
//月份   1  2  3  4  5  6  7  8  9  10 11 12
//闰年   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年 31 28 31 30 31 30 31 31 30 31 30 31
//输入:年份
//输出:该年份是不是闰年.1,是.0,不是
rt_uint8_t Is_Leap_Year(rt_uint16_t year)
{			  
	if(year%4==0) //必须能被4整除
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//如果以00结尾,还要能被400整除 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}	 


//设置时钟
//把输入的时钟转换为秒钟
//以1970年1月1日为基准
//1970~2099年为合法年份
//返回值:0,成功;其他:错误代码.
//月份数据表											 
rt_uint8_t const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表	  
//平年的月份日期表
const rt_uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};

rt_uint8_t rt_setTime(struct tm *tm)
{
	rt_uint16_t t;
	
	rt_uint32_t sec_count=0;
	
	if(tm->tm_year < 1970||tm->tm_year>2099) return 1;	
	
	for(t=1970;t<tm->tm_year;t++)	//把所有年份的秒钟相加
	{
		if(Is_Leap_Year(t)) sec_count += 31622400;//闰年的秒钟数
		else sec_count+=31536000;			  //平年的秒钟数
	}
	
	tm->tm_mon -= 1;
	
	for(t=0; t < tm->tm_mon; t++)	   //把前面月份的秒钟数相加
	{
		sec_count += (rt_uint32_t) mon_table[t] * 86400;//月份秒钟数相加
		if(Is_Leap_Year(tm->tm_year)&&t==1)sec_count+=86400;//闰年2月份增加一天的秒钟数	   
	}
	
	sec_count += (rt_uint32_t)(tm->tm_mday-1)*86400;//把前面日期的秒钟数相加 
	sec_count += (rt_uint32_t)tm->tm_hour*3600;//小时秒钟数
    sec_count += (rt_uint32_t)tm->tm_min*60;	 //分钟秒钟数
	sec_count += tm->tm_sec;//最后的秒钟加上去

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟  
	PWR_BackupAccessCmd(ENABLE);	//使能RTC和后备寄存器访问 
	RTC_SetCounter(sec_count);	//设置RTC计数器的值

	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成  	
	
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
        PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问 

        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成

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
    PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问

    /* Reset Backup Domain */
    BKP_DeInit();	//复位备份区域 

    /* Enable LSE */
    RCC_LSEConfig(RCC_LSE_ON);	//设置外部低速晶振(LSE),使用外设低速晶振
    /* Wait till LSE is ready */
    while ( (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (--count) );//检查指定的RCC标志位设置与否,等待低速晶振就绪
    if ( count == 0 )
    {
        return -1;
    }

    /* Select LSE as RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);	//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟  

    /* Enable RTC Clock */
    RCC_RTCCLKCmd(ENABLE);//使能RTC时钟 

    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();	//等待RTC寄存器同步  

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成

    /* Set RTC prescaler: set RTC period to 1sec *///设置RTC预分频的值
    RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

#if 0
	/* 初始化：年月日时分秒 */
	init_time.tm_year = 2019;
	init_time.tm_mon = 9;
	init_time.tm_mday = 1;
	init_time.tm_hour = 12;
	init_time.tm_min = 0;
	init_time.tm_sec = 0;
	
	rt_setTime(&init_time);  //设置时间	
	RTC_ExitConfigMode(); //退出配置模式  
	BKP_WriteBackupRegister(BKP_DR1, 0X5050);	//向指定的后备寄存器中写入用户程序数据
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
