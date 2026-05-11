/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  * 
 * @par dependencies 
 * - bsp_led_driver.h
 * - stdint.h
 * - bsp_led_handler.h
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
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
///1.编译系统提供的头文件 compiling system provided headers

/***2.	MCU Layer********************* */
//2.1 CPU Driver
#include "cmsis_os.h"
//2.2 Core
#include "iic_hal.h"

/***3.	OS Layer********************* */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/***4.	compiling BSP provided headers******* */

#include "AHT21.h"
#include "MPU6050.h"

#include "delay.h"
#include "system_adaptation.h"//LED
#include "elog.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */


/**四个interface分别从core，driver还有OS层传过来，�?�过handler层传给driver�???? */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */


/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */


/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

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
  /* add threads, ... */




/**  humi_temp_TaskHandle = osThreadNew(aht21_handler_thread_func, //taskfunc 是weak 在handler.c中重定义实现
                                      &input_arg, 
                                      &humi_temp_Task_attributes);
  //user_task 是发送队列消息的线程 
  user_TaskHandle = osThreadNew(user_task_func,
                                NULL,  
                                &user_Task_attributes); */
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
  /**----等待调度器成功之后在进行初始化，
   * 避免在初始化过程中被调度器打断，导致问题
   * 避免后续的任务调度vtaskdelay hard fault--必须用osdelay */
  // delay_ms(500);-
  osDelay(500);
  bsp_mpu_driver_t p_mpu_driver_instance={0};
  mpu_data_t p_data={0};
  MPU6050_Test(&p_mpu_driver_instance);
	for(;;)   
	{	   
    p_mpu_driver_instance.pf_get_accel(&p_mpu_driver_instance, &p_data);
    log_i("ax = %f, ay = %f, az = %f\r\n",\
       p_data.ax, p_data.ay, p_data.az);
    osDelay(200);
    
	}
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

