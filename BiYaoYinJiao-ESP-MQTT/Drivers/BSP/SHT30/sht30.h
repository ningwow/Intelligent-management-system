#ifndef __SHT30_H
#define __SHT30_H

#include "stm32f1xx_hal.h"

void SendHandCommand(void);
char* ReceiveData(void);
void ParseReceivedData(char* data, float* temperature, float* humidity);


#endif /* __SHT30_H */
