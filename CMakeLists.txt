cmake_minimum_required(VERSION 3.12)
project(lokalso VERSION 1.0.0)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_SOURCE_DIR}/src/LokalSo)

# list of source files
set(libsrc ${CMAKE_SOURCE_DIR}/src/LokalSo/Lokal.cpp)

# this is the "object library" target: compiles the sources only once
add_library(lokal_object OBJECT ${libsrc})

# shared libraries need PIC
set_property(TARGET lokal_object PROPERTY POSITION_INDEPENDENT_CODE 1)

# shared and static libraries built from the same object files
add_library(lokal_shared SHARED $<TARGET_OBJECTS:lokal_object>)
add_library(lokal_static STATIC $<TARGET_OBJECTS:lokal_object>)
target_link_libraries(lokal_static PRIVATE curl)
target_link_libraries(lokal_shared PRIVATE curl)

set_target_properties(lokal_static PROPERTIES OUTPUT_NAME lokal)
set_target_properties(lokal_shared PROPERTIES OUTPUT_NAME lokal)

enable_testing()
add_subdirectory(test)