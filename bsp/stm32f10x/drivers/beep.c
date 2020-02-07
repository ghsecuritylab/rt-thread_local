
#include <rtthread.h>
#include <rtdevice.h>

#include "beep.h"

#ifndef RT_USING_PIN

void rt_hw_beep_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
		
	RCC_APB2PeriphClockCmd(beep_rcc, ENABLE);	 //ʹ��GPIOB�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = beep_pin;				 //BEEP-->PB.8 �˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //�ٶ�Ϊ50MHz
	GPIO_Init(beep_gpio, &GPIO_InitStructure);	 //���ݲ�����ʼ��GPIOB.8

	GPIO_ResetBits(beep_gpio, beep_pin);//���0���رշ��������
}

#endif


#ifdef RT_USING_PIN

struct rt_device_pwm     *pwm_device = RT_NULL; //���� pwm �豸ָ��

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
    /* ����������Ϊ���ģʽ */
    rt_pin_mode(BEEP_PIN_NUM, PIN_MODE_OUTPUT);
    /* Ĭ�ϵ͵�ƽ */
    rt_pin_write(BEEP_PIN_NUM, PIN_LOW);

	
#ifdef RT_USING_PWM
    /* ����PWM�豸 */
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

    /* ��Ƶ��ת��Ϊ���� ���ڵ�λ:ns Ƶ�ʵ�λ:HZ */
    period = 1000000000 / freq;  //unit:ns 1/HZ*10^9 = ns

    /* ����������С����ռ�ձ� �������͵�ƽ���� */
    pulse = period - period / 100 * volume;

    /* ���� PWM API �趨 ���ں�ռ�ձ� */
    rt_pwm_set(pwm_device, BEEP_PWM_CH, period, pulse);//channel,period,pulse

    return 0;
}

#endif

#endif


