#ifndef SENT_CRC_H
#define SENT_CRC_H

#include <stdint.h>

uint8_t Sent_Crc4(const uint8_t *nibbles, uint8_t count);
bool Sent_Crc4Check(const uint8_t *nibbles, uint8_t count, uint8_t received);

#endif
