#ifndef _RLS_H
#define _RLS_H

#include "marklist.h"

marktype rls(const marktype image, int tx, int ty);

marktype rls_smoothed_x(const marktype image, int tx);
marktype rls_smoothed_y(const marktype image, int ty);

#endif
