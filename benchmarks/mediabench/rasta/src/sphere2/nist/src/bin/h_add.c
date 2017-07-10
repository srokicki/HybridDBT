
/* File: h_add.c */

/******************************************/
/* This program demonstrates how a file   */
/* with a NIST header can be created      */
/******************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sp/sphere.h>

int main(int argc, char **argv)
{
    char *prog;
    static char usage[] = "Usage: %s [-vh] {-|infile} {-|outfile}\n";
    long databytes, hbytes;
    struct header_t *h;
    register FILE *fp1, *fp2, *outputfp = stdout;
    int n, c;
    
    
    prog = strrchr(argv[0],'/');
    prog = (prog == CNULL) ? argv[0] : (prog + 1);
    
    while (( c=hs_getopt( argc, argv, "hv" )) != -1 )
	switch ( c ) {
	  case 'v':
	    sp_verbose ++;
	    break;
	  case 'h':
	    (void) fprintf(stderr,usage,prog);
	    exit(0);
	  default:
	    (void) fprintf(stderr,usage,prog);
	    printf("Illegal argument: %c\n",c);
	    exit(-1);
	}

    if (sp_verbose > 0) fprintf(spfp,"%s: %s\n",prog,sp_get_version());

    if (argc - hs_optind != 2) {
	(void) fprintf(stderr,usage,prog);
	exit(ERROR_EXIT_STATUS);
    }

    
    if (strcmp(argv[hs_optind],"-") == 0)
	fp1 = stdin;
    else {
	fp1 = fopen(argv[hs_optind],"r");
	if (fp1 == FPNULL) {
	    (void) fprintf(stderr,"%s: Could not open %s\n",prog,argv[hs_optind]);
	    exit(ERROR_EXIT_STATUS);
	}
    }
    
    if (strcmp(argv[hs_optind+1],"-") == 0) {
	fp2 = stdout;
	outputfp = stderr;
    } else {
	fp2 = fopen(argv[hs_optind+1],"w");
	if (fp2 == FPNULL) {
	    (void) fprintf(stderr,"%s: Could not open %s\n",prog,argv[hs_optind+1]);
	    exit(ERROR_EXIT_STATUS);
	}
    }
    
    h = sp_create_header();
    if (h == HDRNULL) {
	(void) fprintf(stderr,"%s: Could not create header\n",prog);
	exit(ERROR_EXIT_STATUS);
    }
    
    /* Here is how one could add fields to the raw file */
    /*
      {
      double r = 205.111;
      long l = 27L;
      
      n = sp_add_field(h,"field1",T_INTEGER,(char *) &l);
      if (n>=0) n = sp_add_field(h,"field2",T_REAL,(char *) &r);
      if (n>=0) n = sp_add_field(h,"field3",T_STRING,"foobar");
      if (n < 0) {
      (void) fprintf(stderr,"%s: error adding fields to header\n");
      exit(ERROR_EXIT_STATUS);
      }
      }
      */
    
    n = sp_write_header(fp2,h,&hbytes,&databytes);
    if (n < 0) {
	(void) fprintf(stderr,"%s: sp_write_header failed\n",prog);
	exit(ERROR_EXIT_STATUS);
    }
    
    (void) fprintf(outputfp,
		   "%s: %s --> %s (header=%ld bytes, data=%ld bytes)\n",
		   prog,argv[hs_optind],argv[hs_optind+1],hbytes,databytes);
    
    if (sp_fpcopy(fp1,fp2) < 0) {
	(void) fprintf(stderr,"%s: error copying samples\n",prog);
	exit(ERROR_EXIT_STATUS);
    }
    
    exit(0);
}
