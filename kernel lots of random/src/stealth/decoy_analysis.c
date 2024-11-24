#include "decoy_patterns.h"
#include "decoy_protection.h"
#include "threat_detection.h"

static VOID AnalyzeAccessPattern(PDECOY_REGION Decoy, PDECOY_ACCESS_PATTERN Access) {
    KIRQL oldIrql;
    KeAcquireSpinLock(&Decoy->AnalysisLock, &oldIrql);

    // Update access statistics
    Decoy->AccessHistory[Decoy->HistoryIndex] = *Access;
    Decoy->HistoryIndex = (Decoy->HistoryIndex + 1) % MAX_ACCESS_HISTORY;
    
    // Analyze access patterns for suspicious behavior
    if (IsAccessPatternSuspicious(Decoy)) {
        TriggerThreatResponse(Decoy, THREAT_SUSPICIOUS_ACCESS);
    }

    KeReleaseSpinLock(&Decoy->AnalysisLock, oldIrql);
}

static BOOLEAN IsAccessPatternSuspicious(PDECOY_REGION Decoy) {
    ULONG rapidAccessCount = 0;
    ULONG currentTime = KeQueryTimeIncrement();

    // Check for rapid successive accesses
    for (ULONG i = 0; i < MAX_ACCESS_HISTORY; i++) {
        if (currentTime - Decoy->AccessHistory[i].TimeStamp < SUSPICIOUS_ACCESS_THRESHOLD) {
            rapidAccessCount++;
        }
    }

    return (rapidAccessCount > SUSPICIOUS_ACCESS_LIMIT);
}

VOID InitializeDecoyAnalysis(PDECOY_REGION Decoy) {
    KeInitializeSpinLock(&Decoy->AnalysisLock);
    Decoy->HistoryIndex = 0;
    RtlZeroMemory(Decoy->AccessHistory, sizeof(Decoy->AccessHistory));
    
    // Set up analysis callbacks
    Decoy->AnalysisCallback = AnalyzeAccessPattern;
} 