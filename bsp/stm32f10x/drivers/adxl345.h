#ifndef __ADXL345_H__
#define __ADXL345_H__

#include <board.h>
#include <rtthread.h>


#define REG_DEVICE_ID		0X00 	//����ID,0XE5
#define REG_THRESH_TAP		0X1D   	//�û���ֵ
#define REG_OFSX			0X1E
#define REG_OFSY			0X1F
#define REG_OFSZ			0X20
#define REG_DUR				0X21
#define REG_Latent			0X22
#define REG_Window  		0X23 
#define REG_THRESH_ACK		0X24
#define REG_THRESH_INACT	0X25 
#define REG_TIME_INACT		0X26
#define REG_ACT_INACT_CTL	0X27	 
#define REG_THRESH_FF		0X28	
#define REG_TIME_FF			0X29 
#define REG_TAP_AXES		0X2A  
#define REG_ACT_TAP_STATUS  0X2B 
#define REG_BW_RATE			0X2C 
#define REG_POWER_CTL		0X2D 

#define REG_INT_ENABLE		0X2E
#define REG_INT_MAP			0X2F
#define REG_INT_SOURCE  	0X30
#define REG_DATA_FORMAT	    0X31
#define REG_DATA_BASE		0X32
#define REG_DATA_X0			0X32
#define REG_DATA_X1			0X33
#define REG_DATA_Y0			0X34
#define REG_DATA_Y1			0X35
#define REG_DATA_Z0			0X36
#define REG_DATA_Z1			0X37
#define REG_FIFO_CTL		0X38
#define REG_FIFO_STATUS		0X39


//0X0B TO OX1F Factory Reserved	 
//���ALT ADDRESS��(12��)�ӵ�,IIC��ַΪ0X53(���������λ).
//�����V3.3,��IIC��ַΪ0X1D(���������λ).
//��Ϊ�������V3.3,����תΪ��д��ַ��,Ϊ0X3B��0X3A(�����GND,��Ϊ0XA7��0XA6)  
#define DEV_ADXL_ADDR		0x1D




rt_uint8_t ADXL345_Init(void); 								//��ʼ��ADXL345
void ADXL345_WR_Reg(rt_uint8_t addr,rt_uint8_t val);				//дADXL345�Ĵ���
rt_uint8_t ADXL345_RD_Reg(rt_uint8_t addr);							//��ADXL345�Ĵ���
void ADXL345_RD_XYZ(short *x,short *y,short *z);	//��ȡһ��ֵ
void ADXL345_RD_Avval(short *x,short *y,short *z);	//��ȡƽ��ֵ
void ADXL345_AUTO_Adjust(char *xval,char *yval,char *zval);//�Զ�У׼
void ADXL345_Read_Average(short *x,short *y,short *z,rt_uint8_t times);//������ȡtimes��,ȡƽ��
short ADXL345_Get_Angle(float x,float y,float z,rt_uint8_t dir);


#endif




































