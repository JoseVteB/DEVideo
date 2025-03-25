find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
	pkg_check_modules(PKG_WAYLAND	wayland-client)
endif()
