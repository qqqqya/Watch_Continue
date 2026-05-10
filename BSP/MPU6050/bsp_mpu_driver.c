
/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_aht21_driver.c
 * 
 * @par dependencies 
 * - stdio.h
 * - stdint.h
 * - "bsp_aht21_driver.h"
 * - elog.h
 * 
 * 
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
 * @version V1.0 2024-10-18
 *
 * @note 1 tab == 4 spaces
 * 
 *****************************************************************************/
#include "bsp_mpu_driver.h"
#include "elog.h"

/******************************** Defines ************************************/
/******************************** Declares ************************************/
int8_t g_inited =   AHT21_NOT_INITED;  //全局变量 记录是否实例化过了
int8_t g_dev_id =   0;                  // 记录设备ID
#include "iic_hal.h"



mpu_status_t mpu_driver_instance(
                                    bsp_mpu_driver_t * const p_mpu_driver,
                                    mpu_iic_driver_instance_t  * const p_iic_instance,
                                    mpu_delay_interface_t       * const p_delay_interface, //delay接口
                                    mpu_timebases_ms_t  *const p_timebase_ms,
                void (*callback_register)    (void (*callback)(void *, void *)),
                void (*callback_register_dma)(void (*callback)(void *, void *)),
#ifdef OS_SUPPORTING
                                    mpu_yield_interface_t   * const p_yield_interface,
                                    mpu_os_interface_t * const p_os_interface, //os接口
                    void *  Q_handler, //队列句柄指针
                    void *  mutex_handler, //互斥锁句柄指针
                    void *  notify_handler, //消息通知句柄
                    void *  binary_handler //二值信号量句柄
#endif
                                    ){

    if( NULL==p_mpu_driver      ||
        NULL==p_iic_instance    || 
        NULL==p_delay_interface || 
        NULL==p_timebase_ms
#ifdef OS_SUPPORTING
        || NULL==p_yield_interface 
        || NULL==p_os_interface
#endif

)
    {
        return MPU_ERRORPARAMETER;
    }

        if( NULL==callback_register      ||
        NULL==callback_register_dma    
            )
    {
        return MPU_ERRORPARAMETER;
    }
    /**iic instance is available */
        if( NULL==p_iic_instance->pf_iic_init      ||
        NULL==p_iic_instance->pf_iic_deinit    || 
        NULL==p_iic_instance->pf_iic_mem_read    || 
        NULL==p_iic_instance->pf_iic_mem_write    || 
        NULL==p_iic_instance->pf_iic_mem_read_dma    
        ){  return MPU_ERRORPARAMETER;  }
        p_mpu_driver->p_iic_instance=p_iic_instance;///挂载iic实例
    /**iic instance is available */



    if (NULL == p_mpu_driver->p_os_interface->os_queue_create            ||
        NULL == p_mpu_driver->p_os_interface->os_queue_delete            ||
        NULL == p_mpu_driver->p_os_interface->os_queue_put               ||
		NULL == p_mpu_driver->p_os_interface->os_queue_put_isr           ||
        NULL == p_mpu_driver->p_os_interface->os_queue_get               ||
        NULL == p_mpu_driver->p_os_interface->os_semaphore_create_mutex  ||
        NULL == p_mpu_driver->p_os_interface->os_semaphore_delete_mutex  ||
        NULL == p_mpu_driver->p_os_interface->os_semaphore_lock_mutex    ||
        NULL == p_mpu_driver->p_os_interface->os_semaphore_unlock_mutex  ||
        NULL == p_mpu_driver->p_os_interface->os_semaphore_create_binary ||
        NULL == p_mpu_driver->p_os_interface->os_semaphore_delete_binary ||
        NULL == p_mpu_driver->p_os_interface->os_semaphore_wait_binary   ||
        NULL == p_mpu_driver->p_os_interface->os_semaphore_signal_binary   )
    {
        return MPU_ERRORPARAMETER;
    }
    if (NULL == p_mpu_driver->p_delay_interface->pf_delay_init ||
        NULL == p_mpu_driver->p_delay_interface->pf_delay_us   ||
        NULL == p_mpu_driver->p_delay_interface->pf_delay_ms)
    {
        return MPU_ERRORPARAMETER;
    }


}
