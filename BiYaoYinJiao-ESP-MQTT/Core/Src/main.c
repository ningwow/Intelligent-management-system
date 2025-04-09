/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "stdio.h"
#include "bh1750.h"
#include "oled.h"
#include "pwm-dj.h"
#include "pwm_duoji.h"
#include "sht30.h"
#include "bsp_aliyun.h"
#include "setdate.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
    uint8_t usart1_RX1[512] = {0};                   //串口3接收数据缓冲区
    uint8_t uart1_rec_flag = 0;                      //串口3接收完成中断
    
    
    
    
    int mode = 0;
    //手动控制模式 0
    //自动阈值模式 1
    //预约时间阈值模式 2


    //控制模块全局变量
    //电机空调（风扇TI_CH1）	舵机窗帘（直流电机马达TI_CH2）
    int ktnum = 0;			int clnum = 0;
    //舵机窗户（SG90舵机T3_CH1）	电机Led灯光(T3_CH2)
    int chnum = 0;			int lednum = 0;

    //默认阈值全局变量
    float temp_yuzhi = 20.0;	//温度全局变量
    float hum_yuzhi = 20.0;	//湿度全局变量
    int Lux_yuzhi = 30;		//光照阈值
    
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
//int find_param(char* str, char* keyword);
    
    char* find_param(char* str, ParamExtract* param);   //寻找参数值的函数
    void process_message(char* message);    //接收数据处理函数
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
    
    uint16_t post_time_count = 0;                  //定时任务变量
    

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */


  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  //初始化一定要放在后面
	SET_L_mode();	//设置光照 低分辨率连续模式
    HAL_Delay(5);
	DJ_init();
    HAL_Delay(5);
	duoji_init();
    HAL_Delay(5);	//防止oled初始化不正确，比如初始化为反转180度
  	OLED_Init();
    
//	//初始化温湿度传感器
	SendHandCommand();  // 温湿度 sht30发送Hand命令
    HAL_Delay(5);
    
    
    OLED_ShowStr(28,16,(uint8_t *)"enable...",2);    
	OLED_RefreshRAM();     //更新缓冲区
/*
    
*/
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1,usart1_RX1,sizeof(usart1_RX1)); //启动DMA接收，第三个参数是指最多能接收的字节数量
	__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);                    //关闭DMA过半中断
  
    HAL_Delay(1000);
	ESP8266_Init();                                //初始化ESP8266模块,连接WIFI
	OLED_ShowStr(28,48,(uint8_t *)"!",2);
	OLED_RefreshRAM();     //更新缓冲区
    
    //获取实时时间
    
    
    
    
    Aliyun_Connect();                              //连接阿里云服务器                                  //清除屏幕内容
	OLED_ShowStr(28,48,(uint8_t *)"!!",2);
    HAL_Delay(100);
    OLED_CLS();
    OLED_ClearRAM();    //清除缓冲区
	OLED_RefreshRAM();     //更新缓冲区
    
    
    OLED_ShowStr(28,16,(uint8_t *)"Aliyun is",2);    
	OLED_ShowStr(28,32,(uint8_t *)"connected",2);
	OLED_RefreshRAM();     //更新缓冲区
    HAL_Delay(500);
    
    OLED_CLS();
    OLED_ClearRAM();    //清除缓冲区
    OLED_RefreshRAM();     //更新缓冲区
  
	//RTC 时间变量
	char dateStr[20];
    char timeStr[20];
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
	
	//温度和湿度变量
	float humidity = 0.0;
    float temperature = 0.0;
    char display_str[32];  // 用于存储拼接后的字符串
	
	//光照数据
	int Lux;
	char display_Lux[20];
    

	
	
	/*  初始化引脚状态为高电平。使能TB6612 电机驱动模块*/
    HAL_GPIO_WritePin(DJ_EN_GPIO_Port, DJ_EN_Pin, GPIO_PIN_SET);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      
    if(++post_time_count >= 50)        //50大概1.5秒，根据里面的延迟计算        //定时上传数据，更改上传频率可适当改小此阈值，
    {
        
        post_time_count = 0;

                
        //#######################################
        //BH1750光照传感器感知数据
        Lux = lightSensorLux();
        //printf("light:%d Lux\n",Lux);
        sprintf(display_Lux, "Lux:%d", Lux);
        

              
        /** #####################################
        温湿度数据，使用串口通信的SHT30模块
        */
              //上电后没有温湿度数据，按复位键就好，推测为5v和3.3v共地问题，重新启动单片机使信号地共地即可
        HAL_Delay(5);
        char* data = ReceiveData();  // 发送Read命令并接收数据
          //printf("%s\n",data);
        ParseReceivedData(data, &temperature, &humidity);  // 解析数据
        sprintf(display_str, "T:%.1fC H:%.1f%%", temperature, humidity);

                
              
        /** #####################################
        OLED show fun	128x64
        汉字：16x16
        英文：8x16	6x8
        */
        HAL_Delay(5);
        HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
        HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD);
        // 将 BIN 格式的日期和时间转换为字符串
        snprintf(dateStr, sizeof(dateStr), "202%01d-%01d-%01d", sDate.Year, sDate.Month, sDate.Date);      //01d%标识宽度为1，和下方showstr函数对应
        snprintf(timeStr, sizeof(timeStr), "%01d:%01d", sTime.Hours, sTime.Minutes);  //时分秒："%01d:%01d:%01d", sTime.Hours, sTime.Minutes, sTime.Seconds
        // 在 OLED 上显示日期和时间
        //OLED_ShowChinese(0,0,(unsigned char *)"时间");	//必须是中文的：
        OLED_ShowStr(0, 0, (unsigned char *)"Timer:", 2);
        OLED_ShowStr(60, 0, (unsigned char *)dateStr, 1);  // 在 OLED 上显示日期，TextSize 为 1
        OLED_ShowStr(60, 9, (unsigned char *)timeStr, 1); 	// 在 OLED 上显示时间，TextSize 为 1 秒是乱的，但分钟准确
        OLED_ShowStr(0, 16, (unsigned char*)display_str, 2);	//显示温湿度数据
        OLED_ShowStr(0, 32, (unsigned char*)display_Lux, 2);	//显示光照数据  
        OLED_RefreshRAM();     //更新缓冲区
        
        memset(dateStr, 0, sizeof(dateStr));  // 清空整个字符串数组
        memset(timeStr, 0, sizeof(timeStr));  // 清空整个字符串数组
        memset(display_str, 0, sizeof(display_str));  // 清空整个字符串数组
        memset(display_Lux, 0, sizeof(display_Lux));  // 清空整个字符串数组
        //printf("oled ok \n\r");
        //printf("%s\n%s\n\r",dateStr,timeStr);
        
        
        
        //上传温度，湿度，光照
        Send_Dat_2_Aliyun(temperature, humidity, Lux);
     }
  
	  
