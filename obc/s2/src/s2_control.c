#include "s2_control.h"


/* ============================================================
 * 时间参数
 * ============================================================ */

#define S2_DEBOUNCE_COUNT                 (3U)
/*
 * 10ms任务 × 3 = 30ms防抖
 */


/*
 * OR1：
 * 电流 < 1A 后100ms内断开
 */
#define S2_TIME_100MS                     (100U)


/*
 * BMS主导CP无效
 */
#define S2_TIME_1P8S                      (1800U)


/*
 * OBC主导CP无效
 */
#define S2_TIME_2P8S                      (2800U)


/*
 * CC断开
 */
#define S2_TIME_3S                        (3000U)


/*
 * CP Duty=100%
 */
#define S2_TIME_3S_DUTY100                (3000U)


/*
 * CP Duty不在8%~97%
 *
 * 8s内要求电流降至<1A
 */
#define S2_TIME_8S                        (8000U)


/*
 * CC半连接持续时间
 *
 * 文件方案中存在5min处理。
 */
#define S2_TIME_5MIN                      (300000U)


/*
 * OBC CP无效确认时间
 *
 * BMS主导路径：
 *
 * OBC确认CP无效约1s
 * +
 * BMS持续1.8s
 *
 * 总体达到约2.8s
 */
#define S2_TIME_OBC_CP_CONFIRM            (1000U)


/*
 * S2执行器超时
 */
#define S2_CLOSE_TIMEOUT_DEFAULT          (1000U)

#define S2_OPEN_TIMEOUT_DEFAULT           (1000U)


/* ============================================================
 * CP参数
 * ============================================================ */

#define S2_CP_VOLTAGE_MIN_DEFAULT         (4.5f)
#define S2_CP_VOLTAGE_MAX_DEFAULT         (7.5f)

#define S2_CP_FREQ_MIN_DEFAULT            (905.0f)
#define S2_CP_FREQ_MAX_DEFAULT            (1050.0f)

#define S2_CP_DUTY_MIN_DEFAULT            (7.5f)
#define S2_CP_DUTY_MAX_DEFAULT            (97.0f)


/* ============================================================
 * 全局变量
 * ============================================================ */

S2_CanInput_t g_s2_input;

static S2_GpioInput_t g_s2_gpio;

static S2_Output_t g_s2_output;

static S2_Timer_t g_s2_timer;

static S2_Debounce_t g_s2_db;

static S2_Config_t g_s2_config;


/* ============================================================
 * 防抖计数器
 * ============================================================ */

static uint8_t s_db_cc_full;

static uint8_t s_db_cc_half;

static uint8_t s_db_cc_disconnect;

static uint8_t s_db_cp_valid;

static uint8_t s_db_lock;

static uint8_t s_db_charge_allow;

static uint8_t s_db_charge_request;

static uint8_t s_db_bms_close;

static uint8_t s_db_bms_open;

static uint8_t s_db_obc_close;

static uint8_t s_db_obc_open;


/* ============================================================
 * Bool防抖
 * ============================================================ */

static bool S2_DebounceBool(
    bool input,
    bool current,
    uint8_t *counter)
{
    if (input == current)
    {
        *counter = 0U;

        return current;
    }


    if (*counter < S2_DEBOUNCE_COUNT)
    {
        (*counter)++;
    }


    if (*counter >= S2_DEBOUNCE_COUNT)
    {
        *counter = 0U;

        return input;
    }


    return current;
}


/* ============================================================
 * 输入防抖
 * ============================================================ */

