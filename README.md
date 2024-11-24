below i what is required to compile and use. please contact "onbot." on discord for all your cheat needs.

there is a fair bit of anti pasta in here and isnt a complete working project. if you have half a brain you will be able to use this to avoid every and all acs 

### 1. Development Environment
- Visual Studio 2015 or later (based on the solution file configuration)
- Windows Driver Kit (WDK) 10 or later
- Windows SDK 10 or later

### 2. Required Tools & SDKs
- C++ compiler with C++17 support (specified in driver.vcxproj)

```174:174:src/driver.vcxproj
      <LanguageStandard>stdcpplatest</LanguageStandard>
```

- Windows Kernel Mode Driver Development tools
- CMake 3.10 or higher (based on CMakeLists.txt)

```1:1:CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
```


### 3. Project Configuration
The project consists of two main components:

#### Driver Component:
- Configured as a WDM driver for x64 platform
- Uses Windows Kernel Mode Driver 10.0 Platform Toolset

```67:70:src/driver.vcxproj
    <PlatformToolset>WindowsKernelModeDriver10.0</PlatformToolset>
    <ConfigurationType>Driver</ConfigurationType>
    <DriverType>WDM</DriverType>
    <DriverTargetPlatform>Desktop</DriverTargetPlatform>
```


#### Loader Component:
- Standard C++ executable
- Uses C++17 standard
- Requires Windows SDK headers

### 4. Build Dependencies
The following libraries and headers are required:

#### System Headers:
- ntddk.h
- ntifs.h
- wdm.h
- ntimage.h
- windef.h
- Windows.h
- TlHelp32.h

#### Project Structure:
The project is organized into several modules:
- Driver core
- GPU support (NVIDIA, AMD, Intel)
- Overlay renderer
- Stealth components
- Input control
- Protection systems

### 5. Build Steps

1. **Configure Visual Studio:**
   - Install Visual Studio with C++ Desktop Development
   - Install WDK
   - Install Windows SDK

2. **Build Order:**
   - First build the driver project
   - Then build the loader project
   - Output directories are configured in the project file:

```130:131:src/driver.vcxproj
    <OutDir>$(SolutionDir)bin\$(Configuration)\</OutDir>
    <IntDir>$(SolutionDir)tmp\$(ProjectName)\$(Configuration)\</IntDir>
```


3. **Required Build Configurations:**
   - Debug|x64
   - Release|x64

```9:12:cheat-driver.sln
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|x64 = Debug|x64
		Release|x64 = Release|x64
	EndGlobalSection
```


### 6. Additional Notes

- The project uses specific memory tags for allocations
- Requires administrator privileges to run
- Driver signing will be required for deployment
- Make sure to have proper security certificates for driver signing
- The build environment should support Windows driver development

### 7. Build Output
The compiled files will be placed in:
- bin/Debug or bin/Release directory
- Separate directories for driver and loader components

Make sure all the required components are installed and properly configured before attempting to build the project.
