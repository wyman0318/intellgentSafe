#ifndef __BLE_H_
#define __BLE_H_
#include "sys.h"
#include <stdio.h>
#include "stm32f4xx.h"
#include "delay.h"
#include "includes.h"

extern void  usart3_init(uint32_t baud);
extern OS_Q g_queue;
extern void usart3_send_str(uint8_t *pbuf);
#endif
