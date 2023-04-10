#include "rtc_RX8130.h"
sbit	RTC_SDA = P3^3;
sbit	RTC_SCL = P3^2;

void SDA_IN()
	{	P3MDOUT=P3MDOUT&0xF7;}

void SDA_OUT()
	{ P3MDOUT=P3MDOUT|0x08;}
	
void i2cstart()
	{	SDA_OUT();
		RTC_SDA=1;
		RTC_SCL=1;
		delay_us(15);
		RTC_SDA=0;
		delay_us(15);
		RTC_SCL=0;
		delay_us(15);}

void i2cstop()
	{	SDA_OUT();
		RTC_SDA=0;
		RTC_SCL=1;
		delay_us(15);
		RTC_SDA=1;
		delay_us(15);
		SDA_IN();}

void mack()
	{	SDA_OUT();
		RTC_SDA=0;
		delay_us(5);
		RTC_SCL=1;
		delay_us(5);
		RTC_SCL=0;
		delay_us(5);}

void mnak()
	{	SDA_OUT();
		RTC_SDA=1;
		delay_us(5);
		RTC_SCL=1;
		delay_us(5);
		RTC_SCL=0;
		delay_us(5);}
	
void cack()
	{	unsigned char i;
		SDA_IN();
		RTC_SDA=1;
		delay_us(5);
		RTC_SCL=1;
		delay_us(5);
		for(i=0;i<50;i++)
	{   if(!RTC_SDA) break;
		delay_us(5);}
		RTC_SCL=0;
		delay_us(5);
		SDA_OUT();}

//I2C 写入1个字节
void i2cbw(unsigned char dat)
	{	char i;
		SDA_OUT();
		for(i=0;i<8;i++)
	{	if(dat&0x80) RTC_SDA=1;
		else RTC_SDA=0;
		dat=(dat<<1);
		delay_us(5);
		RTC_SCL=1;
		delay_us(5);
		RTC_SCL=0;
		delay_us(5);}
		cack();}

//i2c 读取1个字节数据
unsigned char i2cbr()
	{	char i;
		unsigned char dat=0;
		SDA_IN();
		for(i=0;i<7;i++)
	{	delay_us(5);
		RTC_SCL=1;
		if(RTC_SDA) {dat=dat|0x01;}
		else {dat=dat&0xFE;}
		dat=(dat<<1);
		delay_us(5);
		RTC_SCL=0;
		delay_us(5);}
		delay_us(5);
		RTC_SCL=1;
		if(RTC_SDA) {dat=dat|0x01;}
		else {dat=dat&0xFE;}
		delay_us(5);
		RTC_SCL=0;
		delay_us(5);
		return(dat);}

//检查8130有没有掉电，掉电则初始化8130，设置时间为2017.01.01 星期一 00:00:00
void init_rtc()
	{	unsigned char i;
		// unsigned int tmp;
		// unsigned char xdata k_data[8];
		SetPinOut(3,2);
		SetPinOut(3,3);
		//检查有没有掉电
		i2cstart();
		i2cbw(0x64);
		i2cbw(0x1d);
		i2cstop();
		i2cstart();
		i2cbw(0x65);
		i=i2cbr();
		mack();
		i2cbr();
		mnak();
		i2cstop();
		if((i&0x02)==0x02)
	{	//重新配置时间
		i2cstart();		//30=00
		i2cbw(0x64);
		i2cbw(0x30);
		i2cbw(0x00);
		i2cstop();
		i2cstart();		//1C-1F=48 00 40 10
		i2cbw(0x64);
		i2cbw(0x1C);
		i2cbw(0x48);
		i2cbw(0x00);
		i2cbw(0x40);
		i2cbw(0x10);
		i2cstop();
		i2cstart();		//10-16=RTC设置值 BCD格式
		i2cbw(0x64);
		i2cbw(0x10);
		i2cbw(0x00);	//秒
		i2cbw(0x00);	//分
		i2cbw(0x00);	//时
		i2cbw(0x01);	//星期
		i2cbw(0x01);	//日
		i2cbw(0x01);	//月
		i2cbw(0x17);	//年
		i2cstop();
		i2cstart();		//1E-1F 00 10
		i2cbw(0x64);
		i2cbw(0x1E);
		i2cbw(0x00);	
		i2cbw(0x10);			
		i2cstop();}
		// read_dgus(0x1034,4,k_data);
		// tmp = (unsigned int)k_data[2];
		// tmp = (tmp<<8)+ k_data[3];
		// tmp = tmp/24;
		// k_data[6] = (unsigned char)(tmp >> 8);
		// k_data[7] = (unsigned char)tmp;
		// write_dgus(0x1034,4,k_data);
		
	}

