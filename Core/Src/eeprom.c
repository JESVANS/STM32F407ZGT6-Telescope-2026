/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    eeprom.c
  * @brief   AT24C02 EEPROM driver implementation (I2C1)
  *          容量: 2Kbit = 256 Bytes, 页大小 8 Bytes
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "eeprom.h"
#include "i2c.h"
#include <string.h>

/* ---------- 内部辅助 ------------------------------------------------------ */

/**
 * @brief  向 AT24C02 写入一页（不可跨页边界）
 * @param  addr  页内起始地址
 * @param  pBuf  数据
 * @param  len   长度 (≤ AT24C02_PAGE_SIZE, 且不能跨页)
 */
static HAL_StatusTypeDef AT24C02_WritePage(uint8_t addr, const uint8_t *pBuf, uint8_t len)
{
    HAL_StatusTypeDef ret;
    ret = HAL_I2C_Mem_Write(&hi2c1, AT24C02_ADDR, addr,
                            I2C_MEMADD_SIZE_8BIT,
                            (uint8_t *)pBuf, len,
                            AT24C02_TIMEOUT);
    if (ret == HAL_OK)
    {
        HAL_Delay(AT24C02_WRITE_DELAY);   /* 等待 EEPROM 内部写周期完成 */
    }
    return ret;
}

/* ---------- 公开 API ------------------------------------------------------ */

/**
 * @brief  检测 AT24C02 是否在线
 */
HAL_StatusTypeDef AT24C02_IsConnected(void)
{
    return HAL_I2C_IsDeviceReady(&hi2c1, AT24C02_ADDR, 3, AT24C02_TIMEOUT);
}

/**
 * @brief  写入 1 字节
 */
HAL_StatusTypeDef AT24C02_WriteByte(uint8_t addr, uint8_t data)
{
    return AT24C02_WritePage(addr, &data, 1);
}

/**
 * @brief  读取 1 字节
 */
HAL_StatusTypeDef AT24C02_ReadByte(uint8_t addr, uint8_t *data)
{
    return HAL_I2C_Mem_Read(&hi2c1, AT24C02_ADDR, addr,
                            I2C_MEMADD_SIZE_8BIT,
                            data, 1, AT24C02_TIMEOUT);
}

/**
 * @brief  连续写入（自动处理跨页）
 */
HAL_StatusTypeDef AT24C02_Write(uint8_t addr, const uint8_t *pBuf, uint16_t len)
{
    HAL_StatusTypeDef ret;
    uint8_t pageRemain;

    while (len > 0)
    {
        /* 当前页剩余可写字节数 */
        pageRemain = AT24C02_PAGE_SIZE - (addr % AT24C02_PAGE_SIZE);
        if (pageRemain > len)
            pageRemain = (uint8_t)len;

        ret = AT24C02_WritePage(addr, pBuf, pageRemain);
        if (ret != HAL_OK)
            return ret;

        addr  += pageRemain;
        pBuf  += pageRemain;
        len   -= pageRemain;
    }
    return HAL_OK;
}

/**
 * @brief  连续读取
 */
HAL_StatusTypeDef AT24C02_Read(uint8_t addr, uint8_t *pBuf, uint16_t len)
{
    return HAL_I2C_Mem_Read(&hi2c1, AT24C02_ADDR, addr,
                            I2C_MEMADD_SIZE_8BIT,
                            pBuf, len, AT24C02_TIMEOUT);
}

/**
 * @brief  EEPROM 读写自检（逐页遍历全部 256 字节地址空间）
 * @param  testedSize 输出: 实际测试通过的字节数
 * @retval 0 = PASS, 1 = FAIL
 */
uint8_t AT24C02_Test(uint16_t *testedSize)
{
    uint8_t wBuf[AT24C02_PAGE_SIZE];
    uint8_t rBuf[AT24C02_PAGE_SIZE];
    uint16_t passed = 0;

    if (testedSize) *testedSize = 0;

    /* 1. 检测设备在线 */
    if (AT24C02_IsConnected() != HAL_OK)
        return 1;

    /* 2. 逐页写入/回读/校验 */
    for (uint16_t addr = 0; addr < AT24C02_MEM_SIZE; addr += AT24C02_PAGE_SIZE)
    {
        /* 填充测试数据（每页不同模式） */
        for (uint8_t i = 0; i < AT24C02_PAGE_SIZE; i++)
            wBuf[i] = (uint8_t)(addr + i);

        if (AT24C02_Write((uint8_t)addr, wBuf, AT24C02_PAGE_SIZE) != HAL_OK)
            return 1;

        memset(rBuf, 0, AT24C02_PAGE_SIZE);
        if (AT24C02_Read((uint8_t)addr, rBuf, AT24C02_PAGE_SIZE) != HAL_OK)
            return 1;

        if (memcmp(wBuf, rBuf, AT24C02_PAGE_SIZE) != 0)
            return 1;

        passed += AT24C02_PAGE_SIZE;
    }

    if (testedSize) *testedSize = passed;
    return 0;   /* PASS */
}
