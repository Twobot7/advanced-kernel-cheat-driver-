#include "secure_mapper.h"
#include "memory_utils.h"
#include "kernel_utils.h"
#include "crypto.h"

bool SecureMapper::MapDriver(const std::wstring& driverPath) {
    MappingContext ctx = {};
    
    // Read and decrypt driver file
    std::vector<BYTE> driverData = Crypto::DecryptFile(driverPath);
    if (driverData.empty()) return false;

    // Verify digital signature (optional for testing)
    if (!VerifySignature(driverData)) return false;

    // Allocate local memory for the image
    ctx.localImage = VirtualAlloc(nullptr, driverData.size(), 
                                MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!ctx.localImage) return false;

    // Copy and verify PE headers
    memcpy(ctx.localImage, driverData.data(), driverData.size());
    if (!VerifyPEHeaders(ctx.localImage)) {
        VirtualFree(ctx.localImage, 0, MEM_RELEASE);
        return false;
    }

    // Initialize mapping context
    ctx.ntHeaders = GetNTHeaders(ctx.localImage);
    if (!ctx.ntHeaders) {
        VirtualFree(ctx.localImage, 0, MEM_RELEASE);
        return false;
    }

    // Allocate kernel memory using vulnerable driver technique
    ctx.targetBase = KernelAllocator::AllocateKernelMemory(
        ctx.ntHeaders->OptionalHeader.SizeOfImage);
    if (!ctx.targetBase) {
        VirtualFree(ctx.localImage, 0, MEM_RELEASE);
        return false;
    }

    // Apply advanced anti-detection measures
    if (!AntiDetection::PrepareEnvironment()) {
        KernelAllocator::FreeKernelMemory(ctx.targetBase);
        VirtualFree(ctx.localImage, 0, MEM_RELEASE);
        return false;
    }

    // Map sections with advanced protection
    if (!MapSections(ctx)) {
        KernelAllocator::FreeKernelMemory(ctx.targetBase);
        VirtualFree(ctx.localImage, 0, MEM_RELEASE);
        return false;
    }

    // Resolve imports securely
    if (!ResolveImports(ctx)) {
        KernelAllocator::FreeKernelMemory(ctx.targetBase);
        VirtualFree(ctx.localImage, 0, MEM_RELEASE);
        return false;
    }

    // Apply relocations
    if (!ApplyRelocations(ctx)) {
        KernelAllocator::FreeKernelMemory(ctx.targetBase);
        VirtualFree(ctx.localImage, 0, MEM_RELEASE);
        return false;
    }

    // Clear PE headers and add additional obfuscation
    if (!ClearPEHeaders(ctx)) {
        KernelAllocator::FreeKernelMemory(ctx.targetBase);
        VirtualFree(ctx.localImage, 0, MEM_RELEASE);
        return false;
    }

    // Hide driver traces
    if (!HideDriverTraces(ctx)) {
        KernelAllocator::FreeKernelMemory(ctx.targetBase);
        VirtualFree(ctx.localImage, 0, MEM_RELEASE);
        return false;
    }

    // Execute driver entry
    if (!ExecuteEntry(ctx)) {
        KernelAllocator::FreeKernelMemory(ctx.targetBase);
        VirtualFree(ctx.localImage, 0, MEM_RELEASE);
        return false;
    }

    // Cleanup
    VirtualFree(ctx.localImage, 0, MEM_RELEASE);
    return true;
} 