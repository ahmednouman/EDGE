#include "main_app.h"

static void telemetryTask(void *parameters);
static void fanControlTask(void *parameters);
static void adcTask(void *parameters);
static void loggingTask(void *parameters);
static void IWDGTask(void *parameters);

static void hardwareSetupLog();

TaskHandle_t telemetryTaskHandle = NULL;
TaskHandle_t fanCtrlTaskHandle = NULL;
TaskHandle_t adcTaskHandle = NULL;
TaskHandle_t loggingTaskHandle = NULL;
TaskHandle_t applicationTaskHandle = NULL;
TaskHandle_t watchdogTaskHandle = NULL;

QueueHandle_t logQueue;

circ_buf adcBuffer;
circ_buf temperatureBuffer;
circ_buf speedBuffer;

SemaphoreHandle_t xMutexADC;
SemaphoreHandle_t xMutexTemperature;
SemaphoreHandle_t xMutexSpeed;

SemaphoreHandle_t xSemaphoreLogging;


int main() {
	hardwareInit();
	hardwareSetupLog();

    logging_init();
    setLogLevel(DEFAULT_LOG_LEVEL);

    xMutexADC = xSemaphoreCreateMutex();
    xMutexTemperature = xSemaphoreCreateMutex();
    xMutexSpeed = xSemaphoreCreateMutex();

    if (xMutexADC == NULL || xMutexTemperature == NULL || xMutexSpeed == NULL) {
        logMessageDirect("Mutex creation failed!\r\n");
        while (1);
    }

    xSemaphoreLogging = xSemaphoreCreateBinary();
    if (xSemaphoreLogging == NULL) {
        logMessageDirect("Logging semaphore creation failed!\r\n");
        while (1);
    } else {
        xSemaphoreGive(xSemaphoreLogging);
    }

    if (xTaskCreate(telemetryTask, "TelemetryTask", 500, NULL, 3, &telemetryTaskHandle) != pdPASS) {
        logMessageDirect("Telemetry task creation failed!\r\n");
        while (1);
    }

    if (xTaskCreate(fanControlTask, "FanControlTask", 500, NULL, 4, &fanCtrlTaskHandle) != pdPASS) {
        logMessageDirect("Fan control task creation failed!\r\n");
        while (1);
    }

    if (xTaskCreate(adcTask, "ADCTask", 500, NULL, 4, &adcTaskHandle) != pdPASS) {
        logMessageDirect("ADC task creation failed!\r\n");
        while (1);
    }

    if (xTaskCreate(loggingTask, "LoggingTask", 500, NULL, 1, &loggingTaskHandle) != pdPASS) {
        logMessageDirect("Logging task creation failed!\r\n");
        while (1);
    }

    if (xTaskCreate(IWDGTask, "WatchdogTask", 300, NULL, 2, &watchdogTaskHandle) != pdPASS) {
        logMessageDirect("Logging task creation failed!\r\n");
        while (1);
    }

    vTaskStartScheduler();

    return 0;
}

