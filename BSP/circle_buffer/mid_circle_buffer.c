/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file circular_buffer.c
 *
 * @par dependencies
 *
 * - circular_buffer.h
 *
 * @author liu
 *
 * @brief Provide the circular buffer APIs.
 *
 * Processing flow:
 *
 * call directly.
 *
 * @version V1.0 2024-12-06
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/
#include "mid_circle_buffer.h"
#include <string.h>
#include <stdlib.h>

circular_buffer_t circular_buf;

uint8_t *get_wbuffer_addr(circular_buffer_t *buffer)
{
    return buffer->buffer + buffer->wflag * MPU6050_DATA_PACKET_SIZE;
}

uint8_t *get_rbuffer_addr(circular_buffer_t *buffer)
{
    return buffer->buffer + buffer->rflag * MPU6050_DATA_PACKET_SIZE;
}

void data_writed(circular_buffer_t *buffer)
{
    // DMA写数据结束
    // todo:buffer已满
    buffer->wflag = (buffer->wflag + 1) % buffer->size;
}

void data_readed(circular_buffer_t *buffer)
{
    // 读取数据结束
    // todo: 没有可读数据
    buffer->rflag = (buffer->rflag + 1) % buffer->size;
}

void buffer_init(circular_buffer_t *buffer, uint8_t size)
{
    if (NULL == buffer)
    {
        log_i("buffer is NULL");
    }

    buffer->size = size;  // 槽位数量
    buffer->rflag = 0;
    buffer->wflag = 0;

    /*buffer 分配空间: 槽位数量 × 每个槽位的数据包大小*/
    buffer->buffer = (uint8_t *)malloc(size * MPU6050_DATA_PACKET_SIZE);

    buffer->pfget_rbuffer_addr = get_rbuffer_addr;
    buffer->pfget_wbuffer_addr = get_wbuffer_addr;
    buffer->pfdata_readed      = data_readed;
    buffer->pfdata_writed      = data_writed;
	log_i("buffer is inited");
}
