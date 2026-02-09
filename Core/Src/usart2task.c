#include "usart2task.h"


/* usart2接收内容（供 LCD 任务显示） */
char usart2_rx_display[128] = {0};


void Usart2Task(void *argument)
{
  uint8_t rx_buf[256];
  char msg[256];
  /* 初始化蓝牙（启动中断接收） */
  BT_Init(&hBluetooth);

  for (;;)
  {
    BT_SendString_IT(&hBluetooth, "AT+BAUD\r\n");
    /* ---------- 中断接收：处理蓝牙收到的命令 ---------- */
    if (BT_HasLine(&hBluetooth)) {
      uint16_t len = BT_ReadLine(&hBluetooth, rx_buf, sizeof(rx_buf));
      if (len > 0) {
        /* 回显收到的命令 */
        BT_Printf_IT(&hBluetooth, "Received:%s\r\n", (char *)rx_buf);
        /* 将收到的信息拷贝到显示缓冲区，供 LCD 任务刷新 */
        strncpy(usart2_rx_display, (char *)rx_buf, sizeof(usart2_rx_display) - 1);
        usart2_rx_display[sizeof(usart2_rx_display) - 1] = '\0';
      }
    }
    osDelay(500);
  }
}