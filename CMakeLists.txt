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
        Dali/Src/Dali2Bus.cpp
)

# Collect and de-duplicate parent directories
set(LIB_INCLUDE_DIRS "")
foreach(hdr IN LISTS LIB_INCLUDES)
    get_filename_component(dir "${hdr}" DIRECTORY)
    list(APPEND LIB_INCLUDE_DIRS "${dir}")
endforeach()
list(REMOVE_DUPLICATES LIB_INCLUDE_DIRS)

include(cpm/CPM.cmake)
CPMAddPackage("gh:JeroenVandezande/LowLevelCPPClasses#5c75a6b03579d81684d6aced572e0a82886b03cf")

target_compile_definitions(LowLevelEmbedded PUBLIC LOWLEVELCPPCLASSES_DISABLE_EXCEPTIONS)

target_link_libraries(${PROJECT_NAME} PUBLIC microlog LowLevelEmbedded)

target_include_directories (${PROJECT_NAME} PUBLIC ${LIB_INCLUDE_DIRS})