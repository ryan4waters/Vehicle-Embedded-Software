/**
 * @file    EcuM.h
 * @brief   ECU 状态管理器公共接口定义
 */

#ifndef ECUM_H
#define ECUM_H

#include <stdint.h>

/* ECUM 状态枚举 */
typedef enum {
    ECUM_STATE_OFF,        /* ECU 完全下电 */
    ECUM_STATE_INIT,       /* 初始化阶段（上电或唤醒后） */
    ECUM_STATE_RUN,        /* 正常运行模式 */
    ECUM_STATE_PREP_SLEEP, /* 准备休眠（保存上下文、通知各模块） */
    ECUM_STATE_SLEEP,      /* 休眠状态（低功耗，可被唤醒） */
    ECUM_STATE_WAKEUP      /* 唤醒处理（从休眠恢复） */
} Ecum_StateType;

/**
 * @brief  EcuM 主处理函数，需在主循环或定时任务中周期调用
 */
void Ecum_MainFunction(void);

/**
 * @brief  请求进入休眠（由应用或通信管理模块调用）
 */
void Ecum_RequestSleep(void);

/**
 * @brief  设置唤醒源标志（通常在中断服务程序中调用）
 * @param  source: 唤醒源编号（非零值表示有唤醒事件）
 */
void Ecum_SetWakeupSource(uint8_t source);

/**
 * @brief  获取当前状态（可选，用于调试或外部查询）
 * @return 当前 ECUM 状态
 */
Ecum_StateType Ecum_GetState(void);

#endif /* ECUM_H */