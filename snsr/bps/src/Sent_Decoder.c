#include "Sent_Decoder.h"
#include "Sent_Crc.h"

static uint8_t Sent_TicksToNibble(const SentDecoder_t *d, uint16_t ticks)
{
    uint16_t sync = d->cfg.sync_min_ticks;
    (void)sync;

    /*
     * Conventional SENT nibble timing:
     * total falling-edge period = 12 + nibble value ticks.
     * The exact tick and tolerance are profile-dependent.
     */
    if (ticks < 12u) {
        return 0xFFu;
    }
    ticks = (uint16_t)(ticks - 12u);
    if (ticks > 15u) {
        return 0xFFu;
    }
    return (uint8_t)ticks;
}

void SentDecoder_Init(SentDecoder_t *d, const SentConfig_t *cfg)
{
    uint8_t i;
    d->cfg = *cfg;
    d->state = SENT_STATE_SYNC;
    d->nibble_index = 0u;
    d->last_period_ticks = 0u;
    d->frame_counter = 0u;
    d->error_counter = 0u;
    for (i = 0u; i < 8u; i++) {
        d->nibble_buf[i] = 0u;
    }
}

bool SentDecoder_DecodeFrame(SentDecoder_t *d,
                             const uint16_t *period_ticks,
                             uint8_t period_count,
                             SentFrame_t *out)
{
    uint8_t i;
    uint8_t nibble;
    uint8_t expected;

    if ((period_ticks == 0) || (out == 0) ||
        (period_count < (uint8_t)(2u + d->cfg.expected_data_nibbles))) {
        d->error_counter++;
        d->state = SENT_STATE_ERROR;
        return false;
    }

    /* First period is the synchronization pulse. */
    if ((period_ticks[0] < d->cfg.sync_min_ticks) ||
        (period_ticks[0] > d->cfg.sync_max_ticks)) {
        d->error_counter++;
        d->state = SENT_STATE_ERROR;
        return false;
    }

    d->state = SENT_STATE_STATUS;
    expected = d->cfg.expected_data_nibbles;

    /* period[1] = status nibble in this reference frame model */
    nibble = Sent_TicksToNibble(d, period_ticks[1]);
    if (nibble == 0xFFu) {
        d->error_counter++;
        d->state = SENT_STATE_ERROR;
        return false;
    }
    out->status = nibble;

    for (i = 0u; i < expected; i++) {
        nibble = Sent_TicksToNibble(d, period_ticks[2u + i]);
        if (nibble == 0xFFu) {
            d->error_counter++;
            d->state = SENT_STATE_ERROR;
            return false;
        }
        d->nibble_buf[i] = nibble;
        out->data[i] = nibble;
    }

    out->data_count = expected;
    out->crc = 0u;

    if (d->cfg.check_crc) {
        uint8_t crc_period_index = (uint8_t)(2u + expected);
        if (crc_period_index >= period_count) {
            d->error_counter++;
            d->state = SENT_STATE_ERROR;
            return false;
        }

        nibble = Sent_TicksToNibble(d, period_ticks[crc_period_index]);
        if (nibble == 0xFFu ||
            !Sent_Crc4Check(d->nibble_buf, expected, nibble)) {
            d->error_counter++;
            d->state = SENT_STATE_ERROR;
            return false;
        }
        out->crc = nibble;
    }

    out->valid = true;
    d->frame_counter++;
    d->state = SENT_STATE_VALID;
    return true;
}
