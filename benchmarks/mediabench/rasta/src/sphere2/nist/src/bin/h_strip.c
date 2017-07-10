
/* File: h_strip.c */

/***************************************************/
/* This program removes a NIST header from a file. */
/***************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sp/sphere.h>


int main(int argc, char **argv)
{
register FILE *fp, *outfp;
struct header_t *h;
char *prog, *errmsg;
static char usage[] = "Usage:  %s {-|inputfile} {-|outputfile}\n";

prog = strrchr(argv[0],'/');
prog = (prog == CNULL) ? argv[0] : (prog + 1);

if (argc != 3) {
	fprintf(spfp,"%s: %s\n",prog,sp_get_version());
	(void) fprintf(stderr,usage,prog);
	exit(ERROR_EXIT_STATUS);
}

if (strcmp(argv[1],"-") == 0)
	fp = stdin;
else {
	fp = fopen(argv[1],"r");
	if (fp == FPNULL) {
		(void) fprintf(stderr,"%s: could not open %s\n",prog,argv[1]);
		exit(ERROR_EXIT_STATUS);
	}
}

if (strcmp(argv[2],"-") == 0)
	outfp = stdout;
else {
	outfp = fopen(argv[2],"w");
	if (outfp == FPNULL) {
		(void) fprintf(stderr,"%s: could not open %s\n",prog,argv[2]);
		exit(ERROR_EXIT_STATUS);
	}
}

h = sp_open_header(fp,FALSE,&errmsg);
if (h == HDRNULL) {
	(void) fprintf(stderr,
		"%s: error reading header from %s -- %s\n",
		prog,argv[1],errmsg);
	exit(ERROR_EXIT_STATUS);
}

if (sp_fpcopy(fp,outfp) < 0) {
	(void) fprintf(stderr,"%s: error copying samples\n",prog);
	exit(ERROR_EXIT_STATUS);
}

if (sp_close_header(h) < 0) {
	(void) fprintf(stderr,
		       "%s: sp_close_header for %s failed\n",prog,argv[1]);
	exit(ERROR_EXIT_STATUS);
}

exit(0);
}
