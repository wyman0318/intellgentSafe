#include "rtc.h"  

static RTC_DateTypeDef RTC_DateStructure;
static RTC_TimeTypeDef  RTC_TimeStructure;
static RTC_InitTypeDef  RTC_InitStructure;
static EXTI_InitTypeDef  EXTI_InitStructure;
static NVIC_InitTypeDef		NVIC_InitStructure;

void rtc_init(int RstFlag)
{
	//ʹ��rtcӲ��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	
	//ʹ���������RTC
	PWR_BackupAccessCmd(ENABLE);
	
	//ʹ��RTC�ⲿ��ʱ�ӣ�32.768KHz
	RCC_LSEConfig(RCC_LSE_ON);
	
	//�ȴ��ⲿʱ����Ч
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY)==RESET);
	
	//ѡ���ⲿ��ʱ��LSE��ΪRTCʱ��Դ
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	
	//ʹ��RTCӲ��ʱ��
	RCC_RTCCLKCmd(ENABLE);
	
	//�ȴ�RTC��ؼĴ���׼�����
	RTC_WaitForSynchro();
	
	//���÷�Ƶֵ��RTC��Ӳ��ʱ��Ƶ��=32769/��127+1��/��255+1��=1Hz
	RTC_InitStructure.RTC_AsynchPrediv=127;//�첽���䣬��һ�η�Ƶ
	RTC_InitStructure.RTC_SynchPrediv=255;//ͬ����Ƶ��Ҳ��Ϊ�ڶ��η�Ƶ
	RTC_InitStructure.RTC_HourFormat=RTC_HourFormat_24;//24Сʱ��ʽ
	RTC_Init(&RTC_InitStructure);
	
	//����RTC����ֵ������1�봥��һ���ж�
	//�رջ��ѹ���
	RTC_WakeUpCmd(DISABLE);
	
	//Ϊ���ѹ���ѡ��RTC���úõ�ʱ��Դ
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
	
	//���û��Ѽ���ֵΪ�Զ����أ�д��ֵĬ��Ϊ0
	RTC_SetWakeUpCounter(1-1);//��ʾ1�룬����0��ʼ����-1
	
	//���RTC�����жϱ�־
	RTC_ClearITPendingBit(RTC_IT_WUT);
	
	//ʹ��RTC�����ж�
	RTC_ITConfig(RTC_IT_WUT,ENABLE);
	
	//ʹ�ܻ��ѹ���
	RTC_WakeUpCmd(ENABLE);
	
		//����RTC�жϺ��ж����ȼ�
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
		//����RTC�����ڣ������� ���ڼ�
		//2020/1/23 ������
		RTC_DateStructure.RTC_Year = 0x20;
		RTC_DateStructure.RTC_Month = RTC_Month_January;
		RTC_DateStructure.RTC_Date = 0x23;
		RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Thursday;
		RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
		
		//����RTC��ʱ�䣬ʱ����
		//����17��34��55
		RTC_TimeStructure.RTC_H12     = RTC_H12_PM;
		RTC_TimeStructure.RTC_Hours   = 0x17;
		RTC_TimeStructure.RTC_Minutes = 0x34;
		RTC_TimeStructure.RTC_Seconds = 0x55; 
		RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure); 
		
		//������־�������Ѿ����ù����ں�ʱ����
		RTC_WriteBackupRegister(RTC_BKP_DR0,0x0318);
	}
}

void rtc_cofig(void)
{
	//��ȡ��λ��־���ж�RTC�Ƿ�����
	if(RTC_ReadBackupRegister(RTC_BKP_DR0)!=0x0318)
	{
		rtc_init(0);
	}
	else//��ζ�ű����ù�
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
		//���ø���־λ��ȡʱ��
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













