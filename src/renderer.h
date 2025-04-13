#ifndef	RENDERER_H
#define	RENDERER_H

#include <wayland-client.h>

int 
init_renderer(const char* appName, 
	      struct wl_display* pDisplay, 
	      struct wl_surface* pSurface);

void 
render_surface(void);

void 
close_renderer(void);

#endif	/* RENDERER_H */
