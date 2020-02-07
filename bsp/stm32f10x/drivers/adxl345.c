#include <rtthread.h>
#include <rtdevice.h>

#include "drv_i2c.h"
#include "adxl345.h"

static struct rt_i2c_bus_device *i2c_bus = RT_NULL;     /* I2C�����豸��� */

/* д�������Ĵ��� */
static rt_err_t write_regs(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t data)
{
    rt_uint8_t buf[2] = {reg, data};
    struct rt_i2c_msg msgs;
	
    msgs.addr = DEV_ADXL_ADDR;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = 2;
	
    /* ����I2C�豸�ӿڴ������� */
    if (rt_i2c_transfer(bus, &msgs, 1) == 1) {
        return RT_EOK;
    } else {
        return -RT_ERROR;
    }
}

/* ���������Ĵ������� */
static rt_err_t read_regs(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t len, rt_uint8_t *buf)
{
    struct rt_i2c_msg msgs[2];
	
    msgs[0].addr = DEV_ADXL_ADDR;		
    msgs[0].flags = RT_I2C_WR;		//����д������ַ
    msgs[0].buf = &reg;	
    msgs[0].len = 1;
	
    msgs[1].addr = DEV_ADXL_ADDR;
    msgs[1].flags = RT_I2C_RD;		//�ٶ�
    msgs[1].buf = buf;
    msgs[1].len = len;

	/* ����I2C�豸�ӿڴ������� */
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
	
	write_regs(i2c_bus, REG_POWER_CTL, 0x23);	   	//�����豸��Linkʹ��, wakeupģʽ
    rt_thread_mdelay(10);
	
	write_regs(i2c_bus, reg_wbuf[0][0], reg_wbuf[0][1]);	//��д0x5a
	read_regs(i2c_bus, reg_wbuf[0][0], 1, &reg_rbuf[0]);
	
	write_regs(i2c_bus, reg_wbuf[1][0], reg_wbuf[1][1]);	//��д0xa5
	read_regs(i2c_bus, reg_wbuf[1][0], 1, &reg_rbuf[1]);
	
	if(reg_rbuf[0] == 0x5a && reg_rbuf[1] == 0xa5) {
		return RT_EOK;
	} else {
		rt_kprintf("ADXL345 rw_test Faild, write:[0x5a, 0xa5], read:[0x%x, 0x%x] !\r\n", reg_rbuf[0], reg_rbuf[1]);
        return -RT_ERROR;
	}
}

//��ʼ��ADXL345.
//����ֵ:0,��ʼ���ɹ�;1,��ʼ��ʧ��.
rt_uint8_t ADXL345_Init(void)
{		
	rt_uint8_t i = 0;
	
	rt_uint8_t reg_rbuf[1] = {0};
	
	rt_uint8_t reg_wbuf[7][2] = {{REG_DATA_FORMAT, 0X2B}, {REG_BW_RATE, 0x0A}, {REG_POWER_CTL, 0x28}, {REG_INT_ENABLE, 0x00},
									{REG_OFSX, 0x00}, {REG_OFSY, 0x00}, {REG_OFSZ, 0x00}};
						
	rt_kprintf("ADXL345_Init !\r\n");
	
	/* ����I2C�����豸����ȡI2C�����豸��� */
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(I2C1_BUS_NAME);

	if (i2c_bus == RT_NULL) {
        rt_kprintf("can't find %s device!\n", I2C1_BUS_NAME);
    } else {
    	rw_test();	//���Զ�д����
		
    	read_regs(i2c_bus, REG_DEVICE_ID, 1, reg_rbuf);		//��DEVICE_ID�� �ж��豸�Ƿ���������
		
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

//��ȡ3���������
//x,y,z:��ȡ��������
void ADXL345_RD_XYZ(short *x,short *y,short *z)
{
	rt_uint8_t buf[6];
	
	read_regs(i2c_bus, REG_DATA_BASE, 6, buf);		//�����ݼĴ�����������6���ֽ�
	
	*x=(short)(((u16)buf[1]<<8)+buf[0]); 	    
	*y=(short)(((u16)buf[3]<<8)+buf[2]); 	    
	*z=(short)(((u16)buf[5]<<8)+buf[4]); 
}

#if 0

//��ȡADXL��ƽ��ֵ
//x,y,z:��ȡ10�κ�ȡƽ��ֵ
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

//�Զ�У׼
//xval,yval,zval:x,y,z���У׼ֵ
void ADXL345_AUTO_Adjust(char *xval,char *yval,char *zval)
{
	short tx,ty,tz;
	rt_uint8_t i;
	short offx=0,offy=0,offz=0;
	ADXL345_WR_Reg(POWER_CTL,0x00);	   	//�Ƚ�������ģʽ.
	delay_ms(100);
	ADXL345_WR_Reg(DATA_FORMAT,0X2B);	//�͵�ƽ�ж����,13λȫ�ֱ���,��������Ҷ���,16g���� 
	ADXL345_WR_Reg(BW_RATE,0x0A);		//��������ٶ�Ϊ100Hz
	ADXL345_WR_Reg(POWER_CTL,0x28);	   	//����ʹ��,����ģʽ
	ADXL345_WR_Reg(INT_ENABLE,0x00);	//��ʹ���ж�		 

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
//��ȡADXL345������times��,��ȡƽ��
//x,y,z:����������
//times:��ȡ���ٴ�
void ADXL345_Read_Average(short *x,short *y,short *z,rt_uint8_t times)
{
	rt_uint8_t i;
	short tx,ty,tz;
	*x=0;
	*y=0;
	*z=0;
	if(times)//��ȡ������Ϊ0
	{
		for(i=0;i<times;i++)//������ȡtimes��
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
//�õ��Ƕ�
//x,y,z:x,y,z������������ٶȷ���(����Ҫ��λ,ֱ����ֵ����)
//dir:Ҫ��õĽǶ�.0,��Z��ĽǶ�;1,��X��ĽǶ�;2,��Y��ĽǶ�.
//����ֵ:�Ƕ�ֵ.��λ0.1��.
short ADXL345_Get_Angle(float x,float y,float z,rt_uint8_t dir)
{
	float temp;
 	float res=0;
	switch(dir)
	{
		case 0://����ȻZ��ĽǶ�
 			temp=sqrt((x*x+y*y))/z;
 			res=atan(temp);
 			break;
		case 1://����ȻX��ĽǶ�
 			temp=x/sqrt((y*y+z*z));
 			res=atan(temp);
 			break;
 		case 2://����ȻY��ĽǶ�
 			temp=y/sqrt((x*x+z*z));
 			res=atan(temp);
 			break;
 	}
	return res*1800/3.14;
}

#endif







