
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
#define AHT21_MEASUREMENT_TIME_MS 80            // Measurement time [ms]
#define AHT21_NOT_INITED     0            // Not init. flag 
#define AHT21_INITED         1            // Not init. flag 
#define AHT21_ID                  0x18           // AHT21 ID--STATUS寄存器相与0x18，结果为0x18，说明是AHT21传感器

#define AHT21_CRC8_POLYNOMIAL     0x31           // CRC-8 polynomial
#define AHT21_CRC8_INITIAL        0xFF           // CRC-8 initial value

