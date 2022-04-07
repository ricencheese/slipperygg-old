cmake_minimum_required(VERSION 3.16.0)
project(Slipperygg)

if (NOT WIN32)
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-exceptions -Wfatal-errors")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-rtti")
endif()

add_compile_options($<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>,$<C_COMPILER_ID:GNU>>:-s>)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --no-gnu-unique")
endif()

add_library(SlipperyggOBJ OBJECT "")
set_target_properties(SlipperyggOBJ PROPERTIES CXX_VISIBILITY_PRESET hidden C_VISIBILITY_PRESET hidden)
add_subdirectory(lib)
add_subdirectory(src)

set_target_properties(SlipperyggOBJ PROPERTIES POSITION_INDEPENDENT_CODE ON)
#target_precompile_headers(SlipperyggOBJ PRIVATE "${CMAKE_SOURCE_DIR}/src/pch/pch.h")
target_include_directories(SlipperyggOBJ PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(SlipperyggOBJ PUBLIC "${CMAKE_SOURCE_DIR}/lib")

if (NOT WIN32)
  target_link_libraries(SlipperyggOBJ PRIVATE ${CMAKE_DL_LIBS})
  target_compile_options(SlipperyggOBJ PRIVATE -flto)
  target_link_options(SlipperyggOBJ PUBLIC -flto LINKER:--no-undefined)
else()
  target_compile_definitions(SlipperyggOBJ PRIVATE _USE_MATH_DEFINES)
  target_compile_options(SlipperyggOBJ PRIVATE /utf-8)
  add_subdirectory(res/Shaders)
  add_dependencies(SlipperyggOBJ Shaders)
endif()

add_library(Slipperygg SHARED "${CMAKE_SOURCE_DIR}/src/base.cpp")
target_link_libraries(Slipperygg PUBLIC SlipperyggOBJ)

if (NOT WIN32)
  target_link_options(Slipperygg PRIVATE -flto)
else()
  target_link_options(Slipperygg PRIVATE /ENTRY:DllEntryPoint)
endif()
