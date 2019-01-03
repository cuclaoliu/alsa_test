cmake_minimum_required(VERSION 3.13)
project(alsa_test)

set(CMAKE_CXX_STANDARD 11)

include_directories(
        "${PROJECT_SOURCE_DIR}/sound"
        "${PROJECT_SOURCE_DIR}"
)


    #ALSA library
    find_package(ALSA REQUIRED)
    if (NOT ALSA_FOUND)
        message(FATAL_ERROR "ALSA lib not found")
    endif (NOT ALSA_FOUND)
    include_directories(${ALSA_INCLUDE_DIRS})

if(ALSA_LIBRARIES)
    message(STATUS "ALSA library found: " ${ALSA_LIBRARIES})
endif()

#ALSA library
add_library(sound_lib sound/CAlsaSound.cpp)
add_executable(sound_test sound/sound_test.cpp)
#add_executable(laytency sound/test_latency.c)
#target_link_libraries (laytency sound_lib ${ALSA_LIBRARIES})
target_link_libraries (sound_test sound_lib ${ALSA_LIBRARIES})

