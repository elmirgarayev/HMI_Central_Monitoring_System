#include "ui.h"

void P0Func(void)
{
	
}

code void (*PagePointer[])(u16 pageid)={
P0Func,
};

void PageFunction(void)
{
	u16 pageid;

	EA = 0;
	ADR_H = 0x00;
	ADR_M = 0x00;
	ADR_L = 0x0a;
	ADR_INC = 1;
	RAMMODE = 0xAF;		//������
	while(!APP_ACK);
	APP_EN = 1;
	while(APP_EN);
	pageid = DATA3;
	pageid <<= 8;
	pageid |= DATA2;
	RAMMODE = 0;
	EA = 1;
	if(PagePointer[pageid] != 0)
	{
		PagePointer[pageid](pageid);
	}	
}