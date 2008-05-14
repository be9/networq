#include <stdio.h>
#include "network.h"

int main()
{
	if (nw_init()) {
		fprintf(stderr, "Error initializing network\n");
		return 1;
	}

	nw_shutdown();

	return 0;
}
