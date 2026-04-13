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
#ifndef __BSP_AHT21_HANDLER_H__
#define __BSP_AHT21_HANDLER_H__

#include "stdio.h"
#include "stdint.h"
#include "bsp_aht21_driver.h"

#include <stdbool.h>
//******************************* Defines ***********************************//
#define OS_SUPPORTING


typedef struct 
{
    bool is_initated;   /* Initialization flag */
} Private_data_init_t; 

typedef enum
{
	HANDLER_AHT_OK             = 0,      /* Operation completed successfully         */
	HANDLER_AHT_ERROR          = 1,      /* General runtime error                    */
	HANDLER_AHT_ERRORTIMEOUT   = 2,      /* Operation timed out                      */
	HANDLER_AHT_ERRORRESOURCE  = 3,      /* Required resource is unavailable         */
	HANDLER_AHT_ERRORPARAMETER = 4,      /* Invalid parameter error                  */
	HANDLER_AHT_ERRORNOMEMORY  = 5,      /* Memory allocation failed                 */
	HANDLER_AHT_ERRORISR       = 6,      /* Not allowed in ISR context               */
	HANDLER_AHT_RESERVED       = 0xFF,   /* Reserved status                          */
} handler_aht_status_t;

typedef enum
{
    HUMI_TEMP_HUMI = 0,
    HUMI_TEMP_TEMP ,
    HUMI_TEMP_BOTH,
} HUMI_TEMP_SELECT_t;
//******1.构造事件结构体**********//
typedef struct {  //1time？2读temp/humi/both	回调函数（x1.5
    HUMI_TEMP_SELECT_t humi_temp_select;
    
    float * temp;
    float * humi;
    uint32_t * lifetime;
    uint32_t * timestamp;
    void (*pf_callback)(float * , float *);
}handler_aht_event_t;
    
//******2.OS 提供的queue结构体**********//
/**********好像这里没有thread的事情   和led 有点不一样》》实际也可以有  就只是传个句柄
 * 但实际上也可以封装一下 os 创建thread的函数
 */
typedef struct {
    handler_aht_status_t (*Queuecreate)(uint32_t const item_num,
                                        uint32_t const size,
                                        void** Q_handler);
    handler_aht_status_t (*Queuedelete)(void * const Q_handler);

    handler_aht_status_t (*Queueput   )(void * const Q_handler,
                                        void * const item,
                                        uint32_t timeout);
    handler_aht_status_t (*Queueget   )(void * Q_handler,
                                        void * msg,
                                        uint32_t timeout);
}os_queue_t;
    /**queue: 数量 大小 句柄 
     * thread:task func;name;栈; 
     *        param for task; priority;二级 pointer to task
     */

// typedef struct {
//     handler_aht_status_t (*Threadcreate)(void* thread_func,
//             /*二级指针 修改指针*/         void** thread_handler);
//     handler_aht_status_t (*Threaddelete)(void* thread_handler);

// }os_thread_t;
//******3.向传感器层提供接口的结构体--input arg/sensor interface**********//

typedef struct{

    timebases_ms_t            * p_timebase_ms;
    yield_interface_t         * p_yield_interface;  //delay
    iic_driver_instance_t     * p_iic_instance;
    os_queue_t                * p_os_queue;
    // os_thread_t               * p_os_thread;

}sensor_interface_i2c_timebase_delay_t;


/*******handler接口   实例结构体  **********/
typedef struct bsp_handler_aht21_driver
{
    /******driver core层
     * 提供接口的结构体--input arg/sensor interface
     * delay
     * 时基
     * iic driver**********/
    timebases_ms_t            * p_timebase_ms;
    yield_interface_t         * p_yield_interface;  //delay
    iic_driver_instance_t     * p_iic_instance;

    /****aht21 driver 
     * 实例结构体 
     * */
    bsp_aht21_driver_t        * p_aht21_instance;
    /*****私有数据
      * inited-handler
    */
    Private_data_init_t       * p_private_data_init;
    
    /***os-queue
      * include create delete put get 句柄
    */
    os_queue_t                * p_os_queue;
    void                      * p_queue_handler;
    /***os-thread       use threadnew
      * include create delete 句柄
    */
    // os_thread_t               * p_os_thread;
    // void                      * p_thread_handler;
    
    /***参考里面写的
     * tick 
    */
    uint32_t                  last_temp_tick;
    uint32_t                  last_humi_tick;
} bsp_handler_aht21_driver_t;

/**线程任务函数 */
void aht21_handler_thread_func(void * arg);
//******************************* Functions ***********************************//
handler_aht_status_t aht21_handler_instance(
                                    bsp_handler_aht21_driver_t * const p_handler_aht21_instance,//
                                    sensor_interface_i2c_timebase_delay_t * const p_sensor_interface
);
/****读取传感器数据
 * 类似于led的control函数   进行事件发送到队列
 * self->p_os_queue_interface->pf_os_queue_put(self->queue_handler,&led_event,0);
 */
handler_aht_status_t bsp_aht21_handler_read(handler_aht_event_t * event);  //


#endif
