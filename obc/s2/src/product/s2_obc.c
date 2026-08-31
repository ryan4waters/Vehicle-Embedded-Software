#include "s2_control.h"


/* ============================================================
 * OBC主导方案
 * ============================================================ */


/* ============================================================
 * OBC主导闭合
 *
 * OBC负责：
 *
 * AND1 CC全连接
 * AND2 CP有效
 * AND5 电子锁
 * AND6 OBC无故障
 * AND7 OBC不满足休眠
 *
 * 同时：
 *
 * BMS -> 充电允许
 *
 * OBC收到BMS充电允许后：
 *
 *     闭合S2
 * ============================================================ */

bool S2_OBC_Master_CheckClose(void)
{
    extern S2_CanInput_t g_s2_input;

    S2_CanInput_t *in;

    in = &g_s2_input;


    /*
     * AND1：
     * CC全连接
     */
    if (in->cc_state != S2_CC_STATE_FULL_CONNECT)
    {
        return false;
    }


    /*
     * AND2：
     * CP有效
     */
    if (!in->obc_cp_valid &&
        !in->bms_cp_valid)
    {
        return false;
    }


    /*
     * AND5：
     * 电子锁锁止
     */
    if (!in->electronic_lock_locked)
    {
        return false;
    }


    /*
     * AND6：
     * OBC无故障
     */
    if (in->obc_irrecoverable_fault)
    {
        return false;
    }


    /*
     * AND7：
     * OBC不满足休眠条件
     */
    if (in->obc_sleep_condition)
    {
        return false;
    }


    /*
     * OBC收到车辆充电请求
     */
    if (!in->obc_charge_request)
    {
        return false;
    }


    /*
     * BMS允许充电
     */
    if (!in->bms_charge_allow)
    {
        return false;
    }


    /*
     * OBC必须已经下发S2闭合请求
     */
    if (!in->obc_s2_close_request)
    {
        return false;
    }


    return true;
}


/* ============================================================
 * OBC主导断开
 *
 * OBC主导：
 *
 * OR1~OR6
 * 或收到S2断开指令
 *
 * OBC均可以断开S2。
 * ============================================================ */

bool S2_OBC_Master_CheckOpen(S2_OpenReason_e *reason)
{
    extern S2_CanInput_t g_s2_input;

    S2_CanInput_t *in;

    in = &g_s2_input;


    if (reason == NULL)
    {
        return false;
    }


    /*
     * OR7：
     * 收到S2断开请求
     */
    if (in->obc_s2_open_request)
    {
        *reason = S2_OR7_OPEN_REQUEST;

        return true;
    }


    /*
     * OR2：
     * BMS充电不允许
     */
    if (!in->bms_charge_allow)
    {
        *reason = S2_OR2_CHARGE_NOT_ALLOWED;

        return true;
    }


    /*
     * OR4：
     * CC断开
     */
    if (in->obc_cc_disconnect)
    {
        *reason = S2_OR4_CC_DISCONNECT;

        return true;
    }


    /*
     * OR3：
     * CC半连接
     */
    if (in->obc_cc_half)
    {
        *reason = S2_OR3_CC_HALF_CONNECT;

        return true;
    }


    /*
     * OR5：
     * CP无效
     */
    if (!in->obc_cp_valid)
    {
        *reason = S2_OR5_CP_INVALID;

        return true;
    }


    /*
     * OR6：
     * OBC满足休眠条件
     */
    if (in->obc_sleep_condition)
    {
        *reason = S2_OR6_OBC_SLEEP;

        return true;
    }


    /*
     * OBC不可恢复故障
     */
    if (in->obc_irrecoverable_fault)
    {
        *reason = S2_OR2_CHARGE_NOT_ALLOWED;

        return true;
    }


    /*
     * OBC充电中止
     */
    if (in->obc_charge_abort)
    {
        *reason = S2_OR2_CHARGE_NOT_ALLOWED;

        return true;
    }


    return false;
}