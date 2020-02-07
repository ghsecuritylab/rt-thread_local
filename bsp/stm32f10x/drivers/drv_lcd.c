/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-06-08     tanek        first implementation
 */

#include "drv_lcd.h"
#include <finsh.h>

//#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINTF(...)   rt_kprintf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)   
#endif

typedef struct
{
	rt_uint16_t width;			//LCD 宽度
	rt_uint16_t height;			//LCD 高度
	rt_uint16_t id;				//LCD ID
	rt_uint8_t  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。	
	rt_uint16_t	wramcmd;		//开始写gram指令
	rt_uint16_t setxcmd;		//设置x坐标指令
	rt_uint16_t setycmd;		//设置y坐标指令 
} lcd_info_t;

typedef struct
{
	volatile rt_uint16_t reg;
	volatile rt_uint16_t ram;
} lcd_ili9341_t;

//使用NOR/SRAM的 Bank1.sector1,地址位HADDR[27,26]=00 A18作为数据命令区分线 
//注意设置时STM32内部会右移一位对其!
#define LCD_ILI9341_BASE        ((rt_uint32_t)(0x60000000 | 0x0007FFFE))
#define ili9341					((lcd_ili9341_t *) LCD_ILI9341_BASE)
//////////////////////////////////////////////////////////////////////////////////

//扫描方向定义
#define L2R_U2D  0 		//从左到右,从上到下
#define L2R_D2U  1 		//从左到右,从下到上
#define R2L_U2D  2 		//从右到左,从上到下
#define R2L_D2U  3 		//从右到左,从下到上
#define U2D_L2R  4 		//从上到下,从左到右
#define U2D_R2L  5 		//从上到下,从右到左
#define D2U_L2R  6 		//从下到上,从左到右
#define D2U_R2L  7		//从下到上,从右到左	 
#define DFT_SCAN_DIR  L2R_U2D  //默认的扫描方向
 
//-----------------LCD端口定义----------------  
#define LCD_LED_ON GPIO_SetBits(GPIOB,GPIO_Pin_0)
#define LCD_LED_OFF GPIO_ResetBits(GPIOB,GPIO_Pin_0)

static lcd_info_t lcddev;

void delay_us(rt_uint32_t nus)
{
	//rt_thread_delay(1);
	while (nus--) {
		__NOP();
	}
}

void delay_ms(rt_uint32_t nms)
{
	//rt_thread_delay((RT_TICK_PER_SECOND * nms + 999) / 1000);
	while (nms--)
	{
		int i;
		for (i = 0; i < 10000; i++)
		{
			__NOP();
		}
	}
}

static void ili9341_write_reg(rt_uint16_t regval)
{
	ili9341->reg = regval;
}

static void ili9341_write_data(rt_uint16_t data)
{
	ili9341->ram = data;
}

rt_uint16_t ili9341_read_ram(void)
{
	return ili9341->ram;
}

static void ili9341_write_reg_with_value(rt_uint16_t reg, rt_uint16_t regValue)
{
	ili9341->reg = reg;
	ili9341->ram = regValue;
}

static void ili9341_write_ram_prepare(void)
{
	ili9341->reg = lcddev.wramcmd;
}

rt_uint16_t ili9341_bgr2rgb(rt_uint16_t value)
{
	rt_uint16_t  red, green, blue;

	blue = (value >> 0) & 0x1f;
	green = (value >> 5) & 0x3f;
	red = (value >> 11) & 0x1f;

	return (blue << 11) + (green << 5) + (red << 0);
}

static void ili9341_set_cursor(rt_uint16_t Xpos, rt_uint16_t Ypos)
{
	ili9341_write_reg(lcddev.setxcmd);
	ili9341_write_data(Xpos >> 8); 
	ili9341_write_data(Xpos & 0XFF);

	ili9341_write_reg(lcddev.setycmd);
	ili9341_write_data(Ypos >> 8); 
	ili9341_write_data(Ypos & 0XFF);
}
  	   
