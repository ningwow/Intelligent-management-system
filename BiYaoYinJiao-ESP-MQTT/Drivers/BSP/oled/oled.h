
#ifndef __OLED_H_
#define __OLED_H_

#include "i2c.h"

#define  OLED_ADDRESS 		0x78	//OLED从地址  0x78

//OLED?????
#define  	LEFT 	0x27
#define  	RIGHT 0x26
#define  	UP 	0X29
#define  	DOWM	0x2A
#define  	ON			0xA7
#define  	OFF			0xA6


#define    	SCREEN_PAGE_NUM	8    //屏幕页数
#define    	SCREEN_PAGEDATA_NUM	128   //每页数据个数
#define		SCREEN_COLUMN		128   //列数
#define  	SCREEN_ROW	64    //行数


void WriteCmd(unsigned char cmd);		//写命令
void WriteDat(unsigned char dat);		//写数据
void OLED_ON(void);				//开启OLED
void OLED_OFF(void);				//休眠OLED
void OLED_CLS(void);				//OLED清屏
void OLED_Init(void);				//OLED初始化
void OLED_RefreshRAM(void);			//更新缓冲区
void OLED_ClearRAM(void);			//清除缓冲区
void OLED_SetPixel(signed short int x, signed short int y, unsigned char set_pixel);	//设置坐标像素点数据
void OLED_DisplayMode(unsigned char mode);	//屏幕内容取反输出
void OLED_IntensityControl(unsigned char intensity);//屏幕亮度调节
void OLED_Shift(unsigned char shift_num);	//全屏内容偏移指定距离
void OLED_HorizontalShift(unsigned char start_page,unsigned char end_page,unsigned char direction);	//屏幕内容水平滚动
void OLED_ShowStr(signed short int x, signed short int y, unsigned char ch[], unsigned char TextSize);//字符串显示函数
void OLED_ShowChinese(signed short int x, signed short int y, unsigned char* ch);//汉字显示函数
void OLED_ShowBMP(signed short int x0,signed short int y0,signed short int L,signed short int H,const unsigned char BMP[]);//位图显示函数
#endif /*__OLED_H__*/