static void S2_DebounceProcess(void)
{
    g_s2_db.cc_full =
        S2_DebounceBool(
            g_s2_input.cc_state ==
            S2_CC_STATE_FULL_CONNECT,
            g_s2_db.cc_full,
            &s_db_cc_full);


    g_s2_db.cc_half =
        S2_DebounceBool(
            g_s2_input.cc_state ==
            S2_CC_STATE_HALF_CONNECT,
            g_s2_db.cc_half,
            &s_db_cc_half);


    g_s2_db.cc_disconnect =
        S2_DebounceBool(
            g_s2_input.cc_state ==
            S2_CC_STATE_DISCONNECT,
            g_s2_db.cc_disconnect,
            &s_db_cc_disconnect);


    g_s2_db.cp_valid =
        S2_DebounceBool(
            g_s2_input.obc_cp_valid ||
            g_s2_input.bms_cp_valid,
            g_s2_db.cp_valid,
            &s_db_cp_valid);


    g_s2_db.electronic_lock =
        S2_DebounceBool(
            g_s2_input.electronic_lock_locked,
            g_s2_db.electronic_lock,
            &s_db_lock);


    g_s2_db.charge_allow =
        S2_DebounceBool(
            g_s2_input.bms_charge_allow,
            g_s2_db.charge_allow,
            &s_db_charge_allow);


    g_s2_db.charge_request =
        S2_DebounceBool(
            g_s2_input.bms_charge_request ||
            g_s2_input.obc_charge_request,
            g_s2_db.charge_request,
            &s_db_charge_request);


    g_s2_db.bms_s2_close_request =
        S2_DebounceBool(
            g_s2_input.bms_s2_close_request,
            g_s2_db.bms_s2_close_request,
            &s_db_bms_close);


    g_s2_db.bms_s2_open_request =
        S2_DebounceBool(
            g_s2_input.bms_s2_open_request,
            g_s2_db.bms_s2_open_request,
            &s_db_bms_open);


    g_s2_db.obc_s2_close_request =
        S2_DebounceBool(
            g_s2_input.obc_s2_close_request,
            g_s2_db.obc_s2_close_request,
            &s_db_obc_close);


    g_s2_db.obc_s2_open_request =
        S2_DebounceBool(
            g_s2_input.obc_s2_open_request,
            g_s2_db.obc_s2_open_request,
            &s_db_obc_open);
}


/* ============================================================
 * CP PWM判断
 * ============================================================ */

static bool S2_CheckCpValid(void)
{
    bool voltage_valid;

    bool frequency_valid;

    bool duty_valid;


    voltage_valid =
        (g_s2_input.cp_voltage >=
         g_s2_config.cp_voltage_min) &&

        (g_s2_input.cp_voltage <=
         g_s2_config.cp_voltage_max);


    frequency_valid =
        (g_s2_input.cp_frequency >=
         g_s2_config.cp_frequency_min) &&

        (g_s2_input.cp_frequency <=
         g_s2_config.cp_frequency_max);


    duty_valid =
        (g_s2_input.cp_duty >=
         g_s2_config.cp_duty_min) &&

        (g_s2_input.cp_duty <=
         g_s2_config.cp_duty_max);


    return
        voltage_valid &&
        frequency_valid &&
        duty_valid;
}


/* ============================================================
 * CP Duty=100%
 * ============================================================ */

static bool S2_CheckCpDuty100(void)
{
    return
        (g_s2_input.cp_duty >= 99.5f);
}


/* ============================================================
 * CP Duty超范围
 * ============================================================ */

static bool S2_CheckCpDutyOutOfRange(void)
{
    if (g_s2_input.cp_duty <
        g_s2_config.cp_duty_min)
    {
        return true;
    }


    if (g_s2_input.cp_duty >
        g_s2_config.cp_duty_max)
    {
        return true;
    }


    return false;
}


/* ============================================================
 * 1ms计时
 * ============================================================ */

