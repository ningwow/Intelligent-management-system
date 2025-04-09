#ifndef __ALIYUN_H__
#define __ALIYUN_H__

#include "main.h"
#include "usart.h"

/*��Ҫ�޸�WiFi�˺ź����룬����Ϊ�����ȵ㣬��Ƶ�ʽ�Ϊ2.4GHz*/
#define SSID "HUAWEI RS"          //WIFI�˺�
#define PSWD "88888888"      //����

/*��Ҫ�޸İ������豸��MQTT���Ӳ���*/
/*ע�⣺����֮ǰ��\\   a1b4Jc43DrM.Test|securemode=2,signmethod=hmacsha256,timestamp=1719299758491|*/
#define MQTT_clientId "k0b0vEwvuMA.device|securemode=2\\,signmethod=hmacsha256\\,timestamp=1741182822795|"
#define MQTT_username "device&k0b0vEwvuMA"
#define MQTT_passwd	"43f0a44b79b573a4e8e2925f32f24cef2254d60bfb9d4f7edeea4d251df9c8ff"
//�޸�Ϊ���ʵ����ĵ�ַ
#define MQTT_mqttHostUrl "iot-06z00eokeog2hf0.mqtt.iothub.aliyuncs.com"

//post,�޸ĵڶ���������������Ϊ�Լ��ļ���
#define MQTT_post "/sys/k0b0vEwvuMA/device/thing/event/property/post"

void ESP8266_Init(void);
void Aliyun_Connect(void);
void Send_Dat_2_Aliyun(float temp, float humi, int light_lx);
void Send_led_dat(int mode);
void Usart1_Send_Str(uint8_t *buf);

#endif /* __ALIYUN_H__ */
