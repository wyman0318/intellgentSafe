#include "command.h"

static RTC_DateTypeDef RTC_DateStructure;
static RTC_TimeTypeDef  RTC_TimeStructure;
//�û�������
static u8  mfrc552pidbuf[18];
static u8  card_pydebuf[2];
u8  card_numberbuf[5];
static u8  card_key0Abuf[6]={0xff,0xff,0xff,0xff,0xff,0xff};
static u8  card_writebuf[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
static u8  card_readbuf[18];
//flash
static volatile uint8_t  password[5]={0}; 
volatile uint8_t  cardId[10][5]={0}; 
volatile uint32_t cardNum=0;

//����1���ƿ� �޸�ʱ��
OS_TCB Task1_TCB;
void task1(void *parg);
CPU_STK task1_stk[256];			//����1�������ջ����СΪ128�֣�Ҳ����512�ֽ�

//����2���ƿ� ��ʾʱ��
OS_TCB Task2_TCB;
void task2(void *parg);
CPU_STK task2_stk[128];			//����2�������ջ����СΪ128�֣�Ҳ����512�ֽ�

//����3���ƿ� ��ʾ��ʪ��
OS_TCB Task3_TCB;
void task3(void *parg);
CPU_STK task3_stk[256];			//����2�������ջ����СΪ256�֣�Ҳ����1024�ֽ�

//����4���ƿ� RFID
OS_TCB Task4_TCB;
void task4(void *parg);
CPU_STK task4_stk[256];

//����5���ƿ� �������
OS_TCB Task5_TCB;
void task5(void *parg);
CPU_STK task5_stk[128];

//����6���ƿ� �洢��¼
OS_TCB Task6_TCB;
void task6(void *parg);
CPU_STK task6_stk[256];

//����7���ƿ� ��ӿ���
OS_TCB Task7_TCB;
void task7(void *parg);
CPU_STK task7_stk[128];

//��Ϣ����
OS_Q g_queue;
OS_Q card_queue;
OS_Q id_queue;

//��־��
OS_FLAG_GRP myFlag;

//������
OS_MUTEX print_mutex;
OS_MUTEX ble_mutex;
OS_MUTEX flash_mutex;

//������
int main(void)
{
	OS_ERR err;

	systick_init();  													//ʱ�ӳ�ʼ��
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);						//�жϷ�������
	
	usart_init(115200);  				 									//���ڳ�ʼ��
	usart3_init(9600);
	
	rtc_cofig();
	
	LED_Init();         												//LED��ʼ��	


	//OS��ʼ�������ǵ�һ�����еĺ���,��ʼ�����ֵ�ȫ�ֱ����������ж�Ƕ�׼����������ȼ����洢��
	OSInit(&err);


	//��������1
	OSTaskCreate(	(OS_TCB *)&Task1_TCB,		//������ƿ飬��ͬ���߳�id
					(CPU_CHAR *)"Task1",		//��������֣����ֿ����Զ����
					(OS_TASK_PTR)task1,			//����������ͬ���̺߳���
					(void *)0,					//���ݲ�������ͬ���̵߳Ĵ��ݲ���
					(OS_PRIO)6,				 	//��������ȼ�6		
					(CPU_STK *)task1_stk,		//�����ջ����ַ
					(CPU_STK_SIZE)256/10,		//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					(CPU_STK_SIZE)256,			//�����ջ��С			
					(OS_MSG_QTY)0,				//��ֹ������Ϣ����
					(OS_TICK)0,					//Ĭ��ʱ��Ƭ����																
					(void  *)0,					//����Ҫ�����û��洢��
					(OS_OPT)OS_OPT_TASK_NONE,	//û���κ�ѡ��
					&err						//���صĴ�����
				);


	//��������2
	OSTaskCreate(	(OS_TCB *)&Task2_TCB,		//������ƿ�
					(CPU_CHAR *)"Task2",		//���������
					(OS_TASK_PTR)task2,			//������
					(void *)0,					//���ݲ���
					(OS_PRIO)6,				 	//��������ȼ�7		
					(CPU_STK *)task2_stk,		//�����ջ����ַ
					(CPU_STK_SIZE)128/10,		//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					(CPU_STK_SIZE)128,			//�����ջ��С			
					(OS_MSG_QTY)0,				//��ֹ������Ϣ����
					(OS_TICK)0,					//Ĭ��ʱ��Ƭ����																
					(void  *)0,					//����Ҫ�����û��洢��
					(OS_OPT)OS_OPT_TASK_NONE,	//û���κ�ѡ��
					&err						//���صĴ�����
				);
				
	//��������3
	OSTaskCreate(	(OS_TCB *)&Task3_TCB,		//������ƿ�
					(CPU_CHAR *)"Task3",		//���������
					(OS_TASK_PTR)task3,			//������
					(void *)0,					//���ݲ���
					(OS_PRIO)6,				 	//��������ȼ�7		
					(CPU_STK *)task3_stk,		//�����ջ����ַ
					(CPU_STK_SIZE)256/10,		//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					(CPU_STK_SIZE)256,			//�����ջ��С			
					(OS_MSG_QTY)0,				//��ֹ������Ϣ����
					(OS_TICK)0,					//Ĭ��ʱ��Ƭ����																
					(void  *)0,					//����Ҫ�����û��洢��
					(OS_OPT)OS_OPT_TASK_NONE,	//û���κ�ѡ��
					&err						//���صĴ�����
				);
				
	//��������4
	OSTaskCreate(	(OS_TCB *)&Task4_TCB,		//������ƿ�
					(CPU_CHAR *)"Task4",		//���������
					(OS_TASK_PTR)task4,			//������
					(void *)0,					//���ݲ���
					(OS_PRIO)6,				 	//��������ȼ�7		
					(CPU_STK *)task4_stk,		//�����ջ����ַ
					(CPU_STK_SIZE)256/10,		//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					(CPU_STK_SIZE)256,			//�����ջ��С			
					(OS_MSG_QTY)0,				//��ֹ������Ϣ����
					(OS_TICK)0,					//Ĭ��ʱ��Ƭ����																
					(void  *)0,					//����Ҫ�����û��洢��
					(OS_OPT)OS_OPT_TASK_NONE,	//û���κ�ѡ��
					&err						//���صĴ�����
				);
				
	//��������5
	OSTaskCreate(	(OS_TCB *)&Task5_TCB,		//������ƿ�
					(CPU_CHAR *)"Task5",		//���������
					(OS_TASK_PTR)task5,			//������
					(void *)0,					//���ݲ���
					(OS_PRIO)6,				 	//��������ȼ�7		
					(CPU_STK *)task4_stk,		//�����ջ����ַ
					(CPU_STK_SIZE)128/10,		//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					(CPU_STK_SIZE)128,			//�����ջ��С			
					(OS_MSG_QTY)0,				//��ֹ������Ϣ����
					(OS_TICK)0,					//Ĭ��ʱ��Ƭ����																
					(void  *)0,					//����Ҫ�����û��洢��
					(OS_OPT)OS_OPT_TASK_NONE,	//û���κ�ѡ��
					&err						//���صĴ�����
				);
				
	//��������6
	OSTaskCreate(	(OS_TCB *)&Task6_TCB,		//������ƿ�
					(CPU_CHAR *)"Task6",		//���������
					(OS_TASK_PTR)task6,			//������
					(void *)0,					//���ݲ���
					(OS_PRIO)6,				 	//��������ȼ�7		
					(CPU_STK *)task6_stk,		//�����ջ����ַ
					(CPU_STK_SIZE)256/10,		//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					(CPU_STK_SIZE)256,			//�����ջ��С			
					(OS_MSG_QTY)0,				//��ֹ������Ϣ����
					(OS_TICK)0,					//Ĭ��ʱ��Ƭ����																
					(void  *)0,					//����Ҫ�����û��洢��
					(OS_OPT)OS_OPT_TASK_NONE,	//û���κ�ѡ��
					&err						//���صĴ�����
				);
				
	//��������7
	OSTaskCreate(	(OS_TCB *)&Task7_TCB,		//������ƿ�
					(CPU_CHAR *)"Task7",		//���������
					(OS_TASK_PTR)task7,			//������
					(void *)0,					//���ݲ���
					(OS_PRIO)6,				 	//��������ȼ�7		
					(CPU_STK *)task7_stk,		//�����ջ����ַ
					(CPU_STK_SIZE)128/10,		//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					(CPU_STK_SIZE)128,			//�����ջ��С			
					(OS_MSG_QTY)0,				//��ֹ������Ϣ����
					(OS_TICK)0,					//Ĭ��ʱ��Ƭ����																
					(void  *)0,					//����Ҫ�����û��洢��
					(OS_OPT)OS_OPT_TASK_NONE,	//û���κ�ѡ��
					&err						//���صĴ�����
				);

	//������Ϣ����,32����Ϣ����
	OSQCreate(&g_queue,"g_queue",32,&err);
	OSQCreate(&card_queue,"card_queue",32,&err);
	OSQCreate(&id_queue,"id_queue",32,&err);
	
	//����32λ��־��
	OSFlagCreate(&myFlag,"myFlag",0,&err);
	
	//����������
	OSMutexCreate(&print_mutex,"print_mutex",&err);
	OSMutexCreate(&ble_mutex,"ble_mutex",&err);
	OSMutexCreate(&flash_mutex,"beep_mutex",&err);

	//����OS�������������
	OSStart(&err);
					
					
	printf("never run.......\r\n");
					
	while(1);
	
}


