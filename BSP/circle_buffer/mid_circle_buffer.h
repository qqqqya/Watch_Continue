#ifndef __MID_CIRCLE_BUFFER_H__
#define __MID_CIRCLE_BUFFER_H__

#include <stdint.h>//uint8_t uint32_t
#include <stdio.h>//null
#include "elog.h"

// typedef struct{
//   uint8_t data[CIRCLE_BUFFER_SIZE];
//   uint32_t head;
//   uint32_t tail;
// }circle_buffer_t;//结构体内直接包含循环数组
/**独立内存，大小 = 成员总和 + 对齐填充，所有成员共存；
 * --总大小：1+3+4+2 = 10 字节，不是最大成员 4 的整数倍，再填充 2 字节；
联合体：共享内存，大小 =最大成员大小，同一时间只用一个首地址；
 */

 /***#### 相关数据类型--结构体{头  尾  大小}
#### 这几个函数-创建空buffer--判断empyt-full---put get data。 */

// MPU6050数据包大小定义
#define MPU6050_DATA_PACKET_SIZE    14  // 加速度(6) + 温度(2) + 陀螺仪(6) = 14字节

typedef struct circular_buffer
{
    uint8_t *buffer; // 缓冲区
    uint8_t rflag;   // 读位置
    uint8_t wflag;   // 写位置
    uint8_t *(*pfget_wbuffer_addr)(struct circular_buffer *); // 获取写缓冲区地址
    uint8_t *(*pfget_rbuffer_addr)(struct circular_buffer *); // 获取读缓冲区地址
    void (*pfdata_writed)(struct circular_buffer *);          // 写数据
    void (*pfdata_readed)(struct circular_buffer *);          // 读数据
    uint8_t size;    // 缓冲区槽位数量
} circular_buffer_t;
#endif // end __MID_CIRCLE_BUFFER_H__
