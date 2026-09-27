/*
 * CAN NM interaction reference.
 *
 * Receive:
 * CanDrv
 *   -> CanIf_RxIndication
 *      -> CanNm_RxIndication
 *         -> Nm state machine
 *            -> ComM/BswM
 *
 * Transmit:
 * CanNm
 *   -> PduR/CanIf
 *      -> CanDrv
 */

void CanNm_RxIndication_Ref(const uint8_t *nmPdu, uint8_t len)
{
    (void)nmPdu;
    (void)len;

    /* CanNm_RxIndication(NetworkHandle, PduInfoPtr); */
}

void CanNm_MainFunction_Ref(void)
{
    /* Repeat Message / Normal Operation / Ready Sleep / Bus Sleep */
}
