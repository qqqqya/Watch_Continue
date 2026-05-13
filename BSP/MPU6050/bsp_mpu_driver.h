/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_mpu_driver.h
 * 
 * @par dependencies 
 * - stdio.h
 * - stdint.h
 * - aht21_reg.h
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
 * @version V1.0 2024-10-18
 *
 * @note 1 tab == 4 spaces
 * 
 *****************************************************************************/
#ifndef __BSP_MPU_DRIVER_H__
#define __BSP_MPU_DRIVER_H__

#include "stdio.h"
#include "stdint.h"
#include "bsp_mpu6050_reg.h"
#include "mid_circle_buffer.h"


//******************************* Defines ***********************************//
#define OS_SUPPORTING
#define DEBUG_OUT(format, ...) log_i(format, ##__VA_ARGS__) /* Debug output  */
//elog调试输出打印

#define MPU_NOT_INITED        0            // Not init. flag 
#define MPU_IS_INITED         1
#define IIC_MEMADD_SIZE_8BIT 0x00000001U
#define TIME_OUT_MS          1000
#define MPU6050_DATA_PACKET_SIZE 14
extern circular_buffer_t circular_buf;

typedef enum
{
	MPU_OK             = 0,      /* Operation completed successfully         */
	MPU_ERROR          = 1,      /* General runtime error                    */
	MPU_ERRORTIMEOUT   = 2,      /* Operation timed out                      */
	MPU_ERRORRESOURCE  = 3,      /* Required resource is unavailable         */
	MPU_ERRORPARAMETER = 4,      /* Invalid parameter error                  */
	MPU_ERRORNOMEMORY  = 5,      /* Memory allocation failed                 */
	MPU_ERRORISR       = 6,      /* Not allowed in ISR context               */
	MPU_RESERVED       = 0xFF,   /* Reserved status                          */
} mpu_status_t;

/*根据iic的基本需求写函数指针    还有这里的排版也很舒服
    这里iic RW数据都是传的地址+长度+存入/读出 变量的指针  读写多个字节都可以了
*/
//******1.iic 实例结构体**********//

typedef struct{     //void * bus  iicbus init--包含引脚等信息
            //硬件iic指针
    void *hi2c;             /* hi2c pointer to a I2C_HandleTypeDef structure */
    int8_t        (*pf_iic_init       ) (void*);
    int8_t        (*pf_iic_deinit     ) (void*); //反初始化
    int8_t        (*pf_iic_mem_read   ) (void*,   
                                        uint8_t  dev_addr,
                                        uint8_t  mem_addr, 
                                        uint8_t   mem_size,/*设备 mem地址 长度*/
                                        uint8_t  *pdata,    /*数据 data大小*/
                                        uint8_t   size ,
                                        uint32_t   timeout   );//     
    int8_t        (*pf_iic_mem_write  ) (void*,   
                                        uint8_t  dev_addr,
                                        uint8_t  mem_addr, 
                                        uint8_t   mem_size,
                                        uint8_t  *pdata,
                                        uint8_t   size ,
                                        uint32_t   timeout   );                  
    int8_t        (*pf_iic_mem_read_dma ) (void*,   
                                        uint8_t  dev_addr,
                                        uint8_t  mem_addr, 
                                        uint8_t   mem_size,
                                        uint8_t  *pdata,
                                        uint8_t   size);//dma无需timeout？

    // int8_t         (*pf_enter_critical) (void);                            
    // int8_t         (*pf_exit_critical) (void);                            
}mpu_iic_driver_instance_t;
//******1.01 mpu中断引脚实例化 **********//
typedef struct{
    uint32_t    (*pf_INT_init) (void *);
    uint32_t    (*pf_INT_deinit) (void *);
}mpu_INT_interface_t;


//******1.1 buf实例结构体**********//
typedef struct{
    uint32_t    (*pf_buf_init) (void *);/**初始化 buf结构体内的信息 */
    uint32_t    (*pf_buf_deinit) (void *);///后面可以改成   cir head tail malloc什么的
    uint32_t    (*pf_buf_put) (void *);
    uint32_t    (*pf_buf_get) (void *);
}mpu_buf_interface_t;

//from core layer(hal库)
//******2.timebase 实例结构体**********//
typedef struct{
    uint32_t    (*pf_timebase_gettickms) (uint32_t *ptick);
}mpu_timebases_ms_t;

