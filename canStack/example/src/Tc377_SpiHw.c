#include "Tc377_SpiHw.h"

void Tc377_Spi_Init(void)
{
    /* Configure TC377 QSPI + PORT + chip select according to board. */
}

uint8_t Tc377_Spi_ReadReg(uint8_t reg)
{
    (void)reg;
    /* Real project: QSPI transfer to TJA1145. */
    return 0u;
}

void Tc377_Spi_WriteReg(uint8_t reg, uint8_t value)
{
    (void)reg;
    (void)value;
    /* Real project: QSPI transfer to TJA1145. */
}
