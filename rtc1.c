#include "rtc1.h"
#include "iic.h"


IIC_CFG SD2058;

u8 BCD(u8 dat)
{
	  return ((dat/10)<<4)|(dat%10);
}

//���RTC��û�е��磬�����ʼ��2021.01.01 ������ 00:00:00
void rtc_init(void)
{	
	u8 dat1,dat2;

    SD2058.SCLPort = PORTP32;
    SD2058.SDAPort = PORTP33;
    Init_IIc_Interface(&SD2058);
	i2cstart(&SD2058);
	i2cbw(0x64,&SD2058);
	i2cbw(0x0f,&SD2058);		//0x0f���λ
	i2cstart(&SD2058);
	i2cbw(0x65,&SD2058);
	dat2 = i2cbr(&SD2058);
	mack(&SD2058);
	dat1 = i2cbr(&SD2058);
	mnak(&SD2058);
	i2cstop(&SD2058);
	if(dat2&0x01)
	{
		if(dat2&0x84)		//WRTC2 WRTC3�Ƿ�Ϊ0����Ϊ0��д0
		{
			dat2 &= ~0x84;
			i2cstart(&SD2058);		
			i2cbw(0x64,&SD2058);
			i2cbw(0x0,&SD2058);
			i2cbw(dat2,&SD2058);
			i2cstop(&SD2058);
		}
		dat2 &= ~0x01;
		//WRTC1�Ƿ�Ϊ0
		if(dat1 & 0x80)		//WRTC1�Ƿ�Ϊ1
		{
			dat1 &= ~0x80;
			i2cstart(&SD2058);		
			i2cbw(0x64,&SD2058);
			i2cbw(0x10,&SD2058);
			i2cbw(dat1,&SD2058);
			i2cstop(&SD2058);
		}
		
		//дʹ��
		dat1 |= 0x80;
		i2cstart(&SD2058);		
		i2cbw(0x64,&SD2058);
		i2cbw(0x10,&SD2058);
		i2cbw(dat1,&SD2058);
		i2cstop(&SD2058);
		dat2 |= 0x84;
		i2cstart(&SD2058);		
		i2cbw(0x64,&SD2058);
		i2cbw(0x0f,&SD2058);
		i2cbw(dat2,&SD2058);
		i2cstop(&SD2058);
		
		//��������ʱ��
		i2cstart(&SD2058);		//10-16=RTCʱ�䣬BCD��ʽ
		i2cbw(0x64,&SD2058);
		i2cbw(0x00,&SD2058);
		i2cbw(0x00,&SD2058);	//��
		i2cbw(0x00,&SD2058);	//��
		i2cbw(0x80,&SD2058);	//ʱ�����λ������12/24��ʽ��
		i2cbw(0x05,&SD2058);	//����
		i2cbw(0x01,&SD2058);	//��
		i2cbw(0x01,&SD2058);	//��
		i2cbw(0x21,&SD2058);	//��
		i2cstop(&SD2058);
		dat2 &= ~0x84;
		dat1 &= ~0x80;
		i2cstart(&SD2058);		
		i2cbw(0x64,&SD2058);
		i2cbw(0x10,&SD2058);
		i2cbw(dat1,&SD2058);
		i2cstop(&SD2058);
		i2cstart(&SD2058);		
		i2cbw(0x64,&SD2058);
		i2cbw(0x0f,&SD2058);
		i2cbw(dat2,&SD2058);
		i2cstop(&SD2058);
	}
}

void rtc_config(u8* prtc_set)
{
	u8 dat,dat1;

	i2cstart(&SD2058);
	i2cbw(0x64,&SD2058);
	i2cbw(0x0f,&SD2058);		//0x10
	i2cstart(&SD2058);
	i2cbw(0x65,&SD2058);
	dat = i2cbr(&SD2058);
	mack(&SD2058);
	dat1 = i2cbr(&SD2058);
	mnak(&SD2058);
	i2cstop(&SD2058);
	dat1 |= 0x80;
	i2cstart(&SD2058);		
	i2cbw(0x64,&SD2058);
	i2cbw(0x10,&SD2058);
	i2cbw(dat1,&SD2058);
	i2cstop(&SD2058);
	dat |= 0x84;
	i2cstart(&SD2058);		
	i2cbw(0x64,&SD2058);
	i2cbw(0x0f,&SD2058);
	i2cbw(dat,&SD2058);
	i2cstop(&SD2058);
	

	i2cstart(&SD2058);		//10-16=RTCʱ�䣬BCD��ʽ
	i2cbw(0x64,&SD2058);
	i2cbw(0x00,&SD2058);
	i2cbw(prtc_set[6],&SD2058);	//��
	i2cbw(prtc_set[5],&SD2058);	//��
	i2cbw(prtc_set[4],&SD2058);	//ʱ
	i2cbw(prtc_set[3],&SD2058);	//����
	i2cbw(prtc_set[2],&SD2058);	//��
	i2cbw(prtc_set[1],&SD2058);	//��
	i2cbw(prtc_set[0],&SD2058);	//��
	i2cstop(&SD2058);
	dat &= ~0x84;
	dat1 &= ~0x80;
	i2cstart(&SD2058);		
	i2cbw(0x64,&SD2058);
	i2cbw(0x10,&SD2058);
	i2cbw(dat1,&SD2058);
	i2cstop(&SD2058);
	i2cstart(&SD2058);		
	i2cbw(0x64,&SD2058);
	i2cbw(0x0f,&SD2058);
	i2cbw(dat,&SD2058);
	i2cstop(&SD2058);
}

