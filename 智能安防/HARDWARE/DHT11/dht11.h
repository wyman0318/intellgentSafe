#ifndef __DHT11_H_
#define __DHT11_H_
#include "sys.h"
#include <stdio.h>
#include "stm32f4xx.h"
#include "delay.h"	

extern void  dht11_init(void);
extern int32_t dht11_read_data(uint8_t *dht11_data);

#endif
