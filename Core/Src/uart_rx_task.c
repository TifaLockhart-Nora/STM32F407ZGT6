
/**
 * @file uart_rx_task.c
 * @brief 串口接收任务实现
 */

#include "main.h"
#include "usart.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>
#include "uart_rx_task.h"
/* 接收缓冲区大小 */
#define UART_RX_BUFFER_SIZE 256

/* 接收缓冲区 */
static uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];
static volatile uint16_t uart_rx_data_len = 0;
static volatile uint8_t uart_rx_data_ready = 0;

/* 缓冲区状态标志 */
static volatile uint8_t uart_rx_buffer_overflow = 0;

/* 单字节接收缓冲区 */
static uint8_t uart_rx_byte;

/* 接收任务句柄 */


/**
 * @brief 启动UART接收
 * @retval None
 */
void UART_Start_Receive(void)
{
    /* 初始化接收计数器 */
    uart_rx_data_len = 0;
    
    /* 启动UART单字节中断接收 */
    HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);
}

/**
 * @brief UART接收完成回调函数
 * @param huart UART句柄
 * @retval None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        /* 将接收到的字节存入缓冲区 */
        if (uart_rx_data_len < UART_RX_BUFFER_SIZE - 1)
        {
            uart_rx_buffer[uart_rx_data_len++] = uart_rx_byte;
            
            /* 检测结束符（回车换行）或缓冲区满 */
            if (uart_rx_byte == '\n' || uart_rx_byte == '\r' || uart_rx_data_len >= UART_RX_BUFFER_SIZE - 1)
            {
                /* 字符串结束标志 */
                uart_rx_buffer[uart_rx_data_len] = '\0';
                
                /* 设置数据就绪标志 */
                uart_rx_data_ready = 1;
            }
        }
        
        /* 重新启动单字节接收 */
        HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);
    }
}

/**
 * @brief UART错误回调函数
 * @param huart UART句柄
 * @retval None
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        /* 处理错误，重新启动接收 */
        HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);
    }
}

/**
 * @brief UART接收任务
 * @param argument 任务参数
 * @retval None
 */
void StartUartRxTask(void *argument)
{
    /* 启动UART接收 */
    UART_Start_Receive();

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
            // 这里可以添加数据处理代码
            // 例如：解析命令、存储数据等
            
            /* 打印接收到的数据 */
            printf("Received %d bytes: ", rx_len);
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
 * @brief 创建UART接收任务
 * @retval None
 */
void UART_Rx_Task_Create(void)
{
    /* 创建UART接收任务 */
        // 使用FreeRTOS原生API创建任务
    xTaskCreate(
        StartUartRxTask,         // 任务函数
        "uartRxTask",           // 任务名称
        256,                    // 栈大小(单位是字，不是字节)
        NULL,                   // 参数
        tskIDLE_PRIORITY + 2,   // 优先级
        NULL       // 任务句柄
    );
}
