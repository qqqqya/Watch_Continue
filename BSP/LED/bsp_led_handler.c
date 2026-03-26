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
#include "bsp_led_driver.h"
#include "FreeRTOS.h"
#include "queue.h"
/******************************** Declares ************************************/

/*准备LED事件结构体     包含LED周期、次数、比例  索引*/
typedef struct 
{
        uint32_t                    period_ms             ;              //period ms
        uint32_t                    times                 ;               //times
        led_proportion_t            proportion;      //proportion 3:1 2:1 1:1
        led_index_t                 led_index;      //led index
}led_event_t;

led_handler_status_t led_handler_blink(bsp_led_driver_t * self){
#ifdef DEBUG
        DEBUGPRINT("Start_Blink///////// \r\n");
#endif // DEBUG
        led_handler_status_t ret = HANDLER_OK;
/**************** 1、检查目标是否被实例化 ***********************************/
    if( NULL        ==  self   ||
        NOT_INITED   ==  self->is_inited   )    //检查是否被实例化
    {
        #ifdef DEBUG
    DEBUGPRINT("LED_ERROR_PARAMETER\r\n");
        #endif  //debug
        return LED_ERRORPARAMETER;
    }

/****************3、实现闪烁操作 ***********************************/
    {//局部代码块
        //通过对象中断内部数据转存 进行闪烁操作
        uint32_t                    period_local    = self->blink_period_ms;
        uint32_t                    times_local     = self->blink_times;
        led_proportion_t          proportion_local  = self->proportion_on_off;
        uint32_t                 time_off_ms;
        switch(proportion_local){
            case PROPORTIONN_1_3:
                time_off_ms = period_local/ 4;
                break;
            case PROPORTIONN_1_2:
                time_off_ms = period_local/ 3;
                break;
            case PROPORTIONN_1_1:
                time_off_ms = period_local / 2;
                break;
            default:
                break;
        }

        for(uint32_t i = 0; i < times_local; i++){
            
            for(uint32_t j = 0; j < period_local; j++){
                self->p_os_delay_ms->pf_osdelay_ms(500);                
                if(j < time_off_ms){
                    self->p_led_operation->pf_bsp_led_off();
                    // 结构体内部的函数指针指向函数才能调用
#ifdef DEBUG
                    DEBUGPRINT("LED_OFF\r\n");
#endif  //debug
                }
                else{
                        // self->p_os_delay_ms->pf_osdelay_ms(500);
                    self->p_led_operation->pf_bsp_led_on();
                    //这里的ledon已经指向具体的led_on_myown函数
#ifdef DEBUG
                    DEBUGPRINT("LED_ON\r\n");
#endif  //debug
                }
            }
        }
    }

    return ret;
}

led_handler_status_t __event_process(bsp_led_handler_t * self, led_event_t msg)
{
        led_handler_status_t ret = HANDLER_OK;
        /*************0.检查目标是否实例化 ******************************/
        /*************   has checked  *******************/        
        
        /*************1.检查传入参数合法 ******************************/
        /*************   has checked  *******************/

        /*************2.检查index 是否限幅正确 ******************************/
        if(     MAX_LED_INSTANCES < msg.led_index ||
                LED_NOT_INITIALIZED == msg.led_index 
        )
        {
#ifdef DEBUG
        DEBUGPRINT("HANDLER_ERRORPARAMETER\r\n");
#endif  //debug
            ret = HANDLER_ERRORPARAMETER;
            return ret;
        }
        /*************3.检查index 是否被实例化（非初始化模式） ******************************/
        if(LED_NOT_INITIALIZED == msg.led_index)
        {
#ifdef DEBUG
        DEBUGPRINT("index_instance_ERROR\r\n");
#endif  //debug
            ret = HANDLER_ERRORPARAMETER;
            return ret;
        }
        /*************4.检查index 指向的 leddriver 是否合法 ******************************/
/*handler的 self->register_led_instances.led_instance_aarry[*led_index] = \
        led_driver;挂载的是led_driver  */
        if(INIT_PATTERN == self->register_led_instances.led_instance_aarry[msg.led_index])
        {
#ifdef DEBUG
        DEBUGPRINT("HANDLER_ERRORRESOURCE at __event_process\r\n");
#endif  //debug
            ret = HANDLER_ERRORRESOURCE;
            return ret;
        }
        

/**************** 打印msg参数 ***********************************/
        printf("led_period: %d\r\n", msg.period_ms);
        printf("led_times: %d\r\n", msg.times);
        printf("led_proportion: %d\r\n", msg.proportion);
        printf("led_index: %d\r\n", msg.led_index);
/**************** 转存数据 ***********************************/ //非严谨版本 直接赋值成员变量
        self->register_led_instances.led_instance_aarry[msg.led_index]->blink_period_ms\
                                                                        = msg.period_ms;
        self->register_led_instances.led_instance_aarry[msg.led_index]->blink_times\
                                                                        = msg.times;
        self->register_led_instances.led_instance_aarry[msg.led_index]->proportion_on_off\
                                                                        = msg.proportion;
/********LED闪烁执行 ***************/
        led_handler_blink(self->register_led_instances.led_instance_aarry[msg.led_index]);
        return ret;
}

