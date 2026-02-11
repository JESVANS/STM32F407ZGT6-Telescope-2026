#include "usart3task.h"


/* usart3接收内容（供 LCD 任务显示） */
char usart3_rx_display[128] = {0};


void Usart3Task(void *argument)
{
  
  for (;;)
  {
    
    osDelay(500);
  }
}


