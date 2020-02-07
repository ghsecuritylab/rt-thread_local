#ifndef __KEY_H
#define __KEY_H	 

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//按键驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////   	 

#if !defined  (USE_HAL_DRIVER)	

//#define KEY0 PEin(4)   	//PE4
//#define KEY1 PEin(3)	//PE3 
//#define KEY2 PEin(2)	//PE2
//#define KEY3 PAin(0)	//PA0  WK_UP
 
#define KEY0  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)//读取按键0
#define KEY1  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)//读取按键1
#define KEY2  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)//读取按键2 
#define KEY3  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//读取按键3(WK_UP) 

#else

/* 引脚编号， 通过查看设备驱动文件drv_gpio.c 确定*/
#define KEY0_PIN_NUM	3	//PE4
#define KEY1_PIN_NUM 	2	//PE3
#define KEY2_PIN_NUM	1	//PE2
#define KEY3_PIN_NUM 	34	//PA0

#define KEY0  rt_pin_read(KEY0_PIN_NUM)//读取按键0
#define KEY1  rt_pin_read(KEY1_PIN_NUM)//读取按键1
#define KEY2  rt_pin_read(KEY2_PIN_NUM)//读取按键2 
#define KEY3  rt_pin_read(KEY3_PIN_NUM)//读取按键3(WK_UP) 


#endif


#define KEY_UP 		4
#define KEY_LEFT	3
#define KEY_DOWN	2
#define KEY_RIGHT	1

void key_Init(void);//IO初始化
rt_uint8_t key_Scan(rt_uint8_t);  	//按键扫描函数					    
#endif
