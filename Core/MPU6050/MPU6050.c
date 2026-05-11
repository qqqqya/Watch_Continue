
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
//#include "bsp_mpu_handler.h"


#include "delay.h"
#include "elog.h"

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "queue.h"
#include "semphr.h"

// #include "iic_hal.h"//soft iic
#include "i2c.h"
#include "delay.h"

#include "mpu6050.h"
/******************************** Defines ************************************/
#define HANDLER_INITED          true
#define HANDLER_NOT_INITED      false

#define MY_MAX_DELAY 0xffffffffUL

#define MPU_WRITE_REG(p_mpu_driver, reg, p_data, len)\
                        p_mpu_driver->p_iic_instance->pf_iic_mem_write(\
                        p_mpu_driver->p_iic_instance->hi2c,\
                        (MPU_ADDR << 1) | 0,\
                        reg,\
                        IIC_MEMADD_SIZE_8BIT,\
                        p_data,\
                        len,\
                        TIME_OUT_MS)//0x68  设备写地址

#define MPU_READ_REG(p_mpu_driver, reg, p_data, len)\
                        p_mpu_driver->p_iic_instance->pf_iic_mem_read(\
                        p_mpu_driver->p_iic_instance->hi2c,\
                        (MPU_ADDR << 1) | 1,\
                        reg,\
                        IIC_MEMADD_SIZE_8BIT,\
                        p_data, \
                        len, \
                        TIME_OUT_MS)
// // 中断-PB5	硬件iic:PB10-SCL		PB3-SDA
// iic_bus_t MPU_iic_bus_instance={          //iic总线实例
//       .IIC_SCL_PORT=GPIOB,
//     .IIC_SCL_PIN=GPIO_PIN_10,
//     .IIC_SDA_PORT=GPIOB,
//     .IIC_SDA_PIN=GPIO_PIN_3//
// };

mpu_status_t  iic_driver_init        (void * bus){
    // main 中进行硬件iic初始化
    return MPU_OK;
}
mpu_status_t  iic_driver_deinit        (void * bus){
    __HAL_RCC_I2C2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3);
    return MPU_OK;
}

/*****************多个字节**************************/
mpu_status_t        iic_mem_read(void *hi2c, 
                              uint16_t dst_address, 
                              uint16_t mem_addr, 
                              uint16_t mem_size, 
                              uint8_t  *p_data, 
                              uint16_t size, 
                              uint32_t timeout)
{
    HAL_StatusTypeDef ret = HAL_OK;
	ret = HAL_I2C_Mem_Read(hi2c, dst_address, mem_addr, mem_size, p_data, size, timeout);
    if (ret != HAL_OK)
    {
        return MPU_ERROR;
    }
    return MPU_OK;
}//iic_bus_t *bus, uint8_t daddr,uint8_t reg,uint8_t length,uint8_t buff[]
mpu_status_t        iic_mem_write(void *hi2c,
                               uint16_t dst_address, 
                               uint16_t mem_addr, 
                               uint16_t mem_size, 
                               uint8_t  *p_data, 
                               uint16_t size, 
                               uint32_t timeout)
{
    HAL_StatusTypeDef ret = HAL_OK;
	ret = HAL_I2C_Mem_Write(hi2c, dst_address, mem_addr, mem_size, p_data, size, timeout);
    if (ret != HAL_OK)
    {
        return MPU_ERROR;
    }
    return MPU_OK;
}
mpu_status_t    iic_mem_read_dma(void *hi2c, 
                                 uint16_t dst_address, 
                                 uint16_t mem_addr, 
                                 uint16_t mem_size, 
                                 uint8_t  *p_data, 
                                 uint16_t size )
{
    HAL_StatusTypeDef ret = HAL_OK;
    ret = HAL_I2C_Mem_Read_DMA(hi2c, dst_address, mem_addr, mem_size, p_data, size);
    if (ret != HAL_OK)
    {
        return MPU_ERROR;
    }
    return MPU_OK;
}