typedef struct
{
    void (*pf_delay_init) (void);               /* Delay init interface    */
    void (*pf_delay_us) (const uint32_t);       /* Delay us interface      */
    void (*pf_delay_ms) (const uint32_t);       /* Delay ms interface      */
} mpu_delay_interface_t;

//from OS layer

//******3. 让出CPU实例结构体**********//
#ifdef OS_SUPPORTING
typedef struct{///实际上就是vtaskdelay--阻塞状态  让出CPU 给其他任务执行
    void        (*rtos_yield) (const uint32_t );  //os delay ms
}mpu_yield_interface_t;

//******4. OS队列实例结构体**********//
typedef struct{
    mpu_status_t (*Queuecreate)(        uint32_t const item_num,
                                        uint32_t const size,
                                        void** Q_handler);//只有创建是要修改一级指针的
    mpu_status_t (*Queuedelete)(void * const Q_handler);

    mpu_status_t (*Queueput   )(        void * const Q_handler,
                                        void * const item,
                                        uint32_t timeout);
    mpu_status_t (*Queueput_put_isr)(void * const queue_handle,//中断中写队列
									void * const item, 
									long * const HigherPriorityTaskWoken);                                    
    mpu_status_t (*Queueget   )(        void * const Q_handler,
                                        void * msg,
                                        uint32_t timeout);
    mpu_status_t (*semaphore_create_mutex)(void **  mutex_handler);
    mpu_status_t (*semaphore_take_mutex)(void * const mutex_handler);
    mpu_status_t (*semaphore_give_mutex)(void * const mutex_handler);
    mpu_status_t (*semaphore_delete_mutex)(void * const mutex_handler);

    mpu_status_t (*semaphore_create_binary)(void **  binary_handler);
    /** @brief 这里往后就不太懂这个binary作用*/
    mpu_status_t (*semaphore_delete_binary) (void * const binary_handle);
    mpu_status_t (*semaphore_wait_binary)   (void * const binary_handle);
    mpu_status_t (*semaphore_signal_binary) (void * const binary_handle);
	mpu_status_t (*semaphore_signal_binary_isr) (void * const binary_handle,
                                                    long * const HigherPriorityTaskWoken);
    /** @brief 消息通知notify  通知ulValue值 eAction操作覆盖/ */
	mpu_status_t (*semaphore_signal_notify_isr) ( void * const notify_handle, 
                                                 uint32_t ulValue, 
                                                 uint32_t eAction, 
                                                 long * const HigherPriorityTaskWoken);
    mpu_status_t (*semaphore_wait_notify)   ( uint32_t ulBitsToClearOnEntry, 
                                                    uint32_t ulBitsToClearOnExit, 
                                                    uint32_t *pulNotificationValue, 
                                                    uint32_t timeout);


}mpu_os_interface_t;
#endif
typedef struct{///mpu6050的加速度陀螺仪数据结构体
    /* Raw accelerometer data from sensor */
    int16_t accel_x_raw;
    int16_t accel_y_raw;
    int16_t accel_z_raw;

    /* Processed accelerometer data in g units */
    double ax;
    double ay;
    double az;
    /*****加速度原始 处理 */

    /* Raw gyroscope data from sensor */
    int16_t gyro_x_raw;
    int16_t gyro_y_raw;
    int16_t gyro_z_raw;

    /* Processed gyroscope data in degrees/s */
    double gx;
    double gy;
    double gz;
    /*****陀螺仪原始 处理 */
/* Temperature reading in degrees Celsius */
    float temperature;

    /***kalman滤波之后的x y 信号 */
    /* Kalman filter processed angles */
    double kalman_angle_x;
    double kalman_angle_y;

}mpu_data_t;

/*  按照之前学的led桥接模式驱动的方法
    1.    首先应该写的是bsp_aht21_driver_t 内部的成员变量 根据需求写
            再声明instance 后不断填充bsp_aht21_driver_t成员
    2.    填充一些成员变量 定义数据类型如iic_driver_instance_t
            里面包含一些函数指针 这些函数指针指向实际的函数实现
        
        */
