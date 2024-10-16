#ifndef INC_LOGGING_H_
#define INC_LOGGING_H_

#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "hardware_init.h"


#define ENABLE_LOGGING
#define DEFAULT_LOG_LEVEL LOG_DEBUG

#ifdef ENABLE_LOGGING
    #define LOG(level, format, ...) logMessage(level, format, ##__VA_ARGS__)
#else
    #define LOG(level, format, ...)
#endif


#define BUFFER_SIZE 256
#define LOG_QUEUE_SIZE 20

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_CRITICAL
} LogLevel;

typedef struct {
    LogLevel level;
    char message[BUFFER_SIZE];
} LogMessage;


void logging_init(void);
void logMessage(LogLevel level, const char *format, ...);
void logMessageDirect(const char *format, ...);
void setLogLevel(LogLevel level);
void sendLogToUART(const char *message);

#endif /* INC_LOGGING_H_ */
