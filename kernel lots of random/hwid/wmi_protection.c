#include "wmi_protection.h"
#include "hwid_lock.h"

// WMI GUID for our provider
static GUID WmiProviderGuid = 
    {0x12345678, 0x1234, 0x1234, {0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef}};

// Structure to hold randomized hardware information
typedef struct _RANDOMIZED_HWID {
    WCHAR ProcessorId[256];
    WCHAR BiosSerial[256];
    WCHAR MacAddress[256];
    WCHAR SsdSerial[256];
    WCHAR RamDetails[256];
    WCHAR MonitorDetails[256];
    WCHAR GpuDetails[256];
} RANDOMIZED_HWID, *PRANDOMIZED_HWID;

static RANDOMIZED_HWID RandomizedData = {0};

// Function to generate random string
VOID GenerateRandomString(PWCHAR Output, SIZE_T Length) {
    static const WCHAR Charset[] = L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    SIZE_T CharsetLength = wcslen(Charset);
    
    for (SIZE_T i = 0; i < Length - 1; i++) {
        ULONG rand;
        ExGenRandom(&rand, sizeof(rand));
        Output[i] = Charset[rand % CharsetLength];
    }
    Output[Length - 1] = L'\0';
}

// Initialize randomized data
VOID InitializeRandomizedData() {
    GenerateRandomString(RandomizedData.ProcessorId, 32);
    GenerateRandomString(RandomizedData.BiosSerial, 32);
    GenerateRandomString(RandomizedData.MacAddress, 17);  // MAC address format
    GenerateRandomString(RandomizedData.SsdSerial, 32);
    GenerateRandomString(RandomizedData.RamDetails, 32);
    GenerateRandomString(RandomizedData.MonitorDetails, 32);
    GenerateRandomString(RandomizedData.GpuDetails, 32);
}

// WMI query callback
NTSTATUS WmiQueryCallback(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    ULONG GuidIndex,
    ULONG InstanceIndex,
    PULONG InstanceCount,
    PULONG InstanceLengthArray,
    PULONG BufferAvail,
    PUCHAR Buffer
) {
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(GuidIndex);
    UNREFERENCED_PARAMETER(InstanceIndex);
    
    // Return randomized data instead of real hardware information
    if (*BufferAvail < sizeof(RANDOMIZED_HWID)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    RtlCopyMemory(Buffer, &RandomizedData, sizeof(RANDOMIZED_HWID));
    *BufferAvail = sizeof(RANDOMIZED_HWID);
    *InstanceCount = 1;
    if (InstanceLengthArray) {
        *InstanceLengthArray = sizeof(RANDOMIZED_HWID);
    }

    return STATUS_SUCCESS;
}

// Initialize WMI protection
NTSTATUS InitializeWmiProtection() {
    InitializeRandomizedData();
    return STATUS_SUCCESS;
}

// Cleanup WMI protection
void CleanupWmiProtection() {
    // Cleanup code here
}

// Register WMI GUIDs
NTSTATUS RegisterWmiGuids(PDRIVER_OBJECT DriverObject) {
    WMIGUIDREGINFO GuidRegInfo;
    GuidRegInfo.Guid = WmiProviderGuid;
    GuidRegInfo.InstanceCount = 1;
    GuidRegInfo.Flags = WMIREG_FLAG_INSTANCE_PDO;

    return IoWMIRegistrationControl(DriverObject, WMIREG_ACTION_REGISTER);
} 