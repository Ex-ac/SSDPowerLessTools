cmake_minimum_required(VERSION 3.16)

set(PorojectName SSDIoTest)
set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/Sources)
set(CMAKE_C_FLAGES "${CMAKE_C_FLAGES} -g -Wshadow -Wundef -Wno-pointer-arith")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wshadow -Wundef -Wno-pointer-arith")
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