void task1(void *parg)
{	
	OS_ERR err;

	OS_MSG_SIZE msg_size;
	
	uint8_t *p=NULL;
	uint8_t buf[64]={0};
	uint32_t i=0;
	
	printf("task1 is create ok\r\n");

	while(1)
	{
		//�ȴ���Ϣ���У�0�������ȴ�
		p=OSQPend(&g_queue,0,OS_OPT_PEND_BLOCKING,&msg_size,NULL,&err);
		
		//���ָ������Ч����Ч�����ݳ���
		if(p&&msg_size)
		{
			//��ʱ��
			if(strstr(p,"time"))
				changeTime(p);
			
			//ȡ��¼
			OSMutexPend(&ble_mutex,0,OS_OPT_PEND_BLOCKING,NULL,&err);
			if(strstr(p,"show"))
			{
				for(i=10;i<110;i++)
				{
					get_flash_data(buf,i);
					if(!strstr(buf,"\n"))
							break;
					usart3_send_str((uint8_t *)buf);
				}
			}	
			OSMutexPost(&ble_mutex,OS_OPT_POST_NONE,&err);
			
			//�Ŀ��Ż�����
			if(strstr(p,"pw"))
			{
				p=strtok((char *)p,"-");
				p=strtok(NULL,"-");
				sprintf(buf,"%c%c%c%c\n",p[0],p[1],p[2],p[3]);
				OSMutexPend(&flash_mutex,0,OS_OPT_PEND_BLOCKING,NULL,&err);
				write_info(buf,9);
				OSMutexPost(&flash_mutex,OS_OPT_POST_NONE,&err);
				
			}
			
			OSMutexPend(&print_mutex,0,OS_OPT_PEND_BLOCKING,NULL,&err);
		
			printf("msg:[%s],len[%d]\r\n",p,msg_size);
			
			OSMutexPost(&print_mutex,OS_OPT_POST_NONE,&err);
			
			//����ָ��������ݣ�Ҫ�������
			memset(p,0,msg_size);
		}
	}
}

