cmake_minimum_required(VERSION 3.31.6)
project(Dali2_LEDBallastControl CXX)

set(CMAKE_CXX_STANDARD 20)
enable_language(C ASM CXX)

file(GLOB_RECURSE LIB_INCLUDES "Dali/*.h" "Dali/*.hpp")
file(GLOB_RECURSE LIB_SOURCES "Dali/*.c" "Dali/*.cpp")

add_library(${PROJECT_NAME} ${LIB_INCLUDES} ${LIB_SOURCES}
        Dali/Inc/Dali-defs.h
        Dali/Inc/ESUM-230S500BG-Defs.h
        Dali/Inc/Dali2Bus.h
        Dali/Inc/Dali2Device.h
        Dali/Inc/ESUM230S500BGBallast.h
        Dali/Src/Dali2Bus.cpp
        Dali/Src/Dali2Device.cpp
        Dali/Src/ESUM230S500BGBallast.cpp
)

# Collect and de-duplicate parent directories
set(LIB_INCLUDE_DIRS "")
foreach(hdr IN LISTS LIB_INCLUDES)
    get_filename_component(dir "${hdr}" DIRECTORY)
    list(APPEND LIB_INCLUDE_DIRS "${dir}")
endforeach()
list(REMOVE_DUPLICATES LIB_INCLUDE_DIRS)

message("Downloading CPM Package Manager")
file(
        DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.42.3/CPM.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/CPM.cmake
)
include("${CMAKE_CURRENT_BINARY_DIR}/CPM.cmake")

CPMAddPackage("gh:JeroenVandezande/LowLevelCPPClasses@3.6.1")

target_link_libraries(${PROJECT_NAME} PUBLIC microlog LowLevelEmbedded)

target_include_directories (${PROJECT_NAME} PUBLIC ${LIB_INCLUDE_DIRS})
