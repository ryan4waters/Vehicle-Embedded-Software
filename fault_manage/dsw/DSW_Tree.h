#ifndef DSW_TREE_H
#define DSW_TREE_H

#include <stdint.h>
#include <stdbool.h>
#include "DSW_Node.h"

typedef enum
{
    DSW_RESULT_OK = 0,
    DSW_RESULT_NULL_POINTER,
    DSW_RESULT_PARENT_FAULT,
    DSW_RESULT_NODE_ALREADY_FAULT,
    DSW_RESULT_FW_NOT_REGISTERED,
    DSW_RESULT_MONITOR_DISABLED,
    DSW_RESULT_INVALID_ARGUMENT
} DSW_ResultType;

typedef struct
{
    DSW_ResultType result;
    const DSW_NodeTypeDef *blockingNode;
    const DSW_FaultWordType *blockingFw;
} DSW_ReportInfoType;

void DSW_Init(void);

DSW_ResultType DSW_ReportFault(DSW_NodeTypeDef *node,
                               DSW_FaultWordType *fw);

DSW_ResultType DSW_ReportFaultEx(DSW_NodeTypeDef *node,
                                 DSW_FaultWordType *fw,
                                 DSW_ReportInfoType *info);

DSW_ResultType DSW_ClearFault(DSW_FaultWordType *fw);
void DSW_SetDiagDone(DSW_FaultWordType *fw);
void DSW_ClearDiagDone(DSW_FaultWordType *fw);
void DSW_SetMonitorEnable(DSW_FaultWordType *fw);
void DSW_SetMonitorDisable(DSW_FaultWordType *fw);

bool DSW_IsNodeFault(const DSW_NodeTypeDef *node);
bool DSW_IsParentFault(const DSW_NodeTypeDef *node);
const DSW_NodeTypeDef *DSW_GetBlockingParent(const DSW_NodeTypeDef *node);
const DSW_FaultWordType *DSW_GetBlockingParentFw(const DSW_NodeTypeDef *node);

#endif
