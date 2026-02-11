#ifndef __USART3TASK_H
#define __USART3TASK_H

#include "cmsis_os2.h"
#include "wifi.h"
#include "string.h"

extern char usart3_rx_display[128];

void Usart3Task(void *argument);

#endif
