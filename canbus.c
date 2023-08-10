#include "canbus.h"
#include "timer.h"
//如果使用CAN，那么sys.h  必须增加如下宏定义  #define INTVPACTION

/*CAN总线位时间参数的设定与调整
1、	确定时间份额
		时间份额数量为8~25
		位时间=1/波特率           ( 250K           4us)
		时间份额=位时间/时间份额数量        
		BRP=时间份额/(2*tclk)=  时间份额/2*FOSC=206*200/2000=40      
2、	设置时间段和采样点
		时间份额数量=1+T1+T2
		采样点80%最佳
		(1+T1)/(1+T1+T2)=0.8
		就可以确定T1  T2
		传播时间段和相位缓冲器段1  =T1
		相位缓冲器段2              =T2
3、	确定同步跳转宽度和采样次数
		同步跳转宽度1~4  尽量大

*/

CANBUSUNIT   CanData;

u8 textChannelsLenght[] = {4, 2, 14, 4, 2, 14, 13, 13, 32, 27, 17, 14, 10, 56, 68};

	//u8 tmp[4][2];
	u8 tmp[50][4][2];
	
	float group[15][68];
	
	u8 tmpD10B[8];
	u8 tmpD10C[8];
	u8 tmpD10D[8];
	u8 tmpD20D[8];
	u8 tmpD20E[8];
	u8 tmpD210[8];
	u8 tmpD313[8];
	u8 tmpD314[8];
	u8 tmpD408[8];
	
	u8 alarmOn[15][68];
	
	
	u16 stationAlarm[4];
	
	//u16 station1AlarmArray[100];
	
	u16 stationTest = 0;

	
	int j=0;
	
	
	//u8 tmp[2],tmp1[2],tmp2[2],tmp3[2];
	
//https://blog.csdn.net/weixin_44536482/article/details/89030152
//125K{0x3F,0x40,0x72,0x00},250K{0x1F,0x40,0x72,0x00},500K{0x0F,0x40,0x72,0x00},1M{0x07,0x40,0x72,0x00}
//根据T5L应用开发指南3.8对CAN接口进行初始化
void CanBusInit(u8* RegCfg)
{
	SetPinOut(0,2);
	SetPinIn(0,3);
	PinOutput(0,2,1);
	MUX_SEL |= 0x80;		//将CAN接口引出到P0.2,P0.3	
	ADR_H = 0xFF;
	ADR_M = 0x00;
	ADR_L = 0x60;
	ADR_INC = 1;
	RAMMODE = 0x8F;		//写操作
	while(!APP_ACK);
	#if 0
	DATA3 = 0x3F;
	DATA2 = 0x40;
	DATA1 = 0x72;
	DATA0 = 0x00;	
	#else
	DATA3 = RegCfg[0];
	DATA2 = RegCfg[1];
	DATA1 = RegCfg[2];
	DATA0 = RegCfg[3];	 		
	#endif		
	APP_EN = 1;
	while(APP_EN);
	DATA3 = 0;
	DATA2 = 0;
	DATA1 = 0;
	DATA0 = 0;	 		//配置验收寄存器ACR
	APP_EN = 1;	  
	while(APP_EN);
	DATA3 = 0xFF;
	DATA2 = 0xFF;
	DATA1 = 0xFF;
	DATA0 = 0xFF;	 	//配置AMR
	APP_EN = 1;	
	while(APP_EN);
	RAMMODE = 0;
	CAN_CR = 0xA0;
	while(CAN_CR&0x20);	//执行配置FF0060-FF0062动作
	ECAN = 1;			//打开CAN中断	
}


/**************************************************************
D3  1  CAN_RX_BUFFER  [7] IDE ，[6]RTR， [3:0]—DLC，帧数据长度。
0xFF:0068
D2:D0  3  未定义 
ID  ID，扩展帧时 29bit 有效，标准帧时 11bit 有效。
D3  1  ID 第一个字节，标准帧与扩展帧。
D2  1  ID 第二个字节，[7:5]为标准帧的高 3bit，扩展帧第 2 字节。
D1  1  ID 第三个字节，标准帧无效，扩展帧第 3 字节。
0xFF:0069
D0  1  ID 第四个字节，标准帧无效，[7:3]-扩展帧的高 5bit。
0xFF:006A  D3:D0  4  数据  接收数据，DATA1-DATA4。
0xFF:006B  D3:D0  4  数据  接收数据，DATA5-DATA8。
******************************************************************/
//对于T5L1和T5L2必须在main函数，while(1)中调用
void CanErrorReset(void)
{
	// EA=0;
	if(CAN_ET&0X20)
	{
		CAN_ET &= 0XDF;
		CAN_CR |= 0X40;
		delay_us(1000);
		CAN_CR &= 0XBF;  
		CanData.CanTxFlag = 0;
	}
	// EA=1;
}


void LoadOneFrame(void)
{
	ADR_H = 0xFF;
	ADR_M = 0x00;
	ADR_L = 0x64;
	ADR_INC = 1;
	RAMMODE = 0x8F;		//写操作
	while(!APP_ACK);
	DATA3 = CanData.BusTXbuf[CanData.CanTxTail].status;			//帧类长度型以及数据
	DATA2 = 0;
	DATA1 = 0;
	DATA0 = 0;	 		
	APP_EN = 1;
	while(APP_EN);		//写入RTR,IDE,DLC等数据
	DATA3 = (u8)(CanData.BusTXbuf[CanData.CanTxTail].ID>>24);
	DATA2 = (u8)(CanData.BusTXbuf[CanData.CanTxTail].ID>>16);
	DATA1 = (u8)(CanData.BusTXbuf[CanData.CanTxTail].ID>>8);
	DATA0 = (u8)(CanData.BusTXbuf[CanData.CanTxTail].ID);	 		
	APP_EN = 1;
	while(APP_EN);		//写入ID数据
	DATA3 = CanData.BusTXbuf[CanData.CanTxTail].candata[0];
	DATA2 = CanData.BusTXbuf[CanData.CanTxTail].candata[1];
	DATA1 = CanData.BusTXbuf[CanData.CanTxTail].candata[2];
	DATA0 = CanData.BusTXbuf[CanData.CanTxTail].candata[3];	 		
	APP_EN = 1;
	while(APP_EN);		//写入发送数据前4字节
	DATA3 = CanData.BusTXbuf[CanData.CanTxTail].candata[4];
	DATA2 = CanData.BusTXbuf[CanData.CanTxTail].candata[5];
	DATA1 = CanData.BusTXbuf[CanData.CanTxTail].candata[6];
	DATA0 = CanData.BusTXbuf[CanData.CanTxTail].candata[7];	 		
	APP_EN = 1;
	while(APP_EN);		//写入发送数据后4字节
	CanData.CanTxTail++;
	RAMMODE = 0;
}

//status主要用于提供IDE 和 RTR状态，实际发送长度有len自动处理，大于8字节会自动拆分成多包
/*主循环调用，将需要发送的数据放在缓存区即可，同时CAN发送会占用定时器7，其余位置则不能在使用*/
void CanTx(u32 ID, u8 status, u16 len, const u8 *pData)
{
	u8 i,j,k,framnum,framoffset;
	u32 idtmp,statustmp;

	if(len>2048)//发送长度大于队列长度
		return;
	if(status&0x80)//扩展帧
	{
		idtmp = ID << 3;
	}
	else
	{
		idtmp = ID << 21;
	}
	if(CanData.BusTXbuf[CanData.CanTxHead].status&0x40)//远程帧不需要发送数据
	{
		CanData.BusTXbuf[CanData.CanTxHead].ID = idtmp;
		CanData.BusTXbuf[CanData.CanTxHead].status = status&0xC0;//远程帧发送长度强制清零
		CanData.CanTxHead++;
	}
	else
	{
		framnum = len >> 3;
		framoffset = len % 8;
		k=0;
		statustmp = status&0xC0;
		for(i=0;i<framnum;i++)
		{
			CanData.BusTXbuf[CanData.CanTxHead].ID = idtmp;
			CanData.BusTXbuf[CanData.CanTxHead].status = statustmp | 0x08;
			for(j=0;j<8;j++)
			{
				CanData.BusTXbuf[CanData.CanTxHead].candata[j] = pData[k];
				k++;
			}
			CanData.CanTxHead++;
		}
		if(framoffset)
		{
			CanData.BusTXbuf[CanData.CanTxHead].ID = idtmp;
			CanData.BusTXbuf[CanData.CanTxHead].status = statustmp | framoffset;
			for(j=0;j<framoffset;j++)
			{
				CanData.BusTXbuf[CanData.CanTxHead].candata[j] = pData[k];
				k++;
			}
			for(;j<8;j++)
				CanData.BusTXbuf[CanData.CanTxHead].candata[j] = 0;
			CanData.CanTxHead++;
		}
	}
	if(0==CanData.CanTxFlag)
	{
		EA = 0;
		LoadOneFrame();
		EA = 1;
		CanData.CanTxFlag = 1;
		StartTimer(7,3000);//3S还未发送出去，则清空发送标记
		CAN_CR |= 0x04;		//启动发送
	}
 	if(CanData.CanTxFlag!=0)
 	{
 		if(GetTimeOutFlag(7))
 		{
 			CanData.CanTxFlag = 0;
 		}
 	}
}

