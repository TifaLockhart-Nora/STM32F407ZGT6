/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "sdio.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"
#include "uart_dma_rx.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <inttypes.h>
#include "demo.h"
#include "test/test.h"
#include "rtc.h"
#include "lcd.h"
#include "lvgl.h"
#include "lvgl_demo.h"
#include "sram.h"
#include "uart_dma_rx.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#include "log.h"

HAL_StatusTypeDef SD_Init(void)
{
  HAL_StatusTypeDef status;

  // 初始化SD�?????
  LOG_INFO("Initializing SD card...");

  // 禁用SDIO外设
  __HAL_RCC_SDIO_CLK_DISABLE();
  osDelay(10);

  // 重新启用SDIO外设
  __HAL_RCC_SDIO_CLK_ENABLE();
  osDelay(10);

  // 使用较低的时钟频率进行初始化
  hsd.Init.ClockDiv = 118; // �?????400kHz，符合SD卡初始化规范
  LOG_INFO("Setting SDIO clock to low speed (ClockDiv=%d)", hsd.Init.ClockDiv);

  status = HAL_SD_Init(&hsd);
  if (status != HAL_OK)
  {
    LOG_ERROR("SD card initialization failed with status: %d", status);

    // 尝试使用更保守的设置再次初始�?????
    LOG_INFO("Attempting with more conservative settings...");
    hsd.Init.ClockDiv = 178; // 更低的时钟频�?????
    status = HAL_SD_Init(&hsd);
    if (status != HAL_OK)
    {
      LOG_ERROR("Second initialization attempt also failed with status: %d", status);
      return status;
    }
  }
  LOG_INFO("SD card initialized successfully at low speed");

  // 等待SD卡稳�?????
  osDelay(100); // 等待100ms

  // 逐步提高时钟频率
  LOG_INFO("Increasing SDIO clock frequency to medium speed...");
  __HAL_SD_DISABLE(&hsd);
  hsd.Init.ClockDiv = 10; // �?????4MHz
  status = HAL_SD_Init(&hsd);
  if (status != HAL_OK)
  {
    LOG_ERROR("Failed to increase SDIO clock frequency to medium speed, status: %d", status);
    // 继续使用较低的时钟频�?????
  }
  else
  {
    LOG_INFO("SDIO clock frequency increased to medium speed successfully");
    osDelay(50); // 等待50ms

    // 尝试进一步提高时钟频�?????
    LOG_INFO("Increasing SDIO clock frequency to high speed...");
    __HAL_SD_DISABLE(&hsd);
    hsd.Init.ClockDiv = 1; // �?????16MHz
    status = HAL_SD_Init(&hsd);
    if (status != HAL_OK)
    {
      LOG_ERROR("Failed to increase SDIO clock frequency to high speed, status: %d", status);
      // 回�??到中等�?�度
      __HAL_SD_DISABLE(&hsd);
      hsd.Init.ClockDiv = 10;
      HAL_SD_Init(&hsd);
      LOG_INFO("Reverted to medium speed");
    }
    else
    {
      LOG_INFO("SDIO clock frequency increased to high speed successfully");
    }
  }

  // 配置SD卡为4位�?�线宽度
  LOG_INFO("Configuring SD card for 4-bit bus width...");
  status = HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_4B);
  if (status != HAL_OK)
  {
    LOG_ERROR("Failed to configure 4-bit bus width, status: %d", status);
    // 继续使用1位�?�线宽度
    LOG_INFO("Continuing with 1-bit bus width");
  }
  else
  {
    LOG_INFO("SD card configured for 4-bit bus width");
  }

  // 设置SD卡块大小
  status = HAL_SD_ConfigBlockSize(&hsd, 512);
  if (status != HAL_OK)
  {
    LOG_ERROR("Failed to set block size, status: %d", status);
  }

  return HAL_OK;
}

