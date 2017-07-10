#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sp/sphere.h>

/* function prototypes */
void print_usage(void);

void print_usage(void) {
    char *usage="Usage: w_diff [-hvdws] [-c CSTR] file1 file2\n\
       Compare the two NIST SPHERE headered files.\n\n\
Where: -cCSTR Convert the files by the 'CSTR' used in a call to \n\
              sp_set_data_mode()\n\
       -d     Compare the data portions of the files byte by byte.\n\
       -h     Print this help message.\n\
       -s     Compare the SPHERE headers of the files.\n\
       -w     Compare the waveforms of the two files after converting them\n\
              using the conversion string in 'CSTR'.  This is the default\n\
       -v     Set the verbosity level, repeat the v for higher levels\n\n";
    fprintf(spfp,usage);
}

int main(int argc, char **argv)
{
    char *conversion_flag, *filename1, *filename2;
    int c, exit_status=0;
    int do_waveform_diff=0, do_data_diff=0, do_header_diff=0;
    int do_default_diff=1;
    char *prog;

    prog = strrchr(argv[0],'/');
    prog = (prog == CNULL) ? argv[0] : (prog + 1);

    while (( c=hs_getopt( argc, argv, "c:hvmMedws" )) != -1 )
	switch ( c ) {
	  case 'c':
	    conversion_flag = hs_optarg;
	    break;
	  case 'd':
	    do_data_diff = 1;
	    do_default_diff = 0;
	    break;
	  case 'w':
	    do_waveform_diff = 1;
	    do_default_diff = 0;
	    break;
	  case 's':
	    do_header_diff = 1;
	    do_default_diff = 0;
	    break;
	  case 'v':
	    sp_verbose ++;
	    break;
	  case 'h':
	    print_usage();
	    exit(0);
	  default:
	    print_usage();
	    printf("Illegal argument: %c\n",c);
	    exit(-1);
	}

    if (sp_verbose > 0) fprintf(spfp,"%s: %s\n",prog,sp_get_version());

    if (do_default_diff)
	do_waveform_diff=1;

    if (argc - hs_optind != 2){
	fprintf(spfp,"Error: Requires 2 filename arguements\n");
	print_usage();
	exit(-1);
    }
    

    filename1 = argv[hs_optind];
    filename2 = argv[hs_optind+1];
    hs_resetopt();

    if (do_waveform_diff){
	if (diff_waveforms(filename1,filename2,conversion_flag,
			   conversion_flag,0,spfp) > 0){
	    fprintf(spfp,"ERROR: Waveforms differ\n");
	    if (sp_verbose > 0) diff_waveforms(filename1,filename2,
					       conversion_flag,
					       conversion_flag,1,spfp);
	    exit_status = (-1);
	}
	else 
	    if (sp_verbose > 0) fprintf(spfp,"Waveforms are the same\n");
    }

    if (do_data_diff){
	if (diff_data(filename1,filename2,0,spfp) > 0){
	    fprintf(spfp,"ERROR: Raw Waveform data differ\n");
	    if (sp_verbose > 0 && 
		(!strsame(filename1,"-") && !strsame(filename2,"-")))
		diff_data(filename1,filename2,1,spfp);
	    exit_status = (-1);
	}
	else
	    if (sp_verbose > 0) fprintf(spfp,"Raw Waveform are the same\n");
    }
    if (do_header_diff){
	int chg, ins, del;
	diff_header(filename1,filename2,&chg,&ins,&del,0,spfp);
	if (chg > 0 || ins > 0 || del > 0){
	    fprintf(spfp,"ERROR: SPHERE headers differ\n");
	    if (sp_verbose > 0 &&
		(!strsame(filename1,"-") && !strsame(filename2,"-")))
		diff_header(filename1,filename2,&chg,&ins,&del,1,spfp);
	    exit_status = (-1);
	}
	else
	    if (sp_verbose > 0)
		fprintf(spfp,"SPHERE headers are the same\n");
    }

    exit(exit_status);
}

