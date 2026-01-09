
/**
 * @file uart_rx_task.h
 * @brief 串口接收任务头文件
 */

#ifndef __UART_RX_TASK_H
#define __UART_RX_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* 函数声明 */
void UART_Start_Receive(void);
void StartUartRxTask(void *argument);
void UART_Rx_Task_Create(void);

#ifdef __cplusplus
}
#endif

#endif /* __UART_RX_TASK_H */
