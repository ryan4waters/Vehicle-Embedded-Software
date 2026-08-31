#include "s2_control.h"


/* ============================================================
 * F1方案
 * ============================================================ */


/* ============================================================
 * F1闭合
 *
 * F1保持现有X/F逻辑框架。
 * ============================================================ */

bool S2_F1_CheckClose(void)
{
    extern S2_CanInput_t g_s2_input;

    S2_CanInput_t *in;

    in = &g_s2_input;


    /*
     * CC全连接
     */
    if (in->cc_state != S2_CC_STATE_FULL_CONNECT)
    {
        return false;
    }


    /*
     * CP有效
     */
    if (!in->cp_valid)
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
     * 车辆/充电请求
     */
    if (!in->obc_charge_request &&
        !in->bms_charge_request)
    {
        return false;
    }


    /*
     * 电子锁锁止
     */
    if (!in->electronic_lock_locked)
    {
        return false;
    }


    /*
     * OBC无不可恢复故障
     */
    if (in->obc_irrecoverable_fault)
    {
        return false;
    }


    return true;
}


/* ============================================================
 * F1断开
 * ============================================================ */

bool S2_F1_CheckOpen(S2_OpenReason_e *reason)
{
    /*
     * F1公共断开逻辑：
     *
     * OR1
     * OR2
     * OR3
     * OR4
     * OR5
     * OR6
     * OR7
     *
     * 统一由S2_Control.c处理。
     */

    if (reason == NULL)
    {
        return false;
    }

    return false;
}