void S2_Timer1ms(void)
{
    /*
     * --------------------------------------------------------
     * OR1：
     * 电流<1A计时
     * --------------------------------------------------------
     */

    if (g_s2_input.charge_current <
        S2_CURRENT_LOW_A)
    {
        if (g_s2_timer.normal_end_ms <
            S2_TIME_100MS)
        {
            g_s2_timer.normal_end_ms++;
        }


        if (g_s2_timer.current_low_ms <
            S2_TIME_8S)
        {
            g_s2_timer.current_low_ms++;
        }
    }
    else
    {
        g_s2_timer.normal_end_ms = 0U;

        g_s2_timer.current_low_ms = 0U;
    }


    /*
     * --------------------------------------------------------
     * OR3：
     * CC半连接
     * --------------------------------------------------------
     */

    if (g_s2_db.cc_half)
    {
        if (g_s2_timer.cc_half_ms <
            S2_TIME_5MIN)
        {
            g_s2_timer.cc_half_ms++;
        }
    }
    else
    {
        g_s2_timer.cc_half_ms = 0U;
    }


    /*
     * --------------------------------------------------------
     * OR4：
     * CC断开
     * --------------------------------------------------------
     */

    if (g_s2_db.cc_disconnect)
    {
        if (g_s2_timer.cc_disconnect_ms <
            S2_TIME_3S)
        {
            g_s2_timer.cc_disconnect_ms++;
        }
    }
    else
    {
        g_s2_timer.cc_disconnect_ms = 0U;
    }


    /*
     * --------------------------------------------------------
     * OR5：
     * CP无效
     * --------------------------------------------------------
     */

    if (!S2_CheckCpValid())
    {
        if (g_s2_timer.cp_invalid_ms <
            S2_TIME_2P8S)
        {
            g_s2_timer.cp_invalid_ms++;
        }
    }
    else
    {
        g_s2_timer.cp_invalid_ms = 0U;
    }


    /*
     * --------------------------------------------------------
     * CP Duty=100%
     * --------------------------------------------------------
     */

    if (S2_CheckCpDuty100())
    {
        if (g_s2_timer.cp_duty_100_ms <
            S2_TIME_3S_DUTY100)
        {
            g_s2_timer.cp_duty_100_ms++;
        }
    }
    else
    {
        g_s2_timer.cp_duty_100_ms = 0U;
    }


    /*
     * --------------------------------------------------------
     * CP Duty超范围
     *
     * 8s内要求电流降到<1A
     * --------------------------------------------------------
     */

    if (S2_CheckCpDutyOutOfRange())
    {
        if (g_s2_timer.cp_duty_invalid_ms <
            S2_TIME_8S)
        {
            g_s2_timer.cp_duty_invalid_ms++;
        }
    }
    else
    {
        g_s2_timer.cp_duty_invalid_ms = 0U;
    }


    /*
     * --------------------------------------------------------
     * S2闭合动作超时
     * --------------------------------------------------------
     */

    if (g_s2_output.state ==
        S2_STATE_CLOSE_ACTION)
    {
        if (g_s2_timer.close_action_ms <
            g_s2_config.close_timeout_ms)
        {
            g_s2_timer.close_action_ms++;
        }
    }
    else
    {
        g_s2_timer.close_action_ms = 0U;
    }


    /*
     * --------------------------------------------------------
     * S2断开动作超时
     * --------------------------------------------------------
     */

    if (g_s2_output.state ==
        S2_STATE_OPEN_ACTION)
    {
        if (g_s2_timer.open_action_ms <
            g_s2_config.open_timeout_ms)
        {
            g_s2_timer.open_action_ms++;
        }
    }
    else
    {
        g_s2_timer.open_action_ms = 0U;
    }


    /*
     * --------------------------------------------------------
     * CAN超时
     * --------------------------------------------------------
     */

    if (g_s2_input.bms_can_timeout ||
        g_s2_input.obc_timeout)
    {
        if (g_s2_timer.can_timeout_ms <
            g_s2_config.can_timeout_ms)
        {
            g_s2_timer.can_timeout_ms++;
        }
    }
    else
    {
        g_s2_timer.can_timeout_ms = 0U;
    }
}


/* ============================================================
 * OR1
 *
 * 正常结束
 *
 * 电流<1A持续100ms
 * ============================================================ */

static bool S2_CheckOR1(S2_OpenReason_e *reason)
{
    if (g_s2_input.charge_current >=
        S2_CURRENT_LOW_A)
    {
        return false;
    }


    if (g_s2_timer.normal_end_ms >=
        g_s2_config.normal_end_time_ms)
    {
        *reason = S2_OR1_NORMAL_END;

        return true;
    }


    return false;
}


/* ============================================================
 * OR2
 *
 * 充电不允许 / 充电中止
 * ============================================================ */

static bool S2_CheckOR2(S2_OpenReason_e *reason)
{
    if (!g_s2_input.bms_charge_allow)
    {
        *reason =
            S2_OR2_CHARGE_NOT_ALLOWED;

        return true;
    }


    if (g_s2_input.obc_charge_abort)
    {
        *reason =
            S2_OR2_CHARGE_NOT_ALLOWED;

        return true;
    }


    return false;
}


/* ============================================================
 * OR3
 *
 * CC半连接
 *
 * 根据方案要求：
 *
 * 电流降至<1A
 * 后进入断开处理
 * ============================================================ */

static bool S2_CheckOR3(S2_OpenReason_e *reason)
{
    if (!g_s2_db.cc_half)
    {
        return false;
    }


    /*
     * 电流还没有降下来
     */
    if (g_s2_input.charge_current >=
        S2_CURRENT_LOW_A)
    {
        return false;
    }


    /*
     * 100ms确认
     */
    if (g_s2_timer.normal_end_ms >=
        S2_TIME_100MS)
    {
        *reason =
            S2_OR3_CC_HALF_CONNECT;

        return true;
    }


    return false;
}


/* ============================================================
 * OR4
 *
 * CC断开
 *
 * 3s内停止充电，然后断开S2
 * ============================================================ */