mpu_iic_driver_instance_t MPU_iic_instance={
    .hi2c                = &hi2c2,
    .pf_iic_init         = iic_driver_init,
    .pf_iic_deinit       = iic_driver_deinit,
    .pf_iic_mem_read     = iic_mem_read,
    .pf_iic_mem_write    = iic_mem_write,
    .pf_iic_mem_read_dma = iic_mem_read_dma,
};
#if 1
mpu_status_t mpu_queue_create(uint32_t  item_num,
                                        uint32_t  size,
                                        void** Q_handler)
{
    * Q_handler=xQueueCreate(item_num, size);
    //在handler init函数里面打印Q_handler
    if(* Q_handler==NULL)
    {
        return MPU_ERROR;
    }
    return MPU_OK;
}
mpu_status_t mpu_queue_delete(void *  Q_handler)
{///osStatus_t osMessageQueueDelete (osMessageQueueId_t mq_id)
    vQueueDelete(Q_handler);
    return MPU_OK;
}
mpu_status_t mpu_queue_put(void * const Q_handler,
                                        void * const item,
                                        uint32_t timeout)
{//osMessageQueuePut (queue_handle, item, NULL, timeout)  osOK
    if(pdTRUE!=xQueueSend(Q_handler,item,timeout))
    {
        return MPU_ERROR;
    }
    return MPU_OK;
}
mpu_status_t mpu_queue_get(void * Q_handler,
                                        void * msg,
                                        uint32_t timeout)
{
    if(pdTRUE!=xQueueReceive(Q_handler,msg,timeout))
    {//mq_id, void *msg_ptr, uint8_t *msg_prio,  timeout)
        return MPU_ERROR;
    }
    return MPU_OK;
}
mpu_status_t mpu_queue_put_isr(void * const Q_handler,
                                        void * const item,
                                        long * const HigherPriorityTaskWoken)
{//osMessageQueuePut (queue_handle, item, NULL, timeout)  osOK
    xQueueSendFromISR(Q_handler,item,HigherPriorityTaskWoken);
    return MPU_OK;
}

mpu_status_t os_semaphore_create_mutex(void **mutex_handle)
{
    *mutex_handle = xSemaphoreCreateMutex();
    return MPU_OK;
}
mpu_status_t os_semaphore_delete_mutex(void * const mutex_handle)
{
    //vQueueDelete(mutex_handle);
    return MPU_OK;
}
mpu_status_t os_semaphore_take_mutex(void * const mutex_handle)
{
    xSemaphoreTake(mutex_handle, portMAX_DELAY);
    return MPU_OK;
}
mpu_status_t os_semaphore_give_mutex(void * const mutex_handle)
{//unlock
    xSemaphoreGive(mutex_handle);
    return MPU_OK;
}
mpu_status_t os_semaphore_create_binary(void **binary_handle)
{
    *binary_handle = xSemaphoreCreateBinary();
    return MPU_OK;
}
mpu_status_t os_semaphore_delete_binary(void * const binary_handle)
{
    //vQueueDelete(binary_handle);
    return MPU_OK;
}
mpu_status_t os_semaphore_signal_binary(void * const binary_handle)
{
    xSemaphoreGive(binary_handle);
    return MPU_OK;
}
mpu_status_t os_semaphore_signal_binary_isr(void * const binary_handle, long * const HigherPriorityTaskWoken )
{
    xSemaphoreGiveFromISR(binary_handle, HigherPriorityTaskWoken);
    return MPU_OK;
}
mpu_status_t os_semaphore_wait_binary(void * const binary_handle)
{
    xSemaphoreTake(binary_handle, portMAX_DELAY);
    return MPU_OK;
}
mpu_status_t os_semaphore_signal_notify(void * const notify_handle)
{
    xTaskNotifyGive(notify_handle);
    return MPU_OK;
}
mpu_status_t os_semaphore_signal_notify_isr(void * const notify_handle, uint32_t ulValue, uint32_t eAction, long * const HigherPriorityTaskWoken )
{
    xTaskNotifyFromISR( notify_handle, ulValue, eAction, HigherPriorityTaskWoken );
    return MPU_OK;
}
mpu_status_t os_semaphore_wait_notify(uint32_t ulBitsToClearOnEntry, 
                                                    uint32_t ulBitsToClearOnExit, 
                                                    uint32_t *pulNotificationValue, 
                                                    uint32_t timeout)
{
    xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
    return MPU_OK;
}

#endif
mpu_status_t mpu_get_tick_ms(uint32_t *ptick)
{
  //  *ptick = osKernelGetTickCount();
     *ptick = HAL_GetTick();
	return MPU_OK;
}
mpu_timebases_ms_t mpu_timebase_instance={
    .pf_timebase_gettickms=mpu_get_tick_ms,
};
mpu_delay_interface_t mpu_delay_instance={
    .pf_delay_init=delay_init,
    .pf_delay_ms=delay_ms,
    .pf_delay_us=delay_us,
};

