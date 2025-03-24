#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <wayland-client.h>

int 
main(int argc, char* argv[])
{
	struct wl_display* pDisplay = wl_display_connect(nullptr);
	if (!pDisplay) {
		fputs("Failed to connect to a Wayland display.\n", stderr);
		return EXIT_FAILURE;
	}
	puts("Connection stablished!");

	return EXIT_SUCCESS;
}
