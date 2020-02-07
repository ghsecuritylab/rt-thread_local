#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#include <rtthread.h>

#define TCP_CLIENT_THREAD_NAME	"tcp_client"

#define THREAD_STACK	2048

extern rt_uint8_t tcp_client_stack[ THREAD_STACK ];
extern struct rt_thread tcp_client_thread;



extern void tcp_client_thread_entry(void *parameter);

#endif
