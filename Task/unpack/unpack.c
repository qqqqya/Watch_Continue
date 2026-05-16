/*
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file manage_jmp.c
 * 
 * @par dependencies 
 * - stdio.h
 * - stdint.h
 * - "usart.h"
 * - elog.h

 * @author yan | R&D Dept. | EternalChip ?????
 *
 * @brief Provides HAL APIs for LED control and operations.
 * 
 * Usage:
 * Call functions directly.
 * 
 * @version V1.0 2026年5月16日
 *
 * @note 1 tab == 4 spaces
 * 
 *****************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "unpack.h"
#include "bsp_mpu_handler.h"
#include "elog.h"
extern circular_buffer_t circular_buf;
extern bsp_mpu_handler_t handler_instance;//解包队列
void unpack_thread_func(void)
{
    log_i("unpack_thread_func");
    mpu_status_t ret = 0;
    mpu_data_t mpu6050_data;///通过都地址获取到的数据，存储在这个结构体中

    
    uint32_t recv_unpack_event=0;//数据新鲜度--这里方便看数据  实际上高速设备应当一直读
    uint32_t tim=0;
	for(;;)   
	{	   
    if (NULL != handler_instance.p_unpack_queue_handle)
        {
            ret = handler_instance.p_input_arg->p_os_interface->Queueget(handler_instance.p_unpack_queue_handle,
                                                                     &recv_unpack_event, 
                                                                     0xffffffff);//等待解包事件--无限等待
            if(ret == HANDLER_MPU_OK)
            {
              log_i("UnpackThread recv_unpack_event=%d", recv_unpack_event);
            }
            /**获取当前tick */
            handler_instance.p_mpu_driver->p_timebase_ms->pf_timebase_gettickms(&tim);
              log_i("UnpackThread tim=%d", tim);
            /** @brief just for test 数据新鲜度判断 8000ms 8s */
            if((tim-handler_instance.last_tick) > recv_unpack_event||
              tim < recv_unpack_event)
            {
              uint8_t *addr = circular_buf.pfget_rbuffer_addr(&circular_buf);
              log_i("unpack_thread_func: addr = %p\n", addr);
                // temp = (int16_t)(*(addr + 6) << 8 | *(addr + 7));
                // mpu6050_data.temperature = 36.53 + temp/340.0;
            
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



                handler_instance.last_tick=tim;//更新上次解包时间戳
            }

            }

    }
}

