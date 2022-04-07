target_sources(SlipperyggOBJ PRIVATE vmtswap.cpp)

if (WIN32)
    target_sources(SlipperyggOBJ PRIVATE minhook.cpp)
endif ()
