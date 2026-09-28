#include "DSW_Fault.h"

void DSW_FwInit(DSW_FaultWordType *fw)
{
    if (fw != NULL) { *fw = 0u; }
}

void DSW_FwSetCurrentFault(DSW_FaultWordType *fw)
{
    if (fw != NULL) { *fw |= DSW_FW_CURRENT_FAULT; }
}

void DSW_FwClearCurrentFault(DSW_FaultWordType *fw)
{
    if (fw != NULL) { *fw &= (uint8_t)(~DSW_FW_CURRENT_FAULT); }
}

void DSW_FwSetDiagDone(DSW_FaultWordType *fw)
{
    if (fw != NULL) { *fw |= DSW_FW_DIAG_DONE; }
}

void DSW_FwClearDiagDone(DSW_FaultWordType *fw)
{
    if (fw != NULL) { *fw &= (uint8_t)(~DSW_FW_DIAG_DONE); }
}

void DSW_FwSetHistoryFault(DSW_FaultWordType *fw)
{
    if (fw != NULL) { *fw |= DSW_FW_HISTORY_FAULT; }
}

void DSW_FwClearHistoryFault(DSW_FaultWordType *fw)
{
    if (fw != NULL) { *fw &= (uint8_t)(~DSW_FW_HISTORY_FAULT); }
}

void DSW_FwSetMonitorEnable(DSW_FaultWordType *fw)
{
    if (fw != NULL) { *fw |= DSW_FW_MONITOR_ENABLE; }
}

void DSW_FwClearMonitorEnable(DSW_FaultWordType *fw)
{
    if (fw != NULL) { *fw &= (uint8_t)(~DSW_FW_MONITOR_ENABLE); }
}

bool DSW_FwIsCurrentFault(const DSW_FaultWordType *fw)
{
    return ((fw != NULL) && ((*fw & DSW_FW_CURRENT_FAULT) != 0u));
}

bool DSW_FwIsDiagDone(const DSW_FaultWordType *fw)
{
    return ((fw != NULL) && ((*fw & DSW_FW_DIAG_DONE) != 0u));
}

bool DSW_FwIsHistoryFault(const DSW_FaultWordType *fw)
{
    return ((fw != NULL) && ((*fw & DSW_FW_HISTORY_FAULT) != 0u));
}

bool DSW_FwIsMonitorEnable(const DSW_FaultWordType *fw)
{
    return ((fw != NULL) && ((*fw & DSW_FW_MONITOR_ENABLE) != 0u));
}
