#include "input_control.h"
#include "../common/utils.h"

// Mouse input port addresses
#define MOUSE_DATA_PORT      0x60
#define MOUSE_STATUS_PORT    0x64
#define MOUSE_COMMAND_PORT   0x64

// Mouse button flags
#define MOUSE_LEFT_BUTTON    0x01
#define MOUSE_RIGHT_BUTTON   0x02
#define MOUSE_MIDDLE_BUTTON  0x04

// Keyboard controller ports
#define KBD_DATA_PORT        0x60
#define KBD_STATUS_PORT      0x64
#define KBD_COMMAND_PORT     0x64

static PVOID gInputClassDeviceObject = NULL;

NTSTATUS InitializeInputControl(void) {
    LogDebug("Initializing input control system");
    
    // Connect to the mouse class driver
    UNICODE_STRING mouseClassName;
    RtlUnicodeStringInit(&mouseClassName, L"\\Driver\\MouClass");
    
    // Get reference to mouse class driver
    PDRIVER_OBJECT mouseDriver;
    NTSTATUS status = ObReferenceObjectByName(
        &mouseClassName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        0,
        *IoDriverObjectType,
        KernelMode,
        NULL,
        (PVOID*)&mouseDriver
    );
    
    if (!NT_SUCCESS(status)) {
        LogError("Failed to get mouse driver reference: %08X", status);
        return status;
    }
    
    // Store the device object for later use
    gInputClassDeviceObject = mouseDriver->DeviceObject;
    ObDereferenceObject(mouseDriver);
    
    LogInfo("Input control system initialized successfully");
    return STATUS_SUCCESS;
}

void CleanupInputControl(void) {
    LogDebug("Cleaning up input control system");
    gInputClassDeviceObject = NULL;
}

NTSTATUS MoveMouse(LONG DeltaX, LONG DeltaY) {
    if (!gInputClassDeviceObject) {
        return STATUS_DEVICE_NOT_READY;
    }
    
    MOUSE_INPUT mouseInput = {
        .DeltaX = DeltaX,
        .DeltaY = DeltaY,
        .ButtonFlags = 0
    };
    
    // Create and send mouse input IRP
    return SendMouseInput(&mouseInput);
}

NTSTATUS SendKey(USHORT VirtualKey, BOOLEAN KeyDown) {
    KEYBOARD_INPUT keyInput = {
        .VirtualKey = VirtualKey,
        .ScanCode = MapVirtualKeyEx(VirtualKey, 0, NULL),
        .KeyDown = KeyDown
    };
    
    return SendKeyboardInput(&keyInput);
}

// Helper functions
static NTSTATUS SendMouseInput(PMOUSE_INPUT Input) {
    // Create IRP for mouse input
    PIRP irp = IoBuildSynchronousFsdRequest(
        IRP_MJ_INTERNAL_DEVICE_CONTROL,
        gInputClassDeviceObject,
        Input,
        sizeof(MOUSE_INPUT),
        NULL,
        NULL,
        NULL
    );
    
    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    NTSTATUS status = IoCallDriver(gInputClassDeviceObject, irp);
    LogDebug("Mouse input sent: (%d, %d) Status: %08X", 
             Input->DeltaX, Input->DeltaY, status);
    
    return status;
}

static NTSTATUS SendKeyboardInput(PKEYBOARD_INPUT Input) {
    // Similar implementation for keyboard input
    // [Implementation details for keyboard input]
    return STATUS_SUCCESS;
} 