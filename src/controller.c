#include <stdio.h>
#include <stdlib.h>

#include "client.h"
#include "controller.h"

int
init_controller(void) 
{
	if (init_client() == EXIT_FAILURE) {
		fputs("Failed to initialize client!\n", stderr);
		return EXIT_FAILURE;
	}
	puts("Client initialized!\n");

	return EXIT_SUCCESS;
}

void
close_app(void) 
{
	close_client();
}
