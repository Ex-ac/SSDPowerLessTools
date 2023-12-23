cmake_minimum_required(VERSION 3.14)

set(ModuleName Share)
set(${ModuleName}Headers
	${CMAKE_CURRENT_LIST_DIR}
	CACHE STRING "${ModuleName} Headers"
)

set(${ModuleName}Sources
	${CMAKE_CURRENT_LIST_DIR}/SimpleFifo.c
)



add_library(${ModuleName} STATIC)
target_sources(${ModuleName} PUBLIC ${${ModuleName}Sources})
target_include_directories(${ModuleName} PUBLIC ${${ModuleName}Headers})
# set(${ModuleName}Sources
# 	${CMAKE_CURRENT_LIST_DIR}/Share.c
# )