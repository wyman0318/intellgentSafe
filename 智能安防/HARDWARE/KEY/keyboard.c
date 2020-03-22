#include "keyboard.h"

static GPIO_InitTypeDef GPIO_InitStruct;

void key_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_1;			//第6个引脚
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;			//输入模式
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz;		//引脚高速工作，收到指令立即工作；缺点：功耗高
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;		//增加输出电流的能力
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;		//不需要上下拉电阻
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_12;			//第6个引脚
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;			//输入模式
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz;		//引脚高速工作，收到指令立即工作；缺点：功耗高
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;		//增加输出电流的能力
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;		//不需要上下拉电阻
	GPIO_Init(GPIOE,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_14;			//第6个引脚
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN;			//输入模式
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz;		//引脚高速工作，收到指令立即工作；缺点：功耗高
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;		//增加输出电流的能力
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;		//不需要上下拉电阻
	GPIO_Init(GPIOE,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_14 | GPIO_Pin_12 | GPIO_Pin_10;			//第6个引脚
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN;			//输入模式
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz;		//引脚高速工作，收到指令立即工作；缺点：功耗高
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;		//增加输出电流的能力
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;		//不需要上下拉电阻
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	//e14 col1
	//b10 col2
	//b12 col3
	//b14 col4
	PBout(1)=1; //row1
	PEout(8)=1; //row2
	PEout(10)=1;//row3
	PEout(12)=1;//row4
}

char get_key(void)
{
	PBout(1)=0; //row1
	PEout(8)=1; //row2
	PEout(10)=1;//row3
	PEout(12)=1;//row4
	
	if(!PEin(14)) return '1';
	else if(!PBin(10)) return '2';
	else if(!PBin(12)) return '3';
	else if(!PBin(14)) return 'A';
	
	PBout(1)=1; //row1
	PEout(8)=0; //row2
	PEout(10)=1;//row3
	PEout(12)=1;//row4
	
	if(!PEin(14)) return '4';
	else if(!PBin(10)) return '5';
	else if(!PBin(12)) return '6';
	else if(!PBin(14)) return 'B';
	
	PBout(1)=1; //row1
	PEout(8)=1; //row2
	PEout(10)=0;//row3
	PEout(12)=1;//row4
	
	if(!PEin(14)) return '7';
	else if(!PBin(10)) return '8';
	else if(!PBin(12)) return '9';
	else if(!PBin(14)) return 'C';
	
	PBout(1)=1; //row1
	PEout(8)=1; //row2
	PEout(10)=1;//row3
	PEout(12)=0;//row4
	
	if(!PEin(14)) return '*';
	else if(!PBin(10)) return '0';
	else if(!PBin(12)) return '#';
	else if(!PBin(14)) return 'D';
	
	return 'N';
}



