#include "flash.h"




void flash_init(void)
{
	FLASH_Unlock();
	
	//擦除第五个扇区，供电位3.3v，使用等级3，每次需要写入32位数据大小
	if(FLASH_EraseSector(FLASH_Sector_5,VoltageRange_3)!=FLASH_COMPLETE)
	{
		printf("error");
		while(1);
	}

	
	FLASH_Lock();
}

void info_init(void)
{
	FLASH_Unlock();
	if(FLASH_EraseSector(FLASH_Sector_6,VoltageRange_3)!=FLASH_COMPLETE)
	{
		printf("error");
		while(1);
	}
	FLASH_Lock();
}

void write_flash(uint8_t *buf,uint32_t count)
{
	uint32_t j=0,i=0;
	
	//解锁闪存，允许访问，才能擦除和写入数据
	FLASH_Unlock();
	
	for(j=0;j<16;j++)
	{
		if(FLASH_ProgramWord(ADDR_FLASH_SECTOR_5+64*count+j*4,*(uint32_t *)&buf[i])!=FLASH_COMPLETE)
		{
			printf("error");
			while(1);
		}
		i+=4;
	}
	
	FLASH_Lock();
}

void write_info(uint8_t *buf,uint32_t count)
{
		uint32_t j=0,i=0;
	
	//解锁闪存，允许访问，才能擦除和写入数据
	FLASH_Unlock();
	
	for(j=0;j<5;j++)
	{
		if(FLASH_ProgramWord(ADDR_FLASH_SECTOR_6+20*count+j*4,*(uint32_t *)&buf[i])!=FLASH_COMPLETE)
		{
			printf("error");
			while(1);
		}
		i+=4;
	}
	
	FLASH_Lock();
}

void get_flash_data(uint8_t *buf,uint32_t count)
{
	int32_t j,i=0;
	
	for(j=0;j<16;j++)
	{
		*(uint32_t *)&buf[i]=*(volatile uint32_t *)(ADDR_FLASH_SECTOR_5+count*64+4*j);
		i+=4;
	}
}

void get_info_data(uint8_t *buf,uint32_t count)
{
	int32_t j,i=0;
	
	for(j=0;j<5;j++)
	{
		*(uint32_t *)&buf[i]=*(volatile uint32_t *)(ADDR_FLASH_SECTOR_5+count*20+4*j);
		i+=4;
	}
}

void check_flash(uint8_t *buf,uint8_t *pw,uint32_t *idNum,uint32_t *recNum)
{
	uint32_t j=0,i=0;
	//解锁闪存，允许访问，才能擦除和写入数据
	FLASH_Unlock();
	
	get_info_data(buf,9);
	if(!strstr((const char*)buf,"\n"))
	{
		flash_init();
		info_init();
		memset(buf,0,64);
		sprintf((char*)buf,"%c%c%c%c\n",'0','0','0','0');
		write_info(buf,9);
		
		memcpy(pw,buf,4);
		return;
	}
	else
	{
		memcpy(pw,buf,4);
	}
	
	for(i=0;i<9;i++)
	{
		get_info_data(buf,i);
		if(!strstr((const char*)buf,"\n"))
			break;
		memcpy((char*)cardId[i],buf,4);
	}
	
	*idNum=i;	//登记的卡数量
	
	for(i=10;i<110;i++)
	{
		get_flash_data(buf,i);
		if(!strstr((const char*)buf,"\n"))
			break;
	}
	
	*recNum=i;	//记录的数量
	

}

void checkCard(uint8_t *buf)
{
	uint8_t i=0,j;
	for(i=0;i<cardNum;i++)
	{
		for(j=0;j<4;j++)
		{
			if(cardId[i][j]!=buf[j])
				break;
			if(j==3)
			{
				buf[4]=1;
				return;
			}
		}
	}
	buf[4]=0;
}

