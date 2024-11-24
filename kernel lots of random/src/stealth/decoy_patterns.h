#pragma once
#include "stealth_types.h"
#include "decoy_protection.h"

// Pattern types for behavioral analysis
typedef enum _DECOY_PATTERN_TYPE {
    PATTERN_CODE,           // Executable code patterns
    PATTERN_DATA,           // Data structure patterns
    PATTERN_HEAP,          // Heap-like patterns
    PATTERN_STACK,         // Stack frame patterns
    PATTERN_VTable         // Virtual table patterns
} DECOY_PATTERN_TYPE;

typedef struct _DECOY_BEHAVIOR_PATTERN {
    DECOY_PATTERN_TYPE Type;
    UCHAR Complexity;      // 0-100 scale
    BOOLEAN IsVolatile;    // Whether pattern should change frequently
    ULONG UpdateInterval;  // Ticks between updates
} DECOY_BEHAVIOR_PATTERN, *PDECOY_BEHAVIOR_PATTERN;

typedef struct _DECOY_ACCESS_PATTERN {
    ULONG AccessCount;
    ULONG TimeStamp;
    ACCESS_MASK AccessType;
    PVOID AccessAddress;
    ULONG ThreadId;
} DECOY_ACCESS_PATTERN, *PDECOY_ACCESS_PATTERN;

// Function declarations
NTSTATUS InitializeDecoyPatterns(PSTEALTH_CONTEXT Context);
NTSTATUS GeneratePatternContent(PVOID BaseAddress, SIZE_T Size, PDECOY_BEHAVIOR_PATTERN Pattern);
VOID UpdateDecoyPatterns(PDECOY_REGION Decoy); 