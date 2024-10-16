#include "sht4x.h"


void sht4xReadRegister(I2C_HandleTypeDef *pI2CHandle, uint8_t reg_addr, uint8_t *buffer, uint8_t size)
{
    HAL_I2C_Master_Transmit(pI2CHandle, (uint8_t)(SHT4X_SLAVE_ID << 1), &reg_addr, 1, HAL_MAX_DELAY);
    HAL_Delay(10);
    HAL_StatusTypeDef result = HAL_I2C_Master_Receive(pI2CHandle, (uint8_t)(SHT4X_SLAVE_ID << 1), buffer, size, HAL_MAX_DELAY);
    if (result == HAL_OK)
    {

    }else{
    	LOG(LOG_ERROR, "ERROR READING TEMPERATURE SENSOR");
    }
}

void sht4xWrite(I2C_HandleTypeDef *pI2CHandle, uint8_t *data, uint8_t size)
{
    HAL_I2C_Master_Transmit(pI2CHandle, (uint8_t)(SHT4X_SLAVE_ID << 1), data, size, HAL_MAX_DELAY);
}

void sht4xGetTemperature(I2C_HandleTypeDef *pI2CHandle, uint16_t *tempTicks)
{
	uint8_t i2cRxBuffer[6];
	sht4xReadRegister(pI2CHandle, SHT4X_MEASURE_REG, i2cRxBuffer, 6);

	uint16_t t_ticks = (i2cRxBuffer[0] << 8) | i2cRxBuffer[1];

    if (sht4xCheckCrc(i2cRxBuffer, 2, i2cRxBuffer[2])) {
    	LOG(LOG_DEBUG, "Reading temperature sensor OK");
        *tempTicks = t_ticks;
    } else {
    	LOG(LOG_ERROR, "ERROR READING TEMPERATURE SENSOR CHECKSUM");
        *tempTicks = 0;
    }
}

bool sht4xCheckCrc(uint8_t *data, uint8_t len, uint8_t checksum) {
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc <<= 1;
            }
        }
    }
    return (crc == checksum);
}
