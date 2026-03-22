/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_led_driver.c
 * 
 * @par dependencies 
 * - bsp_led_driver.h
 * - stdint.h
 * - bsp_led_handler.h
#include "bsp_led_handler.h"
#include "stdint.h"
 * @author Yan | R&D Dept. | EternalChip
 *
 * @brief Provides HAL APIs for LED control and operations.
 * 
 * Usage:
 * Call functions directly.
 * 
 * @version V1.0 2026-03-19
 *
 * @note 1 tab == 4 spaces
 * 
 * 
******************************* includes ***********************************
******************************* fuctions ***********************************
 *****************************************************************************/
/******************************** includes ************************************/
 #include "bsp_led_handler.h"

/******************************** Declares ************************************/
static led_handler_status_t __array_init__(bsp_led_driver_t *   arry[], 
                                            uint32_t            size
){
    for(uint32_t i = 0; i < size; i++){
        arry[i] = (bsp_led_driver_t *)INIT_PATTERN;   //初始化指向一个特殊值 地址
    }
    //TBD   mem check
    return HANDLER_OK;
}


led_handler_status_t led_register(
                                    bsp_led_handler_t * const self,    //pointer 需要在上面声明
                                    bsp_led_driver_t * const led_driver,//实际的对象-把所有信息都告诉了
                                    led_index_t     * const led_index

)
{//注册函数--内部调用即可
    ////实际的对象-把所有信息都告诉了  注册每一个led对象
#ifdef DEBUG
        DEBUGPRINT("Start_led_register \r\n");
#endif // DEBUG
    led_handler_status_t ret = HANDLER_OK;
/**************** 1、检查目标是否被实例化 ***********************************/
        if( NULL            ==  led_driver      ||
            NULL            ==  led_index       ||
          HANDLER_NOT_INITED ==  self->is_inited   )
        {
#ifdef DEBUG
        DEBUGPRINT("HANDLER_ERRORPARAMETER\r\n");
#endif  //debug	HANDLER_NOT_INITED		HANDLER_IS_INITED
            ret = HANDLER_ERRORPARAMETER;
            return ret;
        }

#ifdef OS_SUPPORTING        
        self->p_os_critical->pf_os_critical_enter();//进入临界区  vPortEnterCritical
#endif
/****************挂载对象       *******************************************/
        if((MAX_LED_INSTANCES-self->register_led_instances.led_instance_num)>0){
            //此时如果多线程调用 会触发数组越界  需要处理越界问题---solve  临界区
            self->register_led_instances.led_instance_aarry[*led_index] = \
                                                                            led_driver;
            *led_index = self->register_led_instances.led_instance_num;//
            //直接操作原来的变量  注册成功后  led_index 会增加1
            self->register_led_instances.led_instance_num++;
        }
#ifdef OS_SUPPORTING        
        self->p_os_critical->pf_os_critical_exit();//退出临界区  vPortExitCritical
#endif
         


#ifdef DEBUG
        DEBUGPRINT("led_register Succees!\r\n");
#endif // DEBUG
        return ret;
}
/*@brief 控制LED的行为本函数用于控制指定LED的闪烁行为，包括闪烁周期、闪烁次数以及亮灭比例

@param self LED处理器的实例指针，用于访问LED处理器的成员变量和函数
@param CycLetime 闪烁周期，单位为毫秒，表示一次完整的亮灭循环所需时间
@param blink_times闪烁次数，表示LED将重复闪烁的次数
@param proportion_on_off 亮火比例，表示在一个闪烁周期内亮起和熄灭时间的比例indexLED索引I，用于指定需要控制的LED@param
@return Led_handler status_t 返回LED处理器的状态码，表示函数执行结果
*/
static led_handler_status_t led_control(        bsp_led_handler_t * const    self,    //pointer 需要在上面声明
                                                uint32_t                    period_ms,              //period ms
                                                uint32_t                    times,               //times 
                                                led_proportion_t            proportion_blink,      //proportion 3:1 2:1 1:1
                                                led_index_t      const     led_index
){

        led_handler_status_t ret = HANDLER_OK;
/**************** 1、检查目标是否被实例化 ***********************************/
        if( NULL                  ==  self   ||
            HANDLER_NOT_INITED    ==  self->is_inited   )
        {
#ifdef DEBUG
        DEBUGPRINT("HANDLER_ERRORPARAMETER\r\n");
#endif  //debug
            return HANDLER_ERRORPARAMETER;
        }
/**************** 2、检查参数是否合法 ***********************************/
        if(  ! ( 
                (6000 > period_ms)                              &&
                (100 > times)                                   &&
                (proportion_blink <= PROPORTIONN_1_1)           &&
                (proportion_blink >= PROPORTIONN_1_3)
                )   //end of !
        ){          //end of if 
#ifdef DEBUG
        DEBUGPRINT("0LED_ERROR_PARAMETER\r\n");
#endif  //debug
            return LED_ERRORPARAMETER;
        }

/****************3、向LED队列中发送事件 ***********************************/
/*准备LED事件结构体     包含LED周期、次数、比例*/
        led_event_t led_event={
                .period_ms=period_ms,
                .times=times,
                .proportion=proportion_blink,
        };
//将led事件放入到队列中控制led行为
        // ret = self->p_os_queue_interface->pf_os_queue_put(self->,&led_event,pdMAX_DELAY);
        //这里的queue从
//检查队列发送成功p_os_queue->pf_os_queue_send(&led_event);

        return ret;
}

