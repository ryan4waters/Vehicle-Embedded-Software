#include "Sent_Crc.h"

/*
 * SENT CRC4 reference implementation.
 * Polynomial: x^4 + x^3 + x^2 + 1 (0x0D).
 * This routine is kept isolated because some sensor profiles use a different
 * protected-field convention. Validate against the selected HELLA interface spec.
 */
uint8_t Sent_Crc4(const uint8_t *nibbles, uint8_t count)
{
    uint8_t crc = 0x5u;
    uint8_t i;

    for (i = 0u; i < count; i++) {
        uint8_t data = (uint8_t)(nibbles[i] & 0x0Fu);
        uint8_t bit;
        for (bit = 0u; bit < 4u; bit++) {
            uint8_t fb = (uint8_t)(((crc >> 3u) & 1u) ^ ((data >> (3u - bit)) & 1u));
            crc = (uint8_t)((crc << 1u) & 0x0Fu);
            if (fb != 0u) {
                crc ^= 0x0Du;
            }
        }
    }
    return (uint8_t)(crc & 0x0Fu);
}

bool Sent_Crc4Check(const uint8_t *nibbles, uint8_t count, uint8_t received)
{
    return Sent_Crc4(nibbles, count) == (uint8_t)(received & 0x0Fu);
}
