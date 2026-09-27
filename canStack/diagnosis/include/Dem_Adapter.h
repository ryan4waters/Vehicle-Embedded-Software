#ifndef DEM_ADAPTER_H
#define DEM_ADAPTER_H
#include <stdint.h>
typedef enum { DEM_PASSED=0, DEM_PREFAILED, DEM_FAILED, DEM_PREPASSED } Dem_StatusType;
void Dem_SetEventStatus(uint16_t id, Dem_StatusType st); uint8_t Dem_GetEventStatus(uint16_t id); void Dem_MainFunction_10ms(void);
#endif
