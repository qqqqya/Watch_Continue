
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
#include "elog.h"

/******************************** Defines ************************************/
#define HANDLER_INITED          true
#define HANDLER_NOT_INITED      false

#define MY_MAX_DELAY 0xffffffffUL
bsp_handler_aht21_driver_t * g_handler_aht21_instance = NULL;
   
static handler_aht_status_t __g_val_init(bsp_handler_aht21_driver_t * p_handler_aht21_instance){
    handler_aht_status_t ret = HANDLER_AHT_OK;
    if(NULL == p_handler_aht21_instance){
        ret = HANDLER_AHT_ERRORPARAMETER;
    }
    g_handler_aht21_instance = p_handler_aht21_instance;
    return ret;
}
static handler_aht_status_t aht21_handler_init(bsp_handler_aht21_driver_t * const p_handler_aht21_instance

){
    handler_aht_status_t ret = HANDLER_AHT_OK;
    printf("aht21_handler_     init begin\r\n");
    /******检查参数是否为空**********/
    if(NULL == p_handler_aht21_instance){
        ret = HANDLER_AHT_ERRORPARAMETER;
    }
    /******队列创建**********/
    ret = p_handler_aht21_instance->p_os_queue->Queuecreate(10,
                                                            sizeof(handler_aht_event_t),
                                                            &(p_handler_aht21_instance->p_queue_handler));
    if(ret != HANDLER_AHT_OK){
        return ret;
    }
    printf("queue_handler____:%p\r\n",p_handler_aht21_instance->p_queue_handler);
    /******进行iic interface 相关挂载 初始化
     * 接口传给aht instance**********/
    printf("aht21_driver   init begin\r\n");
    ret=aht21_driver_instance(p_handler_aht21_instance->p_aht21_instance,
                            p_handler_aht21_instance->p_iic_instance,
                            p_handler_aht21_instance->p_timebase_ms,
                            p_handler_aht21_instance->p_yield_interface);
    ///这里是架构图中的handler层把hal iic驱动 时基 延时的接口传给aht21_instance
    if(ret != HANDLER_AHT_OK){
        return ret;
    }
    printf("aht21_handler_driver   init OK------------\r\n");
    return ret;

}

static handler_aht_status_t aht21_handler_deinit(bsp_handler_aht21_driver_t * const p_handler_aht21_instance

){
    p_handler_aht21_instance->p_private_data_init->is_initated = HANDLER_NOT_INITED;
    return HANDLER_AHT_OK;
}

handler_aht_status_t aht21_handler_instance(
                                    bsp_handler_aht21_driver_t * const p_handler_aht21_instance,//
                                    sensor_interface_i2c_timebase_delay_t * const p_sensor_interface
){
    handler_aht_status_t ret = HANDLER_AHT_OK;
    printf("aht21_handler_instance start------------\r\n");

    /******1. 检查参数是否为空**********/
    if(NULL == p_handler_aht21_instance||
       NULL == p_sensor_interface){
        ret = HANDLER_AHT_ERRORPARAMETER;
    }
    if(NULL == p_sensor_interface->p_iic_instance   ||
       NULL == p_sensor_interface->p_timebase_ms    ||
       NULL == p_sensor_interface->p_yield_interface
    ){
        printf("p_sensor_interface is NULL\r\n");
        ret = HANDLER_AHT_ERRORPARAMETER;
    }
    // 这个 inited 怎么挂载来着if(NULL == p_handler_aht21_instance->p_private_data_init->inited){
    //     ret = HANDLER_AHT_ERRORPARAMETER;
    // }
    /******2. 初始化接口挂载**********/
    p_handler_aht21_instance->p_iic_instance=p_sensor_interface->p_iic_instance;
    p_handler_aht21_instance->p_timebase_ms=p_sensor_interface->p_timebase_ms;
    p_handler_aht21_instance->p_yield_interface=p_sensor_interface->p_yield_interface;
    p_handler_aht21_instance->p_os_queue=p_sensor_interface->p_os_queue;
    /******3. 进行调用handler init**********/
    ret = aht21_handler_init(p_handler_aht21_instance);

    /******4. init 标志位修改**********/
    p_handler_aht21_instance->p_private_data_init->is_initated = HANDLER_INITED;
    printf("aht21_handler_instance OK------------\r\n");
    return ret;
}
static handler_aht_status_t get_humi_temp(
    bsp_handler_aht21_driver_t  *  p_handler_aht21_instance,
    handler_aht_event_t         *  p_event,
    float   *  humi_val,
    float   *  temp_val
){    /******传入handler 句柄  事件 温湿度val**********/

    handler_aht_status_t ret = HANDLER_AHT_OK;
    /******检查参数是否为空**********/
    if(NULL == p_handler_aht21_instance||
       NULL == p_event                  ){
        ret = HANDLER_AHT_ERRORPARAMETER;
    }

    /******读取湿度温度**********/
    switch(p_event->humi_temp_select){/*实际上要先读取湿度 再读取温度   所以事件中选择both*/
        case HUMI_TEMP_HUMI:
            ret = p_handler_aht21_instance->p_aht21_instance->pf_read_humi(p_handler_aht21_instance->p_aht21_instance,
                                                                humi_val);
            break;
        case HUMI_TEMP_TEMP:
            ret = p_handler_aht21_instance->p_aht21_instance->pf_read_temp(p_handler_aht21_instance->p_aht21_instance,
                                                                temp_val);
            break;
        case HUMI_TEMP_BOTH:
        ret = p_handler_aht21_instance->p_aht21_instance->pf_read_humi(p_handler_aht21_instance->p_aht21_instance,
                                                                humi_val);
        ret = p_handler_aht21_instance->p_aht21_instance->pf_read_temp(p_handler_aht21_instance->p_aht21_instance,
                                                            temp_val);
            break;
        default:
            *temp_val = 0;
            *humi_val = 0;
            ret = HANDLER_AHT_ERRORPARAMETER;
            break;
    }

    /*p_handler_aht21_instance->p_aht21_instance->pf_read_humi(p_handler_aht21_instance->p_aht21_instance,
                                                                humi_val);*/

    if(ret != HANDLER_AHT_OK){
        return ret;
    }

    return ret;
}
/**
 * @brief handler 线程函数
 * 
 * @param arg 输入参数结构体 sensor_interface_i2c_timebase_delay_t p_sensor_interface
 * @return handler_aht_status_t 状态码
 */
