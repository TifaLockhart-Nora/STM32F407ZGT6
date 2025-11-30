/**
 * @file rtc.h
 * @brief RTC配置和功能头文件
 */

#ifndef __RTC_H
#define __RTC_H

#include "main.h"

/* RTC句柄声明 */
extern RTC_HandleTypeDef hrtc;

/* RTC初始化函数 */
void MX_RTC_Init(void);

/* 获取当前时间 */
void RTC_GetCurrentTime(char* time_str, size_t buf_size);

#endif /* __RTC_H */
