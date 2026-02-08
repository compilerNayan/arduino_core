cmake_minimum_required(VERSION 3.14)
project(arduino_core VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch dependencies
include(FetchContent)

# Fetch arduinolib4
FetchContent_Declare(
    arduinolib4
    GIT_REPOSITORY https://github.com/compilerNayan/arduinolib4.git
    GIT_TAG        main
)

FetchContent_MakeAvailable(arduinolib4)

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

# Link to arduinolib4
# Try the namespaced target first, fallback to non-namespaced
if(TARGET arduinolib4::arduinolib4)
    target_link_libraries(arduino_core INTERFACE arduinolib4::arduinolib4)
elseif(TARGET arduinolib4)
    target_link_libraries(arduino_core INTERFACE arduinolib4)
else()
    # Fallback: manually add include directory if target doesn't exist
    FetchContent_GetProperties(arduinolib4)
    if(arduinolib4_SOURCE_DIR)
        target_include_directories(arduino_core INTERFACE
            $<BUILD_INTERFACE:${arduinolib4_SOURCE_DIR}/src>
        )
    endif()
endif()

# Note: For INTERFACE libraries, we don't need to add header files to target_sources
# The include directories are sufficient for consumers to find the headers