void task2(void *parg)
{
	OS_ERR err;
	
	OS_FLAGS flags=0;
	
	printf("task2 is create ok\r\n");

	while(1)
	{
		//�ȴ���־λ1
		flags=OSFlagPend(&myFlag,0x01,0,
		OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_BLOCKING,0,&err);
		
		if(flags&0x01)
		{
			RTC_GetTime(RTC_Format_BCD,&RTC_TimeStructure);
			RTC_GetDate(RTC_Format_BCD,&RTC_DateStructure);
			
//			OSMutexPend(&print_mutex,0,OS_OPT_PEND_BLOCKING,NULL,&err);
//		
//			printf("20%02x/%02x/%02x\r\nWeek:%x\r\n%02x:%02x:%02x\r\n",RTC_DateStructure.RTC_Year,
//			RTC_DateStructure.RTC_Month,RTC_DateStructure.RTC_Date,RTC_DateStructure.RTC_WeekDay,
//			RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes,RTC_TimeStructure.RTC_Seconds);
//			
//			OSMutexPost(&print_mutex,OS_OPT_POST_NONE,&err);
		}
	}
}

void task3(void *parg)
{
	OS_ERR err;
	uint8_t buf[5]={0};
	int32_t rt=0;
	
	printf("task3 is create ok\r\n");

	//��ʪ�ȳ�ʼ��
	dht11_init();
	
	while(1)
	{
		rt=dht11_read_data(buf);
		
		if(rt==0)
		{
//			OSMutexPend(&print_mutex,0,OS_OPT_PEND_BLOCKING,NULL,&err);
//			
//			printf("T=%d.%d,H=%d.%d\r\n",buf[2],buf[3],buf[0],buf[1]);
//			
//			//ʹ���깲����Դ���ָ��ں˵���
//			OSMutexPost(&print_mutex,OS_OPT_POST_NONE,&err);
			uint8_t str[20];
			sprintf((char *)str,"T-%d.%d-H-%d.%d\n",buf[2],buf[3],buf[0],buf[1]);
			
			OSMutexPend(&ble_mutex,0,OS_OPT_PEND_BLOCKING,NULL,&err);
			usart3_send_str((uint8_t *)str);
			OSMutexPost(&ble_mutex,OS_OPT_POST_NONE,&err);
		}
		
		delay_ms(5000);
	}
}

