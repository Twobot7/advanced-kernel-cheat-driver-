#pragma once
#include <Windows.h>
#include <vector>
#include <memory>
#include "anti_detection.h"
#include "pe_utils.h"

class SecureMapper {
private:
    struct ImportEntry {
        std::string moduleName;
        std::string functionName;
        ULONGLONG rva;
    };

    struct MappingContext {
        PVOID localImage;
        PVOID targetBase;
        PIMAGE_NT_HEADERS ntHeaders;
        std::vector<ImportEntry> imports;
        std::vector<PVOID> cleanupList;
    };

    static DWORD FindKernelBase();
    static bool ResolveImports(MappingContext& ctx);
    static bool MapSections(MappingContext& ctx);
    static bool ApplyRelocations(MappingContext& ctx);
    static bool ClearPEHeaders(MappingContext& ctx);
    static bool ExecuteEntry(MappingContext& ctx);
    static bool HideDriverTraces(MappingContext& ctx);

public:
    static bool MapDriver(const std::wstring& driverPath);
}; 