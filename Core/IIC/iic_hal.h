#ifndef __IIC_HAL_H
#define __IIC_HAL_H

#include "stm32f4xx_hal.h"

typedef struct
{
	GPIO_TypeDef * IIC_SDA_PORT;
	GPIO_TypeDef * IIC_SCL_PORT;
	uint16_t IIC_SDA_PIN;
	uint16_t IIC_SCL_PIN;
	//void (*CLK_ENABLE)(void);
}iic_bus_t;		//(串口1(PA9)(PA10)  (PB13(SDA)/PB14(SCL)引|脚

void IICStart(iic_bus_t *bus);
void IICStop(iic_bus_t *bus);
unsigned char IICWaitAck(iic_bus_t *bus);
void IICSendAck(iic_bus_t *bus);
void IICSendNotAck(iic_bus_t *bus);
void IICSendByte(iic_bus_t *bus, unsigned char cSendByte);	
unsigned char IICReceiveByte(iic_bus_t *bus);
void IICInit(iic_bus_t *bus);
//发送一个字节--直接传val
//多个字节--传数组+长度--就是传的地址
uint8_t IIC_Write_One_Byte(iic_bus_t *bus, uint8_t daddr,uint8_t reg,uint8_t data);
uint8_t IIC_Write_Multi_Byte(iic_bus_t *bus, uint8_t daddr,uint8_t reg,uint8_t length,uint8_t buff[]);
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
unsigned char IIC_Read_One_Byte(iic_bus_t *bus, uint8_t daddr,uint8_t reg);
uint8_t IIC_Read_Multi_Byte(iic_bus_t *bus, uint8_t daddr, uint8_t reg, uint8_t length, uint8_t buff[]);
#endif
