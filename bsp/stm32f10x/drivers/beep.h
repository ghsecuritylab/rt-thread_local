#ifndef __BEEP_H__
#define __BEEP_H__

#include <rtthread.h>

#ifndef RT_USING_PIN


/* 蜂鸣器   (PB8)  */
#define beep_rcc                    RCC_APB2Periph_GPIOB
#define beep_gpio                   GPIOB
#define beep_pin                    (GPIO_Pin_8)


#define BEEP_OFF() 	GPIO_SetBits(beep_gpio, beep_pin)
#define BEEP_ON() 	GPIO_ResetBits(beep_gpio, beep_pin)


extern void rt_hw_beep_init(void);

#endif




#ifdef RT_USING_PIN


/* 引脚编号，通过查看驱动文件drv_gpio.c确定 */

/* 蜂鸣器   (PB8)  */
#define BEEP_PIN_NUM	139

extern int pin_beep_init(void);
extern void beep_on(void *args);
extern void beep_off(void *args);

#endif

#ifdef RT_USING_PWM

#define BEEP_PWM_DEVICE  "pwm1"
#define BEEP_PWM_CH      1



int beep_set(uint16_t freq, uint8_t volume); //蜂鸣器设定


#endif


#endif

