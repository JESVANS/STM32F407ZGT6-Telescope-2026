/**
  ******************************************************************************
  * @file    bluetooth.c
  * @brief   JDY-31 Bluetooth Module Driver (阻塞发送 + 中断接收)
  ******************************************************************************
  */

#include "bluetooth.h"
#include <string.h>
#include <stdio.h>

/* ----------------------- 全局实例（默认配置） ----------------------------- */
BT_Handle_t hBluetooth = {
    .huart      = BT_DEFAULT_UART,
    .EN_Port    = BT_DEFAULT_EN_PORT,
    .EN_Pin     = BT_DEFAULT_EN_PIN,
    .STATE_Port = BT_DEFAULT_STATE_PORT,
    .STATE_Pin  = BT_DEFAULT_STATE_PIN,
    .baudrate   = BT_DEFAULT_BAUDRATE,
    .initialized = 0,
};

/* ----------------------- 内部接收缓冲区 ---------------------------------- */
static uint8_t  bt_rx_byte;                       /* 单字节中断接收暂存 */
static uint8_t  bt_rx_buf[BT_RX_BUF_SIZE];        /* 环形接收缓冲区    */
static volatile uint16_t bt_rx_head = 0;           /* 写入位置          */
static volatile uint16_t bt_rx_tail = 0;           /* 读取位置          */
static volatile uint8_t  bt_rx_line_flag = 0;      /* 收到完整行标志    */

/* ----------------------- 内部发送缓冲区 ---------------------------------- */
static uint8_t  bt_tx_buf[BT_TX_BUF_SIZE];         /* 中断发送缓冲区    */
static volatile uint8_t  bt_tx_busy = 0;            /* 发送忙标志        */

/* ======================== 内部辅助函数 =================================== */

/**
 * @brief  将一个字节压入环形缓冲区
 */
static void BT_RingBuf_Push(uint8_t byte)
{
    uint16_t next = (bt_rx_head + 1) % BT_RX_BUF_SIZE;
    if (next != bt_rx_tail) {          /* 缓冲区未满 */
        bt_rx_buf[bt_rx_head] = byte;
        bt_rx_head = next;
    }
    /* 检测行结束符 '\n' */
    if (byte == '\n') {
        bt_rx_line_flag = 1;
    }
}

/**
 * @brief  从环形缓冲区中取出可用数据
 * @return 实际读取的字节数
 */
static uint16_t BT_RingBuf_Read(uint8_t *dst, uint16_t max_len)
{
    uint16_t count = 0;
    while (bt_rx_tail != bt_rx_head && count < max_len) {
        dst[count++] = bt_rx_buf[bt_rx_tail];
        bt_rx_tail = (bt_rx_tail + 1) % BT_RX_BUF_SIZE;
    }
    return count;
}

/**
 * @brief  读取一行（到 '\n' 为止），不含换行符，末尾补 '\0'
 * @return 行长度（不含 '\0'），0 表示没有完整行
 */
static uint16_t BT_RingBuf_ReadLine(uint8_t *dst, uint16_t max_len)
{
    uint16_t count = 0;
    uint16_t tmp_tail = bt_rx_tail;

    while (tmp_tail != bt_rx_head && count < max_len - 1) {
        uint8_t ch = bt_rx_buf[tmp_tail];
        tmp_tail = (tmp_tail + 1) % BT_RX_BUF_SIZE;
        if (ch == '\n') {
            bt_rx_tail = tmp_tail;
            dst[count] = '\0';
            bt_rx_line_flag = 0;
            return count;
        }
        if (ch != '\r') {               /* 跳过 '\r' */
            dst[count++] = ch;
        }
    }
    /* 没找到完整行 */
    dst[count] = '\0';
    return 0;
}

/**
 * @brief  获取环形缓冲区中可用字节数
 */
static uint16_t BT_RingBuf_Available(void)
{
    return (bt_rx_head + BT_RX_BUF_SIZE - bt_rx_tail) % BT_RX_BUF_SIZE;
}

/* ======================== HAL 回调 ======================================= */

/**
 * @brief  UART 接收完成回调（中断上下文，由 HAL 调用）
 *         每收到 1 字节后自动重新启动中断接收
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == hBluetooth.huart->Instance) {
        BT_RingBuf_Push(bt_rx_byte);
        /* 重新启动单字节中断接收 */
        HAL_UART_Receive_IT(hBluetooth.huart, &bt_rx_byte, 1);
    }
}

/**
 * @brief  UART 发送完成回调（中断上下文，由 HAL 调用）
 *         清除发送忙标志
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == hBluetooth.huart->Instance) {
        bt_tx_busy = 0;
    }
}

/* ======================== 公开 API ======================================= */

/**
 * @brief  初始化蓝牙模块
 *         - EN 引脚拉低（透传模式）
 *         - 启动中断接收
 */
BT_Status_t BT_Init(BT_Handle_t *hbt)
{
    if (hbt == NULL || hbt->huart == NULL)
        return BT_INVALID;

    /* EN 拉低 → 透传模式 */
    HAL_GPIO_WritePin(hbt->EN_Port, hbt->EN_Pin, GPIO_PIN_RESET);

    /* 清空接收缓冲区 */
    bt_rx_head = 0;
    bt_rx_tail = 0;
    bt_rx_line_flag = 0;
    memset(bt_rx_buf, 0, sizeof(bt_rx_buf));

    /* 启动中断接收（1字节） */
    HAL_UART_Receive_IT(hbt->huart, &bt_rx_byte, 1);

    hbt->initialized = 1;
    return BT_OK;
}

/**
 * @brief  反初始化
 */