static void telemetryTask(void *parameters) {
    LOG(LOG_DEBUG, "Telemetry task started");
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(1000);

    RTC_TimeTypeDef RTC_TimeRead;
    RTC_DateTypeDef RTC_DateRead;

    uint16_t temperatureBuf[2];
    uint16_t adcBuf[10];
    uint16_t speedBuf[3];

    float timerPeriodInMs = 0.78;
    float periodInMs = 0;
    float fanSpeedRpm = 0.0;

    char messagePayload[512];

    while (1) {
    	LOG(LOG_DEBUG, "Telemetry task executing");
        xLastWakeTime = xTaskGetTickCount();

        HAL_RTC_GetTime(&hrtc, &RTC_TimeRead, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &RTC_DateRead, RTC_FORMAT_BIN);

        char timestamp[25];
        snprintf(timestamp, sizeof(timestamp), "%02d/%02d/%04d %02d:%02d:%02d",
                 RTC_DateRead.Date, RTC_DateRead.Month, RTC_DateRead.Year + 2000,
                 RTC_TimeRead.Hours, RTC_TimeRead.Minutes, RTC_TimeRead.Seconds);

        xSemaphoreTake(xMutexTemperature, portMAX_DELAY);
        lifoPop(&temperatureBuffer, temperatureBuf, 2);
        float t_degC1 = -45 + (175 * (float)temperatureBuf[0]) / 65535;
        float t_degC2 = -45 + (175 * (float)temperatureBuf[1]) / 65535;
        xSemaphoreGive(xMutexTemperature);

        xSemaphoreTake(xMutexADC, portMAX_DELAY);
        lifoPop(&adcBuffer, adcBuf, 10);
        float adcVoltages[10];
        for (int i = 0; i < 10; i++) {
            adcVoltages[i] = (adcBuf[i] / 4095.0) * 3.3;
        }
        xSemaphoreGive(xMutexADC);

        xSemaphoreTake(xMutexSpeed, portMAX_DELAY);
        if (lifoPop(&speedBuffer, speedBuf, 3)) {
            uint16_t totalSpeed = 0;
            for (int i = 0; i < 3; i++) {
                totalSpeed += speedBuf[i];
            }
            float averageFanSpeedPeriod = totalSpeed / 3.0;
            periodInMs = averageFanSpeedPeriod * timerPeriodInMs;
            float frequencyHz = 1000.0f / periodInMs;
            fanSpeedRpm = (frequencyHz * 60.0f) / 2.0f;
        } else {
        	LOG(LOG_WARNING, "Reading from an empty Speed Buffer");
            fanSpeedRpm = 0;
        }
        xSemaphoreGive(xMutexSpeed);

        snprintf(messagePayload, sizeof(messagePayload),
            "Timestamp: %s\r\n"
            "Temperature1: %.2f °C\r\n"
            "Temperature2: %.2f °C\r\n"
            "Fan Speed: %.2f RPM\r\n"
            "ADC Value1: %.2f V\r\n"
            "ADC Value2: %.2f V\r\n"
            "ADC Value3: %.2f V\r\n"
            "ADC Value4: %.2f V\r\n"
            "ADC Value5: %.2f V\r\n"
            "ADC Value6: %.2f V\r\n"
            "ADC Value7: %.2f V\r\n"
            "ADC Value8: %.2f V\r\n"
            "ADC Value9: %.2f V\r\n"
            "ADC Value10: %.2f V\r\n"
            "\r\n",
            timestamp,
            t_degC1, t_degC2, fanSpeedRpm,
            adcVoltages[0], adcVoltages[1], adcVoltages[2], adcVoltages[3],
            adcVoltages[4], adcVoltages[5], adcVoltages[6], adcVoltages[7],
            adcVoltages[8], adcVoltages[9]);

        CDC_Transmit_FS((uint8_t *)messagePayload, strlen(messagePayload));

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}


static void fanControlTask(void *parameters) {
    LOG(LOG_DEBUG, "Fan Control task started");
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(500);

    uint16_t temperatureTicks = 0;
    uint16_t pwmPeriod = htim3.Init.Period;

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 13, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    while (1) {
        xLastWakeTime = xTaskGetTickCount();

        sht4xGetTemperature(&hi2c1, &temperatureTicks);

        if (xSemaphoreTake(xMutexTemperature, portMAX_DELAY) == pdTRUE) {
            lifoPush(&temperatureBuffer, temperatureTicks);
            xSemaphoreGive(xMutexTemperature);
        }

        float t_degC = -45 + (175 * (float)temperatureTicks) / 65535;

        uint16_t pwmValue;

        if (t_degC <= 40) {
            pwmValue = 0;
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwmValue);
        } else if (t_degC > 40 && t_degC <= 50) {
            pwmValue = pwmPeriod * 0.45;
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwmValue);
        } else if (t_degC > 50 && t_degC < 70) {
            pwmValue = pwmPeriod * 0.65;
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwmValue);
        } else {
            pwmValue = pwmPeriod;
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwmValue);
        }

        float pwmDutyPercent = ((float)pwmValue / pwmPeriod) * 100.0f;
        LOG(LOG_INFO, "Temperature: %.2f°C, PWM duty cycle: %.2f%%", t_degC, pwmDutyPercent);

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

