#include "oled.h"
#include "oled_Front.h"

unsigned char  ScreenBuffer[SCREEN_PAGE_NUM][SCREEN_COLUMN];//������Ļ�洢�ռ�



/**
* @brief  ��OLED�Ĵ�����ַдһ��byte����
* @param  addr:�Ĵ�����ַ
* @param  data:Ҫд������
* @retval ��
*/
void I2C_WriteByte(uint8_t addr, uint8_t data)
{
	extern I2C_HandleTypeDef hi2c1;
	HAL_I2C_Mem_Write(&hi2c1, OLED_ADDRESS, addr, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
	
}

/**
 * ************************************************************************
 * @brief д�����
 * @param[in] cmd  д�������
 * ************************************************************************
 */
void WriteCmd(unsigned char cmd)
{
	I2C_WriteByte(0x00, cmd);
}

/**
 * ************************************************************************
 * @brief д���ݺ���
 * @param[in] dat  д�������
 * ************************************************************************
 */
void WriteDat(unsigned char dat)
{
	I2C_WriteByte(0x40, dat);
}

/**
 * ************************************************************************
 * @brief ����OLED
 * ************************************************************************
 */
void OLED_ON(void)
{
	WriteCmd(0X8D);  //���õ�ɱ�
	WriteCmd(0X14);  //������ɱ�
	WriteCmd(0XAF);  //OLED����
}

/**
 * ************************************************************************
 * @brief ����OLED
 * ************************************************************************
 */
void OLED_OFF(void)
{
	WriteCmd(0X8D);  //���õ�ɱ�
	WriteCmd(0X10);  //�رյ�ɱ�
	WriteCmd(0XAE);  //OLED�ر�
}

/**
 * ************************************************************************
 * @brief OLED����
 * ************************************************************************
 */
void OLED_CLS(void)//����
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		WriteCmd(0xb0+m);	//page0-page1
		WriteCmd(0x00);		//low column start address
		WriteCmd(0x10);		//high column start address
		for(n=0;n<128;n++)
		{
			WriteDat(0x00);
		}
	}
}

/**
 * ************************************************************************
 * @brief OLED��ʼ��
 * ************************************************************************
 */
void OLED_Init(void)
{
	WriteCmd(0xAE); //��ʾ�ر�
	WriteCmd(0x20);	//�����ڴ�Ѱַģʽ
	WriteCmd(0x02);	//00,ˮƽѰַģʽ;01,��ֱѰַģʽ;10,ҳѰַģʽ(��λ);11,��Ч
	WriteCmd(0xb0);	//ҳѰַģʽ��ҳ��ʼ��ַ,0-7
	WriteCmd(0xc8);	//����COM���ɨ�跽��
	WriteCmd(0x00); //-���õ��е�ַ
	WriteCmd(0x10); //-���ø��е�ַ
	WriteCmd(0x40); //-������ʼ�е�ַ
	WriteCmd(0x81); //���öԱȶȿ��ƼĴ���
	WriteCmd(0xff); //���ȵ��� 0x00~0xff
	WriteCmd(0xa1); //���ö����ӳ���0-127
	WriteCmd(0xa6); //����������ʾ
	WriteCmd(0xa8); //���ø��ñ���(1-64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); //0xa4�������ѭRAM����;0xa5,�������RAM����
	WriteCmd(0xd3); //������ʾƫ��
	WriteCmd(0x00); //��ƫ��
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //����DC-DCʹ��
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--������ʾ
	OLED_CLS();
}

/**
 * ************************************************************************
 * @brief �������ݻ�����
 * ************************************************************************
 */
void OLED_RefreshRAM(void)
{
	// ҳѰַģʽ���
	for(unsigned short int m = 0; m < SCREEN_ROW/8; m++)
	{
		WriteCmd(0xb0+m);		//����ҳ��ַb0~b7
		WriteCmd(0x00);			//������ʾ�е͵�ַ00-0f
		WriteCmd(0x10);			//������ʾ�иߵ�ַ10-1f
		for(unsigned short int n = 0; n < SCREEN_COLUMN; n++)
		{
			WriteDat(ScreenBuffer[m][n]);
		}
	} 
}

/**
 * ************************************************************************
 * @brief ������ݻ�����
 * ************************************************************************
 */
void OLED_ClearRAM(void)
{
	for(unsigned short int m = 0; m < SCREEN_ROW/8; m++)
	{
		for(unsigned short int n = 0; n < SCREEN_COLUMN; n++)
		{
			ScreenBuffer[m][n] = 0x00;
		}
	}
}

