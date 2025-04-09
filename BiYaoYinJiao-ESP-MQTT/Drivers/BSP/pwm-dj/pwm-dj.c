#include "pwm-dj.h"
#include "tim.h"
#include "stdio.h"



/**
    sampal的意思是：只通过pwm控制转速。方向控制手动在高电平与低电平上。减少GPIO占用
*/
void DJ_sampal_init(void){
    /**
    启动定时器C1 C2的pwm
*/    
    MX_TIM1_Init();
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2);
}

/**
    正常初始化，共占用6个gpio口。可以控制停止，方向，速度
*/
void DJ_init(void){
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

	
	/*  初始化引脚状态为高电平。使能TB6612 电机驱动模块*/
    HAL_GPIO_WritePin(DJ_EN_GPIO_Port, DJ_EN_Pin, GPIO_PIN_SET);

/**
    初始值全为0。电机停止    
*/
    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, DJ1_1_Pin|DJ1_2_Pin|DJ2_1_Pin|DJ2_2_Pin, GPIO_PIN_RESET);



    /*Configure GPIO pins : PAPin PAPin PAPin   控制引脚及使能引脚*/
    GPIO_InitStruct.Pin = DJ1_1_Pin|DJ1_2_Pin|DJ2_1_Pin|DJ2_2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pin : PtPin  EN PORT*/
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
/**
    启动定时器C1 C2的pwm
*/    
    //MX_TIM1_Init();
    //HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
    //HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2);
	if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK) {
    // 错误处理
		//printf("tim err");
	}
	if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2) != HAL_OK) {
    // 错误处理
		//printf("tim err");
	}
}

/**
    @num    作为pwm输出的占空比(0-99)，0为停止
    @i      作为通道1或2的控制，对应电机1和电机2的控制
*/
void DJ_S_set(int i, int num){
    if(i == 0){
        __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,num);
    }else{
        __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,num);
    }
}

/**
    @i      选择电机(0/1)
    @a      选择：停止，正转，反转(0/1/2)
    @num    控制速度(0-99)
*/
void DJ_set(int i, int a, int num){
    if(i == 1){
        switch(a){
            case 0:dj11(0);dj12(0);
                break;
            case 1:dj11(1);dj12(0);
                break;
            case 2:dj11(0);dj12(1);
                break;
        }
        __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,num);
    }else if(i == 2){
        switch(a){
            case 0:dj21(0);dj22(0);
                break;
            case 1:dj21(1);dj22(0);
                break;
            case 2:dj21(0);dj22(1);
                break;
        }
        __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,num);
    }
}

/**
    TB6612有低功耗模式，通过STBY引脚输入低电平进入。正常高点品使能
    高点平使能。
*/



