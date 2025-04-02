#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>

#include "client.h"

/* Wayland client context */
struct WLContext {
	struct wl_display*	pDisplay;
	struct wl_registry*	pRegistry;
	struct wl_compositor*	pCompositor;
	struct wl_surface*	pSurface;
	struct xdg_surface*	pXDGsurface;
	struct xdg_toplevel*	pXDGtoplevel;
} wlContext;

/* Register globals */
static void
registry_handle_global(void* pData, 
		       struct wl_registry* pRegistry, 
		       uint32_t name, 
		       const char* pInterface, 
		       uint32_t version) 
{
	if (strcmp(pInterface, wl_compositor_interface.name) == 0) {
		wlContext.pCompositor = wl_registry_bind(pRegistry, 
							name, 
							&wl_compositor_interface, 
							version);
	} 
}

static void 
registry_handle_global_remove(void* pData, 
			      struct wl_registry* pRegistry, 
			      uint32_t name) 
{
	// This space deliberately left blank
}

static const struct wl_registry_listener 
registry_listener = {
	.global = registry_handle_global, 
	.global_remove = registry_handle_global_remove, 
};

int 
init_client(void) 
{
	/* Display */
	wlContext.pDisplay = wl_display_connect(nullptr);
	if (!wlContext.pDisplay) {
		fputs("Failed to connect to a Wayland display.\n", stderr);
		return EXIT_FAILURE;
	}

	/* Registry */
	wlContext.pRegistry = wl_display_get_registry(wlContext.pDisplay);
	wl_registry_add_listener(wlContext.pRegistry, &registry_listener, nullptr);
	wl_display_roundtrip(wlContext.pDisplay);

	return EXIT_SUCCESS;
}

void 
close_client(void) 
{
	wl_compositor_destroy(wlContext.pCompositor);
	wl_registry_destroy(wlContext.pRegistry);
	wl_display_disconnect(wlContext.pDisplay);
}
