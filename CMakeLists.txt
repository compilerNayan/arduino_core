cmake_minimum_required(VERSION 3.14)
project(arduino_core VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch dependencies
include(FetchContent)

# Fetch springbootplusplus
FetchContent_Declare(
    springbootplusplus
    GIT_REPOSITORY https://github.com/compilerNayan/springbootplusplus.git
    GIT_TAG        main
)

FetchContent_MakeAvailable(springbootplusplus)

# Create header-only library (INTERFACE library)
add_library(arduino_core INTERFACE)

# Set target properties
set_target_properties(arduino_core PROPERTIES
    VERSION ${PROJECT_VERSION}
)

# Set include directories for arduino_core's own headers
target_include_directories(arduino_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

# Link to springbootplusplus
# Try the namespaced target first, fallback to non-namespaced
if(TARGET springbootplusplus::springbootplusplus)
    target_link_libraries(arduino_core INTERFACE springbootplusplus::springbootplusplus)
elseif(TARGET springbootplusplus)
    target_link_libraries(arduino_core INTERFACE springbootplusplus)
else()
    # Fallback: manually add include directory if target doesn't exist
    FetchContent_GetProperties(springbootplusplus)
    if(springbootplusplus_SOURCE_DIR)
        target_include_directories(arduino_core INTERFACE
            $<BUILD_INTERFACE:${springbootplusplus_SOURCE_DIR}/src>
        )
    endif()
endif()

# Note: For INTERFACE libraries, we don't need to add header files to target_sources
# The include directories are sufficient for consumers to find the headers

