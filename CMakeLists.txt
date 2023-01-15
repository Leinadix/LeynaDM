add_executable (LeynaDM "LeynaDM.cpp" "LeynaDM.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET LeynaDM PROPERTY CXX_STANDARD 20)
endif()