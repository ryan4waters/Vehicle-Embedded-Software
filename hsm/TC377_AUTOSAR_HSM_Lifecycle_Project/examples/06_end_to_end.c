/*
 * End-to-end lifecycle concept:
 *
 * Factory:
 *   provision -> EOL security test -> production lock
 *
 * Startup:
 *   HSM ready -> hash -> signature verify -> anti rollback -> boot
 *
 * Runtime:
 *   CAN -> SecOC -> CSM -> CryIf -> Crypto -> HSM -> CMAC
 *
 * Update:
 *   UDS 0x10 -> 0x27 -> 0x34 -> 0x36 -> 0x37
 *   -> hash -> signature -> anti rollback -> commit -> reset
 *
 * After-sales:
 *   authenticated tester -> limited authorization -> secure update
 *   -> reboot -> secure boot
 */
void Example_EndToEnd(void)
{
}
