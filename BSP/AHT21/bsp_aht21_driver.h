/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_aht21_driver.h
 * 
 * @par dependencies 
 * - stdio.h
 * - stdint.h
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
#ifndef __BSP_AHT21_DRIVER_H__
#define __BSP_AHT21_DRIVER_H__

#include "stdio.h"
#include "stdint.h"

//******************************* Defines ***********************************//
#define OS_SUPPORTING
typedef enum
{
	AHT_IS_INITED            = 0,      /* Operation completed successfully         */
	AHT_NOT_INITED          = 1,      /* General runtime error                    */
} aht_handler_init_t; 

typedef enum
{
	AHT_OK             = 0,      /* Operation completed successfully         */
	AHT_ERROR          = 1,      /* General runtime error                    */
	AHT_ERRORTIMEOUT   = 2,      /* Operation timed out                      */
	AHT_ERRORRESOURCE  = 3,      /* Required resource is unavailable         */
	AHT_ERRORPARAMETER = 4,      /* Invalid parameter error                  */
	AHT_ERRORNOMEMORY  = 5,      /* Memory allocation failed                 */
	AHT_ERRORISR       = 6,      /* Not allowed in ISR context               */
	AHT_RESERVED       = 0xFF,   /* Reserved status                          */
} aht_status_t;

/*根据iic的基本需求写函数指针    还有这里的排版也很舒服
    这里iic RW数据都是传的地址+长度+存入/读出 变量的指针  读写多个字节都可以了
*/
//******1.iic 实例结构体**********//

typedef struct{
    int8_t        (*pf_iic_init       ) (void);
    int8_t        (*pf_iic_deinit     ) (void); //反初始化
    int8_t        (*pf_iic_start      ) (void);
    int8_t        (*pf_iic_stop       ) (void);
    int8_t        (*pf_iic_waitack    ) (void);
    int8_t        (*pf_iic_waitnotack ) (void);
    int8_t        (*pf_iic_sendbytes  ) (uint8_t address,
                                         uint8_t  *pdata,
                                         uint8_t   size);
    int8_t        (*pf_iic_recevbytes ) (uint8_t address,
                                         uint8_t  *pdata,
                                         uint8_t   size);
    int8_t         (*pf_enter_critical) (void);                            
    int8_t         (*pf_exit_critical) (void);                            
}iic_driver_instance_t;

//from core layer(hal库)
//******2.timebase 实例结构体**********//
typedef struct{
    uint32_t    (*pf_timebase_gettickms) (void);
}timebases_t;
//from OS layer

//******3. 实例结构体**********//
#ifdef OS_SUPPORTING
typedef struct{
    void        (*rtos_yield) (uint32_t );  //os delay ms
}yield_interface_t;


#endif


/*  按照之前学的led桥接模式驱动的方法
    1.    首先应该写的是bsp_aht21_driver_t 内部的成员变量 根据需求写
            再声明instance 后不断填充bsp_aht21_driver_t成员
    2.    填充一些成员变量 定义数据类型如iic_driver_instance_t
            里面包含一些函数指针 这些函数指针指向实际的函数实现
        
        */
/*******aht21接口   实例结构体  **********/
typedef struct bsp_aht21_driver
{
    //外部接口
    iic_driver_instance_t *p_iic_instance; //IIC实例指针
    timebases_t *p_timebase_ms; //timebase实例指针
                            //中断保护

    yield_interface_t   *p_yield_interface;                     //操作系统让出CPU接口

    //iic实例
    // void             *p_iic_instance; //  IIC实例指针

    aht_status_t (*pf_instance)(    //实例化函数的指针
                                    void * const self,
                                    iic_driver_instance_t  * const p_iic_instance,
                                    timebases_t  *const p_timebase_ms,
                                    yield_interface_t   * const p_yield_interface
										); 
    aht_status_t (*pf_init      )(void * const self);         //初始化函数指针
    aht_status_t (*pf_deinit    )(void * const self);       //反初始化
    aht_status_t (*pf_read_id   )(void * const self);      //读取device id
    aht_status_t (*pf_read_temp )(
                                    void * const self,
                                    void * const temp);         //读取temper
    aht_status_t (*pf_read_humi )(
                                    void * const self,
                                    void * const humi);         //读取humidity
    aht_status_t (*pf_sleep     )(void * const self);         //sleep
    aht_status_t (*pf_wakeup    )(void * const self);         //wakeup
} bsp_aht21_driver_t;


//******************************* Functions ***********************************//
aht_status_t aht21_driver_instance(
                                    bsp_aht21_driver_t * const self,
                                    iic_driver_instance_t  * const p_iic_instance,
                                    timebases_t  *const p_timebase_ms,
                                    yield_interface_t   * const p_yield_interface

);
#endif
