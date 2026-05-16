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
#ifndef __BSP_MPU_HANDLER_H__
#define __BSP_MPU_HANDLER_H__


#include "bsp_mpu_driver.h"
#include "FreeRTOS.h"
#include "queue.h"
#include <stdbool.h>
//******************************* Defines ***********************************//
#define OS_SUPPORTING

typedef enum
{
	HANDLER_MPU_OK             = 0,      /* Operation completed successfully         */
	HANDLER_MPU_ERROR          = 1,      /* General runtime error                    */
	HANDLER_MPU_ERRORTIMEOUT   = 2,      /* Operation timed out                      */
	HANDLER_MPU_ERRORRESOURCE  = 3,      /* Required resource is unavailable         */
	HANDLER_MPU_ERRORPARAMETER = 4,      /* Invalid parameter error                  */
	HANDLER_MPU_ERRORNOMEMORY  = 5,      /* Memory allocation failed                 */
	HANDLER_MPU_ERRORISR       = 6,      /* Not allowed in ISR context               */
	HANDLER_MPU_RESERVED       = 0xFF,   /* Reserved status                          */
}handler_mpu_status_t;

typedef struct 
{
    bool is_initated;   /* Initialization flag */
} mpu_Pri_data_init_t; 
/**0. input 接口参数 */
typedef struct
{
    mpu_timebases_ms_t * p_timebases_ms;
    mpu_iic_driver_instance_t * p_iic_driver_instance;
    mpu_delay_interface_t * p_delay_interface;
    mpu_yield_interface_t * p_yield_interface;
    mpu_os_interface_t * p_os_interface;
} mpu_input_arg_t;

/**1. handler结构体 */
typedef struct
{
    mpu_input_arg_t *p_input_arg;
    bsp_mpu_driver_t * p_mpu_driver;
    mpu_Pri_data_init_t *p_private_data_init;
    void * p_unpack_queue_handle;
    void * p_queue_handle;

    uint32_t queue_item_size;
    uint32_t queue_lenth;

    /** @brief totest */
    uint32_t last_tick;
} bsp_mpu_handler_t;
/**2. handler inst函数 */

handler_mpu_status_t handler_mpu_inst(
                                        bsp_mpu_handler_t * p_handler,
                                        mpu_input_arg_t * p_input_arg
                                    );

void mpu_handler_thread_func(void *pvParameters);

#endif