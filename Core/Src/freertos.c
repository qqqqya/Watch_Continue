/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  * 
 * @par dependencies 
 * - bsp_led_driver.h
 * - stdint.h
 * - bsp_led_handler.h
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "bsp_led_driver.h"
#include "bsp_led_handler.h"
#include "queue.h"

#include "usart.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
led_status_t led_on_myown(void)
{
  // printf("led on my own\r\n");
	HAL_GPIO_WritePin(LED_Test_GPIO_Port, LED_Test_Pin, GPIO_PIN_RESET);
  return LED_OK;

};
led_status_t led_off_myown(void){
  // printf("led off my own\r\n");
	HAL_GPIO_WritePin(LED_Test_GPIO_Port, LED_Test_Pin, GPIO_PIN_SET);
  return LED_OK;
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

led_operation_t led_operation={
  .pf_bsp_led_off=led_off_myown,  //结构体中的函数指针  指向实际的led_off_myown函数
  .pf_bsp_led_on=led_on_myown,
};

timebase_t timebase_ms={
  .pf_get_tick_ms=get_tick_own_ms,  //结构体中的函数指针  指向实际的get_tick_own_ms函数
}; 

os_delay_t  os_delay_ms={
  .pf_osdelay_ms=osdelay_own_ms,  //结构体中的函数指针  指向实际的osDelay函数
};

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

//Driver layer test
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
  led_handler_status_t ret=HANDLER_OK;
  if(
      NULL        ==    queue_handler ||
      NULL        ==    item_data     ||
      timeout_ms    >   portMAX_DELAY 
  ){
    return HANDLER_ERRORRESOURCE;
  }
  ret=xQueueSendToBack(queue_handler, item_data, timeout_ms);
  if(pdPASS!=ret){
    ret  = HANDLER_ERROR;    //test send success
  }
  return ret;
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
  ret=xQueueReceive(queue_handler, item_data, timeout_ms);
  if(pdPASS!=ret){
    ret  = HANDLER_ERROR;    //test receive success
  }
  return ret;
}
led_handler_status_t os_handler_queue_delete(
                                    void * queue_handler,
                                    uint32_t timeout_ms)
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

/////Handler layer test
void Test2(){
  
/********************************Handler layer***************************************************** */
  led_handler_status_t ret_handler=HANDLER_OK;
	bsp_led_handler_t handler_1;  //定义了结构体 在内存中有了空间
	led_handler_instance(&handler_1, 
                      &handler1_timebase_ms, 
                      &handler1_delay_ms,
                      &handler1_os_Queue,
                      &os_critical
                    );
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
}

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */


/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */



/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
	printf("hello win\r\n");     
//	Test1();
Test2();
  printf("nihao win2222\r\n");     

	for(;;)   
	{	 
	//	HAL_GPIO_TogglePin(LED_Test_GPIO_Port, LED_Test_Pin);
	//	HAL_Delay(500);
		osDelay(1);
	}
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

