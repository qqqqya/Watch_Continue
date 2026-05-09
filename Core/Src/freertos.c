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
///1.编译系统提供的头文件 compiling system provided headers

/***2.	MCU Layer********************* */
//2.1 CPU Driver
#include "cmsis_os.h"
//2.2 Core
#include "iic_hal.h"

/***3.	OS Layer********************* */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/***4.	compiling BSP provided headers******* */

#include "bsp_aht21_driver.h"
#include "bsp_aht21_handler.h"
#include "delay.h"
#include "system_adaptation.h"
#include "elog.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

iic_bus_t iic_instance={          //iic总线实例
      .IIC_SCL_PORT=GPIOB,
    .IIC_SCL_PIN=GPIO_PIN_14,
    .IIC_SDA_PORT=GPIOB,
    .IIC_SDA_PIN=GPIO_PIN_13
};

int8_t        my_iic_init        (void * bus){
    IICInit(&iic_instance);
    return AHT_OK;
}
int8_t        my_iic_start       (void * bus){
    IICStart(&iic_instance);
    return AHT_OK;
}

int8_t        my_iic_stop      (void * bus){
    IICStop(&iic_instance);
    return AHT_OK;
}
int8_t        my_iic_waitack   (void * bus){
    IICWaitAck(&iic_instance);
    return AHT_OK;
}
int8_t        my_iic_sendack   (void * bus){
    IICSendAck(&iic_instance);
    return AHT_OK;
}
int8_t        my_iic_sendnotack   (void * bus){
    IICSendNotAck(&iic_instance);
    return AHT_OK;
}
int8_t        my_iic_sendbyte   (void * bus,uint8_t data){
    IICSendByte(&iic_instance,data);
    return AHT_OK;
}
int8_t        my_iic_recevbyte       (void * bus,uint8_t * data){
     *data = IICReceiveByte(&iic_instance);
    return AHT_OK;
}
/*****************多个字节暂时不用看后续教�????***************************/
int8_t        my_iic_send_multibyte       (void * bus,uint8_t * pdata,uint8_t size){
    IIC_Write_Multi_Byte(&iic_instance,NULL,NULL,size,pdata); //pdata是地�????�????般是个数�????
    return AHT_OK;                    //dev addr + reg addr + size + pdata
}//iic_bus_t *bus, uint8_t daddr,uint8_t reg,uint8_t length,uint8_t buff[]
int8_t        my_iic_rece_multibyte       (void * bus,uint8_t * pdata,uint8_t size){
    IIC_Read_Multi_Byte(&iic_instance,NULL,NULL,size,pdata);
    return AHT_OK;                    //dev addr + reg addr + size + pdata      
}
static int8_t my_iic_critical_enter(void)//os_critical_enter
{
    vPortEnterCritical();
}
static int8_t my_iic_critical_exit(void)
{
    vPortExitCritical();
}
aht_status_t get_tick_ms(uint32_t *ptick)
{
  //  *ptick = osKernelGetTickCount();
     *ptick = HAL_GetTick();
	return AHT_OK;
}
aht_status_t delay_own_ms(uint32_t ms)
{
//  vTaskDelay(ms);
	delay_ms(ms);
	return AHT_OK;
}


handler_aht_status_t os_queue_create_myown(uint32_t  item_num,
                                        uint32_t  size,
                                        void** Q_handler)
{
    * Q_handler=osMessageQueueNew(item_num, size,NULL);//attr �?? [in] message queue attributes; NULL: default values.
    //在handler init函数里面打印Q_handler
    if(* Q_handler==NULL)
    {
        return HANDLER_AHT_ERROR;
    }
    return HANDLER_AHT_OK;
}
handler_aht_status_t os_queue_delete_myown(void *  Q_handler)
{///osStatus_t osMessageQueueDelete (osMessageQueueId_t mq_id)

    return osOK==osMessageQueueDelete(Q_handler)?HANDLER_AHT_OK:HANDLER_AHT_ERROR;
}
handler_aht_status_t os_queue_put_myown(void * const Q_handler,
                                        void * const item,
                                        uint32_t timeout)
{//osMessageQueuePut (queue_handle, item, NULL, timeout)  osOK
    if(osOK!=osMessageQueuePut(Q_handler,item,NULL,timeout))
    {
        return HANDLER_AHT_ERROR;
    }
    return HANDLER_AHT_OK;
}
handler_aht_status_t os_queue_get_myown(void * Q_handler,
                                        void * msg,
                                        uint32_t timeout)
{
    if(osOK!=osMessageQueueGet(Q_handler,msg,NULL,timeout))
    {//mq_id, void *msg_ptr, uint8_t *msg_prio,  timeout)
      //msg_prio �?? [out] pointer to buffer for message priority or NULL.
        return HANDLER_AHT_ERROR;
    }
    return HANDLER_AHT_OK;
}

