/**
 * @file rtc.c
 * @brief RTC配置和功能实现
 */

#include "rtc.h"
#include <string.h>
#include <stdio.h>
/* RTC句柄定义 */
RTC_HandleTypeDef hrtc;

/**
 * @brief RTC初始化函数
 */
void MX_RTC_Init(void)
{
    /* 使能电源时钟 */
    __HAL_RCC_PWR_CLK_ENABLE();
    /* 允许访问备份域 */
    HAL_PWR_EnableBkUpAccess();

    /* 配置RTC */
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    /* 初始化RTC */
    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
        Error_Handler();
    }

    /* 检查是否首次上电 */
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)
    {
        /* 设置默认时间 */
        RTC_TimeTypeDef sTime = {0};
        RTC_DateTypeDef sDate = {0};

        /* 设置时间 */
        sTime.Hours = 0x0;
        sTime.Minutes = 0x0;
        sTime.Seconds = 0x0;
        sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        sTime.StoreOperation = RTC_STOREOPERATION_RESET;
        if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
        {
            Error_Handler();
        }

        /* 设置日期 */
        sDate.WeekDay = RTC_WEEKDAY_MONDAY;
        sDate.Month = RTC_MONTH_JANUARY;
        sDate.Date = 0x1;
        sDate.Year = 0x0;
        if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
        {
            Error_Handler();
        }
    }
}

/**
 * @brief 获取当前时间
 * @param time_str 存储时间字符串的缓冲区
 * @param buf_size 缓冲区大小
 */
void RTC_GetCurrentTime(char* time_str, size_t buf_size)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    /* 获取当前时间 */
    if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        /* 获取失败，使用默认值 */
        snprintf(time_str, buf_size, "00:00:00.000");
        return;
    }

    /* 获取当前日期 */
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    /* 获取毫秒部分 */
    uint32_t milliseconds = HAL_GetTick() % 1000;

    /* 格式化时间字符串 */
    snprintf(time_str, buf_size, "%02d:%02d:%02d.%03lu", 
             sTime.Hours, sTime.Minutes, sTime.Seconds, milliseconds);
}
