
/**
 * @file uart_dma_rx.h
 * @brief 串口DMA接收头文件
 */

#ifndef __UART_DMA_RX_H
#define __UART_DMA_RX_H

#ifdef __cplusplus
extern "C" {
#endif

/* 函数声明 */
void UART_DMA_Start_Receive(void);
void StartUartDmaRxTask(void *argument);
void UART_DMA_Rx_Task_Create(void);

#ifdef __cplusplus
}
#endif

#endif /* __UART_DMA_RX_H */
