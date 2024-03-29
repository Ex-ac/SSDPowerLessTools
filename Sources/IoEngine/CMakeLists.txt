cmake_minimum_required(VERSION 3.14)

set(ModuleName IoEngine)
set(${ModuleName}Headers
	${CMAKE_CURRENT_LIST_DIR}

	cache string "${ModuleName} Headers"
)
set(${ModuleName}Sources
	${CMAKE_CURRENT_LIST_DIR}/IoEngineFile.c
	${CMAKE_CURRENT_LIST_DIR}/IoEngineAio.c
	${CMAKE_CURRENT_LIST_DIR}/IoEngine.c

)


add_library(${ModuleName} STATIC)
target_sources(${ModuleName} PUBLIC ${${ModuleName}Sources})
target_include_directories(${ModuleName} PUBLIC ${${ModuleName}Headers})
target_link_libraries(${ModuleName} Share Disk)



# gtest
include(CTest)
enable_testing()
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(${ModuleName}Test)
target_sources(${ModuleName}Test PUBLIC ${ModuleName}Test.cpp)
target_include_directories(${ModuleName}Test PUBLIC ${${ModuleName}Headers})
target_link_libraries(${ModuleName}Test Share ${ModuleName})
target_link_libraries(${ModuleName}Test ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

# target_link_libraries(${ModuleName}Test ${ModuleName})

gtest_discover_tests(${ModuleName}Test)

set(CPACK_PROJECT_NAME ${ProjectName})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
