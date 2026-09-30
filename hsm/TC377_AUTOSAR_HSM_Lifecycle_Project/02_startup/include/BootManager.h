#ifndef BOOT_MANAGER_H
#define BOOT_MANAGER_H

typedef enum {
    BOOT_INIT = 0,
    BOOT_HSM_WAIT,
    BOOT_VERIFY,
    BOOT_START_APP,
    BOOT_RECOVERY
} BootStateType;

void BootManager_Init(void);
void BootManager_MainFunction(void);

#endif
