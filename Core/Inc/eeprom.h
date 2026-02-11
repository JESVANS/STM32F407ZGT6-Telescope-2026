/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    eeprom.h
  * @brief   AT24C02 EEPROM driver header (I2C1, 2Kbit = 256 Bytes)
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __EEPROM_H__
#define __EEPROM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* ---------- AT24C02 参数 -------------------------------------------------- */
#define AT24C02_ADDR          0xA0      /* 7-bit 地址左移1位: 1010 000x        */
#define AT24C02_PAGE_SIZE     8         /* AT24C02 每页 8 字节                  */
#define AT24C02_MEM_SIZE      256       /* 总容量 256 字节                      */
#define AT24C02_TIMEOUT       100       /* HAL 超时 (ms)                        */
#define AT24C02_WRITE_DELAY   5         /* 页写周期 tWR ≤ 5 ms                  */

/* ---------- API ----------------------------------------------------------- */

/**
 * @brief  检测 AT24C02 是否在线
 * @retval HAL_OK = 在线
 */
HAL_StatusTypeDef AT24C02_IsConnected(void);

/**
 * @brief  向指定地址写入 1 字节
 * @param  addr  内部地址 0-255
 * @param  data  要写入的字节
 * @retval HAL_OK = 成功
 */
HAL_StatusTypeDef AT24C02_WriteByte(uint8_t addr, uint8_t data);

/**
 * @brief  从指定地址读取 1 字节
 * @param  addr  内部地址 0-255
 * @param  data  读出数据指针
 * @retval HAL_OK = 成功
 */
HAL_StatusTypeDef AT24C02_ReadByte(uint8_t addr, uint8_t *data);

/**
 * @brief  页写入（自动跨页）
 * @param  addr  起始地址 0-255
 * @param  pBuf  数据缓冲区
 * @param  len   数据长度 (不超过剩余空间)
 * @retval HAL_OK = 成功
 */
HAL_StatusTypeDef AT24C02_Write(uint8_t addr, const uint8_t *pBuf, uint16_t len);

/**
 * @brief  连续读取
 * @param  addr  起始地址 0-255
 * @param  pBuf  数据缓冲区
 * @param  len   数据长度
 * @retval HAL_OK = 成功
 */
HAL_StatusTypeDef AT24C02_Read(uint8_t addr, uint8_t *pBuf, uint16_t len);

/**
 * @brief  EEPROM 读写自检（逐页遍历全部地址空间）
 * @param  testedSize 输出: 实际测试通过的字节数
 * @retval 0 = PASS, 1 = FAIL
 */
uint8_t AT24C02_Test(uint16_t *testedSize);

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_H__ */
