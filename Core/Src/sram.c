/**
 * @file    sram.c
 * @brief   IS62WV51216BLL-55TLI 外部 SRAM 驱动
 *          FSMC Bank1 NE3 (PG10)，16-bit 数据总线
 *          19 根地址线 A0-A18 → 512K × 16bit = 1 MB
 *
 * 引脚分配 (AF12_FSMC):
 *   数据 D0-D15 : PD14,15,0,1  PE7-15  PD8,9,10  (与 LCD 共用，已在 HAL_SRAM_MspInit 初始化)
 *   地址 A0-A5  : PF0-PF5
 *   地址 A6-A9  : PF12-PF15
 *   地址 A10-A15: PG0-PG5
 *   地址 A16-A18: PD11-PD13
 *   NE3 (CS)    : PG10
 *   NOE (OE)    : PD4         (与 LCD 共用)
 *   NWE (WE)    : PD5         (与 LCD 共用)
 *   NBL0        : PE0         (低字节使能)
 *   NBL1        : PE1         (高字节使能)
 */

#include "sram.h"
#include <string.h>

/* ---- SRAM 句柄 ---- */
SRAM_HandleTypeDef hsram3;

/* ================================================================
 *  GPIO 初始化 —— 仅初始化 SRAM 独占的引脚
 *  数据线 / NOE / NWE 已在 tftlcd.c 的 HAL_SRAM_MspInit 中配置
 * ================================================================ */
static void SRAM_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    /* 使能时钟 */
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_FSMC_CLK_ENABLE();

    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_PULLUP;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF12_FSMC;

    /* ---- 地址线 A0-A5 : PF0-PF5 ---- */
    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
               GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOF, &gpio);

    /* ---- 地址线 A6-A9 : PF12-PF15 ---- */
    gpio.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOF, &gpio);

    /* ---- 地址线 A10-A15 : PG0-PG5 ---- */
    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
               GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOG, &gpio);

    /* ---- 地址线 A16-A18 : PD11-PD13 ---- */
    gpio.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
    HAL_GPIO_Init(GPIOD, &gpio);

    /* ---- NE3 (CS) : PG10 ---- */
    gpio.Pin = GPIO_PIN_10;
    HAL_GPIO_Init(GPIOG, &gpio);

    /* ---- NOE / NWE : PD4 / PD5 (可能已初始化，重复无害) ---- */
    gpio.Pin = GPIO_PIN_4 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOD, &gpio);

    /* ---- NBL0 / NBL1 : PE0 / PE1 ---- */
    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    HAL_GPIO_Init(GPIOE, &gpio);
}

/* ================================================================
 *  FSMC NOR/SRAM 控制器初始化 —— Bank1 NE3
 *
 *  IS62WV51216BLL-55TLI 时序 (HCLK = 168 MHz, THCLK ≈ 6 ns):
 *    tRC  = 55 ns (读周期)  → (ADDSET+1 + DATAST+1) × 6 ≥ 55
 *    tWC  = 55 ns (写周期)
 *    tWP  = 40 ns (写脉宽)  → (DATAST+1) × 6 ≥ 40
 *
 *  取 ADDSET = 1, DATAST = 8:
 *    读/写周期 = (1+1 + 8+1) × 6 = 66 ns > 55 ns  ✓
 *    写脉宽    = (8+1) × 6  = 54 ns > 40 ns         ✓
 * ================================================================ */
void SRAM_Init(void)
{
    FSMC_NORSRAM_TimingTypeDef timing = {0};

    /* 1. 初始化 GPIO */
    SRAM_GPIO_Init();

    /* 2. FSMC 控制参数 */
    hsram3.Instance                 = FSMC_NORSRAM_DEVICE;
    hsram3.Extended                 = FSMC_NORSRAM_EXTENDED_DEVICE;

    hsram3.Init.NSBank              = FSMC_NORSRAM_BANK3;          /* NE3 */
    hsram3.Init.DataAddressMux      = FSMC_DATA_ADDRESS_MUX_DISABLE;
    hsram3.Init.MemoryType          = FSMC_MEMORY_TYPE_SRAM;
    hsram3.Init.MemoryDataWidth     = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
    hsram3.Init.BurstAccessMode     = FSMC_BURST_ACCESS_MODE_DISABLE;
    hsram3.Init.WaitSignalPolarity  = FSMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram3.Init.WaitSignalActive    = FSMC_WAIT_TIMING_BEFORE_WS;
    hsram3.Init.WriteOperation      = FSMC_WRITE_OPERATION_ENABLE;
    hsram3.Init.WaitSignal          = FSMC_WAIT_SIGNAL_DISABLE;
    hsram3.Init.ExtendedMode        = FSMC_EXTENDED_MODE_DISABLE;  /* 读写同时序 */
    hsram3.Init.AsynchronousWait    = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram3.Init.WriteBurst          = FSMC_WRITE_BURST_DISABLE;

    /* 3. 时序参数 */
    timing.AddressSetupTime         = 1;    /* ADDSET  = 1  HCLK */
    timing.AddressHoldTime          = 0;    /* Mode A 不用       */
    timing.DataSetupTime            = 8;    /* DATAST  = 8  HCLK */
    timing.BusTurnAroundDuration    = 0;
    timing.CLKDivision              = 0;
    timing.DataLatency              = 0;
    timing.AccessMode               = FSMC_ACCESS_MODE_A;

    /* 4. 初始化 FSMC（HAL_SRAM_MspInit 由 LCD 管理，这里已提前配 GPIO） */
    HAL_SRAM_Init(&hsram3, &timing, NULL);
}

