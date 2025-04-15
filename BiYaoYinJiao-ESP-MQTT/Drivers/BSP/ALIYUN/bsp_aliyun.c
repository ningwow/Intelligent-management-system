#include "bsp_aliyun.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "oled.h"

extern uint8_t usart1_RX1[512], uart3_rec_flag;

uint8_t Usart1_tx_buff[512]; //发送缓存区

void Usart1_Send_Data(uint8_t data)
{
	while(!(USART1->SR & 0x01<<7));
	USART1->DR =data;
}

void Usart1_Send_Str(uint8_t *buf)
{
//	while(*buf != 0)
//	{
//		Usart1_Send_Data(*buf);
//		buf++;
//	}
//    
    if (buf == NULL) return;
    HAL_UART_Transmit(&huart1, buf, strlen((const char*)buf), HAL_MAX_DELAY);
    
}

uint8_t FindStr(const char* dest, const char* src, uint16_t retry_nms)
{
    uint16_t retry_count = retry_nms / 10;  // 计算最大重试次数

    while (retry_count--)
    {
        if (strstr(dest, src) != NULL)  // 直接判断是否找到
        {
            return 1;  // 找到返回 1
        }
        HAL_Delay(10);  // 等待 10ms
    }

    return 0;  // 超时未找到
}


