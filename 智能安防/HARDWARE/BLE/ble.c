#include "ble.h"

static GPIO_InitTypeDef GPIO_InitStruct;
static USART_InitTypeDef USART_InitStructure;
static NVIC_InitTypeDef		NVIC_InitStructure;

volatile uint8_t  g_buf[64]={0};
volatile uint32_t g_count=0;

void usart3_send_str(uint8_t *pbuf)
{
	while(pbuf && *pbuf)
	{
		USART_SendData(USART3,*pbuf++);
		while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==RESET);
	}
}

void  usart3_init(uint32_t baud)
{

	//使能端口B的硬件时钟，端口B才能工作，说白了就是对端口B上电
	//串口1硬件时钟使能
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);	
	
	//配置GPIO口，端口
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed=GPIO_High_Speed;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_Init(GPIOC,&GPIO_InitStruct);

	//将PC10,PC11引脚连接到串口1
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_USART3);
	
	//配置串口3
	USART_InitStructure.USART_BaudRate=baud;//波特率
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;//8个数据位
	USART_InitStructure.USART_StopBits=USART_StopBits_1;//1停止位
	USART_InitStructure.USART_Parity=USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;//不需要流控制
	USART_InitStructure.USART_Mode=USART_Mode_Rx | USART_Mode_Tx;//允许串口接收和写
	USART_Init(USART3,&USART_InitStructure);
	
	//接收数据后触发中断
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);
	
	//配置优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	

	USART_Cmd(USART3, ENABLE);
}

void USART3_IRQHandler(void)
{
	uint8_t  g_event=0;
	uint8_t d;
	
	OSIntEnter(); 
	
	if(USART_GetFlagStatus(USART3,USART_IT_RXNE)==SET)
	{
		//读取数据
		d = USART_ReceiveData(USART3);
		
		if(d!='#'&&g_count<64)
		{
			g_buf[g_count++]=d;
		}
		else
		{
			g_event=1;
		}
		
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
	}
	OSIntExit();
	
	if(g_event)
	{
		OS_ERR err;
		OSQPost(&g_queue,(uint8_t *)g_buf,g_count,OS_OPT_POST_FIFO,&err);
		g_count=0;
	}
}


