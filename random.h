#ifndef _DACT_RANDOM_H
#define _DACT_RANDOM_H

typedef int rnd_id;

rnd_id rnd_init(void);
int64_t rnd_getrandom(const rnd_id id, const int64_t low, const int64_t high);
void rnd_deinit(const rnd_id id);


#endif
