#include "command.h"

static RTC_DateTypeDef RTC_DateStructure;
static RTC_TimeTypeDef  RTC_TimeStructure;
//用户缓存区
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

//任务1控制块 修改时间
OS_TCB Task1_TCB;
void task1(void *parg);
CPU_STK task1_stk[256];			//任务1的任务堆栈，大小为128字，也就是512字节

//任务2控制块 显示时间
OS_TCB Task2_TCB;
void task2(void *parg);
CPU_STK task2_stk[128];			//任务2的任务堆栈，大小为128字，也就是512字节

//任务3控制块 显示温湿度
OS_TCB Task3_TCB;
void task3(void *parg);
CPU_STK task3_stk[256];			//任务2的任务堆栈，大小为256字，也就是1024字节

//任务4控制块 RFID
OS_TCB Task4_TCB;
void task4(void *parg);
CPU_STK task4_stk[256];

//任务5控制块 矩阵键盘
OS_TCB Task5_TCB;
void task5(void *parg);
CPU_STK task5_stk[128];

//任务6控制块 存储记录
OS_TCB Task6_TCB;
void task6(void *parg);
CPU_STK task6_stk[256];

//任务7控制块 添加卡号
OS_TCB Task7_TCB;
void task7(void *parg);
CPU_STK task7_stk[128];

//消息队列
OS_Q g_queue;
OS_Q card_queue;
OS_Q id_queue;

//标志组
OS_FLAG_GRP myFlag;

//互斥锁
OS_MUTEX print_mutex;
OS_MUTEX ble_mutex;
OS_MUTEX flash_mutex;

