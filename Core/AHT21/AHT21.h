/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_aht21_handler.h
 * 
 * @par dependencies 
 * - stdio.h
 * - stdint.h
 * - aht21_reg.h
#include "stdint.h"
#include "stm32f4xx_hal.h"
#include "queue.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"              
 * @author Jack | R&D Dept. | EternalChip ?????
 *
 * @brief Provides HAL APIs for LED control and operations.
 * 
 * Usage:
 * Call functions directly.
 * 
 * @version V1.0 2026年4月12日
 *
 * @note 1 tab == 4 spaces
 * 
 *****************************************************************************/
#ifndef __AHT21_H__
#define __AHT21_H__

#include "stdio.h"
#include "stdint.h"
#include "bsp_aht21_driver.h"
#include "bsp_aht21_handler.h"

#include <stdbool.h>
//******************************* Defines ***********************************//


osThreadId_t humi_temp_TaskHandle;
const osThreadAttr_t humi_temp_Task_attributes = {
  .name = "humi_temp_Task",
  .stack_size = 128 *6,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t user_TaskHandle;
const osThreadAttr_t user_Task_attributes = {
  .name = "user_task",
  .stack_size = 128 * 5, 
//  .priority = (osPriority_t) osPriorityNormal,
	.priority = (osPriority_t) osPriorityBelowNormal,
};
// sensor_interface_i2c_timebase_delay_t input_arg;
void aht21_handler_thread_func(void *argument);
void user_task_func(void *argument);


#endif


