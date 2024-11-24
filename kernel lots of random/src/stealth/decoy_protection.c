#include "decoy_protection.h"
#include "crypto_utils.h"

// Common instruction patterns for x64
static const UCHAR g_CommonInstructions[] = {
    0x48, 0x89, 0x5C, 0x24, 0x08,  // mov [rsp+8], rbx
    0x48, 0x83, 0xEC, 0x20,        // sub rsp, 20h
    0xE8,                          // call
    0xFF, 0x25,                    // jmp qword ptr []
    0x48, 0x8B, 0x05,             // mov rax, []
    0xC3,                          // ret
    0x90                           // nop
};

// Add new structure for behavioral patterns
static const DECOY_BEHAVIOR_PATTERN g_BehaviorPatterns[] = {
    { PATTERN_CODE, 0x40, TRUE },    // Code-like sequences
    { PATTERN_DATA, 0x30, FALSE },   // Data structures
    { PATTERN_HEAP, 0x20, TRUE }     // Heap-like patterns
};

static NTSTATUS GenerateDecoyContent(PVOID BaseAddress, SIZE_T Size) {
    PUCHAR buffer = (PUCHAR)BaseAddress;
    ULONG seed = KeQueryTimeIncrement();
    
    // Create fake PE header at the beginning
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)buffer;
    dosHeader->e_magic = IMAGE_DOS_SIGNATURE;
    dosHeader->e_lfanew = 0x80;  // Standard offset

    // Enhanced content generation with behavioral patterns
    for (SIZE_T i = sizeof(IMAGE_DOS_HEADER); i < Size; i++) {
        ULONG choice = RtlRandomEx(&seed) % 100;
        
        if (choice < 25) {
            // Insert realistic function prologue/epilogue
            if (i + 16 < Size) {
                GenerateFunctionFrame(&buffer[i]);
                i += 15;
            }
        }
        else if (choice < 50) {
            // Create fake vtable structures
            if (i + sizeof(PVOID) * 4 < Size) {
                GenerateVTableStructure(&buffer[i]);
                i += sizeof(PVOID) * 4 - 1;
            }
        }
        else if (choice < 75) {
            // Insert common instruction pattern
            ULONG patternIdx = RtlRandomEx(&seed) % sizeof(g_CommonInstructions);
            buffer[i] = g_CommonInstructions[patternIdx];
        }
        else if (choice < 100) {
            // Insert potential pointer-like values
            *(PULONG_PTR)&buffer[i] = (ULONG_PTR)MmHighestUserAddress - 
                                     (RtlRandomEx(&seed) % 0x100000);
            i += sizeof(ULONG_PTR) - 1;
        }
        else {
            // Random data
            buffer[i] = (UCHAR)(RtlRandomEx(&seed));
        }
    }
    
    // Add entropy analysis to make content look more realistic
    AdjustContentEntropy(buffer, Size);
    
    return STATUS_SUCCESS;
}

static VOID MonitorDecoyAccess(PDECOY_REGION Decoy) {
    // Enhanced monitoring with callbacks
    Decoy->AccessCallback = DecoyAccessHandler;
    Decoy->TamperCallback = DecoyTamperHandler;
    
    // Set up advanced protection
    MEMORY_BASIC_INFORMATION mbi;
    if (NT_SUCCESS(ZwQueryVirtualMemory(...))) {
        // Add hardware breakpoints for critical sections
        SetHardwareBreakpoint(Decoy->BaseAddress, Decoy->Size);
    }
    
    // Set up page guard protection
    ULONG oldProtect;
    VirtualProtect(Decoy->BaseAddress, 
                  Decoy->Size, 
                  PAGE_GUARD | PAGE_READWRITE,
                  &oldProtect);
}

