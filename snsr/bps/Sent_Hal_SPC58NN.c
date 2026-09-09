#include "Sent_Hal.h"

/*
 * SPC58NN backend boundary.
 * ST provides SENT support/demo material for SPC57xx/SPC58xx families.
 *
 * Replace TODO sections with the exact SPC58NN MCAL/low-level driver used by
 * the project:
 *   - SENT peripheral clock
 *   - pad mux
 *   - SENT channel configuration
 *   - interrupt/DMA reception
 *   - frame queue
 *
 * The application layer remains unchanged.
 */

static bool Spc58Sent_Init(uint8_t channel)
{
    (void)channel;
    /* TODO: initialize SENT receiver channel. */
    return true;
}

static bool Spc58Sent_ReadFrame(uint8_t channel, SentFrame_t *frame)
{
    (void)channel;
    (void)frame;
    /* TODO: pop a completed SENT frame from driver/DMA buffer. */
    return false;
}

static uint32_t Spc58Sent_GetTimestampMs(void)
{
    /* TODO: map to system timer. */
    return 0u;
}

static const SentHalApi_t g_spc58SentApi = {
    Spc58Sent_Init,
    Spc58Sent_ReadFrame,
    Spc58Sent_GetTimestampMs
};

const SentHalApi_t *SentHal_GetApi(void)
{
    return &g_spc58SentApi;
}
