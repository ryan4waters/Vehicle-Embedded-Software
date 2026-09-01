#include "fault_manage.h"
#include <stdio.h>

/* 示例回调：模拟一个电压故障 */
static void voltage_fault_cb(fault_raw_state_t *raw) {
    /* 假设外部变量 voltage_ok 表示电压是否正常 */
    extern bool voltage_ok;
    *raw = voltage_ok ? FAULT_RAW_NORMAL : FAULT_RAW_FAULT;
}

int main(void) {
    fault_manager_init();

    fault_config_t cfg = {
        .type = FAULT_TYPE_CONTINUOUS,
        .trigger_threshold = 3,     /* 连续 3 次故障触发 */
        .recover_threshold = 2,     /* 连续 2 次正常恢复 */
        .latch_count = 0,           /* 不锁存 */
        .sample_cb = voltage_fault_cb
    };

    int id = fault_register(&cfg);
    if (id < 0) {
        printf("注册失败\n");
        return -1;
    }

    /* 主循环 */
    while (1) {
        fault_manager_process();
        fault_state_t st = fault_get_state(id);
        switch (st) {
            case FAULT_STATE_INIT:   printf("状态: INIT\n"); break;
            case FAULT_STATE_NORMAL: printf("状态: NORMAL\n"); break;
            case FAULT_STATE_FAULT:  printf("状态: FAULT\n"); break;
        }
        /* 延时或等待采样周期 */
    }
    return 0;
}