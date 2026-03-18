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
static led_status_t led_blink(bsp_led_driver_t * const    self ){
    led_status_t ret = LED_OK;

/**************** 1、检查目标是否被实例化 ***********************************/
    if( NULL        ==  self   ||
        NOT_INITED   ==  self->is_inited   )    //检查是否被实例化
    {
        #ifdef DEBUG
    DEBUGPRINT("LED_ERROR_PARAMETER\r\n");
        #endif  //debug
        return LED_ERRORPARAMETER;
    }

/****************3、实现闪烁操作 ***********************************/
    {//函数块
        //通过对象中断内部数据转存 进行闪烁操作
        uint32_t                    period_local    = self->blink_period_ms;
        uint32_t                    times_local     = self->blink_times;
        led_proportion_t          proportion_local  = self->proportion_on_off;
        uint32_t                 time_off_ms;
        switch(proportion_local){
            case PROPORTIONN_1_3:
                time_off_ms = period_local/ 4;
                break;
            case PROPORTIONN_1_2:
                time_off_ms = period_local/ 3;
                break;
            case PROPORTIONN_1_1:
                time_off_ms = period_local / 2;
                break;
            default:
                break;
        }

        for(uint32_t i = 0; i < times_local; i++){
            
            for(uint32_t j = 0; j < period_local; j++){
                self->p_os_delay_ms->pf_osdelay_ms(5);
                
                if(j < time_off_ms){
                    self->p_led_operation->pf_bsp_led_off();
                    // 结构体内部的函数指针指向函数才能调用
#ifdef DEBUG
                    DEBUGPRINT("LED_OFF\r\n");
#endif  //debug
                }
                else{
                    self->p_led_operation->pf_bsp_led_on();
                    //这里的ledon已经指向具体的led_on_myown函数
#ifdef DEBUG
                    DEBUGPRINT("LED_ON\r\n");
#endif  //debug
                }
            }
        }
    }

    return ret;
}



/// @brief 实例化结构体内部的函数指针  指向这里控制led闪烁
/// @param self 
/// @param period_ms 
/// @param times 
/// @param proportion_blink 
/// @return 
static led_status_t led_control(   bsp_led_driver_t * const    self,    //pointer 需要在上面声明
                                            uint32_t                    period_ms,              //period ms
                                            uint32_t                    times,               //times 
                                            led_proportion_t            proportion_blink      //proportion 3:1 2:1 1:1
            ){

        led_status_t ret = LED_OK;
/**************** 1、检查目标是否被实例化 ***********************************/
                /*判断指针 是否为空         是否被实例化    若无则返回错误 */
        if( NULL        ==  self   ||
            NOT_INITED   ==  self->is_inited   )
        {
#ifdef DEBUG
        DEBUGPRINT("0LED_ERROR_PARAMETER\r\n");
#endif  //debug
            return LED_ERRORPARAMETER;
        }
/**************** 2、检查参数是否合法 ***********************************/
        if(  ! ( 
                (6000 > period_ms)                              &&
                (100 > times)                                   &&
                (proportion_blink <= PROPORTIONN_1_1)           &&
                (proportion_blink >= PROPORTIONN_1_3)
                )   //end of !
        ){          //end of if 
#ifdef DEBUG
        DEBUGPRINT("0LED_ERROR_PARAMETER\r\n");
#endif  //debug
            return LED_ERRORPARAMETER;
        }

/****************3、接口中添加目标变量的值 ***********************************/
        self->blink_period_ms         = period_ms;
        self->blink_times             = times;
        self->proportion_on_off       = proportion_blink;
/****************4、实现led_blink_control ***********************************/
///要有时间序列  不能在这闪着一直等着亮灭   调用一次自动去闪烁
            //不能阻塞
        //和时间序列强度相关  不是说在一个task 或者线程中就能实现
            //通过调用一个函数强行实现---先简单粗暴实现 但是最终肯定不能这样

        ret = led_blink(self);
        return ret;
}


//初始化-获取内部的接口
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

/*************3、初始化接口     亮灭 延时 函数指针--控制函数**********************/
        self->p_led_operation = led_operation;
        self->p_timebase_ms   = timebase_ms;
        self->p_os_delay_ms   = os_delay_ms;
        self->pf_led_control = led_control;//函数指针 指向led_control函数

#ifndef OS_SUPPORTING
    
#endif //#ifndef OS_SUPPORTING
/************4 、初始化目标变量     闪烁功能变量**********************/

       self->blink_period_ms    = 0;
       self->blink_times        = 0;
       self->proportion_on_off  = PROPORTIONN_x_x;

       ret=led_driver_init(self);//*初始化函数中 包括 初始化接口     亮灭 延时
       self->is_inited = IS_INITED;

#ifdef DEBUG
        DEBUGPRINT("led instance finished\r\n");
#endif  //debug

        return ret;

}




