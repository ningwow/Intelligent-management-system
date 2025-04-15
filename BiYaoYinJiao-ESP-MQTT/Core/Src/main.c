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
    //extern uint8_t time_received;
    
    
    
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
   // int Alarm_process(char* time);          //预约模式下设置闹钟    



/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
      
//      uint8_t tt,mm;    ///调试用
//      char nn[20];
//      char *oo;


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

/*
    
*/
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1,usart1_RX1,sizeof(usart1_RX1)); //启动DMA接收，第三个参数是指最多能接收的字节数量
	__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);                    //关闭DMA过半中断
  
    HAL_Delay(500);
	ESP8266_Init();                                //初始化ESP8266模块,连接WIFI
	OLED_ShowStr(0,48,(uint8_t *)"W",2);
	OLED_RefreshRAM();     //更新缓冲区
    
    //获取实时时间
    send_time_request(&huart1);
    OLED_ShowStr(0,48,(uint8_t *)"WT",2);
    HAL_Delay(100);
    
    Aliyun_Connect();                              //连接阿里云服务器                                  //清除屏幕内容
	OLED_ShowStr(0,48,(uint8_t *)"WTA",2);
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
    
    RTC_AlarmTypeDef sAlarm;    //获取已设置的闹钟时间
      char alarm_str[10];

	
	/*  初始化引脚状态为高电平。使能TB6612 电机驱动模块*/
    HAL_GPIO_WritePin(DJ_EN_GPIO_Port, DJ_EN_Pin, GPIO_PIN_SET);
    //tim3 channel2 调试，其他正常，就CHANNEL2不正常输出