static void ili9341_set_scan_direction(rt_uint8_t dir)
{
	rt_uint16_t regval = 0;
	rt_uint16_t dirreg = 0;
	rt_uint16_t temp;

	switch (dir)
	{
	case L2R_U2D://从左到右,从上到下
		regval |= (0 << 7) | (0 << 6) | (0 << 5);
		break;
	case L2R_D2U://从左到右,从下到上
		regval |= (1 << 7) | (0 << 6) | (0 << 5);
		break;
	case R2L_U2D://从右到左,从上到下
		regval |= (0 << 7) | (1 << 6) | (0 << 5);
		break;
	case R2L_D2U://从右到左,从下到上
		regval |= (1 << 7) | (1 << 6) | (0 << 5);
		break;
	case U2D_L2R://从上到下,从左到右
		regval |= (0 << 7) | (0 << 6) | (1 << 5);
		break;
	case U2D_R2L://从上到下,从右到左
		regval |= (0 << 7) | (1 << 6) | (1 << 5);
		break;
	case D2U_L2R://从下到上,从左到右
		regval |= (1 << 7) | (0 << 6) | (1 << 5);
		break;
	case D2U_R2L://从下到上,从右到左
		regval |= (1 << 7) | (1 << 6) | (1 << 5);
		break;
	}

	dirreg = 0X36;
	ili9341_write_reg_with_value(dirreg, regval);

	if (regval & 0X20)
	{
		if (lcddev.width < lcddev.height)//交换X,Y
		{
			temp = lcddev.width;
			lcddev.width = lcddev.height;
			lcddev.height = temp;
		}
	}
	else
	{
		if (lcddev.width > lcddev.height)//交换X,Y
		{
			temp = lcddev.width;
			lcddev.width = lcddev.height;
			lcddev.height = temp;
		}
	}
	
	ili9341_write_reg(lcddev.setxcmd);
	ili9341_write_data(0);
	ili9341_write_data(0);
	ili9341_write_data((lcddev.width - 1) >> 8);
	ili9341_write_data((lcddev.width - 1) & 0XFF);

	ili9341_write_reg(lcddev.setycmd);
	ili9341_write_data(0);
	ili9341_write_data(0);
	ili9341_write_data((lcddev.height - 1) >> 8);
	ili9341_write_data((lcddev.height - 1) & 0XFF);
}

void ili9341_set_backlight(rt_uint8_t pwm)
{
	ili9341_write_reg(0xBE);
	ili9341_write_data(0x05);
	ili9341_write_data(pwm*2.55);
	ili9341_write_data(0x01);
	ili9341_write_data(0xFF);
	ili9341_write_data(0x00);
	ili9341_write_data(0x00);
}

void ili9341_set_display_direction(rt_uint8_t dir)
{
	lcddev.dir = dir;
	if (dir == 0)
	{
		lcddev.width = 240;
		lcddev.height = 320;
	}
	else
	{
		lcddev.width = 320;
		lcddev.height = 240;
	}

	lcddev.wramcmd = 0X2C;
	lcddev.setxcmd = 0X2A;
	lcddev.setycmd = 0X2B;

	ili9341_set_scan_direction(DFT_SCAN_DIR);
}

