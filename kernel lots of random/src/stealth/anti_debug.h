#pragma once
#include <ntifs.h>

void InitializeAntiDebug(void);
BOOLEAN IsDebuggerPresent(void);
void ImplementTimingChecks(void); 