//主函数
int main(void)
{
	OS_ERR err;

	systick_init();  													//时钟初始化
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);						//中断分组配置
	
	usart_init(115200);  				 									//串口初始化
	usart3_init(9600);
	
	rtc_cofig();
	
	LED_Init();         												//LED初始化	


	//OS初始化，它是第一个运行的函数,初始化各种的全局变量，例如中断嵌套计数器、优先级、存储器
	OSInit(&err);


	//创建任务1
	OSTaskCreate(	(OS_TCB *)&Task1_TCB,		//任务控制块，等同于线程id
					(CPU_CHAR *)"Task1",		//任务的名字，名字可以自定义的
					(OS_TASK_PTR)task1,			//任务函数，等同于线程函数
					(void *)0,					//传递参数，等同于线程的传递参数
					(OS_PRIO)6,				 	//任务的优先级6		
					(CPU_STK *)task1_stk,		//任务堆栈基地址
					(CPU_STK_SIZE)256/10,		//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					(CPU_STK_SIZE)256,			//任务堆栈大小			
					(OS_MSG_QTY)0,				//禁止任务消息队列
					(OS_TICK)0,					//默认时间片长度																
					(void  *)0,					//不需要补充用户存储区
					(OS_OPT)OS_OPT_TASK_NONE,	//没有任何选项
					&err						//返回的错误码
				);


	//创建任务2
	OSTaskCreate(	(OS_TCB *)&Task2_TCB,		//任务控制块
					(CPU_CHAR *)"Task2",		//任务的名字
					(OS_TASK_PTR)task2,			//任务函数
					(void *)0,					//传递参数
					(OS_PRIO)6,				 	//任务的优先级7		
					(CPU_STK *)task2_stk,		//任务堆栈基地址
					(CPU_STK_SIZE)128/10,		//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					(CPU_STK_SIZE)128,			//任务堆栈大小			
					(OS_MSG_QTY)0,				//禁止任务消息队列
					(OS_TICK)0,					//默认时间片长度																
					(void  *)0,					//不需要补充用户存储区
					(OS_OPT)OS_OPT_TASK_NONE,	//没有任何选项
					&err						//返回的错误码
				);
				
	//创建任务3
	OSTaskCreate(	(OS_TCB *)&Task3_TCB,		//任务控制块
					(CPU_CHAR *)"Task3",		//任务的名字
					(OS_TASK_PTR)task3,			//任务函数
					(void *)0,					//传递参数
					(OS_PRIO)6,				 	//任务的优先级7		
					(CPU_STK *)task3_stk,		//任务堆栈基地址
					(CPU_STK_SIZE)256/10,		//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					(CPU_STK_SIZE)256,			//任务堆栈大小			
					(OS_MSG_QTY)0,				//禁止任务消息队列
					(OS_TICK)0,					//默认时间片长度																
					(void  *)0,					//不需要补充用户存储区
					(OS_OPT)OS_OPT_TASK_NONE,	//没有任何选项
					&err						//返回的错误码
				);
				
	//创建任务4
	OSTaskCreate(	(OS_TCB *)&Task4_TCB,		//任务控制块
					(CPU_CHAR *)"Task4",		//任务的名字
					(OS_TASK_PTR)task4,			//任务函数
					(void *)0,					//传递参数
					(OS_PRIO)6,				 	//任务的优先级7		
					(CPU_STK *)task4_stk,		//任务堆栈基地址
					(CPU_STK_SIZE)256/10,		//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					(CPU_STK_SIZE)256,			//任务堆栈大小			
					(OS_MSG_QTY)0,				//禁止任务消息队列
					(OS_TICK)0,					//默认时间片长度																
					(void  *)0,					//不需要补充用户存储区
					(OS_OPT)OS_OPT_TASK_NONE,	//没有任何选项
					&err						//返回的错误码
				);
				
	//创建任务5
	OSTaskCreate(	(OS_TCB *)&Task5_TCB,		//任务控制块
					(CPU_CHAR *)"Task5",		//任务的名字
					(OS_TASK_PTR)task5,			//任务函数
					(void *)0,					//传递参数
					(OS_PRIO)6,				 	//任务的优先级7		
					(CPU_STK *)task4_stk,		//任务堆栈基地址
					(CPU_STK_SIZE)128/10,		//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					(CPU_STK_SIZE)128,			//任务堆栈大小			
					(OS_MSG_QTY)0,				//禁止任务消息队列
					(OS_TICK)0,					//默认时间片长度																
					(void  *)0,					//不需要补充用户存储区
					(OS_OPT)OS_OPT_TASK_NONE,	//没有任何选项
					&err						//返回的错误码
				);
				
	//创建任务6
	OSTaskCreate(	(OS_TCB *)&Task6_TCB,		//任务控制块
					(CPU_CHAR *)"Task6",		//任务的名字
					(OS_TASK_PTR)task6,			//任务函数
					(void *)0,					//传递参数
					(OS_PRIO)6,				 	//任务的优先级7		
					(CPU_STK *)task6_stk,		//任务堆栈基地址
					(CPU_STK_SIZE)256/10,		//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					(CPU_STK_SIZE)256,			//任务堆栈大小			
					(OS_MSG_QTY)0,				//禁止任务消息队列
					(OS_TICK)0,					//默认时间片长度																
					(void  *)0,					//不需要补充用户存储区
					(OS_OPT)OS_OPT_TASK_NONE,	//没有任何选项
					&err						//返回的错误码
				);
				
	//创建任务7
	OSTaskCreate(	(OS_TCB *)&Task7_TCB,		//任务控制块
					(CPU_CHAR *)"Task7",		//任务的名字
					(OS_TASK_PTR)task7,			//任务函数
					(void *)0,					//传递参数
					(OS_PRIO)6,				 	//任务的优先级7		
					(CPU_STK *)task7_stk,		//任务堆栈基地址
					(CPU_STK_SIZE)128/10,		//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					(CPU_STK_SIZE)128,			//任务堆栈大小			
					(OS_MSG_QTY)0,				//禁止任务消息队列
					(OS_TICK)0,					//默认时间片长度																
					(void  *)0,					//不需要补充用户存储区
					(OS_OPT)OS_OPT_TASK_NONE,	//没有任何选项
					&err						//返回的错误码
				);

	//创建消息队列,32个消息长度
	OSQCreate(&g_queue,"g_queue",32,&err);
	OSQCreate(&card_queue,"card_queue",32,&err);
	OSQCreate(&id_queue,"id_queue",32,&err);
	
	//创建32位标志组
	OSFlagCreate(&myFlag,"myFlag",0,&err);
	
	//创建互斥锁
	OSMutexCreate(&print_mutex,"print_mutex",&err);
	OSMutexCreate(&ble_mutex,"ble_mutex",&err);
	OSMutexCreate(&flash_mutex,"beep_mutex",&err);

	//启动OS，进行任务调度
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
		//等待消息队列，0：阻塞等待
		p=OSQPend(&g_queue,0,OS_OPT_PEND_BLOCKING,&msg_size,NULL,&err);
		
		//检测指针是有效且有效的数据长度
		if(p&&msg_size)
		{
			//改时间
			if(strstr(p,"time"))
				changeTime(p);
			
			//取记录
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
			
			//改卡号或密码
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
			
			//由于指向的是数据，要清空数据
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
		//等待标志位1
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

	//温湿度初始化
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
//			//使用完共享资源，恢复内核调度
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
		status=MFRC522_Request(0x52, card_pydebuf);			//寻卡
		//
		if(status==0)		//如果读到卡
		{
			status=MFRC522_Anticoll(card_numberbuf);			//防撞处理			
			card_size=MFRC522_SelectTag(card_numberbuf);	//选卡
			status=MFRC522_Auth(0x60, 4, card_key0Abuf, card_numberbuf);	//验卡
			status=MFRC522_Write(4, card_writebuf);				//写卡（写卡要小心，特别是各区的块3）
			status=MFRC522_Read(4, card_readbuf);					//读卡
			//MFRC522_Halt();															//使卡进入休眠状态
			//卡类型显示		
			OSMutexPend(&print_mutex,0,OS_OPT_PEND_BLOCKING,NULL,&err);
			//卡序列号显，最后一字节为卡的校验码
			printf("card_numberbuf:%02X %02X %02X %02X\r\n",
					card_numberbuf[3],card_numberbuf[2],card_numberbuf[1],card_numberbuf[0]);

			OSMutexPost(&print_mutex,OS_OPT_POST_NONE,&err);
			
			checkCard(card_numberbuf);
			//生成记录
			OSQPost(&card_queue,(uint8_t *)card_numberbuf,5,OS_OPT_POST_FIFO,&err);
			
			//蜂鸣器
			
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
				//欢迎
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
	
	//看是否为第一次，顺便拿count
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