static void adcTask(void *parameters) {
    LOG(LOG_DEBUG, "ADC task started");
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(100);

    volatile uint16_t rawADC = 0;

    while (1) {
        xLastWakeTime = xTaskGetTickCount();

        HAL_ADC_Start_IT(&hadc1);

        uint32_t ulNotificationValue;
        xTaskNotifyWait(0x00, 0xFFFFFFFF, &ulNotificationValue, portMAX_DELAY);

        rawADC = HAL_ADC_GetValue(&hadc1);

        if (xSemaphoreTake(xMutexADC, portMAX_DELAY) == pdTRUE) {
            lifoPush(&adcBuffer, rawADC);
            xSemaphoreGive(xMutexADC);
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}


static void loggingTask(void *parameters) {
    LOG(LOG_DEBUG, "Logging task started");
    LogMessage logMsg;
    while (1) {
        if (xQueueReceive(logQueue, &logMsg, 0) == pdPASS) {
            if (xSemaphoreTake(xSemaphoreLogging, portMAX_DELAY) == pdTRUE) {
                sendLogToUART(logMsg.message);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

static void IWDGTask(void *parameters) {
    LOG(LOG_DEBUG, "Watchdog task started");
    MX_IWDG_Init();
    while (1) {
    	LOG(LOG_INFO, "Watchdog timer has been refreshed");
    	monitorTaskStack();
    	HAL_IWDG_Refresh(&hiwdg);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void monitorTaskStack() {
    UBaseType_t adcTaskStackHighWaterMark = uxTaskGetStackHighWaterMark(adcTaskHandle);
    UBaseType_t loggingTaskStackHighWaterMark = uxTaskGetStackHighWaterMark(loggingTaskHandle);
    UBaseType_t applicationTaskStackHighWaterMark = uxTaskGetStackHighWaterMark(applicationTaskHandle);
    UBaseType_t telemetryTaskStackHighWaterMark = uxTaskGetStackHighWaterMark(telemetryTaskHandle);
    UBaseType_t fanControlTaskStackHighWaterMark = uxTaskGetStackHighWaterMark(fanCtrlTaskHandle);
    UBaseType_t IWDGTaskStackHighWaterMark = uxTaskGetStackHighWaterMark(watchdogTaskHandle);

    LOG(LOG_DEBUG, "ADC Task High Water Mark: %lu words", adcTaskStackHighWaterMark);
    LOG(LOG_DEBUG, "Logging Task High Water Mark: %lu words", loggingTaskStackHighWaterMark);
    LOG(LOG_DEBUG, "Application Task High Water Mark: %lu words", applicationTaskStackHighWaterMark);
    LOG(LOG_DEBUG, "Telemetry Task High Water Mark: %lu words", telemetryTaskStackHighWaterMark);
    LOG(LOG_DEBUG, "Fan Control Task High Water Mark: %lu words", fanControlTaskStackHighWaterMark);
    LOG(LOG_DEBUG, "IWDG Task High Water Mark: %lu words", IWDGTaskStackHighWaterMark);
}


static void hardwareSetupLog()
{
    logMessageDirect("System Initialization Started\r\n");
    logMessageDirect("System Clock Frequency: %lu Hz\r\n", SystemCoreClock);

    uint32_t apb1Clock = HAL_RCC_GetPCLK1Freq();
    uint32_t apb2Clock = HAL_RCC_GetPCLK2Freq();
    logMessageDirect("Clock Frequency (APB1): %lu Hz\r\n", apb1Clock);
    logMessageDirect("Clock Frequency (APB2): %lu Hz\r\n", apb2Clock);

    uint32_t tim3Clk = apb1Clock * 2;
    uint32_t tim10Clk = apb2Clock * 1;

    logMessageDirect("PWM TIM3 Clock Frequency: %lu Hz\r\n", tim3Clk);
    logMessageDirect("TIM10 Clock Frequency: %lu Hz\r\n", tim10Clk);

    uint32_t tim3Prescaler = TIM3->PSC;
    uint32_t tim3AutoReload = TIM3->ARR;
    uint32_t tim10Prescaler = TIM10->PSC;
    uint32_t tim10AutoReload = TIM10->ARR;

    double tim3Period = (double)(tim3AutoReload + 1) * (tim3Prescaler + 1) / tim3Clk;
    double tim10Period = (double)(tim10AutoReload + 1) * (tim10Prescaler + 1) / tim10Clk;

    double tim3PwmFrequency = (double)tim3Clk / ((tim3Prescaler + 1) * (tim3AutoReload + 1));
    double tim10Resolution = (double)(tim10Prescaler + 1) / tim10Clk;

    logMessageDirect("TIM3 Period: %f seconds\r\n", tim3Period);
    logMessageDirect("TIM3 PWM Frequency: %f Hz\r\n", tim3PwmFrequency);
    logMessageDirect("TIM10 Period: %f seconds\r\n", tim10Period);
    logMessageDirect("TIM10 Timer Resolution: %f seconds\r\n", tim10Resolution);

    uint32_t systemClockSource = RCC->CFGR & RCC_CFGR_SWS;
    uint32_t rtcClockSource = RCC->BDCR & RCC_BDCR_RTCSEL;

    logMessageDirect("System Clock Source: %lu\r\n", systemClockSource);
    logMessageDirect("RTC Clock Source: %lu\r\n", rtcClockSource);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    logMessageDirect("Stack Overflow Detected!\r\n");
    while (1);
}

