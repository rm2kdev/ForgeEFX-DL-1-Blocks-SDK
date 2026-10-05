cmake_minimum_required(VERSION 3.22)
project(ForgeEFXGain LANGUAGES C)
set(FORGEEFX_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/../../sdk" CACHE PATH "Path to the released ForgeEFX SDK")
include("${FORGEEFX_SDK_DIR}/cmake/ForgeEFXBlock.cmake")
# Replace yourcompany here and yourcompany.youreffect in parameters.json together.
forgeefx_add_block(example_gain "${CMAKE_CURRENT_SOURCE_DIR}" DEVELOPER yourcompany)
