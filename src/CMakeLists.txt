#	Wayland library
find_package(Wayland	REQUIRED)
find_package(Vulkan	REQUIRED)

#	Main executable
add_executable(${PROJECT_NAME}) 

set_target_properties(${PROJECT_NAME} 
PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY	${PROJECT_BINARY_DIR}/bin
)

file(GLOB MAIN_SOURCES *.c)
file(GLOB WL_PROTOCOLS wayland-protocols/*.c)
target_sources(${PROJECT_NAME} 
PRIVATE 
	${MAIN_SOURCES} 
	${WL_PROTOCOLS} 
)

target_include_directories(${PROJECT_NAME} 
PRIVATE 
	${PROJECT_SOURCE_DIR}/src/wayland-protocols 
	${PKG_WAYLAND_INCLUDE_DIRS} 
)

target_link_libraries(${PROJECT_NAME} 
PRIVATE
	${PKG_WAYLAND_LIBRARIES} 
	Vulkan::Vulkan 
)
