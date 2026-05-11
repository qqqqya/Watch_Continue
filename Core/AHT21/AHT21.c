
/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_aht21_handler.c
 * 
 * @par dependencies 
 * - stdio.h
 * - stdint.h
 * - "bsp_aht21_driver.h"
 * - elog.h
 * 
 * 
#include "stdint.h"
#include "stm32f4xx_hal.h"
#include "queue.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"
 * @author Jack | R&D Dept. | EternalChip ?????
 *
 * @brief Provides HAL APIs for LED control and operations.
 * 
 * Usage:
 * Call functions directly.
 * 
 * @version V1.0 2026年4月12日  16点55分
 *
 * @note 1 tab == 4 spaces
 * 
 *****************************************************************************/
#include "bsp_aht21_handler.h"
#include "bsp_aht21_driver.h"
#include "bsp_aht21_handler.h"
#include "delay.h"
#include "system_adaptation.h"
#include "elog.h"

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "iic_hal.h"
#include "queue.h"
/******************************** Defines ************************************/
#define HANDLER_INITED          true
#define HANDLER_NOT_INITED      false

#define MY_MAX_DELAY 0xffffffffUL

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
int8_t my_iic_critical_enter(void)//os_critical_enter
{
    vPortEnterCritical();
}
int8_t my_iic_critical_exit(void)
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

iic_driver_instance_t aht_iic_func_instance={
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

yield_interface_t yield_interface={
    .rtos_yield = delay_own_ms,
};
timebases_ms_t timebase_interface={
    .pf_timebase_gettickms =get_tick_ms,
};

os_queue_t os_queue_interface={
    .Queuecreate = os_queue_create_myown,
    .Queuedelete = os_queue_delete_myown,
    .Queueput = os_queue_put_myown,
    .Queueget = os_queue_get_myown,
};
sensor_interface_i2c_timebase_delay_t input_arg={
    .p_timebase_ms     = &timebase_interface,   //ms
    .p_yield_interface = &yield_interface,
    .p_iic_instance    = &aht_iic_func_instance,   //iic interface
    .p_os_queue        = &os_queue_interface,
};

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
    // bsp_aht21_handler_read(&event);
    
	}
  /* USER CODE END StartDefaultTask */
}
