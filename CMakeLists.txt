# Minimum version of CMake
cmake_minimum_required(VERSION 3.10)

#Name of project and the language
project(Merlin C CXX)

#C++ standart
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#Executable
file(GLOB_RECURSE SOURCES
  ${CMAKE_SOURCE_DIR}/source/*.cpp
  ${CMAKE_SOURCE_DIR}/source/*.h
)
add_executable(Merlin ${SOURCES})


#Include
set(INCLUDE_DIR_PATH ${CMAKE_SOURCE_DIR}/deps/include)
target_include_directories(Merlin PRIVATE ${INCLUDE_DIR_PATH})

#Lib
set(LIB_DIR_PATH ${CMAKE_SOURCE_DIR}/deps/lib)

#Shaders
set(SHADER_DIR_PATH ${CMAKE_SOURCE_DIR}/resources/shaders)

#minilog lib
add_library(Minilog STATIC ${LIB_DIR_PATH}/minilog/minilog.cpp)
target_include_directories(Minilog PUBLIC ${CMAKE_SOURCE_DIR}/deps/include/minilog)
target_link_libraries(Merlin PRIVATE Minilog)

#Volk
add_library(Volk STATIC ${LIB_DIR_PATH}/Volk/volk.c)
target_include_directories(Volk PUBLIC ${INCLUDE_DIR_PATH}/Volk)
set_target_properties(Volk PROPERTIES
  C_STANDARD 11
  C_STANDARD_REQUIRED ON
)
target_link_libraries(Merlin PRIVATE Volk)
target_compile_definitions(Merlin PRIVATE VK_NO_PROTOTYPES)

if(WIN32) # Windows
  target_compile_definitions(Merlin PRIVATE WINDOWS)
#
#
elseif(UNIX AND NOT APPLE) # Linux

  #Macro
  target_compile_definitions(Merlin PRIVATE 
    LINUX
    VK_USE_PLATFORM_WAYLAND_KHR)
  target_compile_definitions(Volk PRIVATE VK_USE_PLATFORM_XLIB_KHR)

  set(LIB_LINUX_DIR_PATH ${LIB_DIR_PATH}/linux64)

  #Link other libs
  target_link_libraries(Merlin PRIVATE 
    ${LIB_LINUX_DIR_PATH}/VulkanLoader/libvulkan.so 
    ${LIB_LINUX_DIR_PATH}/GLFW/libglfw.so
    ${LIB_LINUX_DIR_PATH}/Slang/libslang.so
    ${LIB_LINUX_DIR_PATH}/Glslang/libglslang.so
    ${LIB_LINUX_DIR_PATH}/SPIRV-Reflect/libspirv-reflect-static.a
    ${LIB_LINUX_DIR_PATH}/assimp/libassimp.so
)

#Post build
  set(BUILD_DIR ${CMAKE_BINARY_DIR}/Linux)
  set(MERLIN_BUILD_DIR ${BUILD_DIR}/Merlin)
  set(RUNTIME_LIB_DIR ${MERLIN_BUILD_DIR}/Libraries)
  set(SHADER_LINUX_DIR_PATH ${MERLIN_BUILD_DIR}/shaders)
 
  #Set property for Merlin
  set_target_properties(Merlin PROPERTIES
    #Location executable file
    RUNTIME_OUTPUT_DIRECTORY ${MERLIN_BUILD_DIR}
    #Location library
    BUILD_RPATH "$ORIGIN/Libraries/VulkanLoader:$ORIGIN/Libraries/GLFW:$ORIGIN/Libraries/Slang"
  )

  add_custom_command(TARGET Merlin POST_BUILD 
    #Creating in build dir, dir for Linux 
    COMMAND ${CMAKE_COMMAND} -E make_directory ${BUILD_DIR}
    #Creating in linux dir, Merlin dir 
    COMMAND ${CMAKE_COMMAND} -E make_directory ${MERLIN_BUILD_DIR}
    #Creating in Merlin dir, dir for libraries
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RUNTIME_LIB_DIR}
    #Creating in Merlin dir, dir for shaders
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_LINUX_DIR_PATH}

    #Copy libs dir to libraries
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${LIB_LINUX_DIR_PATH}
      ${RUNTIME_LIB_DIR}

    COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${SHADER_DIR_PATH}
      ${SHADER_LINUX_DIR_PATH}

    COMMAND ${CMAKE_COMMAND} -E echo
        "Copied runtime libraries to ${RUNTIME_LIB_DIR}"
  )

elseif(APPLE) # macOS
  target_compile_definitions(Merlin PRIVATE MACOS)
#
#
endif()