static led_handler_status_t __array_init__(bsp_led_driver_t *   arry[], 
                                            uint32_t            size
){
    for(uint32_t i = 0; i < size; i++){
        arry[i] = (bsp_led_driver_t *)INIT_PATTERN;   //初始化指向一个特殊值 地址
    }                                                 //相当于给初值 i=0
    //TBD   mem check
    return HANDLER_OK;
}


led_handler_status_t handler_thread(void * arguments)
{   
	vTaskDelay(1000 );
#ifdef DEBUG
        DEBUGPRINT("Start_Tread// \r\n");
#endif // DEBUG

        led_handler_status_t ret = HANDLER_OK;
        bsp_led_handler_t * p_led_handler;
        led_event_t     msg;
/**************** 1、检查目标是否被实例化 ***********************************/
        if( NULL  ==  arguments)
        {
#ifdef DEBUG
        DEBUGPRINT("HANDLER_ERRORPARAMETER\r\n");
#endif  //debug
            ret = HANDLER_ERRORPARAMETER;
            return ret;
        }else{
                p_led_handler = arguments;//(bsp_led_handler_t *)
        }
        printf("parameter in thread: %p\r\n", p_led_handler);
/**************** 2、检查参数是否合法 ***********************************/

        for(;;){
		DEBUGPRINT("Running_Tread// \r\n");
		vTaskDelay(1000 );
/**************** 3、读取队列中的事件 处理事件  ***********************************/
	ret = p_led_handler->p_os_queue_interface->pf_os_queue_get(
                                                        p_led_handler->queue_handler,
                                                                                &msg,//led_event_t
                                                                                   0);//LED周期、次数、比例  索引
	if(HANDLER_OK==ret){
		printf("message received\r\n");
                /*******************内部处理业务的函数******************** */
                //处理事件
                __event_process(p_led_handler, msg);
	}
 
        }
        return ret;
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
            self->register_led_instances.led_instance_aarry\
                                [self->register_led_instances.led_instance_num] = led_driver;
            *led_index = self->register_led_instances.led_instance_num;//
            //直接操作原来的变量  注册成功后  led_index 会增加1
            self->register_led_instances.led_instance_num++;

            /* 
            self->register_led_instances.led_instance_aarry[*led_index]=led_driver
            printf("=================led_index: %d\r\n", *led_index);*/
        }
#ifdef OS_SUPPORTING        
        self->p_os_critical->pf_os_critical_exit();//退出临界区  vPortExitCritical
#endif
         

#ifdef DEBUG
        DEBUGPRINT("led_register Succees!\r\n");
#endif // DEBUG
        return ret;
}


/*@brief 控制LED的行为本函数用于控制指定LED的闪烁行为，
包括闪烁周期、闪烁次数以及亮灭比例
        将事件发送到LED处理的队列中
                事件处理线程会从队列中读取事件并执行相应的操作
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
#ifdef DEBUG
        DEBUGPRINT("Control_Starttttttttttt\r\n");
#endif  //debug
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
#ifdef DEBUG
        DEBUGPRINT("sending event to queue\r\n");
#endif  //debug
/****************3、向LED队列中发送事件 ***********************************/
/*instance event LED事件结构体     包含LED周期、次数、比例*/
        led_event_t led_event={
                .period_ms=period_ms,
                .times=times,
                .proportion=proportion_blink,
                .led_index=led_index,
        };