void os_yield_delay(uint32_t ms)
{
    vTaskDelay(ms);
}
mpu_yield_interface_t mpu_yield_instance={
    .rtos_yield=os_yield_delay,
    // .rtos_yield=vTaskDelay,
};
mpu_os_interface_t mpu_os_instance={
    .Queuecreate = mpu_queue_create,
    .Queuedelete = mpu_queue_delete,
    .Queueput = mpu_queue_put,
    .Queueget = mpu_queue_get,
    .Queueput_put_isr=mpu_queue_put_isr,

    .semaphore_create_mutex=os_semaphore_create_mutex,
    .semaphore_take_mutex=os_semaphore_take_mutex,
    .semaphore_give_mutex=os_semaphore_give_mutex,
    .semaphore_delete_mutex=os_semaphore_delete_mutex,

    .semaphore_create_binary=os_semaphore_create_binary,
    .semaphore_delete_binary=os_semaphore_delete_binary,
    .semaphore_signal_binary=os_semaphore_signal_binary,
    .semaphore_signal_binary_isr=os_semaphore_signal_binary_isr,

    .semaphore_signal_notify_isr=os_semaphore_signal_notify_isr,
    .semaphore_wait_notify=os_semaphore_wait_notify,
};
// mpu_status_t mpu_driver_instance( p_mpu_driver,&MPU_iic_instance,
//                                     mpu_delay_interface_t       * const mpu_delay_interface_t, //delay接口
//                                     mpu_timebases_ms_t  *const p_timebase_ms,
//                 void (*callback_register)    (void (*callback)(void *, void *)),
//                 void (*callback_register_dma)(void (*callback)(void *, void *)),
// #ifdef OS_SUPPORTING
//                                     mpu_yield_interface_t   * const p_yield_interface,
//                                     mpu_os_interface_t * const p_os_interface, //os接口
//                     void *  Q_handler, //队列句柄指针
//                     void *  mutex_handler, //互斥锁句柄指针
//                     void *  notify_handler, //消息通知句柄
//                     void *  binary_handler //二值信号量句柄
// #endif
//                                     )

mpu_status_t MPU6050_Test(bsp_mpu_driver_t *p_mpu_driver)
{
    uint8_t test_val = 0x5A; // 随便挑一个特征值 (01011010)
    mpu_status_t ret;

    mpu_driver_instance(p_mpu_driver,
                            &MPU_iic_instance,
                            &mpu_delay_instance,
                            &mpu_timebase_instance,
                            NULL, // callback_register
                            NULL, // callback_register_dma
                            &mpu_yield_instance,
                            &mpu_os_instance,
                            NULL, // Q_handler
                            NULL, // mutex_handler
                            NULL, // notify_handler
                            NULL  // binary_handler
                            );
//  p_mpu_driver->p_iic_instance=&MPU_iic_instance;///挂载iic实例
//     // 1. 基础 I2C 初始化 (假设此时还没调 mpu_init)
//     p_mpu_driver->p_iic_instance->pf_iic_init(NULL); // 这里传 NULL 或者 &MPU_iic_bus_instance 都行，取决于你的实现细节

    // --------------------------------------------------------
    // 测试 1：读取 Device ID
    // --------------------------------------------------------
    uint8_t read_data = 0;
    ret = MPU_READ_REG(p_mpu_driver, MPU_DEVICE_ID_REG, &read_data, 1);
    if (ret != MPU_OK) {
        log_i("[ERROR] I2C \r\n");
        return MPU_ERRORRESOURCE;
    }
    if (read_data != MPU_ID) { // MPU_ID 通常是 0x68
        DEBUG_OUT("[ERROR] 0x%02X, 0x%02X\r\n", read_data, MPU_ID);
        return MPU_ERRORRESOURCE;
    }
    log_i("[PASS] 0x%02X\r\n", read_data);

    // --------------------------------------------------------
    // 测试 2：寄存器回环读写测试 (证明写通信也正常)
    // --------------------------------------------------------
    // 写入测试值
    ret = MPU_WRITE_REG(p_mpu_driver, MPU_SAMPLE_RATE_REG, &test_val, 1);
    if (ret != MPU_OK) {
        DEBUG_OUT("[ERROR] I2C 写操作失败。\r\n");
        return MPU_ERROR;
    }
    
    // 读回刚才写入的值
    read_data = 0; // 清空变量
    ret = MPU_READ_REG(p_mpu_driver, MPU_SAMPLE_RATE_REG, &read_data, 1);
    if (read_data != test_val) {
        DEBUG_OUT("[ERROR]  0x%02X, read: 0x%02X\r\n", test_val, read_data);
        return MPU_ERROR;
    }
    
    DEBUG_OUT("[PASS] \r\n");
    DEBUG_OUT("--- test pass ---\r\n");

    return MPU_OK;
}
// aht_status_t get_tick_ms(uint32_t *ptick)
// {
//   //  *ptick = osKernelGetTickCount();
//      *ptick = HAL_GetTick();
// 	return MPU_OK;
// }
// aht_status_t delay_own_ms(uint32_t ms)
// {
// //  vTaskDelay(ms);
// 	delay_ms(ms);
// 	return MPU_OK;
// }


