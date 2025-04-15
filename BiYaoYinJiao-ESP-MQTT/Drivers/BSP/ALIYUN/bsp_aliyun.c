#include "bsp_aliyun.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "oled.h"

extern uint8_t usart1_RX1[512], uart3_rec_flag;

uint8_t Usart1_tx_buff[512]; //���ͻ�����

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
    uint16_t retry_count = retry_nms / 10;  // ����������Դ���

    while (retry_count--)
    {
        if (strstr(dest, src) != NULL)  // ֱ���ж��Ƿ��ҵ�
        {
            return 1;  // �ҵ����� 1
        }
        HAL_Delay(10);  // �ȴ� 10ms
    }

    return 0;  // ��ʱδ�ҵ�
}


static uint8_t ConnectAP(char* ssid,char* pswd)
{
	uint8_t cnt=2;
	while(cnt--)
	{
        
        memset(usart1_RX1,0,512);
		Usart1_Send_Str((uint8_t *)"ATE0\r\n"); //�رջ���
        HAL_Delay(2000);
        
		memset(usart1_RX1,0,512);
		Usart1_Send_Str((uint8_t *)"AT+CWMODE=1\r\n");              //����ΪSTATIONģʽ	
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
		memset(usart1_RX1,0,512);                            //��շ��ͻ���
		memset(Usart1_tx_buff,0,512);                            //��ս��ջ���
		sprintf((char*)Usart1_tx_buff,"AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,pswd);//����Ŀ��AP
		Usart1_Send_Str(Usart1_tx_buff);        
		if(FindStr((char*)usart1_RX1,"OK",5000)!=0)                      //���ӳɹ��ҷ��䵽IP
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
	Usart1_Send_Str((uint8_t *)"AT\r\n"); //���ESP8266�Ƿ����
    //Usart1_Send_Str((uint8_t *)"AT\r\n"); //���ESP8266�Ƿ����
//	if(FindStr((char*)usart1_RX1,"OK",2000) != 0){
//		//printf("Detect ESP8266\r\n");
//        OLED_ShowStr(8,16,(uint8_t *)"esp8266",2); //���������ʾ 
//		OLED_ShowStr(40,32,(uint8_t *)"failed",2);
//        OLED_RefreshRAM();     //���»�����        
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
	//OLED_Clear();                                              //�����Ļ����
	if(flag==0){
		//printf("WIFI����ʧ��\r\n");
		OLED_ShowStr(28,16,(uint8_t *)"WIFI",2); //���������ʾ 
		OLED_ShowStr(40,32,(uint8_t *)"failed",2);
        OLED_RefreshRAM();     //���»�����        
	}
	else{
		//printf("WIFI���ӳɹ�\r\n");
        OLED_CLS();
        OLED_ClearRAM();    //���������
        OLED_RefreshRAM();     //���»�����
		OLED_ShowStr(8,8,(uint8_t *)"WiFi connected",2);
		//OLED_ShowStr(28,32,(uint8_t *)"",2);
        OLED_RefreshRAM();     //���»�����
	}
	//OLED_Refresh();                                          //ˢ����Ļ����
}

void Aliyun_Connect(void)
{
	char texta[488];
	
	memset(usart1_RX1,0,512); 	              //��ս��ջ���
	memset(Usart1_tx_buff,0,512); 	              //��շ��ͻ���
	sprintf((char*)Usart1_tx_buff,"AT+MQTTUSERCFG=0,1,\"NULL\",\"");
	sprintf(texta,"%s\",\"%s\",0,0,\"\"\r\n",MQTT_username,MQTT_passwd);
	strcat((char*)Usart1_tx_buff,texta);           
	Usart1_Send_Str((uint8_t *)Usart1_tx_buff);   //����ATָ��
	HAL_Delay(3000);
	
	memset(usart1_RX1,0,512); 	              //��ս��ջ���
	memset(Usart1_tx_buff,0,512); 	              //��շ��ͻ���
	sprintf((char*)Usart1_tx_buff,"AT+MQTTCLIENTID=0,\"%s\"\r\n",MQTT_clientId);         
	Usart1_Send_Str((uint8_t *)Usart1_tx_buff);   //����ATָ��
	HAL_Delay(3000);
	
	memset(usart1_RX1,0,512); 	              //��ս��ջ���
	memset(Usart1_tx_buff,0,512); 	              //��շ��ͻ���
	sprintf((char*)Usart1_tx_buff,"AT+MQTTCONN=0,\"%s\",1883,1\r\n",MQTT_mqttHostUrl);         
	Usart1_Send_Str((uint8_t *)Usart1_tx_buff);   //����ATָ��
	HAL_Delay(3000);
}

//void Send_Dat_2_Aliyun(float temp, float humi, int light_lx)
//{
//	char text[48];
//	/*
//    ���գ�int Lux
//    ʪ�ȣ�float hum
//    �¶ȣ�float temp
//    
//    */
//    
//	memset(Usart1_tx_buff,0,512); 	                               //��շ��ͻ���
//	sprintf((char*)Usart1_tx_buff,"AT+MQTTPUB=0,\"%s\",\"{\\\"params\\\":{", MQTT_post);
//	sprintf(text,"\\\"temp\\\":%.1f\\,",temp);   //CurrentTemperature�ǰ����Ƶ��¶ȱ�ʶ��
//	strcat((char*)Usart1_tx_buff,text);                            //���������������Ͻ��ַ���
//	sprintf(text,"\\\"hum\\\":%.1f\\,",humi);  //CurrentHumidity�ǰ����Ƶ�ʪ�ȱ�ʶ��
//	strcat((char*)Usart1_tx_buff,text);
//	sprintf(text,"\\\"Lux\\\":%d",light_lx);                //LightLux�ǰ����Ƶ�ʪ�ȱ�ʶ��
//	strcat((char*)Usart1_tx_buff,text);
//	strcat((char*)Usart1_tx_buff,"}}\",1,0\r\n");
//	Usart1_Send_Str((uint8_t *)Usart1_tx_buff);
//}
//�Ż�ƴ�ӷ�ʽ��ֱ��

//���������ϱ��������ֱ�Ϊ�ϱ���֪���ݣ��·����Ʋ�����������״̬
void Send_Dat_2_Aliyun(float temp, float hum, int Lux)
{
    memset(Usart1_tx_buff, 0, sizeof(Usart1_tx_buff));  // ��շ��ͻ�����
//    temp = 3.3;
//    hum = 4.4;
//    Lux = 9;
    //�ǵã�ǰ��ҲҪ��\\��˫б�ܣ���
    snprintf((char*)Usart1_tx_buff, sizeof(Usart1_tx_buff),
             "AT+MQTTPUB=0,\"%s\",\"{\\\"params\\\":{\\\"temp\\\":%.1f\\,\\\"hum\\\":%.1f\\,\\\"Lux\\\":%d}}\",1,0\r\n",
             MQTT_post, temp, hum, Lux);
    
    Usart1_Send_Str((uint8_t *)Usart1_tx_buff);
}


void Send_led_dat(int mode)
{
	char text[48];
		
	memset(Usart1_tx_buff,0,512); 	                //��շ��ͻ���
	sprintf((char*)Usart1_tx_buff,"AT+MQTTPUB=0,\"%s\",\"{\\\"params\\\":{", MQTT_post);
	sprintf(text,"\\\"mode\\\":%d\\,",mode);   //LEDSta�ǰ����Ƶ�LED���ر�ʶ��
	strcat((char*)Usart1_tx_buff,text);                            
	strcat((char*)Usart1_tx_buff,"}}\",1,0\r\n");
	Usart1_Send_Str((uint8_t *)Usart1_tx_buff);
}
