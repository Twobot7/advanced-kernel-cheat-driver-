#pragma once
#include "stealth_types.h"
#include <ntifs.h>

NTSTATUS ApplyDriverStealth(
    IN PVOID DriverBase,
    IN PUNICODE_STRING DriverName
);

NTSTATUS InitializeDriverStealth(
    IN PSTEALTH_CONTEXT Context
);

void CleanupDriverStealth(
    IN PSTEALTH_CONTEXT Context
); 