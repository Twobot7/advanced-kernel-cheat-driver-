cmake_minimum_required(VERSION 3.10)
project(DriverLoader)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files for loader
set(LOADER_SOURCES
    src/loader/main.cpp
    src/loader/loader.cpp
    src/loader/driver_mapper.cpp
    src/loader/driver_interface.cpp
    src/loader/protection.cpp
)

# Add source files for mapper
set(MAPPER_SOURCES
    src/mapper/secure_mapper.cpp
)

# Create loader executable
add_executable(loader ${LOADER_SOURCES})

# Create mapper library
add_library(mapper STATIC ${MAPPER_SOURCES})

# Include directories
target_include_directories(loader PRIVATE
    src/loader
    src/mapper
)

target_include_directories(mapper PRIVATE
    src/mapper
)

# Link libraries
target_link_libraries(loader PRIVATE mapper) 