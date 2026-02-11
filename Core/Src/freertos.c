/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tftlcd.h"
#include "iwdg.h"
#include "getdata.h"
#include "ui.h"
#include "usart2task.h"
#include "cameratask.h"
#include "sram.h"
#include "eeprom.h"
#include "flash.h"
#include <stdio.h>




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
/* USER CODE BEGIN Variables */
osThreadId_t lcdTaskHandle;
const osThreadAttr_t lcdTask_attributes = {
  .name = "lcdTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t iwdgTaskHandle;
const osThreadAttr_t iwdgTask_attributes = {
  .name = "iwdgTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};

osThreadId_t getdataTaskHandle;
const osThreadAttr_t getdataTask_attributes = {
  .name = "getdataTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t usart2TaskHandle;
const osThreadAttr_t usart2Task_attributes = {
  .name = "usart2Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t usart3TaskHandle;
const osThreadAttr_t usart3Task_attributes = {
  .name = "usart3Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t cameraTaskHandle;
const osThreadAttr_t cameraTask_attributes = {
  .name = "cameraTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};

osThreadId_t sramTestTaskHandle;
const osThreadAttr_t sramTestTask_attributes = {
  .name = "sramTestTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};

osThreadId_t eepromTestTaskHandle;
const osThreadAttr_t eepromTestTask_attributes = {
  .name = "eepromTestTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};

osThreadId_t flashTestTaskHandle;
const osThreadAttr_t flashTestTask_attributes = {
  .name = "flashTestTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};



/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void LcdDisplayTask(void *argument);
void IwdgFeedTask(void *argument);
void GetDataTask(void *argument);
void Usart2Task(void *argument);
void Usart3Task(void *argument);

void CameraTask(void *argument);
void SramTestTask(void *argument);
void EepromTestTask(void *argument);
void FlashTestTask(void *argument);

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 2 */
void vApplicationIdleHook( void )
{
   /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
   task. It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()). If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
}
/* USER CODE END 2 */

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
}
/* USER CODE END 5 */

/* USER CODE BEGIN PREPOSTSLEEP */
__weak void PreSleepProcessing(uint32_t ulExpectedIdleTime)
{
/* place for user code */
}

__weak void PostSleepProcessing(uint32_t ulExpectedIdleTime)
{
/* place for user code */
}
/* USER CODE END PREPOSTSLEEP */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  lcdTaskHandle = osThreadNew(LcdDisplayTask, NULL, &lcdTask_attributes);
  iwdgTaskHandle = osThreadNew(IwdgFeedTask, NULL, &iwdgTask_attributes);
  getdataTaskHandle = osThreadNew(GetDataTask, NULL, &getdataTask_attributes);
  usart2TaskHandle = osThreadNew(Usart2Task, NULL, &usart2Task_attributes);
  usart3TaskHandle = osThreadNew(Usart3Task, NULL, &usart3Task_attributes);
  //cameraTaskHandle = osThreadNew(CameraTask, NULL, &cameraTask_attributes);
  sramTestTaskHandle = osThreadNew(SramTestTask, NULL, &sramTestTask_attributes);
  eepromTestTaskHandle = osThreadNew(EepromTestTask, NULL, &eepromTestTask_attributes);
  flashTestTaskHandle = osThreadNew(FlashTestTask, NULL, &flashTestTask_attributes);

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/**
 * @brief  看门狗喂狗任务：周期性刷新 IWDG，防止系统复位
 *         IWDG 配置: 预分频128, 重装载999 → 超时约 4s
 *         喂狗周期设为 1s，留有充足裕量
 */
void IwdgFeedTask(void *argument)
{
  for (;;)
  {
    HAL_IWDG_Refresh(&hiwdg);
    osDelay(1000);
  }
}


/**
 * @brief  SRAM 测试任务
 *         初始化 SRAM 并执行读写自检，结果显示在 LCD 上
 *         测试完成后自动删除任务
 */
void SramTestTask(void *argument)
{
    char msg[40];
    uint32_t testedSize = 0;

    /* 等待 LCD 初始化完成 */
    osDelay(500);

    lcd_show_string(10, 600, 460, 24, 24, "SRAM Testing...", BLUE);

    uint8_t result = SRAM_Test(&testedSize);

    if (result == 0)
    {
        if (testedSize >= 1024)
            sprintf(msg, "SRAM Test: PASS (%luMB)", testedSize / 1024);
        else
            sprintf(msg, "SRAM Test: PASS (%luKB)", testedSize);
        lcd_show_string(10, 630, 460, 24, 24, msg, GREEN);
    }
    else
    {
        lcd_show_string(10, 630, 460, 24, 24, "SRAM Test: FAIL !!!", RED);
    }

    /* 测试完成，删除自身 */
    osThreadTerminate(osThreadGetId());
}

/**
 * @brief  EEPROM (AT24C02) 读写测试任务
 *         执行写入/回读自检，结果显示在 LCD 上
 *         测试完成后自动删除任务
 */
void EepromTestTask(void *argument)
{
    char msg[40];
    uint16_t testedSize = 0;

    /* 等待 LCD 初始化完成 */
    osDelay(500);

    lcd_show_string(10, 660, 460, 24, 24, "EEPROM Testing...", BLUE);

    uint8_t result = AT24C02_Test(&testedSize);

    if (result == 0)
    {
        sprintf(msg, "EEPROM Test: PASS (%dB)", testedSize);
        lcd_show_string(10, 690, 460, 24, 24, msg, GREEN);
    }
    else
    {
        lcd_show_string(10, 690, 460, 24, 24, "EEPROM Test: FAIL !!!", RED);
    }

    /* 测试完成，删除自身 */
    osThreadTerminate(osThreadGetId());
}

/**
 * @brief  SPI Flash (W25Q128) 读写测试任务
 *         执行 ID 校验 + 擦写回读自检，结果显示在 LCD 上
 *         测试完成后自动删除任务
 */
void FlashTestTask(void *argument)
{
    char msg[40];
    uint32_t testedSize = 0;

    /* 等待 LCD 初始化完成 */
    osDelay(500);

    lcd_show_string(10, 720, 460, 24, 24, "Flash Testing...", BLUE);

    /* 初始化驱动（含 PB14 CS GPIO 配置） */
    W25Q128_Init();

    uint8_t result = W25Q128_Test(&testedSize);

    if (result == 0)
    {
        if (testedSize >= 1024)
            sprintf(msg, "Flash Test: PASS (%luMB)", testedSize / 1024);
        else
            sprintf(msg, "Flash Test: PASS (%luKB)", testedSize);
        lcd_show_string(10, 750, 460, 24, 24, msg, GREEN);
    }
    else if (result == 1)
    {
        lcd_show_string(10, 750, 460, 24, 24, "Flash Test: FAIL (ID ERR)", RED);
    }
    else
    {
        lcd_show_string(10, 750, 460, 24, 24, "Flash Test: FAIL (R/W ERR)", RED);
    }

    /* 测试完成，删除自身 */
    osThreadTerminate(osThreadGetId());
}

/* USER CODE END Application */

