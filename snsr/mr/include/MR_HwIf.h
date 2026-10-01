#ifndef MR_HW_IF_H
#define MR_HW_IF_H

#include <stdint.h>

/* MCU-specific adapter: TC377/F29P32/SPC58NN implementation goes here. */
typedef struct
{
    float (*read_sin)(void);
    float (*read_cos)(void);
    float (*read_supply)(void);
    uint32_t (*get_timestamp_us)(void);
} MR_HwIf_t;

#endif
