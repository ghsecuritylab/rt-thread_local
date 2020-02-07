/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2013-07-12     aozima       update for auto initial.
 */

/**
 * @addtogroup STM32
 */
/*@{*/
#include <stdio.h>
#include <board.h>
#include <rtthread.h>

#ifdef RT_USING_DFS
/* dfs filesystem:ELM filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#endif

#ifdef RT_USING_RTGUI
#include <rtgui/rtgui.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>
#include <rtgui/calibration.h>
#endif

#include "adxl345.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "beep.h"
#include "decode.h"
#include "usart2.h"
#include "tcp_client.h"

ALIGN(RT_ALIGN_SIZE)
		
#define BUFFER_SIZE		0x10
//#define BUFFER_SIZE		0x1FFF
	
static rt_uint32_t testBuf[BUFFER_SIZE] = {1};

#define LED_THREAD_NAME	"led"
static rt_uint8_t led_stack[ 256 ];
//static rt_uint8_t led_stack[ BUFFER_SIZE ];
static struct rt_thread led_thread;

#define KEY_THREAD_NAME	"key"
static rt_uint8_t key_stack[ 256 ];
static struct rt_thread key_thread;

#define BEEP_THREAD_NAME "beep"
static rt_uint8_t beep_stack[ 256 ];
static struct rt_thread beep_thread;

#define LCD_THREAD_NAME	"lcd"
static rt_uint8_t lcd_stack[ 512 ];
static struct rt_thread lcd_thread;

#define ADXL345_THREAD_NAME	"adxl345"
static rt_uint8_t adxl345_stack[ 512 ];
static struct rt_thread adxl345_thread;

static void led_thread_entry(void* parameter)
{
    unsigned int count=0;
	rt_uint32_t i = 0;

    rt_hw_led_init();

    while (1)
    {
    	for(i=0; i<BUFFER_SIZE; i++)
    	{
    		testBuf[i]++;
			rt_kprintf("testBuf[%d] : 0x%X\r\n", i, testBuf[i]);
		}
        /* led1 on */
#ifndef RT_USING_FINSH
        rt_kprintf("led on, count : %d\r\n",count);
#endif
        count++;
        rt_hw_led_on(0);
        rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.5 second and switch to other thread */

        /* led1 off */
#ifndef RT_USING_FINSH
        rt_kprintf("led off\r\n");
#endif
        rt_hw_led_off(0);
        rt_thread_delay( RT_TICK_PER_SECOND/2 );
    }
}

static void key_thread_entry(void* parameter)
{
    pin_key_init();

    while (1) {
        rt_thread_delay( RT_TICK_PER_SECOND/2 );
	}

}

static void beep_thread_entry(void* parameter)
{
    /* user app entry */
//    struct beep_song_data data;
//    int len, i;
//    char name[20];
	
    pin_beep_init();

#ifdef RT_USING_PWM	
    beep_song_decode_init();
    beep_song_get_name(&song1, name);
    rt_kprintf("正在播放：%s\n",name);

    while (1) {
		i = 0;
    	len = beep_song_get_len(&song1);
		
	    while (i < len)
	    {
	        /* 解码音乐数据 */
	        beep_song_get_data(&song1, i, &data);
	        beep_set(data.freq, 3);
	        beep_on(NULL);

	        rt_thread_mdelay(data.sound_len);

	        beep_off(NULL);
	        rt_thread_mdelay(data.nosound_len);
	        i++;
	    }
		
        rt_thread_delay_s(5);
	}
#else

	while (1) {
        rt_thread_delay_s(5);
	}

#endif
}

static void lcd_thread_entry(void* parameter)
{
 	u8 x=0;
	u8 lcd_id[12];			//存放LCD ID字符串
	time_t now;
	
	LCD_Init();
	POINT_COLOR=RED;
	sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//将LCD ID打印到lcd_id数组。
	
    while (1) {
		switch(x)
		{
			case 0:LCD_Clear(WHITE);break;
			case 1:LCD_Clear(BLACK);break;
			case 2:LCD_Clear(BLUE);break;
			case 3:LCD_Clear(RED);break;
			case 4:LCD_Clear(MAGENTA);break;
			case 5:LCD_Clear(GREEN);break;
			case 6:LCD_Clear(CYAN);break;

			case 7:LCD_Clear(YELLOW);break;
			case 8:LCD_Clear(BRRED);break;
			case 9:LCD_Clear(GRAY);break;
			case 10:LCD_Clear(LGRAY);break;
			case 11:LCD_Clear(BROWN);break;
		}
		POINT_COLOR=RED;	  
		LCD_ShowString(30,50,200,16,16,"WarShip STM32 ^_^");	
		LCD_ShowString(30,70,200,16,16,"TFTLCD TEST");	
		LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
 		LCD_ShowString(30,110,200,16,16,lcd_id);		//显示LCD ID	      					 
		LCD_ShowString(30,130,200,16,16,"2012/9/5");	      					 
	    x++;
		if(x==12)x=0;
		
		/* 获取时间*/
		//now = time(RT_NULL);
		//rt_kprintf("%s\n", ctime(&now));
		
        rt_thread_delay_s( 1 );
	}

}

