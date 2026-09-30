#include "Tc377_Flash.h"

Hsm_U8 Tc377_Flash_IsValidRange(Hsm_U32 address, Hsm_U32 size)
{
    /* Replace by project memory map / MCAL Fls adapter. */
    (void)address; (void)size;
    return 1u;
}

Hsm_ReturnType Tc377_Flash_Erase(Hsm_U32 address, Hsm_U32 size)
{
    (void)address; (void)size;
    return HSM_E_NOT_READY;
}

Hsm_ReturnType Tc377_Flash_Write(Hsm_U32 address, const Hsm_U8 *data, Hsm_U32 len)
{
    (void)address; (void)data; (void)len;
    return HSM_E_NOT_READY;
}
