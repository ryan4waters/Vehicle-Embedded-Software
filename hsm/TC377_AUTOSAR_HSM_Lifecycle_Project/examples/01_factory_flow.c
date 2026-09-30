#include "FactorySecurity.h"

void Example_Factory(void)
{
    FactorySecurity_ProvisionDevice(0);
    FactorySecurity_RunEndOfLineSecurityTest();
    FactorySecurity_ProductionLockPreCheck();
}
