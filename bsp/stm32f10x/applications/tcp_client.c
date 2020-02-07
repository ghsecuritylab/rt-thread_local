/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date             Author      Notes
 *
 */
/*
 * 程序清单：tcp 客户端
 *
 * 这是一个 tcp 客户端的例程
 * 导出 tcpclient 命令到控制终端
 * 命令调用格式：tcpclient URL PORT
 * URL：服务器地址 PORT:：端口号
 * 程序功能：接收并显示从服务端发送过来的信息，接收到开头是 'q' 或 'Q' 的信息退出程序
*/

#include <rtthread.h>
#include <sys/socket.h> /* 使用BSD socket，需要包含socket.h头文件 */
#include <netdb.h>
#include <string.h>
#include <finsh.h>

#include <tcp_client.h>

#define BUFSZ   64

#define SERVER_IP	"192.168.1.10"
#define PORT_NUM	37777


rt_uint8_t tcp_client_stack[ THREAD_STACK ];
struct rt_thread tcp_client_thread;

void tcp_client_thread_entry(void *parameter)
{
    int ret;
    int sock;
    struct sockaddr_in server_addr;

	char buf[] = {"This is TCP Client from RT-Thread.\r\n"}; /* 发送用到的数据 */

	
    /* 初始化预连接的服务端地址 */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NUM);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    while (1)
    {
	    /* 创建一个socket，类型是SOCKET_STREAM，TCP类型 */
	    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	    {
	        /* 创建socket失败 */
	        rt_kprintf("Socket error\n");
			
			rt_thread_mdelay(5000);
			continue;
	    }

	    /* 连接到服务端 */
	    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
	    {
	        /* 连接失败 */
	        rt_kprintf("Connect fail!\n");
			
			closesocket(sock);
			rt_thread_mdelay(5000);
			continue;
	    }
		else
		{
			rt_kprintf("[%s][%d]: tcp client Connect Success !\r\n", __FUNCTION__, __LINE__);
		}
    	
		ret = send(sock, buf, sizeof(buf), 0);

		if(ret <= 0)
		{
        	rt_kprintf("[%s][%d]: send Faild !\r\n", __FUNCTION__, __LINE__);
		}
		
		if((ret = closesocket(sock)) < 0)
		{
        	rt_kprintf("[%s][%d]: closesocket Faild !\r\n", __FUNCTION__, __LINE__);
		}

		rt_thread_mdelay(3000);
    }
}

//MSH_CMD_EXPORT(tcp_client, a tcp client sample);

