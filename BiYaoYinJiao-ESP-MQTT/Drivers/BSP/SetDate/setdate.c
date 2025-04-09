#include "setdate.h"
#include "string.h"
#include "stdio.h"
#include "time.h"
#include "bsp_aliyun.h" //引用串口发送函数的定义，避免重复定义函数

static uint8_t time_received = 0;

// 发送 AT 指令（使用 Usart1_Send_Str）
void send_at_command(UART_HandleTypeDef *huart, char* cmd, uint32_t timeout) {
    Usart1_Send_Str((uint8_t*)cmd);
    HAL_Delay(timeout);
}

// 设置 RTC 时间
void set_rtc_time(RTC_HandleTypeDef *hrtc, uint32_t epoch_time) {
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    struct tm time_struct;
    time_t raw_time = (time_t)epoch_time;

    time_struct = *localtime(&raw_time);
    sTime.Hours = time_struct.tm_hour;
    sTime.Minutes = time_struct.tm_min;
    sTime.Seconds = time_struct.tm_sec;
    HAL_RTC_SetTime(hrtc, &sTime, RTC_FORMAT_BIN);

    sDate.Year = time_struct.tm_year - 100;
    sDate.Month = time_struct.tm_mon + 1;
    sDate.Date = time_struct.tm_mday;
    sDate.WeekDay = time_struct.tm_wday == 0 ? 7 : time_struct.tm_wday;
    HAL_RTC_SetDate(hrtc, &sDate, RTC_FORMAT_BIN);
}

// 解析 NTP 数据
uint32_t parse_ntp_time(uint8_t* buffer) {
    if (strlen((char*)buffer) >= 48) {
        uint32_t ntp_seconds = (buffer[40] << 24) | (buffer[41] << 16) | 
                              (buffer[42] << 8) | buffer[43];
        return ntp_seconds - 2208988800UL + (8 * 3600); // UTC+8
    }
    return 0;
}

// 主函数：从 NTP 获取时间并设置 RTC
void set_date_from_ntp(UART_HandleTypeDef *huart, RTC_HandleTypeDef *hrtc, uint8_t *rx_buffer, uint16_t buffer_size) {
    // 重置接收状态
    time_received = 0;

    // 建立 TCP 连接到 NTP 服务器
    send_at_command(huart, "AT+CIPSTART=\"TCP\",\"ntp.aliyun.com\",123\r\n", 2000);

    // 发送 NTP 请求
    char ntp_packet[48] = {0x1B, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // NTP v3 请求
    char cip_send[20];
    sprintf(cip_send, "AT+CIPSEND=%d\r\n", 48);
    send_at_command(huart, cip_send, 1000);
    HAL_UART_Transmit(huart, (uint8_t*)ntp_packet, 48, 100);

    // 启动 DMA 接收
    HAL_UARTEx_ReceiveToIdle_DMA(huart, rx_buffer, buffer_size);
    __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT); // 关闭 DMA 过半中断

    // 等待时间接收完成
    uint32_t timeout = HAL_GetTick() + 5000; // 5秒超时
    while (!time_received && HAL_GetTick() < timeout) {
        HAL_Delay(100);
    }

    if (!time_received) {
        char error[] = "NTP Time Failed\r\n";
        Usart1_Send_Str((uint8_t*)error);
    }
}

//// UART 接收事件回调（需在 main.c 中调用）
//void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
//    if (huart->Instance == USART1) {
//        char *pos_test = strstr((char*)usart1_RX1, "+MQTTSUBRECV");
//        if (pos_test != NULL) {
//            uart1_rec_flag = 1; // 保留原有 MQTT 逻辑
//        }

//        // 检查 NTP 数据
//        char *ntp_pos = strstr((char*)usart1_RX1, "+IPD");
//        if (ntp_pos != NULL) {
//            char* ntp_data = strchr(ntp_pos, ':') + 1;
//            uint32_t epoch_time = parse_ntp_time((uint8_t*)ntp_data);
//            if (epoch_time > 0) {
//                set_rtc_time(&hrtc, epoch_time); // 注意：hrtc 需外部传入
//                time_received = 1;
//            }
//        }

//        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, usart1_RX1, sizeof(usart1_RX1));
//        __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
//    }
//}

// 发送字符串函数（已有）调用了其他文件中的相同函数，这里不在重复设置
//void Usart1_Send_Str(uint8_t *buf) {
//    if (buf == NULL) return;
//    HAL_UART_Transmit(&huart1, buf, strlen((const char*)buf), HAL_MAX_DELAY);
//}
