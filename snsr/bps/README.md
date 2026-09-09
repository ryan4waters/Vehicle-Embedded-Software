# HELLA SENT Accelerator Pedal Software Reference

This package implements a portable accelerator-pedal SENT stack:

HAL -> SENT decoder -> pedal signal conversion -> plausibility/diagnostics -> application API.

Target MCUs:
- Infineon AURIX TC377: native SENT peripheral backend placeholder.
- ST SPC58NN: SENT peripheral backend placeholder.

Important:
1. HELLA pedal variants are customer-configurable. Exact SENT tick time, channel assignment,
   status semantics, data nibble mapping, CRC convention and electrical pinout must be taken
   from the selected HELLA part's interface specification.
2. The generic application layer therefore uses a configurable profile rather than hard-coding
   one HELLA part number.
3. The reference parser implements SENT frame timing and CRC4 over the conventional 3-data-nibble
   short-frame structure. For a dual-output pedal, instantiate two SENT channels.
4. The MCU-specific files intentionally isolate register/MCAL/driver calls. Replace the marked
   TODO hooks with the exact TC377 iLLD/MCAL or SPC5 AUTOSAR/MCAL API used by the project.

Recommended task rates:
- SENT peripheral capture: hardware/ISR/DMA driven
- Pedal processing: 1 kHz
- Diagnostics: 1 kHz with counters/debounce
- Application output: 100 Hz or 1 kHz depending system architecture
