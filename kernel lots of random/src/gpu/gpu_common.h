#pragma once
#include "../common/types.h"
#include "../common/utils.h"

// PCI Configuration Space registers
#define PCI_CONFIG_VENDOR_ID        0x00
#define PCI_CONFIG_DEVICE_ID        0x02
#define PCI_CONFIG_COMMAND          0x04
#define PCI_CONFIG_STATUS           0x06
#define PCI_CONFIG_REVISION         0x08
#define PCI_CONFIG_BAR0            0x10
#define PCI_CONFIG_BAR1            0x14
#define PCI_CONFIG_BAR2            0x18
#define PCI_CONFIG_BAR3            0x1C
#define PCI_CONFIG_BAR4            0x20
#define PCI_CONFIG_BAR5            0x24

// PCI Vendor IDs
#define PCI_VENDOR_NVIDIA    0x10DE
#define PCI_VENDOR_AMD      0x1002
#define PCI_VENDOR_INTEL    0x8086

// PCI Class codes
#define PCI_CLASS_DISPLAY    0x03
#define PCI_SUBCLASS_VGA    0x00
#define PCI_SUBCLASS_3D     0x02

// Function declarations
NTSTATUS DetectPrimaryGPU(PGPU_INFO GpuInfo);
NTSTATUS InitializeGPU(PGPU_INFO GpuInfo);
VOID CleanupGPU(PGPU_INFO GpuInfo);
NTSTATUS MapGPURegisters(PGPU_INFO GpuInfo);
VOID UnmapGPURegisters(PGPU_INFO GpuInfo);

// Vendor-specific function tables
extern PFN_GPU_INIT GpuInitFunctions[];
extern PFN_GPU_CLEANUP GpuCleanupFunctions[];
extern PFN_GPU_DRAW_PRIMITIVE GpuDrawFunctions[];

typedef struct _GPU_OPERATION_CONTEXT {
    KSPIN_LOCK OperationLock;
    BOOLEAN IsBusy;
    GPU_INFO* ActiveGpu;
} GPU_OPERATION_CONTEXT, *PGPU_OPERATION_CONTEXT;
