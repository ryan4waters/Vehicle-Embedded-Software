#ifndef MR_NVM_H
#define MR_NVM_H

#include <stdint.h>
#include <stdbool.h>
#include "MR_AngleSensor.h"

typedef struct
{
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
    MR_Calib_t calib;
    uint32_t crc;
} MR_NvmRecord_t;

bool MR_NvmValidate(const MR_NvmRecord_t *r);
uint32_t MR_NvmCrc32(const uint8_t *data, uint32_t len);

#endif
