#include "dact.h"
#include "dact_common.h"
#include "libdact.h"
#include "net.h"

int dact_init(void) {
	static int called = 0;

	if (called) {
		return(0);
	}

	if (dact_init_net() < 0) {
		dact_nonetwork = 1;
	}

	called = 1;
	return(0);
}

void *dact_openfile(const char *pathname) {
	return(NULL);
}

int dact_BuffToBuffDecompress(void) {
	dact_init();
	return(0);
}
int dact_BuffToBuffCompress(void) {
	dact_init();
	return(0);
}
