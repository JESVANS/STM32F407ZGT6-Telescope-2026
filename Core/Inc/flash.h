/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    flash.h
  * @brief   W25Q128JVSIQ SPI Flash driver header (SPI3, CS = PB14)
  *          容量: 128Mbit = 16MB
  *          Page  : 256 Bytes
  *          Sector: 4KB
  *          Block : 64KB
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __FLASH_H
#define __FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* ---------- 芯片常量 ------------------------------------------------------ */
#define W25Q128_MANUFACTURER_ID    0xEF        /* Winbond */
#define W25Q128_DEVICE_ID          0x4018      /* W25Q128 */
#define W25Q128_JEDEC_ID           0xEF4018

#define W25Q128_PAGE_SIZE          256
#define W25Q128_SECTOR_SIZE        4096        /* 4KB  */
#define W25Q128_BLOCK_SIZE         65536       /* 64KB */
#define W25Q128_TOTAL_SIZE         (16 * 1024 * 1024)  /* 16MB */
#define W25Q128_SECTOR_COUNT       (W25Q128_TOTAL_SIZE / W25Q128_SECTOR_SIZE)

/* ---------- SPI Flash 指令集 ----------------------------------------------- */
#define W25X_WriteEnable           0x06
#define W25X_WriteDisable          0x04
#define W25X_ReadStatusReg1        0x05
#define W25X_ReadStatusReg2        0x35
#define W25X_WriteStatusReg        0x01
#define W25X_ReadData              0x03
#define W25X_FastReadData          0x0B
#define W25X_PageProgram           0x02
#define W25X_SectorErase           0x20
#define W25X_BlockErase32K         0x52
#define W25X_BlockErase64K         0xD8
#define W25X_ChipErase             0xC7
#define W25X_PowerDown             0xB9
#define W25X_ReleasePowerDown      0xAB
#define W25X_ManufactDeviceID      0x90
#define W25X_JedecDeviceID         0x9F
#define W25X_EnableReset           0x66
#define W25X_Reset                 0x99

/* ---------- 超时 ---------------------------------------------------------- */
#define W25Q128_TIMEOUT            1000       /* ms */

/* ---------- CS 引脚 ------------------------------------------------------- */
#define W25Q128_CS_PORT            GPIOB
#define W25Q128_CS_PIN             GPIO_PIN_14

#define W25Q128_CS_LOW()           HAL_GPIO_WritePin(W25Q128_CS_PORT, W25Q128_CS_PIN, GPIO_PIN_RESET)
#define W25Q128_CS_HIGH()          HAL_GPIO_WritePin(W25Q128_CS_PORT, W25Q128_CS_PIN, GPIO_PIN_SET)

/* ---------- 公开 API ------------------------------------------------------ */
void     W25Q128_Init(void);
uint16_t W25Q128_ReadID(void);
uint32_t W25Q128_ReadJedecID(void);

void     W25Q128_Read(uint8_t *pBuf, uint32_t addr, uint32_t len);
void     W25Q128_WritePage(const uint8_t *pBuf, uint32_t addr, uint16_t len);
void     W25Q128_WriteNoCheck(const uint8_t *pBuf, uint32_t addr, uint32_t len);
void     W25Q128_Write(const uint8_t *pBuf, uint32_t addr, uint32_t len);

void     W25Q128_EraseSector(uint32_t sectorAddr);
void     W25Q128_EraseBlock(uint32_t blockAddr);
void     W25Q128_EraseChip(void);

void     W25Q128_PowerDown(void);
void     W25Q128_WakeUp(void);

uint8_t  W25Q128_Test(uint32_t *detectedSize);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_H */