void task4(void *parg)
{
	OS_ERR err;
	
	beep_init();
	printf("task4 is create ok\r\n");

	while(1)
	{
		uint8_t i,j,status,card_size;
		
		MFRC522_Initializtion();
		//
		status=MFRC522_Request(0x52, card_pydebuf);			//Ѱ��
		//
		if(status==0)		//���������
		{
			status=MFRC522_Anticoll(card_numberbuf);			//��ײ����			
			card_size=MFRC522_SelectTag(card_numberbuf);	//ѡ��
			status=MFRC522_Auth(0x60, 4, card_key0Abuf, card_numberbuf);	//�鿨
			status=MFRC522_Write(4, card_writebuf);				//д����д��ҪС�ģ��ر��Ǹ����Ŀ�3��
			status=MFRC522_Read(4, card_readbuf);					//����
			//MFRC522_Halt();															//ʹ����������״̬
			//��������ʾ		
			OSMutexPend(&print_mutex,0,OS_OPT_PEND_BLOCKING,NULL,&err);
			//�����к��ԣ����һ�ֽ�Ϊ����У����
			printf("card_numberbuf:%02X %02X %02X %02X\r\n",
					card_numberbuf[3],card_numberbuf[2],card_numberbuf[1],card_numberbuf[0]);

			OSMutexPost(&print_mutex,OS_OPT_POST_NONE,&err);
			
			checkCard(card_numberbuf);
			//���ɼ�¼
			OSQPost(&card_queue,(uint8_t *)card_numberbuf,5,OS_OPT_POST_FIFO,&err);
			
			//������
			
			for(i=0;i<4;i++)
			{
				tim4_set_duty(40);
				delay_ms(50);
				tim4_set_duty(100);
				delay_ms(50);
			}
		}
		
		delay_ms(1000);
	}
}

