#	Wayland library
#
find_package(Wayland	REQUIRED)
find_package(Vulkan	REQUIRED)

set(BUILD_DIR	${PROJECT_BINARY_DIR}/${PROJECT_NAME})

#	Wayland Protocols
#
set(WL_PROTOCOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wayland-protocols)
file(MAKE_DIRECTORY ${WL_PROTOCOLS_DIR})
add_custom_command(
	OUTPUT ${WL_PROTOCOLS_DIR}/xdg-shell-protocol.c 
	COMMAND wayland-scanner private-code 
		< ${WL_PROTOCOLS_PATH}/stable/xdg-shell/xdg-shell.xml 
		> ${WL_PROTOCOLS_DIR}/xdg-shell-protocol.c
	COMMAND wayland-scanner client-header
		< ${WL_PROTOCOLS_PATH}/stable/xdg-shell/xdg-shell.xml 
		> ${WL_PROTOCOLS_DIR}/xdg-shell-client-protocol.h
)

#	Shaders
#
set(SHADERS_DIR ${BUILD_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADERS_DIR})
add_custom_command(
	OUTPUT	vert.spv
	COMMAND ${Vulkan_GLSLC_EXECUTABLE} 
		shader.vert 
		-o ${SHADERS_DIR}/vert.spv 
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/shaders 
)

add_custom_command(
	OUTPUT	frag.spv
	COMMAND ${Vulkan_GLSLC_EXECUTABLE} 
		shader.frag 
		-o ${SHADERS_DIR}/frag.spv 
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/shaders 
)
add_custom_target(shaders 
DEPENDS
	vert.spv 
	frag.spv 
)

#	Main executable
#
add_executable(${PROJECT_NAME}) 
add_dependencies(${PROJECT_NAME} shaders)

set_target_properties(${PROJECT_NAME} 
PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY	${PROJECT_BINARY_DIR}/${PROJECT_NAME}
)

list(APPEND MAIN_SOURCES
	main.c 
	client.c
	controller.c 
	devices.c 
	pipeline.c 
	renderer.c 
	validation.c 
)
target_sources(${PROJECT_NAME} 
PRIVATE 
	${MAIN_SOURCES} 
	${WL_PROTOCOLS_DIR}/xdg-shell-protocol.c
)

target_include_directories(${PROJECT_NAME} 
PRIVATE 
	${WL_PROTOCOLS_DIR} 
	${PKG_WAYLAND_INCLUDE_DIRS} 
)

target_link_libraries(${PROJECT_NAME} 
PRIVATE
	${PKG_WAYLAND_LIBRARIES} 
	Vulkan::Vulkan 
)
