#ifndef INC_SHT4X_H_
#define INC_SHT4X_H_

#include <stdint.h>
#include <stdbool.h>
#include "logging.h"

#define SHT4X_SLAVE_ID 0x44

#define SHT4X_MEASURE_REG 0xFD


void sht4xReadRegister(I2C_HandleTypeDef *pI2CHandle, uint8_t reg_addr, uint8_t *buffer, uint8_t size);
void sht4xWrite(I2C_HandleTypeDef *pI2CHandle, uint8_t *data, uint8_t size);

void sht4xGetTemperature(I2C_HandleTypeDef *pI2CHandle, uint16_t *tempTicks);

bool sht4xCheckCrc(uint8_t *data, uint8_t len, uint8_t checksum);

#endif /* INC_SHT4X_H_ */
