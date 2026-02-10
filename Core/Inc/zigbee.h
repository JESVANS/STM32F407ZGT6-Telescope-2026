#ifndef __ZigBee_H__
#define __ZigBee_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "usart.h"

/**
 * @brief ZigBee 模块操作状态枚举
 */
typedef enum {
    DL_OK = 0,      // 操作成功
    DL_ERROR,       // 通用错误
    DL_TIMEOUT,     // 超时
    DL_INVALID,     // 参数无效
    DL_BUSY         // 忙碌
} DL_Status_t;

/* 默认使用的串口句柄，若未定义则使用 huart2 */
#ifndef ZigBee_DEFAULT_UART
#define ZigBee_DEFAULT_UART      (&huart2)
#endif

/* 默认波特率，硬件固定为 115200 */
#ifndef ZigBee_DEFAULT_BAUDRATE
#define ZigBee_DEFAULT_BAUDRATE  115200U 
#endif

/**
 * @brief ZigBee 数据包结构体 (解包后的应用层数据)
 */
typedef struct {
    uint16_t addr;      // 远程地址 (0x0000: 本模块, 0xFFFF: 广播)
    uint8_t  src;       // 源端口号
    uint8_t  dst;       // 目的端口号
    uint8_t  data[128]; // 数据载荷
    uint16_t data_len;  // 数据长度
} DL_Packet_t;

/* ================= API 函数声明 ================= */

/**
 * @brief 初始化 ZigBee 模块的中断接收
 * @note  需要在 main 初始化阶段调用，启动 UART 接收中断
 */
void ZigBee_InitIT(void);

/**
 * @brief 构建发送帧 (封包)
 * @param src 源端口
 * @param dst 目的端口
 * @param addr 远程地址
 * @param payload 数据载荷指针
 * @param payload_len 数据长度
 * @param frame_out [输出] 指向构建好的原始帧缓冲区的指针
 * @param frame_len_out [输出] 构建好的帧长度
 * @return DL_Status_t
 */
DL_Status_t ZigBee_BuildFrame(uint8_t src, uint8_t dst, uint16_t addr,
                               const uint8_t *payload, uint16_t payload_len,
                               uint8_t **frame_out, uint16_t *frame_len_out);


/**
 * @brief   //数据拆分装填函数
 * @param content  数据内容指针(数组)
 * @param content_p  内容长度(外部指针)**
 * @param data  需要装填的数据
 * @return DL_Status_t
 */
DL_Status_t ZigBee_Load_Data(uint8_t *content, uint8_t content_p, int data);


/**
 * @brief 阻塞式发送数据包 (自动封包并发送)
 * @param src 源端口
 * @param dst 目的端口
 * @param addr 远程地址
 * @param payload 数据载荷
 * @param payload_len 数据长度
 * @return DL_Status_t
 */
DL_Status_t ZigBee_SendPacket(uint8_t src, uint8_t dst, uint16_t addr,
                               const uint8_t *payload, uint16_t payload_len);

/**
 * @brief 中断式发送原始帧 (非阻塞)
 * @param huart 串口句柄 (传 NULL 则使用默认)
 * @param frame 原始帧数据指针
 * @param length 帧长度
 * @return DL_Status_t
 */
DL_Status_t ZigBee_SendFrameIT(UART_HandleTypeDef *huart,
                                const uint8_t *frame,
                                uint16_t length);

/**
 * @brief 阻塞式接收并解析数据包
 * @param pkt [输出] 解析后的数据包结构体
 * @param timeout_ms 超时时间 (毫秒)
 * @return DL_Status_t
 */
DL_Status_t ZigBee_RecvPacket(DL_Packet_t *pkt, uint32_t timeout_ms);

/* ================= 中断回调辅助函数 ================= */
/**
 * @brief  ZigBee UART 接收完成回调（需在 HAL_UART_RxCpltCallback 中调用）
 *         处理收到的字节并重新启动中断接收
 */
void        ZigBee_UART_RxCpltCallback(void);

/**
 * @brief 查询是否接收到完整的一帧数据
 * @return 1: 有新帧, 0: 无
 */
uint8_t     ZigBee_IsRxFrameReady(void);

/**
 * @brief 获取接收到的原始帧数据
 * @param buf [输出] 缓冲区
 * @param buf_len 缓冲区大小
 * @return 实际拷贝的字节数
 */
uint16_t    ZigBee_GetRxFrame(uint8_t *buf, uint16_t buf_len);

/* ================= 接收字节处理函数 ================= */
/* 将字节数组转成十六进制字符串，如 "FE 08 91 90 ..." */
void bytes_to_hex_str(const uint8_t *buf, uint16_t len, char *out, uint16_t out_size);
/* 提取包中实际数据 */
void ZigBee_Read_data(const uint8_t *buf, uint16_t len, DL_Packet_t *repkt);


#ifdef __cplusplus
}
#endif

#endif


