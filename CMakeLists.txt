cmake_minimum_required(VERSION 3.25)
project(BBB_ProcsMan)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(BBB_ProcsMan main.cpp CFileVersionInfo.h CFileVersionInfo.cpp)

target_link_libraries(BBB_ProcsMan Version.lib)
target_link_libraries(BBB_ProcsMan Psapi.lib)

#if (WIN32)
#    set_target_properties(BBB_ProcsMan PROPERTIES WIN32_EXECUTABLE true)
#endif()