/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_led_driver.c
 * 
 * @par dependencies 
 * - bsp_led_driver.h
 * - stdint.h
 * 
#include "bsp_led_driver.h"
#include "stdint.h"
 * @author Yan | R&D Dept. | EternalChip
 *
 * @brief Provides HAL APIs for LED control and operations.
 * 
 * Usage:
 * Call functions directly.
 * 
 * @version V1.0 2026-03-14
 *
 * @note 1 tab == 4 spaces
 * 
 * 
******************************* includes ***********************************
******************************* fuctions ***********************************
 *****************************************************************************/
/******************************** includes ************************************/
 #include "bsp_led_driver.h"


/*根据h文件补充*/
/******************************** Declares ************************************/
led_status_t led_driver_instance(   bsp_led_driver_t * const self,          //led driver struct pointer
                                                                            //这个传入self可以直接操作结构体中的变量
                                    led_operation_t * const led_operation,  //led operation on_off
                                    timebase_t * const timebase_ms,         //timebase -tick ms
#ifdef OS_SUPPORTING
                                    os_delay_t * const os_delay_ms          //os delay ms   
#endif

)
{    /*初始化结构体变量*/

        /*判断指针 是否为空*/
        if( NULL  ==  self          ||
            NULL  ==  led_operation ||
            NULL  ==  timebase_ms   ||
            NULL  ==  os_delay_ms                 //用一个宏怎么检验
            )
        {
            #ifdef DEBUG
        DEBUGPRINT("LED_ERROR_PARAMETER\r\n");
            #endif  //debug
            return LED_ERRORPARAMETER;
        }

        /*判断是否已经实例化*/
        if(IS_INSTANCE  ==  self->is_instance)
        {
            #ifdef DEBUG
                    DEBUGPRINT("LED_ERROR_RESOURCE\r\n");
            #endif  //debug
            return LED_ERRORRESOURCE;
        }


#ifdef DEBUG
        DEBUGPRINT("led instance start\r\n");
#endif  //debug

        /*初始化结构体变量*/
//        self->blink_period = 0;
//        self->blink_times = 1;
//        self->os_delay_ms = os_delay_ms;
//        self->is_instance = IS_INSTANCE;


#ifdef DEBUG
        DEBUGPRINT("led instance finished\r\n");
#endif  //debug

    /*debug信息打印*/

}




