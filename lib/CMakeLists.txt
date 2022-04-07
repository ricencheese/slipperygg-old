add_subdirectory(imgui)

if (WIN32)
  add_subdirectory(freetype)
  add_subdirectory(minhook)
  add_subdirectory(dxsdk)
endif()
