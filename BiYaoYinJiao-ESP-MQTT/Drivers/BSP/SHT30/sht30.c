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
    static char receivedData[32];  // 假设接收的数据不超过32字节
    uint8_t rxByte;
    uint8_t index = 0;

    // 发送Read命令
    HAL_UART_Transmit(&huart2, (uint8_t*)readCommand, strlen(readCommand), HAL_MAX_DELAY);

    // 等待接收数据
    while (1)
    {
        HAL_UART_Receive(&huart2, &rxByte, 1, HAL_MAX_DELAY);
        receivedData[index++] = rxByte;

        // 检查是否接收到完整的字符串（以\r\n结尾）
        if (rxByte == '\n')
        {
            receivedData[index] = '\0';  // 字符串结束符
            break;
        }
    }

    return receivedData;
}

void ParseReceivedData(char* data, float* temperature, float* humidity)
{
    // 假设数据格式为 "R:xxx.xRH yyy.yC\r\n"
    sscanf(data, "R:%fRH %fC", humidity, temperature);
}
