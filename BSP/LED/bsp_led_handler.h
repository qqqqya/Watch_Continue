/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * 
******************************* includes ***********************************
******************************* fuctions ***********************************
 * @file bsp_led_driver.h
 *  
 * @par dependencies 
 * - stdio.h
 * - stdint.h
 * 
#include "stdio.h"
#include "stdint.h"
#include "bsp_led_driver.h"
 * @author Yan | R&D Dept. | EternalChip
 *
 * @brief Provides HAL APIs for LED control and operations.
 * 
 * Usage:
 * Call functions directly.
 * 
 * @version V1.0 2026-03-19
 *
 * @note 1 tab == 4 spaces

 *****************************************************************************/
#ifndef __BSP_LED_HANDLER_H__
#define __BSP_LED_HANDLER_H__
//******************************* includes ***********************************//
#include "stdio.h"
#include "stdint.h"
#include "bsp_led_driver.h"

//******************************* Defines ***********************************//
#define OS_SUPPORTING   //这里都是全局宏定义      
#define DEBUG       
#define DEBUGPRINT(X)       printf(X)       //调试输出打印
#define INIT_PATTERN       0xA6A6A6A6
// #define MAX_LED_INSTANCES       10


typedef struct bsp_led_handler bsp_led_handler_t;
typedef enum
{
	HANDLER_IS_INITED            = 0,      /* Operation completed successfully         */
	HANDLER_NOT_INITED          = 1,      /* General runtime error                    */
} led_handler_init_t; //之前的宏定义 改为枚举

typedef enum{
    LED1        =   0,
    LED2,
    LED3,
    LED4,
    LED5,
    LED6,
    LED7,
    LED8,
    LED9,
    LED10,
    MAX_LED_INSTANCES,
    LED_NOT_INITIALIZED=0xFFFFFFFF, //0xff problem
}led_index_t;


typedef enum
{
	HANDLER_OK             = 0,      /* Operation completed successfully         */
	HANDLER_ERROR          = 1,      /* General runtime error                    */
	HANDLER_ERRORTIMEOUT   = 2,      /* Operation timed out                      */
	HANDLER_ERRORRESOURCE  = 3,      /* Required resource is unavailable         */
	HANDLER_ERRORPARAMETER = 4,      /* Invalid parameter error                  */
	HANDLER_ERRORNOMEMORY  = 5,      /* Memory allocation failed                 */
	HANDLER_ERRORISR       = 6,      /* Not allowed in ISR context               */
	HANDLER_RESERVED       = 0xFF,   /* Reserved status                          */
} led_handler_status_t;


/*声明函数的时候参数名是 注释性的可有可无 只有在定义（实现）的时候才需要参数名）
    这些个函数指针都可以指向实际的实现（函数）
        `const修饰的是指针变量本身，意思是**这个指针变量的值（即它存储的地址）不能被修改
        //还需要一个返回值 返回状态 查看问题
*/
typedef struct{
    led_handler_status_t (*pf_get_tick_ms)(uint32_t * const);
    //把tick的数据传回来  比如uint32_t tick;
}handelr_timebase_t;



#ifdef OS_SUPPORTING
typedef struct{
    led_handler_status_t (*pf_osdelay_ms)(const uint32_t);
}handler_os_delay_t;

typedef struct{
    led_handler_status_t (*pf_os_critical_enter)(void);
    led_handler_status_t (*pf_os_critical_exit)(void);
}handler_os_critical_t;//临界区接口--进稳定区？ vPortEnterCritical(); 

typedef struct{
    /*创建队列*/
    led_handler_status_t (*pf_os_queue_create)(
                                                const uint32_t item_num,
                                                const uint32_t size,
                                                void  ** const queue_handler                                                                                    
                                            );

    /*发送队列  --put*/
    //发送队列仿照着RTOS里边写的    参数--句柄 要写入的数据 发送等待时间  
    led_handler_status_t (*pf_os_queue_put)(
                                                void * const queue_handler,
                                                void *  const  item_data,
                                                uint32_t timeout_ms
                                            );
    /*接收队列  --get*/                                        
    led_handler_status_t (*pf_os_queue_get)(
                                                void * const queue_handler,
                                                void *  const  item_data,
                                                uint32_t timeout_ms
                                            );
    /*删除队列*/
    led_handler_status_t (*pf_os_queue_delete)(
                                                void * const queue_handler
                                            );
}handler_os_queue_t;

