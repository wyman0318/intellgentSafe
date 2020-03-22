#include "beep.h"

static GPIO_InitTypeDef GPIO_InitStruct;
static TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
static TIM_OCInitTypeDef  TIM_OCInitStructure;
static uint32_t tim3_count=0;

void beep_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	
	//蜂鸣器
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_6;			//第6个引脚
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;			//输入模式
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz;		//引脚高速工作，收到指令立即工作；缺点：功耗高
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;		//增加输出电流的能力
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;		//不需要上下拉电阻
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_TIM4);
	
	//配置定时器14的输出频率100Hz
	TIM_TimeBaseStructure.TIM_Period = (10000/780)-1;			//输出脉冲的频率50Hz
	TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;				//配置分频值，确定定时器的时钟频率
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//向上计数，0->TIM_Period就会触发中断请求
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	//配置定时器14通道1的输出功能
	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;				//工作在PWM模式1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//允许输出脉冲
	//TIM_OCInitStructure.TIM_Pulse = CCR1_Val;						//设置占空比
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;		//当计数值小于比较值，输出高电平；当计数值≥比较值，输出低电平。
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);
	
	//使能定时器工作
	TIM_Cmd(TIM4,ENABLE);
	
	TIM_SetCompare1(TIM4,19);
	
	tim3_count= TIM_TimeBaseStructure.TIM_Period;
}

void tim4_set_freq(uint32_t freq)
{
	/*定时器的基本配置，用于配置定时器的输出脉冲的频率为freq Hz */
	TIM_TimeBaseStructure.TIM_Period = (10000/freq)-1;			//输出脉冲的频率50Hz
	TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;				//配置分频值，确定定时器的时钟频率
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;			//第二次分频,当前实现1分频，也就是不分频

	tim3_count= TIM_TimeBaseStructure.TIM_Period;
	
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

}

//设置定时器13的PWM占空比0%~100%
void tim4_set_duty(uint32_t duty)
{
	uint32_t cmp=0;
	
	cmp = (tim3_count+1) * duty/100;

	TIM_SetCompare1(TIM4,cmp);
}

