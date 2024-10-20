#include "logging.h"

#define COLOR_RESET "\033[0m"
#define COLOR_DEBUG "\033[0m"
#define COLOR_INFO "\033[32m"
#define COLOR_WARNING "\033[33m"
#define COLOR_ERROR "\033[31m"
#define COLOR_CRITICAL "\033[41m"

extern QueueHandle_t logQueue;

static volatile LogLevel currentLogLevel = LOG_DEBUG;

void sendLogToUART(const char *message) {
    HAL_UART_Transmit_DMA(&huart6, (uint8_t *)message, strlen(message));
}

void logMessage(LogLevel level, const char *format, ...) {
    if (level < currentLogLevel) {
        return;
    }

    LogMessage logMsg;
    logMsg.level = level;

    const char *color = COLOR_RESET;
    switch (level) {
        case LOG_DEBUG: color = COLOR_DEBUG; break;
        case LOG_INFO: color = COLOR_INFO; break;
        case LOG_WARNING: color = COLOR_WARNING; break;
        case LOG_ERROR: color = COLOR_ERROR; break;
        case LOG_CRITICAL: color = COLOR_CRITICAL; break;
    }

    va_list args;
    va_start(args, format);
    snprintf(logMsg.message, BUFFER_SIZE, "%s[%d] ", color, level);
    vsnprintf(logMsg.message + strlen(logMsg.message), BUFFER_SIZE - strlen(logMsg.message), format, args);
    snprintf(logMsg.message + strlen(logMsg.message), BUFFER_SIZE - strlen(logMsg.message), "%s\r\n", COLOR_RESET);
    va_end(args);

    xQueueSend(logQueue, &logMsg, portMAX_DELAY);
}

void logMessageDirect(const char *format, ...) {
    char message[BUFFER_SIZE];
    va_list args;

    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    HAL_UART_Transmit(&huart6, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
}

void setLogLevel(LogLevel level) {
    currentLogLevel = level;

    const char *levelStr = "";
    switch (level) {
        case LOG_DEBUG: levelStr = "DEBUG"; break;
        case LOG_INFO: levelStr = "INFO"; break;
        case LOG_WARNING: levelStr = "WARNING"; break;
        case LOG_ERROR: levelStr = "ERROR"; break;
        case LOG_CRITICAL: levelStr = "CRITICAL"; break;
    }

    char message[BUFFER_SIZE];
    snprintf(message, sizeof(message), "Log level changed to: %s\r\n", levelStr);
    logMessageDirect(message);
}

void logging_init(void) {
    currentLogLevel = DEFAULT_LOG_LEVEL;

    logQueue = xQueueCreate(LOG_QUEUE_SIZE, sizeof(LogMessage));
    if (logQueue == NULL) {
        logMessageDirect("Log queue creation failed!\r\n");
            while (1);
    }else{
    	logMessageDirect("Log queue created successfully with width: %d, size of each element: %d bytes\r\n", LOG_QUEUE_SIZE, sizeof(LogMessage));
    }

}
