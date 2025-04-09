#include "setdate.h"
#include "string.h"
#include "stdio.h"
#include "time.h"
#include "bsp_aliyun.h" //���ô��ڷ��ͺ����Ķ��壬�����ظ����庯��

static uint8_t time_received = 0;

// ���� AT ָ�ʹ�� Usart1_Send_Str��
void send_at_command(UART_HandleTypeDef *huart, char* cmd, uint32_t timeout) {
    Usart1_Send_Str((uint8_t*)cmd);
    HAL_Delay(timeout);
}

// ���� RTC ʱ��
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

// ���� NTP ����
uint32_t parse_ntp_time(uint8_t* buffer) {
    if (strlen((char*)buffer) >= 48) {
        uint32_t ntp_seconds = (buffer[40] << 24) | (buffer[41] << 16) | 
                              (buffer[42] << 8) | buffer[43];
        return ntp_seconds - 2208988800UL + (8 * 3600); // UTC+8
    }
    return 0;
}

// ���������� NTP ��ȡʱ�䲢���� RTC
void set_date_from_ntp(UART_HandleTypeDef *huart, RTC_HandleTypeDef *hrtc, uint8_t *rx_buffer, uint16_t buffer_size) {
    // ���ý���״̬
    time_received = 0;

    // ���� TCP ���ӵ� NTP ������
    send_at_command(huart, "AT+CIPSTART=\"TCP\",\"ntp.aliyun.com\",123\r\n", 2000);

    // ���� NTP ����
    char ntp_packet[48] = {0x1B, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // NTP v3 ����
    char cip_send[20];
    sprintf(cip_send, "AT+CIPSEND=%d\r\n", 48);
    send_at_command(huart, cip_send, 1000);
    HAL_UART_Transmit(huart, (uint8_t*)ntp_packet, 48, 100);

    // ���� DMA ����
    HAL_UARTEx_ReceiveToIdle_DMA(huart, rx_buffer, buffer_size);
    __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT); // �ر� DMA �����ж�

    // �ȴ�ʱ��������
    uint32_t timeout = HAL_GetTick() + 5000; // 5�볬ʱ
    while (!time_received && HAL_GetTick() < timeout) {
        HAL_Delay(100);
    }

    if (!time_received) {
        char error[] = "NTP Time Failed\r\n";
        Usart1_Send_Str((uint8_t*)error);
    }
}

//// UART �����¼��ص������� main.c �е��ã�
//void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
//    if (huart->Instance == USART1) {
//        char *pos_test = strstr((char*)usart1_RX1, "+MQTTSUBRECV");
//        if (pos_test != NULL) {
//            uart1_rec_flag = 1; // ����ԭ�� MQTT �߼�
//        }

//        // ��� NTP ����
//        char *ntp_pos = strstr((char*)usart1_RX1, "+IPD");
//        if (ntp_pos != NULL) {
//            char* ntp_data = strchr(ntp_pos, ':') + 1;
//            uint32_t epoch_time = parse_ntp_time((uint8_t*)ntp_data);
//            if (epoch_time > 0) {
//                set_rtc_time(&hrtc, epoch_time); // ע�⣺hrtc ���ⲿ����
//                time_received = 1;
//            }
//        }

//        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, usart1_RX1, sizeof(usart1_RX1));
//        __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
//    }
//}

// �����ַ������������У������������ļ��е���ͬ���������ﲻ���ظ�����
//void Usart1_Send_Str(uint8_t *buf) {
//    if (buf == NULL) return;
//    HAL_UART_Transmit(&huart1, buf, strlen((const char*)buf), HAL_MAX_DELAY);
//}
