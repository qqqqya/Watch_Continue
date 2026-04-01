/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * 
******************************* includes ***********************************
******************************* fuctions ***********************************
 * @file system_adaptation.h
 *  
 * @par dependencies 
 * - stdio.h
 * - stdint.h
 * 
 * #include "system_adaptation.h"
#include "stdio.h"
#include "stdint.h"
#include "bsp_led_driver.h"
#include "bsp_led_handler.h"
#include "system_adaptation.h"
 * @author Yan | R&D Dept. | EternalChip
 *
 * @brief Provides HAL APIs for LED control and operations.
 * 
 * Usage:
 * Call functions directly.
 * 
 * @version V1.0 2026-03-26 --sys adaptation
 *
 * @note 1 tab == 4 spaces

 *****************************************************************************/
#ifndef __SYSTEM_ADAPATION_H__
#define __SYSTEM_ADAPATION_H__
//******************************* includes ***********************************//
///1.编译系统提供的头文件 compiling system provided headers
#include "stdio.h"
#include "stdint.h"

/***2.	MCU Layer********************* */
//2.1 CPU Driver
#include "cmsis_os.h"	//ARM supportted

//2.2 Core
#include "main.h"
#include "usart.h"
#include "gpio.h"
//3.	OS Layer
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


///4.	BSP提供的头文件 compiling BSP provided headers
#include "bsp_led_driver.h"
#include "bsp_led_handler.h"

// #include "system_configuration.h"    //定义的基本上是宏
//******************************* Defines ***********************************//

typedef enum
{
	SYSTEM_OK             = 0,      /* Operation completed successfully         */
	SYSTEM_ERROR          = 1,      /* General runtime error                    */
	SYSTEM_ERRORTIMEOUT   = 2,      /* Operation timed out                      */
	SYSTEM_ERRORRESOURCE  = 3,      /* Required resource is unavailable         */
	SYSTEM_ERRORPARAMETER = 4,      /* Invalid parameter error                  */
	SYSTEM_ERRORNOMEMORY  = 5,      /* Memory allocation failed                 */
	SYSTEM_ERRORISR       = 6,      /* Not allowed in ISR context               */
	SYSTEM_RESERVED       = 0xFF,   /* Reserved status                          */
} system_status_t;

#define INIT_PATTERN_SYSTEM  (uint8_t)0xEE
//******************************* Defines ***********************************//

//**************************BSP Layers**************************//

//************** BSP Layer Targets***********************//

extern bsp_led_handler_t handler_1;  //定义了结构体 在内存中有了空间
extern bsp_led_driver_t led_1;

// __attribute__((used, section("BSP_target")))
// static bsp_led_handler_t handler_1={INIT_PATTERN_SYSTEM};
// __attribute__((used, section("BSP_target")))		//((used, section(**)));;;
// static bsp_led_driver_t  led_1={INIT_PATTERN_SYSTEM};
//************** BSP Layer adapters***********************//

//**********LED Driver Layer**********//


//*********LED Handler Layer**********//

led_status_t led_on_myown(void);
led_status_t led_off_myown(void);
extern led_operation_t led_operation;

led_status_t get_tick_own_ms(uint32_t *ptick);
led_status_t osdelay_own_ms(uint32_t delay_ms);
extern timebase_t timebase_ms; 
extern os_delay_t  os_delay_ms;

//Driver layer test
void Test1();

/* handler layer define ------------------------------------------------------------*/
/* handler function instance ----------------------------------------------*/
led_handler_status_t os_handler_queue_create(
                                                const uint32_t item_num,
                                                const uint32_t size,
                                                void  ** const queue_handler                                                                                    
);
led_handler_status_t os_handler_queue_put(
                               void *  queue_handler,
                               void *  item_data,
                               uint32_t timeout_ms
);
  
led_handler_status_t os_handler_queue_get(
                               void *  queue_handler,
                               void *  item_data,
                               uint32_t timeout_ms
);
led_handler_status_t os_handler_queue_delete(                   void * queue_handler);
led_handler_status_t os_critical_enter(void);
led_handler_status_t os_critical_exit(void);

led_handler_status_t os_handlerdelay_own_ms(uint32_t delay_ms);
led_handler_status_t handler_get_tick_ms(uint32_t *ptick);
led_handler_status_t os_handler_thread_create(
                                                void * const    thread_func,//指针常量指向的变量也是常量
                                                char * const    thread_name, //纯指针is常量 void * const th
                                                const uint16_t   StackDepth,
                                                void * const      parameter, //给task传入的 内部参数
                                                uint32_t     priority,  //wait 外部去写
                                                void ** const     task_handler ////把当前句柄指针传出-还得用二级指针
);

led_handler_status_t os_handler_thread_delete(
                                                void * const thread_handler
);
/* handler struct instance ----------------------------------------------*/
extern handelr_timebase_t  handler1_timebase_ms;    //实际是在system_adaptation.c里边定义的 
extern handler_os_delay_t handler1_delay_ms;        //这里只是extern声明一下 不然会multiply definition
extern handler_os_queue_t  handler1_os_Queue;
extern handler_os_critical_t  os_critical;
extern handler_os_thread_t  os_thread;

/////Handler layer test
void Test2();
void Test3(void);

/**
 * @brief init all the resources.
 * 
 * Steps:
 *  1, mix up all the resources in this system.
 *  
 * @brief[]
 * @param[in] self      : Pointer to the input data.
 * @param[in] led_ops   : Length of the input data.
 * @param[in] os_delay  : Pointer to the input data.
 * @param[in] time_base : Pointer to the input data.
 * 
 * @return led_status_t : The status of running
 * 
 * */
led_status_t system_init_resources ( void );


#endif
