
/* File: w_encode.c */

/************************************************/
/* This program encodes data in a NIST file     */
/* in place (i.e. the filename is not changed). */
/************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <sp/sphere.h>
char usage[] = "Usage:  %s [-mvf] [ -t ENC ] { filein | - } { fileout | - }\n\
        %s [-mvi] [ -t ENC ] file1 file2 ...\n\
Where:\n\
	-v	Set the verbose setting up by 1\n\
	-f	Force the output file to be overwritten\n\
	-i	Update the file inplace\n\
	-m	Maintain the input sample_byte_format, Default is to\n\
		convert to the machine's natural sample byte format\n\
	-tENC	Set the output sample encoding/compression to the following\
 formats:\n\
			wavpack\n\
			shorten\n\
			alaw\n\
			ulaw\n\
			pculaw\n\n";

char * prog;

double compute_compression_percent(char *);

int main(int argc, char **argv)
{
    struct stat fileinfo;
    int in_place = 0;
    int force_overwrite = 0;
    int maintain_sbf = 0;
    char format_conversion[256];
    int c;
    char *format_conversion_type=CNULL;

    format_conversion[0] = '\0';

    prog = strrchr( argv[0], '/' );
    prog = ( prog == CNULL ) ? argv[0] : ( prog + 1 );


    while (( c=hs_getopt( argc, argv, "mvift:" )) != -1 )
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
	    maintain_sbf=1;
	    break;
	  case 't':
	    if (strsame(hs_optarg,"wavpack"))
		format_conversion_type = "wavpack";
	    else  if (strsame(hs_optarg,"shorten"))
		format_conversion_type = "shorten";
	    else  if (strsame(hs_optarg,"ulaw"))
		format_conversion_type = "ulaw";
	    else  if (strsame(hs_optarg,"pculaw"))
		format_conversion_type = "pculaw";
	    else  if (strsame(hs_optarg,"alaw"))
		format_conversion_type = "alaw";
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

    if (format_conversion_type == CNULL){
	fprintf(spfp,"Output conversion type required\n");
	fprintf(spfp,usage,prog,prog);
	exit(-1);
    }

    if (strsame(format_conversion_type,"wavpack"))
	strcat(format_conversion,"SE-WAVPACK:");
    else  if (strsame(format_conversion_type,"shorten"))
	strcat(format_conversion,"SE-SHORTEN:");
    else  if (strsame(format_conversion_type,"ulaw"))
	strcat(format_conversion,"SE-ULAW:");
    else  if (strsame(format_conversion_type,"pculaw"))
	strcat(format_conversion,"SE-PCULAW:");
    else  if (strsame(format_conversion_type,"alaw"))
	strcat(format_conversion,"SE-ALAW:");
    else {
	fprintf(spfp,"Internal error\nContact Jon Fiscus");
	exit(-1);
    }
    if (maintain_sbf == 1)
	strcat(format_conversion,"SBF-ORIG");
    else
	strcat(format_conversion,"SBF-N");

    if (in_place == 0){
	char *filein, *fileout;

	if (argc - hs_optind != 2){
	    fprintf(spfp,"Error: Requires 2 filename arguements\n");
	    fprintf(spfp,usage,prog,prog);
	    exit(-1);
	}

	filein=argv[hs_optind];
	fileout=argv[hs_optind+1];
	if (sp_verbose > 0) {fprintf(spfp,"%s -> %s",filein,fileout);
			     fflush(spfp);}

	if (force_overwrite == 0){
	    if (stat(fileout,&fileinfo) == 0) {
		fprintf(spfp,"Unable to overwrite output file %s. ",fileout);
		fprintf(spfp," Use -f to override\n");
		fprintf(spfp,usage,prog,prog);
		exit(-1);
	    }
	}

	hs_resetopt();
	if (convert_file(filein,fileout,format_conversion,prog) != 0){
	    exit(-1);
	}
	if (sp_verbose > 0){
	    if ((! strsame(format_conversion_type,"")) &&
		 (! strsame(fileout,"-")))
		fprintf(spfp," %5.2f%% Compression\n",
			compute_compression_percent(fileout));
	    else
		fprintf(spfp,"\n");
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
	    if (sp_verbose > 0) {fprintf(spfp,"%s",argv[op]); fflush(spfp);}
	    if (do_update(argv[op],format_conversion,prog) != 0)
		exit(-1);
	    if (sp_verbose > 0){
		if (! strsame(format_conversion_type,""))
		    fprintf(spfp," %5.2f%% Compression\n",
			    compute_compression_percent(argv[op]));
		else
		    fprintf(spfp,"\n");
	    }	    
	}
    }
    exit(0);
}

double compute_compression_percent(char *file){
    struct stat fileinfo;
    SP_FILE *sp;
    SP_INTEGER sc, cc, snb;
    double percent;

    if (stat(file,&fileinfo) != 0)
	return(-1.0);

    if ((sp = sp_open(file, "r")) == SPNULL){
	sp_print_return_status(spfp);
	return(-1.0);
    }

    if ((sp_h_get_field(sp,SAMPLE_COUNT_FIELD, T_INTEGER,(void *)&sc) != 0) ||
	(sp_h_get_field(sp,SAMPLE_N_BYTES_FIELD, T_INTEGER,(void *)&snb)!=0)||
	(sp_h_get_field(sp,CHANNEL_COUNT_FIELD, T_INTEGER,(void *)&cc)!=0)){
	sp_close(sp);
	return(-1.0);
    }
    percent = ((double)(sc * snb * cc) - 
	           (double)(fileinfo.st_size -
			   sp->read_spifr->waveform->header_data_size)) /
	      ((double)(sc * snb * cc)) * 100.0;
    sp_close(sp);

    return(percent);
}
    

    
    