/**
 * ************************************************************************
 * @brief �����������ص�����
 * 
 * @param[in] x  			��ʼ������(x:0~127)
 * @param[in] y  			��ʼ������(y:0~63)
 * @param[in] set_pixel  	�õ������  SET_PIXEL = 1, RESET_PIXEL = 0
 * 
 * ************************************************************************
 */
void OLED_SetPixel(signed short int x, signed short int y, unsigned char set_pixel)
{ 
	if (x >= 0 && x < SCREEN_COLUMN && y >= 0 && y < SCREEN_ROW) {
		if(set_pixel){
				ScreenBuffer[y/8][x] |= (0x01 << (y%8));
		}  
		else{
				ScreenBuffer[y/8][x] &= ~(0x01 << (y%8));
		}
	}
}

/**
 * ************************************************************************
 * @brief ��Ļ����ȡ����ʾ
 * 
 * @param[in] mode  ����
 * 					��	ON	0xA7	����ȫ��
 *  				��	OFF	0xA6	Ĭ�ϴ�ģʽ,���ص����
 * 
 * ************************************************************************
 */
void OLED_DisplayMode(unsigned char mode)
{
	WriteCmd(mode);
}

/**
 * ************************************************************************
 * @brief ��Ļ���ȵ���
 * 
 * @param[in] intensity  ���ȴ�С(0~255),Ĭ��ֵ0x7f
 * 
 * ************************************************************************
 */
void OLED_IntensityControl(unsigned char intensity)
{
	WriteCmd(0x81);
	WriteCmd(intensity);
}

/**
 * ************************************************************************
 * @brief ȫ������ƫ��ָ������
 * 
 * @param[in] shift_num  ƫ�ƾ���(0~63)
 * 
 * ************************************************************************
 */
void OLED_Shift(unsigned char shift_num)
{
	for(unsigned char i = 0; i < shift_num; i++)
		{
			WriteCmd(0xd3);//������ʾƫ��,��ֱ����ƫ��
			WriteCmd(i);//ƫ����
			HAL_Delay(10);//��ʱʱ��
		}
}


/**
 * ************************************************************************
 * @brief ��Ļ����ˮƽ�����������
 * 
 * @param[in] start_page  	��ʼҳ��	(0~7)
 * @param[in] end_page  	����ҳ��	(0~7)
 * @param[in] direction  	��������
 * 								��		LEFT	0x27
 * 								��	RIGHT	0x26
 * @note �ٿ�ʼҳ���ͽ���ҳ��֮��Ż����,��д����˳�򲻵øı�
 * ************************************************************************
 */
void OLED_HorizontalShift(unsigned char start_page,unsigned char end_page,unsigned char direction)
{
	WriteCmd(0x2e);  //�رչ���

	WriteCmd(direction);//���ù�������
	WriteCmd(0x00);//�����ֽ�����,Ĭ��Ϊ0x00
	WriteCmd(start_page);//���ÿ�ʼҳ��ַ
	WriteCmd(0x05);//����ÿ������֮���ʱ������֡��
	WriteCmd(end_page);//���ý���ҳ��ַ
	WriteCmd(0x00);//�����ֽ�����,Ĭ��0x00
	WriteCmd(0xff);//�����ֽ�����,Ĭ��0xff

	WriteCmd(0x2f);//��������-0x2f,����-0x2e,��������д����
}
/**
 * ************************************************************************
 * @brief �ַ�����ʾ����
 * 
 * @param[in] x  	��ʼ������(0~127)
 * @param[in] y  	��ʼ������(0~63)
 * @param[in] ch  	��ʾ�ַ���(ͨ��˫��������)
 * @param[in] TextSize  �ַ���С(1:6*8 ;2:8*16)
 * 
 * ************************************************************************
 */
