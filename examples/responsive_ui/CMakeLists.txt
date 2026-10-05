cmake_minimum_required(VERSION 3.22)
project(ForgeEFXResponsiveUI LANGUAGES C)
set(FORGEEFX_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/../../sdk" CACHE PATH "Path to the released ForgeEFX SDK")
include("${FORGEEFX_SDK_DIR}/cmake/ForgeEFXBlock.cmake")
# Unpublished demo: replace both publisher and effect identity before distribution.
forgeefx_add_block(example_responsive_ui "${CMAKE_CURRENT_SOURCE_DIR}" DEVELOPER yourcompany)
