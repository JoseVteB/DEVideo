#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>

#include "client.h"
#include "renderer.h"
#include "xdg-shell-client-protocol.h"

/* Wayland client context */
typedef struct WLstate {
	/* Globals */
	struct wl_display*	pDisplay;
	struct wl_registry*	pRegistry;
	struct wl_compositor*	pCompositor;
	struct xdg_wm_base*	pXDGwmBase;
	/* Objects */
	struct wl_surface*	pSurface;
	struct xdg_surface*	pXDGsurface;
	struct xdg_toplevel*	pXDGtoplevel;
} wlState;
static wlState context;

static void 
xdg_surface_configure(void* pData, 
		      struct xdg_surface* pXDG_surface, 
		      uint32_t serial) 
{
	wlState* pState = pData;
	/* TODO */
}

static const struct xdg_surface_listener 
xdg_surface_listener = {
	.configure = xdg_surface_configure, 
};

static void 
xdg_wm_base_ping(void* pData, struct xdg_wm_base* pXDG_wm_base, uint32_t serial) 
{
	xdg_wm_base_pong(pXDG_wm_base, serial);
}

static const struct xdg_wm_base_listener 
xdg_wm_base_listener = {
	.ping = xdg_wm_base_ping, 
};

/* Register globals */
static void
registry_handle_global(void* pData, 
		       struct wl_registry* pRegistry, 
		       uint32_t name, 
		       const char* pInterface, 
		       uint32_t version) 
{
	wlState* pState = pData;
	if (strcmp(pInterface, wl_compositor_interface.name) == 0) {
		pState->pCompositor = wl_registry_bind(pRegistry, 
							name, 
							&wl_compositor_interface, 
							version);
	} else if (strcmp(pInterface, xdg_wm_base_interface.name) == 0) {
		pState->pXDGwmBase = wl_registry_bind(pRegistry, 
							  name, 
							  &xdg_wm_base_interface, 
							  version);
		xdg_wm_base_add_listener(pState->pXDGwmBase, 
			   		&xdg_wm_base_listener, 
			   		pState);
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
	context.pDisplay = wl_display_connect(nullptr);
	if (!context.pDisplay) {
		fputs("Failed to connect to a Wayland display.\n", stderr);
		return EXIT_FAILURE;
	}

	/* Registry */
	context.pRegistry = wl_display_get_registry(context.pDisplay);
	wl_registry_add_listener(context.pRegistry, &registry_listener, &context);
	wl_display_roundtrip(context.pDisplay);

	/* Surfaces */
	context.pSurface = wl_compositor_create_surface(context.pCompositor);
	context.pXDGsurface = xdg_wm_base_get_xdg_surface(context.pXDGwmBase, 
			      					context.pSurface);
	xdg_surface_add_listener(context.pXDGsurface, &xdg_surface_listener, &context);

	context.pXDGtoplevel = xdg_surface_get_toplevel(context.pXDGsurface);
	xdg_toplevel_set_title(context.pXDGtoplevel, "DEVideo");

	init_renderer("DEVideo");

	return EXIT_SUCCESS;
}

void 
close_client(void) 
{
	close_renderer();

	xdg_toplevel_destroy(context.pXDGtoplevel);
	xdg_surface_destroy(context.pXDGsurface);
	xdg_wm_base_destroy(context.pXDGwmBase);
	wl_surface_destroy(context.pSurface);

	wl_compositor_destroy(context.pCompositor);
	wl_registry_destroy(context.pRegistry);
	wl_display_disconnect(context.pDisplay);
}