void show_sdcard_info(void)
{
  // �?????查SD卡状�?????
  HAL_SD_CardInfoTypeDef cardInfo;
  if (HAL_SD_GetCardInfo(&hsd, &cardInfo) != HAL_OK)
  {
    LOG_ERROR("Failed to get SD card info");
  }
  else
  {
    LOG_INFO("SD Card Info:");
    LOG_INFO("  CardType: %d", cardInfo.CardType);
    LOG_INFO("  CardVersion: %d", cardInfo.CardVersion);
    LOG_INFO("  BlockNbr: %lu", (unsigned long)cardInfo.BlockNbr);
    LOG_INFO("  BlockSize: %lu", (unsigned long)cardInfo.BlockSize);
    LOG_INFO("  LogBlockNbr: %lu", (unsigned long)cardInfo.LogBlockNbr);
    LOG_INFO("  LogBlockSize: %lu", (unsigned long)cardInfo.LogBlockSize); // 计算SD卡�?�容�????? (BlockNbr * BlockSize)
    uint64_t totalBytes = (uint64_t)cardInfo.BlockNbr * cardInfo.BlockSize;
    uint32_t totalMB = (uint32_t)(totalBytes / (1024 * 1024));
    uint32_t totalGB_int = totalMB / 1024;
    uint32_t totalGB_frac = (totalMB % 1024) * 100 / 1024; // 小数部分，保�?????2�?????    LOG_INFO("  Total Capacity: %lu MB (%lu.%02lu GB)", totalMB, totalGB_int, totalGB_frac);

    // 浮点打印测试
    float totalGB_float = (float)totalMB / 1024.0f;
    LOG_INFO("  Total Capacity (float): %.2f GB", totalGB_float);
    LOG_INFO("  totalBytes = %llu", totalBytes);
  }

  // 使用 FatFs 获取磁盘使用情况

  FATFS *fs;
  DWORD fre_clust, fre_sect, tot_sect;

  // 获取卷信息和空闲簇数�?????
  FRESULT res = f_getfree(SDPath, &fre_clust, &fs);
  if (res == FR_OK)
  {
    // 计算总扇区数和空闲扇区数
    tot_sect = (fs->n_fatent - 2) * fs->csize; // 总扇区数
    fre_sect = fre_clust * fs->csize;          // 空闲扇区�?????

    // 转换�????? MB (扇区大小通常�????? 512 字节)
    uint32_t totalMB = tot_sect / 2048; // tot_sect * 512 / 1024 / 1024
    uint32_t freeMB = fre_sect / 2048;  // fre_sect * 512 / 1024 / 1024
    uint32_t usedMB = totalMB - freeMB;
    LOG_INFO("Disk Space Info:");
    LOG_INFO("  Total: %lu MB", totalMB);
    LOG_INFO("  Used:  %lu MB", usedMB);
    LOG_INFO("  Free:  %lu MB", freeMB);
    uint32_t usagePercent = usedMB * 100 / totalMB;
    uint32_t usageFrac = (usedMB * 1000 / totalMB) % 10; // 小数点后�?????�?????
    LOG_INFO("  Usage: %lu.%lu%%", usagePercent, usageFrac);
  }
  else
  {
    LOG_ERROR("Failed to get free space: %d", res);
  }
}

void test_sd_read_write(void)
{
  FRESULT res;
  UINT bytesWritten;
  // 创建文件
  LOG_INFO("Attempting to create file...");

  // �?????查SD卡状�?????
  HAL_SD_CardStateTypeDef cardState = HAL_SD_GetCardState(&hsd);
  LOG_INFO("SD card state before file creation: %d", cardState);

  char filePath[20];
  sprintf(filePath, "%stest.txt", SDPath);
  LOG_INFO("Opening file: %s", filePath);

  res = f_open(&SDFile, filePath, FA_CREATE_ALWAYS | FA_WRITE);

  if (res == FR_OK)
  {
    // 写入数据
    LOG_INFO("File created successfully, preparing to write data...");
    const char *data = "Hello from STM32F407 with FatFs!";
    LOG_INFO("Data length: %d bytes", strlen(data));

    // 添加超时机制
    uint32_t writeStartTime = HAL_GetTick();
    LOG_INFO("Starting write operation at tick: %lu", writeStartTime);

    FRESULT r = f_write(&SDFile, data, strlen(data), &bytesWritten);

    uint32_t writeEndTime = HAL_GetTick();
    LOG_INFO("Write operation completed at tick: %lu, duration: %lu ms", writeEndTime, writeEndTime - writeStartTime);

    if (r == FR_OK)
    {
      LOG_INFO("Data written successfully, bytes written: %d", bytesWritten);
    }
    else
    {
      LOG_ERROR("Failed to write data: %d", r);
    }
    // 关闭文件
    LOG_INFO("Closing file...");
    f_close(&SDFile);
    LOG_INFO("File closed");
  }
  else
  {
    LOG_ERROR("Failed to create file: %d", res);
  }

  // 读取文件
  LOG_INFO("Attempting to read file...");
  res = f_open(&SDFile, filePath, FA_READ);
  if (res == FR_OK)
  {
    char buffer[256];
    UINT bytesRead;
    res = f_read(&SDFile, buffer, sizeof(buffer) - 1, &bytesRead);
    buffer[bytesRead] = '\0'; // 确保字符串终�?????
    if (res == FR_OK)
    {
      LOG_INFO("Data read from file:bytesRead = %d, %s", bytesRead, buffer);
    }
    else
    {
      LOG_ERROR("Failed to read file: %d", res);
    }
    f_close(&SDFile);
  }
}
static void start_task(void *arg)
{

  LOG_INFO("Attempting to mount SD card...");
  FRESULT mountResult = f_mount(&SDFatFS, SDPath, 1);
  LOG_INFO("Mount result: %d,SDPath = %s", mountResult, SDPath);

  switch (mountResult)
  {
  case FR_OK:
    LOG_INFO("SD card mounted successfully");
    break;
  case FR_NO_FILESYSTEM:
    LOG_ERROR("No FAT filesystem on SD card");
    break;
  case FR_DISK_ERR:
    LOG_ERROR("Disk I/O error");
    break;
  case FR_NOT_READY:
    LOG_ERROR("SD card not ready");
    break;
  default:
    LOG_ERROR("Mount failed with error code: %d", mountResult);
    break;
  }

  if (mountResult == FR_OK)
  {

    show_sdcard_info();
    test_sd_read_write();
  }
  else
  {
    LOG_ERROR("Failed to mount SD card");
  }
  demo();
  test();
  while (1)
  {
    // LOG_INFO("Task1 is running");
    // 翻转led0
    HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
    vTaskDelay(500);
  }
}

