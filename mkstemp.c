/*
 * NAME
 *        mkstemp - create a unique temporary file
 * 
 * SYNOPSIS
 *        int mkstemp(char *template);
 *
 * DESCRIPTION
 *        The mkstemp() function generates a unique temporary file name from tem-
 *        plate.  The last six characters of template must be  XXXXXX  and  these
 *        are  replaced with a string that makes the filename unique. The file is
 *        then created with mode read/write  and permissions 0600.  Since it will
 *        be modified,  template  must not  be a string  constant, but  should be
 *        declared as a character array. The file is opened with the O_EXCL flag, 
 *        guaranteeing  that when mkstemp  returns successfully  we are  the only
 *        user.
 *
 * RETURN VALUE
 *        The mkstemp() function returns the file descriptor fd of the  temporary
 *        file or -1 on error.
 *
 * ERRORS
 *        EINVAL The  last  six characters of template were not XXXXXX.  Now tem-
 *               plate is unchanged.
 *
 *        EEXIST Could not create a unique temporary filename.  Now the  contents
 *               of template are undefined.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int mkstemp(char *template) {
	int retfd=-1;
	int i, temp_len;

	temp_len=strlen(template);
	if (temp_len<6) return(-1);
	if (strcmp(template+(temp_len-6), "XXXXXX")!=0) return(-1);

	srand(getpid()+temp_len);
	for (i=0; i<6; i++) {
		template[temp_len-i-1]='A'+((rand()+i)%26);
		retfd=open(template, O_EXCL|O_CREAT|O_RDWR, 0600);
		if (retfd>=0) break;
	}
	
	return(retfd);
}
