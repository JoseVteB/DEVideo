#	Required modules
#
find_package(Wayland	REQUIRED)
find_package(Vulkan	REQUIRED)
find_package(X11	REQUIRED)
find_package(Doxygen)

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
	X11::xkbcommon 
)

include(CMakeDependentOption)
cmake_dependent_option(BUILD_DOCS 
	"Generates the project documentation." 
	OFF 
	"DOXYGEN_FOUND" 
	OFF
)
if (BUILD_DOCS)
	set(DOCS_DIR ${BUILD_DIR}/docs)
	file(MAKE_DIRECTORY ${DOCS_DIR})
	set(DOXYGEN_OUTPUT_DIRECTORY ${DOCS_DIR})
	set(DOXYGEN_EXCLUDE ${WL_PROTOCOLS_DIR})

	set(DOXYGEN_QUIET YES)
	set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)
	set(DOXYGEN_VERBATIM_HEADERS NO)
	set(DOXYGEN_HIDE_UNDOC_MEMBERS YES)
	set(DOXYGEN_HIDE_UNDOC_CLASSES YES)

	doxygen_add_docs(${PROJECT_NAME}_docs
		${CMAKE_CURRENT_SOURCE_DIR} 
		ALL 
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs 
		COMMENT "Generating project documentation." 
	)
endif()
