#include "SecurityPolicy.h"

void Test_SecurityPolicy(void)
{
    SecurityPolicy_SetPhase(SEC_PHASE_RUNTIME);

    /* Expected:
     * SecOC allowed
     * diagnostic unlock allowed
     * key provisioning rejected
     * factory key operations rejected
     */
}
