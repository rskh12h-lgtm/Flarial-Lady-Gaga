cmake_minimum_required(VERSION 3.18)
project(bytemod LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(bytemod SHARED src/mod.cpp)

find_library(log-lib log)
target_link_libraries(bytemod ${log-lib})

# Sortie : libbytemod.so
set_target_properties(bytemod PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/${ANDROID_ABI})