static void adxl345_thread_entry(void* parameter)
{
	short x=0, y=0, z=0;
	
	ADXL345_Init();

	while(1) {
		ADXL345_RD_XYZ(&x, &y, &z);
        rt_kprintf("x: %d, y:%d, z:%d\r\n", x, y, z);
        rt_thread_delay_s(30);
	}
}

void user_thread_init(void)
{
    rt_err_t result;

    /* init led thread */
    result = rt_thread_init(&led_thread,
                            LED_THREAD_NAME,
                            led_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&led_stack[0],
                            sizeof(led_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&led_thread);
    }
	
    /* init led thread */
    result = rt_thread_init(&key_thread,
                            KEY_THREAD_NAME,
                            key_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&key_stack[0],
                            sizeof(key_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&key_thread);
    }
#if 1	
    /* 创建 usart2 线程 */
    result = rt_thread_init(&usart2_thread,
                            USART2_THREAD_NAME,
                            usart2_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&usart2_stack[0],
                            sizeof(usart2_stack),
                            10,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&usart2_thread);
    }
#endif	

    /* init led thread */
    result = rt_thread_init(&beep_thread,
                            BEEP_THREAD_NAME,
                            beep_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&beep_stack[0],
                            sizeof(beep_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&beep_thread);
    }
	
    /* init led thread */
    result = rt_thread_init(&lcd_thread,
                            LCD_THREAD_NAME,
                            lcd_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&lcd_stack[0],
                            sizeof(lcd_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&lcd_thread);
    }
	
    /* init led thread */
    result = rt_thread_init(&adxl345_thread,
                            ADXL345_THREAD_NAME,
                            adxl345_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&adxl345_stack[0],
                            sizeof(adxl345_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&adxl345_thread);
    }
	
#if 1	
    /* 创建 tcp_client 线程 */
    result = rt_thread_init(&tcp_client_thread,
                            TCP_CLIENT_THREAD_NAME,
                            tcp_client_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&tcp_client_stack[0],
                            sizeof(tcp_client_stack),
                            25,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&tcp_client_thread);
    }
#endif	
}


#ifdef RT_USING_RTGUI
rt_bool_t cali_setup(void)
{
    rt_kprintf("cali setup entered\n");
    return RT_FALSE;
}

void cali_store(struct calibration_data *data)
{
    rt_kprintf("cali finished (%d, %d), (%d, %d)\n",
               data->min_x,
               data->max_x,
               data->min_y,
               data->max_y);
}
#endif /* RT_USING_RTGUI */

void rt_init_thread_entry(void* parameter)
{
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif

    /* Filesystem Initialization */
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
    /* mount sd card fat partition 1 as root directory */
    if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
    {
        rt_kprintf("File System initialized!\n");
    }
    else
        rt_kprintf("File System initialzation failed!\n");
#endif  /* RT_USING_DFS */

#ifdef RT_USING_RTGUI
    {
        extern void rt_hw_lcd_init();
        extern void rtgui_touch_hw_init(void);

        rt_device_t lcd;

        /* init lcd */
        rt_hw_lcd_init();

        /* init touch panel */
        rtgui_touch_hw_init();

        /* find lcd device */
        lcd = rt_device_find("lcd");

        /* set lcd device as rtgui graphic driver */
        rtgui_graphic_set_device(lcd);

#ifndef RT_USING_COMPONENTS_INIT
        /* init rtgui system server */
        rtgui_system_server_init();
#endif

        calibration_set_restore(cali_setup);
        calibration_set_after(cali_store);
        calibration_init();
    }
#endif /* #ifdef RT_USING_RTGUI */
}

int rt_application_init(void)
{
    rt_thread_t init_thread;
	
#if (RT_THREAD_PRIORITY_MAX == 32)
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 8, 20);
#else
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 80, 20);
#endif

    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

	user_thread_init();

    return 0;
}

/*@}*/