static bool S2_CheckOR4(S2_OpenReason_e *reason)
{
    if (!g_s2_db.cc_disconnect)
    {
        return false;
    }


    /*
     * 立即停止功率级
     */
    S2_HW_StopPower();


    /*
     * 3s达到
     */
    if (g_s2_timer.cc_disconnect_ms >=
        g_s2_config.cc_disconnect_time_ms)
    {
        *reason =
            S2_OR4_CC_DISCONNECT;

        return true;
    }


    return false;
}


/* ============================================================
 * OR5
 *
 * CP PWM无效
 *
 * 需要根据5个方案选择不同时间
 * ============================================================ */

static bool S2_CheckOR5(S2_OpenReason_e *reason)
{
    uint32_t timeout_ms;


    if (S2_CheckCpValid())
    {
        return false;
    }


    /*
     * CP异常时先停止功率
     */
    S2_HW_StopPower();


    /*
     * ========================================================
     * GBT
     *
     * CP PWM无效：
     *
     * 3s内断开S2
     * ========================================================
     */

    if (g_s2_config.scheme ==
        S2_SCHEME_GBT)
    {
        timeout_ms = S2_TIME_3S;
    }


    /*
     * ========================================================
     * X1
     *
     * 文件中：
     *
     * BMS：CP无效持续2s
     * OBC：CP无效持续2.8s
     *
     * X1使用现有X逻辑
     * ========================================================
     */

    else if (g_s2_config.scheme ==
             S2_SCHEME_X1)
    {
        /*
         * 这里默认采用OBC侧2.8s路径。
         *
         * 若实际X1软件已有BMS CP无效判定，
         * 可在X1.c中将策略切换为2s。
         */
        timeout_ms = S2_TIME_2P8S;
    }


    /*
     * ========================================================
     * F1
     *
     * 文件给出的X/F现有逻辑
     * ========================================================
     */

    else if (g_s2_config.scheme ==
             S2_SCHEME_F1)
    {
        timeout_ms = S2_TIME_2P8S;
    }


    /*
     * ========================================================
     * BMS主导
     *
     * BMS收到OBC上报CP无效持续1.8s。
     *
     * OBC需要约1s确认上报，
     * 因而系统总时间约2.8s。
     *
     * ========================================================
     */

    else if (g_s2_config.scheme ==
             S2_SCHEME_BMS_MASTER)
    {
        timeout_ms =
            S2_TIME_1P8S +
            S2_TIME_OBC_CP_CONFIRM;
    }


    /*
     * ========================================================
     * OBC主导
     *
     * CP无效持续2.8s
     * ========================================================
     */

    else
    {
        timeout_ms = S2_TIME_2P8S;
    }


    if (g_s2_timer.cp_invalid_ms >=
        timeout_ms)
    {
        *reason =
            S2_OR5_CP_INVALID;

        return true;
    }


    /*
     * ========================================================
     * CP Duty=100%
     *
     * 3s内断开
     * ========================================================
     */

    if (g_s2_timer.cp_duty_100_ms >=
        S2_TIME_3S_DUTY100)
    {
        *reason =
            S2_OR5_CP_INVALID;

        return true;
    }


    /*
     * ========================================================
     * CP Duty超8%~97%范围
     *
     * 8s内要求电流降至<1A。
     * ========================================================
     */

    if (g_s2_timer.cp_duty_invalid_ms >=
        S2_TIME_8S)
    {
        if (g_s2_input.charge_current <
            S2_CURRENT_LOW_A)
        {
            *reason =
                S2_OR5_CP_INVALID;

            return true;
        }
    }


    return false;
}


/* ============================================================
 * OR6
 *
 * OBC满足休眠条件
 *
 * OBC主导/BMS主导均保留此安全路径
 * ============================================================ */

static bool S2_CheckOR6(S2_OpenReason_e *reason)
{
    if (!g_s2_input.obc_sleep_condition)
    {
        return false;
    }


    S2_HW_StopPower();


    *reason =
        S2_OR6_OBC_SLEEP;


    return true;
}


/* ============================================================
 * OR7
 *
 * S2断开请求
 * ============================================================ */

static bool S2_CheckOR7(S2_OpenReason_e *reason)
{
    if (g_s2_input.bms_s2_open_request ||
        g_s2_input.obc_s2_open_request)
    {
        *reason =
            S2_OR7_OPEN_REQUEST;

        return true;
    }


    return false;
}


