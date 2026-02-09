#ifndef __USART2TASK_H
#define __USART2TASK_H

#include "cmsis_os.h"
#include "getdata.h"
#include "bluetooth.h"
#include "string.h"

extern char usart2_rx_display[128];

void Usart2Task(void *argument);

#endif
