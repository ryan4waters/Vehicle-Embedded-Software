#include "fault_manage.h"
#include <string.h>
#include <stdlib.h>

#define MAX_FAULTS          32      /* 最大支持故障数量 */
#define INVALID_FAULT_ID    (-1)

/* 运行时数据结构 */
typedef struct {
    fault_config_t config;          /* 配置副本 */
    fault_state_t state;            /* 当前状态 */
    uint32_t fault_cont;            /* 连续型：连续故障计数 */
    uint32_t normal_cont;           /* 连续型：连续正常计数 */
    uint32_t fault_accum;           /* 累积型：故障累积计数器 */
    uint32_t occurrence_count;      /* 故障发生总次数（用于锁存） */
    bool latched;                   /* 是否已锁存 */
    bool used;                      /* 槽位是否已使用 */
} fault_runtime_t;

static fault_runtime_t fault_table[MAX_FAULTS];

void fault_manager_init(void)
{
    memset(fault_table, 0, sizeof(fault_table));
}

int fault_register(const fault_config_t *config)
{
    if (config == NULL || config->sample_cb == NULL) {
        return INVALID_FAULT_ID;
    }

    /* 查找空闲槽位 */
    for (int i = 0; i < MAX_FAULTS; i++) {
        if (!fault_table[i].used) {
            fault_runtime_t *fault = &fault_table[i];
            memset(fault, 0, sizeof(*fault));
            fault->config = *config;
            fault->state = FAULT_STATE_INIT;
            fault->used = true;
            return i;   /* 返回索引作为故障 ID */
        }
    }
    return INVALID_FAULT_ID;    /* 无空闲槽位 */
}

void fault_unregister(int fault_id)
{
    if (fault_id >= 0 && fault_id < MAX_FAULTS) {
        fault_table[fault_id].used = false;
    }
}

void fault_manager_process(void)
{
    for (int i = 0; i < MAX_FAULTS; i++) {
        fault_runtime_t *fault = &fault_table[i];
        if (!fault->used || fault->latched) {
            continue;   /* 未使用或已锁存，跳过 */
        }

        fault_raw_state_t raw;
        fault->config.sample_cb(&raw);    /* 调用外部回调获取原始状态 */

        if (raw == FAULT_RAW_INVALID) {
            continue;   /* 无效采样，忽略 */
        }

        fault_type_t type = fault->config.type;
        uint16_t trigger = fault->config.trigger_threshold;
        uint16_t recover = fault->config.recover_threshold;

        if (type == FAULT_TYPE_CONTINUOUS) {
            /* 连续型处理 */
            if (raw == FAULT_RAW_FAULT) {
                fault->fault_cont++;
                fault->normal_cont = 0;
            } else { /* FAULT_RAW_NORMAL */
                fault->normal_cont++;
                fault->fault_cont = 0;
            }

            if (fault->state == FAULT_STATE_INIT || fault->state == FAULT_STATE_NORMAL) {
                if (fault->fault_cont >= trigger) {
                    fault->state = FAULT_STATE_FAULT;
                    fault->occurrence_count++;
                    if (fault->config.latch_count > 0 &&
                        fault->occurrence_count >= fault->config.latch_count) {
                        fault->latched = true;
                    }
                    fault->fault_cont = 0;  /* 重置连续故障计数，避免重复触发 */
                } else if (fault->normal_cont >= recover) {
                    fault->state = FAULT_STATE_NORMAL;
                    fault->normal_cont = 0;
                }
            } else if (fault->state == FAULT_STATE_FAULT) {
                if (fault->normal_cont >= recover) {
                    fault->state = FAULT_STATE_NORMAL;
                    fault->normal_cont = 0;
                }
            }
        } else { /* FAULT_TYPE_ACCUMULATIVE */
            /* 累积型处理：增减计数器 */
            if (raw == FAULT_RAW_FAULT) {
                if (fault->fault_accum < trigger) {
                    fault->fault_accum++;
                }
            } else { /* FAULT_RAW_NORMAL */
                if (fault->fault_accum > 0) {
                    fault->fault_accum--;
                }
            }

            if (fault->state == FAULT_STATE_INIT || fault->state == FAULT_STATE_NORMAL) {
                if (fault->fault_accum >= trigger) {
                    fault->state = FAULT_STATE_FAULT;
                    fault->occurrence_count++;
                    if (fault->config.latch_count > 0 &&
                        fault->occurrence_count >= fault->config.latch_count) {
                        fault->latched = true;
                    }
                } else if (fault->state == FAULT_STATE_INIT && raw == FAULT_RAW_NORMAL) {
                    /* 首次收到正常样本且未触发故障，认为初始化完成 */
                    fault->state = FAULT_STATE_NORMAL;
                }
            } else if (fault->state == FAULT_STATE_FAULT) {
                if (fault->fault_accum < recover) {
                    fault->state = FAULT_STATE_NORMAL;
                }
            }
        }
    }
}

fault_state_t fault_get_state(int fault_id)
{
    if (fault_id >= 0 && fault_id < MAX_FAULTS && fault_table[fault_id].used) {
        return fault_table[fault_id].state;
    }
    return FAULT_STATE_INIT;    /* 无效 ID 时返回 INIT */
}

void fault_clear_latch(int fault_id)
{
    if (fault_id >= 0 && fault_id < MAX_FAULTS && fault_table[fault_id].used) {
        fault_runtime_t *fault = &fault_table[fault_id];
        if (fault->latched) {
            fault->latched = false;
            fault->state = FAULT_STATE_INIT;
            fault->fault_cont = 0;
            fault->normal_cont = 0;
            fault->fault_accum = 0;
            fault->occurrence_count = 0;
        }
    }
}

void fault_reset(int fault_id)
{
    if (fault_id >= 0 && fault_id < MAX_FAULTS && fault_table[fault_id].used) {
        fault_runtime_t *fault = &fault_table[fault_id];
        fault->state = FAULT_STATE_INIT;
        fault->fault_cont = 0;
        fault->normal_cont = 0;
        fault->fault_accum = 0;
        fault->occurrence_count = 0;
        fault->latched = false;
    }
}