//    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,150);
//    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,150);
	__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,150);
    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,150);
    HAL_Delay(500);
    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,0);
    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,0);
	
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      
    if(++post_time_count >= 30)        //50大概1.5秒，根据里面的延迟计算        //定时上传数据，更改上传频率可适当改小此阈值，
    {
        
        post_time_count = 0;

                
        //#######################################
        //BH1750光照传感器感知数据
        Lux = 0;
        
        Lux = lightSensorLux();
        Lux = Lux/10;
        //printf("light:%d Lux\n",Lux);
        sprintf(display_Lux, "Lux:%2d%%", Lux);
        

              
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
        //注意时间的BIN格式和BCD格式区分，两种格式的解析方式不同，以下方式为BIN格式读取的解析
        HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
        
//        int year = sDate.Year;
//        int month = sDate.Month;
//        int date = sDate.Date;
//        int hours = sTime.Hours;
//        int min = sTime.Minutes;
        // 将 BIN 格式的日期和时间转换为字符串
        snprintf(dateStr, sizeof(dateStr), "20%2u-%2u-%2u", sDate.Year, sDate.Month, sDate.Date);      //01d%标识宽度为1，和下方showstr函数对应
        snprintf(timeStr, sizeof(timeStr), "%2u:%2u", sTime.Hours, sTime.Minutes);  //时分秒："%01d:%01d:%01d", sTime.Hours, sTime.Minutes, sTime.Seconds
//        snprintf(dateStr, sizeof(dateStr), "%02hhu-%02hhu-%02hhu", sDate.Year, sDate.Month, sDate.Date);
//        snprintf(timeStr, sizeof(timeStr), "%02hhu:%02hhu", sTime.Hours, sTime.Minutes);
        // 在 OLED 上显示日期和时间
        //OLED_ShowChinese(0,0,(unsigned char *)"时间");	//必须是中文的：
        OLED_ShowStr(0, 0, (unsigned char *)"Timer:", 2);
        OLED_ShowStr(60, 0, (unsigned char *)dateStr, 1);  // 在 OLED 上显示日期，TextSize 为 1
        OLED_ShowStr(72, 9, (unsigned char *)timeStr, 1); 	// 在 OLED 上显示时间，TextSize 为 1 秒是乱的，但分钟准确
        OLED_ShowStr(0, 16, (unsigned char*)display_str, 2);	//显示温湿度数据
        OLED_ShowStr(0, 32, (unsigned char*)display_Lux, 2);	//显示光照数据  
//                 HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
//                snprintf(alarm_str, sizeof(alarm_str),"%02u:%02u",sAlarm.AlarmTime.Hours,sAlarm.AlarmTime.Minutes);
//                OLED_ShowStr(64, 48, (unsigned char*)alarm_str, 2);
                
                
//                snprintf(nn, sizeof(nn),"%02u:%02u",tt,mm);
//                OLED_ShowStr(0, 48, (unsigned char*)nn, 2);     //调试
                
        switch(mode){
            case 0: OLED_ShowStr(64, 32, (unsigned char*)"HAND", 2);break;
            case 1: OLED_ShowStr(64, 32, (unsigned char*)"AUTO", 2);break;
            case 2: {
                OLED_ShowStr(64, 32, (unsigned char*)"TIME", 2);
                HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
                snprintf(alarm_str, sizeof(alarm_str),"%02u:%02u",sAlarm.AlarmTime.Hours,sAlarm.AlarmTime.Minutes);
                OLED_ShowStr(64, 48, (unsigned char*)alarm_str, 2);
                break; }
        }
		//OLED_ClearRAM();    //清除缓冲区
        OLED_RefreshRAM();     //更新缓冲区
        
        memset(dateStr, 0, sizeof(dateStr));  // 清空整个字符串数组
        memset(timeStr, 0, sizeof(timeStr));  // 清空整个字符串数组
        memset(display_str, 0, sizeof(display_str));  // 清空整个字符串数组
        memset(display_Lux, 0, sizeof(display_Lux));  // 清空整个字符串数组
        //printf("oled ok \n\r");
        //printf("%s\n%s\n\r",dateStr,timeStr);
        
        
        //###############################
        //上传温度，湿度，光照
        Send_Dat_2_Aliyun(temperature, humidity, Lux);
        
        //###############################
        //自动阈值反应操作
        if(mode == 1){

            if(Lux < Lux_yuzhi-10){  //默认30
                led_set(199);
                HAL_Delay(1);
            }else if(Lux < Lux_yuzhi){
                led_set(99);
                HAL_Delay(1);
            }else if(Lux > Lux_yuzhi){
                led_set(0);
                HAL_Delay(1);
            }

            if(temperature > temp_yuzhi+10){   //默认20.0
                DJ_set(1,1,59); //空调，冷风，开大，
                HAL_Delay(1);
            }else if(temperature > temp_yuzhi){
                DJ_set(1,1,29);
                HAL_Delay(1);
            }else if(temperature < temp_yuzhi-10){
                DJ_set(1,2,29);
                HAL_Delay(1);
            }else if(temperature < temp_yuzhi){
                DJ_set(1,2,59);
                HAL_Delay(1);
            }
            
            //这里没有加湿器，湿度阈值没有意义。
//            if(humidity > hum_yuzhi){   //默认20.0
//                
//            }
        
        }
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
        //处理下发的数据，并做出相应操作。
        process_message((char*)usart1_RX1);     //该函数逻辑测试正常

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
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
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
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
		
		//char *pos_test = strstr((char*)usart1_RX1, "+MQTTSUBRECV");           //阿里云下发指令由+MQTTSUBRECV开头，由此判断此数据应该调用那个回调函数
		char *pos_test = strstr((char*)usart1_RX1, "set");              //这里改为set是因为，数据上报还会有返回的ack确认信息，会造成解析拥堵，最终无法控制，只是用set解析控制命令的数据，解决这一问题
        if(pos_test != NULL){
			uart1_rec_flag = 1;
		}
        
        // 检查 NTP 数据，若存在+IPD内容，则说明此数据是NTP时间，调用时间设置回调函数
//        char *ntp_pos = strstr((char*)usart1_RX1, "+IPD");
//        if (ntp_pos != NULL) {
//            char* ntp_data = strchr(ntp_pos, ':') + 1;
//            uint32_t epoch_time = parse_ntp_time((uint8_t*)ntp_data);
//            if (epoch_time > 0) {
//                set_rtc_time(&hrtc, epoch_time); // 注意：hrtc 需外部传入
//                time_received = 1;
//            }
//        }
        
        
        // 检查 SNTP 数据,最终选用SNTP方式获取实时时间。
        char *sntp_pos = strstr((char*)usart1_RX1, "+CIPSNTPTIME:");
        if (sntp_pos != NULL) {
            process_sntp_data(&huart1, &hrtc);
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



//process_message 函数
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
    
    sscanf(params[0].value,"%d",&mode); //赋值mode

    // 根据参数值执行逻辑
    //mode参数处理
    if (strcmp(params[0].value, "0") == 0) {  // mode:0 手动
        //处理手动模式下需要操作的内容
        int num[4]={-1,-1,-1,-1};
        sscanf(params[2].value,"%d",&num[0]);
        sscanf(params[3].value,"%d",&num[1]);
        sscanf(params[4].value,"%d",&num[2]);
        sscanf(params[5].value,"%d",&num[3]);
        

        //程序测试正常，
        //空调控制,方向，速度 ，两位数，第一位是方向，1为正，>1为反。第二位是转速
        if(num[0] != -1){
            int fx = num[0]/10; //方向，求第一个数
            if(fx >=2) fx = 2;  //不能用0做判断，在解析数据时，06的数据会被解析为6
            else if(fx <=1) fx = 1;
            int zs = 10*(num[0]%10); //转速，求第二个数
            //电机编号，0/1/2停止，正传，反转，转速
            DJ_set(1,fx,zs); //第1个，正传，0-99
            HAL_Delay(1);
        }
        //程序测试正常
        //窗帘控制，上，上半，下，下半，0/1/2/3
        if(num[1] != -1){
            if(num[1] == 0){    //上
                DJ_set(2,1,49);
                HAL_Delay(500);
                DJ_set(2,1,0); //关闭
                HAL_Delay(1);
            }else if(num[1] == 1){ //上半
                DJ_set(2,1,49); 
                HAL_Delay(250);
                DJ_set(2,1,0);//关闭
                HAL_Delay(1);
            }else if(num[1] == 2){  //下
                DJ_set(2,2,49); 
                HAL_Delay(500);
                DJ_set(2,2,0); //关闭
                HAL_Delay(1);
            }else if(num[1] == 3){  //下半
                DJ_set(2,2,49); 
                HAL_Delay(250);
                DJ_set(2,2,0); //关闭
                HAL_Delay(1);
            }
        }
        //程序测试正常
        //窗户控制，开，中，关,0/1/2
        if(num[2] != -1){
            if(num[2] == 0){
                num[2] = 4;
            }else if(num[2] == 1){
                num[2] = 12;
            }else if(num[2] == 2){
                num[2] = 24;
            }
            duoji_set(1,num[2]); //第一个，0-199，有效范围4-24,对应2.5%-12.5% 的pwm
            HAL_Delay(1);
        }
        
        
        //灯光控制 ，亮度0-99
        if(num[3] != -1){
            num[3]*=2;
            if(num[3] > 199) num[3] = 199;
            else if(num[3] < 0 ) num[3] = 0;
            //duoji_set(2,num[3]); //第二个，4-24
            led_set(num[3]);
            //duoji_set(2,150); //第二个，4-24
            HAL_Delay(1);
        }
    }
    else if (strcmp(params[0].value, "1") == 0) {  // mode:1 自动阈值
        //自动模式下需要设置的内容
        
        sscanf(params[6].value,"%f",&temp_yuzhi);
        //sscanf(params[7].value,"%f",&hum_yuzhi);  //湿度阈值没有意义，因为没有对应的执行器。
        sscanf(params[8].value,"%d",&Lux_yuzhi);

//    temp_yuzhi = atof(params[6].value);
//    hum_yuzhi  = atof(params[7].value);
//    Lux_yuzhi  = atoi(params[8].value);

        
    }else if (strcmp(params[0].value, "2") == 0) {  // mode:2 预约模式
        //处理预约模式下需要操作的内容
        //设置预约闹钟数据的处理和设置，并设置中断触发预约启动。
         uint8_t hour = 0,min = 0;
        sscanf(params[1].value, "\"%hhu:%hhu\"", &hour, &min);     // params[1].value = "13:23" 带双引号
//         tt = hour;
//        mm = min;
        //oo    = params[1].value;
        
         __HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);  //清除之前的闹钟标志

    
        if(hour != 0 && min != 0){
            //将预约数据设置为闹钟时间，作为预约触发的时间。
            RTC_AlarmTypeDef sAlarm = {0};
            sAlarm.AlarmTime.Hours = hour;
            sAlarm.AlarmTime.Minutes = min;
            sAlarm.AlarmTime.Seconds = 0;  // 秒可以根据需要设置
            sAlarm.Alarm = RTC_ALARM_A;     //设置闹钟ID
            
            //中断方式设置闹钟。 
            HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN);
        }
       
        HAL_Delay(1);
        
    }
    memset(usart1_RX1, 0, sizeof(usart1_RX1));  //检测结束，赋值为空，防止重复检测

}

