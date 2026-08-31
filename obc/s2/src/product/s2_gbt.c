#include "s2_control.h"


/* ============================================================
 * GBT方案
 *
 * GB/T 18487.1-2015
 * ============================================================ */


/* ============================================================
 * GBT闭合
 *
 * GBT方案按照文件中的AND条件执行：
 *
 * AND1 CC全连接
 * AND2 CP PWM有效
 * AND3 电池组可充电
 * AND4 车辆充电请求
 * AND5 电子锁锁止
 * AND6 OBC无故障
 * ============================================================ */

bool S2_GBT_CheckClose(void)
{
    const S2_CanInput_t *in;

    /*
     * 输入由S2_Control.c内部保存，
     * 因此通过公共接口获取当前状态。
     *
     * 实际工程可以将g_s2_input改成getter。
     */
    extern S2_CanInput_t g_s2_input;

    in = &g_s2_input;


    /* AND1 */
    if (in->cc_state != S2_CC_STATE_FULL_CONNECT)
    {
        return false;
    }


    /* AND2 */
    if (!in->cp_valid)
    {
        return false;
    }


    /* AND3 */
    if (!in->bms_charge_allow)
    {
        return false;
    }


    /* AND4 */
    if (!in->obc_charge_request &&
        !in->bms_charge_request)
    {
        return false;
    }


    /* AND5 */
    if (!in->electronic_lock_locked)
    {
        return false;
    }


    /* AND6 */
    if (in->obc_irrecoverable_fault)
    {
        return false;
    }


    return true;
}


/* ============================================================
 * GBT断开
 * ============================================================ */

bool S2_GBT_CheckOpen(S2_OpenReason_e *reason)
{
    extern S2_CanInput_t g_s2_input;

    /*
     * GBT公共OR1~OR7由S2_Control.c统一执行。
     *
     * 本函数预留GBT未来特有需求。
     */

    (void)g_s2_input;

    if (reason == NULL)
    {
        return false;
    }

    return false;
}