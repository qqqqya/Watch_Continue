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


__attribute__((used, section("BSP_target")))
static bsp_led_handler_t handler_1={INIT_PATTERN_SYSTEM};
__attribute__((used, section("BSP_target")))		//((used, section(**)));;;
static bsp_led_driver_t  led_1={INIT_PATTERN_SYSTEM};

led_status_t led_on_myown(void)
{
  // printf("led on my own\r\n");
   HAL_GPIO_WritePin(LED_Test_GPIO_Port, LED_Test_Pin, GPIO_PIN_RESET);
  return LED_OK;

}
led_status_t led_off_myown(void){
  // printf("led off my own\r\n");
	HAL_GPIO_WritePin(LED_Test_GPIO_Port, LED_Test_Pin, GPIO_PIN_SET);
  return LED_OK;
}

led_operation_t led_operation={
  .pf_bsp_led_off=led_off_myown,  //结构体中的函数指针  指向实际的led_off_myown函数
  .pf_bsp_led_on=led_on_myown,
};

led_status_t get_tick_own_ms(uint32_t *ptick)
{
  //  *ptick = osKernelGetTickCount();
     *ptick = HAL_GetTick();

	return LED_OK;
}
led_status_t osdelay_own_ms(uint32_t delay_ms)
{
  osDelay(delay_ms);
  // vTaskDelay(delay_ms);
  // 解耦了osDelay函数  可以在其他地方调用
  return LED_OK;
}

timebase_t timebase_ms={
  .pf_get_tick_ms=get_tick_own_ms,  //结构体中的函数指针  指向实际的get_tick_own_ms函数
}; 

os_delay_t  os_delay_ms={
  .pf_osdelay_ms=osdelay_own_ms,  //结构体中的函数指针  指向实际的osDelay函数
};

//Driver layer test -- test led driver instance
void Test1(){
	bsp_led_driver_t led_1;  //定义了结构体 在内存中有了空间
	bsp_led_driver_t led_2;  //定义了结构体 在内存中有了空间
  /*传入参数也都定义好了 h文件只是声明好了*/
	led_driver_instance(&led_1, 
                      &led_operation, 
                      &timebase_ms, 
                      &os_delay_ms);
	led_driver_instance(&led_2, 
                      &led_operation, 
                      &timebase_ms, 
                      &os_delay_ms);
  led_1.pf_led_control(&led_1, 5, 2, PROPORTIONN_1_1);//10ms闪烁2times
  led_2.pf_led_control(&led_2, 10, 1, PROPORTIONN_1_1);//

  uint32_t tick_;
// led_1.p_timebase_ms->pf_get_tick_ms(&tick);
	
printf("%u\r\n", led_1.p_timebase_ms->pf_get_tick_ms(&tick_));  //测试gettick
	///注意这里还要通过结构体内部的函数指针指向函数才能调用
// printf("%d\r\n", &led_1.p_timebase_ms());  //测试gettick


}

/* handler layer define ------------------------------------------------------------*/
/* handler function instance ----------------------------------------------*/
led_handler_status_t os_handler_queue_create(
                                                const uint32_t item_num,
                                                const uint32_t size,
                                                void  ** const queue_handler                                                                                    
){
  QueueHandle_t tempQueueHandler=NULL;
  tempQueueHandler = xQueueCreate(item_num, size);
  if(NULL==tempQueueHandler){
    return HANDLER_ERRORRESOURCE;
  }

  *queue_handler=tempQueueHandler;  //return queue_handler
  return HANDLER_OK;
}
led_handler_status_t os_handler_queue_put(
                               void *  queue_handler,
                               void *  item_data,
                               uint32_t timeout_ms
){  //xQueueSendToBack(data->xQueRx, &data->rxdata, NULL);
#ifdef DEBUG
      printf("os_queue_put_handler_1 \r\n");
#endif // HANLDER_1_DEBUG
  led_handler_status_t ret=HANDLER_OK;
  if(
      NULL        ==    queue_handler ||
      NULL        ==    item_data     ||
      timeout_ms    >   portMAX_DELAY 
  ){
    return HANDLER_ERRORRESOURCE;
  }
  else{
    ret=xQueueSend(queue_handler, item_data, timeout_ms);
    // if(pdPASS!=ret){
    //   ret  = HANDLER_ERROR;    //test send success  ToBack
    // }
  }
  
  return (ret ==  pdTRUE) ? HANDLER_OK:HANDLER_ERROR;
}