/*
闹钟设定函数，接收上位机下发的定时数据，处理并设置给闹钟。
*/
//int Alarm_process(char* time){
//    // 提取小时和分钟
//    uint8_t hour = 0xFF,min = 0xFF;
//    //uint8_t hour = 20,min = 11;
//     //char alarm_str[10];
//    sscanf(time, "%hhu:%hhu", &hour, &min);
//             tt = hour;
//        mm = min;
//    
//    
//    __HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);  //清除之前的闹钟标志

//    
//    if(hour != 0xFF && min != 0xFF){
//        //将预约数据设置为闹钟时间，作为预约触发的时间。
//        RTC_AlarmTypeDef sAlarm = {0};
//        sAlarm.AlarmTime.Hours = hour;
//        sAlarm.AlarmTime.Minutes = min;
//        sAlarm.AlarmTime.Seconds = 0;  // 秒可以根据需要设置
//        sAlarm.Alarm = RTC_ALARM_A;     //设置闹钟ID
//        
//        //中断方式设置闹钟。
//        if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK) {
//            return -1;
//        }
////         snprintf(alarm_str, sizeof(alarm_str),"%02u:%02u",hour,min);
////        OLED_ShowStr(0, 48, (unsigned char*)alarm_str, 2);

//    }

//    return 0;
//}


void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
    // 这里是闹钟触发后的处理逻辑
    //预约模式触发，设置运行模式为自动模式
    mode = 1;
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
