#	Wayland library
find_package(Wayland	REQUIRED)

#	Main executable
add_executable(${PROJECT_NAME}) 
target_sources(${PROJECT_NAME} 
PRIVATE 
	main.c
)

target_include_directories(${PROJECT_NAME} 
PRIVATE 
	${PKG_WAYLAND_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} 
PRIVATE
	${PKG_WAYLAND_LIBRARIES}
)
