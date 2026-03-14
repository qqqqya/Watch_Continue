/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_led_driver.h
 * 
 * @par dependencies 
 * - stdio.h
 * - stdint.h
 * 
#include "stdio.h"
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
#ifndef __BSP_LED_DRIVER_H__
#define __BSP_LED_DRIVER_H__
//******************************* includes ***********************************//
#include "stdio.h"
#include "stdint.h"
//******************************* Defines ***********************************//
#define IS_INSTANCE         1
#define ISNT_INSTANCE       0
#define OS_SUPPORTING       
#define DEBUG       
#define DEBUGPRINT(X)       printf(X)       //调试输出打印


typedef struct bsp_led_driver bsp_led_driver_t;
// typedef enum{
//     LED_OK = 0,
//     LED_ERROR = 1,
//     LED_MEMERROR = 2,
//     LED_XX       = 0xff,

// }led_status_t;
typedef enum
{
	LED_OK             = 0,      /* Operation completed successfully         */
	LED_ERROR          = 1,      /* General runtime error                    */
	LED_ERRORTIMEOUT   = 2,      /* Operation timed out                      */
	LED_ERRORRESOURCE  = 3,      /* Required resource is unavailable         */
	LED_ERRORPARAMETER = 4,      /* Invalid parameter error                  */
	LED_ERRORNOMEMORY  = 5,      /* Memory allocation failed                 */
	LED_ERRORISR       = 6,      /* Not allowed in ISR context               */
	LED_RESERVED       = 0xFF,   /* Reserved status                          */
} led_status_t;


typedef enum{
    BSP_LED_PROPORTION_3_1 = 0,
    BSP_LED_PROPORTION_2_1 = 1,
    BSP_LED_PROPORTION_1_1 = 2,
    BSP_LED_PROPORTION_x_x = 0xff,

}led_proportion_t;

typedef struct{

    led_status_t (*pf_bsp_led_on)(void);//还需要一个返回值  返回状态给定义enum
    led_status_t (*pf_bsp_led_off)(void);//RETURN       VOID -> led_status_t
}led_operation_t;

/*声明函数的时候参数名是 注释性的可有可无 只有在定义（实现）的时候才需要参数名）
    这些个函数指针都可以指向实际的实现（函数）
        `const修饰的是指针变量本身，意思是**这个指针变量的值（即它存储的地址）不能被修改

        //还需要一个返回值 返回状态 查看问题
*/
typedef struct{
    led_status_t (*pf_get_tick_ms)(uint32_t *  const);
    //把tick的数据传回来  比如uint32_t tick;
}timebase_t;

#ifdef OS_SUPPORTING
typedef struct{
    led_status_t (*pf_osdelay_ms)(const uint32_t);
}os_delay_t;
#endif

/*直接定义指针函数*/
typedef led_status_t (*pf_led_control_t)(     bsp_led_driver_t * const led_driver,    //led driver struct pointer
                                            uint32_t     ,              //period ms
                                            uint32_t     ,               //times 
                                            led_proportion_t       //proportion 3:1 2:1 1:1
            );

/*这里面定义的都是结构体的指针
    具体的函数在对应的结构体数据类型中封装着  到时候只需要调用对应的函数指针，指向定义的函数即可
        这也是二层索引  在结构体中定义函数指针 定义结构体指针 再指向定义的函数
    */
//******************************* fuctions ***********************************//

typedef struct bsp_led_driver
{
    /*初始化次数的状态 这个类定义好了 实例化的时候只一次  防止多读多写（多个任务线程同时操作）
        借助枚举告知 实例化次数  是否实例化*/
            uint8_t                     is_instance;/* is instance  0:no 1:yes*/
    /***********define led Blink Configuration feature********* */
            uint32_t                     blink_period_ms;/*blink period              */
            uint32_t                     blink_times;/* blink times                  */
            led_proportion_t         proportion_on_off;/* proportion   3:1 2:1           */
            
    /***********define led driver interior feature********* */
            /* led gpio   GPIO_TypeDef*               gpio;    int16_t   pin;*/
            /*因为这里的struct 定义的都是函数指针 所以变量应当都是指针p_*/
            led_operation_t         *p_led_operation;
            timebase_t                  *timebase_ms;
    /***********define rtos change feature********* */
#ifdef OS_SUPPORTING
            os_delay_t                  *os_delay_ms;
#endif
    /***********Control API out interface feature*********
     *  外部结构 接口调用  就是调用函数而已  不需要再定义结构体指针 二次索引
     */
            pf_led_control_t            pf_led_control;
                        
}bsp_led_driver_t;

/*************************define                      ********************************** */
// led_driver_instance(&led_1);

/*************************declare 声明 函数实例化声明  *********************************** */

led_status_t led_driver_instance(   bsp_led_driver_t * const self,          //led driver struct pointer
                                                                            //这个传入self可以直接操作结构体中的变量
                                    led_operation_t * const led_operation,  //led operation on_off
                                    timebase_t * const timebase_ms,         //timebase -tick ms
#ifdef OS_SUPPORTING
                                    os_delay_t * const os_delay_ms          //os delay ms   
#endif

);

#endif
