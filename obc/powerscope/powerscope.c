#include "powerscope.h"
#include "powerscope_platform.h"
#include <string.h>
#include <math.h>

typedef struct {
    PS_ChannelConfig cfg;
    float ring[PS_MAX_SAMPLES];
    float prev;
} PS_ChannelRuntime;

static PS_ChannelRuntime g_ch[PS_MAX_CHANNELS];
static uint16_t g_ch_count;
static uint32_t g_sample_tick;
static uint32_t g_total_samples;
static uint16_t g_wr;
static uint16_t g_captured;
static uint16_t g_post_count;
static uint16_t g_pre_samples;
static uint16_t g_post_samples;
static PS_TriggerConfig g_trig;
static PS_State g_state;
static uint16_t g_sequence;

static float ps_read(const PS_ChannelRuntime *c)
{
    float x = 0.0f;
    if (!c->cfg.source) return 0.0f;

    switch (c->cfg.type) {
    case PS_TYPE_FLOAT32: x = *(const volatile float*)c->cfg.source; break;
    case PS_TYPE_INT32:   x = (float)*(const volatile int32_t*)c->cfg.source; break;
    case PS_TYPE_UINT32:  x = (float)*(const volatile uint32_t*)c->cfg.source; break;
    case PS_TYPE_BOOL:    x = (*(const volatile uint8_t*)c->cfg.source) ? 1.0f : 0.0f; break;
    default: break;
    }
    return x * c->cfg.scale + c->cfg.offset;
}

static bool ps_trigger(float prev, float now)
{
    float l = g_trig.level;
    switch (g_trig.mode) {
    case PS_TRIG_RISING:  return (prev < l) && (now >= l);
    case PS_TRIG_FALLING: return (prev > l) && (now <= l);
    case PS_TRIG_ABOVE:   return now >= l;
    case PS_TRIG_BELOW:   return now <= l;
    default: return false;
    }
}

void PowerScope_Init(uint32_t sample_period_us)
{
    (void)sample_period_us;
    memset(g_ch, 0, sizeof(g_ch));
    g_ch_count = 0;
    g_sample_tick = 0;
    g_total_samples = 0;
    g_wr = 0;
    g_captured = 0;
    g_post_count = 0;
    g_state = PS_STATE_IDLE;
    g_sequence = 0;
    memset(&g_trig, 0, sizeof(g_trig));
}

bool PowerScope_AddChannel(uint16_t id, const PS_ChannelConfig *cfg)
{
    if (!cfg || id >= PS_MAX_CHANNELS) return false;
    g_ch[id].cfg = *cfg;
    if (id >= g_ch_count) g_ch_count = id + 1u;
    return true;
}

bool PowerScope_SetChannelEnabled(uint16_t id, bool enable)
{
    if (id >= g_ch_count) return false;
    g_ch[id].cfg.enabled = enable;
    return true;
}

bool PowerScope_ConfigTrigger(const PS_TriggerConfig *cfg)
{
    if (!cfg || cfg->channel >= g_ch_count) return false;
    if (cfg->pre_samples >= PS_MAX_SAMPLES) return false;
    if (cfg->post_samples == 0u || cfg->post_samples >= PS_MAX_SAMPLES) return false;
    g_trig = *cfg;
    g_pre_samples = cfg->pre_samples;
    g_post_samples = cfg->post_samples;
    return true;
}

bool PowerScope_Arm(void)
{
    g_wr = 0;
    g_captured = 0;
    g_post_count = 0;
    g_total_samples = 0;
    g_state = PS_STATE_ARMED;
    for (uint16_t i=0; i<g_ch_count; i++) g_ch[i].prev = ps_read(&g_ch[i]);
    return true;
}

void PowerScope_Stop(void)
{
    g_state = PS_STATE_IDLE;
}

