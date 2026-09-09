#include "Tja1145_Drv.h"
#include "Tc377_SpiHw.h"

#define TJA1145_REG_MODE_CTRL   (0x01u) /* example placeholder */
#define TJA1145_REG_STATUS      (0x10u) /* example placeholder */
#define TJA1145_REG_WAKE_STATUS (0x11u) /* example placeholder */

/*
 * IMPORTANT:
 * Register addresses/bit fields above are intentionally placeholders.
 * Use the exact TJA1145/TJA1145A datasheet revision used by the project.
 */

static Tja1145_ModeType g_mode = TJA1145_MODE_SLEEP;

void Tja1145_Init(void)
{
    Tc377_Spi_Init();
    /* Configure PN filters, CAN bitrate, wake source and interrupts here. */
    g_mode = TJA1145_MODE_STANDBY;
}

void Tja1145_SetNormalMode(void)
{
    /* Tc377_Spi_WriteReg(TJA1145_REG_MODE_CTRL, exact_value); */
    g_mode = TJA1145_MODE_NORMAL;
}

void Tja1145_SetStandbyMode(void)
{
    g_mode = TJA1145_MODE_STANDBY;
}

void Tja1145_SetSleepMode(void)
{
    /* Write exact sleep command through SPI. */
    g_mode = TJA1145_MODE_SLEEP;
}

uint8_t Tja1145_ReadStatus(void)
{
    return Tc377_Spi_ReadReg(TJA1145_REG_STATUS);
}

void Tja1145_ClearWakeStatus(void)
{
    /* Read/clear exact wake source registers according to datasheet. */
    (void)Tc377_Spi_ReadReg(TJA1145_REG_WAKE_STATUS);
}

Tja1145_WakeupSourceType Tja1145_GetWakeupSource(void)
{
    uint8_t status = Tc377_Spi_ReadReg(TJA1145_REG_WAKE_STATUS);

    /* Example decoding only; replace with exact bit definitions. */
    if (status & 0x01u) {
        return TJA1145_WAKE_LOCAL;
    }
    if (status & 0x02u) {
        return TJA1145_WAKE_CAN;
    }
    if (status & 0x04u) {
        return TJA1145_WAKE_SELECTIVE;
    }
    return TJA1145_WAKE_NONE;
}
