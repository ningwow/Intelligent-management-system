#include "bh1750.h"
#include "i2c.h"

void SET_L_mode(void){
	BH1750_Send_Cmd(CONT_H_MODE);	//设置连续高分辨率测量
	//必须使用这个模式，其余模式光照范围过小，过亮的光照会使之失效。而单次读取会读取失败无法更新数据
}

uint8_t	BH1750_Send_Cmd(BH1750_MODE cmd)
{
	return HAL_I2C_Master_Transmit(&hi2c2, BH1750_ADDR_WRITE, (uint8_t*)&cmd, 1, 0xFFFF);
}

uint8_t BH1750_Read_Dat(uint8_t* dat)
{
	HAL_StatusTypeDef status;
	status =  HAL_I2C_Master_Receive(&hi2c2, BH1750_ADDR_READ, dat, 2, 0xFFFF);
	if (status != HAL_OK) {
        return HAL_ERROR;  // 读取失败，可能 I2C 挂起
    }
    return HAL_OK;
}

uint16_t BH1750_Dat_To_Lux(uint8_t* dat)
{
	uint16_t lux = 0;
	lux = dat[0];
	lux <<= 8;
	lux += dat[1];
	lux = (int)(lux / 1.2);

	return lux;
}

int lightSensorLux() {
	uint8_t dat[2] = { 0 };
	//if (HAL_OK == BH1750_Send_Cmd(ONCE_L_MODE)) {
		//HAL_Delay(5);
		if (HAL_OK == BH1750_Read_Dat(dat))
			return BH1750_Dat_To_Lux(dat);
		else
			return -1;
	//}
//return -2;
}
