/**
  ******************************************************************************
  * File Name          : dma.c
  * Description        : This file provides code for the configuration
  *                      of all the requested memory to memory DMA transfers.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "dma.h"

/* USER CODE BEGIN 0 */
#include "lcd.h"

/* LCD DMA 传输句柄 - 使用 DMA2 Stream0 */
DMA_HandleTypeDef hdma_lcd;

/* DMA 传输完成标志 */
volatile uint8_t lcd_dma_transfer_complete = 1;
/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure DMA                                                              */
/*----------------------------------------------------------------------------*/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * Enable DMA controller clock
  */
void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 11, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 11, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

}

/* USER CODE BEGIN 2 */

/**
  * @brief  DMA 传输完成回调
  */
void HAL_DMA_LCD_XferCpltCallback(DMA_HandleTypeDef *hdma)
{
    lcd_dma_transfer_complete = 1;
}

/**
  * @brief  LCD DMA 初始化
  *         使用 DMA2 Stream0 进行内存到内存传输（用于LCD数据）
  * @note   DMA2 才能访问内存到内存传输
  */
void LCD_DMA_Init(void)
{
    /* 配置 DMA2 Stream0 用于 LCD 数据传输 */
    hdma_lcd.Instance = DMA2_Stream0;
    hdma_lcd.Init.Channel = DMA_CHANNEL_0;
    hdma_lcd.Init.Direction = DMA_MEMORY_TO_MEMORY;
    hdma_lcd.Init.PeriphInc = DMA_PINC_ENABLE;       /* 源地址递增 */
    hdma_lcd.Init.MemInc = DMA_MINC_DISABLE;         /* 目标地址不递增(LCD固定地址) */
    hdma_lcd.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;  /* 16位 */
    hdma_lcd.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;     /* 16位 */
    hdma_lcd.Init.Mode = DMA_NORMAL;
    hdma_lcd.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_lcd.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_lcd.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_lcd.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_lcd.Init.PeriphBurst = DMA_PBURST_SINGLE;
    
    if (HAL_DMA_Init(&hdma_lcd) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* 注册 DMA 传输完成回调函数 */
    hdma_lcd.XferCpltCallback = HAL_DMA_LCD_XferCpltCallback;
    
    /* 配置 DMA 中断 */
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}



/* USER CODE END 2 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