unsigned char code table[24] ={5,1,2,5,0,3,5,1,4,6,2,4,6,2,2,5,0,3,5,1,4,6,2,4};
unsigned char week_calculate(unsigned char year,unsigned char month,unsigned char day)
{
	unsigned char tmp,tmp1,tmp2=0;
	
	if(year%4)
	{
		tmp = table[11+month];
	}
	else
	{
		tmp = table[month-1];
	}
	tmp += day;
	tmp1 = (year/4 + year)%7;
	tmp += tmp1;
	tmp = tmp%7;
	if(tmp == 0)
	{
		tmp2 = 0x01;
	}
	else if(tmp <= 6)
	{
		tmp2 = 0x01 << tmp;
	}
	return(tmp2);
}

void rtc_set(unsigned char*buf)
{
		i2cstart();		//30=00
		i2cbw(0x64);
		i2cbw(0x30);
		i2cbw(0x00);
		i2cstop();
		i2cstart();		//1C-1F=48 00 40 10
		i2cbw(0x64);
		i2cbw(0x1C);
		i2cbw(0x48);
		i2cbw(0x00);
		i2cbw(0x40);
		i2cbw(0x10);
		i2cstop();
		i2cstart();		//10-16=RTC设置值 BCD格式
		i2cbw(0x64);
		i2cbw(0x10);
		i2cbw(buf[6]);	//秒
		i2cbw(buf[5]);	//分
		i2cbw(buf[4]);	//时
		i2cbw(buf[3]);	//星期
		i2cbw(buf[2]);	//日
		i2cbw(buf[1]);	//月
		i2cbw(buf[0]);	//年
		i2cstop();
		i2cstart();		//1E-1F 00 10
		i2cbw(0x64);
		i2cbw(0x1E);
		i2cbw(0x00);	
		i2cbw(0x10);			
		i2cstop();
}

////把RTC读取并处理，写到DGUS对应的变量空间，主程序中每0.5秒调用一次
void rdtime()
		{	
			unsigned char i,n,m;
			u8 rtcdata[8];
			SetPinOut(3,2);
	    SetPinOut(3,3);
			i2cstart();
			i2cbw(0x64);
			i2cbw(0x10);
			i2cstop();
			i2cstart();
			i2cbw(0x65);
			for(i=6;i>0;i--)
		{	rtcdata[i]=i2cbr();
			mack();}
			rtcdata[0]=i2cbr();
			mnak();
			i2cstop();
			for(i=0;i<3;i++)	//年月日转换成HEX
		{	n=rtcdata[i]/16;
			m=rtcdata[i]%16;
			rtcdata[i]=n*10+m;}
			for(i=4;i<7;i++)	//时分秒转换成HEX
		{	n=rtcdata[i]/16;
			m=rtcdata[i]%16;
			rtcdata[i]=n*10+m;}
			//处理星期的数据格式
			n=0;
			m=rtcdata[3];
			for(i=0;i<7;i++)
		{   if(m&0x01)  {break;}
			n++;
			m=(m>>1);}
			rtcdata[3]=n;
			rtcdata[7]=0x00;
			write_dgus_vp(0x0010,rtcdata,4);	//写入DGUS变量空间
		}

