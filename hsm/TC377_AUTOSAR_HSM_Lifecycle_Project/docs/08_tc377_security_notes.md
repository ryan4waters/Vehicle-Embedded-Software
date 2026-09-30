# TC377 Security Notes

TC37xx HSM is a dedicated security subsystem. Exact HSM firmware and low-level interfaces are project/toolchain dependent.

Infineon community guidance identifies `UCB_HSMCOTP0/1` as responsible for HSM PFlash locking on TC37xx.

Therefore:
- keep HSM HAL isolated
- separate development vs production UCB configuration
- maintain a tested recovery procedure
- never experiment with production locking on the only ECU/debug path
- keep HSM firmware and application image ownership separate

Some HSM details may be available only through Infineon/FAE or licensed security documentation. The project therefore avoids inventing undocumented TC377 HSM command registers.
