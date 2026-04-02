/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_led_driver.c
 * 
 * @par dependencies 
 * - stdio.h
 * - stdint.h
 * 
#include "stdio.h"
#include "stdint.h"
#include "iic_hal.h"
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
 *****************************************************************************/


/******************************** includes ************************************/
 #include "iic_hal.h"



void SDA_OutputMode(iic_bus_t* bus){

   GPIO_InitTypeDef gpioinitstructure={0};

    gpioinitstructure.Pin=bus->SDA_PIN;
    gpioinitstructure.Mode=GPIO_MODE_OUTPUT_OD;
    gpioinitstructure.Pull=GPIO_PULLUP;
    gpioinitstructure.Speed=GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(bus->SDA_PORT,&gpioinitstructure);
}
void SDA_InputMode(iic_bus_t* bus){//从机发过来之后要改为input
   GPIO_InitTypeDef gpioinitstructure={0};

    gpioinitstructure.Pin=bus->SDA_PIN;
    gpioinitstructure.Mode=GPIO_MODE_INPUT;
    gpioinitstructure.Pull=GPIO_PULLUP;
    gpioinitstructure.Speed=GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(bus->SDA_PORT,&gpioinitstructure);

}
void SDA_Output(iic_bus_t* bus,uint16_t val){//write SDA pin value.
    HAL_GPIO_WritePin(bus->SDA_PORT,bus->SDA_PIN,   val==1?GPIO_PIN_SET:GPIO_PIN_RESET);
}
/**
 * @brief Read SDA pin value.
 * @param bus I2C bus structure.
 * @return uint16_t SDA pin value.-读出sda  pin slave send数据的时候
 */
uint16_t SDA_Input(iic_bus_t* bus){
    return (HAL_GPIO_ReadPin(bus->SDA_PORT,bus->SDA_PIN)==GPIO_PIN_SET)?1:0;
}

void SCL_Output(iic_bus_t* bus,uint16_t val){   
    HAL_GPIO_WritePin(bus->SCL_PORT,bus->SCL_PIN,   val==1?GPIO_PIN_SET:GPIO_PIN_RESET);
}
/**
 * @brief Initialize I2C bus.
 * SCL_PIN | I2C_SDA_PIN;   // 开漏输出  内部上拉（如果可用）
 * @param bus I2C bus structure.
 */
 void IIC_init(iic_bus_t* bus){
    GPIO_InitTypeDef gpioinitstructure={0};

    gpioinitstructure.Pin=bus->SCL_PIN|bus->SDA_PIN;
    gpioinitstructure.Mode=GPIO_MODE_OUTPUT_OD;
    gpioinitstructure.Pull=GPIO_PULLUP;
    gpioinitstructure.Speed=GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(bus->SCL_PORT,&gpioinitstructure);

 }
 
void IIC_start  (iic_bus_t* bus){
//    SDA_OutputMode(bus);
	SDA_Output(bus,1);	//还是先释放sda好一点
	SCL_Output(bus,1);

	SDA_Output(bus,0);//SDA Down
	SCL_Output(bus,0);
}      
void IIC_stop   (iic_bus_t* bus){
    //SCL 高电平上升沿
	SDA_Output(bus,0);//先拉底SDA因为起始可能不是低电平--实际都一样
	SCL_Output(bus,0);

	SDA_Output(bus,1);//
    SCL_Output(bus,1);
}

uint8_t IIC_recv_byte(iic_bus_t* bus){
    uint8_t recv=0,i=0;
    SDA_InputMode(bus);
    for(i;i<8;i++){

		SCL_Output(bus,0);
		SCL_Output(bus,1);
        recv |=SDA_Input(bus);  
        recv <<=1;//recv =recv <<1 
	}
    SCL_Output(bus,0);//主机掌控scl
    SDA_OutputMode(bus);
    return recv;
}
iic_status_t IIC_wait_ack(iic_bus_t* bus){//等待从机ack

    SDA_InputMode(bus);
    // SCL_Output(bus,0);   //实际上我在每个小块之前加一个0也是周全一点
    SCL_Output(bus,1);
    if(SDA_Input(bus)==1){
        return IIC_NACK;
    }
    SDA_OutputMode(bus);
    SCL_Output(bus,0);
    return IIC_ACK;
}

iic_status_t IIC_send_ack(iic_bus_t* bus){//主机sda发送ack

    SCL_Output(bus,0);

    SDA_Output(bus,0);//拉下
    SCL_Output(bus,1);
    SCL_Output(bus,0);
    return IIC_OK;
}
iic_status_t IIC_send_notack(iic_bus_t* bus){
    SCL_Output(bus,0);

    SDA_Output(bus,1);//不拉下
    SCL_Output(bus,1);
    SCL_Output(bus,0);
    return IIC_OK;
}


