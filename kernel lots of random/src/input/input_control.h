#pragma once
#include <ntifs.h>
#include <ntddmou.h>
#include <ntdd8042.h>
#include "../common/types.h"
#include "../stealth/memory_protection.h"

// Device context structure
typedef struct _INPUT_DEVICE_CONTEXT {
    PDEVICE_OBJECT MouseDevice;
    PDEVICE_OBJECT KeyboardDevice;
    KSPIN_LOCK InputLock;
    BOOLEAN IsInitialized;
} INPUT_DEVICE_CONTEXT, *PINPUT_DEVICE_CONTEXT;

// Mouse movement structure with flags for kernel mode
typedef struct _MOUSE_INPUT {
    LONG DeltaX;
    LONG DeltaY;
    union {
        USHORT ButtonFlags;
        struct {
            USHORT LeftButton   : 1;
            USHORT RightButton  : 1;
            USHORT MiddleButton : 1;
            USHORT Reserved     : 13;
        };
    };
    KIRQL PreviousIrql;
} MOUSE_INPUT, *PMOUSE_INPUT;

// Keyboard input structure with kernel-specific fields
typedef struct _KEYBOARD_INPUT {
    USHORT VirtualKey;
    USHORT ScanCode;
    BOOLEAN KeyDown;
    BOOLEAN Extended;
    KIRQL PreviousIrql;
} KEYBOARD_INPUT, *PKEYBOARD_INPUT;

// Input protection context structure
typedef struct _INPUT_PROTECTION_CONTEXT {
    KSPIN_LOCK InputLock;
    BOOLEAN IsProtected;
    PSTEALTH_PROTECTION_STATE StealthState;
} INPUT_PROTECTION_CONTEXT, *PINPUT_PROTECTION_CONTEXT;

// Function declarations
NTSTATUS InitializeInputControl(PDRIVER_OBJECT DriverObject);
void CleanupInputControl(void);

// Mouse control (kernel mode)
NTSTATUS MoveMouse(LONG DeltaX, LONG DeltaY, KIRQL* PreviousIrql);
NTSTATUS ClickMouse(BOOLEAN LeftButton, BOOLEAN IsDown, KIRQL* PreviousIrql);

// Keyboard control (kernel mode)
NTSTATUS SendKey(USHORT VirtualKey, BOOLEAN KeyDown, KIRQL* PreviousIrql);
NTSTATUS SendCharacter(WCHAR Character, KIRQL* PreviousIrql);

// Internal helper functions
NTSTATUS ConnectToClassDriver(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
void DisconnectFromClassDriver(void); 