BT_Status_t BT_DeInit(BT_Handle_t *hbt)
{
    if (hbt == NULL)
        return BT_INVALID;

    HAL_UART_AbortReceive_IT(hbt->huart);
    hbt->initialized = 0;
    return BT_OK;
}

/**
 * @brief  阻塞发送数据
 */
BT_Status_t BT_Send(BT_Handle_t *hbt, const uint8_t *data, uint16_t len,
                     uint32_t timeout_ms)
{
    if (hbt == NULL || !hbt->initialized)
        return BT_INVALID;

    HAL_StatusTypeDef ret = HAL_UART_Transmit(hbt->huart, data, len, timeout_ms);
    if (ret == HAL_OK)
        return BT_OK;
    else if (ret == HAL_TIMEOUT)
        return BT_TIMEOUT;
    return BT_ERROR;
}

/**
 * @brief  发送字符串（阻塞）
 */
BT_Status_t BT_SendString(BT_Handle_t *hbt, const char *str, uint32_t timeout_ms)
{
    return BT_Send(hbt, (const uint8_t *)str, strlen(str), timeout_ms);
}

/**
 * @brief  格式化发送（类似 printf，阻塞）
 */
BT_Status_t BT_Printf(BT_Handle_t *hbt, uint32_t timeout_ms, const char *fmt, ...)
{
    char buf[BT_TX_BUF_SIZE];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (len <= 0)
        return BT_ERROR;
    return BT_Send(hbt, (const uint8_t *)buf, (uint16_t)len, timeout_ms);
}

/**
 * @brief  中断发送原始数据（非阻塞）
 */
BT_Status_t BT_Send_IT(BT_Handle_t *hbt, const uint8_t *data, uint16_t len)
{
    if (hbt == NULL || !hbt->initialized)
        return BT_INVALID;
    if (bt_tx_busy)
        return BT_ERROR;

    bt_tx_busy = 1;
    HAL_StatusTypeDef ret = HAL_UART_Transmit_IT(hbt->huart, data, len);
    if (ret != HAL_OK) {
        bt_tx_busy = 0;
        return BT_ERROR;
    }
    return BT_OK;
}

/**
 * @brief  中断发送字符串（非阻塞）
 */
BT_Status_t BT_SendString_IT(BT_Handle_t *hbt, const char *str)
{
    return BT_Send_IT(hbt, (const uint8_t *)str, strlen(str));
}

/**
 * @brief  格式化中断发送（非阻塞，使用内部静态缓冲区）
 * @note   下一次调用前必须等上次发送完成（BT_TxReady() == 1）
 */
BT_Status_t BT_Printf_IT(BT_Handle_t *hbt, const char *fmt, ...)
{
    if (hbt == NULL || !hbt->initialized)
        return BT_INVALID;
    if (bt_tx_busy)
        return BT_ERROR;

    va_list args;
    va_start(args, fmt);
    int len = vsnprintf((char *)bt_tx_buf, sizeof(bt_tx_buf), fmt, args);
    va_end(args);
    if (len <= 0)
        return BT_ERROR;

    return BT_Send_IT(hbt, bt_tx_buf, (uint16_t)len);
}

/**
 * @brief  查询发送是否完成
 * @return 1=空闲可发送, 0=发送中
 */
uint8_t BT_TxReady(BT_Handle_t *hbt)
{
    (void)hbt;
    return !bt_tx_busy;
}

/**
 * @brief  从接收缓冲区读取原始数据（非阻塞）
 * @return 实际读取的字节数
 */
uint16_t BT_Read(BT_Handle_t *hbt, uint8_t *buf, uint16_t max_len)
{
    if (hbt == NULL || !hbt->initialized)
        return 0;
    return BT_RingBuf_Read(buf, max_len);
}

/**
 * @brief  从接收缓冲区读取一行（非阻塞）
 * @return 行长度，0 表示当前无完整行
 */
uint16_t BT_ReadLine(BT_Handle_t *hbt, uint8_t *buf, uint16_t max_len)
{
    if (hbt == NULL || !hbt->initialized)
        return 0;
    return BT_RingBuf_ReadLine(buf, max_len);
}

/**
 * @brief  查询接收缓冲区中是否有完整行
 */
uint8_t BT_HasLine(BT_Handle_t *hbt)
{
    (void)hbt;
    return bt_rx_line_flag;
}

/**
 * @brief  查询接收缓冲区中可用字节数
 */
uint16_t BT_Available(BT_Handle_t *hbt)
{
    (void)hbt;
    return BT_RingBuf_Available();
}

/**
 * @brief  阻塞接收指定字节数
 */
BT_Status_t BT_Receive(BT_Handle_t *hbt, uint8_t *buf, uint16_t len,
                        uint32_t timeout_ms)
{
    if (hbt == NULL || !hbt->initialized)
        return BT_INVALID;

    uint32_t start = HAL_GetTick();
    uint16_t received = 0;

    while (received < len) {
        if (bt_rx_tail != bt_rx_head) {
            buf[received++] = bt_rx_buf[bt_rx_tail];
            bt_rx_tail = (bt_rx_tail + 1) % BT_RX_BUF_SIZE;
        }
        if ((HAL_GetTick() - start) >= timeout_ms)
            return BT_TIMEOUT;
    }
    return BT_OK;
}

/**
 * @brief  读取 STATE 引脚，判断蓝牙是否已连接
 * @return 1=已连接, 0=未连接
 */
uint8_t BT_GetState(BT_Handle_t *hbt)
{
    if (hbt == NULL)
        return 0;
    return (HAL_GPIO_ReadPin(hbt->STATE_Port, hbt->STATE_Pin) == GPIO_PIN_SET)
           ? (uint8_t)BT_CONNECTED : (uint8_t)BT_DISCONNECTED;
}
