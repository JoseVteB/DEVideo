find_package(PkgConfig)
pkg_check_modules(PKG_WAYLAND REQUIRED wayland-client)

#	Wayland-protocols
#
find_program(WL_SCANNER wayland-scanner REQUIRED)
if (NOT WL_SCANNER)
	message(FATAL_ERROR "Wayland-scanner required and not found.")
endif()

unset(WL_PROTOCOLS_PATH)
execute_process(
COMMAND 
	${PKG_CONFIG_EXECUTABLE} --variable=pkgdatadir wayland-protocols 
OUTPUT_VARIABLE 
	WL_PROTOCOLS_PATH OUTPUT_STRIP_TRAILING_WHITESPACE
)

mark_as_advanced(WL_PROTOCOLS_PATH)
