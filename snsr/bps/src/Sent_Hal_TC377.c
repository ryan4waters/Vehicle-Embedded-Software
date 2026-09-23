#include "Sent_Hal.h"

/*
 * TC377 backend boundary.
 * TC3xx contains a native SENT peripheral. The exact SENT channel/pin mapping
 * must be selected from the TC377 device datasheet and the project's iLLD/MCAL.
 *
 * Replace the TODO sections with:
 *   - module clock enable
 *   - port alternate-function configuration
 *   - SENT channel timing configuration
 *   - interrupt/DMA result handling
 *
 * Do not copy arbitrary pin numbers between TC377 package variants.
 */

static bool Tc377Sent_Init(uint8_t channel)
{
    (void)channel;
    /* TODO: configure SENTx channel using project iLLD/MCAL. */
    return true;
}

static bool Tc377Sent_ReadFrame(uint8_t channel, SentFrame_t *frame)
{
    (void)channel;
    (void)frame;
    /*
     * TODO: read decoded status/data/CRC from the configured SENT peripheral.
     * Recommended: ISR/DMA fills a software frame queue; this API only pops
     * a completed frame.
     */
    return false;
}

static uint32_t Tc377Sent_GetTimestampMs(void)
{
    /* TODO: map to STM/system tick. */
    return 0u;
}

static const SentHalApi_t g_tc377SentApi = {
    Tc377Sent_Init,
    Tc377Sent_ReadFrame,
    Tc377Sent_GetTimestampMs
};

const SentHalApi_t *SentHal_GetApi(void)
{
    return &g_tc377SentApi;
}
