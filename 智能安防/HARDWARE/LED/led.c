#include "led.h" 

//��ʼ�� PF9��PF10��PE13��PE14 Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void LED_Init(void)
{    	 
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);			//ʹ��GPIOE��GPIOFʱ��
	
	//GPIOF9,F10��ʼ������ 
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;		//LED0��LED1��ӦIO��
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;			    //��ͨ���ģʽ��
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				//�������������LED��Ҫ��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		    //100MHz
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;				    //����
	GPIO_Init(GPIOA, &GPIO_InitStructure);						//��ʼ��GPIOF�������õ�����д��Ĵ���
	
	GPIO_SetBits(GPIOA,GPIO_Pin_6  | GPIO_Pin_7);			    //GPIOF9,PF10���øߣ�����			//GPIOE13,PE14���øߣ�����
//	GPIO_ResetBits(GPIOA,GPIO_Pin_7  | GPIO_Pin_7);
}






