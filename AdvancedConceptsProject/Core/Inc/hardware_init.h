#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f4xx_hal.h"
#include "usb_device.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

extern ADC_HandleTypeDef hadc1;
extern I2C_HandleTypeDef hi2c1;
extern IWDG_HandleTypeDef hiwdg;
extern RTC_HandleTypeDef hrtc;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim10;
extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef hdma_usart6_tx;

void hardwareInit();
void MX_IWDG_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

void Error_Handler(void);



#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
