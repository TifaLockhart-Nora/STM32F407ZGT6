/**
 * @file high_res_timer.c
 * @brief 高精度定时器实现，提供毫秒级时间戳
 */

#include "high_res_timer.h"
#include "stm32f4xx_hal.h"

/* 定时器句柄 */
TIM_HandleTypeDef htim;

/**
 * @brief 初始化高精度定时器
 * @note 使用TIM2，配置为1ms精度
 */
void HighResTimer_Init(void)
{
    /* 使能TIM2时钟 */
    __HAL_RCC_TIM2_CLK_ENABLE();

    /* 定时器配置 */
    htim.Instance = TIM2;
    htim.Init.Prescaler = (SystemCoreClock / 1000000) - 1;  // 1MHz计数频率
    htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim.Init.Period = 0xFFFFFFFF;  // 最大周期，32位定时器
    htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim) != HAL_OK)
    {
        Error_Handler();
    }

    /* 启动定时器 */
    if (HAL_TIM_Base_Start(&htim) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief 获取高精度毫秒时间戳
 * @return 当前毫秒时间戳
 */
uint32_t HighResTimer_GetMs(void)
{
    /* 获取定时器计数值，转换为毫秒 */
    return __HAL_TIM_GET_COUNTER(&htim) / 1000;
}

/**
 * @brief 获取高精度微秒时间戳
 * @return 当前微秒时间戳
 */
uint32_t HighResTimer_GetUs(void)
{
    /* 获取定时器计数值，转换为微秒 */
    return __HAL_TIM_GET_COUNTER(&htim);
}
