#ifndef __RTC_RX8130_H__
#define __RTC_RX8130_H__
#include "sys.h"

void init_rtc();
void rdtime();
void rtc_set(unsigned char*buf);
unsigned char week_calculate(unsigned char year,unsigned char month,unsigned char day);
#endif

