#ifndef __BEEP_H_
#define __BEEP_H_
#include "sys.h"
#include <stdio.h>
#include "stm32f4xx.h"
#include "delay.h"	

extern void tim4_set_duty(uint32_t duty);
extern void tim4_set_freq(uint32_t freq);
extern void beep_init(void);

#endif
