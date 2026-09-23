#include "Pedal_Sent.h"

static uint16_t ClampU16(uint32_t x, uint16_t lo, uint16_t hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return (uint16_t)x;
}

static float RawToPct(uint16_t raw, uint16_t zero, uint16_t full)
{
    if (full <= zero) return 0.0f;
    if (raw <= zero) return 0.0f;
    if (raw >= full) return 100.0f;
    return ((float)(raw - zero) * 100.0f) / (float)(full - zero);
}

/*
 * Reference mapping:
 * 3 data nibbles => 12-bit signal.
 * nibbles [0..2] are packed MS nibble first.
 *
 * If the selected HELLA sensor sends a different payload layout,
 * change ONLY this extraction function.
 */
static uint16_t Extract12(const SentFrame_t *f)
{
    if (f->data_count < 3u) return 0u;
    return (uint16_t)(((uint16_t)(f->data[0] & 0x0Fu) << 8u) |
                      ((uint16_t)(f->data[1] & 0x0Fu) << 4u) |
                      ((uint16_t)(f->data[2] & 0x0Fu)));
}

static bool ReadOne(PedalSent_t *ctx, uint8_t ch, uint8_t index, SentFrame_t *f)
{
    if (!ctx->hal.ReadFrame(ch, f)) {
        return false;
    }
    if (!f->valid) {
        return false;
    }
    ctx->last_frame_ms[index] = ctx->hal.GetTimestampMs();
    return true;
}

bool PedalSent_Init(PedalSent_t *ctx,
                    const PedalProfile_t *profile,
                    uint8_t sent_ch1,
                    uint8_t sent_ch2)
{
    if ((ctx == 0) || (profile == 0)) return false;

    ctx->hal = *SentHal_GetApi();
    ctx->profile = *profile;

    if (!ctx->hal.Init(sent_ch1)) return false;
    if (!ctx->hal.Init(sent_ch2)) return false;

    ctx->signal.status = PEDAL_STALE;
    ctx->signal.diag_bits = 0u;
    ctx->initialized = true;
    return true;
}

void PedalSent_Main1ms(PedalSent_t *ctx,
                       uint8_t sent_ch1,
                       uint8_t sent_ch2)
{
    SentFrame_t f1 = {0};
    SentFrame_t f2 = {0};
    uint32_t now;
    bool ok1, ok2;
    uint16_t raw1, raw2;
    float p1, p2, diff;

    if ((ctx == 0) || !ctx->initialized) return;

    now = ctx->hal.GetTimestampMs();
    ok1 = ReadOne(ctx, sent_ch1, 0u, &f1);
    ok2 = ReadOne(ctx, sent_ch2, 1u, &f2);

    ctx->signal.diag_bits = 0u;

    if (!ok1) {
        ctx->signal.diag_bits |= PEDAL_DIAG_S1_CRC | PEDAL_DIAG_S1_TIMEOUT;
    }
    if (!ok2) {
        ctx->signal.diag_bits |= PEDAL_DIAG_S2_CRC | PEDAL_DIAG_S2_TIMEOUT;
    }

    if (ok1) {
        raw1 = ClampU16(Extract12(&f1), ctx->profile.raw_min, ctx->profile.raw_max);
        ctx->signal.raw1 = raw1;
        ctx->signal.status1 = f1.status;
        ctx->last_raw[0] = raw1;
    } else {
        raw1 = ctx->last_raw[0];
    }

    if (ok2) {
        raw2 = ClampU16(Extract12(&f2), ctx->profile.raw_min, ctx->profile.raw_max);
        ctx->signal.raw2 = raw2;
        ctx->signal.status2 = f2.status;
        ctx->last_raw[1] = raw2;
    } else {
        raw2 = ctx->last_raw[1];
    }

    p1 = RawToPct(raw1, ctx->profile.raw_zero, ctx->profile.raw_full);
    p2 = RawToPct(raw2, ctx->profile.raw_zero, ctx->profile.raw_full);

    ctx->signal.position1_pct = p1;
    ctx->signal.position2_pct = p2;

    diff = p1 - p2;
    if (diff < 0.0f) diff = -diff;

    if (diff > (float)ctx->profile.max_plausibility_delta_pct) {
        ctx->signal.diag_bits |= PEDAL_DIAG_IMPLAUSIBLE;
        ctx->signal.status = PEDAL_PLAUSIBILITY;
    } else if (!ok1 || !ok2) {
        ctx->signal.status = PEDAL_TIMEOUT;
    } else {
        ctx->signal.position_pct = (p1 + p2) * 0.5f;
        ctx->signal.demand_pct = ctx->signal.position_pct;
        ctx->signal.status = PEDAL_OK;
    }

    if ((now - ctx->last_frame_ms[0]) > ctx->profile.timeout_ms) {
        ctx->signal.diag_bits |= PEDAL_DIAG_S1_TIMEOUT;
        ctx->signal.status = PEDAL_TIMEOUT;
    }
    if ((now - ctx->last_frame_ms[1]) > ctx->profile.timeout_ms) {
        ctx->signal.diag_bits |= PEDAL_DIAG_S2_TIMEOUT;
        ctx->signal.status = PEDAL_TIMEOUT;
    }

    ctx->signal.timestamp_ms = now;
}

const PedalSignal_t *PedalSent_GetSignal(const PedalSent_t *ctx)
{
    if (ctx == 0) return 0;
    return &ctx->signal;
}