void _lcd_low_level_init(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  readWriteTiming; 
	FSMC_NORSRAMTimingInitTypeDef  writeTiming;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC,ENABLE);	//使能FSMC时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOG|RCC_APB2Periph_AFIO,ENABLE);//使能PORTB,D,E,G以及AFIO复用功能时钟

 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				 //PB0 推挽输出 背光
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);

 	//PORTD复用推挽输出  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_14|GPIO_Pin_15;				 //	//PORTD复用推挽输出  
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //复用推挽输出   
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOD, &GPIO_InitStructure); 
	  
	//PORTE复用推挽输出  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;				 //	//PORTD复用推挽输出  
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //复用推挽输出   
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOE, &GPIO_InitStructure); 
	  
   	//	//PORTG12复用推挽输出 A0	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_12;	 //	//PORTD复用推挽输出  
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //复用推挽输出   
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOG, &GPIO_InitStructure); 
 
	readWriteTiming.FSMC_AddressSetupTime = 0x01;	 //地址建立时间（ADDSET）为2个HCLK 1/36M=27ns
    readWriteTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（ADDHLD）模式A未用到	
    readWriteTiming.FSMC_DataSetupTime = 0x0f;		 // 数据保存时间为16个HCLK,因为液晶驱动IC的读数据的时候，速度不能太快，尤其对1289这个IC。
    readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
    readWriteTiming.FSMC_CLKDivision = 0x00;
    readWriteTiming.FSMC_DataLatency = 0x00;
    readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 
    

	writeTiming.FSMC_AddressSetupTime = 0x00;	 //地址建立时间（ADDSET）为1个HCLK  
    writeTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（A		
    writeTiming.FSMC_DataSetupTime = 0x03;		 ////数据保存时间为4个HCLK	
    writeTiming.FSMC_BusTurnAroundDuration = 0x00;
    writeTiming.FSMC_CLKDivision = 0x00;
    writeTiming.FSMC_DataLatency = 0x00;
    writeTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 

 
    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;//  这里我们使用NE4 ，也就对应BTCR[6],[7]。
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; // 不复用数据地址
    FSMC_NORSRAMInitStructure.FSMC_MemoryType =FSMC_MemoryType_SRAM;// FSMC_MemoryType_SRAM;  //SRAM   
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;//存储器数据宽度为16bit   
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =FSMC_BurstAccessMode_Disable;// FSMC_BurstAccessMode_Disable; 
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable; 
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;   
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;  
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;	//  存储器写使能
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;   
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable; // 读写使用不同的时序
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable; 
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming; //读写时序
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &writeTiming;  //写时序

    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  //初始化FSMC配置

   	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);  // 使能BANK1 
				 
	delay_ms(50);

	ili9341_write_reg(0XD3);
	lcddev.id = ili9341_read_ram();
	lcddev.id = ili9341_read_ram();
	lcddev.id = ili9341_read_ram();
	lcddev.id <<= 8;
	lcddev.id |= ili9341_read_ram();

	DEBUG_PRINTF(" LCD ID:%x\r\n", lcddev.id); //打印LCD ID   

	ili9341_write_reg(0xCF);
	ili9341_write_data(0x00);
	ili9341_write_data(0xC1);
	ili9341_write_data(0X30);
	ili9341_write_reg(0xED);
	ili9341_write_data(0x64);
	ili9341_write_data(0x03);
	ili9341_write_data(0X12);
	ili9341_write_data(0X81);
	ili9341_write_reg(0xE8);
	ili9341_write_data(0x85);
	ili9341_write_data(0x10);
	ili9341_write_data(0x7A);
	ili9341_write_reg(0xCB);
	ili9341_write_data(0x39);
	ili9341_write_data(0x2C);
	ili9341_write_data(0x00);
	ili9341_write_data(0x34);
	ili9341_write_data(0x02);
	ili9341_write_reg(0xF7);
	ili9341_write_data(0x20);
	ili9341_write_reg(0xEA);
	ili9341_write_data(0x00);
	ili9341_write_data(0x00);
	ili9341_write_reg(0xC0);    //Power control 
	ili9341_write_data(0x1B);   //VRH[5:0] 
	ili9341_write_reg(0xC1);    //Power control 
	ili9341_write_data(0x01);   //SAP[2:0];BT[3:0] 
	ili9341_write_reg(0xC5);    //VCM control 
	ili9341_write_data(0x30); 	//3F
	ili9341_write_data(0x30);   //3C
	ili9341_write_reg(0xC7);    //VCM control2 
	ili9341_write_data(0XB7);
	ili9341_write_reg(0x36);    // memory access control 
	ili9341_write_data(0x08);   // change here
	ili9341_write_reg(0x3A);
	ili9341_write_data(0x55);
	ili9341_write_reg(0xB1);
	ili9341_write_data(0x00);
	ili9341_write_data(0x1A);
	ili9341_write_reg(0xB6);    //display function control 
	ili9341_write_data(0x0A);
	ili9341_write_data(0xA2);
	ili9341_write_reg(0xF2);    //3gamma function disable 
	ili9341_write_data(0x00);
	ili9341_write_reg(0x26);    //gamma curve selected 
	ili9341_write_data(0x01);
	ili9341_write_reg(0xE0);    //set gamma 
	ili9341_write_data(0x0F);
	ili9341_write_data(0x2A);
	ili9341_write_data(0x28);
	ili9341_write_data(0x08);
	ili9341_write_data(0x0E);
	ili9341_write_data(0x08);
	ili9341_write_data(0x54);
	ili9341_write_data(0XA9);
	ili9341_write_data(0x43);
	ili9341_write_data(0x0A);
	ili9341_write_data(0x0F);
	ili9341_write_data(0x00);
	ili9341_write_data(0x00);
	ili9341_write_data(0x00);
	ili9341_write_data(0x00);
	ili9341_write_reg(0XE1);    //set gamma 
	ili9341_write_data(0x00);
	ili9341_write_data(0x15);
	ili9341_write_data(0x17);
	ili9341_write_data(0x07);
	ili9341_write_data(0x11);
	ili9341_write_data(0x06);
	ili9341_write_data(0x2B);
	ili9341_write_data(0x56);
	ili9341_write_data(0x3C);
	ili9341_write_data(0x05);
	ili9341_write_data(0x10);
	ili9341_write_data(0x0F);
	ili9341_write_data(0x3F);
	ili9341_write_data(0x3F);
	ili9341_write_data(0x0F);
	ili9341_write_reg(0x2B);
	ili9341_write_data(0x00);
	ili9341_write_data(0x00);
	ili9341_write_data(0x01);
	ili9341_write_data(0x3f);
	ili9341_write_reg(0x2A);
	ili9341_write_data(0x00);
	ili9341_write_data(0x00);
	ili9341_write_data(0x00);
	ili9341_write_data(0xef);
	ili9341_write_reg(0x11); //exit sleep
	delay_ms(120);
	ili9341_write_reg(0x29); //display on	

	ili9341_set_display_direction(0);
	
	LCD_LED_ON;					//点亮背光
	//LCD_Clear(WHITE);
}


