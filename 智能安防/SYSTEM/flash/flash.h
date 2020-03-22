#ifndef __FLASH_H
#define __FLASH_H

#include "command.h"

extern void flash_init(void);
extern void write_flash(uint8_t *buf,uint32_t count);
extern void get_flash_data(uint8_t *buf,uint32_t count);
extern void check_flash(uint8_t *buf,uint8_t *pw,uint32_t *idNum,uint32_t *recNum);
extern void checkCard(uint8_t *buf);
#endif