/*******mpuxxxx接口   实例结构体  **********/
typedef struct bsp_mpu_driver
{
    /* Core Layer  */
    mpu_iic_driver_instance_t   *p_iic_instance; //IIC实例指针
    mpu_INT_interface_t         *p_INT_instance; //mpu中断引脚实例指针
    mpu_buf_interface_t         *p_buf_instance; //buf实例指针
    mpu_timebases_ms_t          *p_timebase_ms; //timebase实例指针   
    mpu_delay_interface_t       *p_delay_interface; //delay接口
    /* OS Layer  */
#ifdef OS_SUPPORTING
    mpu_yield_interface_t       *p_yield_interface; //操作系统让出CPU接口
    mpu_os_interface_t          *p_os_interface; //os接口
    mpu_data_t                  *mpu_data; //mpu数据结构体实例
    void *  Q_handler; //队列句柄指针
    void *  mutex_handler; //互斥锁句柄指针
    void *  notify_handler; //消息通知句柄
    void *  binary_handler; //二值信号量句柄

    void (*pf_dma_completed_callback)(void);//dma传输完成回调函数
    void (*pf_int_interrupt_callback)(void);//中断回调函数

#endif
    //iic实例
    // void             *p_iic_instance; //  IIC实例指针
    mpu_status_t (*pf_instance)(    //实例化函数的指针
                                    void * const self,
                                    mpu_iic_driver_instance_t  * const p_iic_instance,
                                    mpu_timebases_ms_t  *const p_timebase_ms,
#ifdef OS_SUPPORTING
                                    mpu_yield_interface_t   * const p_yield_interface,
                                    mpu_os_interface_t * const p_os_interface
#endif
                                    ); 
    mpu_status_t (*pf_init      )(void * const self);         //初始化函数指针
    mpu_status_t (*pf_deinit    )(void * const self);       //反初始化
    mpu_status_t (*pf_sleep)               (void *);
    mpu_status_t (*pf_wakeup)              (void *);
    mpu_status_t (*pf_set_gyro_fsr)        (void *, uint8_t);
    mpu_status_t (*pf_set_accel_fsr)       (void *, uint8_t);
    mpu_status_t (*pf_get_temperature)     (void *, mpu_data_t *);
    mpu_status_t (*pf_get_accel)           (void *, mpu_data_t *);
    mpu_status_t (*pf_get_gyro)            (void *, mpu_data_t *);
    mpu_status_t (*pf_get_all_data)        (void *, mpu_data_t *);
    mpu_status_t (*pf_get_interrupt_status_reg)(void *, uint8_t *);
    mpu_status_t (*pf_read_fifo_packet)    (void *p_mpu_driver, mpu_data_t *p_data);
    mpu_status_t (*pf_read_fifo_isr_occur) (void *p_mpu_driver, mpu_data_t *p_data);
    mpu_status_t (*pf_set_lpf)             (void *, uint8_t);
    mpu_status_t (*pf_set_rate)            (void *, uint8_t);
    mpu_status_t (*pf_set_interrupt_enable)(void *, uint8_t);
    mpu_status_t (*pf_set_motion_threshold)(void *, uint8_t);
    mpu_status_t (*pf_set_INT_level)       (void *, uint8_t);
    mpu_status_t (*pf_set_user_ctrl)       (void *, uint8_t);
    mpu_status_t (*pf_set_pwr_mgmt1_reg)   (void *, uint8_t);
    mpu_status_t (*pf_set_pwr_mgmt2_reg)   (void *, uint8_t);
    mpu_status_t (*pf_set_fifo_en_reg)     (void *, uint8_t);    
} bsp_mpu_driver_t;

//******************************* Functions ***********************************//
mpu_status_t mpu_driver_instance(
                                    bsp_mpu_driver_t * const p_mpu_driver,
                                    mpu_iic_driver_instance_t  * const p_iic_instance,
                                    mpu_delay_interface_t       * const p_delay_interface, //delay接口
                                    mpu_timebases_ms_t  *const p_timebase_ms,
                void (*callback_register)    (void (*callback)(void *, void *)),
                void (*callback_register_dma)(void (*callback)(void *, void *)),
#ifdef OS_SUPPORTING
                                    mpu_yield_interface_t   * const p_yield_interface,
                                    mpu_os_interface_t * const p_os_interface, //os接口
                    void *  Q_handler, //队列句柄指针
                    void *  mutex_handler, //互斥锁句柄指针
                    void *  notify_handler, //消息通知句柄
                    void *  binary_handler //二值信号量句柄
#endif
                                    );
uint32_t mpu_flag_read();
void mpu_flag_set(uint8_t flag);
#endif//end of file
