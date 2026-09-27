#include "CanDiag.h"
#include "CanDiag_Cfg.h"
#include "Dem_Adapter.h"
#include "Tja1145_Drv.h"
#include "Tc377_CanHw.h"
static CanDiag_StatusType st; static uint32_t rxT,txT,nmT;
static void dem(void){Dem_SetEventStatus(CANDIAG_BUSOFF,st.busOff?DEM_FAILED:DEM_PASSED);Dem_SetEventStatus(CANDIAG_ERROR_PASSIVE,st.errorPassive?DEM_FAILED:DEM_PASSED);Dem_SetEventStatus(CANDIAG_TRCV_UV,st.trcvUv?DEM_FAILED:DEM_PASSED);Dem_SetEventStatus(CANDIAG_TRCV_OT,st.trcvOt?DEM_FAILED:DEM_PASSED);Dem_SetEventStatus(CANDIAG_TRCV_TXD_DOM,st.trcvTxdDom?DEM_FAILED:DEM_PASSED);Dem_SetEventStatus(CANDIAG_RX_TIMEOUT,st.rxTimeout?DEM_FAILED:DEM_PASSED);Dem_SetEventStatus(CANDIAG_TX_TIMEOUT,st.txTimeout?DEM_FAILED:DEM_PASSED);Dem_SetEventStatus(CANDIAG_NM_TIMEOUT,st.nmTimeout?DEM_FAILED:DEM_PASSED);}
void CanDiag_Init(void){st=(CanDiag_StatusType){0};rxT=txT=nmT=0;}
void CanDiag_ControllerBusOff(void){st.busOff=true;}
void CanDiag_ControllerErrorPassive(uint16_t tec,uint16_t rec){st.tec=tec;st.rec=rec;st.errorPassive=true;}
void CanDiag_RxIndication(uint16_t pdu){if(pdu==CANDIAG_BMS_PDU){rxT=0;st.rxTimeout=false;}}
void CanDiag_TxConfirmation(uint16_t pdu){(void)pdu;txT=0;st.txTimeout=false;}
void CanDiag_NmRxIndication(void){nmT=0;st.nmTimeout=false;}
void CanDiag_MainFunction_10ms(void){uint16_t tec=0,rec=0;bool passive=false;Tja1145_StatusType tr;Tc377_Can_GetErrorState(&tec,&rec,&passive);st.tec=tec;st.rec=rec;st.errorPassive=passive;rxT+=10;txT+=10;nmT+=10;if(rxT>=CANDIAG_RX_TIMEOUT_MS)st.rxTimeout=true;if(txT>=CANDIAG_TX_TIMEOUT_MS)st.txTimeout=true;if(nmT>=CANDIAG_NM_TIMEOUT_MS)st.nmTimeout=true;if(Tja1145_ReadDiagnostic(&tr)){st.trcvUv=tr.undervoltage;st.trcvOt=tr.overtemperature;st.trcvTxdDom=tr.txdDominantTimeout;}dem();}
void CanDiag_GetStatus(CanDiag_StatusType *s){if(s)*s=st;}
uint8_t CanDiag_GetDtcStatus(CanDiag_EventType e){return Dem_GetEventStatus((uint16_t)e);}
