#ifndef __SETDATE_H
#define __SETDATE_H

#include "main.h" // ���� HAL ��ͷ�ļ�

// ��������
extern uint8_t usart1_RX1[512];
// ��������
void process_sntp_data(UART_HandleTypeDef *huart, RTC_HandleTypeDef *hrtc);
void send_time_request(UART_HandleTypeDef *huart);


#endif /* __SETDATE_H */

