
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
#include "bsp_aht21_driver.h"
#include "elog.h"

/******************************** Defines ************************************/
#define AHT21_MEASUREMENT_TIME_MS 80            // Measurement time [ms]
#define AHT21_NOT_INITED     0            // Not init. flag 
#define AHT21_INITED         1            // Not init. flag 
#define AHT21_ID                  0x18           // AHT21 ID--STATUS寄存器相与0x18，结果为0x18，说明是AHT21传感器

#define AHT21_CRC8_POLYNOMIAL     0x31           // CRC-8 polynomial
#define AHT21_CRC8_INITIAL        0xFF           // CRC-8 initial value

#define AHT21_IIC_inst      p_aht21_instance->p_iic_instance
/******************************** Declares ************************************/
int8_t g_inited =   AHT21_NOT_INITED;  //全局变量 记录是否实例化过了
int8_t g_dev_id =   0;                  // 记录设备ID
#include "iic_hal.h"
extern iic_bus_t iic_instance;
static aht_status_t __read_id(bsp_aht21_driver_t *  p_aht21_instance){
    
    aht_status_t ret=AHT_OK;
    uint8_t recv=0;
    if(AHT21_INITED==g_inited)
    {
        return AHT_ERRORRESOURCE;  //已经初始化过了
    }

    AHT21_IIC_inst->pf_iic_start(NULL);  
    AHT21_IIC_inst->pf_iic_sendbytes(NULL,AHT21_READ_DATA_REG);//send 0x71
    if(AHT_OK  != AHT21_IIC_inst->pf_iic_waitack(NULL))       return AHT_ERROR;  //等待ack失败
	AHT21_IIC_inst->pf_iic_recevbytes(NULL,&recv);  //读取device id 存入全局变量
	// recv=IICReceiveByte(&iic_instance);
//     p_aht21_instance->p_iic_instance->pf_iic_send_notack(NULL);//IICSendNotAck(&AHT_bus);
//    p_aht21_instance->p_iic_instance->pf_iic_stop(NULL);
    if((recv & AHT21_ID) != AHT21_ID)  //判断device id是否正确
    {   //检查AHT21传感器 id
        return AHT_ERROR;  //device id错误
    }
    // log_d("------------aht21 device id read: 0x%02X", recv);
    g_dev_id = AHT21_ID;
    return ret;
}

static aht_status_t aht21_read_id(bsp_aht21_driver_t *  p_aht21_instance){

    if(g_inited!=AHT21_INITED)  
    {
        return AHT_ERRORRESOURCE;  //未初始化化
    }
    // log_d("aht21_read_id");
    // log_d("aht21_read_id_ing");
    return g_dev_id;    //__read_id函数中会把读取到的device id存入全局变量g_dev_id中  这里直接返回就好了
}

static aht_status_t aht21_init( bsp_aht21_driver_t *  p_aht21_instance){
    
    aht_status_t ret=AHT_OK;
    if(AHT21_INITED == g_inited)  
    {
        return AHT_ERRORRESOURCE;  //已经初始化过了
    }
    if (NULL == p_aht21_instance) {
        return AHT_ERRORPARAMETER;
    }
    if (NULL == AHT21_IIC_inst->pf_iic_init ||
        NULL == AHT21_IIC_inst                    ) {
        log_d("IIC instance is NULL");
    }
    log_d("aht21 init start");
   p_aht21_instance->p_yield_interface->rtos_yield(200);       //等待200ms 让传感器上电稳定
	
    AHT21_IIC_inst->pf_iic_init(NULL);  //调用iic实例的初始化函数
    // delay_ms(40);
    //
    ret=__read_id(p_aht21_instance); 
    log_d("aht21 IIC instance inited");
    if(ret != AHT_OK)
    {
        log_d("aht21 read id failed");
        return AHT_ERRORRESOURCE;
    }
    g_inited=AHT21_INITED;  //标记已经初始化过了
    log_d("aht21 device id: 0x%02X\r\n", aht21_read_id(p_aht21_instance));
    
    return AHT_OK;
}
static aht_status_t aht21_deinit( bsp_aht21_driver_t *  p_aht21_instance){
    g_inited=AHT21_NOT_INITED;  
    g_dev_id=0;                  
    return AHT_OK;

}
float g_temp = 0;
// float g_temp = 0.0f;
static aht_status_t aht21_read_humi(bsp_aht21_driver_t *  p_aht21_instance, float * const humi){
    if(AHT21_NOT_INITED == g_inited)  
    {
        return AHT_ERRORRESOURCE;  //未初始化化
    }
    log_d("aht21_read_humi");
    uint8_t ret=AHT_OK;
    uint8_t aht21_cmd[]={AHT21_MEASURE_CMD,AHT21_MEASURE_CMD_PARAMS1,AHT21_MEASURE_CMD_PARAMS2};
    uint8_t recv[7]={0};
    int i=0;

    AHT21_IIC_inst->pf_iic_start(NULL);
    AHT21_IIC_inst->pf_iic_sendbytes(NULL,AHT21_WRITE_DATA_REG);  //发送写数据reg 0x70
    AHT21_IIC_inst->pf_iic_waitack(NULL);
    for(i=0;i<sizeof(aht21_cmd);i++)
    {
        AHT21_IIC_inst->pf_iic_sendbytes(NULL,aht21_cmd[i]);  //发送测量指令
        AHT21_IIC_inst->pf_iic_waitack(NULL);  //等待ack失败
    }
    AHT21_IIC_inst->pf_iic_stop(NULL);        //----------end of command transmission

    p_aht21_instance->p_yield_interface->rtos_yield(AHT21_MEASUREMENT_TIME_MS);  //等待测量完成80ms
        /******如果要读状态7个全部读出
         *  但是其实也不用  for里面也读了
         */
    AHT21_IIC_inst->pf_iic_start(NULL);       //-----------start of data reception
    AHT21_IIC_inst->pf_iic_sendbytes(NULL,AHT21_READ_DATA_REG);  //发送读数据reg 0x71
    //这里还得加 waitack   -------------------------元凶在这里********************************************
    AHT21_IIC_inst->pf_iic_waitack(NULL);
    
    for(i=0;i<sizeof(recv);i++)
    {
        AHT21_IIC_inst->pf_iic_recevbytes(NULL,&recv[i]);  //读取测量结果
        if(i==0)        //第一个字节是状态寄存器 需要检查是否测量完成
        {
            if((recv[0] & 0x80) != 0)  //检查测量完成标志位
            {
                /**加iic  notack+ stop */
                return AHT_ERROR;  //测量未完成
            }
            /**************************** */
            // xxxxxxxxxxxxxxxx其实这个都无所谓xxxxxxxxxxxx p_aht21_instance->p_yield_interface->rtos_yield(80);  //测量完成后等待80ms
            AHT21_IIC_inst->pf_iic_send_ack(NULL);  //空闲确认就  发送ack
            
        }
        else if(i==sizeof(recv)-1)  //最后一个CRC 字节发送不ack
        {
            AHT21_IIC_inst->pf_iic_send_notack(NULL);  
        }
        else        //五个数据字节
        {
            AHT21_IIC_inst->pf_iic_send_ack(NULL);  //发送ack
        }
    }
    AHT21_IIC_inst->pf_iic_stop(NULL);        //end of data reception

    //解析测量结果
    uint32_t humi_data=     ( (uint32_t)recv[1]        <<  (12))   | 
                            ( (uint32_t)recv[2]        <<  (4 ))   |   
                            ( (recv[3]&0xf0) >>  (4 ));  //湿度数据 20bit    (uint32_t)

    uint32_t temp_data=     ((uint32_t)( recv[3]&0x0f) <<  (16))   | 
                            ((uint32_t)recv[4]        <<  (8 ))   |   
                            (recv[5]             );  //温度数据 20bit
    // *humi = (humi_data * 1000) >> 20;  //湿度百分比 *10  
    // *humi /= 10;                        // 保留一位小数 /10
    // g_temp = ((temp_data * 2000) >> 20) - 500;  
    // g_temp /= 10;
                        /*保留全部精度 消耗资源大*/
    *humi = ((float)humi_data * 100.0f) / 1048576.0f;  // 直接使用浮点数计算，保留全部精度  
    g_temp = ((float)temp_data * 200.0f) / 1048576.0f - 50.0f;  // 利用浮点计算避免小数值时uint32下溢
    // log_d("humi: %f", *humi );
    return AHT_OK;
}

