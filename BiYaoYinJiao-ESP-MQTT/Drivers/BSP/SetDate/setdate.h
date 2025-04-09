#ifndef __SETDATE_H
#define __SETDATE_H

#include "main.h" // 包含 HAL 库头文件

// 函数声明
void set_date_from_ntp(UART_HandleTypeDef *huart, RTC_HandleTypeDef *hrtc, uint8_t *rx_buffer, uint16_t buffer_size);
uint32_t parse_ntp_time(uint8_t* buffer);
void set_rtc_time(RTC_HandleTypeDef *hrtc, uint32_t epoch_time);

#endif /* __SETDATE_H */

