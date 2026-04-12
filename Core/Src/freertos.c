/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  * 
 * @par dependencies 
 * - bsp_led_driver.h
 * - stdint.h
 * - bsp_led_handler.h
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "system_adaptation.h"
#include "iic_hal.h"
#include "bsp_aht21_driver.h"
#include "delay.h"

#include "elog.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

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
/*****************多个字节暂时不用看后续教�??***************************/
int8_t        my_iic_send_multibyte       (void * bus,uint8_t * pdata,uint8_t size){
    IIC_Write_Multi_Byte(&iic_instance,NULL,NULL,size,pdata); //pdata是地�??�??般是个数�??
    return AHT_OK;                    //dev addr + reg addr + size + pdata
}//iic_bus_t *bus, uint8_t daddr,uint8_t reg,uint8_t length,uint8_t buff[]
int8_t        my_iic_rece_multibyte       (void * bus,uint8_t * pdata,uint8_t size){
    IIC_Read_Multi_Byte(&iic_instance,NULL,NULL,size,pdata);
    return AHT_OK;                    //dev addr + reg addr + size + pdata      
}
static int8_t my_iic_critical_enter(void)//os_critical_enter
{
    vPortEnterCritical();
}
static int8_t my_iic_critical_exit(void)
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
timebases_ms_t aht21_timebase_ms={
    .pf_timebase_gettickms =get_tick_ms,
};
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */


/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */



/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  printf("hello win\r\n");  
  bsp_aht21_driver_t aht21_instance;  
  aht21_driver_instance(&aht21_instance,      //初始化里里面只有init
                      &aht_iic_func_instance, 
                      &aht21_timebase_ms, 
                      &yield_interface
                    );
  printf("aht21_instance = %p\r\n",&aht21_instance);
  
  /************这里是后续handler 业务层做�? 现在这里只是单元测试*/
  float temp;
  float humi;
	aht21_instance.pf_read_humi(&aht21_instance,&humi);  //
  aht21_instance.pf_read_temp(&aht21_instance,&temp);  //
  printf("humi = %f\r\n",humi);
  printf("humi end\r\n");
  printf("temp = %f\r\n",temp);

  
//	printf("hello win\r\n");     
//  system_init_resources();
//  Test3();
//  printf("nihao win2222\r\n");     

	for(;;)   
	{	   

	//	HAL_GPIO_TogglePin(LED_Test_GPIO_Port, LED_Test_Pin);
	//	HAL_Delay(500);
		osDelay(1000);
	}
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

