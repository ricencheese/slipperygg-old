cmake_minimum_required(VERSION 3.16.0)
project(slipperygg)

if (NOT WIN32)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fvisibility=hidden -fno-exceptions -Wfatal-errors")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-rtti")
endif ()

add_compile_options($<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>,$<C_COMPILER_ID:GNU>>:-s>)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --no-gnu-unique")
endif ()

file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

if (NOT WIN32)
    list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/lib/imgui/imgui_impl_dx9.cpp")
    list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/lib/imgui/imgui_impl_win32.cpp")
    list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/hooking/minhook.cpp")
else ()
    list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/lib/imgui/imgui_impl_opengl3.cpp")
    list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/lib/imgui/imgui_impl_sdl.cpp")
    list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/lib/MinHook/buffer.c")
    list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/lib/MinHook/hde/hde32.c")
    list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/lib/MinHook/hde/hde64.c")
    list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/lib/MinHook/hook.c")
    list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/lib/MinHook/trampoline.c")
endif ()

list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/pch/pch.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/base.cpp")

add_library(slipperyggOBJ OBJECT ${SOURCES})
set_target_properties(slipperyggOBJ PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_precompile_headers(slipperyggOBJ PRIVATE "${CMAKE_SOURCE_DIR}/src/pch/pch.h")
target_include_directories(slipperyggOBJ PUBLIC "${CMAKE_SOURCE_DIR}/lib")

if (NOT WIN32)
    find_package(Freetype REQUIRED)
    find_package(SDL2 REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(mesa-vulkan-drivers REQUIRED)

    target_link_libraries(slipperyggOBJ PRIVATE SDL2 GL ${CMAKE_DL_LIBS} Freetype::Freetype)
    target_link_options(slipperyggOBJ PUBLIC -flto LINKER:--no-undefined)

    target_include_directories(slipperyggOBJ PRIVATE ${SDL2_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})

    target_compile_options(slipperyggOBJ PRIVATE -flto)
else ()
    target_include_directories(slipperyggOBJ PRIVATE "${CMAKE_SOURCE_DIR}/lib/dxsdk/Include")
    target_include_directories(slipperyggOBJ PRIVATE "${CMAKE_SOURCE_DIR}/lib/dxsdk/Lib/x86")
    target_include_directories(slipperyggOBJ PRIVATE "${CMAKE_SOURCE_DIR}/lib/freetype")

    target_link_libraries(slipperyggOBJ PRIVATE "../lib/dxsdk/Lib/x86/d3dx9")
    target_link_libraries(slipperyggOBJ PRIVATE FreeType)

    target_compile_definitions(slipperyggOBJ PRIVATE _USE_MATH_DEFINES)
    target_compile_options(slipperyggOBJ PRIVATE /utf-8)

    add_subdirectory(lib/freetype)
    add_subdirectory(res/Shaders)

    add_dependencies(slipperyggOBJ Shaders)
endif ()

add_library(slipperygg SHARED "${CMAKE_SOURCE_DIR}/src/base.cpp")
target_link_libraries(slipperygg PRIVATE slipperyggOBJ)

if (NOT WIN32)
    target_link_options(slipperygg PRIVATE -flto)
else ()
    target_link_options(slipperygg PRIVATE /ENTRY:DllEntryPoint)
endif ()
