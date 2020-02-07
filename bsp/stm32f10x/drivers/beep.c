
#include <rtthread.h>
#include <rtdevice.h>

#include "beep.h"

#ifndef RT_USING_PIN

void rt_hw_beep_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
		
	RCC_APB2PeriphClockCmd(beep_rcc, ENABLE);	 //使能GPIOB端口时钟

	GPIO_InitStructure.GPIO_Pin = beep_pin;				 //BEEP-->PB.8 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //速度为50MHz
	GPIO_Init(beep_gpio, &GPIO_InitStructure);	 //根据参数初始化GPIOB.8

	GPIO_ResetBits(beep_gpio, beep_pin);//输出0，关闭蜂鸣器输出
}

#endif


#ifdef RT_USING_PIN

struct rt_device_pwm     *pwm_device = RT_NULL; //定义 pwm 设备指针

void beep_on(void *args)
{
    rt_kprintf("turn on beep!\n");
    rt_pin_write(BEEP_PIN_NUM, PIN_HIGH);
}

void beep_off(void *args)
{
    rt_kprintf("turn off beep!\n");
    rt_pin_write(BEEP_PIN_NUM, PIN_LOW);
}


int pin_beep_init(void)
{
    /* 蜂鸣器引脚为输出模式 */
    rt_pin_mode(BEEP_PIN_NUM, PIN_MODE_OUTPUT);
    /* 默认低电平 */
    rt_pin_write(BEEP_PIN_NUM, PIN_LOW);

	
#ifdef RT_USING_PWM
    /* 查找PWM设备 */
    pwm_device = (struct rt_device_pwm *)rt_device_find(BEEP_PWM_DEVICE);
    if (pwm_device == RT_NULL)
    {
        rt_kprintf("pwm device %s not found!\n", BEEP_PWM_DEVICE);
        return -RT_ERROR;
    }
#endif

	return RT_EOK;
}


#ifdef RT_USING_PWM

int beep_set(uint16_t freq, uint8_t volume)
{
    rt_uint32_t period, pulse;

    /* 将频率转化为周期 周期单位:ns 频率单位:HZ */
    period = 1000000000 / freq;  //unit:ns 1/HZ*10^9 = ns

    /* 根据声音大小计算占空比 蜂鸣器低电平触发 */
    pulse = period - period / 100 * volume;

    /* 利用 PWM API 设定 周期和占空比 */
    rt_pwm_set(pwm_device, BEEP_PWM_CH, period, pulse);//channel,period,pulse

    return 0;
}

#endif

#endif


