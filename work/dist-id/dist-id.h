#ifndef _DISTID_H
#define _DISTID_H

char *distid_os_name[]={"Slackware Linux","RedHat Linux","SunOS","Mandrake Linux", "FreeBSD", "OpenBSD", "NetBSD", "BSDi"};
char *distid_arch[]={"i586","sparc64", "sparc32", "alpha", "i386"};


/*
 *  The weird number here is the finger print:
 *     [04]  [06]  [08]  [14]
 *    (arch)(vmaj)(vmin)(name) 
 */
#define MD5PS {\
	{"113c2897a324722f5aab7ddb3635d25a", "1086390272"}, /* Slack 3.4(i386) */ \
	{"05dd84221547408c55cb921c241950a8", "1103101952"},/* Slack 7.0 (i386) */ \
	{"65a87e191542811e092c6c6001ce9f57", "289538050"}, /* Solaris 8 (Sparc64) */ \
	{"8e1de84e397983cc5dd52de120c048d3", "1107296256"}, \
	{"ac0b58050deb21db1ed701277521320b", "1103118337"},  /* RedHat Linux 7.1 (i386) */ \
	{"32e0d62fadb68a8bb6c17bfc2d8918b5", "1103118336"},  /* Slackware Linux 7.1 (i386) */ \
	{"8cc6f96d1bd21250b731eb0ac85214a7", "1111490561"}, /* RedHat Linux 9.0 (i386) */ \
}
#endif
