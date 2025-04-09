
#ifndef __OLED_H_
#define __OLED_H_

#include "i2c.h"

#define  OLED_ADDRESS 		0x78	//OLED�ӵ�ַ  0x78

//OLED?????
#define  	LEFT 	0x27
#define  	RIGHT 0x26
#define  	UP 	0X29
#define  	DOWM	0x2A
#define  	ON			0xA7
#define  	OFF			0xA6


#define    	SCREEN_PAGE_NUM	8    //��Ļҳ��
#define    	SCREEN_PAGEDATA_NUM	128   //ÿҳ���ݸ���
#define		SCREEN_COLUMN		128   //����
#define  	SCREEN_ROW	64    //����


void WriteCmd(unsigned char cmd);		//д����
void WriteDat(unsigned char dat);		//д����
void OLED_ON(void);				//����OLED
void OLED_OFF(void);				//����OLED
void OLED_CLS(void);				//OLED����
void OLED_Init(void);				//OLED��ʼ��
void OLED_RefreshRAM(void);			//���»�����
void OLED_ClearRAM(void);			//���������
void OLED_SetPixel(signed short int x, signed short int y, unsigned char set_pixel);	//�����������ص�����
void OLED_DisplayMode(unsigned char mode);	//��Ļ����ȡ�����
void OLED_IntensityControl(unsigned char intensity);//��Ļ���ȵ���
void OLED_Shift(unsigned char shift_num);	//ȫ������ƫ��ָ������
void OLED_HorizontalShift(unsigned char start_page,unsigned char end_page,unsigned char direction);	//��Ļ����ˮƽ����
void OLED_ShowStr(signed short int x, signed short int y, unsigned char ch[], unsigned char TextSize);//�ַ�����ʾ����
void OLED_ShowChinese(signed short int x, signed short int y, unsigned char* ch);//������ʾ����
void OLED_ShowBMP(signed short int x0,signed short int y0,signed short int L,signed short int H,const unsigned char BMP[]);//λͼ��ʾ����
#endif /*__OLED_H__*/
