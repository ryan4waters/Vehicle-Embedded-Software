#include "PDU_FSM_Example.h"

int main(void)
{
    PDU_FSM_Init();

    for (;;) {
        /* Call from the project's scheduler, e.g. 1 ms task. */
        PDU_FSM_Run();
    }

    return 0;
}
