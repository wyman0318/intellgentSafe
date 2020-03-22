#ifndef __KEYBOARD_H_
#define __KEYBOARD_H_
#include "sys.h"
#include <stdio.h>
#include "stm32f4xx.h"
#include "delay.h"	

extern void key_init(void);
extern char get_key(void);

#endif