/*实例化包含
    接口指向  接口init 另一个函数 设置亮灭
        目标变量init 全给0即可
            后续的目标变量修改用的是另一个函数指针*/
led_handler_status_t led_handler_instance(   
                                    bsp_led_handler_t           * const self    ,          //led handler struct pointer
                                                                //这个传入self可以直接操作结构体中的变量
                                    handelr_timebase_t       * const timebase_ms,         //timebase -tick ms
#ifdef OS_SUPPORTING        
                                    handler_os_delay_t       * const os_delay_ms,          //os delay ms   
                                    handler_os_queue_t          * const os_queue, //os queue interface
                                    handler_os_critical_t       * const os_critical //os critical interface
#endif
)
{    
        led_handler_status_t ret = HANDLER_OK;
        /*判断指针 是否为空*/
        if( NULL  ==  self          ||
#ifdef DEBUG
            NULL  ==  os_delay_ms   ||
            NULL  ==  os_queue      ||
            NULL  ==  os_critical   ||
#endif  //debug          
            NULL  ==  timebase_ms       
         )
        {
#ifdef DEBUG
        DEBUGPRINT("HANDLER_ERRORPARAMETER\r\n");
#endif  //debug
            ret = HANDLER_ERRORPARAMETER;
            return ret;
        }

        /*判断是否已经实例化*/
        if(HANDLER_IS_INITED  ==  self->is_inited)
        {
#ifdef DEBUG
            DEBUGPRINT("HANDLER_ERROR_RESOURCE\r\n");
#endif  //debug
            ret = HANDLER_ERRORRESOURCE;
            return ret;
        }
 
#ifdef DEBUG
        DEBUGPRINT("HANDLER instance start\r\n");
#endif  //debug

/*************3、初始化接口  **********************/
        //外部接口
        self->p_os_delay_ms             = os_delay_ms;
        self->p_os_queue_interface      = os_queue   ;
        self->p_timebase_ms             = timebase_ms;
        self->p_os_critical             = os_critical;
        
        //内部接口
        self->pf_led_control            = led_control   ;//函数指针 指向led_control函数
        self->pf_led_register           = led_register  ;

#ifndef OS_SUPPORTING
    
#endif //#ifndef OS_SUPPORTING
/************4 、初始化目标变量     闪烁功能变量**********************/
        /************内部资源初始化--之前是闪烁次数周期等具体的--现在是led对象（实例）的数组**********************/
        self->register_led_instances.led_instance_num = 0;  //index init=0
        ret = __array_init__(self->register_led_instances.led_instance_aarry,
                                MAX_LED_INSTANCES);
        if(ret != HANDLER_OK)
        {
#ifdef DEBUG
            DEBUGPRINT("HANDLER_ERRORPARAMETER\r\n");
#endif  //debug
            return ret;
        }
        
        self->is_inited = HANDLER_IS_INITED;
#ifdef DEBUG
        DEBUGPRINT("HANDLER instance finished\r\n");
#endif  //debug

        return ret;

}





