
/* File: w_decode.c */

/************************************************/
/* This program decodes data in a NIST file     */
/* in place (i.e. the filename is not changed). */
/************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <sp/sphere.h>
char usage[] = "Usage:  %s [-vf] [ -oOUT ] { filein | - } { fileout | - }\n\
        %s [-vi] [ -oOUT ] file1 file2 ...\n\
Where:\n\
	-v	Set the verbose setting up by 1\n\
	-f	Force the output file to be overwritten\n\
	-i	Update the file inplace\n\
	-oOUT	Set the output format to the following formats:\n\
	         	alaw\n\
	         	ulaw\n\
	         	pculaw\n\
			pcm_01 | short_01\n\
			pcm_10 | short_10\n\
			pcm | short_natural\n";

char * prog;


int main(int argc, char **argv)
{
    struct stat fileinfo;
    int in_place = 0;
    int force_overwrite = 0;
    char format_conversion[100];
    int c;

    format_conversion[0] = '\0';
    prog = strrchr( argv[0], '/' );
    prog = ( prog == CNULL ) ? argv[0] : ( prog + 1 );

    strcpy(format_conversion,"SE-ORIG:SBF-ORIG");

    while (( c=hs_getopt( argc, argv, "mvifo:" )) != -1 )
	switch ( c ) {
	  case 'v':
	    sp_verbose++;
	    break;
	  case 'i':
	    in_place=1;
	    break;
	  case 'f':
	    force_overwrite=1;
	    break;
	  case 'm':
	    mtrf_set_verbose(1);
	    break;
	  case 'o':
	    if (strsame(hs_optarg,"short_01") || strsame(hs_optarg,"pcm_01"))
		strcpy(format_conversion,"SE-PCM:SBF-01");
	    else  if (strsame(hs_optarg,"short_10") || 
		      strsame(hs_optarg,"pcm_10"))
		strcpy(format_conversion,"SE-PCM:SBF-10");
	    else  if (strsame(hs_optarg,"short_natural") || 
		      strsame(hs_optarg,"pcm"))
		strcpy(format_conversion,"SE-PCM:SBF-N");
	    else  if (strsame(hs_optarg,"ulaw"))
		strcpy(format_conversion,"SE-ULAW");
	    else  if (strsame(hs_optarg,"pculaw"))
		strcpy(format_conversion,"SE-PCULAW");
	    else  if (strsame(hs_optarg,"alaw"))
		strcpy(format_conversion,"SE-ALAW");
	    else{
		fprintf(spfp,"Error: unknown output type option '%s'\n",
			hs_optarg);
		fprintf(spfp,usage,prog,prog);
		exit(-1);
	    }
	    break;
	  default:
	    fprintf(spfp,"Error: unknown flag -%c\n",c);
	    fprintf(spfp,usage,prog,prog);
	    exit(-1);
	}

    if (sp_verbose > 0) fprintf(spfp,"%s: %s\n",prog,sp_get_version());

    if (in_place == 0){
	char *filein, *fileout;

	if (argc - hs_optind != 2){
	    fprintf(spfp,"Error: Requires 2 filename arguements\n");
	    fprintf(spfp,usage,prog,prog);
	    exit(-1);
	}

	filein=argv[hs_optind];
	fileout=argv[hs_optind+1];
	if (sp_verbose > 0) fprintf(spfp,"%s\n",filein);

	if (force_overwrite == 0){
	    if (stat(fileout,&fileinfo) == 0) {
		fprintf(spfp,
		   "Unable to overwrite output file %s.  Use -f to override\n",
			fileout);
		fprintf(spfp,usage,prog,prog);
		exit(-1);
	    }
	}

	hs_resetopt();
	if (convert_file(filein,fileout,format_conversion,prog) != 0){
	    exit(-1);
	}
    } else {
	int op, baseop;
	if (argc - hs_optind < 1){
	    fprintf(spfp,"Error: Requires at least one filename\n");
	    fprintf(spfp,usage,prog,prog);
	    exit(-1);
	}
	baseop = hs_optind;
	hs_resetopt();
	for (op=baseop; op<argc; op++){
	    if (sp_verbose > 0) fprintf(spfp,"%s\n",argv[op]);
	    if (do_update(argv[op],format_conversion,prog) != 0)
		exit(-1);
	}
    }
    exit(0);
}