/* ============================================================
 * 通用OR1~OR7
 * ============================================================ */

static bool S2_CheckCommonOpen(
    S2_OpenReason_e *reason)
{
    /*
     * OR1
     */
    if (S2_CheckOR1(reason))
    {
        return true;
    }


    /*
     * OR2
     */
    if (S2_CheckOR2(reason))
    {
        return true;
    }


    /*
     * OR3
     */
    if (S2_CheckOR3(reason))
    {
        return true;
    }


    /*
     * OR4
     */
    if (S2_CheckOR4(reason))
    {
        return true;
    }


    /*
     * OR5
     */
    if (S2_CheckOR5(reason))
    {
        return true;
    }


    /*
     * OR6
     */
    if (S2_CheckOR6(reason))
    {
        return true;
    }


    /*
     * OR7
     */
    if (S2_CheckOR7(reason))
    {
        return true;
    }


    return false;
}


/* ============================================================
 * 五方案闭合判断
 *
 * ★方案选择的核心位置
 * ============================================================ */

static bool S2_CheckCloseRequest(void)
{
    switch (g_s2_config.scheme)
    {
        /*
         * 方案1：
         * GB/T
         */
        case S2_SCHEME_GBT:

            return S2_GBT_CheckClose();


        /*
         * 方案2：
         * X1
         */
        case S2_SCHEME_X1:

            return S2_X1_CheckClose();


        /*
         * 方案3：
         * F1
         */
        case S2_SCHEME_F1:

            return S2_F1_CheckClose();


        /*
         * 方案4：
         * BMS主导
         */
        case S2_SCHEME_BMS_MASTER:

            return S2_BMS_Master_CheckClose();


        /*
         * 方案5：
         * OBC主导
         */
        case S2_SCHEME_OBC_MASTER:

            return S2_OBC_Master_CheckClose();


        default:

            return false;
    }
}


/* ============================================================
 * 五方案断开判断
 *
 * 这里先执行方案差异，
 * 再执行公共OR1~OR7。
 * ============================================================ */

static bool S2_CheckOpenRequest(
    S2_OpenReason_e *reason)
{
    bool project_open;


    if (reason == NULL)
    {
        return false;
    }


    /*
     * ========================================================
     * BMS主导
     * ========================================================
     */

    if (g_s2_config.scheme ==
        S2_SCHEME_BMS_MASTER)
    {
        project_open =
            S2_BMS_Master_CheckOpen(reason);

        if (project_open)
        {
            return true;
        }
    }


    /*
     * ========================================================
     * OBC主导
     * ========================================================
     */

    if (g_s2_config.scheme ==
        S2_SCHEME_OBC_MASTER)
    {
        project_open =
            S2_OBC_Master_CheckOpen(reason);

        if (project_open)
        {
            return true;
        }
    }


    /*
     * ========================================================
     * GBT/X1/F1
     * ========================================================
     */

    if (g_s2_config.scheme ==
        S2_SCHEME_GBT)
    {
        project_open =
            S2_GBT_CheckOpen(reason);

        if (project_open)
        {
            return true;
        }
    }


    if (g_s2_config.scheme ==
        S2_SCHEME_X1)
    {
        project_open =
            S2_X1_CheckOpen(reason);

        if (project_open)
        {
            return true;
        }
    }


    if (g_s2_config.scheme ==
        S2_SCHEME_F1)
    {
        project_open =
            S2_F1_CheckOpen(reason);

        if (project_open)
        {
            return true;
        }
    }


    /*
     * ========================================================
     * 公共OR1~OR7
     * ========================================================
     */

    return S2_CheckCommonOpen(reason);
}


/* ============================================================
 * 状态机
 * ============================================================ */