void aht21_handler_thread_func(void * arg){
    float   humi_val = 0.0f;
    float   temp_val = 0.0f;
    handler_aht_status_t ret = HANDLER_AHT_OK;
        /******在线程函数内部初始化**********/
        bsp_handler_aht21_driver_t * p_handler_aht21_instance = {0};
        sensor_interface_i2c_timebase_delay_t * input_arg = NULL;
        handler_aht_event_t * p_event = {0};

        bsp_aht21_driver_t * aht21_driver_instance = {0};///////---------
    
    input_arg = (sensor_interface_i2c_timebase_delay_t *)arg;
    /******1. handler inst**********/
        //handler 实例如何传入？？？---定义一个
        //输入参数结构体 ________input arg ____sensor_interface_i2c_timebase_delay_t p_sensor_interface
        //****挂载driver_instance */
        p_handler_aht21_instance->p_aht21_instance=aht21_driver_instance;
    ret = aht21_handler_instance(p_handler_aht21_instance,
                                input_arg);
    if(ret != HANDLER_AHT_OK){  /*  p_aht21_instance->pf_init     (p_handler_aht21_instance->p_aht21_instance);
                                    // handler_instance contain> handler_init contain> aht21_driver_instance*/
        printf("aht21_handler_instance error\r\n");
    }
    /******1.1 全局handler 变量赋值 __靠近内核**********/
    __g_val_init(p_handler_aht21_instance);
    /******2. 读取出队列数据**********/
    ret = p_handler_aht21_instance->p_os_queue->Queueget(
                                                &(p_handler_aht21_instance->p_queue_handler),
                                                p_event,     //接收到队列中的 msg
                                                MY_MAX_DELAY);

    /******3. 数据处理 需要单独的业务函数**********/
        //调用get_humi_temp函数     需要传给业务函数handler 实例  事件event  温湿度val
    ret = get_humi_temp(
                        p_handler_aht21_instance,
                        p_event,
                        &humi_val,
                        &temp_val
                        );
    if(ret != HANDLER_AHT_OK){
        printf("get_humi_temp error\r\n");
    }
    printf("get_humi_temp!!!!!!!!!\r\n");
    /******4. 简单回调函数**********/
    p_event->pf_callback(&humi_val,&temp_val);
  

}
/**
 * @brief app层进行事件通知 写队列
 * 
 * @param event 事件指针
 */
handler_aht_status_t bsp_aht21_handler_read(handler_aht_event_t * event){
    handler_aht_status_t ret = HANDLER_AHT_OK;
    
    printf("handle send event start\r\n");
    /******1. 检查是否初始化**********/
    if(HANDLER_NOT_INITED == g_handler_aht21_instance->p_private_data_init->is_initated){
        printf("handler_read: not init\r\n");
        return HANDLER_AHT_ERRORPARAMETER;
    }
    /******2. 写队列**********/
    ret = g_handler_aht21_instance->p_os_queue->Queueput(&(g_handler_aht21_instance->p_queue_handler),
                                                        event,
                                                        MY_MAX_DELAY);
    if(ret != HANDLER_AHT_OK){
        printf("handler_read Queueput error\r\n");
        return ret;
    }                                       
    printf("handle send event OK\r\n");
    
    return ret;                                           
}