static uint8_t ConnectAP(char* ssid,char* pswd)
{
	uint8_t cnt=2;
	while(cnt--)
	{
        
        memset(usart1_RX1,0,512);
		Usart1_Send_Str((uint8_t *)"ATE0\r\n"); //关闭回显
        HAL_Delay(2000);
        
		memset(usart1_RX1,0,512);
		Usart1_Send_Str((uint8_t *)"AT+CWMODE=1\r\n");              //设置为STATION模式	
		if(FindStr((char*)usart1_RX1,"K",3000) != 0)
		{
			break;
		}             		
	}
	if(cnt == 0) 
		return 0;

	cnt=2;
	while(cnt--)
	{
		memset(usart1_RX1,0,512);                            //清空发送缓冲
		memset(Usart1_tx_buff,0,512);                            //清空接收缓冲
		sprintf((char*)Usart1_tx_buff,"AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,pswd);//连接目标AP
		Usart1_Send_Str(Usart1_tx_buff);        
		if(FindStr((char*)usart1_RX1,"OK",5000)!=0)                      //连接成功且分配到IP
		{
			return 1;
		}
	}
	return 0;
}

void ESP8266_Init(void)
{
	uint8_t flag=0;
	
	memset(usart1_RX1,0,512);
	Usart1_Send_Str((uint8_t *)"AT\r\n"); //检测ESP8266是否存在
    //Usart1_Send_Str((uint8_t *)"AT\r\n"); //检测ESP8266是否存在
//	if(FindStr((char*)usart1_RX1,"OK",2000) != 0){
//		//printf("Detect ESP8266\r\n");
//        OLED_ShowStr(8,16,(uint8_t *)"esp8266",2); //拆成两行显示 
//		OLED_ShowStr(40,32,(uint8_t *)"failed",2);
//        OLED_RefreshRAM();     //更新缓冲区        
//	}
//	else{
//		if(FindStr((char*)usart1_RX1,"K",2000) != 0){
//			//printf("Detect ESP8266\r\n");
//		}
//		else{
//			//printf("Detect ESP8266 Fail\r\n");
//		}
//	}
	HAL_Delay(100);
	
	flag = ConnectAP(SSID,PSWD);
	//OLED_Clear();                                              //清除屏幕内容
	if(flag==0){
		//printf("WIFI连接失败\r\n");
		OLED_ShowStr(28,16,(uint8_t *)"WIFI",2); //拆成两行显示 
		OLED_ShowStr(40,32,(uint8_t *)"failed",2);
        OLED_RefreshRAM();     //更新缓冲区        
	}
	else{
		//printf("WIFI连接成功\r\n");
        OLED_CLS();
        OLED_ClearRAM();    //清除缓冲区
        OLED_RefreshRAM();     //更新缓冲区
		OLED_ShowStr(8,8,(uint8_t *)"WiFi connected",2);
		//OLED_ShowStr(28,32,(uint8_t *)"",2);
        OLED_RefreshRAM();     //更新缓冲区
	}
	//OLED_Refresh();                                          //刷新屏幕内容
}

void Aliyun_Connect(void)
{
	char texta[488];
	
	memset(usart1_RX1,0,512); 	              //清空接收缓冲
	memset(Usart1_tx_buff,0,512); 	              //清空发送缓冲
	sprintf((char*)Usart1_tx_buff,"AT+MQTTUSERCFG=0,1,\"NULL\",\"");
	sprintf(texta,"%s\",\"%s\",0,0,\"\"\r\n",MQTT_username,MQTT_passwd);
	strcat((char*)Usart1_tx_buff,texta);           
	Usart1_Send_Str((uint8_t *)Usart1_tx_buff);   //发送AT指令
	HAL_Delay(3000);
	
	memset(usart1_RX1,0,512); 	              //清空接收缓冲
	memset(Usart1_tx_buff,0,512); 	              //清空发送缓冲
	sprintf((char*)Usart1_tx_buff,"AT+MQTTCLIENTID=0,\"%s\"\r\n",MQTT_clientId);         
	Usart1_Send_Str((uint8_t *)Usart1_tx_buff);   //发送AT指令
	HAL_Delay(3000);
	
	memset(usart1_RX1,0,512); 	              //清空接收缓冲
	memset(Usart1_tx_buff,0,512); 	              //清空发送缓冲
	sprintf((char*)Usart1_tx_buff,"AT+MQTTCONN=0,\"%s\",1883,1\r\n",MQTT_mqttHostUrl);         
	Usart1_Send_Str((uint8_t *)Usart1_tx_buff);   //发送AT指令
	HAL_Delay(3000);
}

//void Send_Dat_2_Aliyun(float temp, float humi, int light_lx)
//{
//	char text[48];
//	/*
//    光照：int Lux
//    湿度：float hum
//    温度：float temp
//    
//    */
//    
//	memset(Usart1_tx_buff,0,512); 	                               //清空发送缓冲
//	sprintf((char*)Usart1_tx_buff,"AT+MQTTPUB=0,\"%s\",\"{\\\"params\\\":{", MQTT_post);
//	sprintf(text,"\\\"temp\\\":%.1f\\,",temp);   //CurrentTemperature是阿里云的温度标识符
//	strcat((char*)Usart1_tx_buff,text);                            //将传感器数据整合进字符串
//	sprintf(text,"\\\"hum\\\":%.1f\\,",humi);  //CurrentHumidity是阿里云的湿度标识符
//	strcat((char*)Usart1_tx_buff,text);
//	sprintf(text,"\\\"Lux\\\":%d",light_lx);                //LightLux是阿里云的湿度标识符
//	strcat((char*)Usart1_tx_buff,text);
//	strcat((char*)Usart1_tx_buff,"}}\",1,0\r\n");
//	Usart1_Send_Str((uint8_t *)Usart1_tx_buff);
//}
//优化拼接方式更直观

//设置两个上报函数，分别为上报感知数据，下发控制操作后反馈控制状态
void Send_Dat_2_Aliyun(float temp, float hum, int Lux)
{
    memset(Usart1_tx_buff, 0, sizeof(Usart1_tx_buff));  // 清空发送缓冲区
//    temp = 3.3;
//    hum = 4.4;
//    Lux = 9;
    //记得，前面也要加\\（双斜杠），
    snprintf((char*)Usart1_tx_buff, sizeof(Usart1_tx_buff),
             "AT+MQTTPUB=0,\"%s\",\"{\\\"params\\\":{\\\"temp\\\":%.1f\\,\\\"hum\\\":%.1f\\,\\\"Lux\\\":%d}}\",1,0\r\n",
             MQTT_post, temp, hum, Lux);
    
    Usart1_Send_Str((uint8_t *)Usart1_tx_buff);
}


void Send_led_dat(int mode)
{
	char text[48];
		
	memset(Usart1_tx_buff,0,512); 	                //清空发送缓冲
	sprintf((char*)Usart1_tx_buff,"AT+MQTTPUB=0,\"%s\",\"{\\\"params\\\":{", MQTT_post);
	sprintf(text,"\\\"mode\\\":%d\\,",mode);   //LEDSta是阿里云的LED开关标识符
	strcat((char*)Usart1_tx_buff,text);                            
	strcat((char*)Usart1_tx_buff,"}}\",1,0\r\n");
	Usart1_Send_Str((uint8_t *)Usart1_tx_buff);
}
