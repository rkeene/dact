#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <curses.h>
#include <fcntl.h>
#include <stdio.h>
#include "dact.h"
#include "net.h"
#include "parse.h"
#include "ui.h"
void mtrace (void);

uint32_t DACT_BLK_SIZE=3;

int main(int argc, char **argv) {
	char *ret;
	char scheme[5];
	char username[128], password[128];
	char host[512],file[1024];
	int port, x;

#if 0
	mtrace();

	dact_ui_init();

	dact_ui_setup(300);
	for (x=0;x<=100;x++) {
		dact_ui_incrblkcnt(3);
		if (x==13) dact_ui_status(0,"Test.");
		usleep(40000);
	}
	fprintf(stderr, "\n");

	if (argv[1]==NULL) return(1);
#endif

	ret=parse_url_subst(argv[1],"joe");

SPOTVAR_STR(mime64(ret));
SPOTVAR_NUM(hash_fourbyte(ret,':'));


	if (!parse_url(ret,scheme,username,password,host,&port,file)) {
		printf("SCHEME=[%s]\n",scheme);
		printf("HOST  =[%s]\n",host);
		printf("PORT  =[%i]\n",port);
		printf("USER  =[%s]\n",username);
		printf("PASS  =[%s]\n",password);
	}
	printf("FILE  =[%s]\n",file);

	return(0);
}
