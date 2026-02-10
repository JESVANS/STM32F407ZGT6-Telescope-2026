#include "usart2task.h"
#include "zigbee.h"


/* usart2接收内容（供 LCD 任务显示） */
char usart2_rx_display[128] = {0};


void Usart2Task(void *argument)
{
  uint8_t rx_buf[256];
  char msg[256];
  /* 初始化蓝牙（启动中断接收） */
  //BT_Init(&hBluetooth);
  //ZigBee_InitIT();
  osDelay(1000);

  for (;;)
  {
    // BT_Send(&hBluetooth, "HELLO\r\n", 7); 
    // /* ---------- 中断接收：处理蓝牙收到的命令 ---------- */
    // if (bt_has_new) {
    //     BT_Receive(&hBluetooth, rx_buf, sizeof(rx_buf) - 1);
    //     rx_buf[sizeof(rx_buf) - 1] = '\0'; // 确保字符串结尾
    //     strncpy(usart2_rx_display, (char *)rx_buf, sizeof(usart2_rx_display) - 1);
    //     usart2_rx_display[sizeof(usart2_rx_display) - 1] = '\0';
    //     bt_has_new = 0; // 重置标记
    //   }

    // /* === 检查是否有 ZigBee 模块发来的完整帧 === */
    // if (ZigBee_IsRxFrameReady())
    // {
    //     uint8_t raw_frame[128];
    //     uint16_t rlen = ZigBee_GetRxFrame(raw_frame, sizeof(raw_frame));
    //     if (rlen > 0)
    //     {
    //         /* 把原始帧转成十六进制字符串，例如 "FE 05 91 21 00 00 01 FF" */
    //         bytes_to_hex_str(raw_frame, rlen, usart2_rx_display, sizeof(usart2_rx_display));
    //         DL_Packet_t receive_pkg;
    //         ZigBee_Read_data(raw_frame, rlen, &receive_pkg);  // 提取包中实际数据
    //         //lcd_show_string(10, 700, 400, 32, 24, "Received LN33 Packet Data:", MAGENTA);
    //         //lcd_show_xnum(350, 700, receive_pkg.data[0], 5, 24, 1, MAGENTA); //以十进制显示数据
    //     }
    // }

    osDelay(500);
  }
}


