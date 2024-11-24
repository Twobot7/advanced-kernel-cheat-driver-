#pragma once
#include <ntddk.h>
#include <windef.h>

// Forward declarations
struct _GPU_INFO;
typedef struct _GPU_INFO GPU_INFO, *PGPU_INFO;

#define OVERLAY_TAG 'ylvO'
#define MAX_VERTICES 10000
#define MAX_BUFFERS 4

// Basic types
typedef struct _POINT2D {
    INT x;
    INT y;
} POINT2D, *PPOINT2D;

typedef struct _RECT2D {
    INT x;
    INT y;
    INT width;
    INT height;
} RECT2D, *PRECT2D;

typedef struct _COLOR_RGBA {
    UCHAR r;
    UCHAR g;
    UCHAR b;
    UCHAR a;
} COLOR_RGBA, *PCOLOR_RGBA;

typedef struct _VERTEX {
    FLOAT x, y;      // Screen coordinates
    FLOAT u, v;      // Texture coordinates
    COLOR_RGBA color;
} VERTEX, *PVERTEX;

typedef struct _OVERLAY_BUFFER {
    PVOID VirtualAddress;
    PHYSICAL_ADDRESS PhysicalAddress;
    SIZE_T Size;
    BOOLEAN InUse;
    LIST_ENTRY ListEntry;
} OVERLAY_BUFFER, *POVERLAY_BUFFER;

// GPU types
typedef enum _GPU_VENDOR {
    GPU_VENDOR_UNKNOWN = 0,
    GPU_VENDOR_NVIDIA,
    GPU_VENDOR_AMD,
    GPU_VENDOR_INTEL,
    GPU_VENDOR_MAX
} GPU_VENDOR;

typedef struct _GPU_INFO {
    GPU_VENDOR Vendor;
    ULONG DeviceId;
    ULONG VendorId;
    PHYSICAL_ADDRESS FrameBufferPA;
    PVOID FrameBufferVA;
    PVOID RegistersVA;
    SIZE_T FrameBufferSize;
    ULONG ScreenWidth;
    ULONG ScreenHeight;
    ULONG Bpp;
    ULONG Pitch;
    BOOLEAN IsPrimary;
    KSPIN_LOCK Lock;
    PVOID VendorSpecific;  // Pointer to vendor-specific data
} GPU_INFO, *PGPU_INFO;

// Function pointer types
typedef NTSTATUS (*PFN_GPU_INIT)(PGPU_INFO GpuInfo);
typedef VOID (*PFN_GPU_CLEANUP)(PGPU_INFO GpuInfo);
typedef NTSTATUS (*PFN_GPU_DRAW_PRIMITIVE)(PGPU_INFO GpuInfo, PVERTEX Vertices, ULONG Count);

// Error codes
#define STATUS_GPU_NOT_FOUND         ((NTSTATUS)0xE0000001L)
#define STATUS_GPU_INIT_FAILED       ((NTSTATUS)0xE0000002L)
#define STATUS_BUFFER_INIT_FAILED    ((NTSTATUS)0xE0000003L)
#define STATUS_OVERLAY_INIT_FAILED   ((NTSTATUS)0xE0000004L)