led_handler_status_t os_handler_queue_get(
                               void *  queue_handler,
                               void *  item_data,
                               uint32_t timeout_ms
){  //xQueueReceive(Privdata->xDMAQueRx, &dmabatch_data, timeout_ms)
  led_handler_status_t ret=HANDLER_OK;
  if(
      NULL        ==    queue_handler ||
      NULL        ==    item_data     ||
      timeout_ms    >   portMAX_DELAY 
  ){
    return HANDLER_ERRORRESOURCE;
  }
  printf("////////////////get_handler_1 \r\n");
  ret=xQueueReceive(queue_handler, item_data, timeout_ms);
  // if(pdPASS!=ret){
  //   ret  = HANDLER_ERROR;    //test receive success
  // }
  return (ret ==  pdTRUE) ? HANDLER_OK:HANDLER_ERROR;
}
led_handler_status_t os_handler_queue_delete(
                                    void * queue_handler)
{
  if(NULL==queue_handler){
    return HANDLER_ERRORRESOURCE;
  }
  vQueueDelete(queue_handler);
  return HANDLER_OK;
}
led_handler_status_t os_critical_enter(void)
{
#ifdef HANLDER_1_DEBUG
      printf("os_critical_enter_handler_1 \r\n");
#endif // HANLDER_1_DEBUG
  //TBD:if Already in critical state, return error
  vPortEnterCritical();
  return HANDLER_OK;
}
led_handler_status_t os_critical_exit(void){
#ifdef HANLDER_1_DEBUG
      printf("os_critical_exit_handler_1 \r\n");
#endif // HANLDER_1_DEBUG
  //TBD:if Already in critical state, return error
  vPortExitCritical();
  return HANDLER_OK;
}

led_handler_status_t os_handlerdelay_own_ms(uint32_t delay_ms)
{
  vTaskDelay(delay_ms);
  // 解耦了osDelay函数  可以在其他地方调用  osDelay(delay_ms);
  return HANDLER_OK;
}
led_handler_status_t handler_get_tick_ms(uint32_t *ptick)
{
  // *ptick = osKernelGetTickCount();
  if(NULL==ptick){
    return HANDLER_ERRORRESOURCE;
  }
  *ptick = HAL_GetTick();
  return HANDLER_OK;
}
led_handler_status_t os_handler_thread_create(
                                                void * const    thread_func,//指针常量指向的变量也是常量
                                                char * const    thread_name, //纯指针is常量 void * const th
                                                const uint16_t   StackDepth,
                                                void * const      parameter, //给task传入的 内部参数
                                                uint32_t     priority,  //wait 外部去写
                                                void ** const     task_handler ////把当前句柄指针传出-还得用二级指针
){
  led_handler_status_t ret=HANDLER_OK;
  if(NULL==task_handler){
    return HANDLER_ERRORRESOURCE;
  }
#ifdef DEBUG
  printf("parameter in thread  = %p\r\n",parameter);//其实传的是 bsp_led_handler
      printf("thread_create_handler_1 Down ----------------------\r\n");
#endif
  ret =  xTaskCreate((TaskFunction_t)thread_func, 
                        "handler1_thread", 
                                    4*128, 
                                parameter, //其实传的是 bsp_led_handler
          (osPriority_t) osPriorityNormal,
        (TaskHandle_t * const)task_handler 
            );

  return ret==pdPASS?HANDLER_OK:HANDLER_ERROR;
                                              
} 

led_handler_status_t os_handler_thread_delete(
                                                void * const thread_handler
){
  if(NULL==thread_handler){
    return HANDLER_ERRORRESOURCE;
  }
  vTaskDelete((TaskHandle_t)thread_handler);
  return HANDLER_OK;
}
/* handler struct instance ----------------------------------------------*/
handelr_timebase_t  handler1_timebase_ms={

  .pf_get_tick_ms=handler_get_tick_ms,  //结构体中的函数指针  指向实际的get_tick_own_ms函数
};

