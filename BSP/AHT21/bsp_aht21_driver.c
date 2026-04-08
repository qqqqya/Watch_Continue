
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
 * #include "aht21_reg.h"
 *  "bsp_aht21_driver.h"
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
#include "bsp_aht21_driver.h"
#include "aht21_reg.h"

/******************************** Defines ************************************/
#define AHT21_MEASUREMENT_TIME_MS 80            // Measurement time [ms]
#define AHT21_NOT_INITED     0            // Not init. flag 
#define AHT21_INITED         1            // Not init. flag 
#define AHT21_ID                  0x18           // AHT21 ID--STATUS寄存器相与0x18，结果为0x18，说明是AHT21传感器

#define AHT21_CRC8_POLYNOMIAL     0x31           // CRC-8 polynomial
#define AHT21_CRC8_INITIAL        0xFF           // CRC-8 initial value

/******************************** Declares ************************************/
int8_t g_inited=AHT21_NOT_INITED;  //全局变量 记录是否实例化过了
int8_t g_dev_id=0;                  // 记录设备ID
static aht_status_t __read_id(bsp_aht21_driver_t *  self){
    
}

static aht_status_t aht21_read_id(bsp_aht21_driver_t *  self){

    if(g_inited!=AHT21_INITED)  
    {
        return AHT_ERRORRESOURCE;  //未初始化化
    }
    return g_dev_id;    //__read_id函数中会把读取到的device id存入全局变量g_dev_id中  这里直接返回就好了
}
static aht_status_t aht21_init( bsp_aht21_driver_t *  self){

    if (self == NULL ) {
        return AHT_ERRORPARAMETER;
    }
    //

}
static aht_status_t aht21_deinit( bsp_aht21_driver_t *  self){

    g_inited=AHT21_NOT_INITED;  
    g_dev_id=0;                  
    return AHT_OK;

}
aht_status_t aht21_driver_instance(
                                    bsp_aht21_driver_t * const self,
                                    iic_driver_instance_t  * const p_iic_instance,
                                    timebases_t  *const p_timebase_ms,
                                    yield_interface_t   * const p_yield_interface
                                    )
{
    if(g_inited==AHT21_INITED)  
    {
        return AHT_ERRORRESOURCE;  //已经实例化过了
    }

    if (self == NULL || 
        p_iic_instance == NULL || 
        p_timebase_ms == NULL || 
        p_yield_interface == NULL) 
    {
        return AHT_ERRORPARAMETER;
    }
    //外部接口赋值
    self->p_iic_instance = p_iic_instance;  //rtos c(app)层传入iic实例指针
    self->p_timebase_ms = p_timebase_ms;
    self->p_yield_interface = p_yield_interface;
/*    // Initialize function pointers
    self->pf_init = aht21_init;
    self->pf_deinit = aht21_deinit;
    self->pf_read_id = aht21_read_id;
    self->pf_read_temp = aht21_read_temp;
    self->pf_read_humi = aht21_read_humi;
    self->pf_sleep = aht21_sleep;
    self->pf_wakeup = aht21_wakeup;
*/
    //调用初始化函数
    aht21_init(self);


    return AHT_OK;
}
