#include "pwm_duoji.h"
#include "tim.h"


void duoji_init(void){
    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
}


void duoji_set(int a, int num){    //舵机控制范围为50HZ。0.5-2.5ms。这里0-199。20ms。计算：20ms/200=0.1ms。0.5ms则为5。2.5ms则为25。范围为4-24
    if(num > 25){
        num = 25;
    }else if(num < 5){
        num = 5;
    }
    
    if(a == 1){
        __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,num);

    }else if(a == 2){
        __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,num);

    }
}

void led_set(int num){
    if(num > 199)   num = 199;
    else if(num < 0)    num = 0;
    
    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,num);

}