void process_task(void *arg)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(1000); // 1秒周�?????

  while (1)
  {
    // LOG_INFO("Task2 is running");
    // 使用绝对延时：确保任务以精确�????? 1 秒周期执�?????
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

void test_sram()
{
  /* 测试外部 SRAM 读写 */
  {
    volatile uint16_t *sram_test = (volatile uint16_t *)0x68000000;
    uint8_t sram_ok = 1;
    uint16_t read_val;
    
    LOG_INFO("Testing External SRAM at 0x68000000...\r\n");
    
    /* 测试1: �??单的单字写读 */
    sram_test[0] = 0x1234;
    read_val = sram_test[0];
    // LOG_INFO("Single word test: wrote 0x1234, read 0x%04X\r\n", read_val);
    if(read_val != 0x1234) {
      LOG_ERROR("Single word test FAILED!\r\n");
      sram_ok = 0;
    }
    
    /* 测试2: 另一个地�?? */
    sram_test[100] = 0xABCD;
    read_val = sram_test[100];
    // LOG_INFO("Offset test: wrote 0xABCD at [100], read 0x%04X\r\n", read_val);
    if(read_val != 0xABCD) {
      LOG_ERROR("Offset test FAILED!\r\n");
      sram_ok = 0;
    }
      /* 测试3: 连续写读 - 每写�??个立即验�?? */
    if(sram_ok) {
      for(int i = 0; i < 100; i++) {
        sram_test[i] = (uint16_t)(0x5A00 + i);
        __DSB();  /* 数据同步屏障，确保写入完�?? */
        read_val = sram_test[i];
        // LOG_INFO("Wrote 0x%04X, read 0x%04X at index %d\r\n", (0x5A00 + i), read_val, i);
        if(read_val != (uint16_t)(0x5A00 + i)) {
          LOG_ERROR("Write-verify failed at %d: wrote 0x%04X, read 0x%04X\r\n", 
                    i, (0x5A00 + i), read_val);
          sram_ok = 0;
          break;
        }
      }
      
      /* 再次全部读取验证 */
      if(sram_ok) {
        __DSB();
        for(int i = 0; i < 100; i++) {
          read_val = sram_test[i];
          // LOG_INFO("Re-read 0x%04X test 0x%04X at index %d\r\n", read_val, (0x5A00 + i), i);
          if(read_val != (uint16_t)(0x5A00 + i)) {
            LOG_ERROR("Re-read failed at %d: expected 0x%04X, read 0x%04X\r\n", 
                      i, (0x5A00 + i), read_val);
            sram_ok = 0;
            break;
          }
        }
      }
    }
    
    if(sram_ok) {
      LOG_INFO("External SRAM test PASSED!\r\n");
    } else {
      LOG_ERROR("External SRAM test FAILED! Check hardware connection.\r\n");
      /* 不再死循环，继续运行看看其他问题 */
    }
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  LCD_DMA_Init();  /* 初始化LCD DMA传输 */
  MX_USART1_UART_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  MX_FSMC_Init();
  // MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  MX_RTC_Init();
  // sram_init();
  log_set_level(LOG_LEVEL_DEBUG);
  // lcd_init();
  /* 打印RTC时钟源信�????? */
  if (RTC_GetClockSource() == 1)
  {
    LOG_INFO("RTC clock source: LSE (32.768kHz)\r\n");
  }
  else
  {
    LOG_INFO("RTC clock source: LSI (~32kHz, less accurate)\r\n");
  }
  test_sram();
  // lcd_show_string(10, 10, 220, 32, 32, "STM32", RED);
  // lcd_show_string(10, 47, 220, 24, 24, "Timer", RED);
  // lcd_show_string(10, 76, 220, 16, 16, "ATOM@ALIENTEK", RED);
  // xTaskCreate(start_task, "Task1", 2048, NULL, 1, NULL);
  xTaskCreate(process_task, "Task2", 128, NULL, 1, NULL);
  // UART_Rx_Task_Create();  // 创建串口接收任务
  UART_DMA_Rx_Task_Create();
  lvgl_demo();
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();
  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