// mpu_status_t os_queue_create(uint32_t  item_num,
//                                         uint32_t  size,
//                                         void** Q_handler)
// {
//     * Q_handler=osMessageQueueNew(item_num, size,NULL);//attr �?? [in] message queue attributes; NULL: default values.
//     //在handler init函数里面打印Q_handler
//     if(* Q_handler==NULL)
//     {
//         return MPU_ERROR;
//     }
//     return HANDLER_MPU_OK;
// }
// mpu_status_t os_queue_delete(void *  Q_handler)
// {///osStatus_t osMessageQueueDelete (osMessageQueueId_t mq_id)

//     return osOK==osMessageQueueDelete(Q_handler)?HANDLER_MPU_OK:MPU_ERROR;
// }
// mpu_status_t os_queue_put(void * const Q_handler,
//                                         void * const item,
//                                         uint32_t timeout)
// {//osMessageQueuePut (queue_handle, item, NULL, timeout)  osOK
//     if(osOK!=osMessageQueuePut(Q_handler,item,NULL,timeout))
//     {
//         return MPU_ERROR;
//     }
//     return HANDLER_MPU_OK;
// }
// mpu_status_t os_queue_get(void * Q_handler,
//                                         void * msg,
//                                         uint32_t timeout)
// {
//     if(osOK!=osMessageQueueGet(Q_handler,msg,NULL,timeout))
//     {//mq_id, void *msg_ptr, uint8_t *msg_prio,  timeout)
//       //msg_prio �?? [out] pointer to buffer for message priority or NULL.
//         return MPU_ERROR;
//     }
//     return HANDLER_MPU_OK;
// }

// yield_interface_t yield_interface={
//     .rtos_yield = delay_own_ms,
// };
// timebases_ms_t timebase_interface={
//     .pf_timebase_gettickms =get_tick_ms,
// };

// os_queue_t os_queue_interface={
//     .Queuecreate = os_queue_create,
//     .Queuedelete = os_queue_delete,
//     .Queueput = os_queue_put,
//     .Queueget = os_queue_get,
// };
// sensor_interface_i2c_timebase_delay_t input_arg={
//     .p_timebase_ms     = &timebase_interface,   //ms
//     .p_yield_interface = &yield_interface,
//     .p_MPU_iic_instance    = &aht_iic_func_instance,   //iic interface
//     .p_os_queue        = &os_queue_interface,
// };

// float g_temperature=0;
// void temp_humi_callback(float *humidity, float *temperature)
// {
//     log_d("callback:temperature = %f, humidity = %f", *temperature, *humidity);
    
//     //Forbiden :#1035-D: single-precision operand 
//     //implicitly converted to double-precision so with (float)1.5
//     g_temperature = (*temperature)*(float)1.5;
//     log_d("g_temperature = %f\r\n", g_temperature);
// }

// __weak void aht21_handler_thread_func(void *argument)
// {
//   /* USER CODE BEGIN temp_humi_handler_thread */
//   /* Infinite loop */
//   for(;;)
//   {
//     osDelay(1);
//   }
//   /* USER CODE END temp_humi_handler_thread */
// }

// void user_task_func(void *argument)
// {
//   /* USER CODE BEGIN StartDefaultTask */
//   /* Infinite loop */
// //  log_d("hello win\r\n");  
//   handler_aht_event_t event={
//     .lifetime = 8000,// 2s 2000ms  data freshness
//     .humi_temp_select = HUMI_TEMP_BOTH,
//     .pf_callback = temp_humi_callback,
//   };
// 	for(;;)   
// 	{	   
    
//     log_d("userTaskFunction start_send event\r\n");
//     log_d("userTaskFunction start_send event\r\n");
//     osDelay(2000);
//     bsp_aht21_handler_read(&event);
    
// 	}
//   /* USER CODE END StartDefaultTask */
// }
