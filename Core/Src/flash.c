/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    flash.c
  * @brief   W25Q128JVSIQ SPI Flash driver (SPI3, CS = PB14)
  *          容量: 128Mbit = 16MB, Page 256B, Sector 4KB, Block 64KB
  *          PB14 未经 CubeMX 初始化，在此文件中完成 GPIO 配置
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "flash.h"
#include "spi.h"
#include <string.h>

/* ---------- 内部缓冲 ------------------------------------------------------ */
static uint8_t W25Q128_SectorBuf[W25Q128_SECTOR_SIZE];

/* ========================================================================== */
/*                        底层 SPI 字节读写                                    */
/* ========================================================================== */

/**
 * @brief  通过 SPI3 收发 1 字节
 */
static uint8_t SPI3_ReadWriteByte(uint8_t txData)
{
    uint8_t rxData = 0;
    HAL_SPI_TransmitReceive(&hspi3, &txData, &rxData, 1, W25Q128_TIMEOUT);
    return rxData;
}

/* ========================================================================== */
/*                        CS 引脚 GPIO 初始化                                  */
/* ========================================================================== */

/**
 * @brief  初始化 PB14 为推挽输出，用作 CS
 */
static void W25Q128_CS_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* 默认拉高（片选无效） */
    HAL_GPIO_WritePin(W25Q128_CS_PORT, W25Q128_CS_PIN, GPIO_PIN_SET);

    GPIO_InitStruct.Pin   = W25Q128_CS_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(W25Q128_CS_PORT, &GPIO_InitStruct);
}

/* ========================================================================== */
/*                        内部辅助函数                                         */
/* ========================================================================== */

/**
 * @brief  写使能
 */
static void W25Q128_WriteEnable(void)
{
    W25Q128_CS_LOW();
    SPI3_ReadWriteByte(W25X_WriteEnable);
    W25Q128_CS_HIGH();
}

/**
 * @brief  读 Status Register 1
 */
static uint8_t W25Q128_ReadSR1(void)
{
    uint8_t sr;
    W25Q128_CS_LOW();
    SPI3_ReadWriteByte(W25X_ReadStatusReg1);
    sr = SPI3_ReadWriteByte(0xFF);
    W25Q128_CS_HIGH();
    return sr;
}

/**
 * @brief  等待 BUSY 位清零
 */
static void W25Q128_WaitBusy(void)
{
    while ((W25Q128_ReadSR1() & 0x01) == 0x01)
    {
        /* 可以加小延时以降低 CPU 占用 */
    }
}

/* ========================================================================== */
/*                        公开 API 实现                                        */
/* ========================================================================== */

/**
 * @brief  初始化 W25Q128：配置 CS GPIO 并唤醒芯片
 */
void W25Q128_Init(void)
{
    W25Q128_CS_GPIO_Init();
    W25Q128_CS_HIGH();
    W25Q128_WakeUp();
}

/**
 * @brief  读取 Manufacturer + Device ID (0x90)
 * @return 高8位 Manufacturer(0xEF)，低8位 DeviceID(0x17)
 */
uint16_t W25Q128_ReadID(void)
{
    uint16_t id = 0;
    W25Q128_CS_LOW();
    SPI3_ReadWriteByte(W25X_ManufactDeviceID);
    SPI3_ReadWriteByte(0x00);
    SPI3_ReadWriteByte(0x00);
    SPI3_ReadWriteByte(0x00);
    id  = (uint16_t)SPI3_ReadWriteByte(0xFF) << 8;
    id |= SPI3_ReadWriteByte(0xFF);
    W25Q128_CS_HIGH();
    return id;
}

/**
 * @brief  读取 JEDEC ID (0x9F)
 * @return 0xEF4018 for W25Q128
 */
uint32_t W25Q128_ReadJedecID(void)
{
    uint32_t id = 0;
    W25Q128_CS_LOW();
    SPI3_ReadWriteByte(W25X_JedecDeviceID);
    id  = (uint32_t)SPI3_ReadWriteByte(0xFF) << 16;
    id |= (uint32_t)SPI3_ReadWriteByte(0xFF) << 8;
    id |= SPI3_ReadWriteByte(0xFF);
    W25Q128_CS_HIGH();
    return id;
}

/**
 * @brief  读取数据（任意长度，无需对齐）
 * @param  pBuf  目标缓冲
 * @param  addr  24-bit 起始地址
 * @param  len   读取长度
 */
