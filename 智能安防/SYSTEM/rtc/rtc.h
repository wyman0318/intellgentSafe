#ifndef __RTC_H
#define __RTC_H	 
#include "stm32f4xx.h" 
#include "includes.h"

extern OS_FLAG_GRP myFlag;
extern void rtc_cofig(void);
extern void rtc_init(int RstFlag);
extern uint8_t DectoBCD(int32_t dec);
extern uint8_t changeTime(uint8_t *p);
#endif







