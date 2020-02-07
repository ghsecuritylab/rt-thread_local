
#ifndef __KEY_H__
#define __KEY_H__

#include <rtthread.h>

#ifndef RT_USING_PIN

/* 按键0 (PE4) */
#define key0_rcc                    RCC_APB2Periph_GPIOE
#define key0_gpio                   GPIOE
#define key0_pin                    (GPIO_Pin_4)

/* 按键1 (PE3) */
#define key1_rcc                    RCC_APB2Periph_GPIOE		
#define key1_gpio                   GPIOE
#define key1_pin                    (GPIO_Pin_3)

/* 按键2 (PE2) */
#define key2_rcc                    RCC_APB2Periph_GPIOB
#define key2_gpio                   GPIOB
#define key2_pin                    (GPIO_Pin_2)

/* 按键3 (WK_UP) (PA0)  */
#define key3_rcc                    RCC_APB2Periph_GPIOA
#define key3_gpio                   GPIOA
#define key3_pin                    (GPIO_Pin_0)

void rt_hw_key_init(void);

#endif


#ifdef RT_USING_PIN

/* 引脚编号，通过查看驱动文件drv_gpio.c确定 */

/* 按键0 (PE4) */
#define KEY0_PIN_NUM 	3

/* 按键1 (PE3) */
#define KEY1_PIN_NUM 	2

/* 按键2 (PE2) */
#define KEY2_PIN_NUM 	1

/* 按键3 (WK_UP) (PA0)  */
#define KEY3_PIN_NUM	34



extern void pin_key_init(void);


#endif



#endif
