/*
 * AUTOSAR communication management interaction reference.
 *
 * These are intentionally pseudocode-like wrappers. In a real project:
 *
 * Application/RTE
 *      |
 *      v
 * ComM_RequestComMode()
 *      |
 *      v
 * CanSM_RequestComMode()
 *      |
 *      v
 * CanIf_SetControllerMode()
 *      |
 *      v
 * CanDrv
 *
 * BswM evaluates ComM/CanSM/Nm states and performs mode rules.
 */

void App_RequestCanFullCom(void)
{
    /* ComM_RequestComMode(NetworkHandle, COMM_FULL_COMMUNICATION); */
}

void App_ReleaseCanCom(void)
{
    /* ComM_RequestComMode(NetworkHandle, COMM_NO_COMMUNICATION); */
}

void BswM_CanModeRuleAction(void)
{
    /* Evaluate:
     * CanSM state
     * ComM state
     * Nm state
     * ECU power mode
     * Then enable/disable application communication.
     */
}