static void S2_StateMachine(void)
{
    S2_OpenReason_e reason;

    bool open_request;


    reason = S2_OR_NONE;


    /* ========================================================
     * OPEN
     * ======================================================== */

    if (g_s2_output.state ==
        S2_STATE_OPEN)
    {
        /*
         * S2保持打开
         */
        S2_HW_Open();


        g_s2_output.s2_close_output = false;

        g_s2_output.s2_open_output = true;

        g_s2_output.s2_closed = false;

        g_s2_output.s2_open = true;

        g_s2_output.power_stop = false;


        /*
         * 检查闭合
         */
        if (S2_CheckCloseRequest())
        {
            g_s2_output.state =
                S2_STATE_CLOSE_CHECK;
        }


        return;
    }


    /* ========================================================
     * CLOSE_CHECK
     * ======================================================== */

    if (g_s2_output.state ==
        S2_STATE_CLOSE_CHECK)
    {
        /*
         * 闭合前重新检查所有前置条件。
         *
         * 防止CAN信号在等待过程中发生变化。
         */
        if (!S2_CheckCloseRequest())
        {
            g_s2_output.state =
                S2_STATE_OPEN;

            return;
        }


        /*
         * 开始执行S2闭合
         */
        g_s2_output.state =
            S2_STATE_CLOSE_ACTION;


        g_s2_timer.close_action_ms = 0U;


        return;
    }


    /* ========================================================
     * CLOSE_ACTION
     * ======================================================== */

    if (g_s2_output.state ==
        S2_STATE_CLOSE_ACTION)
    {
        S2_HW_Close();


        g_s2_output.s2_close_output = true;

        g_s2_output.s2_open_output = false;


        /*
         * 检查实际反馈
         */
        if (S2_HW_GetFeedback())
        {
            g_s2_output.s2_closed = true;

            g_s2_output.s2_open = false;

            g_s2_output.state =
                S2_STATE_CLOSED;

            g_s2_timer.close_action_ms = 0U;


            S2_CAN_SendCloseResult();


            return;
        }


        /*
         * S2闭合超时
         */
        if (g_s2_timer.close_action_ms >=
            g_s2_config.close_timeout_ms)
        {
            /*
             * S2闭合失败：
             *
             * 必须停止功率
             * 强制打开S2
             */
            S2_HW_StopPower();

            S2_HW_Open();


            g_s2_output.fault =
                S2_FAULT_CLOSE_TIMEOUT;


            g_s2_output.state =
                S2_STATE_FAULT;


            return;
        }


        return;
    }


    /* ========================================================
     * CLOSED
     * ======================================================== */

    if (g_s2_output.state ==
        S2_STATE_CLOSED)
    {
        /*
         * S2保持闭合
         */
        S2_HW_Close();


        g_s2_output.s2_close_output = true;

        g_s2_output.s2_open_output = false;

        g_s2_output.s2_closed = true;

        g_s2_output.s2_open = false;


        /*
         * 检查断开条件
         */
        open_request =
            S2_CheckOpenRequest(&reason);


        if (open_request)
        {
            g_s2_output.open_reason =
                reason;


            /*
             * 先停止功率
             */
            g_s2_output.state =
                S2_STATE_STOP_POWER;


            g_s2_output.power_stop =
                true;


            return;
        }


        return;
    }


    /* ========================================================
     * STOP_POWER
     * ======================================================== */

    if (g_s2_output.state ==
        S2_STATE_STOP_POWER)
    {
        /*
         * 首先关闭功率级。
         *
         * 注意：
         * S2_Control只发停止功率请求，
         * 不直接控制PFC/DCDC内部状态机。
         */
        S2_HW_StopPower();


        g_s2_output.power_stop = true;


        /*
         * ----------------------------------------------------
         * OR1
         *
         * 已经<1A：
         * 可以进入S2 OPEN。
         * ----------------------------------------------------
         */
        if (g_s2_output.open_reason ==
            S2_OR1_NORMAL_END)
        {
            g_s2_output.state =
                S2_STATE_OPEN_ACTION;

            return;
        }


        /*
         * ----------------------------------------------------
         * OR3
         *
         * CC半连接：
         * 电流<1A后断S2
         * ----------------------------------------------------
         */
        if (g_s2_output.open_reason ==
            S2_OR3_CC_HALF_CONNECT)
        {
            g_s2_output.state =
                S2_STATE_OPEN_ACTION;

            return;
        }


        /*
         * ----------------------------------------------------
         * OR4
         *
         * CC断开：
         * 3s停止充电后断S2
         * ----------------------------------------------------
         */
        if (g_s2_output.open_reason ==
            S2_OR4_CC_DISCONNECT)
        {
            g_s2_output.state =
                S2_STATE_OPEN_ACTION;

            return;
        }


        /*
         * ----------------------------------------------------
         * OR5
         *
         * CP异常：
         * 已经执行停止功率。
         *
         * 直接进入S2打开。
         * ----------------------------------------------------
         */
        if (g_s2_output.open_reason ==
            S2_OR5_CP_INVALID)
        {
            g_s2_output.state =
                S2_STATE_OPEN_ACTION;

            return;
        }


        /*
         * ----------------------------------------------------
         * OR2 / OR6 / OR7
         *
         * 安全路径：
         * 直接打开S2。
         * ----------------------------------------------------
         */
        g_s2_output.state =
            S2_STATE_OPEN_ACTION;


        return;
    }


    /* ========================================================
     * OPEN_ACTION
     * ======================================================== */

    if (g_s2_output.state ==
        S2_STATE_OPEN_ACTION)
    {
        /*
         * S2打开
         */
        S2_HW_Open();


        g_s2_output.s2_close_output = false;

        g_s2_output.s2_open_output = true;


        /*
         * 检查实际反馈
         */
        if (!S2_HW_GetFeedback())
        {
            g_s2_output.s2_closed = false;

            g_s2_output.s2_open = true;

            g_s2_output.power_stop = false;

            g_s2_output.state =
                S2_STATE_OPEN;


            g_s2_timer.open_action_ms = 0U;


            S2_CAN_SendOpenResult();


            return;
        }


        /*
         * S2打开超时
         */
        if (g_s2_timer.open_action_ms >=
            g_s2_config.open_timeout_ms)
        {
            /*
             * 断开失败：
             *
             * 保持功率停止
             */
            S2_HW_StopPower();

            S2_HW_Open();


            g_s2_output.fault =
                S2_FAULT_OPEN_TIMEOUT;


            g_s2_output.power_stop = true;

            g_s2_output.state =
                S2_STATE_FAULT;


            return;
        }


        return;
    }


    /* ========================================================
     * FAULT
     * ======================================================== */

    if (g_s2_output.state ==
        S2_STATE_FAULT)
    {
        /*
         * 故障状态：
         *
         * 1. 功率必须停止
         * 2. S2必须打开
         * 3. 不允许自动重新闭合
         */
        S2_HW_StopPower();

        S2_HW_Open();


        g_s2_output.s2_close_output = false;

        g_s2_output.s2_open_output = true;

        g_s2_output.power_stop = true;

        g_s2_output.s2_closed = false;

        g_s2_output.s2_open = false;


        return;
    }


    /*
     * 状态异常：
     * 回OPEN
     */
    S2_HW_StopPower();

    S2_HW_Open();

    g_s2_output.state =
        S2_STATE_OPEN;
}


