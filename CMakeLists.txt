cmake_minimum_required(VERSION 3.22)
project(ForgeEFXBlocksSDK VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CTest)
find_package(Python3 3.10 COMPONENTS Interpreter REQUIRED)
# Use one canonical SDK path across examples in the combined build.
set(FORGEEFX_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdk")
add_subdirectory(sdk)
add_subdirectory(examples/gain)
add_subdirectory(examples/phaser)

if(BUILD_TESTING)
    add_test(NAME package_compatibility
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/PackageCompatibilityTests.py")
    add_test(NAME macos_platform
        COMMAND "${CMAKE_COMMAND}" "-DPython3_EXECUTABLE=${Python3_EXECUTABLE}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/MacPlatformTests.cmake")
    add_executable(gain_tests tests/GainTests.c examples/gain/dsp.c)
    add_test(NAME gain_behavior COMMAND gain_tests)
    add_executable(phaser_tests tests/PhaserTests.c examples/phaser/dsp.c)
    if(NOT MSVC)
        target_link_libraries(phaser_tests PRIVATE m)
    endif()
    add_test(NAME phaser_behavior COMMAND phaser_tests)
endif()