//��ȡrtcʱ��
void rdtime(void)
{	
	unsigned char rtcdata[8];
	unsigned char i,n,m;
	i2cstart(&SD2058);
	i2cbw(0x64,&SD2058);
	i2cbw(0x00,&SD2058);
	i2cstart(&SD2058);
	i2cbw(0x65,&SD2058);
	for(i=6;i>0;i--)
	{	
		rtcdata[i]=i2cbr(&SD2058);
		mack(&SD2058);
	}
	rtcdata[0]=i2cbr(&SD2058);
	mnak(&SD2058);
	i2cstop(&SD2058);
	rtcdata[4] &= 0x7F;
	for(i=0;i<3;i++)	//������ת����hex
	{	
		n=rtcdata[i]/16;
		m=rtcdata[i]%16;
		rtcdata[i]=n*10+m;
	}
	for(i=4;i<7;i++)	//ʱ����ת����hex
	{	
		n=rtcdata[i]/16;
		m=rtcdata[i]%16;
		rtcdata[i]=n*10+m;
	}
	//���ڲ��ô���
	rtcdata[7]=0;
	write_dgus_vp(0x0010,(u8*)&rtcdata,4);	//д��ϵͳ�ӿ�
	rtcdata[3]=rtcdata[4];
	rtcdata[4]=rtcdata[5];
	rtcdata[5]=rtcdata[6];
	write_dgus_vp(0xa003,(u8*)&rtcdata,3);
}

u8 rtc_get_week(u8 year,u8 month,u8 day)
{	
	u16 tmp,mon,y;
	u8 week;
	if((month == 1) || (month == 2))
	{
		mon = month + 12;
		y = year - 1;
	}
	else 
	{
		mon = month;
		y = year;
	}
	tmp = y + (y / 4) +(((mon + 1) * 26) / 10) + day - 36;
	week = tmp % 7;
	return week;
}
//you can use this function to set time
void Rtc_set_time(u8* date)
{
	u8 rtc_set[8];
	rtc_set[0] = BCD(date[0]);
	rtc_set[1] = BCD(date[1]);
	rtc_set[2] = BCD(date[2]);
	rtc_set[3] = rtc_get_week(date[0],date[1],date[2]);
	rtc_set[4] = BCD(date[3]);
	rtc_set[4] |= 0x80;
	rtc_set[5] = BCD(date[4]);
	rtc_set[6] = BCD(date[5]);
	rtc_config(rtc_set);


}

// void check_rtc_set(void)
// {
// 	u8 rtc_parm[8],rtc_set[8];
// 	read_dgus_vp(0x9c,rtc_parm,4);
// 	if((rtc_parm[0] == 0x5A) && (rtc_parm[1] == 0xA5))		//����д������
// 	{
// 		rtc_set[0] = BCD(rtc_parm[2]);
// 		rtc_set[1] = BCD(rtc_parm[3]);
// 		rtc_set[2] = BCD(rtc_parm[4]);
// 		rtc_set[3] = rtc_get_week(rtc_parm[2],rtc_parm[3],rtc_parm[4]);
// 		rtc_set[4] = BCD(rtc_parm[5]);
// 		rtc_set[4] |= 0x80;
// 		rtc_set[5] = BCD(rtc_parm[6]);
// 		rtc_set[6] = BCD(rtc_parm[7]);
// 		rtc_config(rtc_set);
// 		rtc_parm[0] = 0;
// 		rtc_parm[1] = 0;
// 		write_dgus_vp(0x9c,rtc_parm,1);
// 	}
// }