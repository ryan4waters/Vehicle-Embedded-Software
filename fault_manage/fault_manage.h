#ifndef FAULT_MANAGE_H
#define FAULT_MANAGE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 原始采样状态 */
typedef enum {
    FAULT_RAW_NORMAL = 0,   /* 正常 */
    FAULT_RAW_FAULT,        /* 故障 */
    FAULT_RAW_INVALID       /* 无效（本次采样忽略） */
} fault_raw_state_t;

/* 故障当前状态 */
typedef enum {
    FAULT_STATE_INIT = 0,   /* 初始化（尚未确定） */
    FAULT_STATE_NORMAL,     /* 正常 */
    FAULT_STATE_FAULT       /* 故障 */
} fault_state_t;

/* 故障类型 */
typedef enum {
    FAULT_TYPE_CONTINUOUS = 0,  /* 连续型：需连续 N 次故障才触发，连续 M 次正常才恢复 */
    FAULT_TYPE_ACCUMULATIVE     /* 累积型：使用增减计数器，达到阈值触发，低于恢复阈值恢复 */
} fault_type_t;

/* 回调函数原型：外部实现，通过指针返回原始状态 */
typedef void (*fault_sample_callback_t)(fault_raw_state_t *raw_state);

/* 故障注册配置 */
typedef struct {
    fault_type_t type;              /* 故障类型 */
    uint16_t trigger_threshold;     /* 触发阈值（连续型：连续故障次数；累积型：累积故障计数值上限） */
    uint16_t recover_threshold;     /* 恢复阈值（连续型：连续正常次数；累积型：故障计数值须低于此值） */
    uint16_t latch_count;           /* 锁存次数：0 表示不锁存，>0 表示故障发生该次数后永久锁存 */
    fault_sample_callback_t sample_cb;  /* 采样回调函数 */
} fault_config_t;

/* 初始化故障管理器（可省略，使用静态数组时无需调用） */
void fault_manager_init(void);

/* 注册故障，返回故障 ID（索引），失败返回 -1 */
int fault_register(const fault_config_t *config);

/* 注销故障 */
void fault_unregister(int fault_id);

/* 周期处理函数：调用所有已注册故障的回调并更新状态 */
void fault_manager_process(void);

/* 获取指定故障当前状态 */
fault_state_t fault_get_state(int fault_id);

/* 清除锁存（若已锁存，则重置为 INIT 并清除计数） */
void fault_clear_latch(int fault_id);

/* 完全重置指定故障（状态、计数、锁存标志） */
void fault_reset(int fault_id);

#ifdef __cplusplus
}
#endif

#endif /* FAULT_MANAGE_H */