void PowerScope_SampleISR(void)
{
    if (g_state != PS_STATE_ARMED) return;

    float values[PS_MAX_CHANNELS];

    for (uint16_t i=0; i<g_ch_count; i++) {
        values[i] = g_ch[i].cfg.enabled ? ps_read(&g_ch[i]) : 0.0f;
        g_ch[i].ring[g_wr] = values[i];
    }

    bool hit = false;
    if (g_trig.mode != PS_TRIG_NONE && g_trig.channel < g_ch_count) {
        hit = ps_trigger(g_ch[g_trig.channel].prev, values[g_trig.channel]);
        g_ch[g_trig.channel].prev = values[g_trig.channel];
    }

    g_wr++;
    if (g_wr >= PS_MAX_SAMPLES) g_wr = 0;
    g_total_samples++;

    if (g_trig.mode == PS_TRIG_NONE) {
        if (g_total_samples >= PS_MAX_SAMPLES) {
            g_captured = PS_MAX_SAMPLES;
            g_state = PS_STATE_CAPTURED;
        }
        return;
    }

    if (g_post_count == 0u) {
        if (hit && g_total_samples >= g_pre_samples) {
            g_post_count = 1u;
        }
    } else {
        g_post_count++;
        if (g_post_count >= g_post_samples) {
            g_captured = (g_pre_samples + g_post_samples <= PS_MAX_SAMPLES) ?
                         (uint16_t)(g_pre_samples + g_post_samples) : PS_MAX_SAMPLES;
            g_state = PS_STATE_CAPTURED;
        }
    }
}

void PowerScope_Task(void)
{
    /* 后续可放：统计、命令解析、上传调度、压缩等低优先级工作 */
}

bool PowerScope_IsCaptured(void) { return g_state == PS_STATE_CAPTURED; }
PS_State PowerScope_GetState(void) { return g_state; }
uint16_t PowerScope_GetChannelCount(void) { return g_ch_count; }
uint32_t PowerScope_GetCapturedSamples(void) { return g_captured; }

bool PowerScope_GetStatistics(uint16_t channel, PS_Statistics *st)
{
    if (!st || channel >= g_ch_count || g_captured == 0u) return false;

    float mn = 1e30f, mx = -1e30f, sum = 0.0f, sum2 = 0.0f;
    uint16_t n = g_captured;
    uint16_t start = (g_wr + PS_MAX_SAMPLES - g_captured) % PS_MAX_SAMPLES;

    for (uint16_t k=0; k<n; k++) {
        uint16_t p = (start + k) % PS_MAX_SAMPLES;
        float x = g_ch[channel].ring[p];
        if (x < mn) mn = x;
        if (x > mx) mx = x;
        sum += x;
        sum2 += x*x;
    }
    st->min = mn;
    st->max = mx;
    st->avg = sum / (float)n;
    st->rms = sqrtf(sum2 / (float)n);
    st->pk_pk = mx - mn;
    return true;
}

uint16_t PowerScope_BuildFrame(uint8_t *dst, uint16_t max_len)
{
    if (!dst || g_state != PS_STATE_CAPTURED) return 0u;

    /* 简化二进制协议：
       [AA55][SEQ][CH][N][TS][float32 samples...]
       每个sample按channel交错。
    */
    uint16_t header = 12u;
    uint32_t need = header + (uint32_t)g_captured * g_ch_count * 4u;
    if (need > max_len) return 0u;

    uint16_t p = 0;
    dst[p++] = 0xAA; dst[p++] = 0x55;
    memcpy(&dst[p], &g_sequence, 2); p += 2;
    memcpy(&dst[p], &g_ch_count, 2); p += 2;
    memcpy(&dst[p], &g_captured, 2); p += 2;
    uint32_t ts = PowerScope_Platform_GetTimestampUs();
    memcpy(&dst[p], &ts, 4); p += 4;

    uint16_t start = (g_wr + PS_MAX_SAMPLES - g_captured) % PS_MAX_SAMPLES;
    for (uint16_t k=0; k<g_captured; k++) {
        uint16_t idx = (start + k) % PS_MAX_SAMPLES;
        for (uint16_t c=0; c<g_ch_count; c++) {
            memcpy(&dst[p], &g_ch[c].ring[idx], 4);
            p += 4;
        }
    }
    g_sequence++;
    return p;
}
