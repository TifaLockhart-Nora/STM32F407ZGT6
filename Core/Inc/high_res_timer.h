/**
 * @file high_res_timer.h
 * @brief 高精度定时器头文件
 */

#ifndef __HIGH_RES_TIMER_H
#define __HIGH_RES_TIMER_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 函数声明 */
void HighResTimer_Init(void);
uint32_t HighResTimer_GetMs(void);
uint32_t HighResTimer_GetUs(void);

#ifdef __cplusplus
}
#endif

#endif /* __HIGH_RES_TIMER_H */