void W25Q128_Read(uint8_t *pBuf, uint32_t addr, uint32_t len)
{
    W25Q128_CS_LOW();
    SPI3_ReadWriteByte(W25X_ReadData);
    SPI3_ReadWriteByte((uint8_t)(addr >> 16));
    SPI3_ReadWriteByte((uint8_t)(addr >> 8));
    SPI3_ReadWriteByte((uint8_t)(addr));

    for (uint32_t i = 0; i < len; i++)
    {
        pBuf[i] = SPI3_ReadWriteByte(0xFF);
    }
    W25Q128_CS_HIGH();
}

/**
 * @brief  页编程（最多写 256 字节，不可跨页）
 * @param  pBuf  源数据
 * @param  addr  24-bit 起始地址（需页对齐或保证不跨页）
 * @param  len   长度 (≤ 256)
 */
void W25Q128_WritePage(const uint8_t *pBuf, uint32_t addr, uint16_t len)
{
    W25Q128_WriteEnable();

    W25Q128_CS_LOW();
    SPI3_ReadWriteByte(W25X_PageProgram);
    SPI3_ReadWriteByte((uint8_t)(addr >> 16));
    SPI3_ReadWriteByte((uint8_t)(addr >> 8));
    SPI3_ReadWriteByte((uint8_t)(addr));

    for (uint16_t i = 0; i < len; i++)
    {
        SPI3_ReadWriteByte(pBuf[i]);
    }
    W25Q128_CS_HIGH();

    W25Q128_WaitBusy();
}

/**
 * @brief  无检查连续写（跨页自动切换，但不处理擦除）
 */
void W25Q128_WriteNoCheck(const uint8_t *pBuf, uint32_t addr, uint32_t len)
{
    uint16_t pageRemain = W25Q128_PAGE_SIZE - (addr % W25Q128_PAGE_SIZE);
    if (len <= pageRemain)
    {
        pageRemain = (uint16_t)len;
    }

    while (1)
    {
        W25Q128_WritePage(pBuf, addr, pageRemain);

        if (len == pageRemain)
        {
            break;
        }
        else
        {
            pBuf += pageRemain;
            addr += pageRemain;
            len  -= pageRemain;

            if (len > W25Q128_PAGE_SIZE)
                pageRemain = W25Q128_PAGE_SIZE;
            else
                pageRemain = (uint16_t)len;
        }
    }
}

/**
 * @brief  带擦除的智能写入
 *         自动判断目标区域是否需要擦除，仅在必要时擦除扇区
 * @param  pBuf  源数据
 * @param  addr  24-bit 起始地址
 * @param  len   写入长度
 */
void W25Q128_Write(const uint8_t *pBuf, uint32_t addr, uint32_t len)
{
    uint32_t sectorPos   = addr / W25Q128_SECTOR_SIZE;
    uint16_t sectorOff   = addr % W25Q128_SECTOR_SIZE;
    uint16_t sectorRemain = W25Q128_SECTOR_SIZE - sectorOff;

    if (len <= sectorRemain)
        sectorRemain = (uint16_t)len;

    while (1)
    {
        /* 读出整个扇区 */
        W25Q128_Read(W25Q128_SectorBuf, sectorPos * W25Q128_SECTOR_SIZE,
                     W25Q128_SECTOR_SIZE);

        /* 检查是否需要擦除 */
        uint8_t needErase = 0;
        for (uint16_t i = 0; i < sectorRemain; i++)
        {
            if (W25Q128_SectorBuf[sectorOff + i] != 0xFF)
            {
                needErase = 1;
                break;
            }
        }

        if (needErase)
        {
            W25Q128_EraseSector(sectorPos * W25Q128_SECTOR_SIZE);
            /* 合并新数据到 buffer */
            memcpy(W25Q128_SectorBuf + sectorOff, pBuf, sectorRemain);
            /* 回写整个扇区 */
            W25Q128_WriteNoCheck(W25Q128_SectorBuf,
                                 sectorPos * W25Q128_SECTOR_SIZE,
                                 W25Q128_SECTOR_SIZE);
        }
        else
        {
            /* 无需擦除，直接写入 */
            W25Q128_WriteNoCheck(pBuf, addr, sectorRemain);
        }

        if (len == sectorRemain)
            break;

        /* 下一个扇区 */
        sectorPos++;
        sectorOff = 0;
        pBuf += sectorRemain;
        addr += sectorRemain;
        len  -= sectorRemain;

        if (len > W25Q128_SECTOR_SIZE)
            sectorRemain = W25Q128_SECTOR_SIZE;
        else
            sectorRemain = (uint16_t)len;
    }
}

