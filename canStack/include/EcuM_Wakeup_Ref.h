#ifndef ECUM_WAKEUP_REF_H
#define ECUM_WAKEUP_REF_H

#include "CanStack_Ref.h"

void EcuM_CheckWakeupSources(void);
void EcuM_ProcessWakeup(CanStack_WakeupSourceType source);

#endif