static rt_err_t lcd_init(rt_device_t dev)
{
	return RT_EOK;
}

static rt_err_t lcd_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t lcd_close(rt_device_t dev)
{
	return RT_EOK;
}

static rt_err_t lcd_control(rt_device_t dev, int cmd, void *args)
{
	switch (cmd)
	{
	case RTGRAPHIC_CTRL_GET_INFO:
	{
		struct rt_device_graphic_info *info;

		info = (struct rt_device_graphic_info*) args;
		RT_ASSERT(info != RT_NULL);

		info->bits_per_pixel = 16;
		info->pixel_format = RTGRAPHIC_PIXEL_FORMAT_RGB565;
		info->framebuffer = RT_NULL;
		info->width = 240;
		info->height = 320;
	}
	break;

	case RTGRAPHIC_CTRL_RECT_UPDATE:
		/* nothong to be done */
		break;

	default:
		break;
	}

	return RT_EOK;
}

static void ili9341_lcd_set_pixel(const char* pixel, int x, int y)
{
	ili9341_set_cursor(x, y);
	ili9341_write_ram_prepare();
	ili9341->ram = *(uint16_t *)pixel;
}
#ifdef RT_USING_FINSH
static void lcd_set_pixel(uint16_t color, int x, int y)
{
	rt_kprintf("lcd set pixel, color: %X, x: %d, y: %d", color, x, y);
	ili9341_lcd_set_pixel((const char *)&color, x, y);
}
FINSH_FUNCTION_EXPORT(lcd_set_pixel, set pixel in lcd display);
#endif