void OLED_ShowStr(signed short int x, signed short int y, unsigned char ch[], unsigned char TextSize)
{ 
	if (x >= 0 && x < SCREEN_COLUMN && y >= 0 && y < SCREEN_ROW) 
	{
		int32_t c = 0;
		unsigned char j = 0;
	
		switch(TextSize)
		{
			case 1:
			{
				while(ch[j] != '\0')
				{
					c = ch[j] - 32;
					if(c < 0)	//��Ч����
						break;
					
					if(x >= 125 || (127-x < 6))//һ�������ʾ�ַ���:21�ֽ���ʾ,������в���ʾ || ʣ����С��6������ʾ�����ַ�,������ʾ
					{
						x = 0;
						y += 8;//������ʾ
						if(63 - y < 8)	// ��������ʾһ�в���ʾ
							break;
					}
					for(unsigned char m = 0; m < 6; m++)
					{
						for(unsigned char n = 0; n < 8; n++)
						{
							OLED_SetPixel(x+m, y+n, (F6x8[c][m] >> n) & 0x01);
						}
					}
					x += 6;
					j++;
				}
			}break;
			case 2:
			{
				while(ch[j] != '\0')
				{
					c = ch[j] - 32;
					if(c < 0)	//��Ч����
						break;
					
					if(x >= 127 || (127-x < 8))//16�ֽ���ʾ|| ʣ����С��8������ʾ�����ַ�,������ʾ
					{
						x = 0;
						y += 16;//????
						if(63 - y < 16)	// ???????????
							break;
					}
					for(unsigned char m = 0; m < 2; m++)
					{
						for(unsigned char n = 0; n < 8; n++)
						{
							for(unsigned char i = 0; i < 8; i++)
							{
								OLED_SetPixel(x+n, y+i+m*8, (F8X16[c][n+m*8] >> i) & 0x01);
							}
						}	
					}
					x += 8;
					j++;
				}
			}break;
		}
	}
	OLED_RefreshRAM();
}


/**
 * ************************************************************************
 * @brief ���ĺ�����ʾ����
 * 
 * @param[in] x  	��ʼ�������(0~127)
 * @param[in] y  	��ʼ��������(0~63)
 * @param[in] ch  	������ģ����
 * 
 * @example OLED_ShowCN(0,0,"��");
 * ************************************************************************
 */
void OLED_ShowChinese(signed short int x, signed short int y, unsigned char* ch)
{
	if (x >= 0 && x < SCREEN_COLUMN && y >= 0 && y < SCREEN_ROW) {
		int32_t  len = 0,offset = sizeof(F16x16_CN[0].index);
		
		while(ch[len] != '\0')
		{
			if(x >= 127 || (127-x < 16))//8��������ʾ|| ʣ����С��16������ʾ�����ַ�,������ʾ
			{
				x = 0;
				y += 16;
				if(63 - y < 16)	// ��������ʾһ�в���ʾ
					break;
			}
					
			//��Ҫ�������ݴ�����ʾ��������
			for(unsigned char i = 0; i < sizeof(F16x16_CN)/sizeof(GB2312_CN); i++)
			{
				if(((F16x16_CN[i].index[0] == ch[len]) && (F16x16_CN[i].index[1] == ch[len+1]))){
						for(unsigned char m = 0; m < 2; m++)	//ҳ
						{
							for(unsigned char n = 0; n < 16; n++) // ��
							{
								for(unsigned char j = 0; j < 8; j++)	// ��
								{
									OLED_SetPixel(x+n, y+j+m*8, (F16x16_CN[i].encoder[n+m*16] >> j) & 0x01);
								}
							}
						}			
						x += 16;
						len += offset;
						break;
				}
				else if(F16x16_CN[i].index[0] == ch[len] && ch[len] == 0x20){
					for(unsigned char m = 0; m < 2; m++)
					{
						for(unsigned char n = 0; n < 16; n++)
						{
							for(unsigned char j = 0; j < 8; j++)
							{
								OLED_SetPixel(x+n, y+j+m*8, (F16x16_CN[i].encoder[n+m*16] >> j) & 0x01);
							}								
						}	
					}			
					x += 16; 
					len++;
					break;
				}
			}
		}
	}
	OLED_RefreshRAM();
}


/**
 * ************************************************************************
 * @brief BMPͼƬ��ʾ����
 * 
 * @param[in] x0  	��ʼ�������(0~127)
 * @param[in] y0  	��ʼ��������(0~63)
 * @param[in] L  	BMPͼƬ���
 * @param[in] H  	BMPͼƬ�߶�
 * @param[in] BMP  	ͼƬȡģ��ַ
 * 
 * @example OLED_ShowBMP(0,0,52,48,(unsigned char *)astronaut_0);
 * ************************************************************************
 */
void OLED_ShowBMP(signed short int x0,signed short int y0,signed short int L,signed short int H,const unsigned char BMP[])
{
	if (x0 >= 0 && x0 < SCREEN_COLUMN && x0+L <= SCREEN_ROW &&\
		y0 >= 0 && y0 < SCREEN_COLUMN && y0+H <= SCREEN_ROW) {
		
		unsigned char *p = (unsigned char *)BMP;
		for(signed short int y = y0; y < y0+H; y+=8)
		{
			for(signed short int x = x0; x < x0+L; x++)
			{
				for(signed short int i = 0; i < 8; i++)
				{
					OLED_SetPixel(x, y+i, ((*p) >> i) & 0x01);
				}
				p++;
			}
		}
	}

	OLED_RefreshRAM();
}


