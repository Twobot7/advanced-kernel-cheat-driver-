#include "decoy_patterns.h"
#include "crypto_utils.h"
#include "entropy_analyzer.h"
#include "instruction_set.h"

static VOID TransformCodePattern(PVOID Address, SIZE_T Size) {
    PUCHAR buffer = (PUCHAR)Address;
    LARGE_INTEGER timestamp;
    KeQuerySystemTimePrecise(&timestamp);
    ULONG seed = (ULONG)(timestamp.QuadPart ^ KeGetCurrentProcessorNumber());

    for (SIZE_T i = 0; i < Size; i++) {
        if (IsInstructionBoundary(buffer + i)) {
            switch (GetRandomValue(seed) % 4) {
                case 0:
                    InsertJunkCode(buffer + i, &i);
                    break;
                case 1:
                    SubstituteEquivalentInstruction(buffer + i, &i);
                    break;
                case 2:
                    ReorderInstructions(buffer + i, &i);
                    break;
                case 3:
                    ModifyInstruction(buffer + i, &i);
                    break;
            }
        }
    }
}

static VOID TransformDataStructure(PVOID Address, SIZE_T Size) {
    PULONG_PTR data = (PULONG_PTR)Address;
    SIZE_T count = Size / sizeof(ULONG_PTR);

    ENTROPY_CONTEXT entropy;
    InitializeEntropyContext(&entropy, data, count);

    for (SIZE_T i = 0; i < count; i++) {
        if (IsPointerValue(data[i])) {
            data[i] = TransformPointer(
                ObfuscateWithEntropy(
                    data[i], 
                    &entropy
                )
            );
        }
    }
}

NTSTATUS PolymorphicTransform(PDECOY_REGION Decoy) {
    KIRQL oldIrql;
    KeAcquireSpinLock(&Decoy->TransformLock, &oldIrql);

    ULONG originalChecksum = CalculateChecksum(Decoy->BaseAddress, Decoy->Size);
    
    PVOID tempBuffer = AllocateSecureBuffer(Decoy->Size, 'myoP');
    if (!tempBuffer) {
        KeReleaseSpinLock(&Decoy->TransformLock, oldIrql);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    for (UINT8 round = 0; round < Decoy->TransformationRounds; round++) {
        RtlCopyMemory(tempBuffer, Decoy->BaseAddress, Decoy->Size);
        
        switch (Decoy->Pattern.Type) {
            case PATTERN_CODE:
                TransformCodePattern(tempBuffer, Decoy->Size);
                ValidateCodeIntegrity(tempBuffer, Decoy->Size);
                break;
            case PATTERN_DATA:
                TransformDataStructure(tempBuffer, Decoy->Size);
                ValidateDataStructure(tempBuffer, Decoy->Size);
                break;
        }

        if (!VerifyTransformation(tempBuffer, Decoy->Size)) {
            SecureFreeBuffer(tempBuffer, 'myoP');
            KeReleaseSpinLock(&Decoy->TransformLock, oldIrql);
            return STATUS_UNSUCCESSFUL;
        }

        RtlCopyMemory(Decoy->BaseAddress, tempBuffer, Decoy->Size);
    }

    SecureFreeBuffer(tempBuffer, 'myoP');
    KeReleaseSpinLock(&Decoy->TransformLock, oldIrql);
    return STATUS_SUCCESS;
} 