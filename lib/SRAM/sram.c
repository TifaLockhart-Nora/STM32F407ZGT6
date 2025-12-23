/**
 ****************************************************************************************************
 * @file        sram.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2021-11-04
 * @brief       �ⲿSRAM ��������
 * @license     Copyright (c) 2020-2032, �������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32F407������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20211103
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "sram.h"
// #include "./SYSTEM/usart/usart.h"
#include "stm32f4xx_hal.h"

/**
 * @brief       ��ʼ�� �ⲿSRAM
 * @param       ��
 * @retval      ��
 */
void sram_init(void)
{
    SRAM_HandleTypeDef sram_handle; /* SRAM��� */
    GPIO_InitTypeDef gpio_init_struct;
    FSMC_NORSRAM_TimingTypeDef fsmc_readwritetim;

    SRAM_CS_GPIO_CLK_ENABLE();    /* SRAM_CS��ʱ��ʹ�� */
    SRAM_WR_GPIO_CLK_ENABLE();    /* SRAM_WR��ʱ��ʹ�� */
    SRAM_RD_GPIO_CLK_ENABLE();    /* SRAM_RD��ʱ��ʹ�� */
    __HAL_RCC_FSMC_CLK_ENABLE();  /* ʹ��FSMCʱ�� */
    __HAL_RCC_GPIOD_CLK_ENABLE(); /* ʹ��GPIODʱ�� */
    __HAL_RCC_GPIOE_CLK_ENABLE(); /* ʹ��GPIOEʱ�� */
    __HAL_RCC_GPIOF_CLK_ENABLE(); /* ʹ��GPIOFʱ�� */
    __HAL_RCC_GPIOG_CLK_ENABLE(); /* ʹ��GPIOGʱ�� */

    gpio_init_struct.Pin = SRAM_CS_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;
    gpio_init_struct.Pull = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio_init_struct.Alternate = GPIO_AF12_FSMC;
    HAL_GPIO_Init(SRAM_CS_GPIO_PORT, &gpio_init_struct); /* SRAM_CS����ģʽ���� */

    gpio_init_struct.Pin = SRAM_WR_GPIO_PIN;
    HAL_GPIO_Init(SRAM_WR_GPIO_PORT, &gpio_init_struct); /* SRAM_WR����ģʽ���� */

    gpio_init_struct.Pin = SRAM_RD_GPIO_PIN;
    HAL_GPIO_Init(SRAM_RD_GPIO_PORT, &gpio_init_struct); /* SRAM_CS����ģʽ���� */

    /* PD0,1,4,5,8~15 */
    gpio_init_struct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9 | 
                       GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |
                       GPIO_PIN_14 | GPIO_PIN_15;
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;       /* ���츴�� */
    gpio_init_struct.Pull = GPIO_PULLUP;           /* ���� */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH; /* ���� */
    HAL_GPIO_Init(GPIOD, &gpio_init_struct);

    /* PE0,1,7~15 */
    gpio_init_struct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |
                       GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                       GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &gpio_init_struct);

    /* PF0~5,12~15 */
    gpio_init_struct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |
                       GPIO_PIN_5 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOF, &gpio_init_struct);

    /* PG0~5,10 */
    gpio_init_struct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOG, &gpio_init_struct);

    sram_handle.Instance = FSMC_NORSRAM_DEVICE;
    sram_handle.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;

    sram_handle.Init.NSBank = (SRAM_FSMC_NEX == 1) ? FSMC_NORSRAM_BANK1 : \
                                 (SRAM_FSMC_NEX == 2) ? FSMC_NORSRAM_BANK2 : \
                                 (SRAM_FSMC_NEX == 3) ? FSMC_NORSRAM_BANK3 : 
                                                        FSMC_NORSRAM_BANK4; /* ��������ѡ��FSMC_NE1~4 */
    sram_handle.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;        /* ��ַ/�����߲����� */
    sram_handle.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;                    /* SRAM */
    sram_handle.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;       /* 16λ���ݿ��� */
    sram_handle.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;      /* �Ƿ�ʹ��ͻ������,����ͬ��ͻ���洢����Ч,�˴�δ�õ� */
    sram_handle.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;    /* �ȴ��źŵļ���,����ͻ��ģʽ���������� */
    sram_handle.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;         /* �洢�����ڵȴ�����֮ǰ��һ��ʱ�����ڻ��ǵȴ������ڼ�ʹ��NWAIT */
    sram_handle.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;          /* �洢��дʹ�� */
    sram_handle.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;                 /* �ȴ�ʹ��λ,�˴�δ�õ� */
    sram_handle.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;             /* ��дʹ����ͬ��ʱ�� */
    sram_handle.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;     /* �Ƿ�ʹ��ͬ������ģʽ�µĵȴ��ź�,�˴�δ�õ� */
    sram_handle.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;                 /* ��ֹͻ��д */
    /* FSMC��ʱ����ƼĴ��� */
    fsmc_readwritetim.AddressSetupTime = 0x02;                              /* ��ַ����ʱ�䣨ADDSET��Ϊ2��HCLK 1/168M=6ns*2=12ns */
    fsmc_readwritetim.AddressHoldTime = 0x00;                               /* ��ַ����ʱ�䣨ADDHLD��ģʽAδ�õ� */
    fsmc_readwritetim.DataSetupTime = 0x08;                                 /* ���ݱ���ʱ��Ϊ8��HCLK =6*8= 48ns */
    fsmc_readwritetim.BusTurnAroundDuration = 0x00;
    fsmc_readwritetim.AccessMode = FSMC_ACCESS_MODE_A;                      /* ģʽA */
    HAL_SRAM_Init(&sram_handle, &fsmc_readwritetim, &fsmc_readwritetim);
}

