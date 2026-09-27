#ifndef TC377_QSPI_HW_H
#define TC377_QSPI_HW_H
#include <stdint.h>
void Tc377_Qspi_Init(void); uint8_t Tc377_Qspi_Read8(uint8_t reg); void Tc377_Qspi_Write8(uint8_t reg,uint8_t value);
#endif
