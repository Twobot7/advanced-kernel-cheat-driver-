#pragma once
#include "stealth_types.h"
#include "decoy_protection.h"

// Threat detection configuration
#define MAX_THREAT_HISTORY 32
#define THREAT_CORRELATION_WINDOW 5000  // 5 seconds in milliseconds
#define SUSPICIOUS_ACCESS_THRESHOLD 10
#define THREAT_LEVEL_CRITICAL 5

typedef enum _THREAT_TYPE {
    THREAT_MEMORY_SCAN,
    THREAT_PATTERN_MATCH,
    THREAT_SUSPICIOUS_ACCESS,
    THREAT_INTEGRITY_VIOLATION,
    THREAT_DEBUG_DETECTED
} THREAT_TYPE;

typedef struct _THREAT_EVENT {
    THREAT_TYPE Type;
    LARGE_INTEGER Timestamp;
    PVOID DecoyAddress;
    ULONG ThreadId;
    ULONG ProcessId;
    ULONG ThreatLevel;
} THREAT_EVENT, *PTHREAT_EVENT;

typedef struct _THREAT_DETECTION_STATE {
    LIST_ENTRY ThreatEventList;
    KSPIN_LOCK ThreatLock;
    ULONG CurrentThreatLevel;
    THREAT_EVENT History[MAX_THREAT_HISTORY];
    ULONG HistoryIndex;
} THREAT_DETECTION_STATE, *PTHREAT_DETECTION_STATE;

NTSTATUS InitializeThreatDetection(IN PSTEALTH_CONTEXT Context);
VOID ProcessThreatEvent(IN PDECOY_REGION Decoy, IN THREAT_TYPE Type);
VOID CleanupThreatDetection(IN PSTEALTH_CONTEXT Context); 