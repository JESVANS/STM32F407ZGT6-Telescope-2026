#ifndef __UI_H
#define __UI_H

#include "cmsis_os.h"
#include "tftlcd.h"
#include "image.h"
#include "getdata.h"
#include "usart2task.h"

void LcdDisplayTask(void *argument);

#endif
