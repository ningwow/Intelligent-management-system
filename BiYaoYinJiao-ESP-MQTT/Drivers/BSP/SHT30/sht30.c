#include "sht30.h"
#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include <usart.h>

void SendHandCommand(void)
{
    char handCommand[] = "Hand\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)handCommand, strlen(handCommand), HAL_MAX_DELAY);
}


char* ReceiveData(void)
{
    char readCommand[] = "Read\r\n";
    static char receivedData[32];  // ������յ����ݲ�����32�ֽ�
    uint8_t rxByte;
    uint8_t index = 0;

    // ����Read����
    HAL_UART_Transmit(&huart2, (uint8_t*)readCommand, strlen(readCommand), HAL_MAX_DELAY);

    // �ȴ���������
    while (1)
    {
        HAL_UART_Receive(&huart2, &rxByte, 1, HAL_MAX_DELAY);
        receivedData[index++] = rxByte;

        // ����Ƿ���յ��������ַ�������\r\n��β��
        if (rxByte == '\n')
        {
            receivedData[index] = '\0';  // �ַ���������
            break;
        }
    }

    return receivedData;
}

void ParseReceivedData(char* data, float* temperature, float* humidity)
{
    // �������ݸ�ʽΪ "R:xxx.xRH yyy.yC\r\n"
    sscanf(data, "R:%fRH %fC", humidity, temperature);
}