iic_status_t IIC_send_byte(iic_bus_t* bus, uint8_t byte)
{
    uint8_t offset=0x80,i=8;
    IIC_start( bus);//先发高位
    SCL_Output(bus,0);

    while(i--){
	    SCL_Output(bus,1);
        SDA_Output(bus,byte&offset);
        offset >>=1;
        SCL_Output(bus,0);
    }
    
    SCL_Output(bus,0);


}

/**
 * @brief Write a byte to I2C register.
 * 写到设备地址  寄存器地址 写的data值  ,写多个值还得数组
 * 
 * @param bus I2C bus structure.
 * @param daddr I2C device address.
 * @param reg I2C register address.
 * @param byte Byte to write.
 * @return iic_status_t Status of the operation.
 * 写指定设备 指定寄存器 写入数据
 */
iic_status_t IIC_write_one_byte(iic_bus_t* bus, uint8_t daddr,uint8_t reg,uint8_t byte){

    IIC_start( bus);

    IIC_send_byte(bus, daddr<<1|0);//写设备地址 7位地址+1位写标志
    if(IIC_wait_ack(bus)){IIC_stop(bus);return IIC_ERROR_TIMEOUT;}
                                                    //只需要等待从机ack 的时候检查一下就行了
    IIC_send_byte(bus, reg);    //写寄存器地址
    IIC_wait_ack(bus);
    IIC_send_byte(bus, byte);//写data数据
    IIC_wait_ack(bus);
    IIC_stop(bus);

    return IIC_OK;
}


uint8_t IIC_read_one_byte                       (iic_bus_t* bus, uint8_t daddr,uint8_t reg){
    uint8_t rbyte=0;
    IIC_start( bus);

    IIC_send_byte(bus, daddr<<1|0);//写设备地址 7位地址+1位写标志
    if(IIC_wait_ack(bus)){IIC_stop(bus);return IIC_ERROR_TIMEOUT;}

    IIC_send_byte(bus, reg);    //写寄存器地址
    if(IIC_wait_ack(bus)){IIC_stop(bus);return IIC_ERROR_TIMEOUT;}

    IIC_start( bus);
    IIC_send_byte(bus, daddr<<1|1);//写设备地址 7位地址+读标志 -1
    rbyte=IIC_recv_byte(bus);//读data数据
    IIC_send_notack(bus);//读完了之后主机发送nack 告诉从机我不想读了
    IIC_stop(bus);
    return rbyte;
}

iic_status_t IIC_write_multiply_byte    (iic_bus_t* bus, uint8_t daddr,uint8_t reg,uint8_t buff[],uint8_t len){
    uint8_t i=0;
    IIC_start( bus);
    IIC_send_byte(bus, daddr<<1|0);//写设备地址 7位地址+1位写标志
    if(IIC_wait_ack(bus)){IIC_stop(bus);return IIC_ERROR_TIMEOUT;}
                                                    //只需要等待从机ack 的时候检查一下就行了
    IIC_send_byte(bus, reg);    //写寄存器地址
    IIC_wait_ack(bus);
    for(i;i<len;i++){           //写multiply data数据
        IIC_send_byte(bus, buff[i]);
        IIC_wait_ack(bus);
    }
    IIC_stop(bus);
    return IIC_OK;
}

iic_status_t IIC_read_multiply_byte    (iic_bus_t* bus, uint8_t daddr,uint8_t reg,uint8_t buff[],uint8_t len){

    iic_start(bus);

    IIC_send_byte(bus, daddr<<1|0);//写设备地址 7位地址+1位写标志
    if(IIC_wait_ack(bus)){IIC_stop(bus);return IIC_ERROR_TIMEOUT;}
    IIC_send_byte(bus, reg);    //写寄存器地址
    IIC_wait_ack(bus);

    IIC_start( bus);
    IIC_send_byte(bus, daddr<<1|1);//写设备地址 7位地址+读标志 -1
    IIC_wait_ack(bus);
    uint8_t i=0;
    for(i;i<len;i++){           //读multiply data数据
        buff[i]=IIC_recv_byte(bus);
        IIC_send_ack(bus);//读完了之后主机发送ack 告诉从机我还想读
        if(i==len-1){
            IIC_send_notack(bus);//最后一个数据读完了之后主机发送nack 告诉从机我不想读了
        }
    }
    IIC_stop(bus);
    return IIC_OK;

}
#if 0




#endif
