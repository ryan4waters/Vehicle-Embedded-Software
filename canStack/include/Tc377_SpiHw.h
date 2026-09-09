#ifndef TC377_SPI_HW_H
#define TC377_SPI_HW_H

#include <stdint.h>

void Tc377_Spi_Init(void);
uint8_t Tc377_Spi_ReadReg(uint8_t reg);
void Tc377_Spi_WriteReg(uint8_t reg, uint8_t value);

#endif