/** #####################################
控制电机和舵机,19有点转不动，29吧，刚好
不要长时间驱动，TB6612撑不住，发热就会卡顿
*/
//HAL_Delay(5);	//这些delay函数及其重要，是为了确保上一步控制完成，
//	  DJ_set(0,1,39);	// 0/1/电机，0/1/2/停车/正转/反转，0-99速度
//HAL_Delay(5);	//必须加delay，不然会出现，控制无效的现象，就是应为没有给延迟，让上一步操作进行完成。
//	  DJ_set(1,1,39);
//HAL_Delay(5); 		//类似于完成后退出执行delay，然后重新进入下一个控制，防止连续的进行控制，可能会导致只有最后一个控制有效
//	  duoji_set(15);	//范围 4-24	，定时器3通道1，PA6
//	  //printf("djdj ok \n\r");
//	
    
    if(uart1_rec_flag != 0)
    {
        uart1_rec_flag = 0;
        process_message((char*)usart1_RX1);
//        //找到报文中关键字的位置
//        char *pos_test_ledsta = strstr((char*)usart1_RX1, "mode"); 
//        if(pos_test_ledsta != NULL){               //寻找关键词，然后抓取对应关键词的数据
//            mode= find_param((char*)usart1_RX1, (char*)"mode"); //find_param只返回int类型
//        }
//        
//        if(mode == 1){
//            DJ_set(0,1,39);
//            HAL_Delay(1);
//            DJ_set(1,1,39);
//            HAL_Delay(1);
//            
////            //窗户控制
////            duoji_set(1,4); //第一个，0-199
////            //灯光控制
////            duoji_set(2,4); //第二个，4-24
////            //空调控制
////            DJ_set(1,1,39); //第1个，正传，速度39，0-99
////            //窗帘控制
////            DJ_set(2,1,39); //第2个，正传，速度39

//        }else if(mode == 2){
//            DJ_set(0,1,0);
//            HAL_Delay(1);
//            DJ_set(1,1,0);
//            HAL_Delay(1);
//        }
//        
//        //这里根据确定的值做相应操作
        
        
        //这里返回控制过后的状态
        Send_led_dat(mode);            //将数据上传至阿里云
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

    void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart->Instance == USART1)
	{
		//HAL_UART_Transmit(&huart1,usart1_RX1,Size,10);                        //串口1打印接受的数据
		
		char *pos_test = strstr((char*)usart1_RX1, "+MQTTSUBRECV");           //阿里云下发指令由+MQTTSUBRECV开头，由此判断此数据应该调用那个回调函数
		if(pos_test != NULL){
			uart1_rec_flag = 1;
		}
        
        // 检查 NTP 数据，若存在+IPD内容，则说明此数据是NTP时间，调用时间设置回调函数
        char *ntp_pos = strstr((char*)usart1_RX1, "+IPD");
        if (ntp_pos != NULL) {
            char* ntp_data = strchr(ntp_pos, ':') + 1;
            uint32_t epoch_time = parse_ntp_time((uint8_t*)ntp_data);
            if (epoch_time > 0) {
                set_rtc_time(&hrtc, epoch_time); // 注意：hrtc 需外部传入
                //time_received = 1;
            }
        }
        
		HAL_UARTEx_ReceiveToIdle_DMA(&huart1,usart1_RX1,sizeof(usart1_RX1));  //重新打开DMA接收
		__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);                     //关闭DMA过半中断
	}
}

