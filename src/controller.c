#include <stdio.h>
#include <stdlib.h>

#include "client.h"
#include "controller.h"

static bool terminate = false;

int
init_controller(void) 
{
	if (init_client() == EXIT_FAILURE) {
		fputs("Failed to initialize client!\n", stderr);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void
close_app(void) 
{
	close_client();
}

int 
run_app(void) 
{
	if (init_controller() != EXIT_SUCCESS) { return EXIT_FAILURE; }

	while(!terminate) {
		update_client();
	}

	close_app();

	return EXIT_SUCCESS;
}