/**
 * @brief  擦除一个扇区 (4KB)
 * @param  sectorAddr  扇区内任意地址（自动对齐到 4KB 边界）
 */
void W25Q128_EraseSector(uint32_t sectorAddr)
{
    sectorAddr = (sectorAddr / W25Q128_SECTOR_SIZE) * W25Q128_SECTOR_SIZE;

    W25Q128_WriteEnable();
    W25Q128_WaitBusy();

    W25Q128_CS_LOW();
    SPI3_ReadWriteByte(W25X_SectorErase);
    SPI3_ReadWriteByte((uint8_t)(sectorAddr >> 16));
    SPI3_ReadWriteByte((uint8_t)(sectorAddr >> 8));
    SPI3_ReadWriteByte((uint8_t)(sectorAddr));
    W25Q128_CS_HIGH();

    W25Q128_WaitBusy();
}

/**
 * @brief  擦除一个块 (64KB)
 */
void W25Q128_EraseBlock(uint32_t blockAddr)
{
    blockAddr = (blockAddr / W25Q128_BLOCK_SIZE) * W25Q128_BLOCK_SIZE;

    W25Q128_WriteEnable();
    W25Q128_WaitBusy();

    W25Q128_CS_LOW();
    SPI3_ReadWriteByte(W25X_BlockErase64K);
    SPI3_ReadWriteByte((uint8_t)(blockAddr >> 16));
    SPI3_ReadWriteByte((uint8_t)(blockAddr >> 8));
    SPI3_ReadWriteByte((uint8_t)(blockAddr));
    W25Q128_CS_HIGH();

    W25Q128_WaitBusy();
}

/**
 * @brief  擦除整片（耗时较长，约 20~100 秒）
 */
void W25Q128_EraseChip(void)
{
    W25Q128_WriteEnable();
    W25Q128_WaitBusy();

    W25Q128_CS_LOW();
    SPI3_ReadWriteByte(W25X_ChipErase);
    W25Q128_CS_HIGH();

    W25Q128_WaitBusy();
}

/**
 * @brief  进入掉电模式
 */
void W25Q128_PowerDown(void)
{
    W25Q128_CS_LOW();
    SPI3_ReadWriteByte(W25X_PowerDown);
    W25Q128_CS_HIGH();
    /* tDP max 3us */
}

/**
 * @brief  唤醒（释放掉电模式）
 */
void W25Q128_WakeUp(void)
{
    W25Q128_CS_LOW();
    SPI3_ReadWriteByte(W25X_ReleasePowerDown);
    W25Q128_CS_HIGH();
    /* tRES1 max 3us */
}

/* ========================================================================== */
/*                        自检函数                                             */
/* ========================================================================== */

/**
 * @brief  W25Q128 读写自检
 *         1. 读 JEDEC ID 并据此推算芯片容量
 *         2. 擦除第 0 扇区 → 写入 256 字节 → 回读比较
 * @param  detectedSizeKB 输出: 从 JEDEC ID 推算的容量 (KB)
 * @return 0 = PASS, 1 = ID 错误, 2 = 读写校验失败
 */
uint8_t W25Q128_Test(uint32_t *detectedSize)
{
    uint8_t txBuf[256];
    uint8_t rxBuf[256];

    if (detectedSize) *detectedSize = 0;

    /* ---- 1. JEDEC ID → 推算容量 ---- */
    uint32_t jedecId = W25Q128_ReadJedecID();
    uint8_t  mfr     = (jedecId >> 16) & 0xFF;
    uint8_t  capCode = jedecId & 0xFF;   /* 0x18 → 2^24 = 16MB */

    if (mfr != W25Q128_MANUFACTURER_ID || capCode < 0x11 || capCode > 0x21)
    {
        return 1;   /* 非 Winbond 或容量码异常 */
    }

    /* 容量 = 2^capCode 字节, 转 KB */
    uint32_t sizeKB = (1UL << capCode) / 1024;
    if (detectedSize) *detectedSize = sizeKB;

    /* ---- 2. 填充测试数据 ---- */
    for (uint16_t i = 0; i < 256; i++)
    {
        txBuf[i] = (uint8_t)i;
    }

    /* ---- 3. 擦除 → 写入 → 回读 ---- */
    W25Q128_EraseSector(0);
    W25Q128_Write(txBuf, 0, 256);
    W25Q128_Read(rxBuf, 0, 256);

    /* ---- 4. 比较 ---- */
    if (memcmp(txBuf, rxBuf, 256) != 0)
    {
        return 2;   /* 数据不匹配 */
    }

    return 0;   /* PASS */
}
