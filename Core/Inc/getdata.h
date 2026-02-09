#ifndef __GETDATA_H
#define __GETDATA_H

#include "cmsis_os.h"
#include "bmp280.h"
#include "sht30.h"
#include "bh1750.h"
#include "hc_sr04.h"


extern float T1;   //from bmp280 (℃)
extern float T2;   //from sht30 (℃)
extern float H;    //Humidity 湿度 (%RH)
extern float L;    //Light 光照 (lux)
extern float P;    //Pressure 气压 (Pa)
extern float A;    //Altitude 海拔 (m)
extern float D;    //Distance 距离 (cm)

void GetDataTask(void *argument);

#endif
