#ifndef S2_CONTROL_H
#define S2_CONTROL_H

#include <stdint.h>
#include <stdbool.h>


/* ============================================================
 * 基础宏
 * ============================================================ */

#define S2_TRUE                         (true)
#define S2_FALSE                        (false)

#define S2_CURRENT_LOW_A                (1.0f)


/* ============================================================
 * S2五种方案
 *
 * 1. GB/T 18487.1-2015
 * 2. X1
 * 3. F1
 * 4. BMS主导
 * 5. OBC主导
 * ============================================================ */

typedef enum
{
    S2_SCHEME_GBT = 0,
    S2_SCHEME_X1,
    S2_SCHEME_F1,
    S2_SCHEME_BMS_MASTER,
    S2_SCHEME_OBC_MASTER

} S2_Scheme_e;


/* ============================================================
 * S2状态机
 * ============================================================ */

typedef enum
{
    S2_STATE_OPEN = 0,

    S2_STATE_CLOSE_CHECK,

    S2_STATE_CLOSE_ACTION,

    S2_STATE_CLOSED,

    S2_STATE_STOP_POWER,

    S2_STATE_WAIT_CURRENT_LOW,

    S2_STATE_OPEN_ACTION,

    S2_STATE_FAULT

} S2_State_e;


/* ============================================================
 * S2断开原因 OR1 ~ OR7
 * ============================================================ */

typedef enum
{
    S2_OR_NONE = 0,

    /* OR1：正常结束 */
    S2_OR1_NORMAL_END,

    /* OR2：充电不允许 / 充电中止 */
    S2_OR2_CHARGE_NOT_ALLOWED,

    /* OR3：CC半连接 */
    S2_OR3_CC_HALF_CONNECT,

    /* OR4：CC断开 */
    S2_OR4_CC_DISCONNECT,

    /* OR5：CP PWM无效 */
    S2_OR5_CP_INVALID,

    /* OR6：OBC满足休眠条件 */
    S2_OR6_OBC_SLEEP,

    /* OR7：S2断开请求 */
    S2_OR7_OPEN_REQUEST

} S2_OpenReason_e;


/* ============================================================
 * S2故障
 * ============================================================ */

typedef enum
{
    S2_FAULT_NONE = 0,

    S2_FAULT_CLOSE_TIMEOUT,

    S2_FAULT_OPEN_TIMEOUT,

    S2_FAULT_CAN_TIMEOUT,

    S2_FAULT_FEEDBACK_ERROR

} S2_Fault_e;


/* ============================================================
 * CC状态
 * ============================================================ */

typedef enum
{
    S2_CC_STATE_UNKNOWN = 0,

    S2_CC_STATE_DISCONNECT,

    S2_CC_STATE_HALF_CONNECT,

    S2_CC_STATE_FULL_CONNECT

} S2_CCState_e;


/* ============================================================
 * CAN输入
 *
 * 实际工程中由CAN信号映射模块更新
 * ============================================================ */

typedef struct
{
    /* --------------------------------------------------------
     * BMS信号
     * -------------------------------------------------------- */

    bool bms_charge_allow;

    bool bms_s2_close_request;

    bool bms_s2_open_request;

    bool bms_charge_request;

    bool bms_cp_valid;

    bool bms_cc_full;

    bool bms_cc_half;

    bool bms_cc_disconnect;

    bool bms_ocb_fault_valid;

    bool bms_obc_no_fault;

    bool bms_can_timeout;


    /* --------------------------------------------------------
     * OBC信号
     * -------------------------------------------------------- */

    bool obc_charge_request;

    bool obc_charge_abort;

    bool obc_charge_allow;

    bool obc_s2_close_request;

    bool obc_s2_open_request;

    bool obc_cp_valid;

    bool obc_cc_full;

    bool obc_cc_half;

    bool obc_cc_disconnect;

    bool obc_sleep_condition;

    bool obc_irrecoverable_fault;

    bool obc_timeout;

    bool obc_status_valid;


    /* --------------------------------------------------------
     * CP
     * -------------------------------------------------------- */

    float cp_voltage;

    float cp_frequency;

    float cp_duty;


    /* --------------------------------------------------------
     * CC
     * -------------------------------------------------------- */

    S2_CCState_e cc_state;


    /* --------------------------------------------------------
     * 充电电流
     * -------------------------------------------------------- */

    float charge_current;


    /* --------------------------------------------------------
     * 电子锁
     * -------------------------------------------------------- */

    bool electronic_lock_locked;


    /* --------------------------------------------------------
     * AC状态
     * -------------------------------------------------------- */

    bool ac_ready;

} S2_CanInput_t;


