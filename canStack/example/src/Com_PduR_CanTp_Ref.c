/*
 * Application CAN signal:
 *   RTE -> COM -> PduR -> CanIf -> CanDrv
 *
 * Diagnostic:
 *   Dcm -> PduR -> CanTp -> PduR -> CanIf -> CanDrv
 *
 * The two paths must not be confused with CanNm.
 */

void Com_TxSignal_Ref(void)
{
    /* Com_SendSignal(SignalId, &SignalValue); */
}

void Dcm_DiagnosticTx_Ref(void)
{
    /* Dcm -> PduR -> CanTp -> PduR -> CanIf */
}
