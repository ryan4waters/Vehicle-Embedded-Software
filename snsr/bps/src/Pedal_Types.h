#ifndef PEDAL_TYPES_H
#define PEDAL_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    PEDAL_OK = 0,
    PEDAL_STALE,
    PEDAL_SENSOR1_INVALID,
    PEDAL_SENSOR2_INVALID,
    PEDAL_PLAUSIBILITY,
    PEDAL_RANGE,
    PEDAL_TIMEOUT,
    PEDAL_CRC,
    PEDAL_PROTOCOL,
    PEDAL_INTERNAL
} PedalStatus_e;

typedef enum {
    SENT_STATE_UNINIT = 0,
    SENT_STATE_SYNC,
    SENT_STATE_STATUS,
    SENT_STATE_DATA,
    SENT_STATE_CRC,
    SENT_STATE_VALID,
    SENT_STATE_ERROR
} SentState_e;

typedef struct {
    uint8_t status;
    uint8_t data[6];
    uint8_t data_count;
    uint8_t crc;
    uint16_t ticks[8];
    uint32_t timestamp_us;
    bool valid;
} SentFrame_t;

typedef struct {
    uint32_t tick_ns;
    uint16_t sync_min_ticks;
    uint16_t sync_max_ticks;
    uint16_t nibble_min_ticks;
    uint16_t nibble_max_ticks;
    uint8_t expected_data_nibbles;
    bool check_crc;
} SentConfig_t;

typedef struct {
    uint16_t raw1;
    uint16_t raw2;
    float position1_pct;
    float position2_pct;
    float position_pct;
    float demand_pct;
    uint8_t status1;
    uint8_t status2;
    uint32_t timestamp_ms;
    PedalStatus_e status;
    uint32_t diag_bits;
} PedalSignal_t;

typedef struct {
    uint16_t raw_min;
    uint16_t raw_max;
    uint16_t raw_zero;
    uint16_t raw_full;
    uint8_t max_plausibility_delta_pct;
    uint16_t timeout_ms;
    uint16_t debounce_ms;
} PedalProfile_t;

#define PEDAL_DIAG_S1_CRC        (1u << 0)
#define PEDAL_DIAG_S2_CRC        (1u << 1)
#define PEDAL_DIAG_S1_TIMEOUT    (1u << 2)
#define PEDAL_DIAG_S2_TIMEOUT    (1u << 3)
#define PEDAL_DIAG_S1_RANGE      (1u << 4)
#define PEDAL_DIAG_S2_RANGE      (1u << 5)
#define PEDAL_DIAG_IMPLAUSIBLE   (1u << 6)
#define PEDAL_DIAG_FRAME         (1u << 7)
#define PEDAL_DIAG_SIGNAL_STALE  (1u << 8)

#endif
