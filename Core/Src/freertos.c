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
#include "i2c.h"
/***3.	OS Layer********************* */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/***4.	compiling BSP provided headers******* */

#include "AHT21.h"
#include "MPU6050.h"
#include "bsp_mpu_handler.h"
#include "unpack.h"

#include "mid_circle_buffer.h"

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
// extern bsp_mpu_driver_t p_mpu_driver_instance;/******************** */
extern bsp_mpu_handler_t handler_instance;

extern mpu_os_interface_t mpu_os_instance;
extern mpu_yield_interface_t mpu_yield_instance;
extern mpu_timebases_ms_t mpu_timebase_instance;
extern mpu_delay_interface_t mpu_delay_instance;
extern mpu_iic_driver_instance_t MPU_iic_instance;
extern mpu_input_arg_t input_arg_mpu;
TaskHandle_t mpu_handler_threadHandle = NULL;

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
void (*pf_pin_interrupt_callback)(void *, void *) = NULL;
void (*pf_dma_interrupt_callback)(void *, void *) = NULL;

#if 1
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	// INT test	
	// HAL_GPIO_WritePin(INT_FUNC_GPIO_Port, INT_FUNC_Pin, 0); // PA2
	
	//log_i("HAL_GPIO_EXTI_Callback");

	if(NULL != pf_pin_interrupt_callback)
	{
		pf_pin_interrupt_callback(handler_instance.p_mpu_driver, NULL);
		/*
		log_i("mpu6050 data: \r\n accel_x : %f, accel_y : %f, accel_z : %f \r\n gyro_x : %f, gyro_y : %f, gyro_z : %f \r\n temperature : %f \r\n ax : %f, ay : %f, az : %f\r\n gx : %f, gy : %f, gz : %f\r\n",
			  mpu6050_data.accel_x_raw,
			  mpu6050_data.accel_y_raw,
			  mpu6050_data.accel_z_raw,
			  mpu6050_data.gyro_x_raw,
			  mpu6050_data.gyro_y_raw,
			  mpu6050_data.gyro_z_raw,
			  mpu6050_data.temperature,
			  mpu6050_data.ax,
			  mpu6050_data.ay,
			  mpu6050_data.az,
			  mpu6050_data.gx,
			  mpu6050_data.gy,
			  mpu6050_data.gz);
		*/
	}
	// INT test
	// HAL_GPIO_WritePin(INT_FUNC_GPIO_Port, INT_FUNC_Pin, 1); // PA2
}
#endif

// DMA完成回调函数
#if 1
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	// HAL_GPIO_WritePin(DMA_FUNC_GPIO_Port, DMA_FUNC_Pin, 0); // PA1
	//log_i("HAL_I2C_MemRxCpltCallback");
	if(hi2c == &hi2c2)
	{
		pf_dma_interrupt_callback(handler_instance.p_mpu_driver, NULL);
	}  
	// HAL_GPIO_WritePin(DMA_FUNC_GPIO_Port, DMA_FUNC_Pin, 1); // PA1
}
#endif

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

  xTaskCreate(mpu_handler_thread_func, "mpu_handler_thread", 128 * 10, 
              &input_arg_mpu, osPriorityNormal, &mpu_handler_threadHandle);
  xTaskCreate(unpack_thread_func, "unpack_thread", 128 * 10, 
              NULL, osPriorityNormal, NULL);  /* USER CODE BEGIN RTOS_THREADS */
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

  // mpu_data_t p_data={0};
  // MPU6050_Test(&p_mpu_driver_instance);
  // buffer_init(&circular_buf, MPU6050_DATA_PACKET_SIZE);
  // mpu_data_t mpu6050_data;///通过都地址获取到的数据，存储在这个结构体中

  // uint8_t *rbuff = NULL;
	for(;;)   
	{	   

  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