NTSTATUS InitializeDecoys(PSTEALTH_CONTEXT Context) {
    InitializeListHead(&Context->DecoyList);
    ULONG seed = KeQueryTimeIncrement();
    
    // Create random number of decoys
    ULONG numDecoys = 4 + (RtlRandomEx(&seed) % (MAX_DECOY_REGIONS - 4));
    
    for (ULONG i = 0; i < numDecoys; i++) {
        PDECOY_REGION decoy = ExAllocatePoolWithTag(NonPagedPool, 
                                                   sizeof(DECOY_REGION),
                                                   'yocD');
        if (!decoy) continue;

        // Random size between MIN and MAX
        SIZE_T size = MIN_DECOY_SIZE + 
                     (RtlRandomEx(&seed) % (MAX_DECOY_SIZE - MIN_DECOY_SIZE));
        
        decoy->BaseAddress = ExAllocatePoolWithTag(NonPagedPool, size, 'yocD');
        if (!decoy->BaseAddress) {
            ExFreePoolWithTag(decoy, 'yocD');
            continue;
        }

        decoy->Size = size;
        decoy->LastModifiedTick = KeQueryTimeIncrement();
        decoy->AccessCount = 0;
        decoy->IsActive = TRUE;

        // Generate initial content
        GenerateDecoyContent(decoy->BaseAddress, size);
        
        // Set up monitoring
        MonitorDecoyAccess(decoy);
        
        // Add to list
        InsertTailList(&Context->DecoyList, &decoy->ListEntry);
    }
    
    // Add polymorphic behavior
    Context->PolymorphicTimer = CreatePolymorphicTimer(Context);
    
    // Initialize threat detection
    InitializeThreatDetection(Context);
    
    return STATUS_SUCCESS;
}

VOID UpdateDecoyRegions(PSTEALTH_CONTEXT Context) {
    KIRQL oldIrql;
    KeAcquireSpinLock(&Context->ProtectionLock, &oldIrql);
    
    PLIST_ENTRY entry = Context->DecoyList.Flink;
    while (entry != &Context->DecoyList) {
        PDECOY_REGION decoy = CONTAINING_RECORD(entry, DECOY_REGION, ListEntry);
        
        // Periodically modify content
        ULONG currentTick = KeQueryTimeIncrement();
        if (currentTick - decoy->LastModifiedTick > 10000) {
            GenerateDecoyContent(decoy->BaseAddress, decoy->Size);
            decoy->LastModifiedTick = currentTick;
        }
        
        // Randomly relocate some decoys
        if (RtlRandomEx(&seed) % 100 < 10) {
            PVOID newBase = ExAllocatePoolWithTag(NonPagedPool, 
                                                decoy->Size,
                                                'yocD');
            if (newBase) {
                RtlCopyMemory(newBase, decoy->BaseAddress, decoy->Size);
                ExFreePoolWithTag(decoy->BaseAddress, 'yocD');
                decoy->BaseAddress = newBase;
                MonitorDecoyAccess(decoy);
            }
        }
        
        // Add advanced behavioral analysis
        AnalyzeAccessPatterns(Context);
        
        // Implement polymorphic transformations
        if (ShouldTransformDecoys(Context)) {
            TransformDecoyStructures(Context);
        }
        
        // Add correlation analysis between decoys
        CorrelateDecoyActivity(Context);
        
        entry = entry->Flink;
    }
    
    KeReleaseSpinLock(&Context->ProtectionLock, oldIrql);
}

VOID CleanupDecoys(PSTEALTH_CONTEXT Context) {
    KIRQL oldIrql;
    KeAcquireSpinLock(&Context->ProtectionLock, &oldIrql);
    
    while (!IsListEmpty(&Context->DecoyList)) {
        PLIST_ENTRY entry = RemoveHeadList(&Context->DecoyList);
        PDECOY_REGION decoy = CONTAINING_RECORD(entry, DECOY_REGION, ListEntry);
        
        if (decoy->BaseAddress) {
            RtlSecureZeroMemory(decoy->BaseAddress, decoy->Size);
            ExFreePoolWithTag(decoy->BaseAddress, 'yocD');
        }
        
        ExFreePoolWithTag(decoy, 'yocD');
    }
    
    KeReleaseSpinLock(&Context->ProtectionLock, oldIrql);
}
