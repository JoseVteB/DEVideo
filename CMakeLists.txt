cmake_minimum_required(VERSION 3.30)
project(DEVideo 
	DESCRIPTION	"A simple video decoder/encoder."
	LANGUAGES	C
)

set(CMAKE_C_STANDARD		23)
set(CMAKE_C_STANDARD_REQUIRED	ON)
set(CMAKE_C_EXTENSIONS		OFF)

list(APPEND	CMAKE_MODULE_PATH	"${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(src)
