#include "stm32f10x.h"
#include "stdio.h"

uint64_t delay_ticks;

uint16_t pwm_38k_width = 1894;         // 72MHz / 38K = 1894
uint16_t pwm_38k_duty_cycle = 631;     // 1894 / 3 = 631. Duty cycle is 1/3, must less than 50% .

uint64_t head_high  = 6000;
uint64_t head_low   = 7500;
uint64_t data_high  =  600;
uint64_t data_1_low = 1650;
uint64_t data_0_low =  600;
uint64_t tail_high  =  600;
uint64_t tail_low   = 7300;

uint8_t cmd[12] = {0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x9B, 0x64, 0xAB, 0x54, 0x54, 0xAB};

void SysTick_Handler(void) {
	if (delay_ticks > 0) {
		delay_ticks--;
	} else {
		SysTick->CTRL = 0x00;
		SysTick->VAL  = 0x00;
	}
}

void delay_us(unsigned long n) {
	delay_ticks = n/10;
	SysTick_Config(SystemCoreClock/(1000000/10));
	while(delay_ticks>0);
}

void delay_ms(unsigned long n) {
	delay_ticks = n;
	SysTick_Config(SystemCoreClock/1000);
	while(delay_ticks>0);
}

int main() {
	int i, j;

	GPIO_InitTypeDef        GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef       TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);	
	
	TIM_TimeBaseStructure.TIM_Period = pwm_38k_width;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_ARRPreloadConfig(TIM2, ENABLE);
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
	
	TIM_Cmd(TIM2, ENABLE);
	
	TIM_SetCompare2(TIM2, pwm_38k_duty_cycle);
	delay_us(head_high);
	
	TIM_SetCompare2(TIM2, 0);
	delay_us(head_low);
	
	for (i=0;i<12;i++) {
		for (j=0;j<8;j++) {
			TIM_SetCompare2(TIM2, pwm_38k_duty_cycle);
			delay_us(data_high);
			
			TIM_SetCompare2(TIM2, 0);
			if ((cmd[i]<<j) & 0x80) {
				delay_us(data_1_low);
			} else {
				delay_us(data_0_low);
			}
		}
	}
	
	TIM_SetCompare2(TIM2, pwm_38k_duty_cycle);
	delay_us(tail_high);
	
	TIM_SetCompare2(TIM2, 0);
	delay_us(tail_low);
	
	TIM_SetCompare2(TIM2, pwm_38k_duty_cycle);
	delay_us(tail_high);
	
	TIM_SetCompare2(TIM2, 0);
	
	while (1) {
	
	}
}
