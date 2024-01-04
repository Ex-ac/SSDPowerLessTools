cmake_minimum_required(VERSION 3.16)

set(PorojectName SSDIoTest)
set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/Sources)
set(cmake_c_flags "${cmake_c_flags} -g -Wall -Werror -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-but-set-parameter -Wno-unused-parameter -Wno-unused-value -Wno-unused-label -Wno-unused-result -Wno-unused-local-typedefs -Wno-unused-macros -Wno-unused-const-variable -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-but-set-parameter -Wno-unused-parameter -Wno-unused-value -Wno-unused-label -Wno-unused-result -Wno-unused-local-typedefs -Wno-unused-macros -Wno-unused-const-variable")

set(CMAKE_EXE_LINKER_FLAGS "-Wl,-Map=output.map")

project(${PorojectName})

add_subdirectory(${SOURCE_DIR}/Share)
add_subdirectory(${SOURCE_DIR}/Study)
add_subdirectory(${SOURCE_DIR}/Disk)
add_subdirectory(${SOURCE_DIR}/IoEngine)

add_executable(${PorojectName} 
	${SOURCE_DIR}/main.c
)

target_link_libraries(${PorojectName} aio)