handler_os_delay_t handler1_delay_ms={
  .pf_osdelay_ms=os_handlerdelay_own_ms,  //结构体中的函数指针  指向实际的osDelay函数
};

handler_os_queue_t  handler1_os_Queue={
  .pf_os_queue_create=os_handler_queue_create,
  .pf_os_queue_delete=os_handler_queue_delete,
  .pf_os_queue_put=os_handler_queue_put,
  .pf_os_queue_get=os_handler_queue_get,

};
handler_os_critical_t  os_critical={
  .pf_os_critical_enter=os_critical_enter,
  .pf_os_critical_exit=os_critical_exit,
};

handler_os_thread_t  os_thread={
  .pf_os_thread_create=os_handler_thread_create,
  .pf_os_thread_delete=os_handler_thread_delete,
};

/////Handler layer test
void Test2(){
  
/********************************Handler layer***************************************************** */
  led_handler_status_t ret_handler=HANDLER_OK;
	bsp_led_handler_t handler_1;  //定义了结构体 在内存中有了空间
	led_handler_instance(&handler_1, 
                      &handler1_timebase_ms, 
                      &handler1_delay_ms,
                      &handler1_os_Queue,
                      &os_thread,
                      &os_critical
                    );
  printf("_handler = %p\r\n",&handler_1);

/********************************Driver layer***************************************************** */
  led_status_t ret_led=LED_OK;                  
  bsp_led_driver_t led_1;  //定义了结构体 在内存中有了空间
	bsp_led_driver_t led_2;  //定义了结构体 在内存中有了空间
  /*传入参数也都定义好了 h文件只是声明好了*/
	led_driver_instance(&led_1, 
                      &led_operation, 
                      &timebase_ms, 
                      &os_delay_ms);
	led_driver_instance(&led_2, 
                      &led_operation, 
                      &timebase_ms, 
                      &os_delay_ms);
  led_1.pf_led_control(&led_1, 4, 2, PROPORTIONN_1_1);//10ms闪烁2 times    
  
/********************************Register LED***************************************************** */
  led_index_t LED_index=LED_NOT_INITIALIZED;
  ret_handler=handler_1.pf_led_register(&handler_1,
										                    &led_1,
										                    &LED_index);
  printf("register return ret_handler=%d\r\n",ret_handler);
  printf("LED_index=%d\r\n",LED_index);
  
  ret_handler=handler_1.pf_led_register(&handler_1,
										                    &led_2,
										                    &LED_index);
  printf("register return ret_handler=%d\r\n",ret_handler);
  printf("LED_index=%d\r\n",LED_index);
  
  handler_1.pf_led_control(&handler_1, 4, 2, PROPORTIONN_1_1,LED1);//10ms闪烁2 times    
  printf("event over\r\n");
  
}

 void Test3(void){
 ///系统集成工程师
 /********************************Handler layer******************************************** */


bsp_led_handler_t handler_1;  //定义了结构体 在内存中有了空间
 bsp_led_driver_t led_1;
  
   led_handler_status_t ret_handler=HANDLER_OK;
   led_status_t ret_led=LED_OK;  	

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

 ///APP工程师
 /********************************Handler layer******************************************** */
   led_index_t LED_index=LED_NOT_INITIALIZED;
   //app 线程1
   ret_handler=handler_1.pf_led_register(&handler_1,
 										                    &led_1,
 										                    &LED_index);
   printf("register return ret_handler=%d\r\n",ret_handler);
   printf("LED_index=%d\r\n",LED_index);
   if(HANDLER_OK==ret_handler){
     printf("_register_finished\r\n");
   }
   //app 线程2
   ret_handler=handler_1.pf_led_control(&handler_1, 4, 2, PROPORTIONN_1_1,LED_index);//10ms闪烁2 times    
   if(HANDLER_OK==ret_handler){
     printf("_control_OK\r\n");
   }
   //到这里app  结束  app结束调用
   while(1);

 }

/******************************** fuctions ************************************/

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

