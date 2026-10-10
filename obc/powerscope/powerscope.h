#ifndef POWERSCOPE_H
#define POWERSCOPE_H

#include <stdint.h>
#include <stdbool.h>

#define PS_MAX_CHANNELS       32u
#define PS_MAX_SAMPLES        4096u
#define PS_NAME_LEN           24u

typedef enum {
    PS_TYPE_FLOAT32 = 0,
    PS_TYPE_INT32,
    PS_TYPE_UINT32,
    PS_TYPE_BOOL
} PS_DataType;

typedef enum {
    PS_TRIG_NONE = 0,
    PS_TRIG_RISING,
    PS_TRIG_FALLING,
    PS_TRIG_ABOVE,
    PS_TRIG_BELOW
} PS_TriggerMode;

typedef enum {
    PS_STATE_IDLE = 0,
    PS_STATE_ARMED,
    PS_STATE_CAPTURED
} PS_State;

typedef struct {
    const char *name;
    PS_DataType type;
    float scale;
    float offset;
    volatile const void *source;
    bool enabled;
} PS_ChannelConfig;

typedef struct {
    uint16_t channel;
    PS_TriggerMode mode;
    float level;
    uint16_t pre_samples;
    uint16_t post_samples;
    bool single_shot;
} PS_TriggerConfig;

typedef struct {
    float min;
    float max;
    float avg;
    float rms;
    float pk_pk;
} PS_Statistics;

typedef struct {
    uint32_t timestamp;
    uint16_t sequence;
    uint16_t sample_count;
    uint16_t channel_count;
    uint8_t payload[PS_MAX_SAMPLES * 4u];
} PS_Frame;

void PowerScope_Init(uint32_t sample_period_us);
bool PowerScope_AddChannel(uint16_t id, const PS_ChannelConfig *cfg);
bool PowerScope_SetChannelEnabled(uint16_t id, bool enable);
bool PowerScope_ConfigTrigger(const PS_TriggerConfig *cfg);
bool PowerScope_Arm(void);
void PowerScope_Stop(void);
void PowerScope_SampleISR(void);
void PowerScope_Task(void);
bool PowerScope_IsCaptured(void);
PS_State PowerScope_GetState(void);
uint16_t PowerScope_GetChannelCount(void);
bool PowerScope_GetStatistics(uint16_t channel, PS_Statistics *st);
uint32_t PowerScope_GetCapturedSamples(void);

/* 将当前采集结果打包成传输帧；返回实际字节数 */
uint16_t PowerScope_BuildFrame(uint8_t *dst, uint16_t max_len);

#endif