/* ============================================================
 * 初始化
 * ============================================================ */

void S2_Init(S2_Scheme_e scheme)
{
    /*
     * 清零
     */
    g_s2_input =
        (S2_CanInput_t){0};

    g_s2_gpio =
        (S2_GpioInput_t){0};

    g_s2_output =
        (S2_Output_t){0};

    g_s2_timer =
        (S2_Timer_t){0};

    g_s2_db =
        (S2_Debounce_t){0};


    /*
     * ========================================================
     * ★方案选择
     * ========================================================
     */

    g_s2_config.scheme = scheme;


    /*
     * CP参数
     */
    g_s2_config.cp_voltage_min =
        S2_CP_VOLTAGE_MIN_DEFAULT;

    g_s2_config.cp_voltage_max =
        S2_CP_VOLTAGE_MAX_DEFAULT;

    g_s2_config.cp_frequency_min =
        S2_CP_FREQ_MIN_DEFAULT;

    g_s2_config.cp_frequency_max =
        S2_CP_FREQ_MAX_DEFAULT;

    g_s2_config.cp_duty_min =
        S2_CP_DUTY_MIN_DEFAULT;

    g_s2_config.cp_duty_max =
        S2_CP_DUTY_MAX_DEFAULT;


    /*
     * 时间参数
     */
    g_s2_config.normal_end_time_ms =
        S2_TIME_100MS;

    g_s2_config.cc_half_time_ms =
        S2_TIME_5MIN;

    g_s2_config.cc_disconnect_time_ms =
        S2_TIME_3S;

    g_s2_config.cp_invalid_time_ms =
        S2_TIME_2P8S;

    g_s2_config.cp_invalid_confirm_ms =
        S2_TIME_OBC_CP_CONFIRM;

    g_s2_config.cp_duty_100_time_ms =
        S2_TIME_3S_DUTY100;

    g_s2_config.cp_duty_invalid_time_ms =
        S2_TIME_8S;

    g_s2_config.close_timeout_ms =
        S2_CLOSE_TIMEOUT_DEFAULT;

    g_s2_config.open_timeout_ms =
        S2_OPEN_TIMEOUT_DEFAULT;

    g_s2_config.can_timeout_ms =
        100U;


    /*
     * 初始状态
     */
    g_s2_output.state =
        S2_STATE_OPEN;

    g_s2_output.open_reason =
        S2_OR_NONE;

    g_s2_output.fault =
        S2_FAULT_NONE;


    /*
     * 安全起始状态：
     *
     * S2必须OPEN
     */
    S2_HW_Open();

    S2_HW_StopPower();
}


