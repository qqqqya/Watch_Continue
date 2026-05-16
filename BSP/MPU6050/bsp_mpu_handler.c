
/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_aht21_handler.c
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
 * @version V1.0 2026年4月12日  16点55分
 *
 * @note 1 tab == 4 spaces
 * 
 *****************************************************************************/
#include "bsp_mpu_handler.h"
#include "elog.h"

/******************************** Defines ************************************/
#define HANDLER_MPU_INITED          true
#define HANDLER_MPU_NOT_INITED      false
QueueHandle_t mpu_queue_handle = NULL;

extern void (*pf_pin_interrupt_callback)(void *, void *);//FROM RTOS C
extern void (*pf_dma_interrupt_callback)(void *, void *);    
void callback_register(void (*callback)(void *, void *))
{
	pf_pin_interrupt_callback = callback;
}
void callback_register_dma(void (*callback)(void *, void *))
{
	pf_dma_interrupt_callback = callback;
}
static mpu_status_t handler_mpu_init(bsp_mpu_handler_t * handler_instance){
    mpu_status_t ret = HANDLER_MPU_OK;
    if(handler_instance == NULL){
        return HANDLER_MPU_ERRORPARAMETER;
    }
    /** @brief 创建解包队列 ---thread func里面put*/
    handler_instance->p_input_arg->p_os_interface->Queuecreate(
        handler_instance->queue_item_size,
        handler_instance->queue_lenth,
        &handler_instance->p_unpack_queue_handle//队列句柄 二级指针
    );
    ret = mpu_driver_instance(handler_instance->p_mpu_driver,
                        handler_instance->p_input_arg->p_iic_driver_instance,
                        handler_instance->p_input_arg->p_delay_interface,
                        handler_instance->p_input_arg->p_timebases_ms,
                        /** @brief 回调函数*/
                        callback_register,
                        callback_register_dma,
                        handler_instance->p_input_arg->p_yield_interface,
                        handler_instance->p_input_arg->p_os_interface,
                        handler_instance->p_queue_handle,
                        NULL,//notify_handler
                        NULL,//binary_handler
                        NULL    
                    );
    if(ret != HANDLER_MPU_OK){
        log_e("mpu_driver_instance failed");
        return ret;
    }
    return ret;
}
handler_mpu_status_t handler_mpu_inst(
    bsp_mpu_handler_t * handler_instance,
    mpu_input_arg_t * p_input_arg
){
    if(handler_instance == NULL || p_input_arg == NULL){
        return HANDLER_MPU_ERRORPARAMETER;
    }
    handler_instance->p_input_arg = p_input_arg;
    handler_instance->p_queue_handle = mpu_queue_handle;
    handler_mpu_init(handler_instance);


    handler_instance->p_private_data_init->is_initated = HANDLER_MPU_INITED;
    return HANDLER_MPU_OK;
}
bsp_mpu_handler_t handler_instance={0};

void mpu_handler_thread_func(void *p_input_arg){
    log_i("mpu_handler_thread_func");
    handler_mpu_status_t ret = HANDLER_MPU_OK;
    bsp_mpu_driver_t p_mpu_driver_instance={0};

    mpu_input_arg_t * input=NULL;
    input = (mpu_input_arg_t *)p_input_arg;
        handler_instance.p_mpu_driver = &p_mpu_driver_instance;//挂载driver
        handler_instance.queue_item_size = sizeof(uint8_t);
        handler_instance.queue_lenth = 20;
        handler_instance.p_private_data_init = HANDLER_MPU_NOT_INITED;
        handler_instance.p_queue_handle = mpu_queue_handle;
        handler_instance.last_tick=0;//
    ret = handler_mpu_inst(&handler_instance, input);//给handler挂载
    /** @brief 初始化解包缓冲区  大小为MPU6050_DATA_PACKET_SIZE
     */
    buffer_init(&circular_buf, MPU6050_DATA_PACKET_SIZE);
    if(ret != HANDLER_MPU_OK){
        log_e("handler_mpu_inst failed");
//        return ret;
    }
    uint32_t unpack_event = 8000;
    while(1){
            if (1 == mpu_flag_read()){//dma完成  解包队列消息通知
                handler_instance.p_input_arg->p_os_interface->Queueput(
                    handler_instance.p_unpack_queue_handle,//队列句柄
                    &unpack_event,
                    0
                );                
            }
            mpu_flag_set(0);
#if 0 // queue test
        // Get the data from the mpuxxx queue
        ret = handler_instance.p_input_args->p_os->os_queue_get(handler_instance.p_queue_handle,
                                                    &data,
                                                    0xffffffff);
        if (ret != MPUxxx_OK)
        {
#ifdef DEBUG
            DEBUG_OUT("mpuxxx_handler_thread: queue get failed\n");
#endif
        }

#ifdef DEBUG
        DEBUG_OUT("mpuxxx_handler_thread: data = %d\n", data);
#endif

#endif // queue test
/*********************************************************/
#if 0 // binary test
        ret = handler_instance.p_input_args->p_os->os_semaphore_wait_binary(handler_instance.semaphore_binary_handle);
        if (ret != MPUxxx_OK)
        {
#ifdef DEBUG
            DEBUG_OUT("mpuxxx_handler_thread: semaphore wait failed\n");
#endif
		}
#endif // binary test            
    }
}
