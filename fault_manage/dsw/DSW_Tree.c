#include "DSW_Tree.h"

static bool DSW_IsFwRegistered(const DSW_NodeTypeDef *node,
                               const DSW_FaultWordType *fw)
{
    uint8_t i;
    if ((node == NULL) || (fw == NULL)) { return false; }

    for (i = 0u; i < node->faultCount; ++i)
    {
        if (node->faultWords[i] == fw) { return true; }
    }
    return false;
}

void DSW_Init(void)
{
    /* Configuration/NVM integration is intentionally external. */
}

static void DSW_ClearReportInfo(DSW_ReportInfoType *info)
{
    if (info != NULL)
    {
        info->result = DSW_RESULT_OK;
        info->blockingNode = NULL;
        info->blockingFw = NULL;
    }
}

DSW_ResultType DSW_ReportFault(DSW_NodeTypeDef *node,
                               DSW_FaultWordType *fw)
{
    return DSW_ReportFaultEx(node, fw, NULL);
}

DSW_ResultType DSW_ReportFaultEx(DSW_NodeTypeDef *node,
                                 DSW_FaultWordType *fw,
                                 DSW_ReportInfoType *info)
{
    uint8_t i;
    const DSW_NodeTypeDef *blockingParent;
    const DSW_FaultWordType *blockingFw;

    DSW_ClearReportInfo(info);

    if ((node == NULL) || (fw == NULL))
    {
        if (info != NULL) { info->result = DSW_RESULT_NULL_POINTER; }
        return DSW_RESULT_NULL_POINTER;
    }

    if (!DSW_IsFwRegistered(node, fw))
    {
        if (info != NULL) { info->result = DSW_RESULT_FW_NOT_REGISTERED; }
        return DSW_RESULT_FW_NOT_REGISTERED;
    }

    if (!DSW_FwIsMonitorEnable(fw))
    {
        if (info != NULL) { info->result = DSW_RESULT_MONITOR_DISABLED; }
        return DSW_RESULT_MONITOR_DISABLED;
    }

    blockingParent = DSW_NodeGetFirstFaultParent(node);
    if (blockingParent != NULL)
    {
        blockingFw = DSW_NodeGetFirstFaultParentFw(node);
        if (info != NULL)
        {
            info->result = DSW_RESULT_PARENT_FAULT;
            info->blockingNode = blockingParent;
            info->blockingFw = blockingFw;
        }
        return DSW_RESULT_PARENT_FAULT;
    }

    for (i = 0u; i < node->faultCount; ++i)
    {
        if ((node->faultWords[i] != fw) &&
            DSW_FwIsCurrentFault(node->faultWords[i]))
        {
            if (info != NULL)
            {
                info->result = DSW_RESULT_NODE_ALREADY_FAULT;
                info->blockingNode = node;
                info->blockingFw = node->faultWords[i];
            }
            return DSW_RESULT_NODE_ALREADY_FAULT;
        }
    }

    DSW_FwSetCurrentFault(fw);
    DSW_FwSetDiagDone(fw);
    DSW_FwSetHistoryFault(fw);

    return DSW_RESULT_OK;
}

DSW_ResultType DSW_ClearFault(DSW_FaultWordType *fw)
{
    if (fw == NULL) { return DSW_RESULT_NULL_POINTER; }
    DSW_FwClearCurrentFault(fw);
    return DSW_RESULT_OK;
}

void DSW_SetDiagDone(DSW_FaultWordType *fw)
{
    DSW_FwSetDiagDone(fw);
}

void DSW_ClearDiagDone(DSW_FaultWordType *fw)
{
    DSW_FwClearDiagDone(fw);
}

void DSW_SetMonitorEnable(DSW_FaultWordType *fw)
{
    DSW_FwSetMonitorEnable(fw);
}

void DSW_SetMonitorDisable(DSW_FaultWordType *fw)
{
    DSW_FwClearMonitorEnable(fw);
}

bool DSW_IsNodeFault(const DSW_NodeTypeDef *node)
{
    return DSW_NodeHasCurrentFault(node);
}

bool DSW_IsParentFault(const DSW_NodeTypeDef *node)
{
    return DSW_NodeHasParentFault(node);
}

const DSW_NodeTypeDef *DSW_GetBlockingParent(const DSW_NodeTypeDef *node)
{
    return DSW_NodeGetFirstFaultParent(node);
}

const DSW_FaultWordType *DSW_GetBlockingParentFw(const DSW_NodeTypeDef *node)
{
    return DSW_NodeGetFirstFaultParentFw(node);
}
