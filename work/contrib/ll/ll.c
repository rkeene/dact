#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "linklist.h"

int main(void) {
	struct linklist_info *joe;

	joe=linklist_init(LINKLIST_TYPE_DOUBLE);
	linklist_add(joe, LINKLIST_MODE_APPEND);

	printf("%s\n",joe->link_curr);
	printf("%i\n",joe->link_cnt);

	return(0);
}
