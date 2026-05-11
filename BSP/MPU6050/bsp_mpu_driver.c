
/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_aht21_driver.c
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
 * @version V1.0 2024-10-18
 *
 * @note 1 tab == 4 spaces
 * 
 *****************************************************************************/
#include "iic_hal.h"
#include "bsp_mpu_driver.h"

#include "bsp_mpu6050_reg_bit.h"//移位bit定义
#include "bsp_mpu6050_reg.h"
#include "elog.h"

/******************************** Defines ************************************/
#define MPU_WRITE_REG(p_mpu_driver, reg, p_data, len)\
                        p_mpu_driver->p_iic_instance->pf_iic_mem_write(\
                        p_mpu_driver->p_iic_instance->hi2c,\
                        (MPU_ADDR << 1) | 0,\
                        reg,\
                        IIC_MEMADD_SIZE_8BIT,\
                        p_data,\
                        len,\
                        TIME_OUT_MS)

#define MPU_READ_REG(p_mpu_driver, reg, p_data, len)\
                        p_mpu_driver->p_iic_instance->pf_iic_mem_read(\
                        p_mpu_driver->p_iic_instance->hi2c,\
                        (MPU_ADDR << 1) | 1,\
                        reg,\
                        IIC_MEMADD_SIZE_8BIT,\
                        p_data, \
                        len, \
                        TIME_OUT_MS)

/******************************** Declares ************************************/
int8_t g_mpu_inited =   MPU_NOT_INITED;  //全局变量 记录是否实例化过了
int8_t g_mpu_dev_id =   0;                  // 记录设备ID
static uint32_t g_is_dma_readed = 0;

static double g_accel_scale = 16384.0;
static double g_gyro_scale = 131.0;


static mpu_status_t mpu_get_temperature(bsp_mpu_driver_t *p_mpu_driver, mpu_data_t *p_data)
{
#ifdef DEBUG
    DEBUG_OUT("mpu_get_temperature\r\n");
#endif
    mpu_status_t ret = MPU_OK;
    uint8_t data[2] = {0};
    int16_t temp = 0;

    ret = MPU_READ_REG(p_mpu_driver, MPU_TEMP_OUTH_REG, data, 2);
    if (ret != MPU_OK)
    {
#ifdef DEBUG
        DEBUG_OUT("mpu_get_temperature read temperature error\r\n");
#endif
        return ret;
    }
    temp = (int16_t)(data[0] << 8 | data[1]);
    p_data->temperature = (float)(temp / 340.0 + 36.53);

    return ret;
}

static mpu_status_t mpu_get_gyro(bsp_mpu_driver_t *p_mpu_driver, mpu_data_t *p_data)
{
#ifdef DEBUG
    DEBUG_OUT("mpu_get_gyro\r\n");
#endif
    mpu_status_t ret = MPU_OK;
	uint8_t data[6] = {0};
	
    ret = MPU_READ_REG(p_mpu_driver, MPU_GYRO_XOUTH_REG, data, 6);
    if (ret != MPU_OK)
    {
#ifdef DEBUG
        DEBUG_OUT("mpu_get_gyro read gyro error\r\n");
#endif
        return ret;
    }
    p_data->gyro_x_raw = (int16_t)(data[0] << 8 | data[1]);
    p_data->gyro_y_raw = (int16_t)(data[2] << 8 | data[3]);
    p_data->gyro_z_raw = (int16_t)(data[4] << 8 | data[5]);

    p_data->gx = (double)(p_data->gyro_x_raw / g_gyro_scale);
    p_data->gy = (double)(p_data->gyro_y_raw / g_gyro_scale);
    p_data->gz = (double)(p_data->gyro_z_raw / g_gyro_scale);

    return ret;
}

