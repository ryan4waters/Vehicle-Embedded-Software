#include "s2_control.h"


/* ============================================================
 * BMS主导方案
 * ============================================================ */


/* ============================================================
 * BMS主导闭合
 *
 * BMS负责前置条件：
 *
 * AND1 CC全连接
 * AND2 CP PWM有效
 * AND3 电池组可充电
 * AND6 OBC无故障
 *
 * 全部满足后：
 *
 * BMS -> S2闭合动作指令
 * ============================================================ */

bool S2_BMS_Master_CheckClose(void)
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
    if (!in->bms_cp_valid &&
        !in->obc_cp_valid)
    {
        return false;
    }


    /*
     * AND3：
     * 电池组可充电
     */
    if (!in->bms_charge_allow)
    {
        return false;
    }


    /*
     * AND6：
     * OBC无故障
     */
    if (!in->bms_obc_no_fault)
    {
        return false;
    }


    /*
     * BMS必须已经下发S2闭合请求
     */
    if (!in->bms_s2_close_request)
    {
        return false;
    }


    return true;
}


/* ============================================================
 * BMS主导断开
 *
 * BMS满足OR1~OR5后：
 *
 * BMS -> 下发S2断开动作指令
 *
 * OR6由OBC保留自主断开能力。
 * ============================================================ */

bool S2_BMS_Master_CheckOpen(S2_OpenReason_e *reason)
{
    extern S2_CanInput_t g_s2_input;

    S2_CanInput_t *in;

    in = &g_s2_input;


    if (reason == NULL)
    {
        return false;
    }


    /*
     * BMS主动下发S2断开
     */
    if (in->bms_s2_open_request)
    {
        *reason = S2_OR7_OPEN_REQUEST;

        return true;
    }


    /*
     * BMS检测CC断开
     */
    if (in->bms_cc_disconnect)
    {
        *reason = S2_OR4_CC_DISCONNECT;

        return true;
    }


    /*
     * BMS检测CC半连接
     */
    if (in->bms_cc_half)
    {
        *reason = S2_OR3_CC_HALF_CONNECT;

        return true;
    }


    /*
     * BMS收到CP无效
     */
    if (!in->bms_cp_valid)
    {
        *reason = S2_OR5_CP_INVALID;

        return true;
    }


    /*
     * BMS禁止充电
     */
    if (!in->bms_charge_allow)
    {
        *reason = S2_OR2_CHARGE_NOT_ALLOWED;

        return true;
    }


    return false;
}