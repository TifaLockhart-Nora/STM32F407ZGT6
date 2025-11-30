/**
 * @file log.c
 * @brief 日志系统实现
 */

#include "log.h"
#include <stdarg.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "rtc.h"

/* 日志级别字符串 */
const char LOG_LEVEL_STR[] = {
    'D',
    'I',
    'W',
    'E'
};

/* 当前日志级别，默认为INFO级别 */
LogLevel current_log_level = LOG_LEVEL_INFO;

/**
 * @brief 设置日志级别
 * @param level 日志级别
 */
void log_set_level(LogLevel level)
{
    current_log_level = level;
}

/**
 * @brief 获取当前日志级别
 * @return 当前日志级别
 */
LogLevel log_get_level(void)
{
    return current_log_level;
}

/**
 * @brief 获取当前系统时间（精确到毫秒）
 * @param time_str 存储时间字符串的缓冲区
 * @param buf_size 缓冲区大小
 */
void log_get_timestamp(char* time_str, size_t buf_size)
{
    // 使用RTC获取实际时间
    RTC_GetCurrentTime(time_str, buf_size);
}

/**
 * @brief 输出日志
 * @param level 日志级别
 * @param file 文件名
 * @param line 行号
 * @param func 函数名
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void log_output(LogLevel level, const char* file, int line, const char* func, const char* format, ...)
{
    // 检查日志级别是否有效
    if (level < LOG_LEVEL_DEBUG || level > LOG_LEVEL_ERROR) {
        return;
    }

    // 检查是否应该输出此级别的日志
    if (level < current_log_level) {
        return;
    }

    // 提取文件名（去掉路径）
    const char* filename = strrchr(file, '/');
    if (filename == NULL) {
        filename = strrchr(file, '\\');
        if (filename == NULL) {
            filename = file;
        } else {
            filename++; // 跳过反斜杠
        }
    } else {
        filename++; // 跳过斜杠
    }

    // 构建日志前缀，包含时间戳
    char time_str[16]; // 足够存储 HH:MM:SS.mmm 格式的时间字符串
    log_get_timestamp(time_str, sizeof(time_str));
    printf("[%s][%c][%s:%d][%s] ", time_str, LOG_LEVEL_STR[level], filename, line, func);

    // 处理可变参数
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    // 添加换行符
    printf("\r\n");
}