/**
 * @brief get accel
 * 
 * @param[in] p_mpu_driver: pointer to a mpu6050 driver structure
 * @param[out] p_data: pointer to a mpu6050 data structure
 * 
 * @return mpu_status_t
*/
static mpu_status_t mpu_get_accel(bsp_mpu_driver_t *p_mpu_driver, mpu_data_t *p_data)
{
#ifdef DEBUG
    DEBUG_OUT("mpu_get_accel\r\n");
#endif
    mpu_status_t ret = MPU_OK;
    uint8_t data[6] = {0};

    ret = MPU_READ_REG(p_mpu_driver, MPU_ACCEL_XOUTH_REG, data, 6);
    if (ret != MPU_OK)
    {
#ifdef DEBUG
        DEBUG_OUT("mpu_get_accel read error\r\n");
#endif
        return ret;
    }
    p_data->accel_x_raw = (int16_t)(data[0] << 8 | data[1]);
    p_data->accel_y_raw = (int16_t)(data[2] << 8 | data[3]);
    p_data->accel_z_raw = (int16_t)(data[4] << 8 | data[5]);

    p_data->ax = (double)(p_data->accel_x_raw / g_accel_scale);
    p_data->ay = (double)(p_data->accel_y_raw / g_accel_scale);
    p_data->az = (double)(p_data->accel_z_raw / g_accel_scale);

    return ret;
}

/**
 * @brief get all data
 * 
 * @param[in] p_mpu_driver: pointer to a mpu6050 driver structure
 * @param[out] p_data: pointer to a mpu6050 data structure
 * 
 * @return mpu_status_t
*/
static mpu_status_t mpu_get_all_data(bsp_mpu_driver_t *p_mpu_driver, mpu_data_t *p_data)
{
#ifdef DEBUG
    DEBUG_OUT("mpu_get_all_data\r\n");
#endif
    mpu_status_t ret = MPU_OK;
    uint8_t data[14] = {0};
    int16_t temp = 0;

    ret = MPU_READ_REG(p_mpu_driver, MPU_ACCEL_XOUTH_REG, data, 14);
	if (ret != MPU_OK)
    {
#ifdef DEBUG
        DEBUG_OUT("mpu_get_all_data read error\r\n");
#endif
        return ret;
    }
	
    p_data->accel_x_raw = (int16_t)(data[0] << 8 | data[1]);
    p_data->accel_y_raw = (int16_t)(data[2] << 8 | data[3]);
    p_data->accel_z_raw = (int16_t)(data[4] << 8 | data[5]);
    
    p_data->ax = (double)(p_data->accel_x_raw / g_accel_scale);
    p_data->ay = (double)(p_data->accel_y_raw / g_accel_scale);
    p_data->az = (double)(p_data->accel_z_raw / g_accel_scale);

	temp = (int16_t)(data[6] << 8 | data[7]);
    p_data->temperature = (float)(temp / 340.0 + 36.53);
									
    p_data->gyro_x_raw = (int16_t)(data[8] << 8 | data[9]);
    p_data->gyro_y_raw = (int16_t)(data[10] << 8 | data[11]);
    p_data->gyro_z_raw = (int16_t)(data[12] << 8 | data[13]);

    p_data->gx = (double)(p_data->gyro_x_raw / g_gyro_scale);
    p_data->gy = (double)(p_data->gyro_y_raw / g_gyro_scale);
    p_data->gz = (double)(p_data->gyro_z_raw / g_gyro_scale);
    
    return ret;
}


static mpu_status_t mpu_sleep(bsp_mpu_driver_t *p_mpu_driver)
{
    return MPU_OK;
}

static mpu_status_t mpu_wakeup(bsp_mpu_driver_t *p_mpu_driver)
{
    mpu_status_t ret = MPU_OK;
   
    ret = MPU_WRITE_REG(p_mpu_driver, MPU_PWR_MGMT1_REG, (uint8_t[]){SLEEP_BIT(0)}, 1); 
    return ret;
}
/**
 * @brief 初始化MPU6050 FIFO
 * @param p_mpu_driver MPU驱动结构体指针
 * @return mpu_status_t 状态码
 */
