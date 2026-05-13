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
extern bsp_mpu_driver_t p_mpu_driver_instance;
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

  mpu_data_t p_data={0};
  MPU6050_Test(&p_mpu_driver_instance);
  buffer_init(&circular_buf, MPU6050_DATA_PACKET_SIZE);
  mpu_data_t mpu6050_data;///通过都地址获取到的数据，存储在这个结构体中

  uint8_t *rbuff = NULL;
	for(;;)   
	{	   

    /**INT触发--读取数据 */

    // p_mpu_driver_instance.pf_get_accel(&p_mpu_driver_instance, &p_data);
    // log_i("ax = %f, ay = %f, az = %f\r\n",\
    //    p_data.ax, p_data.ay, p_data.az);
    // 检查全局变量，看 DMA 是否完成了一次搬运
        // if (mpu_flag_read() == 1)
        // {
        //     // 清除标志位
        //     mpu_flag_set(0);
if (1 == mpu_flag_read()){
            // 4. 从环形缓冲区获取【读指针】
            // (这里假设你的 buffer_interface 提供了获取读地址的接口)
            uint8_t *addr = circular_buf.pfget_rbuffer_addr(&circular_buf);
            log_i("unpack_task: addr = %p\n", addr);
              // temp = (int16_t)(*(addr + 6) << 8 | *(addr + 7));
              // mpu6050_data.temperature = 36.53 + temp/340.0;
          
              mpu6050_data.accel_x_raw = (int16_t)(*(addr + 0) << 8 | *(addr + 1));
              mpu6050_data.accel_y_raw = (int16_t)(*(addr + 2) << 8 | *(addr + 3));
              mpu6050_data.accel_z_raw = (int16_t)(*(addr + 4) << 8 | *(addr + 5));

              mpu6050_data.ax = mpu6050_data.accel_x_raw / 16384.0;
              mpu6050_data.ay = mpu6050_data.accel_y_raw / 16384.0;
              mpu6050_data.az = mpu6050_data.accel_z_raw / 14418.0;

              mpu6050_data.gyro_x_raw = (int16_t)(*(addr + 8) << 8 | *(addr + 9));
              mpu6050_data.gyro_y_raw = (int16_t)(*(addr + 10) << 8 | *(addr + 11));
              mpu6050_data.gyro_z_raw = (int16_t)(*(addr + 12) << 8 | *(addr + 13));

              mpu6050_data.gx = mpu6050_data.gyro_x_raw / 131.0;
              mpu6050_data.gy = mpu6050_data.gyro_y_raw / 131.0;
              mpu6050_data.gz = mpu6050_data.gyro_z_raw / 131.0;
              
              log_i("UnpackThread temp=%f", mpu6050_data.temperature);
              log_i("UnpackThread ax=%f", mpu6050_data.ax);
              log_i("UnpackThread ay=%f", mpu6050_data.ay);
              log_i("UnpackThread az=%f", mpu6050_data.az);
              log_i("UnpackThread gx=%f", mpu6050_data.gx);
              log_i("UnpackThread gy=%f", mpu6050_data.gy);
              log_i("UnpackThread gz=%f", mpu6050_data.gz);

              circular_buf.pfdata_readed(&circular_buf);
             mpu_flag_set(0);
}
    // osDelay(2);
    
	}
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

