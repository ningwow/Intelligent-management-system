#ifndef _PWM_DJ_H_
#define _PWM_DJ_H_

/*
    ���1�ķ���������ţ�sampleģʽ�²�ʹ�� 00ֹͣ��01����10��
*/
#define DJ1_1_Pin GPIO_PIN_10
#define DJ1_1_GPIO_Port GPIOA
#define DJ1_2_Pin GPIO_PIN_11
#define DJ1_2_GPIO_Port GPIOA
/*
    ���2�ķ���������ţ�sampleģʽ�²�ʹ��
*/
#define DJ2_1_Pin GPIO_PIN_12
#define DJ2_1_GPIO_Port GPIOA
#define DJ2_2_Pin GPIO_PIN_15
#define DJ2_2_GPIO_Port GPIOA

/**
    ʹ�����ţ��ߵ�ƽ��Ч���͵�ƽ�͹�������
*/
#define DJ_EN_Pin GPIO_PIN_3
#define DJ_EN_GPIO_Port GPIOB



/*  ���÷�������״̬*/
#define dj11(x) HAL_GPIO_WritePin(DJ1_1_GPIO_Port, DJ1_1_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET);
#define dj12(x) HAL_GPIO_WritePin(DJ1_2_GPIO_Port, DJ1_2_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET);
#define dj21(x) HAL_GPIO_WritePin(DJ2_1_GPIO_Port, DJ2_1_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET);
#define dj22(x) HAL_GPIO_WritePin(DJ2_2_GPIO_Port, DJ2_2_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET);


void DJ_sampal_init(void);  //�򵥿��Ƴ�ʼ��
void DJ_init(void);         //��ȫ���Ƴ�ʼ��
void DJ_S_set(int i, int num);      //�򵥿��ƺ���
void DJ_set(int i, int a, int num); //��ȫ���ƺ���



#endif
