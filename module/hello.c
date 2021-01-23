#include "iopmod/module.h"
#include "iopmod/stdio.h"

static enum module_init_status hello(int argc, char *argv[])
{
	printf("Hello World!\n");

	return MODULE_EXIT;
}
module_init(hello);
