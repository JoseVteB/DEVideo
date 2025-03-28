#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>

#include "client.h"

/* Wayland client context */
static struct wl_display*	p_display;
static struct wl_registry*	p_registry;
static struct wl_compositor*	p_compositor;

/* Register globals */
static void
registry_handle_global(void* p_data, 
		       struct wl_registry* p_registry, 
		       uint32_t name, 
		       const char* p_interface, 
		       uint32_t version) 
{
	if (strcmp(p_interface, wl_compositor_interface.name) == 0) {
		p_compositor = wl_registry_bind(p_registry, 
						name, 
					  	&wl_compositor_interface, 
					  	version);
	} 
}

static void 
registry_handle_global_remove(void* p_data, 
			      struct wl_registry* p_registry, 
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
	p_display = wl_display_connect(nullptr);
	if (!p_display) {
		fputs("Failed to connect to a Wayland display.\n", stderr);
		return EXIT_FAILURE;
	}

	/* Registry */
	p_registry = wl_display_get_registry(p_display);
	wl_registry_add_listener(p_registry, &registry_listener, nullptr);
	wl_display_roundtrip(p_display);

	return EXIT_SUCCESS;
}

void 
close_client(void) 
{
	wl_display_disconnect(p_display);
}
