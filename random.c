#include "dact.h"
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include "random.h"
#include "dendian.h"

rnd_id rnd_init(void) {
	int fd;

	fd=open("/dev/urandom", O_RDONLY);
	if (fd<0) fd=open("/dev/random", O_RDONLY);
	return(fd);
}

int64_t rnd_getrandom(const rnd_id id, const int64_t low, const int64_t high) {
	uint64_t randval=0;
	int64_t retval;
	ssize_t read_ret;

	if (id>=0) {
		read_ret=read_de(id, &randval, sizeof(randval), sizeof(randval));
		if (read_ret!=sizeof(randval)) randval=0;
	}
	if (randval==0) {
#if defined(HAVE_SRAND48) && defined(HAVE_LRAND48)
		srand48(time(NULL)+rand());
		randval=lrand48();
#else
		srand(time(NULL)+rand());
		randval=rand();
#endif
	}

	retval=low+(int64_t) ((high-low+1)*rand()/(RAND_MAX+1.0));
	DPRINTF("rnd_getrandom returns %lli [%lli,%lli]", retval, low, high);

	return(retval);
}

void rnd_deinit(const rnd_id id) {
	if (id>=0) close(id);
	return;
}
