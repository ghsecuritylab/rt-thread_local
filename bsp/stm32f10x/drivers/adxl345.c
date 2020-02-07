#include <rtthread.h>
#include <rtdevice.h>

#include "drv_i2c.h"
#include "adxl345.h"

static struct rt_i2c_bus_device *i2c_bus = RT_NULL;     /* I2C总线设备句柄 */

/* 写传感器寄存器 */
static rt_err_t write_regs(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t data)
{
    rt_uint8_t buf[2] = {reg, data};
    struct rt_i2c_msg msgs;
	
    msgs.addr = DEV_ADXL_ADDR;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = 2;
	
    /* 调用I2C设备接口传输数据 */
    if (rt_i2c_transfer(bus, &msgs, 1) == 1) {
        return RT_EOK;
    } else {
        return -RT_ERROR;
    }
}

/* 读传感器寄存器数据 */
static rt_err_t read_regs(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t len, rt_uint8_t *buf)
{
    struct rt_i2c_msg msgs[2];
	
    msgs[0].addr = DEV_ADXL_ADDR;		
    msgs[0].flags = RT_I2C_WR;		//先是写器件地址
    msgs[0].buf = &reg;	
    msgs[0].len = 1;
	
    msgs[1].addr = DEV_ADXL_ADDR;
    msgs[1].flags = RT_I2C_RD;		//再读
    msgs[1].buf = buf;
    msgs[1].len = len;

	/* 调用I2C设备接口传输数据 */
    if (rt_i2c_transfer(bus, msgs, 2) == 2) {
        return RT_EOK;
    } else {
        return -RT_ERROR;
    }
}

static rt_err_t rw_test(void)
{
	rt_uint8_t reg_rbuf[2] = {0, 0};
	rt_uint8_t reg_wbuf[2][2] = {{REG_ACT_INACT_CTL, 0x5a}, {REG_ACT_INACT_CTL, 0xa5}};
	
	write_regs(i2c_bus, REG_POWER_CTL, 0x23);	   	//激活设备，Link使能, wakeup模式
    rt_thread_mdelay(10);
	
	write_regs(i2c_bus, reg_wbuf[0][0], reg_wbuf[0][1]);	//先写0x5a
	read_regs(i2c_bus, reg_wbuf[0][0], 1, &reg_rbuf[0]);
	
	write_regs(i2c_bus, reg_wbuf[1][0], reg_wbuf[1][1]);	//再写0xa5
	read_regs(i2c_bus, reg_wbuf[1][0], 1, &reg_rbuf[1]);
	
	if(reg_rbuf[0] == 0x5a && reg_rbuf[1] == 0xa5) {
		return RT_EOK;
	} else {
		rt_kprintf("ADXL345 rw_test Faild, write:[0x5a, 0xa5], read:[0x%x, 0x%x] !\r\n", reg_rbuf[0], reg_rbuf[1]);
        return -RT_ERROR;
	}
}

//初始化ADXL345.
//返回值:0,初始化成功;1,初始化失败.
rt_uint8_t ADXL345_Init(void)
{		
	rt_uint8_t i = 0;
	
	rt_uint8_t reg_rbuf[1] = {0};
	
	rt_uint8_t reg_wbuf[7][2] = {{REG_DATA_FORMAT, 0X2B}, {REG_BW_RATE, 0x0A}, {REG_POWER_CTL, 0x28}, {REG_INT_ENABLE, 0x00},
									{REG_OFSX, 0x00}, {REG_OFSY, 0x00}, {REG_OFSZ, 0x00}};
						
	rt_kprintf("ADXL345_Init !\r\n");
	
	/* 查找I2C总线设备，获取I2C总线设备句柄 */
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(I2C1_BUS_NAME);

	if (i2c_bus == RT_NULL) {
        rt_kprintf("can't find %s device!\n", I2C1_BUS_NAME);
    } else {
    	rw_test();	//测试读写函数
		
    	read_regs(i2c_bus, REG_DEVICE_ID, 1, reg_rbuf);		//读DEVICE_ID， 判断设备是否正常在线
		
    	while(reg_rbuf[0] != 0xE5) {
        	rt_kprintf("ADXL345_Init Faild: REG_DEVICE_ID: 0x%x\n", reg_rbuf[0]);
			
    		read_regs(i2c_bus, REG_DEVICE_ID, 1, reg_rbuf);
			
        	rt_thread_delay( RT_TICK_PER_SECOND );
		}

		for(i=0; i<sizeof(reg_wbuf)/sizeof(reg_wbuf[0]); i++) {
			write_regs(i2c_bus, reg_wbuf[i][0], reg_wbuf[i][1]);
		}
    }	   	

	rt_kprintf("ADXL345_Init Success !\r\n");

	return RT_EOK;
}   

