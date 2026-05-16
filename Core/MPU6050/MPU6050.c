
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
#include "mid_circle_buffer.h"
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
mpu_input_arg_t input_arg_mpu={
    .p_delay_interface=&mpu_delay_instance,
    .p_iic_driver_instance=&MPU_iic_instance,
    .p_timebases_ms=&mpu_timebase_instance,
    .p_yield_interface=&mpu_yield_instance,
    .p_os_interface=&mpu_os_instance,
};
#if 0
  bsp_mpu_driver_t p_mpu_driver_instance={0};
void (*pf_pin_interrupt_callback)(void *, void *) = NULL;
void (*pf_dma_interrupt_callback)(void *, void *) = NULL;


#if 1
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	// INT test	
	// HAL_GPIO_WritePin(INT_FUNC_GPIO_Port, INT_FUNC_Pin, 0); // PA2
	
	//log_i("HAL_GPIO_EXTI_Callback");

	if(NULL != pf_pin_interrupt_callback)
	{
		pf_pin_interrupt_callback(&p_mpu_driver_instance, NULL);
		/*
		log_i("mpu6050 data: \r\n accel_x : %f, accel_y : %f, accel_z : %f \r\n gyro_x : %f, gyro_y : %f, gyro_z : %f \r\n temperature : %f \r\n ax : %f, ay : %f, az : %f\r\n gx : %f, gy : %f, gz : %f\r\n",
			  mpu6050_data.accel_x_raw,
			  mpu6050_data.accel_y_raw,
			  mpu6050_data.accel_z_raw,
			  mpu6050_data.gyro_x_raw,
			  mpu6050_data.gyro_y_raw,
			  mpu6050_data.gyro_z_raw,
			  mpu6050_data.temperature,
			  mpu6050_data.ax,
			  mpu6050_data.ay,
			  mpu6050_data.az,
			  mpu6050_data.gx,
			  mpu6050_data.gy,
			  mpu6050_data.gz);
		*/
	}
	// INT test
	// HAL_GPIO_WritePin(INT_FUNC_GPIO_Port, INT_FUNC_Pin, 1); // PA2
}
#endif

// DMA完成回调函数
#if 1
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	// HAL_GPIO_WritePin(DMA_FUNC_GPIO_Port, DMA_FUNC_Pin, 0); // PA1
	//log_i("HAL_I2C_MemRxCpltCallback");
	if(hi2c == &hi2c2)
	{
		pf_dma_interrupt_callback(&p_mpu_driver_instance, NULL);
	}  
	// HAL_GPIO_WritePin(DMA_FUNC_GPIO_Port, DMA_FUNC_Pin, 1); // PA1
}
#endif
#endif

#if 0
mpu_status_t MPU6050_Test(bsp_mpu_driver_t *p_mpu_driver)
{
    uint8_t test_val = 0x5A; // 随便挑一个特征值 (01011010)
    mpu_status_t ret;

    mpu_driver_instance(p_mpu_driver,
                            &MPU_iic_instance,
                            &mpu_delay_instance,
                            &mpu_timebase_instance,
                            callback_register, // callback_register
                            callback_register_dma, // callback_register_dma
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

    DEBUG_OUT("--- test pass ---\r\n");
    return MPU_OK;
}
#endif

#if 0
extern circular_buffer_t circular_buf;//外部声明环形缓冲区实例  定义在circle_buffer.c中
void unpack_task(void *p_args)
{
    log_i("unpack_task start\n");  
    mpu_status_t ret = MPU_OK;
    uint8_t data = 0;
    int16_t temp = 0;
    mpu_data_t mpu6050_data;

    for (;;)
    {
        // if (NULL != handler_instance.p_unpack_queue_handle)
        // {
            // ret = handler_instance.p_input_args->p_os->os_queue_get( handler_instance.p_unpack_queue_handle,
            //                                             &data,
            //                                             0xffffffff );
            if (MPU_OK == ret)
            {
                log_i("unpack_task: data = %d\n", data);
            }
            uint8_t *addr = circular_buf.pfget_rbuffer_addr(&circular_buf);
            log_i("unpack_task: addr = %p\n", addr);

            temp = (int16_t)(*(addr + 6) << 8 | *(addr + 7));
            mpu6050_data.temperature = 36.53 + temp/340.0;
        
            mpu6050_data.accel_x_raw = (int16_t)(*(addr + 0) << 8 | *(addr + 1));
            mpu6050_data.accel_y_raw = (int16_t)(*(addr + 2) << 8 | *(addr + 3));
            mpu6050_data.accel_z_raw = (int16_t)(*(addr + 4) << 8 | *(addr + 5));

            mpu6050_data.ax = mpu6050_data.accel_x_raw / 16384.0;
            mpu6050_data.ay = mpu6050_data.accel_y_raw / 16384.0;
            mpu6050_data.az = mpu6050_data.accel_z_raw / 14418.0;

            mpu6050_data.gyro_x_raw = (int16_t)(*(addr + 8) << 8 | *(addr + 9));
            mpu6050_data.gyro_y_raw = (int16_t)(*(addr + 10) << 8 | *(addr + 11));
            mpu6050_data.gyro_z_raw = (int16_t)(*(addr + 12) << 8 | *(addr + 13));

            mpu6050_data.gx = mpu6050_data.gyro_x_raw / 131.0;
            mpu6050_data.gy = mpu6050_data.gyro_y_raw / 131.0;
            mpu6050_data.gz = mpu6050_data.gyro_z_raw / 131.0;
            
            log_i("UnpackThread temp=%f", mpu6050_data.temperature);
            log_i("UnpackThread ax=%f", mpu6050_data.ax);
            log_i("UnpackThread ay=%f", mpu6050_data.ay);
            log_i("UnpackThread az=%f", mpu6050_data.az);
            log_i("UnpackThread gx=%f", mpu6050_data.gx);
            log_i("UnpackThread gy=%f", mpu6050_data.gy);
            log_i("UnpackThread gz=%f", mpu6050_data.gz);

            circular_buf.pfdata_readed(&circular_buf);
        // }
    }
}
#endif
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
