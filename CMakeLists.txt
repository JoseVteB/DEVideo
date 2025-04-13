cmake_minimum_required(VERSION 3.30)
project(DEVideo 
	DESCRIPTION	"A simple video decoder/encoder."
	LANGUAGES	C
)

set(CMAKE_C_STANDARD		23)
set(CMAKE_C_STANDARD_REQUIRED	ON)
set(CMAKE_C_EXTENSIONS		OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND	CMAKE_MODULE_PATH	"${PROJECT_SOURCE_DIR}/cmake")

add_subdirectory(src)
