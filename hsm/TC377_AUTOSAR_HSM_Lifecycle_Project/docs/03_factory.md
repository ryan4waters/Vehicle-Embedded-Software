# Factory / Production

## Typical sequence

1. Program HSM firmware / boot components
2. Program bootloader
3. Program application
4. Provision device identity
5. Provision authorized key material / certificate
6. Validate key state
7. Validate secure boot
8. Validate diagnostic authentication
9. Validate CAN SecOC
10. Validate update/recovery
11. Record device security state
12. Apply production security configuration

## Example

Factory Station
      |
      | authenticated provisioning
      v
TC377
      |
      +--> Device ID
      +--> Certificate
      +--> SecOC key
      +--> Diagnostic key
      +--> Boot public key
      |
      v
HSM Key Store
      |
      v
Production Lock

Do not implement factory provisioning as a generic "write secret bytes" API exposed to the normal application.
