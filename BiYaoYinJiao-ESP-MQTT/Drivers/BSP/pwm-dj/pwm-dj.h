#ifndef _PWM_DJ_H_
#define _PWM_DJ_H_

/*
    电机1的方向控制引脚，sample模式下不使用 00停止，01正，10反
*/
#define DJ1_1_Pin GPIO_PIN_10
#define DJ1_1_GPIO_Port GPIOA
#define DJ1_2_Pin GPIO_PIN_11
#define DJ1_2_GPIO_Port GPIOA
/*
    电机2的方向控制引脚，sample模式下不使用
*/
#define DJ2_1_Pin GPIO_PIN_12
#define DJ2_1_GPIO_Port GPIOA
#define DJ2_2_Pin GPIO_PIN_15
#define DJ2_2_GPIO_Port GPIOA

/**
    使能引脚，高电平有效，低电平低功耗休眠
*/
#define DJ_EN_Pin GPIO_PIN_3
#define DJ_EN_GPIO_Port GPIOB



/*  设置方向引脚状态*/
#define dj11(x) HAL_GPIO_WritePin(DJ1_1_GPIO_Port, DJ1_1_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET);
#define dj12(x) HAL_GPIO_WritePin(DJ1_2_GPIO_Port, DJ1_2_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET);
#define dj21(x) HAL_GPIO_WritePin(DJ2_1_GPIO_Port, DJ2_1_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET);
#define dj22(x) HAL_GPIO_WritePin(DJ2_2_GPIO_Port, DJ2_2_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET);


void DJ_sampal_init(void);  //简单控制初始化
void DJ_init(void);         //完全控制初始化
void DJ_S_set(int i, int num);      //简单控制函数
void DJ_set(int i, int a, int num); //完全控制函数



#endif