/* ============================================================
 * GPIO输入
 * ============================================================ */

typedef struct
{
    /*
     * S2辅助触点
     *
     * true  = S2闭合
     * false = S2断开
     */
    bool s2_feedback;

} S2_GpioInput_t;


/* ============================================================
 * S2输出
 * ============================================================ */

typedef struct
{
    bool s2_close_output;

    bool s2_open_output;

    bool power_stop;

    bool charge_enable;

    bool s2_closed;

    bool s2_open;

    S2_State_e state;

    S2_OpenReason_e open_reason;

    S2_Fault_e fault;

} S2_Output_t;


/* ============================================================
 * 定时器
 * ============================================================ */

typedef struct
{
    uint32_t normal_end_ms;

    uint32_t cc_half_ms;

    uint32_t cc_disconnect_ms;

    uint32_t cp_invalid_ms;

    uint32_t cp_duty_100_ms;

    uint32_t cp_duty_invalid_ms;

    uint32_t current_low_ms;

    uint32_t close_action_ms;

    uint32_t open_action_ms;

    uint32_t can_timeout_ms;

    uint32_t cc_full_wait_ms;

    uint32_t sleep_ms;

} S2_Timer_t;


/* ============================================================
 * 防抖结构
 * ============================================================ */

typedef struct
{
    bool cc_full;

    bool cc_half;

    bool cc_disconnect;

    bool cp_valid;

    bool electronic_lock;

    bool charge_allow;

    bool charge_request;

    bool bms_s2_close_request;

    bool bms_s2_open_request;

    bool obc_s2_close_request;

    bool obc_s2_open_request;

} S2_Debounce_t;


/* ============================================================
 * 方案配置
 * ============================================================ */

typedef struct
{
    S2_Scheme_e scheme;

    /*
     * CP正常范围
     */
    float cp_voltage_min;

    float cp_voltage_max;

    float cp_frequency_min;

    float cp_frequency_max;

    float cp_duty_min;

    float cp_duty_max;


    /*
     * 方案时间参数
     */

    uint32_t normal_end_time_ms;

    uint32_t cc_half_time_ms;

    uint32_t cc_disconnect_time_ms;

    uint32_t cp_invalid_time_ms;

    uint32_t cp_invalid_confirm_ms;

    uint32_t cp_duty_100_time_ms;

    uint32_t cp_duty_invalid_time_ms;

    uint32_t close_timeout_ms;

    uint32_t open_timeout_ms;

    uint32_t can_timeout_ms;

} S2_Config_t;


/* ============================================================
 * 初始化
 * ============================================================ */

void S2_Init(S2_Scheme_e scheme);

void S2_SetConfig(const S2_Config_t *config);


/* ============================================================
 * 输入
 * ============================================================ */

void S2_SetCanInput(const S2_CanInput_t *input);

void S2_SetGpioInput(const S2_GpioInput_t *input);


/* ============================================================
 * 周期任务
 * ============================================================ */

void S2_Timer1ms(void);

void S2_MainFunction10ms(void);


/* ============================================================
 * 输出
 * ============================================================ */

const S2_Output_t *S2_GetOutput(void);

S2_State_e S2_GetState(void);

S2_OpenReason_e S2_GetOpenReason(void);

S2_Fault_e S2_GetFault(void);


/* ============================================================
 * 项目差异化接口
 * ============================================================ */

/*
 * GBT
 */
bool S2_GBT_CheckClose(void);

bool S2_GBT_CheckOpen(S2_OpenReason_e *reason);


/*
 * X1
 */
bool S2_X1_CheckClose(void);

bool S2_X1_CheckOpen(S2_OpenReason_e *reason);


/*
 * F1
 */
bool S2_F1_CheckClose(void);

bool S2_F1_CheckOpen(S2_OpenReason_e *reason);


/*
 * BMS主导
 */
bool S2_BMS_Master_CheckClose(void);

bool S2_BMS_Master_CheckOpen(S2_OpenReason_e *reason);


/*
 * OBC主导
 */
bool S2_OBC_Master_CheckClose(void);

bool S2_OBC_Master_CheckOpen(S2_OpenReason_e *reason);


/* ============================================================
 * HAL接口
 * ============================================================ */

void S2_HW_Close(void);

void S2_HW_Open(void);

bool S2_HW_GetFeedback(void);

void S2_HW_StopPower(void);


/* ============================================================
 * CAN发送
 * ============================================================ */

void S2_CAN_SendStatus(void);

void S2_CAN_SendCloseResult(void);

void S2_CAN_SendOpenResult(void);

#endif