static mpu_status_t mpu_fifo_init(bsp_mpu_driver_t * const p_mpu_driver){
    mpu_status_t ret=MPU_OK;
    DEBUG_OUT("mpu_fifo_init start\r\n");

    //FIFO Reset--FIFO_CTRL
    MPU_WRITE_REG(p_mpu_driver, MPU_USER_CTRL_REG,(uint8_t[]){FIFO_RESET_BIT(1)}, 1);
    // Wait for reset to complete
#ifdef OS_SUPPORTING
    p_mpu_driver->p_yield_interface->rtos_yield(10);
#else
    p_mpu_driver->p_delay_interface->pf_delay_ms(10);
#endif
    //配置FIFO使能--FIFO_CTRL
    MPU_WRITE_REG(p_mpu_driver, MPU_FIFO_EN_REG,(uint8_t[]){ACCEL_FIFO_EN_BIT(1) |  // Enable accelerometer first
                                                                XG_FIFO_EN_BIT(1)    |  // Then gyroscope
                                                                YG_FIFO_EN_BIT(1)    |
                                                                ZG_FIFO_EN_BIT(1)}, 1);
    // Enable FIFO last
    MPU_WRITE_REG(p_mpu_driver, MPU_USER_CTRL_REG,(uint8_t[]){FIFO_EN_BIT(1)}, 1);

    //配置中断使能--INT_ENABLE
    MPU_WRITE_REG(p_mpu_driver, MPU_INTBP_CFG_REG,(uint8_t[]){INT_RD_CLEAR_BIT(1) | INT_LEVEL_BIT(1)}, 1);

    //溢出中断 (FIFO Overflow Interrupt)
    MPU_WRITE_REG(p_mpu_driver, MPU_INT_EN_REG,(uint8_t[]){ FIFO_OVERFLOW_EN_BIT(1)}, 1);
    DEBUG_OUT("mpu_fifo_init ok\r\n");
    return ret;
}
/**
 * @brief 设置陀螺仪量程
 * 量程被设置为 **±2000 °/s**（度每秒，dps）    **最大量程/最低灵敏度**模式。
 * 原始寄存器数据每变化 16.4，代表物体正在以 1度/秒 的速度旋转。
 * @param p_mpu_driver MPU驱动结构体指针
 * @param fsr 陀螺仪量程选择位
 * @return mpu_status_t 状态码
 */
static mpu_status_t mpu_set_gyro_fsr(bsp_mpu_driver_t *p_mpu_driver, uint8_t fsr)
{
    mpu_status_t ret = MPU_OK;

    ret = MPU_WRITE_REG(p_mpu_driver, MPU_GYRO_CFG_REG, &fsr, 1);
    if(ret != MPU_OK)
    {
#ifdef DEBUG
        DEBUG_OUT("mpu_set_gyro_fsr write MPU_GYRO_CFG_REG error\r\n");
#endif
        return ret;
    }

    switch(fsr)
    {
        case 0x00: g_gyro_scale = 131.0; break;
        case 0x01: g_gyro_scale = 65.5; break;
        case 0x02: g_gyro_scale = 32.8; break;
        case 0x03: g_gyro_scale = 16.4; break;
        default:   g_gyro_scale = 131.0;
    }

    return ret;
}
/**
 * @brief 设置加速度计量程
 * **最高精度/最高灵敏度**模式。原始寄存器数据每变化 16384，代表受力变化了 1g
 * @param p_mpu_driver MPU驱动结构体指针
 * @param fsr 加速度计量程选择位
 * @return mpu_status_t 状态码
 */
