#ifndef DSW_FAULT_H
#define DSW_FAULT_H

#include <stdint.h>
#include <stdbool.h>

#define DSW_FW_CURRENT_FAULT      ((uint8_t)0x01u) /* Bit0 */
#define DSW_FW_DIAG_DONE          ((uint8_t)0x02u) /* Bit1 */
#define DSW_FW_HISTORY_FAULT      ((uint8_t)0x04u) /* Bit2 */
#define DSW_FW_MONITOR_ENABLE     ((uint8_t)0x08u) /* Bit3 */
#define DSW_FW_RESERVED           ((uint8_t)0x10u) /* Bit4 */

typedef uint8_t DSW_FaultWordType;

void DSW_FwInit(DSW_FaultWordType *fw);
void DSW_FwSetCurrentFault(DSW_FaultWordType *fw);
void DSW_FwClearCurrentFault(DSW_FaultWordType *fw);
void DSW_FwSetDiagDone(DSW_FaultWordType *fw);
void DSW_FwClearDiagDone(DSW_FaultWordType *fw);
void DSW_FwSetHistoryFault(DSW_FaultWordType *fw);
void DSW_FwClearHistoryFault(DSW_FaultWordType *fw);
void DSW_FwSetMonitorEnable(DSW_FaultWordType *fw);
void DSW_FwClearMonitorEnable(DSW_FaultWordType *fw);

bool DSW_FwIsCurrentFault(const DSW_FaultWordType *fw);
bool DSW_FwIsDiagDone(const DSW_FaultWordType *fw);
bool DSW_FwIsHistoryFault(const DSW_FaultWordType *fw);
bool DSW_FwIsMonitorEnable(const DSW_FaultWordType *fw);

#endif
