cmake_minimum_required(VERSION 3.14)

set(ModuleName IoEngine)
set(${ModuleName}Headers
	${CMAKE_CURRENT_LIST_DIR}

	cache string "${ModuleName} Headers"
)
set(${ModuleName}Sources
	${CMAKE_CURRENT_LIST_DIR}/IoEngineFile.c
	${CMAKE_CURRENT_LIST_DIR}/IoEngineAio.c

)


add_library(${ModuleName} STATIC)
target_sources(${ModuleName} PUBLIC ${${ModuleName}Sources})
target_include_directories(${ModuleName} PUBLIC ${${ModuleName}Headers})
target_include_directories(${ModuleName} PUBLIC ${ShareHeaders})