cmake_minimum_required(VERSION 3.27)
project(c_media_codec C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(JNI REQUIRED)

message(STATUS "JNI_INCLUDE_DIRS = ${JNI_INCLUDE_DIRS}")
message(STATUS "JNI_LIBRARIES    = ${JNI_LIBRARIES}")

include_directories(
        ${JNI_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/jni
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_library(c_media_codec SHARED
        src/jni_media_codec.c
        src/media_codec_core.c
        src/g711.c
        src/g722_encode.c
        src/g722_decode.c
)

target_link_libraries(c_media_codec
        ${JNI_LIBRARIES}
)

if (WIN32)
    set_target_properties(c_media_codec PROPERTIES PREFIX "")
endif ()