/* ============================================================
 * 配置更新
 * ============================================================ */

void S2_SetConfig(const S2_Config_t *config)
{
    if (config == NULL)
    {
        return;
    }


    g_s2_config = *config;
}


/* ============================================================
 * CAN输入
 * ============================================================ */

void S2_SetCanInput(const S2_CanInput_t *input)
{
    if (input == NULL)
    {
        return;
    }


    g_s2_input = *input;
}


/* ============================================================
 * GPIO输入
 * ============================================================ */

void S2_SetGpioInput(const S2_GpioInput_t *input)
{
    if (input == NULL)
    {
        return;
    }


    g_s2_gpio = *input;
}


/* ============================================================
 * 10ms任务
 * ============================================================ */

void S2_MainFunction10ms(void)
{
    /*
     * ========================================================
     * Step 1
     *
     * CAN输入防抖
     * ========================================================
     */
    S2_DebounceProcess();


    /*
     * ========================================================
     * Step 2
     *
     * CAN超时
     * ========================================================
     */

    if (g_s2_timer.can_timeout_ms >=
        g_s2_config.can_timeout_ms)
    {
        /*
         * CAN丢失：
         *
         * 正在充电时禁止继续维持功率。
         */
        if (g_s2_output.state ==
            S2_STATE_CLOSED)
        {
            S2_HW_StopPower();


            /*
             * BMS主导：
             * BMS通信失联必须停止充电。
             */
            if (g_s2_config.scheme ==
                S2_SCHEME_BMS_MASTER)
            {
                g_s2_output.open_reason =
                    S2_OR2_CHARGE_NOT_ALLOWED;

                g_s2_output.state =
                    S2_STATE_STOP_POWER;
            }


            /*
             * OBC主导：
             * OBC通信异常同样进入安全路径。
             */
            else if (g_s2_config.scheme ==
                     S2_SCHEME_OBC_MASTER)
            {
                g_s2_output.open_reason =
                    S2_OR2_CHARGE_NOT_ALLOWED;

                g_s2_output.state =
                    S2_STATE_STOP_POWER;
            }
        }
    }


    /*
     * ========================================================
     * Step 3
     *
     * 状态机
     * ========================================================
     */

    S2_StateMachine();


    /*
     * ========================================================
     * Step 4
     *
     * CAN状态发送
     * ========================================================
     */

    S2_CAN_SendStatus();
}


/* ============================================================
 * 状态获取
 * ============================================================ */

const S2_Output_t *S2_GetOutput(void)
{
    return &g_s2_output;
}


S2_State_e S2_GetState(void)
{
    return g_s2_output.state;
}


S2_OpenReason_e S2_GetOpenReason(void)
{
    return g_s2_output.open_reason;
}


S2_Fault_e S2_GetFault(void)
{
    return g_s2_output.fault;
}


/* ============================================================
 * HAL
 *
 * 实际项目替换成MCAL/GPIO驱动。
 * ============================================================ */

void S2_HW_Close(void)
{
    /*
     * TODO:
     *
     * S2_GPIO = 1;
     */
}


void S2_HW_Open(void)
{
    /*
     * TODO:
     *
     * S2_GPIO = 0;
     */
}


bool S2_HW_GetFeedback(void)
{
    /*
     * TODO:
     *
     * 读取S2辅助触点。
     *
     * true  = S2闭合
     * false = S2打开
     */

    return g_s2_gpio.s2_feedback;
}


void S2_HW_StopPower(void)
{
    /*
     * TODO:
     *
     * 调用PFC/DCDC功率级停止接口。
     *
     * 例如：
     *
     * PFC_Stop();
     * DCDC_Stop();
     *
     * 注意：
     * S2_Control不应该直接操作PFC/DCDC内部状态机。
     */
}


/* ============================================================
 * CAN
 * ============================================================ */

void S2_CAN_SendStatus(void)
{
    /*
     * TODO：
     *
     * 上报：
     *
     * S2_State
     * S2_Closed
     * S2_Open
     * S2_OpenReason
     * S2_Fault
     * S2_PowerStop
     */
}


void S2_CAN_SendCloseResult(void)
{
    /*
     * TODO
     */
}


void S2_CAN_SendOpenResult(void)
{
    /*
     * TODO
     */
}