// 原函数，只能提取整数类型的参数
//int find_param(char* str, char* keyword) 
//{
//	char* pos = strstr(str, keyword);
//	int num = -1;    //如果没找到，默认返回值-1
//	
//	if (pos != NULL)
//	{
//		pos += strlen(keyword);
//		while (*pos != ':'){ 
//			pos++; //寻找冒号,定位到参数起始位置
//		}
//		
//		pos++;  //跳过冒号
//		char param_value[100];
//		int i = 0;
//		
//		while(*pos != ',' && *pos != '}'){ 
//			param_value[i++] = *pos++; //提取参数,直到遇到逗号或者右花括号
//		}
//		param_value[i] = '\0';
//		num = atoi(param_value);
//	}
//	
//	return num;
//}
// 修改后的 find_param 函数
char* find_param(char* str, ParamExtract* param) 
{
    char* pos = strstr(str, param->keyword);  
    if (pos != NULL)
    {
        pos += strlen(param->keyword);  
        while (*pos != ':' && *pos != '\0') { 
            pos++;  
        }
        
        if (*pos == '\0') return NULL;  
        
        pos++;  
        int i = 0;
        
        while (*pos != ',' && *pos != '}' && *pos != '\0' && i < param->max_len - 1) { 
            param->value[i++] = *pos++;  
        }
        param->value[i] = '\0';  
        return param->value;  
    }
    
    return NULL;  
}


// 重新输出的 process_message 函数
void process_message(char* message) 
{
    // 定义需要提取的参数列表
    Param params[] = {
        //设置需要提取的参数名称
        {"mode", ""},
        {"Time", ""},   //预约时间设置
        {"ktnum", ""},  //手动空调设置
        {"clnum", ""},  //手动窗帘设置
        {"chnum", ""},  //手动窗户设置
        {"lednum", ""}, //手动灯光设置
        {"temp_yuzhi", ""}, //温度阈值设置    默认阈值20.0
        {"hum_yuzhi", ""},  //湿度阈值设置    默认阈值20.0
        {"Lux_yuzhi", ""}  //光照阈值设置     默认阈值30

    };
    int param_count = sizeof(params) / sizeof(params[0]);   //提取的次数

    // 提取所有参数
    for (int i = 0; i < param_count; i++) {
        ParamExtract extract = {params[i].keyword, params[i].value, 100};
        if (find_param(message, &extract) == NULL) {
            params[i].value[0] = '\0';  // 没找到时清空
        }
    }

    // 根据参数值执行逻辑
    //mode参数处理
    if (strcmp(params[0].value, "0") == 0) {  // mode:0 手动
        //处理手动模式下需要操作的内容
        DJ_set(0, 1, 39);
        HAL_Delay(1);
        DJ_set(1, 1, 39);
        HAL_Delay(1);
        
        
    }
    else if (strcmp(params[0].value, "1") == 0) {  // mode:1 自动阈值
        //自动模式下需要设置的内容
        DJ_set(0, 1, 0);
        HAL_Delay(1);
        DJ_set(1, 1, 0);
        HAL_Delay(1);
        
        
        
    }else if (strcmp(params[0].value, "2") == 0) {  // mode:2 预约模式
        //处理预约模式下需要操作的内容
        DJ_set(0, 1, 0);
        HAL_Delay(1);
        DJ_set(1, 1, 0);
        HAL_Delay(1);
        
        
    }

}



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
