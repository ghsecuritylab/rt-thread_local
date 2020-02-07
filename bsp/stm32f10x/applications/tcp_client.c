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
 * �����嵥��tcp �ͻ���
 *
 * ����һ�� tcp �ͻ��˵�����
 * ���� tcpclient ��������ն�
 * ������ø�ʽ��tcpclient URL PORT
 * URL����������ַ PORT:���˿ں�
 * �����ܣ����ղ���ʾ�ӷ���˷��͹�������Ϣ�����յ���ͷ�� 'q' �� 'Q' ����Ϣ�˳�����
*/

#include <rtthread.h>
#include <sys/socket.h> /* ʹ��BSD socket����Ҫ����socket.hͷ�ļ� */
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

	char buf[] = {"This is TCP Client from RT-Thread.\r\n"}; /* �����õ������� */

	
    /* ��ʼ��Ԥ���ӵķ���˵�ַ */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NUM);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    while (1)
    {
	    /* ����һ��socket��������SOCKET_STREAM��TCP���� */
	    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	    {
	        /* ����socketʧ�� */
	        rt_kprintf("Socket error\n");
			
			rt_thread_mdelay(5000);
			continue;
	    }

	    /* ���ӵ������ */
	    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
	    {
	        /* ����ʧ�� */
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

