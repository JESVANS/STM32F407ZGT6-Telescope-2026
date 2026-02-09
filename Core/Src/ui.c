#include "ui.h"

/**
 * @brief  LCD 显示任务：初始化屏幕并周期刷新显示内容
 */
void LcdDisplayTask(void *argument)
{
  lcd_init();
  lcd_clear(WHITE);
  lcd_show_string(10, 210, 460, 32, 32, "Telescope System", BLUE);
  lcd_show_string(10, 250, 460, 24, 24, "System Running...", BLACK);
  lcd_show_image(0, 0, gImage_xueyuan);
  lcd_show_image(0, 623, gImage_school);
  uint32_t tick_count = 0;

  for (;;)
  {
    /* 每1000ms 刷新一次运行计数 */
    tick_count++;
    lcd_show_string(10, 290, 200, 24, 24, "Tick:", DARKBLUE);
    lcd_show_num(90, 290, tick_count, 8, 24, RED);

    lcd_show_xnum(150, 100, T1, 2, 32, 0, BLACK);
    lcd_show_xnum(200, 100, T2, 2, 32, 0, BLACK);
    lcd_show_xnum(150, 150, H, 2, 32, 0, BLACK);
    lcd_show_xnum(170, 200, P, 5, 32, 0, BLACK);
    lcd_show_xnum(150, 250, A, 3, 32, 0, BLACK);
    lcd_show_xnum(170, 300, L, 5, 32, 0, BLACK);
    lcd_show_xnum(150, 350, D, 3, 32, 0, BLACK);

    /* 显示usart2接收内容 */
    if (usart2_rx_display[0] != '\0') {
      lcd_show_string(10, 500, 460, 24, 24, usart2_rx_display, BLACK);
    }

    osDelay(1000);
  }
}