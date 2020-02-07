#include <rtthread.h>

#include "drv_enc28j60.h"
#include <drivers/pin.h>

#include "enc28j60.h"

static void INT_handler(void *args)
{
	/* enter interrupt */
	//  char *a = args;// 获取参数

	rt_interrupt_enter();

	enc28j60_isr();

	/* leave interrupt */
	rt_interrupt_leave();

//   rt_kprintf("28j60 down! %s\n",a);
}

#define ENC28J60_SPI_BUS	"spi2"
#define ENC28J60_SPI_DEV_NAME		"spi22"


int rt_hw_enc28j60_init(void)
{
	/* 配置复位引脚 */
	rt_pin_mode(ENC28J60_RST, PIN_MODE_INPUT_PULLUP);
	/* reset pe1 */
	rt_pin_write(ENC28J60_RST, PIN_LOW);
	rt_thread_delay(1);
	/* release reset */
	rt_pin_write(ENC28J60_RST, PIN_HIGH);

	//注册设备驱动
	if(enc28j60_attach(ENC28J60_SPI_DEV_NAME) == RT_EOK) {
    	rt_kprintf("^^^ enc28j60_attach success, access by SPI2 ^^^\r\n");
	}
	
	/* 配置中断 */
	rt_pin_mode(ENC28J60_IRQ, PIN_MODE_INPUT_PULLUP);
	//绑定中断
	rt_pin_attach_irq(ENC28J60_IRQ, PIN_IRQ_MODE_FALLING, INT_handler, (void*)"callbackargs");
	//使能中断
	rt_pin_irq_enable(ENC28J60_IRQ, PIN_IRQ_ENABLE);


	return 0;
}

INIT_DEVICE_EXPORT(rt_hw_enc28j60_init);


