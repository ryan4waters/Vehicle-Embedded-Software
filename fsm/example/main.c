#include "PDU_FSM_Example.h"

int main(void)
{
    PDU_FSM_Init();

    for (;;)
    {
        /* 1ms scheduler */
        PDU_FSM_1msTask();

        /*
         * Actual project scheduler should call PDU_FSM_10msTask()
         * every 10 ms.
         */
    }

    return 0;
}
