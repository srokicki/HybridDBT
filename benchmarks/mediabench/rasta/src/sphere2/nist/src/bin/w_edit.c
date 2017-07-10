
/* File: w_edit.c */

/************************************************/
/* This program is used to edit NIST SPHERE     */
/* headered files, creating a new file          */
/************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#define SPHERE_PRE_2_2
#include <sp/sphere.h>

char usage[] = "Usage:  %s [-v] -o [-[t|p]F:T]] [-c EXP] [-o OUT ] { filein | - } \
{ fileout | - }\n\
Where:  -tF:T	 Set the range for output from time F (seconds) to time T\n\
	-sF:T    Set the range for output from F samples to T samples\n\
\n\
		 - if F is missing, it defaults to the beginning of the \
file,\n\
		   if T is missing, go to the end of the file. \n\
\n\
	-cEXP    Extract only the samples from channels in EXP.  The \n\
		 expression will also add channels together if the '+'\n\
		 is used.\n\
\n\
	-oOUT	 Set the output format to the following formats:\n\
                        alaw\n\
	         	ulaw\n\
	         	pculaw\n\
			pcm_01 | short_01\n\
			pcm_10 | short_10\n\
			pcm | short_natural\n\
	-v	 give verbose output\n\n";
char * prog;

#define BUF_SAMPLES 4096

/* function prototypes */
int set_channel_conversions(char *, SP_INTEGER);

