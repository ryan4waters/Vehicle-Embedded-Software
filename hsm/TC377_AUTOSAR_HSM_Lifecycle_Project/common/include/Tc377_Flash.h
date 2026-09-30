#ifndef TC377_FLASH_H
#define TC377_FLASH_H
#include "Hsm_Types.h"

Hsm_U8 Tc377_Flash_IsValidRange(Hsm_U32 address, Hsm_U32 size);
Hsm_ReturnType Tc377_Flash_Erase(Hsm_U32 address, Hsm_U32 size);
Hsm_ReturnType Tc377_Flash_Write(Hsm_U32 address, const Hsm_U8 *data, Hsm_U32 len);

#endif
