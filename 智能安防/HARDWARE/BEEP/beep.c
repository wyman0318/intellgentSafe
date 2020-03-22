#include "beep.h"

static GPIO_InitTypeDef GPIO_InitStruct;
static TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
static TIM_OCInitTypeDef  TIM_OCInitStructure;
static uint32_t tim3_count=0;

void beep_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	
	//������
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_6;			//��6������
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;			//����ģʽ
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz;		//���Ÿ��ٹ������յ�ָ������������ȱ�㣺���ĸ�
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;		//�����������������
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;		//����Ҫ����������
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_TIM4);
	
	//���ö�ʱ��14�����Ƶ��100Hz
	TIM_TimeBaseStructure.TIM_Period = (10000/780)-1;			//��������Ƶ��50Hz
	TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;				//���÷�Ƶֵ��ȷ����ʱ����ʱ��Ƶ��
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//���ϼ�����0->TIM_Period�ͻᴥ���ж�����
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	//���ö�ʱ��14ͨ��1���������
	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;				//������PWMģʽ1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//�����������
	//TIM_OCInitStructure.TIM_Pulse = CCR1_Val;						//����ռ�ձ�
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;		//������ֵС�ڱȽ�ֵ������ߵ�ƽ��������ֵ�ݱȽ�ֵ������͵�ƽ��
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);
	
	//ʹ�ܶ�ʱ������
	TIM_Cmd(TIM4,ENABLE);
	
	TIM_SetCompare1(TIM4,19);
	
	tim3_count= TIM_TimeBaseStructure.TIM_Period;
}

void tim4_set_freq(uint32_t freq)
{
	/*��ʱ���Ļ������ã��������ö�ʱ������������Ƶ��Ϊfreq Hz */
	TIM_TimeBaseStructure.TIM_Period = (10000/freq)-1;			//��������Ƶ��50Hz
	TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;				//���÷�Ƶֵ��ȷ����ʱ����ʱ��Ƶ��
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;			//�ڶ��η�Ƶ,��ǰʵ��1��Ƶ��Ҳ���ǲ���Ƶ

	tim3_count= TIM_TimeBaseStructure.TIM_Period;
	
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

}

//���ö�ʱ��13��PWMռ�ձ�0%~100%
void tim4_set_duty(uint32_t duty)
{
	uint32_t cmp=0;
	
	cmp = (tim3_count+1) * duty/100;

	TIM_SetCompare1(TIM4,cmp);
}

