#include "s2_control.h"


/* ============================================================
 * X1方案
 * ============================================================ */


/* ============================================================
 * X1闭合
 *
 * X1方案维持现有X逻辑。
 *
 * 文件给出的核心条件：
 *
 * 1. CC全连接
 * 2. CP有效
 * 3. 电池组可充电
 * 4. 车辆充电请求
 * 5. 电子锁锁止
 * 6. OBC无故障
 * ============================================================ */

bool S2_X1_CheckClose(void)
{
    extern S2_CanInput_t g_s2_input;

    S2_CanInput_t *in;

    in = &g_s2_input;


    /* CC全连接 */
    if (in->cc_state != S2_CC_STATE_FULL_CONNECT)
    {
        return false;
    }


    /* CP有效 */
    if (!in->cp_valid)
    {
        return false;
    }


    /* 电池组可充电 */
    if (!in->bms_charge_allow)
    {
        return false;
    }


    /* 车辆充电请求 */
    if (!in->obc_charge_request &&
        !in->bms_charge_request)
    {
        return false;
    }


    /* 电子锁 */
    if (!in->electronic_lock_locked)
    {
        return false;
    }


    /* OBC无故障 */
    if (in->obc_irrecoverable_fault)
    {
        return false;
    }


    return true;
}


/* ============================================================
 * X1断开
 * ============================================================ */

bool S2_X1_CheckOpen(S2_OpenReason_e *reason)
{
    /*
     * X1的OR1~OR7统一由S2_Control处理。
     *
     * 特殊的X1差异在这里扩展。
     */

    if (reason == NULL)
    {
        return false;
    }

    return false;
}