#include "getdata.h"

float T1 = 0;
float T2 = 0;
float H  = 0;
float L  = 0;
float P  = 0;
float A  = 0;
float D  = 0;


void GetDataTask(void *argument)
{
  /* USER CODE BEGIN GetDataTask */

  BMP280_Init();
  if(BMP280_Init() != BMP280_OK){
    // 初始化失败处理
  }
  
  if(!SHT30_Check())
  {
    // 初始化失败处理
  }
    
  BH1750_PowerOn();
  BH1750_Reset();
  if(BH1750_SetMode(0x21) != BH1750_OK)
  {
    // 初始化失败处理
  }

  HC_SR04_Init(GPIOG, GPIO_PIN_6, GPIOG, GPIO_PIN_7, GPIO_NOPULL);
  HC_SR04_SetTimeoutUs(30000);  //测量超时(30ms) 

  /* Infinite loop */
  for(;;)
  {
    // 在此处添加获取数据的代码
    BMP280_ReadTempPressure(&T1 , &P);
    A = (int)BMP280_CalcAltitude(P, 101325.0f);
    SHT30_Read_SingleShot(&T2, &H);
    BH1750_ReadLux(&L);
    HC_SR04_Measure(NULL,&D);
    
    osDelay(1000);
  }
  /* USER CODE END GetDataTask */
}

