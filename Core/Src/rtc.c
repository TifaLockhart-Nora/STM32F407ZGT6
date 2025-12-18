/**
 * @file rtc.c
 * @brief RTC配置和功能实现
 */

#include "rtc.h"
#include <string.h>
#include <stdio.h>
#include "log.h"
/* RTC句柄定义 */
RTC_HandleTypeDef hrtc;

/* 标记使用的时钟源: 0=LSI, 1=LSE */
static uint8_t rtc_clock_source = 0;

/**
 * @brief RTC MSP 初始化（时钟配置）
 */
void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{
    if(rtcHandle->Instance == RTC)
    {
        /* 使能电源时钟 */
        __HAL_RCC_PWR_CLK_ENABLE();
        /* 允许访问备份域 */
        HAL_PWR_EnableBkUpAccess();
        
        /* 使能 LSE 作为 RTC 时钟源（外部 32.768kHz 晶振） */
        RCC_OscInitTypeDef RCC_OscInitStruct = {0};
        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
        RCC_OscInitStruct.LSEState = RCC_LSE_ON;
        RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
        if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        {
            LOG_WARNING("LSE start failed, switching to LSI for RTC clock source.\r\n");
            /* LSE 启动失败，可能没有外部晶振，回退到 LSI */
            RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
            RCC_OscInitStruct.LSIState = RCC_LSI_ON;
            RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
            if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
            {
                Error_Handler();
            }
            
            /* 配置 RTC 时钟源为 LSI */
            RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
            PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
            PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
            if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
            {
                Error_Handler();
            }
            rtc_clock_source = 0;  /* LSI */
        }
        else
        {
            LOG_INFO("LSE started successfully for RTC clock source.\r\n");
            /* 配置 RTC 时钟源为 LSE */
            RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
            PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
            PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
            if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
            {
                Error_Handler();
            }
            rtc_clock_source = 1;  /* LSE */
        }
        
        /* 使能 RTC 时钟 */
        __HAL_RCC_RTC_ENABLE();
    }
}

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
    /* 先使用 LSE 的预分频值，HAL_RTC_Init 会调用 MspInit 配置时钟
     * LSE (32.768kHz): 32768 / (127+1) / (255+1) = 1Hz
     */
    hrtc.Init.SynchPrediv = 255;  /* LSE: 255 */
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    /* 初始化RTC - 这会调用 HAL_RTC_MspInit 配置时钟源 */
    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* 如果实际使用的是 LSI，需要重新配置预分频值 */
    if (rtc_clock_source == 0)
    {
        /* LSI (~32kHz): 32000 / (127+1) / (249+1) = 1Hz */
        hrtc.Init.SynchPrediv = 249;
        
        /* 重新初始化 RTC 以应用新的预分频值 */
        if (HAL_RTC_Init(&hrtc) != HAL_OK)
        {
            Error_Handler();
        }
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
    }    /* 获取当前日期（顺序要求：先读时间再读日期） */
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    /* 使用 RTC 子秒计算毫秒
     * SubSeconds 是递减计数器：SynchPrediv → 0
     * 毫秒 = (SynchPrediv - SubSeconds) * 1000 / (SynchPrediv + 1)
     */
    uint32_t sync = hrtc.Init.SynchPrediv;
    uint32_t subsec = sTime.SubSeconds;
    uint32_t milliseconds = 0;
    
    if (sync > 0 && subsec <= sync)
    {
        milliseconds = ((sync - subsec) * 1000U) / (sync + 1U);
    }
      /* 格式化时间字符串：HH:MM:SS.mmm */
    /* 手动格式化，避免 snprintf 的兼容性问题 */
    if (buf_size >= 13)  // 需要至少 13 字节: "HH:MM:SS.mmm\0"
    {
        time_str[0] = '0' + (sTime.Hours / 10);
        time_str[1] = '0' + (sTime.Hours % 10);
        time_str[2] = ':';
        time_str[3] = '0' + (sTime.Minutes / 10);
        time_str[4] = '0' + (sTime.Minutes % 10);
        time_str[5] = ':';
        time_str[6] = '0' + (sTime.Seconds / 10);
        time_str[7] = '0' + (sTime.Seconds % 10);
        time_str[8] = '.';
        time_str[9] = '0' + (milliseconds / 100);
        time_str[10] = '0' + ((milliseconds / 10) % 10);
        time_str[11] = '0' + (milliseconds % 10);
        time_str[12] = '\0';
    }
    else if (buf_size > 0)
    {
        time_str[0] = '\0';
    }
}

/**
 * @brief 获取RTC时钟源
 * @return 0=LSI, 1=LSE
 */
uint8_t RTC_GetClockSource(void)
{
    return rtc_clock_source;
}
