#ifndef OTA_H
#define OTA_H

// Accept password-protected firmware updates over Wi-Fi (off when no password is set)
void setupOta(void (*onStart)());

// Handle a pending update (call on every loop iteration)
void handleOta();

#endif