static iic_driver_instance_t aht_iic_func_instance={
  .pf_iic_init              = my_iic_init,     //iic_hal.h中的函数
  .pf_iic_deinit            = NULL,     //反初始化函数 目前没有实现
  .pf_iic_start             = my_iic_start,
  .pf_iic_stop              = my_iic_stop,
  .pf_iic_waitack           = my_iic_waitack,
  .pf_iic_send_ack          = my_iic_sendack,
  .pf_iic_send_notack       = my_iic_sendnotack,

  .pf_iic_sendbytes         = my_iic_sendbyte,
  .pf_iic_recevbytes        = my_iic_recevbyte,
  .pf_iic_sendmulti_bytes   = my_iic_send_multibyte,//iic_bus_t *bus, uint8_t daddr,uint8_t reg,uint8_t length,uint8_t buff[]
  .pf_iic_recevmulti_bytes  = my_iic_rece_multibyte,
  

  .pf_enter_critical        = my_iic_critical_enter,
  .pf_exit_critical         = my_iic_critical_exit,

};

static yield_interface_t yield_interface={
    .rtos_yield = delay_own_ms,
};
static timebases_ms_t timebase_interface={
    .pf_timebase_gettickms =get_tick_ms,
};

static os_queue_t os_queue_interface={
    .Queuecreate = os_queue_create_myown,
    .Queuedelete = os_queue_delete_myown,
    .Queueput = os_queue_put_myown,
    .Queueget = os_queue_get_myown,
};
/**四个interface分别从core，driver还有OS层传过来，�?�过handler层传给driver�?? */
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
osThreadId_t humi_temp_TaskHandle;
const osThreadAttr_t humi_temp_Task_attributes = {
  .name = "humi_temp_Task",
  .stack_size = 128 *6,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t user_TaskHandle;
const osThreadAttr_t user_Task_attributes = {
  .name = "user_task",
  .stack_size = 128 * 5, 
//  .priority = (osPriority_t) osPriorityNormal,
	.priority = (osPriority_t) osPriorityBelowNormal,
};

void aht21_handler_thread_func(void *argument);
void user_task_func(void *argument);
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
static sensor_interface_i2c_timebase_delay_t input_arg={
    .p_timebase_ms     = &timebase_interface,   //ms
    .p_yield_interface = &yield_interface,
    .p_iic_instance    = &aht_iic_func_instance,   //iic interface
    .p_os_queue        = &os_queue_interface,
};

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
  humi_temp_TaskHandle = osThreadNew(aht21_handler_thread_func, //taskfunc 是weak 在handler.c中重定义实现
                                      &input_arg, 
                                      &humi_temp_Task_attributes);
  /* user_task 是发送队列消息的线程 */
  user_TaskHandle = osThreadNew(user_task_func,
                                NULL,  
                                &user_Task_attributes);



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
	for(;;)   
	{	   
    osDelay(2);
    
	}
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
float g_temperature=0;
void temp_humi_callback(float *humidity, float *temperature)
{
    log_d("callback:temperature = %f, humidity = %f", *temperature, *humidity);
    
    //Forbiden :#1035-D: single-precision operand 
    //implicitly converted to double-precision so with (float)1.5
    g_temperature = (*temperature)*(float)1.5;
    log_d("g_temperature = %f\r\n", g_temperature);
}

__weak void aht21_handler_thread_func(void *argument)
{
  /* USER CODE BEGIN temp_humi_handler_thread */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END temp_humi_handler_thread */
}

void user_task_func(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
//  log_d("hello win\r\n");  
  handler_aht_event_t event={
    .lifetime = 8000,// 2s 2000ms  data freshness
    .humi_temp_select = HUMI_TEMP_BOTH,
    .pf_callback = temp_humi_callback,
  };
	for(;;)   
	{	   
    
    log_d("userTaskFunction start_send event\r\n");
    log_d("userTaskFunction start_send event\r\n");
    osDelay(2000);
    bsp_aht21_handler_read(&event);
    
	}
  /* USER CODE END StartDefaultTask */
}
/* USER CODE END Application */