static aht_status_t aht21_read_temp(bsp_aht21_driver_t *  p_aht21_instance, float * const temp){
    if(AHT21_NOT_INITED == g_inited)  
    {
        return AHT_ERRORRESOURCE;  //未初始化化
    }
    // 先humi 在 temp temp直接是gval
    // log_d("aht21_read_temp");
    *temp = (float)g_temp;
    // log_d("driver temp: %f", g_temp);
    return AHT_OK;
}

#if 1   //暂时不实现sleep和wakeup功能 后续教程会讲到电源管理相关内容 这里先占位
static aht_status_t aht21_sleep(bsp_aht21_driver_t *  p_aht21_instance){
    if(AHT21_NOT_INITED == g_inited)  
    {
        return AHT_ERRORRESOURCE;  //未初始化化
    }
    log_d("aht21_sleep");
    return AHT_OK;
}
static aht_status_t aht21_wakeup(bsp_aht21_driver_t *  p_aht21_instance){
    if(AHT21_NOT_INITED == g_inited)  
    {
        return AHT_ERRORRESOURCE;  //未初始化化
    }
    log_d("aht21_wakeup");
    return AHT_OK;
}
#endif
aht_status_t aht21_driver_instance(
                                    bsp_aht21_driver_t * const p_aht21_instance,
                                    iic_driver_instance_t  * const p_iic_instance,
                                    timebases_ms_t  *const p_timebase_ms,
                                    yield_interface_t   * const p_yield_interface
                                    )
{
    if(AHT21_INITED==g_inited)  
    {
        return AHT_ERRORRESOURCE;  //已经实例化过了
    }

    if (NULL == p_aht21_instance || 
        NULL == p_iic_instance   || 
        NULL == p_timebase_ms    || 
        NULL == p_yield_interface  ) 
    {
        return AHT_ERRORPARAMETER;
    }
    //外部接口赋值      AHT21_IIC_inst  p_aht21_instance->p_iic_instance
    p_aht21_instance->p_iic_instance = p_iic_instance;  //rtos c(app)层传入iic实例指针
    p_aht21_instance->p_timebase_ms = p_timebase_ms;
    p_aht21_instance->p_yield_interface = p_yield_interface;
   // Initialize function pointers
    p_aht21_instance->pf_init = aht21_init;
    p_aht21_instance->pf_deinit = aht21_deinit;
    p_aht21_instance->pf_read_id = aht21_read_id;
    p_aht21_instance->pf_read_temp = aht21_read_temp;
    p_aht21_instance->pf_read_humi = aht21_read_humi;
    p_aht21_instance->pf_sleep = aht21_sleep;
    p_aht21_instance->pf_wakeup = aht21_wakeup;

    //调用初始化函数
    aht21_init(p_aht21_instance);
    g_inited = AHT21_INITED;
    log_d("aht21_driver_instance init done\r\n");

    return AHT_OK;
}
