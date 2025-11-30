/**
 * @file log.h
 * @brief 日志系统头文件
 */

#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>
#include "usart.h"
#include "stm32f4xx_hal.h"
#include <stddef.h>

/* 日志级别定义 */
typedef enum {
    LOG_LEVEL_DEBUG = 0,    // 调试级别
    LOG_LEVEL_INFO,         // 信息级别
    LOG_LEVEL_WARNING,      // 警告级别
    LOG_LEVEL_ERROR,        // 错误级别
    LOG_LEVEL_NONE          // 不输出任何日志
} LogLevel;

/* 日志级别字符串 */
extern const char LOG_LEVEL_STR[];

/* 当前日志级别 */
extern LogLevel current_log_level;

/**
 * @brief 设置日志级别
 * @param level 日志级别
 */
void log_set_level(LogLevel level);

/**
 * @brief 获取当前日志级别
 * @return 当前日志级别
 */
LogLevel log_get_level(void);

/**
 * @brief 获取当前系统时间戳（毫秒）
 * @param time_str 存储时间字符串的缓冲区
 * @param buf_size 缓冲区大小
 */
void log_get_timestamp(char* time_str, size_t buf_size);

/**
 * @brief 输出日志
 * @param level 日志级别
 * @param file 文件名
 * @param line 行号
 * @param func 函数名
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void log_output(LogLevel level, const char* file, int line, const char* func, const char* format, ...);

/* 日志宏定义 */
#define LOG_DEBUG(format, ...)     do {         if (LOG_LEVEL_DEBUG >= log_get_level()) {             log_output(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__);         }     } while(0)

#define LOG_INFO(format, ...)     do {         if (LOG_LEVEL_INFO >= log_get_level()) {             log_output(LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__);         }     } while(0)

#define LOG_WARNING(format, ...)     do {         if (LOG_LEVEL_WARNING >= log_get_level()) {             log_output(LOG_LEVEL_WARNING, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__);         }     } while(0)

#define LOG_ERROR(format, ...)     do {         if (LOG_LEVEL_ERROR >= log_get_level()) {             log_output(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__);         }     } while(0)

#endif /* __LOG_H */
