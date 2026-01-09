
/**
 * @file uart_dma_rx.c
 * @brief 串口DMA接收实现
 */

#include "main.h"
#include "usart.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>

/* 接收缓冲区大小 */
#define UART_RX_BUFFER_SIZE 5

/* 接收缓冲区 */
static uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];
static volatile uint16_t uart_rx_data_len = 0;
static volatile uint8_t uart_rx_data_ready = 0;

/* DMA接收状态 */
static volatile uint8_t uart_rx_dma_idle_flag = 0;

/**
 * @brief 启动串口DMA接收
 * @retval None
 */
void UART_DMA_Start_Receive(void)
{
    /* 启动DMA接收 */
    HAL_UART_Receive_DMA(&huart1, uart_rx_buffer, UART_RX_BUFFER_SIZE);

    /* 启用IDLE中断 */
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
}

/**
 * @brief 串口IDLE中断回调函数
 * @param huart UART句柄
 * @retval None
 */
void HAL_UART_IdleCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        /* 停止DMA接收 */
        HAL_UART_DMAStop(&huart1);

        /* 获取已接收数据长度 */
        uart_rx_data_len = UART_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);

        /* 设置数据就绪标志 */
        if (uart_rx_data_len > 0)
        {
            uart_rx_data_ready = 1;
        }

        /* 设置IDLE标志 */
        uart_rx_dma_idle_flag = 1;

        /* 重新启动DMA接收 */
        UART_DMA_Start_Receive();
    }
}

/**
 * @brief 串口DMA接收任务
 * @param argument 任务参数
 * @retval None
 */
void StartUartDmaRxTask(void *argument)
{
    /* 启动串口DMA接收 */
    UART_DMA_Start_Receive();

    /* 任务循环 */
    for(;;)
    {
        /* 检查是否有数据就绪 */
        if (uart_rx_data_ready)
        {
            /* 清除数据就绪标志 */
            uart_rx_data_ready = 0;

            /* 保存接收到的数据长度，用于后续处理 */
            uint16_t rx_len = uart_rx_data_len;

            /* 处理接收到的数据 */
            printf("DMA Received %d bytes: ", rx_len);
            for(uint16_t i = 0; i < rx_len; i++) {
                printf("%02X ", uart_rx_buffer[i]);
            }
            printf("\r\n");

            /* 示例：将接收到的数据回显 */
            HAL_UART_Transmit(&huart1, uart_rx_buffer, rx_len, 100);

            /* 重置接收计数器，准备接收下一条数据 */
            uart_rx_data_len = 0;
        }

        /* 任务延时 */
        osDelay(10);
    }
}

/**
 * @brief 创建UART DMA接收任务
 * @retval None
 */
void UART_DMA_Rx_Task_Create(void)
{
    /* 创建UART DMA接收任务 */
    // 使用FreeRTOS原生API创建任务
    xTaskCreate(
        StartUartDmaRxTask,        // 任务函数
        "uartDmaRxTask",           // 任务名称
        256,                       // 栈大小(单位是字，不是字节)
        NULL,                      // 参数
        tskIDLE_PRIORITY + 2,      // 优先级
        NULL                       // 任务句柄
    );
}