int main(int argc, char **argv)
{
    char *filein=CNULL, *fileout=CNULL;
    SP_FILE *sp_in=SPNULL, *sp_out=SPNULL;
    char *format_conversion="SE-ORIG:SBF-ORIG";
    char sdmod[100];
    char data_origins[200];
    char *channel_conversions="", *tstr;
    int in_place=0, sample_change=0;
    SP_INTEGER channel_count, in_sample_rate;
    double begin_time=0.0, end_time=(-1.0);
    SP_REAL spreal;
    long begin_sample=0, end_sample=(-1);
    int use_sample=1;
    char *read_buf=CNULL;
    int c, n1, exit_val, samples_read=0;
    SP_INTEGER in_snb, in_sample_count;
    SP_INTEGER out_sample_count;
    SP_STRING str;

    prog = strrchr( argv[0], '/' );
    prog = ( prog == CNULL ) ? argv[0] : ( prog + 1 );


    while (( c=hs_getopt( argc, argv, "vo:c:s:t:" )) != -1 )
	switch ( c ) {
	  case 'v':
	    sp_verbose++;
	    printf("verbose = %d\n",sp_verbose);
	    break;
	  case 'i':
	    in_place=1;
	    break;
	  case 't':
	  case 's':
	    sample_change = 1;
	    if (c == 't') use_sample=0;
	    tstr = strtok(hs_optarg,":");
	    if (*hs_optarg != ':'){
		if  (tstr != CNULL){
		    if (! strsame(tstr,"")){
			if (is_number(tstr)){
			    if (c == 't') {
				if ((begin_time = atof(tstr)) < 0){
				    fprintf(spfp,"Error: Beginning ");
				    fprintf(spfp,"time must be positive\n");
				    fprintf(spfp,usage,prog,prog);
				    goto FATAL_EXIT;
				}
			    } else {
				if ((begin_sample = atoi(tstr)) < 0){
				    fprintf(spfp,"Error: Beginning sample");
				    fprintf(spfp," must be positive\n");
				    fprintf(spfp,usage,prog,prog);
				    goto FATAL_EXIT;
				}
			    }
			} else {
			    fprintf(spfp,"Error: numeric value required ");
			    fprintf(spfp,"for '%c' option\n",c);
			    fprintf(spfp,usage,prog,prog);
			    goto FATAL_EXIT;
			}
		    }		    
		}
		tstr = strtok(CNULL,":");
	    }
	    if  (tstr != CNULL){
		if (! strsame(tstr,"")){
		    if (is_number(tstr)){
			if (c == 't'){
			    if ((end_time = atof(tstr)) < 0){
				fprintf(spfp,"Error: Ending time must ");
				fprintf(spfp,"be positive\n");
				fprintf(spfp,usage,prog,prog);
				goto FATAL_EXIT;
			    }
			} else {
			    if ((end_sample = atoi(tstr)) < 0){
				fprintf(spfp,"Error: Ending sample must ");
				fprintf(spfp,"be positive\n");
				fprintf(spfp,usage,prog,prog);
				goto FATAL_EXIT;
			    }
			}
		    } else {
			fprintf(spfp,"Error: numeric value required ");
			fprintf(spfp,"for '%c' option\n",c);
			fprintf(spfp,usage,prog,prog);
			goto FATAL_EXIT;
		    }
		}		    
	    }
	    if ((c == 't' && (begin_time > end_time) && (end_time != (-1.0)))||
		(c == 's' && (begin_sample > end_sample) && (end_sample != 
							     (-1)))){
		    fprintf(spfp,"Error: Beginning %s is after Ending %s.\n",
			    (c == 't') ? "time" : "sample",
			    (c == 't') ? "time" : "sample");
		    goto FATAL_EXIT;
		}
	    break;
	  case 'c':
	    channel_conversions = mtrf_strdup(hs_optarg);
	    break;
	  case 'o':
	    if (strsame(hs_optarg,"short_01") || strsame(hs_optarg,"pcm_01"))
		format_conversion = "SE-PCM:SBF-01";
	    else  if (strsame(hs_optarg,"short_10") ||
		      strsame(hs_optarg,"pcm_10"))
		format_conversion = "SE-PCM:SBF-10";
	    else  if (strsame(hs_optarg,"short_natural") || 
		      strsame(hs_optarg,"pcm"))
		format_conversion = "SE-PCM:SBF-N";
	    else  if (strsame(hs_optarg,"ulaw"))
		format_conversion = "SE-ULAW";
	    else  if (strsame(hs_optarg,"pculaw"))
		format_conversion = "SE-PCULAW";
	    else  if (strsame(hs_optarg,"alaw"))
		format_conversion = "SE-ALAW";
	    else{
		fprintf(spfp,"Error: unknown output type ");
		fprintf(spfp,"option '%s'\n",hs_optarg);
		fprintf(spfp,usage,prog,prog);
		goto FATAL_EXIT;
	    }
	    break;
	  default:
	    fprintf(spfp,"Error: unknown flag -%c\n",c);
	    fprintf(spfp,usage,prog,prog);
	    goto FATAL_EXIT;
	}

    if (sp_verbose > 0) fprintf(spfp,"%s: %s\n",prog,sp_get_version());

    if (argc - hs_optind != 2){
	fprintf(spfp,"Error: Requires 2 filename arguements\n");
	fprintf(spfp,usage,prog,prog);
	goto FATAL_EXIT;
    }

    filein=argv[hs_optind];
    fileout=argv[hs_optind+1];

    strcpy(sdmod,format_conversion);
    if (! strsame(channel_conversions,"")){
	strcat(sdmod,":CH-");
	strcat(sdmod,channel_conversions);
    }

    if ((sp_in=sp_open(filein,"r")) == SPNULL){
	fprintf(spfp,"%s: Unable to open file '%s'\n",prog,
		(strsame(filein,"-") ? "stdin" : filein ));
	sp_print_return_status(spfp);
	goto FATAL_EXIT;
    }

    /* get the channel count from the input file */
    if (sp_h_get_field(sp_in,"channel_count",T_INTEGER,
		       (void *)&channel_count) > 0){
	if (sp_get_return_status() != 104){
	    fprintf(spfp,"Error: Unable to get the '%s' ","channel_count");
	    fprintf(spfp,"field from file '%s'\n",
		    (strsame(filein,"-") ? "stdin" : filein ));
	    sp_print_return_status(spfp);
	    goto FATAL_EXIT;
	}
    }
    if (sp_h_get_field(sp_in,"sample_n_bytes",T_INTEGER,(void *)&in_snb) > 0){
	if (sp_get_return_status() != 104){
	    fprintf(spfp,"Error: Unable to get the '%s' ","sample_n_bytes");
	    fprintf(spfp,"field from file '%s'\n",
		    (strsame(filein,"-") ? "stdin" : filein ));
	    sp_print_return_status(spfp);
	    goto FATAL_EXIT;
	}
    }
    if (sp_h_get_field(sp_in,"sample_rate",T_INTEGER,
		       (void *)&in_sample_rate) > 0){
	if (sp_get_return_status() != 104){
	    fprintf(spfp,"Error: Unable to get the '%s' ","sample_rate");
	    fprintf(spfp,"field from file '%s'\n",
		    (strsame(filein,"-") ? "stdin" : filein ));
	    sp_print_return_status(spfp);
	    goto FATAL_EXIT;
	}
    }
    if (sp_h_get_field(sp_in,"sample_count",T_INTEGER,
		       (void *)&in_sample_count) > 0){
	if (sp_get_return_status() != 104){
	    fprintf(spfp,"Error: Unable to get the '%s'","sample_count");
	    fprintf(spfp," field from file '%s'\n",
		    (strsame(filein,"-") ? "stdin" : filein ));
	    sp_print_return_status(spfp);
	    goto FATAL_EXIT;
	}
    }

    /* if times were used on the command line, convert them to samples */
    if (use_sample) {
	/* printf("Using samples from %d:%d\n",begin_sample,end_sample); */ 
	begin_time = (double)begin_sample / (double)in_sample_rate;
	end_time   = (double)end_sample   / (double)in_sample_rate;
    } else {
	/* printf("Converting times from %5.2f:%5.2f  to  ",
	   begin_time,end_time); */
	if (begin_time > 0.0)
	    begin_sample = in_sample_rate * begin_time;
	if (end_time > 0.0)
	    end_sample = in_sample_rate * end_time;
	else
	    end_sample = (-1);
	/* printf("samples from %d:%d\n",begin_sample,end_sample); */
    }
    if (end_sample == (-1)){
	out_sample_count = (SP_INTEGER) (in_sample_count - begin_sample);
	end_sample = in_sample_count;
	end_time   = (double)end_sample   / (double)in_sample_rate;
    } else
	out_sample_count = (SP_INTEGER) (end_sample - begin_sample);

    /* create the output file */
    if ((sp_out=sp_open(fileout,"w")) == SPNULL){
	fprintf(spfp,"%s: Unable to open output file '%s'\n",prog,
		(strsame(fileout,"-") ? "stdout" : fileout ));
	sp_print_return_status(spfp);
	goto FATAL_EXIT;
    }
    /* duplicate the input sphere file's header */
    if (sp_copy_header(sp_in,sp_out) != 0){
	fprintf(spfp,"%s: Unable to duplicate the input file header\n",prog);
	sp_print_return_status(spfp);
	goto FATAL_EXIT;
    }

    if (sample_change){
	/* Add start_time and end_time to the header fields */
	spreal = (SP_REAL) begin_time;
	if (sp_h_set_field(sp_out,"start_time",T_REAL,(void *)&spreal) != 0){
	    fprintf(spfp,"Warning: sp_h_set_field failed on 'start_time'\n");
	    sp_print_return_status(spfp);
	}    
	spreal = (SP_REAL) end_time;
	if (sp_h_set_field(sp_out,"end_time",T_REAL,(void *)&spreal) != 0){
	    fprintf(spfp,"Warning: sp_h_set_field failed on 'end_time'\n");
	    sp_print_return_status(spfp);
	}    
	
	/* set the 'data_origins' field */
	*data_origins = '\0';
	if (sp_h_get_field(sp_out,"database_id",T_STRING,(void *)&str) == 0){
	    strcat(data_origins,str);
	    free(str);
	}
	if (sp_h_get_field(sp_out,"database_version",
			   T_STRING,(void *)&str) == 0){
	    if (*data_origins != '\0')
		strcat(data_origins,",");
	    strcat(data_origins,str);
	    free(str);
	}
	if (sp_h_get_field(sp_out,"utterance_id",T_STRING,(void *)&str) == 0)
	    ;
	else if (sp_h_get_field(sp_out,
				"conversation_id",T_STRING,(void *)&str)==0)
	    ;
	else  /* default to the filename */
	    str = mtrf_strdup(filein);
	if (*data_origins != '\0')
	    strcat(data_origins,",");
	strcat(data_origins,str);
	free(str);
	
	/* Set the new 'data_origins' field */
	if (sp_h_set_field(sp_out,"data_origins",
			   T_STRING,(void *)&data_origins) != 0){
	    fprintf(spfp,"Warning: sp_h_set_field failed on 'data_origins'\n");
	    sp_print_return_status(spfp);
	}    
    }

    /* convert the file using the format_conversion field */
    if (sp_set_data_mode(sp_out,sdmod) != 0){
	fprintf(spfp,"%s: Unable to set data mode to '%s' on file '%s'\n",
		prog,sdmod,(strsame(fileout,"-") ? "stdout" : fileout ));
	sp_print_return_status(spfp);
	goto FATAL_EXIT;
    }

    /* delete the sample_checksum of the times have changed */
    if ((begin_sample != 0) || (end_sample != in_sample_count))
	if (sp_h_delete_field(sp_out,SAMPLE_CHECKSUM_FIELD) > 100){
	    fprintf(spfp,"%s: Unable to delete sample checksum field",prog);
	    sp_print_return_status(spfp);
	    goto FATAL_EXIT;
	}

    /* allocate memory for the reading buffer */
    if ((read_buf=(char *)sp_data_alloc(sp_in,BUF_SAMPLES)) == CNULL){
	fprintf(spfp,"%s: Unable to allocate read buffer\n",prog);
	sp_print_return_status(spfp);
	goto FATAL_EXIT;
    }


    if (sp_seek(sp_in,begin_sample,0) != 0){
	fprintf(spfp,"Error: sp_seek() failed\n");
	sp_print_return_status(spfp);
	goto FATAL_EXIT;
    }
    samples_read = begin_sample;

    /* begin processing the file */
    do {
	int samp_to_get;
	if (samples_read + BUF_SAMPLES < end_sample)
	    samp_to_get = BUF_SAMPLES;
	else
	    samp_to_get = end_sample - samples_read;
	if (samp_to_get == 0){
	    break;
	}
	
	n1 = sp_read_data((char *)read_buf,in_snb,samp_to_get,sp_in);
	if (n1 == 0 && sp_error(sp_in) != 0) {
	    fprintf(spfp,"%s: sp_error() returned Non-Zero code\n",prog);
	    sp_print_return_status(spfp);
	    goto FATAL_EXIT;
	}
	samples_read += n1;
	if (sp_write_data((char *)read_buf,in_snb,n1,sp_out) != n1){
	    fprintf(spfp,"%s: Unable to write block to output file\n",prog);
	    sp_print_return_status(spfp);
	    goto FATAL_EXIT;
	}
    } while (n1 > 0);
    /* everything went OK */
    exit_val = 0;
    goto CLEAN_UP;

  FATAL_EXIT:
    exit_val = 1;

  CLEAN_UP:

    if (channel_conversions == CNULL) mtrf_free(channel_conversions);
    if (sp_in != SPNULL) {
	if (read_buf != CNULL)
	    sp_data_free(sp_in,read_buf);
	if (sp_close(sp_in) != 0){
	    fprintf(spfp,"%s: sp_close() failed on input file '%s'.\n",
		    prog,filein);
	    exit_val = 1;
	}
    }
    if (sp_out != SPNULL)
	if (sp_close(sp_out) != 0){
	    fprintf(spfp,"%s: sp_close() failed on output file '%s'.\n",
		    prog,fileout);
	    exit_val = 1;
	}
    
    if (exit_val == 1)
	if ((fileout != CNULL) && ! strsame(fileout,"-")) unlink(fileout);

    exit(exit_val);
}

