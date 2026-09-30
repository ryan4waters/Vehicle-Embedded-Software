#include "AfterSalesSecurity.h"

void Example_AfterSales(void)
{
    /* authenticated service operation */
    (void)AfterSales_AuthenticateTester(0, 0);

    /* enter controlled update mode */
    (void)AfterSales_StartSecureUpdate();
}
