#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>

#include "controller.h"

int 
main(int argc, char* argv[])
{
	const char* appPath = dirname(argv[0]);
	chdir(appPath);

	return run_app();
}
