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
led_status_t led_driver_init(   bsp_led_driver_t * const self          //led driver struct pointer
)
{   
        led_status_t ret = LED_OK;
        DEBUGPRINT("led init start\r\n");
        /*判断指针 是否为空*/
        if( NULL  ==  self      )
        {
            #ifdef DEBUG
        DEBUGPRINT("LED_ERROR_PARAMETER\r\n");
            #endif  //debug
            return LED_ERRORPARAMETER;
        }
    
        /*初始化接口的函数指针      即之前定义过的onoff 
                                    获取tick
                                    os delay函数
        将他们初始化的值指向具体的值--onoff--返回时间数--delay时长--
                                   */
        self->p_led_operation->pf_bsp_led_off();
        uint32_t time_stamp = 0;
        self->p_timebase_ms->pf_get_tick_ms(&time_stamp);
        
        self->p_os_delay_ms->pf_osdelay_ms(500);

        return ret;

    
    }


/*实例化包含
    接口指向  接口init 另一个函数 设置亮灭
        目标变量init 全给0即可
            后续的目标变量修改用的是另一个函数指针*/
led_status_t led_driver_instance(   bsp_led_driver_t * const self,          //led driver struct pointer
                                                                            //这个传入self可以直接操作结构体中的变量
                                    led_operation_t * const led_operation,  //led operation on_off
                                    timebase_t * const timebase_ms,         //timebase -tick ms
#ifdef OS_SUPPORTING
                                    os_delay_t * const os_delay_ms          //os delay ms   
#endif

)
{    
        led_status_t ret = LED_OK;
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
        if(IS_INITED  ==  self->is_inited)
        {
            #ifdef DEBUG
                    DEBUGPRINT("LED_ERROR_RESOURCE\r\n");
            #endif  //debug
            return LED_ERRORRESOURCE;
        }

 
#ifdef DEBUG
        DEBUGPRINT("led instance start\r\n");
#endif  //debug

/*************3、初始化接口     亮灭 延时**********************/
        self->p_led_operation = led_operation;
        self->p_timebase_ms   = timebase_ms;
        self->p_os_delay_ms   = os_delay_ms;

#ifndef OS_SUPPORTING
    self->pf_led_countroler = led_control;
#endif //#ifndef OS_SUPPORTING
/************4 、初始化目标变量     闪烁功能变量**********************/

       self->blink_period_ms    = 0;
       self->blink_times        = 0;
       self->proportion_on_off  = BSP_LED_PROPORTION_x_x;

       ret=led_driver_init(self);//*初始化函数中 包括 初始化接口     亮灭 延时
       self->is_inited = IS_INITED;

#ifdef DEBUG
        DEBUGPRINT("led instance finished\r\n");
#endif  //debug

        return ret;

}




