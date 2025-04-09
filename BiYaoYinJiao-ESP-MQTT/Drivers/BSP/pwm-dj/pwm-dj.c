#include "pwm-dj.h"
#include "tim.h"
#include "stdio.h"



/**
    sampal����˼�ǣ�ֻͨ��pwm����ת�١���������ֶ��ڸߵ�ƽ��͵�ƽ�ϡ�����GPIOռ��
*/
void DJ_sampal_init(void){
    /**
    ������ʱ��C1 C2��pwm
*/    
    MX_TIM1_Init();
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2);
}

/**
    ������ʼ������ռ��6��gpio�ڡ����Կ���ֹͣ�������ٶ�
*/
void DJ_init(void){
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

	
	/*  ��ʼ������״̬Ϊ�ߵ�ƽ��ʹ��TB6612 �������ģ��*/
    HAL_GPIO_WritePin(DJ_EN_GPIO_Port, DJ_EN_Pin, GPIO_PIN_SET);

/**
    ��ʼֵȫΪ0�����ֹͣ    
*/
    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, DJ1_1_Pin|DJ1_2_Pin|DJ2_1_Pin|DJ2_2_Pin, GPIO_PIN_RESET);



    /*Configure GPIO pins : PAPin PAPin PAPin   �������ż�ʹ������*/
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
    ������ʱ��C1 C2��pwm
*/    
    //MX_TIM1_Init();
    //HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
    //HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2);
	if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK) {
    // ������
		//printf("tim err");
	}
	if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2) != HAL_OK) {
    // ������
		//printf("tim err");
	}
}

/**
    @num    ��Ϊpwm�����ռ�ձ�(0-99)��0Ϊֹͣ
    @i      ��Ϊͨ��1��2�Ŀ��ƣ���Ӧ���1�͵��2�Ŀ���
*/
void DJ_S_set(int i, int num){
    if(i == 0){
        __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,num);
    }else{
        __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,num);
    }
}

/**
    @i      ѡ����(0/1)
    @a      ѡ��ֹͣ����ת����ת(0/1/2)
    @num    �����ٶ�(0-99)
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
    TB6612�е͹���ģʽ��ͨ��STBY��������͵�ƽ���롣�����ߵ�Ʒʹ��
    �ߵ�ƽʹ�ܡ�
*/



