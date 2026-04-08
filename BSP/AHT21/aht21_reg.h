/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_aht21_driver.h
 * 
 * @par dependencies 
 * - stdio.h
 * - stdint.h
 * 
#include "stdint.h"              
 * @author Jack | R&D Dept. | EternalChip ?????
 *
 * @brief Provides HAL APIs for LED control and operations.
 * 
 * Usage:
 * Call functions directly.
 * 
 * @version V1.0 2026年4月8日 19点43分
 *
 * @note 1 tab == 4 spaces
 * 
 *****************************************************************************/
#ifndef __AHT21_REG_H__
#define __AHT21_REG_H__



//******************************* Defines ***********************************//
#define AHT21_READ_DATA_REG             0x71           //aht21  读数据
#define AHT21_WRITE_DATA_REG            0x70           //aht21  写数据reg  0x38+0
#define AHT21_MEASURE_CMD               0xAC           //measure code
#define AHT21_MEASURE_CMD_PARAMS1       0x33           //measure 指令的 两个参数
#define AHT21_MEASURE_CMD_PARAMS2       0x00           //两个参数

#endif
