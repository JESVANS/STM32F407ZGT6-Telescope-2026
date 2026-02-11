/**
 * @file    sram.h
 * @brief   IS62WV51216BLL-55TLI 外部 SRAM 驱动（FSMC NE3 / PG10）
 *          容量: 512K × 16bit = 1 MB
 *          映射地址: 0x6800_0000 – 0x680F_FFFF
 */
#ifndef __SRAM_H
#define __SRAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* ---- 地址与容量 ---- */
#define SRAM_BASE_ADDR      ((uint32_t)0x68000000)   /* FSMC Bank1 NE3 起始地址 */
#define SRAM_SIZE           (512 * 1024 * 2)          /* 1 MB (512K × 16bit)     */

/* ---- 对外接口 ---- */
void    SRAM_Init(void);
void    SRAM_WriteBuffer(uint16_t *pBuf, uint32_t addr, uint32_t len);
void    SRAM_ReadBuffer(uint16_t *pBuf, uint32_t addr, uint32_t len);
void    SRAM_WriteByte(uint32_t addr, uint8_t data);
uint8_t SRAM_ReadByte(uint32_t addr);
uint8_t SRAM_Test(uint32_t *testedSize);   /* 返回 0 = OK, 1 = FAIL; testedSizeKB 输出实测容量 */

/* SRAM 句柄（若外部需要，例如 DMA） */
extern SRAM_HandleTypeDef hsram3;

#ifdef __cplusplus
}
#endif

#endif /* __SRAM_H */