/**
 * @brief       ��SRAMָ����ַд��ָ����������
 * @param       pbuf    : ���ݴ洢��
 * @param       addr    : ��ʼд��ĵ�ַ(���32bit)
 * @param       datalen : Ҫд����ֽ���(���32bit)
 * @retval      ��
 */
void sram_write(uint8_t *pbuf, uint32_t addr, uint32_t datalen)
{
    for (; datalen != 0; datalen--)
    {
        *(volatile uint8_t *)(SRAM_BASE_ADDR + addr) = *pbuf;
        addr++;
        pbuf++;
    }
}

/**
 * @brief       ��SRAMָ����ַ��ȡָ����������
 * @param       pbuf    : ���ݴ洢��
 * @param       addr    : ��ʼ��ȡ�ĵ�ַ(���32bit)
 * @param       datalen : Ҫ��ȡ���ֽ���(���32bit)
 * @retval      ��
 */
void sram_read(uint8_t *pbuf, uint32_t addr, uint32_t datalen)
{
    for (; datalen != 0; datalen--)
    {
        *pbuf++ = *(volatile uint8_t *)(SRAM_BASE_ADDR + addr);
        addr++;
    }
}

/*******************���Ժ���**********************************/

/**
 * @brief       ���Ժ��� ��SRAMָ����ַд��1���ֽ�
 * @param       addr    : ��ʼд��ĵ�ַ(���32bit)
 * @param       data    : Ҫд����ֽ�
 * @retval      ��
 */
void sram_test_write(uint32_t addr, uint8_t data)
{
    sram_write(&data, addr, 1); /* д��1���ֽ� */
}

/**
 * @brief       ���Ժ��� ��SRAMָ����ַ��ȡ1���ֽ�
 * @param       addr    : ��ʼ��ȡ�ĵ�ַ(���32bit)
 * @retval      ��ȡ��������(1���ֽ�)
 */
uint8_t sram_test_read(uint32_t addr)
{
    uint8_t data;
    sram_read(&data, addr, 1); /* ��ȡ1���ֽ� */
    return data;
}
