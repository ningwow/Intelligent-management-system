#include "setdate.h"
#include "string.h"
#include "stdio.h"
//#include "time.h"
#include "oled.h"
#include "bsp_aliyun.h" //���ô��ڷ��ͺ����Ķ��壬�����ظ����庯��

uint8_t time_received = 0;

// ���� AT ָ�ʹ�� Usart1_Send_Str��
void send_at_command(UART_HandleTypeDef *huart, char* cmd, uint32_t timeout) {
    Usart1_Send_Str((uint8_t*)cmd);
    HAL_Delay(timeout);
}

//// ���� RTC ʱ��
//void set_rtc_time(RTC_HandleTypeDef *hrtc, uint32_t epoch_time) {
//    RTC_TimeTypeDef sTime = {0};
//    RTC_DateTypeDef sDate = {0};
//    struct tm time_struct;
//    time_t raw_time = (time_t)epoch_time;

//    time_struct = *localtime(&raw_time);
//    sTime.Hours = time_struct.tm_hour;
//    sTime.Minutes = time_struct.tm_min;
//    sTime.Seconds = time_struct.tm_sec;
//    HAL_RTC_SetTime(hrtc, &sTime, RTC_FORMAT_BIN);

//    sDate.Year = time_struct.tm_year - 100;
//    sDate.Month = time_struct.tm_mon + 1;
//    sDate.Date = time_struct.tm_mday;
//    sDate.WeekDay = time_struct.tm_wday == 0 ? 7 : time_struct.tm_wday;
//    HAL_RTC_SetDate(hrtc, &sDate, RTC_FORMAT_BIN);
//}


// ���� SNTP ���ݲ����� RTC
void process_sntp_data(UART_HandleTypeDef *huart, RTC_HandleTypeDef *hrtc) {
    char *sntp_pos = strstr((char*)usart1_RX1, "+CIPSNTPTIME:");
    if (sntp_pos != NULL) {
        sntp_pos += 13; // ���� "+CIPSNTPTIME:"
//        char weekday[4], month_str[4];
//        uint8_t day, hour, min, sec, year;
        char weekday[4], month_str[4];
        uint8_t day, hour, min, sec;
        uint16_t year; // ��ݿ��ܳ��� 255������ uint16_t

        
       // "%3s %3s %2u %2u:%2u:%2u %4u"
        // ������ʽ��Wed Apr  9 18:48:20 2025
        sscanf(sntp_pos, "%3s %3s %hhu %hhu:%hhu:%hhu %hu", 
               weekday, month_str, &day, &hour, &min, &sec, &year);
        //��������ʱ��ȫ���������Ѿ������ԡ�

        // ʹ�� strtok �ָ��ַ���
//        char *token = strtok(sntp_pos, " ");
//        if (token) strcpy(weekday, token);      // Wed

//        token = strtok(NULL, " ");
//        if (token) strcpy(month_str, token);    // Apr

//        token = strtok(NULL, " ");
//        if (token) day = atoi(token);           // 9

//        token = strtok(NULL, ":");              // 19:10:57���ָ�Сʱ
//        if (token) hour = atoi(token);          // 19

//        token = strtok(NULL, ":");              // 10:57���ָ����
//        if (token) min = atoi(token);           // 10

//        token = strtok(NULL, " ");              // 57���ָ���
//        if (token) sec = atoi(token);           // 57

//        token = strtok(NULL, " ");              // 2025
//        if (token) year = atoi(token);          // 2025
        
        // �·�ת��
        const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        uint8_t month = 0;
        for (uint8_t i = 0; i < 12; i++) {
            if (strcmp(month_str, months[i]) == 0) {
                month = i + 1;
                break;
            }
        }

        // ����ת��
        const char* weekdays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
        uint8_t wday = 0;
        for (uint8_t i = 0; i < 7; i++) {
            if (strcmp(weekday, weekdays[i]) == 0) {
                wday = i + 1; // STM32 RTC ���ڴ� 1-7
                break;
            }
        }

        // ���� RTC
        RTC_TimeTypeDef sTime = {0};
        RTC_DateTypeDef sDate = {0};

        sTime.Hours = hour;
        sTime.Minutes = min;
        sTime.Seconds = sec;

        //sDate.Year = year - 2000; // STM32 RTC ��ݴ� 2000 ��ʼ
        year-=2000;
        sDate.Year = (uint8_t)year; // STM32 RTC ��ݴ� 2000 ��ʼ
        sDate.Month = month;
        sDate.Date = day;
        sDate.WeekDay = wday;

        HAL_RTC_SetTime(hrtc, &sTime, RTC_FORMAT_BIN);
        HAL_RTC_SetDate(hrtc, &sDate, RTC_FORMAT_BIN);
        
        //�����������������Ƿ���ȷ
        char Str[20];
        char Str1[20];
        snprintf(Str, sizeof(Str), "%2u-%2u-%2u", year, month, day);

        snprintf(Str1, sizeof(Str), "%2u:%2u:%2u %2u",hour, min, sec, wday);
        OLED_ShowStr(36, 32, (unsigned char*)Str, 2);
        OLED_ShowStr(36, 48, (unsigned char*)Str1, 2);
//        HAL_Delay(200);
//        
        time_received = 1; // ���ñ�־λ��ʾ�ɹ�
    }
}

// ���� NTP ����
//uint32_t parse_ntp_time(uint8_t* buffer) {
//    if (strlen((char*)buffer) >= 48) {
//        uint32_t ntp_seconds = (buffer[40] << 24) | (buffer[41] << 16) | 
//                              (buffer[42] << 8) | buffer[43];
//        return ntp_seconds - 2208988800UL + (8 * 3600); // UTC+8
//    }
//    return 0;
//}

// ���� SNTP ʱ������
void send_time_request(UART_HandleTypeDef *huart) {
    //time_received = 0;

    // ���� SNTP��ʱ�� UTC+8
    send_at_command(huart, "AT+CIPSNTPCFG=1,8\r\n", 1000);
    HAL_Delay(7000);    //��Ҫһ��ʱ����ܻ�ȡ��ʱ��

    // ��ѯ SNTP ʱ��
    send_at_command(huart, "AT+CIPSNTPTIME?\r\n", 1000);

//    // ���� DMA ����
//    HAL_UARTEx_ReceiveToIdle_DMA(huart, usart1_RX1, sizeof(usart1_RX1));
//    __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
}



//void set_date_from_ntp(UART_HandleTypeDef *huart, RTC_HandleTypeDef *hrtc) {
//    time_received = 0;

//    send_at_command(huart, "AT+CIPSTART=\"UDP\",\"ntp.aliyun.com\",123\r\n", 2000);

//    char ntp_packet[48] = {0x1B, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//    char cip_send[20];
//    sprintf(cip_send, "AT+CIPSEND=%d\r\n", 48);
//    send_at_command(huart, cip_send, 1000);
//    HAL_UART_Transmit(huart, (uint8_t*)ntp_packet, 48, 100);

////    HAL_UARTEx_ReceiveToIdle_DMA(huart, usart1_RX1, sizeof(usart1_RX1));
////    __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);

//    uint32_t timeout = HAL_GetTick() + 5000;
//    while (!time_received && HAL_GetTick() < timeout) {
//        HAL_Delay(100);
//    }

//    if (!time_received) {
//        //char error[] = "NTP Time Failed\r\n";
//        OLED_ShowStr(28,64,(uint8_t *)"ERR",2);
//    }
//}

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
