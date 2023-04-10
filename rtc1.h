#ifndef  __RTC1__H___
#define  __RTC1__H___
#include "sys.h"
#include "t5los8051.h"
void rtc_init(void);
void rdtime(void);
void check_rtc_set(void);
void Rtc_set_time(u8* date);
#endif