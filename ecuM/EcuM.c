/**
 * @file    EcuM.c
 * @brief   ECU 状态管理器实现
 *
 * 包含状态机主逻辑、内部辅助函数及静态变量。
 * 该模块负责管理 ECU 的上电、运行、休眠和唤醒流程。
 */

#include "EcuM.h"

/* 当前状态（模块内部可见） */
static Ecum_StateType currentState = ECUM_STATE_OFF;

/* 唤醒源标志（模拟） */
static uint8_t wakeupSource = 0;

/* 休眠请求标志 */
static uint8_t sleepRequested = 0;

/* 初始化完成标志 */
static uint8_t initDone = 0;

/* ---------------- 内部辅助函数声明 ---------------- */
static uint8_t Ecum_CheckWakeupEvent(void);
static void    Ecum_StartInitSequence(void);
static uint8_t Ecum_InitStep(void);
static void    Ecum_EnterRun(void);
static void    Ecum_EnterPrepSleep(void);
static uint8_t Ecum_PrepSleepStep(void);
static void    Ecum_EnterSleep(void);
static void    Ecum_EnterWakeup(void);
static uint8_t Ecum_WakeupStep(void);

/* ---------------- 公共接口实现 ---------------- */

void Ecum_MainFunction(void)
{
    switch (currentState)
    {
        case ECUM_STATE_OFF:
            /* 在 OFF 状态检测唤醒事件（如上电、CAN 报文、RTC 等） */
            if (Ecum_CheckWakeupEvent())
            {
                Ecum_StartInitSequence();
            }
            break;

        case ECUM_STATE_INIT:
            /* 执行初始化步骤，全部完成后跳转至 RUN */
            if (Ecum_InitStep())
            {
                Ecum_EnterRun();
            }
            break;

        case ECUM_STATE_RUN:
            /* 正常运行，检测休眠请求 */
            if (sleepRequested)
            {
                Ecum_EnterPrepSleep();
            }
            break;

        case ECUM_STATE_PREP_SLEEP:
            /* 执行休眠前准备工作，完成后进入 SLEEP */
            if (Ecum_PrepSleepStep())
            {
                Ecum_EnterSleep();
            }
            break;

        case ECUM_STATE_SLEEP:
            /* 在 SLEEP 状态检测唤醒事件 */
            if (Ecum_CheckWakeupEvent())
            {
                Ecum_EnterWakeup();
            }
            break;

        case ECUM_STATE_WAKEUP:
            /* 处理唤醒后的恢复流程，完成后回到 RUN */
            if (Ecum_WakeupStep())
            {
                Ecum_EnterRun();
            }
            break;

        default:
            /* 不应到达，复位到 OFF */
            currentState = ECUM_STATE_OFF;
            break;
    }
}

void Ecum_RequestSleep(void)
{
    sleepRequested = 1;
}

void Ecum_SetWakeupSource(uint8_t source)
{
    wakeupSource = source;
}

Ecum_StateType Ecum_GetState(void)
{
    return currentState;
}

/* ---------------- 内部辅助函数实现 ---------------- */

/**
 * @brief 检查是否有唤醒事件，若有则清除标志并返回 1
 */
static uint8_t Ecum_CheckWakeupEvent(void)
{
    if (wakeupSource != 0)
    {
        wakeupSource = 0;   /* 清除标志 */
        return 1;
    }
    return 0;
}

/**
 * @brief 开始初始化序列：复位内部变量，进入 INIT 状态
 */
static void Ecum_StartInitSequence(void)
{
    initDone = 0;
    currentState = ECUM_STATE_INIT;
    /* 可在此调用底层驱动初始化（如时钟、GPIO 等） */
}

/**
 * @brief 执行初始化步骤（模拟分步初始化）
 * @return 1: 初始化完成；0: 仍需继续
 */
static uint8_t Ecum_InitStep(void)
{
    /* 实际中可在此初始化各个模块，例如：
       - 初始化 MCU 驱动
       - 初始化通信栈
       - 初始化 OS
       本例仅模拟一个步骤完成 */
    initDone = 1;
    return initDone;
}

/**
 * @brief 进入运行状态
 */
static void Ecum_EnterRun(void)
{
    currentState = ECUM_STATE_RUN;
    /* 启动正常运行所需的任务或调度 */
}

/**
 * @brief 进入准备休眠状态
 */
static void Ecum_EnterPrepSleep(void)
{
    sleepRequested = 0;
    currentState = ECUM_STATE_PREP_SLEEP;
    /* 通知各模块保存数据、关闭外设等 */
}

/**
 * @brief 执行休眠准备步骤（模拟）
 * @return 1: 准备完成；0: 仍需继续
 */
static uint8_t Ecum_PrepSleepStep(void)
{
    /* 此处应完成：
       - 保存非易失数据
       - 通知所有 SWC 停止运行
       - 关闭非必要外设
       完成后返回 1 */
    return 1;
}

/**
 * @brief 进入休眠状态
 */
static void Ecum_EnterSleep(void)
{
    currentState = ECUM_STATE_SLEEP;
    /* 配置唤醒源，进入低功耗模式（如 STOP 模式） */
}

/**
 * @brief 进入唤醒处理状态
 */
static void Ecum_EnterWakeup(void)
{
    currentState = ECUM_STATE_WAKEUP;
    /* 恢复时钟、重新初始化必要外设 */
}

/**
 * @brief 执行唤醒恢复步骤（模拟）
 * @return 1: 恢复完成；0: 仍需继续
 */
static uint8_t Ecum_WakeupStep(void)
{
    /* 此处应完成：
       - 恢复外设状态
       - 通知各模块恢复运行
       完成后返回 1 */
    return 1;
}