void task5(void *parg)
{
	OS_ERR err;
	uint8_t d,cur='N';
	uint8_t msg[5]={0};
	uint32_t count=0;
	
	printf("task5 is create ok\r\n");

	key_init();
	
	while(1)
	{
		d=get_key();
		if(d!='N'&&cur=='N')
		{
			OSMutexPend(&print_mutex,0,OS_OPT_PEND_BLOCKING,NULL,&err);
			printf("%c\r\n",d);
			OSMutexPost(&print_mutex,OS_OPT_POST_NONE,&err);
			cur=d;
			msg[count++]=d;
			
			tim4_set_duty(40);
			delay_ms(50);
			tim4_set_duty(100);
		}
		else if(d=='N'&&cur!='N')
		{
			cur='N';
		}
		
		if(count>=4)
		{
			if(strcmp(msg,(const char *)password)==0)
			{
				//��ӭ
			}
			count=0;
		}
		
		delay_ms(20);
	}
}

void task6(void *parg)
{
	OS_ERR err;
	
	uint8_t buf[64]={0};
	uint32_t recNum=0;
	
	//���Ƿ�Ϊ��һ�Σ�˳����count
	OSMutexPend(&flash_mutex,0,OS_OPT_PEND_BLOCKING,NULL,&err);
	check_flash(buf,password,&cardNum,&recNum);
	OSMutexPost(&flash_mutex,OS_OPT_POST_NONE,&err);
	
	printf("task6 is create ok\r\n");

	while(1)
	{
		uint8_t *info=NULL;
		OS_MSG_SIZE len;
		info=(uint8_t *)OSQPend(&card_queue,0,OS_OPT_PEND_BLOCKING,&len,NULL,&err);
		
		RTC_GetTime(RTC_Format_BCD,&RTC_TimeStructure);
		RTC_GetDate(RTC_Format_BCD,&RTC_DateStructure);
		sprintf((char *)buf,"[%d]20%02x/%02x/%02x Week%02x %02x:%02x:%02x %02X%02X%02X%02X %d\r\n",
							recNum+1,RTC_DateStructure.RTC_Year,RTC_DateStructure.RTC_Month,RTC_DateStructure.RTC_Date
							,RTC_DateStructure.RTC_WeekDay,RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes,
							RTC_TimeStructure.RTC_Seconds,info[3],info[2],info[1],info[0],info[4]);
		
		if(recNum<=100)
		{
			OSMutexPend(&flash_mutex,0,OS_OPT_PEND_BLOCKING,NULL,&err);
			write_flash(buf,10+recNum);
			OSMutexPost(&flash_mutex,OS_OPT_POST_NONE,&err);
			recNum = recNum+1;
		}
	}

}

void task7(void *parg)
{
	OS_ERR err;
	uint8_t buf[64]={0};
	
	OLED_Init();
	printf("task7 is create ok\r\n");
	
	
	while(1)
	{
		OLED_Clear();
		
		RTC_GetTime(RTC_Format_BCD,&RTC_TimeStructure);
		RTC_GetDate(RTC_Format_BCD,&RTC_DateStructure);
		sprintf((char *)buf,"20%02x/%02x/%02x",
							RTC_DateStructure.RTC_Year,RTC_DateStructure.RTC_Month,RTC_DateStructure.RTC_Date);
		
		OLED_ShowString(20,0,(u8 *)buf,12);
		
		sprintf((char *)buf,"%02x:%02x",
							RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes);
		OLED_ShowString(34,1,(u8 *)buf,16);
		
		
		delay_ms(10000);
	}

}







