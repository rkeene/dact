#include "conf.h"
#include "crc.h"

uint32_t ELFCRC(const uint32_t start, const unsigned char *name, const uint32_t n) {
	uint32_t i,h,g;

	h=start;
	for (i=0;i<n;i++) {
		h = (h << 4) + (*name++);
		if ((g = (h & 0xf0000000)))
			h ^= (g >> 24);
		h &= ~g;
	}

	return(h);
}