/* ================================================================
 *  读写接口
 * ================================================================ */

/**
 * @brief  向 SRAM 写入 16-bit 数据块
 * @param  pBuf  源缓冲区
 * @param  addr  SRAM 内偏移（以 16-bit 为单位）
 * @param  len   要写入的 16-bit 字数
 */
void SRAM_WriteBuffer(uint16_t *pBuf, uint32_t addr, uint32_t len)
{
    volatile uint16_t *dst = (volatile uint16_t *)(SRAM_BASE_ADDR + addr * 2);
    for (uint32_t i = 0; i < len; i++)
    {
        dst[i] = pBuf[i];
    }
}

/**
 * @brief  从 SRAM 读取 16-bit 数据块
 * @param  pBuf  目标缓冲区
 * @param  addr  SRAM 内偏移（以 16-bit 为单位）
 * @param  len   要读取的 16-bit 字数
 */
void SRAM_ReadBuffer(uint16_t *pBuf, uint32_t addr, uint32_t len)
{
    volatile uint16_t *src = (volatile uint16_t *)(SRAM_BASE_ADDR + addr * 2);
    for (uint32_t i = 0; i < len; i++)
    {
        pBuf[i] = src[i];
    }
}

/**
 * @brief  写一个字节到 SRAM
 */
void SRAM_WriteByte(uint32_t addr, uint8_t data)
{
    *(volatile uint8_t *)(SRAM_BASE_ADDR + addr) = data;
}

/**
 * @brief  从 SRAM 读一个字节
 */
uint8_t SRAM_ReadByte(uint32_t addr)
{
    return *(volatile uint8_t *)(SRAM_BASE_ADDR + addr);
}

/* ================================================================
 *  SRAM 自检
 *  分三轮测试:
 *    1) 0x5555     2) 0xAAAA     3) 地址 == 数据
 *  每轮写满 → 读回校验，记录实际通过的最大地址
 *  返回 0 = 通过, 1 = 失败
 *  testedSizeKB: 输出实际测试通过的容量 (KB)
 * ================================================================ */
uint8_t SRAM_Test(uint32_t *testedSize)
{
    volatile uint16_t *p = (volatile uint16_t *)SRAM_BASE_ADDR;
    uint32_t words = SRAM_SIZE / 2;   /* 512K 个 16-bit 字 */
    uint32_t passedWords = 0;

    if (testedSize) *testedSize = 0;

    /* ---- 第 1 轮: 0x5555 ---- */
    for (uint32_t i = 0; i < words; i++)
        p[i] = 0x5555;
    for (uint32_t i = 0; i < words; i++)
    {
        if (p[i] != 0x5555) return 1;
    }

    /* ---- 第 2 轮: 0xAAAA ---- */
    for (uint32_t i = 0; i < words; i++)
        p[i] = 0xAAAA;
    for (uint32_t i = 0; i < words; i++)
    {
        if (p[i] != 0xAAAA) return 1;
    }

    /* ---- 第 3 轮: 地址值 ---- */
    for (uint32_t i = 0; i < words; i++)
        p[i] = (uint16_t)(i & 0xFFFF);
    for (uint32_t i = 0; i < words; i++)
    {
        if (p[i] != (uint16_t)(i & 0xFFFF)) return 1;
    }

    /* 全部通过，计算容量 (KB) */
    passedWords = words;
    if (testedSize) *testedSize = (passedWords * 2) / 1024;

    return 0;   /* 全部通过 */
}
