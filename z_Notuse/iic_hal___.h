/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file iic_hal.h
 * 
 * @par dependencies 
 * - stdio.h
 * - stdint.h
 * 
#include "stdio.h"
#include "stdint.h"

#include "stm32f4xx_hal.h"
 * @author Yan | R&D Dept. | EternalChip
 *
 * @brief Provides HAL APIs for LED control and operations.
 * 
 * Usage:
 * Call functions directly.
 * 
 * @version V1.0 2026-04-01
 *
 * @note 1 tab == 4 spaces
 * 
 * 
******************************* includes ***********************************
******************************* fuctions ***********************************
 *****************************************************************************/


/******************************** includes ************************************/
#ifndef __IIC_HAL_H__
#define __IIC_HAL_H__
#include "stm32f4xx_hal.h"
typedef enum{
    IIC_OK = 0,
    IIC_ACK,
    IIC_NACK,
    IIC_ERROR_TIMEOUT, 

}iic_status_t;

typedef struct{
    GPIO_TypeDef*   SCL_PORT;
    uint16_t        SCL_PIN;
    GPIO_TypeDef*   SDA_PORT;
    uint16_t        SDA_PIN;   
    // void(*iic_delay)(uint32_t us);

}iic_bus_t;//(串口1(PA9)(PA10)  (PB13(SDA)/PB14(SCL)引|脚


void IIC_init(iic_bus_t* bus);////////cube直接做了

void IIC_start  (iic_bus_t* bus);       
void IIC_stop   (iic_bus_t* bus);

uint8_t IIC_recv_byte       (iic_bus_t* bus);  //直接  return 接收到的数据
iic_status_t IIC_wait_ack   (iic_bus_t* bus);

iic_status_t IIC_send_ack   (iic_bus_t* bus);//主机sda发送0 拉下来
iic_status_t IIC_send_notack(iic_bus_t* bus);
iic_status_t IIC_send_byte(iic_bus_t* bus, uint8_t byte);

/**
 * @brief Write a byte to I2C register.
 * 写到设备地址  寄存器地址 写的data值  ,写多个值还得数组
 * 
 * @param bus I2 bus structure.
 * @param daddr I2C device address.
 * @param reg I2C register address.
 * @param byte Byte to write.
 * @return iic_status_t Status of the operation.
 */
iic_status_t IIC_write_one_byte                       (iic_bus_t* bus, uint8_t daddr,uint8_t reg,uint8_t byte);
iic_status_t IIC_write_multiply_byte    (iic_bus_t* bus, uint8_t daddr,uint8_t reg,uint8_t buff[],uint8_t len);
/**
 * @brief Read a byte from I2C register.
 * 从设备地址  寄存器地址 读的data值  ,读多个值还得数组
 *      读单个的值直接return即可
 * @param bus I2 bus structure.
 * @param daddr I2C device address.
 * @param reg I2C register address.
 * @param byte Byte to read.
 * @return uint8_t Byte read.
 */
uint8_t IIC_read_one_byte                       (iic_bus_t* bus, uint8_t daddr,uint8_t reg,uint8_t byte);
iic_status_t IIC_read_multiply_byte    (iic_bus_t* bus, uint8_t daddr,uint8_t reg,uint8_t buff[],uint8_t len);
// /
#endif
