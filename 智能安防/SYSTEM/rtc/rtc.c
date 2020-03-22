#include "rtc.h"  

static RTC_DateTypeDef RTC_DateStructure;
static RTC_TimeTypeDef  RTC_TimeStructure;
static RTC_InitTypeDef  RTC_InitStructure;
static EXTI_InitTypeDef  EXTI_InitStructure;
static NVIC_InitTypeDef		NVIC_InitStructure;

void rtc_init(int RstFlag)
{
	//使能rtc硬件时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	
	//使能允许访问RTC
	PWR_BackupAccessCmd(ENABLE);
	
	//使能RTC外部震荡时钟，32.768KHz
	RCC_LSEConfig(RCC_LSE_ON);
	
	//等待外部时钟生效
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY)==RESET);
	
	//选择外部震荡时钟LSE作为RTC时钟源
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	
	//使能RTC硬件时钟
	RCC_RTCCLKCmd(ENABLE);
	
	//等待RTC相关寄存器准备完成
	RTC_WaitForSynchro();
	
	//配置分频值，RTC的硬件时钟频率=32769/（127+1）/（255+1）=1Hz
	RTC_InitStructure.RTC_AsynchPrediv=127;//异步分配，第一次分频
	RTC_InitStructure.RTC_SynchPrediv=255;//同步分频，也称为第二次分频
	RTC_InitStructure.RTC_HourFormat=RTC_HourFormat_24;//24小时格式
	RTC_Init(&RTC_InitStructure);
	
	//配置RTC计数值，配置1秒触发一次中断
	//关闭唤醒功能
	RTC_WakeUpCmd(DISABLE);
	
	//为唤醒功能选择RTC配置好的时钟源
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
	
	//设置唤醒计数值为自动重载，写入值默认为0
	RTC_SetWakeUpCounter(1-1);//表示1秒，但从0开始所以-1
	
	//清除RTC唤醒中断标志
	RTC_ClearITPendingBit(RTC_IT_WUT);
	
	//使能RTC唤醒中断
	RTC_ITConfig(RTC_IT_WUT,ENABLE);
	
	//使能唤醒功能
	RTC_WakeUpCmd(ENABLE);
	
		//配置RTC中断和中断优先级
	EXTI_ClearITPendingBit(EXTI_Line22);
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	if(!RstFlag)
	{
		//配置RTC的日期，年月日 星期几
		//2020/1/23 星期四
		RTC_DateStructure.RTC_Year = 0x20;
		RTC_DateStructure.RTC_Month = RTC_Month_January;
		RTC_DateStructure.RTC_Date = 0x23;
		RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Thursday;
		RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
		
		//配置RTC的时间，时分秒
		//下午17：34：55
		RTC_TimeStructure.RTC_H12     = RTC_H12_PM;
		RTC_TimeStructure.RTC_Hours   = 0x17;
		RTC_TimeStructure.RTC_Minutes = 0x34;
		RTC_TimeStructure.RTC_Seconds = 0x55; 
		RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure); 
		
		//建立标志，告诉已经重置过日期和时间了
		RTC_WriteBackupRegister(RTC_BKP_DR0,0x0318);
	}
}

void rtc_cofig(void)
{
	//读取复位标志，判断RTC是否被重置
	if(RTC_ReadBackupRegister(RTC_BKP_DR0)!=0x0318)
	{
		rtc_init(0);
	}
	else//意味着被重置过
	{
		rtc_init(1);
	}
}

void RTC_WKUP_IRQHandler(void)
{
	uint8_t flag=0;
	OS_ERR err;
	
	OSIntEnter(); 
	if(RTC_GetITStatus(RTC_IT_WUT) != RESET)
	{
		//设置个标志位获取时间
		flag=1;
		RTC_ClearITPendingBit(RTC_IT_WUT);
		EXTI_ClearITPendingBit(EXTI_Line22);
	}
	OSIntExit();	
	
	if(flag)
	{
		OSFlagPost(&myFlag,0x01,OS_OPT_POST_FLAG_SET,&err);
	}
}

uint8_t DectoBCD(int32_t dec)
{
	if(dec>2000) dec -= 2000;
	return (((dec%10)&0x0F) | (((dec/10)<<4)&0xF0));
}

uint8_t changeTime(uint8_t *p)
{
	int32_t i=0;
	uint8_t info[4]={0};
	
	if(strstr((const char*)p,"date"))
	{
		p=strtok((char *)p,"-");
		for(i=0;i<3;i++)
		{
			info[i]=DectoBCD(atoi(strtok(NULL,"-")));
		}
		RTC_DateStructure.RTC_Year = info[0];
		RTC_DateStructure.RTC_Month = info[1];
		RTC_DateStructure.RTC_Date = info[2];
		RTC_SetDate(RTC_Format_BCD,&RTC_DateStructure);
		
		return 0;
	}
	else if(strstr((const char*)p,"time"))
	{
		p=strtok((char *)p,"-");
		for(i=0;i<3;i++)
		{
			info[i]=DectoBCD(atoi(strtok(NULL,"-")));
		}
		
		RTC_TimeStructure.RTC_H12     = RTC_H12_PM;
		RTC_TimeStructure.RTC_Hours   = info[0];
		RTC_TimeStructure.RTC_Minutes = info[1];
		RTC_TimeStructure.RTC_Seconds = info[2]; 
		
		RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);
		return 0;
	}
	return 1;
}













