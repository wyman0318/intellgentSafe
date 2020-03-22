
#include "dht11.h"

static GPIO_InitTypeDef GPIO_InitStruct;

void  dht11_init(void)
{

	//使能端口B的硬件时钟，端口B才能工作，说白了就是对端口B上电
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);	
	
	//配置硬件，配置GPIO，端口B，第6个引脚
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_5;			//第6个引脚
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;		//输出模式
	GPIO_InitStruct.GPIO_Speed=GPIO_High_Speed;		//引脚高速工作，收到指令立即工作；缺点：功耗高
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;		//增加输出电流的能力
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;		//不需要上下拉电阻
	GPIO_Init(GPIOE,&GPIO_InitStruct);	

	//模块还没工作，触发引脚电平为低电平
	PEout(5)=1;

}

void dht11_data_pin_mode(GPIOMode_TypeDef gpio_mode)
{
	//配置硬件，配置GPIO，端口B，第6个引脚
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_5;			//第6个引脚
	GPIO_InitStruct.GPIO_Mode=gpio_mode;		//输出模式
	GPIO_InitStruct.GPIO_Speed=GPIO_High_Speed;		//引脚高速工作，收到指令立即工作；缺点：功耗高
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;		//增加输出电流的能力
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;		//不需要上下拉电阻
	GPIO_Init(GPIOE,&GPIO_InitStruct);	
}

int32_t dht11_read_data(uint8_t *dht11_data)
{
	uint32_t t = 0;
	int32_t i=0,j=0;
	uint8_t d=0;
	uint8_t check_sum=0;
	
	//保证为输出模式
	dht11_data_pin_mode(GPIO_Mode_OUT);
	
	PEout(5)=0;
	delay_ms(18);
	
	PEout(5)=1;
	delay_us(30);
	
	//保证引脚为输出模式
	dht11_data_pin_mode(GPIO_Mode_IN);
	
	//等待低电平出现,若在100us内检测不了低电平，就返回
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
	
	//等待低电平持续完毕,若超过100us，则认为当前低电平是无效
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
	
	//等待高电平持续完毕,若超过100us则认为高电平无效
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
	
	
	
	//开始接收数据
	for(j=0;j<5;j++)
	{
		//一次接收一个字节
		for(i=7;i>=0;i--)
		{
			//等待低电平持续完毕,若超过100us，则认为当前低电平是无效
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
			
			//延时40us可判断当前电平即可知道0还是1
			delay_us(40);
			
			//判断电平
			if(PEin(5))
			{
				d |= 1<<i;
			}
			
			//等待高电平持续完毕,若超过100us则认为高电平无效
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
	
	//通信结束
	delay_us(100);
	
	//进行数据校验,检测数据的准确性,超出100其实只比较低八位
	check_sum = dht11_data[0]+dht11_data[1]+dht11_data[2]+dht11_data[3];

	if(check_sum != dht11_data[4])
	{
		return -6;
	}
	
	return 0;
}
