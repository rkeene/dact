#ifndef __ZONE_H
#define __ZONE_H

#include "marklist.h"


typedef struct {
  int num_zones;
  marklistptr *zone;
} zonetype;

void zone_init(zonetype *z);
void zone_addzone(zonetype *z, marklistptr list);
void zone_addmark(zonetype *z, int zone, marktype m);
void zone_remove(zonetype *z, int zone);
void zone_free(zonetype *z); 

void zone_dump(zonetype *z);

void zone_test();

#endif
