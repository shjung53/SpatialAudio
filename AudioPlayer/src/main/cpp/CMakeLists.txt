cmake_minimum_required(VERSION 3.22.1)

project("AudioPlayer")

set(OBOE_DIR ../../../../oboe-main)

add_subdirectory(${OBOE_DIR} ./oboe-bin)

include_directories(
        ${OBOE_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}
)

set(APP_SOURCES
        AudioPlayer.cpp
        SimpleNoiseMaker.cpp
)

add_library(${CMAKE_PROJECT_NAME} SHARED
        ${APP_SOURCES}
)

target_link_libraries(${CMAKE_PROJECT_NAME}
        oboe
        log
)
