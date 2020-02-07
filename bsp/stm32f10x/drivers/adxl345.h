#ifndef __ADXL345_H__
#define __ADXL345_H__

#include <board.h>
#include <rtthread.h>


#define REG_DEVICE_ID		0X00 	//器件ID,0XE5
#define REG_THRESH_TAP		0X1D   	//敲击阀值
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
//如果ALT ADDRESS脚(12脚)接地,IIC地址为0X53(不包含最低位).
//如果接V3.3,则IIC地址为0X1D(不包含最低位).
//因为开发板接V3.3,所以转为读写地址后,为0X3B和0X3A(如果接GND,则为0XA7和0XA6)  
#define DEV_ADXL_ADDR		0x1D




rt_uint8_t ADXL345_Init(void); 								//初始化ADXL345
void ADXL345_WR_Reg(rt_uint8_t addr,rt_uint8_t val);				//写ADXL345寄存器
rt_uint8_t ADXL345_RD_Reg(rt_uint8_t addr);							//读ADXL345寄存器
void ADXL345_RD_XYZ(short *x,short *y,short *z);	//读取一次值
void ADXL345_RD_Avval(short *x,short *y,short *z);	//读取平均值
void ADXL345_AUTO_Adjust(char *xval,char *yval,char *zval);//自动校准
void ADXL345_Read_Average(short *x,short *y,short *z,rt_uint8_t times);//连续读取times次,取平均
short ADXL345_Get_Angle(float x,float y,float z,rt_uint8_t dir);


#endif




