static mpu_status_t mpu_set_accel_fsr(bsp_mpu_driver_t *p_mpu_driver, uint8_t fsr)
{
    mpu_status_t ret = MPU_OK;

    ret = MPU_WRITE_REG(p_mpu_driver, MPU_ACCEL_CFG_REG, &fsr, 1);
    if(ret != MPU_OK)
    {
#ifdef DEBUG
        DEBUG_OUT("mpu_set_accel_fsr write MPU_ACCEL_CFG_REG error\r\n");
#endif
        return ret;
    }

    switch(fsr)
    {
        case 0x00: g_accel_scale = 16384.0; break;
        case 0x01: g_accel_scale = 8192.0; break;
        case 0x02: g_accel_scale = 4096.0; break;
        case 0x03: g_accel_scale = 2048.0; break;
        default:   g_accel_scale = 16384.0;
    }

    return ret;
}
/**
 * @brief 初始化MPU6050寄存器 iic +register config
 * @param p_mpu_driver MPU驱动结构体指针
 * @return mpu_status_t 状态码
 */
static mpu_status_t mpu_init_reg_iic(bsp_mpu_driver_t * const p_mpu_driver){
    mpu_status_t ret=MPU_OK;
    DEBUG_OUT("mpu_init_reg_iic start\r\n");
    
    //Initialize I2C
    p_mpu_driver->p_iic_instance->pf_iic_init(NULL);
    //Device Reset--PWR   (uint8_t[]){0x80}  &DEVICE_RESET_BIT(1)
    MPU_WRITE_REG(p_mpu_driver, MPU_PWR_MGMT1_REG,(uint8_t[]){DEVICE_RESET_BIT(1)}, 1);
    /////Delay 100ms for reset to complete
#ifdef OS_SUPPORTING
    p_mpu_driver->p_yield_interface->rtos_yield(100);
#else
    p_mpu_driver->p_delay_interface->pf_delay_ms(100);
#endif
    //唤醒传感器 (Wake Up)--PWR
    MPU_WRITE_REG(p_mpu_driver, MPU_PWR_MGMT1_REG,(uint8_t[]){SLEEP_BIT(0)}, 1);

    //配置陀螺仪量程--fsr  Gyroscope for ±2000°/s (0x03)   case 0x03: gyro_scale = 16.4;
    ret = mpu_set_gyro_fsr(p_mpu_driver, FS_SEL_BIT(0X03));//MPU_GYRO_CFG_REG  0x18
    //配置加速度计量程  Accelerometer for ±2g (0x00 << 3)   case 0x03{<<3}: accel_scale = 2048.0;
    ret = mpu_set_accel_fsr(p_mpu_driver, AFS_SEL_BIT(0X00));
       
    
    //配置采样率    Configure sample rate to 50Hz
    /*    // Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
    // where Gyroscope Output Rate = 8kHz when the DLPF is disabled (DLPF_CFG = 0 or 7), and 1kHz 
    // when the DLPF is enabled
    // 50Hz = 1000 / (1 + 19), so SMPLRT_DIV = 19
    // 0x04 : 200hz*/
    MPU_WRITE_REG(p_mpu_driver, MPU_SAMPLE_RATE_REG,(uint8_t[]){SMPLRT_DIV_BIT(0x13)}, 1);//MPU_SAMPLE_RATE_REG

    //配置低通滤波器Configure DLPF, This value should be half of the sampling rate
    // when the sample rate is 50Hz, the DLPF should be 25Hz
    MPU_WRITE_REG(p_mpu_driver, MPU_CFG_REG,(uint8_t[]){DLPF_CFG_BIT(0x04)}, 1);//MPU_CFG_REG
    //配置中断
    MPU_WRITE_REG(p_mpu_driver, MPU_INT_EN_REG,(uint8_t[]){DATA_RDY_EN_BIT(1)}, 1);//MPU_INT_EN_REG    
    //验证设备ID
    uint8_t id_recv=0;
    MPU_READ_REG(p_mpu_driver, MPU_DEVICE_ID_REG, &id_recv, 1);
    if(id_recv != MPU_ID)
    {
        return MPU_ERROR;
    }
        DEBUG_OUT("mpu_init_reg_iic verify device id :%02x\r\n", id_recv);
                    // MPU_READ_REG(p_mpu_driver, MPU_GYRO_CFG_REG, &id_recv, 1);
                    //     DEBUG_OUT("mpu_init_reg_iic verify gyro cfg :%02x\r\n", id_recv);
    //设置时钟源
    MPU_WRITE_REG(p_mpu_driver, MPU_PWR_MGMT1_REG,(uint8_t[]){CLKSEL_BIT(0x01) }, 1);//MPU_PWR_MGMT1_REG
    //解除各轴待机
    MPU_WRITE_REG(p_mpu_driver, MPU_PWR_MGMT2_REG,(uint8_t[]){   LP_WAKE_CTRL_BIT(0) | 
                                                                    STBY_XA_BIT(0)      | 
                                                                    STBY_YA_BIT(0)      | 
                                                                    STBY_ZA_BIT(0)      | 
                                                                    STBY_XG_BIT(0)      | 
                                                                    STBY_YG_BIT(0)      | 
                                                                    STBY_ZG_BIT(0)}, 1);//MPU_PWR_MGMT2_REG

    DEBUG_OUT("mpu_init_reg_iic ok\r\n");
    return ret;
}
mpu_status_t mpu_driver_init(bsp_mpu_driver_t * const p_mpu_driver){
    mpu_status_t ret=MPU_OK;
    DEBUG_OUT("mpu_driver_init start\r\n");
    if(NULL ==  p_mpu_driver)
    {
        return MPU_ERRORPARAMETER;
    }
    if(g_mpu_inited == MPU_IS_INITED)
    {
        return MPU_ERRORRESOURCE;  //已经初始化过了
    }
    /*********1. init reg iic ****************/
    ret = mpu_init_reg_iic(p_mpu_driver);
    /*********2. init fifo mode****************/
    ret = mpu_fifo_init(p_mpu_driver);
    DEBUG_OUT("mpu_driver_init all ok\r\n");


    g_mpu_inited = MPU_IS_INITED;

    return ret;
}
mpu_status_t mpu_driver_deinit(bsp_mpu_driver_t * const p_mpu_driver){
    mpu_status_t ret=MPU_OK;
    if(NULL ==  p_mpu_driver)
    {
        return MPU_ERRORPARAMETER;
    }
    if(g_mpu_inited != MPU_IS_INITED)
    {
        return MPU_ERRORPARAMETER;
    }
    g_mpu_inited = MPU_NOT_INITED;
    return ret;
}

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
                                    ){
    mpu_status_t ret=MPU_OK;
    if( NULL==p_mpu_driver      ||
        NULL==p_iic_instance    || 
        NULL==p_delay_interface || 
        NULL==p_timebase_ms
#ifdef OS_SUPPORTING
        || NULL==p_yield_interface 
        || NULL==p_os_interface
#endif
)
    {
        return MPU_ERRORPARAMETER;
    }
    DEBUG_OUT("mpu_driver_instance start\r\n");
    // /*********0.0. callback  is available ****************/
    // if( NULL==callback_register      ||
    //     NULL==callback_register_dma    
    // )
    // {
    //     return MPU_ERRORPARAMETER;
    // }
    /*********0.1. iic instance is available *************/
    p_mpu_driver->p_iic_instance=p_iic_instance;///挂载iic实例
    if( NULL==p_iic_instance->pf_iic_init      ||
        NULL==p_iic_instance->pf_iic_deinit    || 
        NULL==p_iic_instance->pf_iic_mem_read    || 
        NULL==p_iic_instance->pf_iic_mem_write    || 
        NULL==p_iic_instance->pf_iic_mem_read_dma    
        ){  return MPU_ERRORPARAMETER;  }
    /*********0.2. os interface is available *************/
    p_mpu_driver->p_os_interface=p_os_interface;///挂载os接口
    if (NULL == p_mpu_driver->p_os_interface->Queuecreate                   ||
        NULL == p_mpu_driver->p_os_interface->Queuedelete                   ||
        NULL == p_mpu_driver->p_os_interface->Queueget                      ||
		NULL == p_mpu_driver->p_os_interface->Queueput                      ||
        NULL == p_mpu_driver->p_os_interface->Queueput_put_isr              ||

        NULL == p_mpu_driver->p_os_interface->semaphore_create_binary       ||
        NULL == p_mpu_driver->p_os_interface->semaphore_delete_binary       ||
        NULL == p_mpu_driver->p_os_interface->semaphore_signal_binary       ||
        NULL == p_mpu_driver->p_os_interface->semaphore_signal_binary_isr   ||
        NULL == p_mpu_driver->p_os_interface->semaphore_create_mutex        ||
        NULL == p_mpu_driver->p_os_interface->semaphore_delete_mutex        ||
        NULL == p_mpu_driver->p_os_interface->semaphore_give_mutex          )
    {
        return MPU_ERRORPARAMETER;
    }
    /*********0.3. delay interface is available **********/
    p_mpu_driver->p_delay_interface=p_delay_interface;///挂载delay接口
    if (NULL == p_mpu_driver->p_delay_interface->pf_delay_init ||
        NULL == p_mpu_driver->p_delay_interface->pf_delay_us   ||
        NULL == p_mpu_driver->p_delay_interface->pf_delay_ms)
    {
        return MPU_ERRORPARAMETER;
    }
    /*******0.4. timebase_ms interface is available ******/
    p_mpu_driver->p_timebase_ms=p_timebase_ms;///挂载timebase_ms接口
    if (NULL == p_timebase_ms->pf_timebase_gettickms)
    {
        return MPU_ERRORPARAMETER;
    }
    /*********0.5. yield interface is available **********/
     p_mpu_driver->p_yield_interface=p_yield_interface;///挂载yield接口
   if (NULL == p_yield_interface->rtos_yield)
    {
        return MPU_ERRORPARAMETER;
    }
    ////////////总共七个interface   除了INT buf接口
    /******1. 挂载bsp_mpu_driver_t内部的函数指针 ******/

    p_mpu_driver->pf_init                    =mpu_driver_init;
    p_mpu_driver->pf_deinit                  =mpu_driver_deinit;
    p_mpu_driver->pf_sleep                   =mpu_sleep;
    p_mpu_driver->pf_wakeup                  =mpu_wakeup;
    p_mpu_driver->pf_set_gyro_fsr            =mpu_set_gyro_fsr;
    p_mpu_driver->pf_set_accel_fsr           =mpu_set_accel_fsr;
    p_mpu_driver->pf_get_temperature         =mpu_get_temperature;
    p_mpu_driver->pf_get_accel               =mpu_get_accel;
    p_mpu_driver->pf_get_gyro                =mpu_get_gyro;
    p_mpu_driver->pf_get_all_data            =mpu_get_all_data;
    // p_mpu_driver->pf_get_interrupt_status_reg
    // p_mpu_driver->pf_read_fifo_packet   
    // p_mpu_driver->pf_read_fifo_isr_occur
    /**都是些设置寄存器的函数-- */
    // p_mpu_driver->pf_set_lpf                 =
    // p_mpu_driver->pf_set_rate                =   
    // p_mpu_driver->pf_set_interrupt_enable    =
    // p_mpu_driver->pf_set_motion_threshold
    // p_mpu_driver->pf_set_INT_level    
    // p_mpu_driver->pf_set_user_ctrl    
    // p_mpu_driver->pf_set_pwr_mgmt1_reg 
    // p_mpu_driver->pf_set_pwr_mgmt2_reg  
    // p_mpu_driver->pf_set_fifo_en_reg    

    /******2. bsp_mpu_driver_t内init ******/
    ret=mpu_driver_init(p_mpu_driver);
    if(ret!=MPU_OK)
    {
        return ret;
    }

    /******3. 回调函数挂载 ******/

    return ret;
}
