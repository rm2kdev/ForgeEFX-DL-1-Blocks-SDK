cmake_minimum_required(VERSION 3.22)
project(ForgeEFXBlocksSDK VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CTest)
find_package(Python3 3.10 COMPONENTS Interpreter REQUIRED)
# Share one canonical SDK path across examples (also in CMake regeneration inputs).
set(FORGEEFX_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdk" CACHE PATH "Path to the released ForgeEFX SDK")
get_filename_component(FORGEEFX_SDK_DIR "${FORGEEFX_SDK_DIR}" ABSOLUTE)
add_subdirectory(sdk)
add_subdirectory(examples/gain)
add_subdirectory(examples/delay)

if(BUILD_TESTING)
    add_test(NAME package_compatibility
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/PackageCompatibilityTests.py")
    add_test(NAME macos_platform
        COMMAND "${CMAKE_COMMAND}" "-DPython3_EXECUTABLE=${Python3_EXECUTABLE}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/MacPlatformTests.cmake")
    add_executable(gain_tests tests/GainTests.c examples/gain/dsp.c)
    add_test(NAME gain_behavior COMMAND gain_tests)
    add_executable(delay_tests tests/DelayTests.c examples/delay/dsp.c)
    add_test(NAME delay_behavior COMMAND delay_tests)
endif()
