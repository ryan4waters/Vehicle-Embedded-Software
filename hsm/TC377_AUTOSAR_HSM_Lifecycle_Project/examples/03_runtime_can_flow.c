#include "SecOC_Runtime.h"

void Example_Can(void)
{
    SecCanFrameType frame = {
        0x180u,
        8u,
        {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08}
    };

    Hsm_U8 mac[16];
    Hsm_U32 macLen = sizeof(mac);

    (void)SecOC_RuntimeTx(
        &frame, 1234u, mac, &macLen);
}
