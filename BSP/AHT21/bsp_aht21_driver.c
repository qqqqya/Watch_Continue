
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

aht_status_t aht21_init( bsp_aht21_driver_t *  self){



}

aht_status_t aht21_driver_instance(
                                    bsp_aht21_driver_t * const self,
                                    iic_driver_instance_t  * const p_iic_instance,
                                    timebases_t  *const p_timebase_ms,
                                    yield_interface_t   * const p_yield_interface
                                    )
{
    if (self == NULL || 
        p_iic_instance == NULL || 
        p_timebase_ms == NULL || 
        p_yield_interface == NULL) {
        return AHT_ERRORPARAMETER;
    }

    self->p_iic_instance = p_iic_instance;//p_iic_instance
    self->p_timebase_ms = p_timebase_ms;
    self->p_yield_interface = p_yield_interface;
/*    // Initialize function pointers
    self->pf_init = aht21_init;
    self->pf_deinit = aht21_deinit;
    self->pf_read_id = aht21_read_id;
    self->pf_read_temp = aht21_read_temp;
    self->pf_read_humi = aht21_read_humi;
    self->pf_sleep = aht21_sleep;
    self->pf_wakeup = aht21_wakeup;
*/

    return AHT_OK;
}
