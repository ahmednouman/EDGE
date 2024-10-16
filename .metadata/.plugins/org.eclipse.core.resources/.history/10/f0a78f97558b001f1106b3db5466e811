#include "hardware_init.h"
#include "stm32f4xx_it.h"

#include "circular_buffer.h"
#include "logging.h"


extern TaskHandle_t adcTaskHandle;

extern SemaphoreHandle_t xSemaphoreLogging;
extern SemaphoreHandle_t xMutexSpeed;
extern circ_buf speedBuffer;

static uint16_t lastCaptureTime = 0;
static uint16_t period = 0;
static uint32_t overflow = 0;
static uint32_t lastOverflow = 0;

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern TIM_HandleTypeDef htim1;


void NMI_Handler(void)
{

   while (1)
  {
  }

}


void HardFault_Handler(void)
{
  logMessageDirect("HarFault!\r\n");
  while (1)
  {

  }
}


void MemManage_Handler(void)
{
  logMessageDirect("Memory Manage ISSUE!\r\n");
  while (1)
  {

  }
}


void BusFault_Handler(void)
{
  logMessageDirect("Bus Fault!\r\n");
  while (1)
  {

  }
}

void UsageFault_Handler(void)
{
  logMessageDirect("Usage Fault!\r\n");
  while (1)
  {

  }
}


void DebugMon_Handler(void)
{

}


void ADC_IRQHandler(void)
{
  HAL_ADC_IRQHandler(&hadc1);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }

  if (htim->Instance == TIM10)
  {
	  overflow++;
  }
}

void TIM1_UP_TIM10_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim1);
  HAL_TIM_IRQHandler(&htim10);
}


void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
}


void OTG_FS_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}


void DMA2_Stream6_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart6_tx);
}


void USART6_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart6);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_12)
    {
        uint16_t currentTime = __HAL_TIM_GET_COUNTER(&htim10);
        uint32_t currentOverflow = overflow;

        uint32_t tempPeriod = 0;

        if (currentOverflow == lastOverflow)
        {
            if (currentTime >= lastCaptureTime)
            {
                tempPeriod = currentTime - lastCaptureTime;
            }
            else
            {
                tempPeriod = (0xFFFF - lastCaptureTime) + currentTime + 1;
            }
        }
        else
        {
            tempPeriod = ((0xFFFF - lastCaptureTime) + 1) + (currentOverflow - lastOverflow - 1) * (0xFFFF + 1) + currentTime;
        }

        period = (uint16_t)tempPeriod;

        lastCaptureTime = currentTime;
        lastOverflow = currentOverflow;

        if (xSemaphoreTakeFromISR(xMutexSpeed, NULL) == pdTRUE) {
        	lifoPush(&speedBuffer, period);
            xSemaphoreGiveFromISR(xMutexSpeed, NULL);
        }

    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc->Instance == ADC1) {

        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTaskNotifyFromISR(
        	adcTaskHandle,
            0x01,
            eSetBits,
            &xHigherPriorityTaskWoken
        );

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == huart6.Instance) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(xSemaphoreLogging, &xHigherPriorityTaskWoken);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
