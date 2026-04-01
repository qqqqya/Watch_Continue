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
 * @version V1.0 2026-03-14
 *
 * @note 1 tab == 4 spaces
 * 
 * 
******************************* includes ***********************************
******************************* fuctions ***********************************
 *****************************************************************************/

/******************************** includes ************************************/
#include "system_adaptation.h"

/******************************** fuctions ************************************/
#if 0
led_status_t system_init_resources ( void )
{
  printf("System Starting.....\r\n");
  led_handler_status_t ret_handler=HANDLER_OK;
  led_status_t ret_led=LED_OK;  	
	// bsp_led_handler_t handler_1;  //定义了结构体 在内存中有了空间
  // bsp_led_driver_t led_1;
  
  ret_handler=led_handler_instance(&handler_1, 
                      &handler1_timebase_ms, 
                      &handler1_delay_ms,
                      &handler1_os_Queue,
                      &os_thread,
                      &os_critical
                    );
  if(HANDLER_OK==ret_handler){
    printf("_HANDLER_instance_finished\r\n");
  }
  printf("_handler_instance_finished handler = %p\r\n",&handler_1);


	ret_led=led_driver_instance(&led_1, 
                      &led_operation, 
                      &timebase_ms, 
                      &os_delay_ms);
  if(LED_OK==ret_led){
    printf("_led_instance_finished\r\n");
  }
  //根据自己的  sct文件内存布局 来设置p_handler_1的地址
  bsp_led_handler_t * p_handler_1 = (bsp_led_handler_t *) (0x08007000);


}
#endif
