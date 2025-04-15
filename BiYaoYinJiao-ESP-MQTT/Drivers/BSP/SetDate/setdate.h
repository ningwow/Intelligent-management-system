#ifndef __SETDATE_H
#define __SETDATE_H

#include "main.h" // 包含 HAL 库头文件

// 函数声明
extern uint8_t usart1_RX1[512];
// 函数声明
void process_sntp_data(UART_HandleTypeDef *huart, RTC_HandleTypeDef *hrtc);
void send_time_request(UART_HandleTypeDef *huart);


#endif /* __SETDATE_H */