//将led事件放入到队列中控制led行为
        ret = self->p_os_queue_interface->pf_os_queue_put(
                                                        self->queue_handler,
                                                                &led_event,
                                                                        0);
//检查队列发送成功p_os_queue->pf_os_queue_send(&led_event);
        if(ret != HANDLER_OK)   ///枚举变量名出错  返回的是HANDLER_ERROR
        {
#ifdef DEBUG
            DEBUGPRINT("queueHANDLER_ERRORPARAMETER\r\n");
#endif  //debug
        }
        return ret;
}

/*实例化包含
    接口指向  接口init 另一个函数 设置亮灭
        目标变量init 全给0即可
            后续的目标变量修改用的是另一个函数指针*/
led_handler_status_t led_handler_instance(   
                                    bsp_led_handler_t           *       const self    ,          //led handler struct pointer
                                                                //      这个传入self可以直接操作结构体中的变量
                                    handelr_timebase_t          *       const timebase_ms,         //timebase -tick ms
#ifdef OS_SUPPORTING            
                                    handler_os_delay_t          *       const os_delay_ms,          //os delay ms   
                                    handler_os_queue_t          *       const os_queue, //os queue interface
                                    handler_os_thread_t         *       const os_thread, //os thread interface
                                    handler_os_critical_t       *       const os_critical //os critical interface
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
            NULL  ==  os_thread     ||
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
        self->p_os_thread               = os_thread;

        //内部接口              ----挂载到handler
        self->pf_led_control            = led_control   ;//函数指针 指向led_control函数
        self->pf_led_register           = led_register  ;

#ifndef OS_SUPPORTING
    
#endif //#ifndef OS_SUPPORTING
/************4 、初始化目标变量     闪烁功能变量**********************/

        /******4.1 任务（线程）初始化*************/
        ret =    os_thread->pf_os_thread_create(        //
                                                        handler_thread,
                                                        "handler_thread1",      //tbd Thread name 不可重名
                                                        4*128,                  //tbd
                                                        self,   //param，给task传入的 内部参数
                                                        0,      ////wait 外部去写   一般是normal
                                                &(self->thread_handler)     //二级 当前线程的栈指针
                                        );///这个二级是因为要改变当前的任务的指针所以传二级  删除的时候只需要传入告知即可
        if(ret != HANDLER_OK)
        {
#ifdef DEBUG//创建线程失败打印
            DEBUGPRINT("HANDLER_ERROR in thread_create\r\n");
#endif  //debug
            return ret;
        }


        /******4.2 队列初始化*************/
        ret = os_queue->pf_os_queue_create(             10,
                                        sizeof(led_event_t),
                                        &(self->queue_handler)     //内部定义了一个void *queue_handler;
                                        ); //！！！传入的是二级指针  加取地址符号才可以传出去队列指针 不然会卡死
        printf("queue_handler = %p\r\n",self->queue_handler);
        if(ret != HANDLER_OK)
        {
#ifdef DEBUG//创建队列失败打印
            DEBUGPRINT("HANDLER_ERROR in queue_create\r\n");
#endif  //debug
            os_thread->pf_os_thread_delete(self->thread_handler); //后一步创建失败才会删除线程 防止内存泄漏
            return ret;
        }

        /******4.3 内部资源初始化************/
        /*-之前是闪烁次数周期等具体的--现在是led对象（实例）的数组**********************/
        self->register_led_instances.led_instance_num = 0;  //index init=0
        ret = __array_init__(self->register_led_instances.led_instance_aarry,
                                MAX_LED_INSTANCES);
        if(ret != HANDLER_OK)
        {
#ifdef DEBUG
            DEBUGPRINT("HANDLER_ERRORPARAMETER\r\n");
#endif  //debug
                self->p_os_queue_interface->pf_os_queue_delete(self->queue_handler);
                self->queue_handler = NULL;
                //shutdown os queue 防止内存泄漏
                return ret;
        }
        
        self->is_inited = HANDLER_IS_INITED;
#ifdef DEBUG
        DEBUGPRINT("HANDLER instance finished\r\n");
#endif  //debug

        return ret;

}





