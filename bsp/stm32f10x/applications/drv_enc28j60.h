
#ifndef DRV_ENC28J60_H_INCLUDED
#define DRV_ENC28J60_H_INCLUDED

#include <rtthread.h>


#define ENC28J60_CS			93	//PG8
#define ENC28J60_SCLK		74	//PB13
#define ENC28J60_MOSI		75	//PB14
#define ENC28J60_MISO		76	//PB15
#define ENC28J60_RST		91	//PG6
#define ENC28J60_IRQ		92	//PG7
#define ENC28J60_WPL			//



int rt_hw_enc28j60_init(void);



#endif
