#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>

#include "client.h"
#include "renderer.h"
#include "xdg-shell-client-protocol.h"

/* Wayland client state */
typedef struct WLState {
	/* Globals */
	struct wl_display*	pDisplay;
	struct wl_registry*	pRegistry;
	struct wl_compositor*	pCompositor;
	struct xdg_wm_base*	pXDGwmBase;
	struct wl_seat*		pSeat;
	/* Objects */
	struct wl_surface*	pSurface;
	struct xdg_surface*	pXDGsurface;
	struct xdg_toplevel*	pXDGtoplevel;
	struct wl_keyboard*	pKeyboard;
	/* State */
	struct xkb_state*	pXKBstate;
	struct xkb_context*	pXKBcontext;
	struct xkb_keymap*	pXKBkeymap;
} wlState;
static wlState state;

static void 
xdg_surface_configure(void* pData, 
		      struct xdg_surface* pXDG_surface, 
		      uint32_t serial) 
{
	wlState* pState = pData;

	xdg_surface_ack_configure(pXDG_surface, serial);
	wl_surface_commit(pState->pSurface);
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

static void 
wl_keyboard_keymap(void* pData, 
		   struct wl_keyboard* pKeyboard, 
		   uint32_t format, 
		   int32_t fd, 
		   uint32_t size) 
{
	wlState* pState = pData;
	assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);

	char* map_shm = mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
	assert(map_shm != MAP_FAILED);

	struct xkb_keymap* pXKBkeymap = 
		xkb_keymap_new_from_string(pState->pXKBcontext, 
					     map_shm, 
					     XKB_KEYMAP_FORMAT_TEXT_V1, 
					     XKB_KEYMAP_COMPILE_NO_FLAGS);
	munmap(map_shm, size);
	close(fd);

	struct xkb_state* pXKBstate = xkb_state_new(pXKBkeymap);
	xkb_keymap_unref(pState->pXKBkeymap);
	xkb_state_unref(pState->pXKBstate);
	pState->pXKBkeymap = pXKBkeymap;
	pState->pXKBstate = pXKBstate;
}

static void 
wl_keyboard_enter(void* pData, 
		  struct wl_keyboard* pKeyboard, 
		  uint32_t serial, 
		  struct wl_surface* pSurface, 
		  struct wl_array* keys) 
{
	/* This space deliberately left blank. */
}

static void 
wl_keyboard_key(void* pData, 
		struct wl_keyboard* pKeyboard, 
		uint32_t serial, 
		uint32_t time, 
		uint32_t key, 
		uint32_t state) 
{
	wlState* pState = pData;

	/* TODO: temporary solution to close the client. */
	uint32_t keycode = key + 8;
	xkb_keysym_t sym = xkb_state_key_get_one_sym(pState->pXKBstate, keycode);
	if (sym == XKB_KEY_q) {
		close_client();
	}
}

static void 
wl_keyboard_leave(void* pData, 
		  struct wl_keyboard* pKeyboard, 
		  uint32_t serial, 
		  struct wl_surface* pSurface) 
{
	/* This space deliberately left blank. */
}

static void 
wl_keyboard_modifiers(void* pData, 
		      struct wl_keyboard* pKeyboard, 
		      uint32_t serial, 
		      uint32_t mods_depressed, 
		      uint32_t mods_latched, 
		      uint32_t mods_locked, 
		      uint32_t group) 
{
	wlState* pState = pData;

	xkb_state_update_mask(pState->pXKBstate, 
			       mods_depressed, 
			       mods_latched, 
			       mods_locked, 
			       0, 0, group);
}

static void 
wl_keyboard_repeat_info(void* data, 
			struct wl_keyboard* pKeyboard, 
			int32_t rate, 
			int32_t delay) 
{
	/* Intentionally left blank. */
}

static const struct wl_keyboard_listener 
wl_keyboard_listener = {
	.keymap = wl_keyboard_keymap, 
	.enter = wl_keyboard_enter, 
	.leave = wl_keyboard_leave, 
	.key = wl_keyboard_key, 
	.modifiers = wl_keyboard_modifiers, 
	.repeat_info = wl_keyboard_repeat_info, 
};

static void 
wl_seat_capabilities(void* pData, 
		     struct wl_seat* pSeat, 
		     uint32_t capabilities) 
{
	wlState* pState = pData;
	bool have_keyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;

	if (have_keyboard && pState->pKeyboard == nullptr) {
		pState->pKeyboard = wl_seat_get_keyboard(pState->pSeat);
		wl_keyboard_add_listener(pState->pKeyboard, 
					   &wl_keyboard_listener, 
					   &state);
	} else if (!have_keyboard && pState->pKeyboard != nullptr) {
		wl_keyboard_release(pState->pKeyboard);
		pState->pKeyboard = nullptr;
	}
}

static void 
wl_seat_name(void* pData, struct wl_seat* pSeat, const char* name) 
{
	/* This space deliberatly left blank. */
}

static const struct wl_seat_listener 
wl_seat_listener = {
	.capabilities = wl_seat_capabilities, 
	.name = wl_seat_name, 
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
	} else if (strcmp(pInterface, wl_seat_interface.name) == 0) {
		pState->pSeat = wl_registry_bind(pRegistry, 
						   name, 
						   &wl_seat_interface, 
						   version);
		wl_seat_add_listener(pState->pSeat, &wl_seat_listener, pState);
	}
}

static void 
registry_handle_global_remove(void* pData, 
			      struct wl_registry* pRegistry, 
			      uint32_t name) 
{
	/* This space deliberately left blank */
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
	state.pDisplay = wl_display_connect(nullptr);
	if (!state.pDisplay) {
		fputs("Failed to connect to a Wayland display.\n", stderr);
		return EXIT_FAILURE;
	}

	/* Registry */
	state.pRegistry = wl_display_get_registry(state.pDisplay);
	state.pXKBcontext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	wl_registry_add_listener(state.pRegistry, &registry_listener, &state);
	wl_display_roundtrip(state.pDisplay);

	/* Surfaces */
	state.pSurface = wl_compositor_create_surface(state.pCompositor);
	state.pXDGsurface = xdg_wm_base_get_xdg_surface(state.pXDGwmBase, 
			      					state.pSurface);
	xdg_surface_add_listener(state.pXDGsurface, &xdg_surface_listener, &state);

	state.pXDGtoplevel = xdg_surface_get_toplevel(state.pXDGsurface);
	xdg_toplevel_set_title(state.pXDGtoplevel, "DEVideo");
	wl_surface_commit(state.pSurface);
	wl_display_roundtrip(state.pDisplay);

	if (init_renderer("DEVideo", 
		   	state.pDisplay, 
		   	state.pSurface) != EXIT_SUCCESS) { return EXIT_FAILURE; }

	return EXIT_SUCCESS;
}

void 
update_client(void) 
{
	render_surface();
	wl_display_dispatch(state.pDisplay);
}

void 
close_client(void) 
{
	close_renderer();

	xdg_toplevel_destroy(state.pXDGtoplevel);
	xdg_surface_destroy(state.pXDGsurface);
	xdg_wm_base_destroy(state.pXDGwmBase);
	wl_surface_destroy(state.pSurface);

	wl_compositor_destroy(state.pCompositor);
	wl_registry_destroy(state.pRegistry);
	wl_display_disconnect(state.pDisplay);
}
