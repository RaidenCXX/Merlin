# Minimum version of CMake
cmake_minimum_required(VERSION 3.10)

#Name of project and the language
project(Merlin CXX)

#C++ standart
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#Executable
file(GLOB_RECURSE SOURCES
  ${CMAKE_SOURCE_DIR}/source/*.cpp
  ${CMAKE_SOURCE_DIR}/source/*.h
)
add_executable(Merlin ${SOURCES})


#Include
target_include_directories(Merlin PRIVATE ${CMAKE_SOURCE_DIR}/deps/include)

#Lib
if(WIN32) # Windows
#
#
elseif(UNIX AND NOT APPLE) # Linux
  set(LIB_DIR_PATH ${CMAKE_SOURCE_DIR}/deps/lib/linux64)
  target_link_libraries(Merlin PRIVATE 
    ${LIB_DIR_PATH}/VulkanLoader/libvulkan.so 
    ${LIB_DIR_PATH}/GLFW/libglfw.so                                          )
elseif(APPLE) # macOS
#
#
endif()





