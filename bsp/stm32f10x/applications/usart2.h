#ifndef __USART2_H__
#define __USART2_H__

#include <rtthread.h>



#define USART2_THREAD_NAME	"usart2"

extern rt_uint8_t usart2_stack[ 1024 ];
extern struct rt_thread usart2_thread;


extern void usart2_thread_entry(void *parameter);


#endif