//receive example:
void canRxTreat(void)
{
	//u8 tmp[16];

	if(CanData.CanRxHead != CanData.CanRxTail)
	{
		
		
		if(0x100==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[0][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[0][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[0][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[0][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[0][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[0][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
			
			tmp[0][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[0][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			group[2][6] 	= ((((u16)tmp[0][0][1])<<8) + (u16)tmp[0][0][0]) + ((float)tmp[0][1][1])/100;
			group[6][12] 	= ((((u16)tmp[0][2][1])<<8) + (u16)tmp[0][2][0]) + ((float)tmp[0][3][1])/100;
				
			alarmOn[2][6]		=(u16)tmp[0][1][0]&1;
			alarmOn[6][12]		=(u16)tmp[0][3][0]&1;
			
			/*
			group[2][4] = (((u16)tmp[0][2][1])<<8) + (u16)tmp[0][2][0];
			
			group[2][5] = (((u16)tmp[0][1][1])<<8) + (u16)tmp[0][1][0];
			
			
			write_dgus_vp(0x1140,tmp[0][0],1);
			write_dgus_vp(0x1160,tmp[0][1],1);
			write_dgus_vp(0x1180,tmp[0][2],1);
			write_dgus_vp(0x1200,tmp[0][3],1);
			*/
			
//			tmp[1] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
//			tmp[0] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
//			
//			tmp1[1] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
//			tmp1[0] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
//			
//			tmp2[1] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
//			tmp2[0] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
//			
//			tmp3[1] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
//			tmp3[0] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
//			
//			write_dgus_vp(0x1000,tmp,1);
//			write_dgus_vp(0x1020,tmp1,1);
//			write_dgus_vp(0x1040,tmp2,1);
//			write_dgus_vp(0x1060,tmp3,1);
		}
		
		if(0x101==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[1][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[1][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[1][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[1][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[1][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[1][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[1][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[1][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			group[2][9] 	= ((((u16)tmp[1][0][1])<<8) + (u16)tmp[1][0][0]) + ((float)tmp[1][1][1])/100;
			//group[6][4] 	= ((((u16)tmp[1][2][1])<<8) + (u16)tmp[1][2][0]) + ((float)tmp[1][3][1])/100;
				
			alarmOn[2][9]		=(u16)tmp[1][1][0]&1;
			//alarmOn[6][4]		=((u16)tmp[1][3][0]&1;
		}
		
		if(0x102==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[2][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[2][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
		 
			tmp[2][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[2][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[2][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[2][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[2][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[2][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			
			group[8][8] 	= ((((u16)tmp[2][0][1])<<8) + (u16)tmp[2][0][0]) + ((float)tmp[2][1][1])/100;
			group[8][12] 	= ((((u16)tmp[2][2][1])<<8) + (u16)tmp[2][2][0]) + ((float)tmp[2][3][1])/100;
				
			alarmOn[8][8]		=(u16)tmp[2][1][0]&1;
			alarmOn[8][12]	=(u16)tmp[2][3][0]&1;
				
		}
		
		if(0x103==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[3][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[3][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[3][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[3][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[3][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[3][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
	 
			tmp[3][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[3][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			group[2][4] 	= ((((u16)tmp[3][0][1])<<8) + (u16)tmp[3][0][0]) + ((float)tmp[3][1][1])/100;
			group[2][5] 	= ((((u16)tmp[3][2][1])<<8) + (u16)tmp[3][2][0]) + ((float)tmp[3][3][1])/100;
				
			alarmOn[2][4]		=(u16)tmp[3][1][0]&1;
			alarmOn[2][5]		=(u16)tmp[3][3][0]&1;
		}
		/*
		if(0x104==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[4][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[4][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[4][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[4][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[4][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[4][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[4][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[4][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
		}
		*/
		if(0x105==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[5][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[5][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[5][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[5][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[5][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[5][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[5][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[5][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			group[2][7] 	= ((((u16)tmp[5][0][1])<<8) + (u16)tmp[5][0][0]) + ((float)tmp[5][1][1])/100;
			group[2][8] 	= ((((u16)tmp[5][2][1])<<8) + (u16)tmp[5][2][0]) + ((float)tmp[5][3][1])/100;
				
			alarmOn[2][7]		=(u16)tmp[5][1][0]&1;
			alarmOn[2][8]		=(u16)tmp[5][3][0]&1;
		}
		
		if(0x106==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[6][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[6][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[6][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[6][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[6][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[6][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[6][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[6][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			group[2][10] 	= ((((u16)tmp[6][0][1])<<8) + (u16)tmp[6][0][0]) + ((float)tmp[6][1][1])/100;
			//group[6][5] 	= ((((u16)tmp[6][2][1])<<8) + (u16)tmp[6][2][0]) + ((float)tmp[6][3][1])/100;
			
			alarmOn[2][10]	=(u16)tmp[6][1][0]&1;
			//alarmOn[6][5]		=((u16)tmp[6][3][0]&1;
		}
		
		if(0x107==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			tmp[7][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[7][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[7][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[7][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[7][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[7][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[7][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[7][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			//group[6][6] 	= ((((u16)tmp[7][0][1])<<8) + (u16)tmp[7][0][0]) + ((float)tmp[7][1][1])/100;
			group[6][8] 	= ((((u16)tmp[7][2][1])<<8) + (u16)tmp[7][2][0]) + ((float)tmp[7][3][1])/100;
				
			//alarmOn[6][6]		=((u16)tmp[7][1][0]&1;
			alarmOn[6][8]		=(u16)tmp[7][3][0]&1;
		}
		
		if(0x108==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			tmp[8][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[8][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[8][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[8][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[8][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[8][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[8][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[8][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			group[8][9] 	= ((((u16)tmp[8][0][1])<<8) + (u16)tmp[8][0][0]) + ((float)tmp[8][1][1])/100;
			group[8][10] 	= ((((u16)tmp[8][2][1])<<8) + (u16)tmp[8][2][0]) + ((float)tmp[8][3][1])/100;
				
			alarmOn[8][9] 	=(u16)tmp[8][1][0]&1;
			alarmOn[8][10]	=(u16)tmp[8][3][0]&1;
		}
				if(0x109==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			tmp[9][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[9][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[9][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[9][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[9][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[9][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[9][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[9][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			group[8][11] 	= ((((u16)tmp[9][0][1])<<8) + (u16)tmp[9][0][0]) + ((float)tmp[9][1][1])/100;
			group[8][13] 	= ((((u16)tmp[9][2][1])<<8) + (u16)tmp[9][2][0]) + ((float)tmp[9][3][1])/100;
				
			alarmOn[8][11]		=(u16)tmp[9][1][0]&1;
			alarmOn[8][13]		=(u16)tmp[9][3][0]&1;
		}

				if(0x10B==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			tmpD10B[0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmpD10B[1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			tmpD10B[2] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmpD10B[3] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			tmpD10B[4] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmpD10B[5] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
			tmpD10B[6] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmpD10B[7] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			alarmOn[0][0]		= tmpD10B[0]  	& 1;
			alarmOn[0][1]		= tmpD10B[0]>>2	& 1;
			alarmOn[0][2]		= tmpD10B[0]>>4	& 1;
			alarmOn[0][3]		= tmpD10B[0]>>6	& 1;
			
			alarmOn[2][0]		= tmpD10B[1]		& 1;
			alarmOn[2][1]		= tmpD10B[1]>>2	& 1;
			alarmOn[2][2]		= tmpD10B[1]>>4	& 1;
			alarmOn[2][3]		= tmpD10B[1]>>6	& 1;
			
			alarmOn[6][0]		= tmpD10B[2]		& 1;
			
			alarmOn[2][11]	= tmpD10B[2]>>2	& 1;
			alarmOn[2][12]	= tmpD10B[2]>>4	& 1;
			alarmOn[1][0]		= tmpD10B[2]>>6	& 1;
			alarmOn[1][1]		= tmpD10B[3]		& 1;
			alarmOn[2][13]	= tmpD10B[3]>>2	& 1;
			alarmOn[6][1]		= tmpD10B[3]>>4	& 1;
			alarmOn[6][4]		= tmpD10B[3]>>6	& 1;
			
			alarmOn[6][2]		= tmpD10B[4]		& 1;
			alarmOn[6][3]		= tmpD10B[4]>>2	& 1;
			
			alarmOn[6][7]		= tmpD10B[4]>>4	& 1;
			
			alarmOn[6][9]		= tmpD10B[4]>>6	& 1;
			alarmOn[6][10]	= tmpD10B[5]		& 1;
			alarmOn[6][11]	= tmpD10B[5]>>2	& 1;
			alarmOn[8][0]		= tmpD10B[5]>>4	& 1;
			alarmOn[8][1]		= tmpD10B[5]>>6	& 1;
			alarmOn[8][2]		= tmpD10B[6]		& 1;
			alarmOn[8][3]		= tmpD10B[6]>>2	& 1;
			alarmOn[8][4]		= tmpD10B[6]>>4	& 1;
			alarmOn[8][5]		= tmpD10B[6]>>6	& 1;
			alarmOn[8][6]		= tmpD10B[7]		& 1;
			alarmOn[8][7]		= tmpD10B[7]>>2	& 1;
			alarmOn[8][14]	= tmpD10B[7]>>4	& 1;
		}
		
				if(0x10C==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			tmpD10C[0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmpD10C[1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			tmpD10C[2] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmpD10C[3] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			tmpD10C[4] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmpD10C[5] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
			tmpD10C[6] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmpD10C[7] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			alarmOn[9][0]		= tmpD10C[0]	  & 1;
			alarmOn[9][1]		= tmpD10C[0]>>2	& 1;
			alarmOn[9][2]		= tmpD10C[0]>>4	& 1;
			alarmOn[9][3]		= tmpD10C[0]>>6	& 1;
			
			
			alarmOn[9][4]		= tmpD10C[1]>>2 & 1;
			alarmOn[9][5]		= tmpD10C[1]>>4	& 1;
			alarmOn[9][6]		= tmpD10C[1]>>6	& 1;
			alarmOn[9][7]		= tmpD10C[2]		& 1;
			
			alarmOn[9][8]		= tmpD10C[2]>>4 & 1;
			alarmOn[9][9]		= tmpD10C[2]>>6	& 1;
			alarmOn[9][10]		= tmpD10C[3]		& 1;
			alarmOn[9][11]		= tmpD10C[3]>>2 & 1;
			alarmOn[9][12]		= tmpD10C[3]>>4	& 1;
			alarmOn[9][13]		= tmpD10C[3]>>6	& 1;
			alarmOn[9][14]		= tmpD10C[4]		& 1;
			alarmOn[9][15]		= tmpD10C[4]>>2 & 1;
			alarmOn[9][16]		= tmpD10C[4]>>4	& 1;
			alarmOn[9][17]		= tmpD10C[4]>>6	& 1;
			alarmOn[9][18]		= tmpD10C[5]		& 1;
			alarmOn[9][19]		= tmpD10C[5]>>2 & 1;
			
			alarmOn[10][0]		= tmpD10C[5]>>6	& 1;
			alarmOn[10][1]		= tmpD10C[6]		& 1;
			alarmOn[10][2]		= tmpD10C[6]>>2	& 1;
			alarmOn[10][3]		= tmpD10C[6]>>4	& 1;
			alarmOn[10][4]		= tmpD10C[6]>>6	& 1;
			alarmOn[10][5]		= tmpD10C[7]		& 1;
			alarmOn[10][6]		= tmpD10C[7]>>2	& 1;
			alarmOn[10][7]		= tmpD10C[7]>>4	& 1;
			alarmOn[10][8]		= tmpD10C[7]>>6	& 1;
			
		}
				if(0x10D==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			tmpD10D[0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmpD10D[1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			tmpD10D[2] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmpD10D[3] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			tmpD10D[4] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmpD10D[5] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
			tmpD10D[6] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmpD10D[7] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			alarmOn[10][9]		= tmpD10D[0]		& 1;
			
			alarmOn[6][5]		= tmpD10D[1]>>4	& 1;
			alarmOn[6][6]		= tmpD10D[1]>>6	& 1;
			
			alarmOn[14][0]		= tmpD10D[2]		& 1;
			alarmOn[14][1]		= tmpD10D[2]>>2	& 1;
			alarmOn[14][2]		= tmpD10D[2]>>4	& 1;
			alarmOn[14][3]		= tmpD10D[2]>>6	& 1;
			alarmOn[14][4]		= tmpD10D[3]		& 1;
		}
		
			if(0x110==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			stationAlarm[0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
		}
			if(0x250==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			stationAlarm[1] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
		}
			if(0x350==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			stationAlarm[2] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
		}
			if(0x450==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			stationAlarm[3] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
		}
		
		
		
		
		/*
				if(0x10E==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[46][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[46][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[46][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[46][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[46][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[46][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[46][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[46][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			
			
			stationAlarm[0] 	= (((u16)tmp[46][0][1])<<8) + (u16)tmp[46][0][0];
			stationAlarm[1] 	= (((u16)tmp[46][1][1])<<8) + (u16)tmp[46][1][0];
			stationAlarm[2] 	= (((u16)tmp[46][2][1])<<8) + (u16)tmp[46][2][0];
			stationAlarm[3] 	= (((u16)tmp[46][3][1])<<8) + (u16)tmp[46][3][0];
			
			if(station1AlarmArray[0] != stationAlarm[0])
			{
			
				station1AlarmArray[0] = stationAlarm[0];
				for(j=0;j<100;j++)
				{
					station1AlarmArray[j+1] = station1AlarmArray[j];
				}
				
			}
			
			//station alarm serisi arrayi yart ve bunlari ora depola
		}
		*/

		if(0x200==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[10][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[10][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[10][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[10][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[10][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[10][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[10][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[10][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			group[5][6] 	= ((((u16)tmp[10][0][1])<<8) + (u16)tmp[10][0][0]) + ((float)tmp[10][1][1])/100;
			group[7][12] 	= ((((u16)tmp[10][2][1])<<8) + (u16)tmp[10][2][0]) + ((float)tmp[10][3][1])/100; 
			
			alarmOn[5][6]		=(u16)tmp[10][1][0]&1;
			alarmOn[7][12]	=(u16)tmp[10][3][0]&1;
		}
		
		
		
		if(0x201==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[11][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[11][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[11][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[11][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[11][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[11][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[11][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[11][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			group[5][9] 	= ((((u16)tmp[11][0][1])<<8) + (u16)tmp[11][0][0]) + ((float)tmp[11][1][1])/100;
			//group[7][4] 	= ((((u16)tmp[11][2][1])<<8) + (u16)tmp[11][2][0]) + ((float)tmp[11][3][1])/100; 
				
			alarmOn[5][9]		=(u16)tmp[11][1][0]&1;
			//alarmOn[7][4]		=((u16)tmp[11][3][0]&1;
		}
		
		if(0x202==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[12][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[12][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[12][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[12][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[12][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[12][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[12][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[12][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			group[8][23] 	= ((((u16)tmp[12][0][1])<<8) + (u16)tmp[12][0][0]) + ((float)tmp[12][1][1])/100;
			group[8][27] 	= ((((u16)tmp[12][2][1])<<8) + (u16)tmp[12][2][0]) + ((float)tmp[12][3][1])/100; 
				
			alarmOn[8][23]		=(u16)tmp[12][1][0]&1;
			alarmOn[8][27]		=(u16)tmp[12][3][0]&1;
		}
		if(0x203==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[13][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[13][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[13][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[13][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[13][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[13][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[13][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[13][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			//group[8][23] 	= ((((u16)tmp[12][0][1])<<8) + (u16)tmp[12][0][0]) + ((float)tmp[12][1][1])/100;
			//group[8][27] 	= ((((u16)tmp[12][2][1])<<8) + (u16)tmp[12][2][0]) + ((float)tmp[12][3][1])/100; 
		}
		if(0x204==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[14][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[14][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[14][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[14][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[14][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[14][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[14][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[14][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			group[5][4] 	= ((((u16)tmp[14][0][1])<<8) + (u16)tmp[14][0][0]) + ((float)tmp[14][1][1])/100;
			group[5][5] 	= ((((u16)tmp[14][2][1])<<8) + (u16)tmp[14][2][0]) + ((float)tmp[14][3][1])/100; 
				
			alarmOn[5][4]		=(u16)tmp[14][1][0]&1;
			alarmOn[5][5]		=(u16)tmp[14][3][0]&1;
		}
		/*
		if(0x205==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[15][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[15][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[15][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[15][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[15][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[15][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[15][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[15][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			//group[5][4] 	= ((((u16)tmp[15][0][1])<<8) + (u16)tmp[15][0][0]) + ((float)tmp[15][1][1])/100;
			//group[5][5] 	= ((((u16)tmp[15][2][1])<<8) + (u16)tmp[15][2][0]) + ((float)tmp[15][3][1])/100; 
		}
		*/
		if(0x206==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[16][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[16][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[16][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[16][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[16][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[16][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[16][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[16][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			group[5][7] 	= ((((u16)tmp[16][0][1])<<8) + (u16)tmp[16][0][0]) + ((float)tmp[16][1][1])/100;
			group[5][8] 	= ((((u16)tmp[16][2][1])<<8) + (u16)tmp[16][2][0]) + ((float)tmp[16][3][1])/100; 
				
			alarmOn[5][7]		=(u16)tmp[16][1][0]&1;
			alarmOn[5][8]		=(u16)tmp[16][3][0]&1;
		}
		if(0x207==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[17][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[17][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[17][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[17][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[17][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[17][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[17][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[17][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			group[5][10] 	= ((((u16)tmp[17][0][1])<<8) + (u16)tmp[17][0][0]) + ((float)tmp[17][1][1])/100;
			//group[7][5] 	= ((((u16)tmp[17][2][1])<<8) + (u16)tmp[17][2][0]) + ((float)tmp[17][3][1])/100;
				
			alarmOn[5][10]	=(u16)tmp[17][1][0]&1;
			//alarmOn[5][5]		=((u16)tmp[17][3][0]&1;
		}
		if(0x208==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[18][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[18][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[18][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[18][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[18][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[18][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[18][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[18][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			//group[7][6] 	= ((((u16)tmp[18][0][1])<<8) + (u16)tmp[18][0][0]) + ((float)tmp[18][1][1])/100;
			group[7][8] 	= ((((u16)tmp[18][2][1])<<8) + (u16)tmp[18][2][0]) + ((float)tmp[18][3][1])/100;
				
			//alarmOn[7][6]		=((u16)tmp[18][1][0]&1;
			alarmOn[7][8]		=(u16)tmp[18][3][0]&1;
		}
		if(0x209==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[19][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[19][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
		
			tmp[19][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[19][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[19][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[19][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[19][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[19][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			group[8][24] 	= ((((u16)tmp[19][0][1])<<8) + (u16)tmp[19][0][0]) + ((float)tmp[19][1][1])/100;
			group[8][25] 	= ((((u16)tmp[19][2][1])<<8) + (u16)tmp[19][2][0]) + ((float)tmp[19][3][1])/100;
				
			alarmOn[8][24]		=(u16)tmp[19][1][0]&1;
			alarmOn[8][25]		=(u16)tmp[19][3][0]&1;
		}
		if(0x20A==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[20][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[20][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
		
			tmp[20][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[20][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[20][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[20][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[20][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[20][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			group[8][26] 	= ((((u16)tmp[20][0][1])<<8) + (u16)tmp[20][0][0]) + ((float)tmp[20][1][1])/100;
			group[8][28] 	= ((((u16)tmp[20][2][1])<<8) + (u16)tmp[20][2][0]) + ((float)tmp[20][3][1])/100;
				
			alarmOn[8][26]		=(u16)tmp[20][1][0]&1;
			alarmOn[8][28]		=(u16)tmp[20][3][0]&1;
		}
		/*
		if(0x20B==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[21][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[21][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
		
			tmp[21][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[21][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[21][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[21][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[21][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[21][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			//group[8][26] 	= ((((u16)tmp[21][0][1])<<8) + (u16)tmp[21][0][0]) + ((float)tmp[21][1][1])/100;
			//group[8][28] 	= ((((u16)tmp[21][2][1])<<8) + (u16)tmp[21][2][0]) + ((float)tmp[21][3][1])/100;
		}
		*/
		if(0x20D==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			tmpD20D[0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmpD20D[1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			tmpD20D[2] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmpD20D[3] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			tmpD20D[4] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmpD20D[5] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
			tmpD20D[6] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmpD20D[7] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			alarmOn[14][5]	= tmpD20D[0]  	& 1;
			alarmOn[3][0]		= tmpD20D[0]>>2	& 1;
			alarmOn[3][1]		= tmpD20D[0]>>4	& 1;
			alarmOn[3][2]		= tmpD20D[0]>>6	& 1;
			alarmOn[3][3]		= tmpD20D[1]   	& 1;
			
			alarmOn[5][0]		= tmpD20D[1]>>2	& 1;
			alarmOn[5][1]		= tmpD20D[1]>>4	& 1;
			alarmOn[5][2]		= tmpD20D[1]>>6	& 1;
			alarmOn[5][3]		= tmpD20D[2]		& 1;
			
			alarmOn[7][4]		= tmpD20D[4]	& 1;
			
			alarmOn[7][0]		= tmpD20D[2]>>2	& 1;
			
			alarmOn[5][11]	= tmpD20D[2]>>4	& 1;
			alarmOn[5][12]	= tmpD20D[2]>>6	& 1;
			alarmOn[4][0]		= tmpD20D[3]		& 1;
			alarmOn[4][1]		= tmpD20D[3]>>2	& 1;
			alarmOn[5][13]	= tmpD20D[3]>>4	& 1;
			
			alarmOn[7][1]		= tmpD20D[3]>>6	& 1;
			
			alarmOn[7][2]		= tmpD20D[4]>>2	& 1;
			alarmOn[7][3]		= tmpD20D[4]>>4	& 1;
			
			alarmOn[7][7]		= tmpD20D[4]>>6	& 1;
			
			alarmOn[7][9]		= tmpD20D[5]		& 1;
			alarmOn[7][10]	= tmpD20D[5]>>2	& 1;
			alarmOn[7][11]	= tmpD20D[5]>>4	& 1;
			alarmOn[8][15]	= tmpD20D[5]>>6	& 1;
			alarmOn[8][16]	= tmpD20D[6]		& 1;
			alarmOn[8][17]	= tmpD20D[6]>>2	& 1;
			alarmOn[8][18]	= tmpD20D[6]>>4	& 1;
			alarmOn[8][19]	= tmpD20D[6]>>6	& 1;
			alarmOn[8][20]	= tmpD20D[7]		& 1;
			alarmOn[8][21]	= tmpD20D[7]>>2	& 1;
			alarmOn[8][22]	= tmpD20D[7]>>4	& 1;
			
			alarmOn[8][29]	= tmpD20D[7]>>6	& 1;
		}
		
		if(0x20E==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			tmpD20E[0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmpD20E[1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			tmpD20E[2] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmpD20E[3] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			tmpD20E[4] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmpD20E[5] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
			tmpD20E[6] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmpD20E[7] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			alarmOn[11][0]		= tmpD20E[0]		& 1;
			alarmOn[11][1]		= tmpD20E[0]>>2	& 1;
			alarmOn[11][2]		= tmpD20E[0]>>4	& 1;
			alarmOn[11][3]		= tmpD20E[0]>>6	& 1;
			alarmOn[11][4]		= tmpD20E[1]		& 1;
			alarmOn[11][5]		= tmpD20E[1]>>2 & 1;
			alarmOn[11][6]		= tmpD20E[1]>>4	& 1;
			alarmOn[11][7]		= tmpD20E[1]>>6	& 1;
			alarmOn[11][8]		= tmpD20E[2]		& 1;
			alarmOn[11][9]		= tmpD20E[2]>>2 & 1;
			alarmOn[11][10]		= tmpD20E[2]>>4 & 1;
			alarmOn[11][11]		= tmpD20E[2]>>6	& 1;
			alarmOn[11][12]		= tmpD20E[3]		& 1;
			alarmOn[11][13]		= tmpD20E[3]>>2 & 1;
			
			alarmOn[8][30]		= tmpD20E[3]>>4	& 1;
			alarmOn[8][31]		= tmpD20E[3]>>6	& 1;
			alarmOn[12][0]		= tmpD20E[4]		& 1;
			alarmOn[12][1]		= tmpD20E[4]>>2 & 1;
			alarmOn[12][2]		= tmpD20E[4]>>4	& 1;
			alarmOn[12][3]		= tmpD20E[4]>>6	& 1;
			alarmOn[12][4]		= tmpD20E[5]		& 1;
			alarmOn[12][5]		= tmpD20E[5]>>2 & 1;
			alarmOn[12][6]		= tmpD20E[5]>>4 & 1;
			alarmOn[12][7]		= tmpD20E[5]>>6	& 1;
			alarmOn[12][8]		= tmpD20E[6]		& 1;
			alarmOn[12][9]		= tmpD20E[6]>>2	& 1;
			
			alarmOn[7][5]		= tmpD20E[6]>>4	& 1;
			alarmOn[7][6]		= tmpD20E[6]>>6	& 1;
			
			alarmOn[14][6]		= tmpD20E[7]		& 1;
			
			alarmOn[14][7]		= tmpD20E[7]>>4	& 1;
			alarmOn[9][20]		= tmpD20E[7]>>6	& 1;
			
		}
		
		if(0x210==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			tmpD210[0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmpD210[1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			tmpD210[2] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmpD210[3] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			tmpD210[4] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmpD210[5] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
			tmpD210[6] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmpD210[7] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			alarmOn[9][21]		= tmpD210[0]		& 1;
			alarmOn[9][22]		= tmpD210[0]>>2 & 1;
			alarmOn[9][23]		= tmpD210[0]>>4	& 1;
			alarmOn[9][24]		= tmpD210[0]>>6	& 1;
			alarmOn[9][25]		= tmpD210[1]		& 1;
			alarmOn[9][26]		= tmpD210[1]>>2 & 1;
			
		}
		
		if(0x300==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[22][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[22][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
	 
			tmp[22][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[22][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[22][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[22][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[22][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[22][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][19] 	= ((((u16)tmp[22][0][1])<<8) + (u16)tmp[22][0][0]) + ((float)tmp[22][1][1])/100;
			group[13][20] 	= ((((u16)tmp[22][2][1])<<8) + (u16)tmp[22][2][0]) + ((float)tmp[22][3][1])/100;
				
			alarmOn[13][19]		=(u16)tmp[22][1][0]&1;
			alarmOn[13][20]		=(u16)tmp[22][3][0]&1;
		}
		if(0x301==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[23][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[23][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
		
			tmp[23][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[23][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[23][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[23][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[23][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[23][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][21] 	= ((((u16)tmp[23][0][1])<<8) + (u16)tmp[23][0][0]) + ((float)tmp[23][1][1])/100;
			group[13][22] 	= ((((u16)tmp[23][2][1])<<8) + (u16)tmp[23][2][0]) + ((float)tmp[23][3][1])/100;
				
			alarmOn[13][21]		=(u16)tmp[23][1][0]&1;
			alarmOn[13][22]		=(u16)tmp[23][3][0]&1;
		}
		if(0x302==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[24][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[24][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
	 
			tmp[24][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[24][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[24][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[24][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[24][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[24][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][23] 	= ((((u16)tmp[24][0][1])<<8) + (u16)tmp[24][0][0]) + ((float)tmp[24][1][1])/100;
			group[13][24] 	= ((((u16)tmp[24][2][1])<<8) + (u16)tmp[24][2][0]) + ((float)tmp[24][3][1])/100;
				
			alarmOn[13][23]		=(u16)tmp[24][1][0]&1;
			alarmOn[13][24]		=(u16)tmp[24][3][0]&1;
		}
		if(0x303==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[25][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[25][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
		
			tmp[25][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[25][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[25][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[25][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[25][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[25][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][25] 	= ((((u16)tmp[25][0][1])<<8) + (u16)tmp[25][0][0]) + ((float)tmp[25][1][1])/100;
			group[13][26] 	= ((((u16)tmp[25][2][1])<<8) + (u16)tmp[25][2][0]) + ((float)tmp[25][3][1])/100;
				
			alarmOn[13][25]		=(u16)tmp[25][1][0]&1;
			alarmOn[13][26]		=(u16)tmp[25][3][0]&1;
		}
		if(0x304==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[26][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[26][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
		
			tmp[26][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[26][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[26][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[26][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[26][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[26][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][27] 	= ((((u16)tmp[26][0][1])<<8) + (u16)tmp[26][0][0]) + ((float)tmp[26][1][1])/100;
			group[13][28] 	= ((((u16)tmp[26][2][1])<<8) + (u16)tmp[26][2][0]) + ((float)tmp[26][3][1])/100;
				
			alarmOn[13][27]		=(u16)tmp[26][1][0]&1;
			alarmOn[13][28]		=(u16)tmp[26][3][0]&1;
		}
		if(0x305==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[27][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[27][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
		
			tmp[27][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[27][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[27][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[27][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[27][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[27][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][29] 	= ((((u16)tmp[27][0][1])<<8) + (u16)tmp[27][0][0]) + ((float)tmp[27][1][1])/100;
			group[13][30] 	= ((((u16)tmp[27][2][1])<<8) + (u16)tmp[27][2][0]) + ((float)tmp[27][3][1])/100;
				
			alarmOn[13][29]		=(u16)tmp[27][1][0]&1;
			alarmOn[13][30]		=(u16)tmp[27][3][0]&1;
		}
		if(0x306==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[28][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[28][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
		
			tmp[28][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[28][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[28][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[28][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[28][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[28][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][31] 	= ((((u16)tmp[28][0][1])<<8) + (u16)tmp[28][0][0]) + ((float)tmp[28][1][1])/100;
			group[13][32] 	= ((((u16)tmp[28][2][1])<<8) + (u16)tmp[28][2][0]) + ((float)tmp[28][3][1])/100;
				
			alarmOn[13][31]		=(u16)tmp[28][1][0]&1;
			alarmOn[13][32]		=(u16)tmp[28][3][0]&1;
		}
		if(0x307==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[29][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[29][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
		
			tmp[29][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[29][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[29][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[29][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[29][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[29][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][33] 	= ((((u16)tmp[29][0][1])<<8) + (u16)tmp[29][0][0]) + ((float)tmp[29][1][1])/100;
			group[13][34] 	= ((((u16)tmp[29][2][1])<<8) + (u16)tmp[29][2][0]) + ((float)tmp[29][3][1])/100;
				
			alarmOn[13][33]		=(u16)tmp[29][1][0]&1;
			alarmOn[13][34]		=(u16)tmp[29][3][0]&1;
		}
		if(0x308==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[30][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[30][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
		
			tmp[30][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[30][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[30][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[30][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[30][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[30][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][35] 	= ((((u16)tmp[30][0][1])<<8) + (u16)tmp[30][0][0]) + ((float)tmp[30][1][1])/100;
			group[13][36] 	= ((((u16)tmp[30][2][1])<<8) + (u16)tmp[30][2][0]) + ((float)tmp[30][3][1])/100;
				
			alarmOn[13][35]		=(u16)tmp[30][1][0]&1;
			alarmOn[13][36]		=(u16)tmp[30][3][0]&1;
		}
		if(0x309==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[31][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[31][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
	 
			tmp[31][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[31][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[31][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[31][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[31][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[31][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][37] 	= ((((u16)tmp[31][0][1])<<8) + (u16)tmp[31][0][0]) + ((float)tmp[31][1][1])/100;
			group[13][38] 	= ((((u16)tmp[31][2][1])<<8) + (u16)tmp[31][2][0]) + ((float)tmp[31][3][1])/100;
				
			alarmOn[13][37]		=(u16)tmp[31][1][0]&1;
			alarmOn[13][38]		=(u16)tmp[31][3][0]&1;
		}
		if(0x30A==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[32][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[32][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
	
			tmp[32][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[32][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[32][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[32][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[32][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[32][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][39] 	= ((((u16)tmp[32][0][1])<<8) + (u16)tmp[32][0][0]) + ((float)tmp[32][1][1])/100;
			group[13][40] 	= ((((u16)tmp[32][2][1])<<8) + (u16)tmp[32][2][0]) + ((float)tmp[32][3][1])/100;
				
			alarmOn[13][39]		=(u16)tmp[32][1][0]&1;
			alarmOn[13][40]		=(u16)tmp[32][3][0]&1;
		}
		if(0x30B==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[33][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[33][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
	
			tmp[33][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[33][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[33][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[33][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[33][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[33][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][41] 	= ((((u16)tmp[33][0][1])<<8) + (u16)tmp[33][0][0]) + ((float)tmp[33][1][1])/100;
			group[13][42] 	= ((((u16)tmp[33][2][1])<<8) + (u16)tmp[33][2][0]) + ((float)tmp[33][3][1])/100;
				
			alarmOn[13][41]		=(u16)tmp[33][1][0]&1;
			alarmOn[13][42]		=(u16)tmp[33][3][0]&1;
		}
		if(0x30C==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[34][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[34][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
	
			tmp[34][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[34][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[34][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[34][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[34][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[34][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][43] 	= ((((u16)tmp[34][0][1])<<8) + (u16)tmp[34][0][0]) + ((float)tmp[34][1][1])/100;
			group[13][44] 	= ((((u16)tmp[34][2][1])<<8) + (u16)tmp[34][2][0]) + ((float)tmp[34][3][1])/100;
				
			alarmOn[13][43]		=(u16)tmp[34][1][0]&1;
			alarmOn[13][44]		=(u16)tmp[34][3][0]&1;
		}
		if(0x30D==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[35][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[35][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
	
			tmp[35][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[35][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[35][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[35][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[35][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[35][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][45] 	= ((((u16)tmp[35][0][1])<<8) + (u16)tmp[35][0][0]) + ((float)tmp[35][1][1])/100;
			group[13][46] 	= ((((u16)tmp[35][2][1])<<8) + (u16)tmp[35][2][0]) + ((float)tmp[35][3][1])/100;
				
			alarmOn[13][45]		=(u16)tmp[35][1][0]&1;
			alarmOn[13][46]		=(u16)tmp[35][3][0]&1;
		}
		if(0x30E==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[36][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[36][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
	
			tmp[36][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[36][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[36][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[36][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[36][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[36][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][47] 	= ((((u16)tmp[36][0][1])<<8) + (u16)tmp[36][0][0]) + ((float)tmp[36][1][1])/100;
			group[13][48] 	= ((((u16)tmp[36][2][1])<<8) + (u16)tmp[36][2][0]) + ((float)tmp[36][3][1])/100;
				
			alarmOn[13][47]		=(u16)tmp[36][1][0]&1;
			alarmOn[13][48]		=(u16)tmp[36][3][0]&1;
		}
		if(0x30F==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[37][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[37][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
	
			tmp[37][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[37][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[37][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[37][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[37][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[37][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][49] 	= ((((u16)tmp[37][0][1])<<8) + (u16)tmp[37][0][0]) + ((float)tmp[37][1][1])/100;
			group[13][50] 	= ((((u16)tmp[37][2][1])<<8) + (u16)tmp[37][2][0]) + ((float)tmp[37][3][1])/100;
				
			alarmOn[13][49]		=(u16)tmp[37][1][0]&1;
			alarmOn[13][50]		=(u16)tmp[37][3][0]&1;
		}
		if(0x310==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[38][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[38][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
	
			tmp[38][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[38][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[38][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[38][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[38][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[38][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][51] 	= ((((u16)tmp[38][0][1])<<8) + (u16)tmp[38][0][0]) + ((float)tmp[38][1][1])/100;
			group[13][52] 	= ((((u16)tmp[38][2][1])<<8) + (u16)tmp[38][2][0]) + ((float)tmp[38][3][1])/100;
				
			alarmOn[13][51]		=(u16)tmp[38][1][0]&1;
			alarmOn[13][52]		=(u16)tmp[38][3][0]&1;
		}
		if(0x311==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[39][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[39][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
	
			tmp[39][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[39][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[39][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[39][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[39][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[39][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][53] 	= ((((u16)tmp[39][0][1])<<8) + (u16)tmp[39][0][0]) + ((float)tmp[39][1][1])/100;
			group[13][54] 	= ((((u16)tmp[39][2][1])<<8) + (u16)tmp[39][2][0]) + ((float)tmp[39][3][1])/100;
				
			alarmOn[13][53]		=(u16)tmp[39][1][0]&1;
			alarmOn[13][54]		=(u16)tmp[39][3][0]&1;
		}
		if(0x312==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[40][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[40][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
	
			tmp[40][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[40][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[40][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[40][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[40][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[40][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[13][55] 	= ((((u16)tmp[40][0][1])<<8) + (u16)tmp[40][0][0]) + ((float)tmp[40][1][1])/100;
			group[14][13] 	= ((((u16)tmp[40][2][1])<<8) + (u16)tmp[40][2][0]) + ((float)tmp[40][3][1])/100;
			
			alarmOn[13][55]		=(u16)tmp[40][1][0]&1;
			alarmOn[14][13]		=(u16)tmp[40][3][0]&1;
		}
		
		if(0x313==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			tmpD313[0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmpD313[1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			tmpD313[2] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmpD313[3] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			tmpD313[4] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmpD313[5] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
			tmpD313[6] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmpD313[7] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			alarmOn[14][8]		= tmpD313[0]		& 1;
			alarmOn[13][0]		= tmpD313[0]>>2 & 1;
			alarmOn[13][1]		= tmpD313[0]>>4	& 1;
			alarmOn[13][2]		= tmpD313[0]>>6	& 1;
			alarmOn[13][3]		= tmpD313[1]		& 1;
			alarmOn[13][4]		= tmpD313[1]>>2 & 1;
			alarmOn[13][5]		= tmpD313[1]>>4	& 1;
			alarmOn[13][6]		= tmpD313[1]>>6	& 1;
			alarmOn[13][7]		= tmpD313[2]		& 1;
			alarmOn[13][8]		= tmpD313[2]>>2 & 1;
			
			alarmOn[13][9]		= tmpD313[3]>>4 & 1;
			alarmOn[13][10]		= tmpD313[3]>>6 & 1;
			alarmOn[13][11]		= tmpD313[4]		& 1;
			alarmOn[13][12]		= tmpD313[4]>>2 & 1;
			alarmOn[13][13]		= tmpD313[4]>>4	& 1;
			alarmOn[13][14]		= tmpD313[4]>>6	& 1;
			alarmOn[13][15]		= tmpD313[5]		& 1;
			alarmOn[13][16]		= tmpD313[5]>>2 & 1;
			alarmOn[13][17]		= tmpD313[5]>>4	& 1;
			alarmOn[13][18]		= tmpD313[5]>>6	& 1;
			
			alarmOn[14][9]		= tmpD313[7]>>2 & 1;
			alarmOn[14][10]		= tmpD313[7]>>4	& 1;
			alarmOn[14][11]		= tmpD313[7]>>6	& 1;
		
			
		}
		
		if(0x314==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			tmpD314[0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmpD314[1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			tmpD314[2] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmpD314[3] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			tmpD314[4] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmpD314[5] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
			tmpD314[6] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmpD314[7] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			alarmOn[14][12]		= tmpD314[0]		& 1;
			
			alarmOn[14][14]		= tmpD314[0]>>2 & 1;
			alarmOn[14][15]		= tmpD314[0]>>4 & 1;
			
			alarmOn[14][16]		= tmpD314[0]>>6	& 1;
			alarmOn[14][17]		= tmpD314[1]		& 1;
			alarmOn[14][18]		= tmpD314[1]>>2	& 1;
			alarmOn[14][19]		= tmpD314[1]>>4	& 1;
			alarmOn[14][20]		= tmpD314[1]>>6	& 1;
			alarmOn[14][21]		= tmpD314[2]		& 1;
			
			alarmOn[14][22]		= tmpD314[2]>>6	& 1;
			
			alarmOn[14][23]		= tmpD314[3]>>2	& 1;
			alarmOn[14][24]		= tmpD314[3]>>4	& 1;
			alarmOn[14][25]		= tmpD314[3]>>6	& 1;
			alarmOn[14][26]		= tmpD314[4]		& 1;
			alarmOn[14][27]		= tmpD314[4]>>2	& 1;
			alarmOn[14][28]		= tmpD314[4]>>4	& 1;
			alarmOn[14][29]		= tmpD314[4]>>6	& 1;
			alarmOn[14][30]		= tmpD314[5]		& 1;
			alarmOn[14][31]		= tmpD314[5]>>2	& 1;
			alarmOn[14][32]		= tmpD314[5]>>4	& 1;
			alarmOn[14][33]		= tmpD314[5]>>6	& 1;
			alarmOn[14][34]		= tmpD314[6]		& 1;
			alarmOn[14][35]		= tmpD314[6]>>2	& 1;
			alarmOn[14][36]		= tmpD314[6]>>4	& 1;
			alarmOn[14][37]		= tmpD314[6]>>6	& 1;
			alarmOn[14][38]		= tmpD314[7]		& 1;
			alarmOn[14][39]		= tmpD314[7]>>2	& 1;
			alarmOn[14][40]		= tmpD314[7]>>4	& 1;
		}
		
		if(0x400==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[41][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[41][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
	
			tmp[41][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[41][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[41][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[41][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[41][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[41][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[14][54] 	= ((((u16)tmp[41][0][1])<<8) + (u16)tmp[41][0][0]) + ((float)tmp[41][1][1])/100;
			group[14][55] 	= ((((u16)tmp[41][2][1])<<8) + (u16)tmp[41][2][0]) + ((float)tmp[41][3][1])/100;
				
			alarmOn[14][54]		=(u16)tmp[41][1][0]&1;
			alarmOn[14][55]		=(u16)tmp[41][3][0]&1;
		}
		if(0x401==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[42][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[42][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
	
			tmp[42][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[42][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[42][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[42][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[42][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[42][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[14][56] 	= ((((u16)tmp[42][0][1])<<8) + (u16)tmp[42][0][0]) + ((float)tmp[42][1][1])/100;
			group[14][57] 	= ((((u16)tmp[42][2][1])<<8) + (u16)tmp[42][2][0]) + ((float)tmp[42][3][1])/100;
				
			alarmOn[14][56]		=(u16)tmp[42][1][0]&1;
			alarmOn[14][57]		=(u16)tmp[42][3][0]&1;
		}
		if(0x402==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[43][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[43][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
	
			tmp[43][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[43][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[43][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[43][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[43][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[43][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[14][62] 	= ((((u16)tmp[43][0][1])<<8) + (u16)tmp[43][0][0]) + ((float)tmp[43][1][1])/100;
			group[14][63] 	= ((((u16)tmp[43][2][1])<<8) + (u16)tmp[43][2][0]) + ((float)tmp[43][3][1])/100;
				
			alarmOn[14][62]		=(u16)tmp[43][1][0]&1;
			alarmOn[14][63]		=(u16)tmp[43][3][0]&1;
		}
		if(0x403==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[44][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[44][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
	
			tmp[44][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[44][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[44][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[44][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[44][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[44][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[14][64] 	= ((((u16)tmp[44][0][1])<<8) + (u16)tmp[44][0][0]) + ((float)tmp[44][1][1])/100;
			group[14][65] 	= ((((u16)tmp[44][2][1])<<8) + (u16)tmp[44][2][0]) + ((float)tmp[44][3][1])/100;
				
			alarmOn[14][64]		=(u16)tmp[44][1][0]&1;
			alarmOn[14][65]		=(u16)tmp[44][3][0]&1;
		}
		if(0x404==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[45][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[45][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
	
			tmp[45][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[45][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[45][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[45][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[45][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[45][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
		 
			group[14][66] 	= ((((u16)tmp[45][0][1])<<8) + (u16)tmp[45][0][0]) + ((float)tmp[45][1][1])/100;
			group[14][67] 	= ((((u16)tmp[45][2][1])<<8) + (u16)tmp[45][2][0]) + ((float)tmp[45][3][1])/100;
				
			alarmOn[14][66]		=(u16)tmp[45][1][0]&1;
			alarmOn[14][67]		=(u16)tmp[45][3][0]&1;
		}
		
		if(0x408==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			tmpD408[0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmpD408[1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			tmpD408[2] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmpD408[3] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			tmpD408[4] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmpD408[5] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
			tmpD408[6] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmpD408[7] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			alarmOn[14][41]		= tmpD408[0]		& 1;
			alarmOn[14][42]		= tmpD408[0]>>2 & 1;
			alarmOn[14][43]		= tmpD408[0]>>4 & 1;
			alarmOn[14][44]		= tmpD408[0]>>6	& 1;
			alarmOn[14][45]		= tmpD408[1]		& 1;
			alarmOn[14][46]		= tmpD408[1]>>2 & 1;
			alarmOn[14][47]		= tmpD408[1]>>4 & 1;
			
			alarmOn[14][48]		= tmpD408[2]>>2 & 1;
			alarmOn[14][49]		= tmpD408[2]>>4 & 1;
			alarmOn[14][50]		= tmpD408[2]>>6	& 1;
			alarmOn[14][51]		= tmpD408[3]		& 1;
			alarmOn[14][52]		= tmpD408[3]>>2 & 1;
			alarmOn[14][53]		= tmpD408[3]>>4 & 1;
			
			alarmOn[14][58]		= tmpD408[4]>>2 & 1;
			alarmOn[14][59]		= tmpD408[4]>>4 & 1;
			alarmOn[14][60]		= tmpD408[4]>>6	& 1;
			alarmOn[14][61]		= tmpD408[5]		& 1;
			//alarmOn[14][62]		= tmpD408[5]>>2 & 1;
			//alarmOn[14][63]		= tmpD408[5]>>4 & 1;
			
			alarmOn[10][10]		= tmpD408[5]>>2 & 1;
			alarmOn[10][11]		= tmpD408[5]>>4 & 1;
			alarmOn[10][12]		= tmpD408[5]>>6	& 1;
			alarmOn[10][13]		= tmpD408[6]		& 1;
			alarmOn[10][14]		= tmpD408[6]>>2 & 1;
			alarmOn[10][15]		= tmpD408[6]>>4 & 1;
			alarmOn[10][16]		= tmpD408[6]>>6	& 1;
		}
		
		
///////////////////////////////////////////alarimlari qebul edenler///////////////////////////////////////////////////////////////////////////////////

		/*
		
		if(0x101==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[1][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[1][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[1][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[1][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[1][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[1][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[1][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[1][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
		}
		
		if(0x101==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[1][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[1][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[1][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[1][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[1][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[1][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[1][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[1][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
		}
		
		if(0x101==CanData.BusRXbuf[CanData.CanRxTail].ID)
		{
			
			tmp[1][0][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmp[1][0][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			
			tmp[1][1][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmp[1][1][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			
			tmp[1][2][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmp[1][2][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
		
			tmp[1][3][0] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmp[1][3][1] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
		}
		*/
		/*
			tmpD20A[0] = CanData.BusRXbuf[CanData.CanRxTail].candata[0];
			tmpD20A[1] = CanData.BusRXbuf[CanData.CanRxTail].candata[1];
			tmpD20A[2] = CanData.BusRXbuf[CanData.CanRxTail].candata[2];
			tmpD20A[3] = CanData.BusRXbuf[CanData.CanRxTail].candata[3];
			tmpD20A[4] = CanData.BusRXbuf[CanData.CanRxTail].candata[4];
			tmpD20A[5] = CanData.BusRXbuf[CanData.CanRxTail].candata[5];
			tmpD20A[6] = CanData.BusRXbuf[CanData.CanRxTail].candata[6];
			tmpD20A[7] = CanData.BusRXbuf[CanData.CanRxTail].candata[7];
			
			group10[20]	= tmpD20A[1]>>7	& 1;
			group10[21]	= tmpD20A[0]  	& 1;
			group10[22]	= tmpD20A[0]>>1	& 1;
			group10[23]	= tmpD20A[0]>>2	& 1;
			group10[24]	= tmpD20A[0]>>3	& 1;
			group10[25]	= tmpD20A[0]>>4	& 1;
		*/
		
		CanData.CanRxTail++;
	}
}


void Can_Isr() interrupt 9
{
	u8 status;

	EA = 0;
	if((CAN_IR&0x80) == 0x80)
	{
		CAN_IR &= 0x3F;	//清空远程帧标记位			
	}
	if((CAN_IR&0x40) == 0x40)
	{
		CAN_IR &= 0xBF;	//清空数据帧标记位
		ADR_H = 0xFF;
		ADR_M = 0x00;
		ADR_L = 0x68;
		ADR_INC = 1;
		RAMMODE = 0xAF;		//读操作
		while(!APP_ACK);
		APP_EN = 1;
		while(APP_EN);
		status = DATA3;
		CanData.BusRXbuf[CanData.CanRxHead].status = status;
		APP_EN = 1;
		while(APP_EN);			
		CanData.BusRXbuf[CanData.CanRxHead].ID <<= 8;
		CanData.BusRXbuf[CanData.CanRxHead].ID |= DATA3;
		CanData.BusRXbuf[CanData.CanRxHead].ID <<= 8;
		CanData.BusRXbuf[CanData.CanRxHead].ID |= DATA2;
		CanData.BusRXbuf[CanData.CanRxHead].ID <<= 8;
		CanData.BusRXbuf[CanData.CanRxHead].ID |= DATA1;
		CanData.BusRXbuf[CanData.CanRxHead].ID <<= 8;
		CanData.BusRXbuf[CanData.CanRxHead].ID |= DATA0;
		CanData.BusRXbuf[CanData.CanRxHead].ID=CanData.BusRXbuf[CanData.CanRxHead].ID>>3;
		if(0==(status&0x80))//标准帧ID还需要右移18位
		{
			CanData.BusRXbuf[CanData.CanRxHead].ID >>= 18;
		}
		if(0==(status&0x40))//数据帧才需要读取数据
		{
			APP_EN = 1;
			while(APP_EN);
			CanData.BusRXbuf[CanData.CanRxHead].candata[0] = DATA3;
			CanData.BusRXbuf[CanData.CanRxHead].candata[1] = DATA2;
			CanData.BusRXbuf[CanData.CanRxHead].candata[2] = DATA1;
			CanData.BusRXbuf[CanData.CanRxHead].candata[3] = DATA0;
			APP_EN = 1;
			while(APP_EN);
			CanData.BusRXbuf[CanData.CanRxHead].candata[4] = DATA3;
			CanData.BusRXbuf[CanData.CanRxHead].candata[5] = DATA2;
			CanData.BusRXbuf[CanData.CanRxHead].candata[6] = DATA1;
			CanData.BusRXbuf[CanData.CanRxHead].candata[7] = DATA0;	
		}
		RAMMODE = 0;
		CanData.CanRxHead++;
	}
	if((CAN_IR&0x20) == 0x20)
	{
		CAN_IR &= ~(0x20);	//清空发送帧标记位
		if(CanData.CanTxTail != CanData.CanTxHead)
		{
			LoadOneFrame();
			CAN_CR |= 0x04;		//启动发送		
			StartTimer(7,3000);//3S还未发送出去，则清空发送标记
		}
		else
		{
			CanData.CanTxFlag = 0;//清空发送标记位
		}
	}
	if((CAN_IR&0x10) == 0x10)
	{
		CAN_IR &= 0xEF;	//清空接收溢出标记位
	}
	if((CAN_IR&0x08) == 0x08)
	{
		CAN_IR &= 0xF7;	//清空错误标记位
	}
	if((CAN_IR&0x04) == 0x04)
	{
		CAN_IR &= 0xFB;	//清空仲裁失败标记位
		CAN_CR |= 0x04;	//重新启动发送	
	}
	CAN_ET=0;
	EA = 1;  
}


/*主循环调用，将需要发送的数据放在缓存区即可，同时CAN发送会占用定时器7，其余位置则不能在使用*/
// void CANTx(void)
// {
// 	u32 tmp32;
// 	if((0==CanData.CanTxFlag)&&(CanData.CanTxTail != CanData.CanTxFlag))
// 	{
// 		EA = 0;
// 		tmp32=CanData.BusTXbuf[CanData.CanTxTail].ID;
// 		ADR_H = 0xFF;
// 		ADR_M = 0x00;
// 		ADR_L = 0x64;
// 		ADR_INC = 1;
// 		RAMMODE = 0x8F;		//写操作
// 		while(!APP_ACK);
// 		DATA3 = CanData.BusTXbuf[CanData.CanTxTail].status;			//帧类长度型以及数据
// 		DATA2 = 0;
// 		DATA1 = 0;
// 		DATA0 = 0;	 		
// 		APP_EN = 1;
// 		while(APP_EN);		//写入RTR,IDE,DLC等数据
// 		DATA3 = (u8)(tmp32>>24);
// 		DATA2 = (u8)(tmp32>>16);
// 		DATA1 = (u8)(tmp32>>8);
// 		DATA0 = (u8)(tmp32>>0);	 		
// 		APP_EN = 1;
// 		while(APP_EN);		//写入ID数据
// 		DATA3 = CanData.BusTXbuf[CanData.CanTxTail].candata[0];
// 		DATA2 = CanData.BusTXbuf[CanData.CanTxTail].candata[1];
// 		DATA1 = CanData.BusTXbuf[CanData.CanTxTail].candata[2];
// 		DATA0 = CanData.BusTXbuf[CanData.CanTxTail].candata[3];	 		
// 		APP_EN = 1;
// 		while(APP_EN);		//写入发送数据前4字节
// 		DATA3 = CanData.BusTXbuf[CanData.CanTxTail].candata[4];
// 		DATA2 = CanData.BusTXbuf[CanData.CanTxTail].candata[5];
// 		DATA1 = CanData.BusTXbuf[CanData.CanTxTail].candata[6];
// 		DATA0 = CanData.BusTXbuf[CanData.CanTxTail].candata[7];	 		
// 		APP_EN = 1;
// 		while(APP_EN);		//写入发送数据后4字节
// 		CanData.CanTxTail++;
// 		CanData.CanTxFlag = 1;
// 		StartTimer(7,3000);//3S还未发送出去，则清空发送标记
// 		EA = 1;
// 		RAMMODE = 0;
// 		CAN_CR |= 0x04;		//启动发送		
// 	}
// 	if(CanData.CanTxFlag!=0)
// 	{
// 		if(GetTimeOutFlag(7))
// 		{
// 			CanData.CanTxFlag = 0;
// 		}
// 	}
// }