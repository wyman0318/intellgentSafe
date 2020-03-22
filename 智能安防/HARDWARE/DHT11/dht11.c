
#include "dht11.h"

static GPIO_InitTypeDef GPIO_InitStruct;

void  dht11_init(void)
{

	//ʹ�ܶ˿�B��Ӳ��ʱ�ӣ��˿�B���ܹ�����˵���˾��ǶԶ˿�B�ϵ�
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);	
	
	//����Ӳ��������GPIO���˿�B����6������
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_5;			//��6������
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;		//���ģʽ
	GPIO_InitStruct.GPIO_Speed=GPIO_High_Speed;		//���Ÿ��ٹ������յ�ָ������������ȱ�㣺���ĸ�
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;		//�����������������
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;		//����Ҫ����������
	GPIO_Init(GPIOE,&GPIO_InitStruct);	

	//ģ�黹û�������������ŵ�ƽΪ�͵�ƽ
	PEout(5)=1;

}

void dht11_data_pin_mode(GPIOMode_TypeDef gpio_mode)
{
	//����Ӳ��������GPIO���˿�B����6������
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_5;			//��6������
	GPIO_InitStruct.GPIO_Mode=gpio_mode;		//���ģʽ
	GPIO_InitStruct.GPIO_Speed=GPIO_High_Speed;		//���Ÿ��ٹ������յ�ָ������������ȱ�㣺���ĸ�
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;		//�����������������
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;		//����Ҫ����������
	GPIO_Init(GPIOE,&GPIO_InitStruct);	
}

int32_t dht11_read_data(uint8_t *dht11_data)
{
	uint32_t t = 0;
	int32_t i=0,j=0;
	uint8_t d=0;
	uint8_t check_sum=0;
	
	//��֤Ϊ���ģʽ
	dht11_data_pin_mode(GPIO_Mode_OUT);
	
	PEout(5)=0;
	delay_ms(18);
	
	PEout(5)=1;
	delay_us(30);
	
	//��֤����Ϊ���ģʽ
	dht11_data_pin_mode(GPIO_Mode_IN);
	
	//�ȴ��͵�ƽ����,����100us�ڼ�ⲻ�˵͵�ƽ���ͷ���
	t=0;
	while(PEin(5))
	{
		t++;
		delay_us(1);
		if(t>=1000)
		{
			return -1;
		}
	}
	
	//�ȴ��͵�ƽ�������,������100us������Ϊ��ǰ�͵�ƽ����Ч
	t=0;
	while(PEin(5)==0)
	{
		t++;
		delay_us(1);
		if(t>=100)
		{
			return -2;
		}
	}
	
	//�ȴ��ߵ�ƽ�������,������100us����Ϊ�ߵ�ƽ��Ч
	t=0;
	while(PEin(5))
	{
		t++;
		delay_us(1);
		if(t>=100)
		{
			return -3;
		}
	}
	
	
	
	//��ʼ��������
	for(j=0;j<5;j++)
	{
		//һ�ν���һ���ֽ�
		for(i=7;i>=0;i--)
		{
			//�ȴ��͵�ƽ�������,������100us������Ϊ��ǰ�͵�ƽ����Ч
			t=0;
			while(PEin(5)==0)
			{
				t++;
				delay_us(1);
				if(t>=100)
				{
					return -4;
				}
			}
			
			//��ʱ40us���жϵ�ǰ��ƽ����֪��0����1
			delay_us(40);
			
			//�жϵ�ƽ
			if(PEin(5))
			{
				d |= 1<<i;
			}
			
			//�ȴ��ߵ�ƽ�������,������100us����Ϊ�ߵ�ƽ��Ч
			t=0;
			while(PEin(5))
			{
				t++;
				delay_us(1);
				if(t>=100)
				{
					return -5;
				}
			}
			
		}
		
		dht11_data[j]=d;
		d=0;
	}
	
	//ͨ�Ž���
	delay_us(100);
	
	//��������У��,������ݵ�׼ȷ��,����100��ʵֻ�ȽϵͰ�λ
	check_sum = dht11_data[0]+dht11_data[1]+dht11_data[2]+dht11_data[3];

	if(check_sum != dht11_data[4])
	{
		return -6;
	}
	
	return 0;
}
