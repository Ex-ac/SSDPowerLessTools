cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME SSDIoTest)
set(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

project(${PROJECT_NAME})


# add_subdirectory(Study)

add_executable(${PROJECT_NAME} 
	${ROOT_PATH}/Study/LearnAio.c
)

target_link_libraries(${PROJECT_NAME} aio)