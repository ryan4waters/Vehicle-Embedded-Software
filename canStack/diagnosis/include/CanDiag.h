#ifndef CAN_DIAG_H
#define CAN_DIAG_H
#include <stdint.h>
#include <stdbool.h>
typedef enum { CANDIAG_BUSOFF=0, CANDIAG_ERROR_PASSIVE, CANDIAG_TRCV_UV, CANDIAG_TRCV_OT, CANDIAG_TRCV_TXD_DOM, CANDIAG_RX_TIMEOUT, CANDIAG_TX_TIMEOUT, CANDIAG_NM_TIMEOUT, CANDIAG_EVENT_COUNT } CanDiag_EventType;
typedef struct { bool busOff; bool errorPassive; uint16_t tec; uint16_t rec; bool trcvUv; bool trcvOt; bool trcvTxdDom; bool rxTimeout; bool txTimeout; bool nmTimeout; } CanDiag_StatusType;
void CanDiag_Init(void); void CanDiag_MainFunction_10ms(void); void CanDiag_ControllerBusOff(void); void CanDiag_ControllerErrorPassive(uint16_t tec,uint16_t rec); void CanDiag_RxIndication(uint16_t pduId); void CanDiag_TxConfirmation(uint16_t pduId); void CanDiag_NmRxIndication(void); void CanDiag_GetStatus(CanDiag_StatusType *s); uint8_t CanDiag_GetDtcStatus(CanDiag_EventType e);
#endif
