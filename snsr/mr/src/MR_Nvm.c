#include "MR_Nvm.h"

#define MR_NVM_MAGIC 0x4D52434Cu

uint32_t MR_NvmCrc32(const uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFFu;
    uint32_t i, j;

    for (i = 0u; i < len; ++i)
    {
        crc ^= data[i];
        for (j = 0u; j < 8u; ++j)
            crc = (crc >> 1u) ^ (0xEDB88320u & (-(int32_t)(crc & 1u)));
    }
    return ~crc;
}

bool MR_NvmValidate(const MR_NvmRecord_t *r)
{
    uint32_t crc;

    if (r->magic != MR_NVM_MAGIC)
        return false;

    crc = MR_NvmCrc32((const uint8_t *)r,
                      (uint32_t)(sizeof(MR_NvmRecord_t) -
                                 sizeof(r->crc)));

    return (crc == r->crc) && (r->version != 0u);
}
