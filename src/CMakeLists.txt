#	Wayland library
find_package(Wayland	REQUIRED)

#	Main executable
add_executable(${PROJECT_NAME}) 
set_target_properties(${PROJECT_NAME} 
PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY	${PROJECT_BINARY_DIR}/bin
)

file(GLOB MAIN_SOURCES *.c)
target_sources(${PROJECT_NAME} 
PRIVATE 
	${MAIN_SOURCES}
)

target_include_directories(${PROJECT_NAME} 
PRIVATE 
	${PKG_WAYLAND_INCLUDE_DIRS} 
)

target_link_libraries(${PROJECT_NAME} 
PRIVATE
	${PKG_WAYLAND_LIBRARIES}
)