typedef struct{
    /*创建线程*/
    led_handler_status_t (*pf_os_thread_create)(
                                                void * const    thread_func,//指针常量指向的变量也是常量
                                                char * const    thread_name, //纯指针is常量 void * const th
                                                const uint16_t   StackDepth,
                                                void * const      parameter, //给task传入的 内部参数
                                                uint32_t     priority,  //wait 外部去写
                                                void ** const     task_handler ////把当前句柄指针传出-还得用二级指针
                                            );                                
    /*删除线程*/
    led_handler_status_t (*pf_os_thread_delete)(
                                                void * const thread_handler
                                            );
}handler_os_thread_t;
#endif  //end of OS_SUPPORTING



/*定义一个新的数据类型，函数指针类型
    这是外部调用的接口*/
typedef led_handler_status_t (*pf_handeler_led_control_t)(   bsp_led_handler_t * const self,    //pointer 需要在上面声明
                                            uint32_t     ,              //period ms
                                            uint32_t     ,               //times 
                                            led_proportion_t,       //proportion 3:1 2:1 1:1
                                                led_index_t      const     
                                        );

typedef led_handler_status_t (*pf_handeler_led_register_t)(      
                                            bsp_led_handler_t * const self,    //pointer 需要在上面声明
                                            bsp_led_driver_t * const led_driver,//实际的对象-把所有信息都告诉了
                                            led_index_t  * const led_index
            );


typedef struct{
    bsp_led_driver_t                    *led_instance_aarry[MAX_LED_INSTANCES]  ;
    uint32_t                             led_instance_num                       ;
}instance_mounted_t;

/*这里面定义的都是结构体的指针
    具体的函数在对应的结构体数据类型中封装着  到时候只需要调用对应的函数指针，指向定义的函数即可
        这也是二层索引  在结构体中定义函数指针 定义结构体指针 再指向定义的函数
    */
//******************************* fuctions ***********************************//

typedef struct bsp_led_handler
{

    /********************************内部数据变量***************************************************** */
    /*初始化次数的状态 这个类定义好了 实例化的时候只一次  防止多读多写（多个任务线程同时操作）
        借助枚举告知 实例化次数  是否实例化*/
            uint8_t                             is_inited;             /* is inited  0:no 1:yes*/
            //包含led对象（实例）的数组
            // bsp_led_driver_t *led_instance_aarry[10];//atterntion type not uint8_t
            instance_mounted_t                  register_led_instances;

    /********************************内部接口***************************************************** */
            handelr_timebase_t                 *p_timebase_ms;
            void                 *              queue_handler;///适配不同的os  queue
            void                 *              thread_handler;///适配不同的os  queue
#ifdef OS_SUPPORTING
            handler_os_delay_t                  *p_os_delay_ms;
            handler_os_queue_t                  *p_os_queue_interface;
             ///包含队列创建删除
            handler_os_critical_t               *p_os_critical;//临界区
            handler_os_thread_t                 *p_os_thread;//线程
#endif
    /********************************外部接口***************************************************** */
    //      FOR APP
    /**************设置目标变量（闪烁）的函数指针  外部接口调用  就是调用函数而已  不需要再定义结构体指针 二次索引     */
            pf_handeler_led_control_t             pf_led_control;
            //挂载的led
            pf_handeler_led_register_t            pf_led_register;                 
}bsp_led_handler_t;




/*************************declare 声明 函数实例化声明  *********************************** */

led_handler_status_t led_handler_instance(   
                                    bsp_led_handler_t           * const self    ,          //led handler struct pointer
                                                                //这个传入self可以直接操作结构体中的变量
                                    handelr_timebase_t       * const timebase_ms,         //timebase -tick ms
#ifdef OS_SUPPORTING        
                                    handler_os_delay_t       * const os_delay_ms,          //os delay ms   
                                    handler_os_queue_t          * const os_queue, //os queue interface
                                    handler_os_thread_t          * const os_thread, //os thread interface
                                    handler_os_critical_t       * const os_critical //os critical interface
#endif
);




#endif

//******************************* Defines ***********************************//