//读取3个轴的数据
//x,y,z:读取到的数据
void ADXL345_RD_XYZ(short *x,short *y,short *z)
{
	rt_uint8_t buf[6];
	
	read_regs(i2c_bus, REG_DATA_BASE, 6, buf);		//读数据寄存器，连续读6个字节
	
	*x=(short)(((u16)buf[1]<<8)+buf[0]); 	    
	*y=(short)(((u16)buf[3]<<8)+buf[2]); 	    
	*z=(short)(((u16)buf[5]<<8)+buf[4]); 
}

#if 0

//读取ADXL的平均值
//x,y,z:读取10次后取平均值
void ADXL345_RD_Avval(short *x,short *y,short *z)
{
	short tx=0,ty=0,tz=0;	   
	rt_uint8_t i;  
	for(i=0;i<10;i++)
	{
		ADXL345_RD_XYZ(x,y,z);
		delay_ms(10);
		tx+=(short)*x;
		ty+=(short)*y;
		tz+=(short)*z;	   
	}
	*x=tx/10;
	*y=ty/10;
	*z=tz/10;
} 

//自动校准
//xval,yval,zval:x,y,z轴的校准值
void ADXL345_AUTO_Adjust(char *xval,char *yval,char *zval)
{
	short tx,ty,tz;
	rt_uint8_t i;
	short offx=0,offy=0,offz=0;
	ADXL345_WR_Reg(POWER_CTL,0x00);	   	//先进入休眠模式.
	delay_ms(100);
	ADXL345_WR_Reg(DATA_FORMAT,0X2B);	//低电平中断输出,13位全分辨率,输出数据右对齐,16g量程 
	ADXL345_WR_Reg(BW_RATE,0x0A);		//数据输出速度为100Hz
	ADXL345_WR_Reg(POWER_CTL,0x28);	   	//链接使能,测量模式
	ADXL345_WR_Reg(INT_ENABLE,0x00);	//不使用中断		 

	ADXL345_WR_Reg(OFSX,0x00);
	ADXL345_WR_Reg(OFSY,0x00);
	ADXL345_WR_Reg(OFSZ,0x00);
	delay_ms(12);
	for(i=0;i<10;i++)
	{
		ADXL345_RD_Avval(&tx,&ty,&tz);
		offx+=tx;
		offy+=ty;
		offz+=tz;
	}	 		
	offx/=10;
	offy/=10;
	offz/=10;
	*xval=-offx/4;
	*yval=-offy/4;
	*zval=-(offz-256)/4;	  
 	ADXL345_WR_Reg(OFSX,*xval);
	ADXL345_WR_Reg(OFSY,*yval);
	ADXL345_WR_Reg(OFSZ,*zval);	
} 
//读取ADXL345的数据times次,再取平均
//x,y,z:读到的数据
//times:读取多少次
void ADXL345_Read_Average(short *x,short *y,short *z,rt_uint8_t times)
{
	rt_uint8_t i;
	short tx,ty,tz;
	*x=0;
	*y=0;
	*z=0;
	if(times)//读取次数不为0
	{
		for(i=0;i<times;i++)//连续读取times次
		{
			ADXL345_RD_XYZ(&tx,&ty,&tz);
			*x+=tx;
			*y+=ty;
			*z+=tz;
			delay_ms(5);
		}
		*x/=times;
		*y/=times;
		*z/=times;
	}
}
//得到角度
//x,y,z:x,y,z方向的重力加速度分量(不需要单位,直接数值即可)
//dir:要获得的角度.0,与Z轴的角度;1,与X轴的角度;2,与Y轴的角度.
//返回值:角度值.单位0.1°.
short ADXL345_Get_Angle(float x,float y,float z,rt_uint8_t dir)
{
	float temp;
 	float res=0;
	switch(dir)
	{
		case 0://与自然Z轴的角度
 			temp=sqrt((x*x+y*y))/z;
 			res=atan(temp);
 			break;
		case 1://与自然X轴的角度
 			temp=x/sqrt((y*y+z*z));
 			res=atan(temp);
 			break;
 		case 2://与自然Y轴的角度
 			temp=y/sqrt((x*x+z*z));
 			res=atan(temp);
 			break;
 	}
	return res*1800/3.14;
}

#endif







