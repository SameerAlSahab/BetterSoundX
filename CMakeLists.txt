cmake_minimum_required(VERSION 3.20)
project(ALCDSPPlugin VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ALCDSPCore STATIC DSP/ALCDSP.cpp DSP/ALCDSP_C.cpp)
target_include_directories(ALCDSPCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/DSP)
set_target_properties(ALCDSPCore PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(MSVC)
  target_compile_options(ALCDSPCore PRIVATE /W4)
else()
  target_compile_options(ALCDSPCore PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(ALCDSPTests Tests/dsp_tests.cpp)
target_link_libraries(ALCDSPTests PRIVATE ALCDSPCore)

add_executable(dsp_process Tools/process_f32_stereo.cpp)
target_link_libraries(dsp_process PRIVATE ALCDSPCore)

enable_testing()
add_test(NAME ALCDSPCoreTests COMMAND ALCDSPTests)
