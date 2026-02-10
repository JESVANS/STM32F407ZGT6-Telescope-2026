#include "bluetooth.h"
#include "usart.h"
#include <stdint.h>
#include <string.h>
#include <sys/_intsup.h>

/* 默认实例 */
BT_Handle_t hBluetooth = {
    .huart       = BT_DEFAULT_UART,
    .EN_Port     = BT_DEFAULT_EN_PORT,
    .EN_Pin      = BT_DEFAULT_EN_PIN,
    .STATE_Port  = BT_DEFAULT_STATE_PORT,
    .STATE_Pin   = BT_DEFAULT_STATE_PIN,
    .baudrate    = BT_DEFAULT_BAUDRATE,
    .initialized = 0
};

char bt_line_buf[128] = {0};
uint8_t bt_line_idx = 0;
char received[128] = {0};
uint8_t bt_has_new = 0;

/* 单字节中断接收暂存 */
static uint8_t rx_byte_it;

/* 内部帮助：若传入 NULL 使用默认实例 */
static inline BT_Handle_t* _handle(BT_Handle_t *hbt) {
    return hbt ? hbt : &hBluetooth;
}

BT_Status_t BT_Init(BT_Handle_t *hbt_in)
{
    BT_Handle_t *hbt = _handle(hbt_in);
    if (!hbt || !hbt->huart) return BT_INVALID;

    /* EN 输出：默认低（透传模式） */
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = hbt->EN_Pin;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(hbt->EN_Port, &gpio);
    HAL_GPIO_WritePin(hbt->EN_Port, hbt->EN_Pin, GPIO_PIN_RESET);

    /* STATE 输入 */
    gpio.Pin  = hbt->STATE_Pin;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(hbt->STATE_Port, &gpio);

    hbt->initialized = 1;
    //开启接收中断
    if (BT_DEFAULT_UART != NULL) {
        HAL_UART_Receive_IT(BT_DEFAULT_UART, &rx_byte_it, 1);
    }
    device_on_uart2 = 2; // 标记 UART2 连接了 Bluetooth
    return BT_OK;
}

void Bluetooth_UART_RxCpltCallback(void)
{

        extern uint8_t usart2_has_new;

        if (rx_byte_it == '\r') {
            /* 忽略裸回车 */
        } else if (rx_byte_it == '\n') {
            /* 一行结束，拷贝到接收缓冲 */
            bt_line_buf[bt_line_idx] = '\0';
            strncpy(received, bt_line_buf, sizeof(received) - 1);
            received[sizeof(received) - 1] = '\0';
            bt_line_idx = 0;  // 清空，准备下一行
            bt_has_new = 1; // 标记有新数据
        } else {
            /* 普通字符，累加到当前行（防溢出） */
            if (bt_line_idx < sizeof(bt_line_buf) - 1) {
                bt_line_buf[bt_line_idx++] = (char)rx_byte_it;
            }
        }
    HAL_UART_Receive_IT(BT_DEFAULT_UART, &rx_byte_it, 1);
}

BT_Status_t BT_Send(BT_Handle_t *hbt_in, const uint8_t *data, uint16_t len)
{
    BT_Handle_t *hbt = _handle(hbt_in);
    if (!hbt || !hbt->initialized || !data || !len) return BT_INVALID;

    /* 仅在透传模式 (EN 低) 允许发送 */
    if (HAL_GPIO_ReadPin(hbt->EN_Port, hbt->EN_Pin) == GPIO_PIN_SET)
        return BT_ERROR;

    /* 中断方式发送 */
    if (HAL_UART_Transmit_IT(hbt->huart, data, len) != HAL_OK)
        return BT_ERROR;


    return BT_OK;
}

BT_Status_t BT_Receive(BT_Handle_t *hbt_in, uint8_t *buf, uint16_t len)
{
    memcpy(buf, received, sizeof(received) < len ? sizeof(received) : len);
    return BT_OK;
}

