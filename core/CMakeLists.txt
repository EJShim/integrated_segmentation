cmake_minimum_required(VERSION 3.3)

PROJECT(Core)

set(SRC_FILES
    E_Volume
)

add_library(${PROJECT_NAME} ${SRC_FILES})

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})


find_package(Tensorflow)
target_link_libraries(${PROJECT_NAME} tensorflow)