static void ili9341_lcd_get_pixel(char* pixel, int x, int y)
{
	rt_uint16_t red = 0;
	rt_uint16_t green = 0;
	rt_uint16_t blue = 0;

	if (x >= lcddev.width || y >= lcddev.height)
	{
		*(rt_uint16_t*)pixel = 0;
		return;
	}

	ili9341_set_cursor(x, y);

	ili9341_write_reg(0X2E);
	ili9341_read_ram();
	red = ili9341_read_ram();
	delay_us(2);

	blue = ili9341_read_ram();
	green = red & 0XFF;

	*(rt_uint16_t*)pixel = (((red >> 11) << 11) | ((green >> 10) << 5) | (blue >> 11));
}
#ifdef RT_USING_FINSH
static void lcd_get_pixel(int x, int y)
{
	uint16_t pixel;
	ili9341_lcd_get_pixel((char *)&pixel, x, y);
	rt_kprintf("lcd get pixel, pixel: 0x%X, x: %d, y: %d", pixel, x, y);
}
FINSH_FUNCTION_EXPORT(lcd_get_pixel, get pixel in lcd display);
#endif

static void ili9341_lcd_draw_hline(const char* pixel, int x1, int x2, int y)
{
	ili9341_set_cursor(x1, y);
	ili9341_write_ram_prepare();

	for (; x1 < x2; x1++)
	{
		ili9341->ram = *(uint16_t *)pixel;
	}
}
#ifdef RT_USING_FINSH
static void lcd_draw_hline(uint16_t pixel, int x1, int x2, int y)
{
	ili9341_lcd_draw_hline((const char *)&pixel, x1, x2, y);
	rt_kprintf("lcd draw hline, pixel: 0x%X, x1: %d, x2: %d, y: %d", pixel, x1, x2, y);
}
FINSH_FUNCTION_EXPORT(lcd_draw_hline, draw hline in lcd display);
#endif

static void ili9341_lcd_draw_vline(const char* pixel, int x, int y1, int y2)
{
	for (; y1 < y2; y1++)
	{
		ili9341_lcd_set_pixel(pixel, x, y1);  //write red data
	}
}
#ifdef RT_USING_FINSH
static void lcd_draw_vline(uint16_t pixel, int x, int y1, int y2)
{
	ili9341_lcd_draw_vline((const char *)&pixel, x, y1, y2);
	rt_kprintf("lcd draw hline, pixel: 0x%X, x: %d, y: %d", pixel, y1, y2);
}
FINSH_FUNCTION_EXPORT(lcd_draw_vline, draw vline in lcd display);
#endif

static void ili9341_lcd_blit_line(const char* pixels, int x, int y, rt_size_t size)
{
	rt_uint16_t *ptr = (rt_uint16_t*)pixels;

	ili9341_set_cursor(x, y);
	ili9341_write_ram_prepare();

	while (size--)
	{
		ili9341->ram = *ptr++;
	}
}
#ifdef RT_USING_FINSH
#define LINE_LEN 30
static void lcd_blit_line(int x, int y)
{
	uint16_t pixels[LINE_LEN];
	int i;

	for (i = 0; i < LINE_LEN; i++)
	{
		pixels[i] = i * 40 + 50;
	}

	ili9341_lcd_blit_line((const char *)pixels, x, y, LINE_LEN);
	rt_kprintf("lcd blit line, x: %d, y: %d", x, y);
}
FINSH_FUNCTION_EXPORT(lcd_blit_line, draw blit line in lcd display);
#endif

static int rt_hw_lcd_init(void)
{
	_lcd_low_level_init();

	static struct rt_device lcd_device;

	static struct rt_device_graphic_ops ili9341_ops =
	{
		ili9341_lcd_set_pixel,
		ili9341_lcd_get_pixel,
		ili9341_lcd_draw_hline,
		ili9341_lcd_draw_vline,
		ili9341_lcd_blit_line
	};

	/* register lcd device */
	lcd_device.type = RT_Device_Class_Graphic;
	lcd_device.init = lcd_init;
	lcd_device.open = lcd_open;
	lcd_device.close = lcd_close;
	lcd_device.control = lcd_control;
	lcd_device.read = RT_NULL;
	lcd_device.write = RT_NULL;

	lcd_device.user_data = &ili9341_ops;

	/* register graphic device driver */
	rt_device_register(&lcd_device, "lcd",
		RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);

	return 0;
}
INIT_BOARD_EXPORT(rt_hw_lcd_init);

