#include <stdio.h>
#include <math.h>
#include <string.h>
#define SPHERE_PRE_2_2 
#include <sp/sphere.h>
#include <util/hsgetopt.h>
#include <util/memory.h>

#if defined(NARCH_SUN) || defined(NARCH_HP)	/* cth */
#include <sys/types.h>
#include <netinet/in.h>
#endif

#define EX1_10_BASE           "ex1_10.wav"
#define EX1_10_CORRUPT_BASE   "ex1_10.wvc"
#define EX1_01_BASE           "ex1_01.wav"
#define EX1_10_WAVPACK_BASE   "ex1_10wp.wav"
#define EX1_10_SHORTEN_BASE   "ex1_10st.wav"
#define EX1_10_SHORTEN_CORRUPT_BASE   "ex1_10st.wvc"
#define EX1_10_SHORTPACK_BASE "ex1_10sh.wav"
#define EX1_01_WAVPACK_BASE   "ex1_01wp.wav"
#define EX1_01_SHORTEN_BASE   "ex1_01st.wav"
#define EX1_01_SHORTPACK_BASE "ex1_01sh.wav"

#define EX2_10_BASE           "ex2_10.wav"
#define EX2_10_BASE           "ex2_10.wav"
#define EX2_01_BASE           "ex2_01.wav"
#define EX2_10_WAVPACK_BASE   "ex2_10wp.wav"
#define EX2_10_SHORTEN_BASE   "ex2_10st.wav"
#define EX2_01_WAVPACK_BASE   "ex2_01wp.wav"
#define EX2_01_SHORTEN_BASE   "ex2_01st.wav"

#define EX4_ULAW_BASE           "ex4.wav"
#define EX4_ULAW_WAVPACK_BASE   "ex4_wp.wav"
#define EX4_ULAW_SHORTEN_BASE   "ex4_sh.wav"
#define EX4_ULAW_10_BASE        "ex4_10.wav"
#define EX4_ULAW_01_BASE        "ex4_01.wav"

#define EX5_ULAW_2CHAN_BASE             "ex5.wav"
#define EX5_ULAW_2CHAN_PCM_BASE         "ex5_p.wav"
#define EX5_ULAW_2CHAN_PCM_01_BASE      "ex5_p01.wav"
#define EX5_ULAW_2CHAN_PCM_SHORTEN_BASE "ex5_p_sh.wav"
#define EX5_ULAW_2CHAN_PCM_WAVPACK_BASE "ex5_p_wp.wav"
#define EX5_ULAW_2CHAN_WAVPACK_BASE     "ex5_wp.wav"
#define EX5_ULAW_2CHAN_SHORTEN_BASE     "ex5_sh.wav"
#define EX5_ULAW_CHAN1_BASE             "ex5_c1.wav"
#define EX5_ULAW_CHAN1_BITREV_BASE      "ex5_c1br.wav"
#define EX5_ULAW_CHAN1_PCM_BASE         "ex5_c1_p.wav"
#define EX5_ULAW_CHAN2_BASE             "ex5_c2.wav"
#define EX5_ULAW_CHAN2_BITREV_BASE      "ex5_c2br.wav"
#define EX5_ULAW_CHAN2_PCM_BASE         "ex5_c2_p.wav"
#define EX5_ULAW_CHAN12_BASE            "ex5_12.wav"
#define EX5_ULAW_CHAN12_PCM_BASE        "ex5_12_p.wav"

#define EX6_BASE                        "ex6.wav"

#define EX7_PCULAW_BASE           "ex7.wav"
#define EX7_PCULAW_WAVPACK_BASE   "ex7_wp.wav"
#define EX7_PCULAW_SHORTEN_BASE   "ex7_sh.wav"
#define EX7_PCULAW_10_BASE        "ex7_10.wav"
#define EX7_PCULAW_01_BASE        "ex7_01.wav"

char EX1_10[256], EX1_10_CORRUPT[256], EX1_01[256], EX1_10_WAVPACK[256];
char EX1_10_SHORTEN_CORRUPT[256];
char EX1_10_SHORTEN[256], EX1_01_WAVPACK[256], EX1_01_SHORTEN[256];
char EX1_10_SHORTPACK[256], EX1_01_SHORTPACK[256];

char EX2_10[256], EX2_01[256], EX2_10_WAVPACK[256];
char EX2_10_SHORTEN[256], EX2_01_WAVPACK[256], EX2_01_SHORTEN[256];

char EX4_ULAW[256], EX4_ULAW_10[256], EX4_ULAW_01[256];
char EX4_ULAW_SHORTEN[256], EX4_ULAW_WAVPACK[256];

char EX5_2CHAN[256], EX5_2CHAN_SHORTEN[256],  EX5_2CHAN_WAVPACK[256];
char EX5_CHAN1[256], EX5_CHAN2[256], EX5_CHAN1_BITREV[256];
char EX5_CHAN2_BITREV[256];
char EX5_2CHAN_PCM[256], EX5_CHAN1_PCM[256], EX5_CHAN2_PCM[256];
char EX5_2CHAN_PCM_SHORTEN[256], EX5_2CHAN_PCM_WAVPACK[256];
char EX5_CHAN12[256], EX5_CHAN12_PCM[256], EX5_2CHAN_PCM_01[256];

char EX6[256];

char EX7_PCULAW[256], EX7_PCULAW_10[256], EX7_PCULAW_01[256];
char EX7_PCULAW_SHORTEN[256], EX7_PCULAW_WAVPACK[256];

/* function prototypes */
void header_test(int);
void doc_example_2_test(int);
void doc_example_4_test(int);
void ulaw_test(int);
void pculaw_test(int);
void converted_read_check(char *, char *, char *);
void two_channel_test(int);
int  perform_2_channel_write_test(char *, char *, char *, char *);
int  perform_2_channel_read_test(char *, char *, char *, char *);
void large_file_test(int);
void do_large_file_conversion(char *, char *, char *);
void make_test_file(char *, int, char *);
void write_required_field_test(int);
void write_check_adding_fields(char *, char *);
void write_check_adding_fields_test(int);
void update_test(int);
void header_update(char *, int);
void waveform_update(char *, char *, char *);
void checksum_pre_post_verification(int);
void pre_read_check(char *, int);
void post_write_check(char *);
void print_usage(void);
void build_example_file_names(char *);
void mult_channel_raw_data_test(int);
void selective_channel_test(int);
int do_selective_read_test(char *, char *, char *, char *, int);
void array_access_tests(int);
int write_with_array_access(char *, char *, char *, int);
int verify_checksum_computations(int);
void rewind_tests(int);
void seek_test(int);
void perform_seek_test(char *);
void rewind_file_compare(char *, char *, char *, char *, char *);
void rewind_file_multi_channel(char *,char *,char *,char *);
void sp_compute_checksum_test(int);
void do_sp_compute_checksum_test(char *, int, int, int);
int do_sp_tell_check(SP_FILE *sp, char *file);

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#define COPY "cp"

void print_usage(void) {
    fprintf(spfp,
  "Usage: tsphere -[vmMeh] [-l iterations] [ -[bf] n ]  -d sphere_lib_dir\n");
}

int warning = 0;


int main(int argc, char **argv)
{
    char *sphere_lib_dir=".";
    int c,l,loop=1,start_test=0,final_test=100;
    char *prog;
    
    prog = strrchr(argv[0],'/');
    prog = (prog == CNULL) ? argv[0] : (prog + 1);

    while (( c=hs_getopt( argc, argv, "d:l:b:f:hvmMe" )) != -1 )
	switch ( c ) {
	  case 'd':
	    sphere_lib_dir = hs_optarg;
	    break;
	  case 'v':
	    sp_verbose ++;
	    break;
	  case 'l':
	    loop = atoi(hs_optarg);
	    break;
	  case 'b':
	    start_test = atoi(hs_optarg);
	    break;
	  case 'f':
	    final_test = atoi(hs_optarg);
	    break;
	  case 'm':
	    mtrf_set_dealloc(0);
	    mtrf_set_verbose(1);
	    break;
	  case 'M':
 	    mtrf_set_dealloc(1);
	    mtrf_set_verbose(1);
	    break;
	  case 'e':	    
	    set_error_util_debug(1);
	    break;
	  case 'h':
	    print_usage();
	    exit(0);
	  default:
	    print_usage();
	    fprintf(spfp,"Illegal argument: %c\n",c);
	    exit(100);
	}

    fprintf(spfp,"%s: %s\n\n",prog,sp_get_version());

    if (hs_optind != argc){
	print_usage();
	exit(100);
    }
    hs_resetopt();
    build_example_file_names(sphere_lib_dir);

    for (l=0; l!=loop; l++){
	if (loop != 1)
	    fprintf(spfp,
		  "*************   Beginning Iteration %d   ************\n\n",
		  l+1);
	
	if (start_test <= 1 && final_test >= 1)  header_test(1);  
	if (start_test <= 2 && final_test >= 2)
	    checksum_pre_post_verification(2);
	if (start_test <= 3 && final_test >= 3)  write_required_field_test(3);
	if (start_test <= 4 && final_test >= 4)  
	    write_check_adding_fields_test(4);
	if (start_test <= 5 && final_test >= 5)
	    verify_checksum_computations(5);
	if (start_test <= 6 && final_test >= 6)  update_test(6);
	if (start_test <= 7 && final_test >= 7)  large_file_test(7); 
	if (start_test <= 8 && final_test >= 8)  ulaw_test(8);
	if (start_test <= 9 && final_test >= 9)  two_channel_test(9); 
	if (start_test <= 10 && final_test >= 10)
	    mult_channel_raw_data_test(10);
	if (start_test <= 11 && final_test >= 11)  doc_example_2_test(11);
	if (start_test <= 12 && final_test >= 12)  doc_example_4_test(12);
	if (start_test <= 13 && final_test >= 13) selective_channel_test(13);
	if (start_test <= 14 && final_test >= 14) array_access_tests(14); 
	if (start_test <= 15 && final_test >= 15) rewind_tests(15); 
	if (start_test <= 16 && final_test >= 16) pculaw_test(16); 
	if (start_test <= 17 && final_test >= 17) seek_test(17); 
	if (start_test <= 18 && final_test >= 18) sp_compute_checksum_test(18); 

#ifdef SUN
	if (malloc_verify() == 0){
  	    fprintf(spfp,"******  all tests completed BUT THE MEMORY HEAP");
	    fprintf(spfp,"IS TRASHED  ******\n");
	}
#endif 
   }  

    if (warning > 0)
	fprintf(spfp,
		"*******  There were %d non-fatal warnings *******\n",warning);
    fprintf(spfp,"*******  ALL TESTS SUCCESSFULLY COMPLETED *******\n");
    exit(0);
}

void sp_compute_checksum_test(int test)
{
    fprintf(spfp,"Test %2d: sp_compute_checksum() testing:\n",test);
    fprintf(spfp,"-- Test on files w/o previous checksums:\n");
    do_sp_compute_checksum_test(EX4_ULAW,0,0,0);
    do_sp_compute_checksum_test(EX5_CHAN12_PCM,0,0,0);

    fprintf(spfp,"-- Test on files with previous checksums:\n");
    do_sp_compute_checksum_test(EX1_10,1,0,0);
    do_sp_compute_checksum_test(EX1_10_SHORTEN,1,0,0);

    fprintf(spfp,"-- Test on corrupt files:\n");
    do_sp_compute_checksum_test(EX1_10_SHORTEN_CORRUPT,1,1,0);
    do_sp_compute_checksum_test(EX1_10_CORRUPT,1,1,0);

    fprintf(spfp,"-- Test on pre-seek'd files:\n");
    do_sp_compute_checksum_test(EX1_10,1,0,1);
    do_sp_compute_checksum_test(EX1_10_SHORTEN,1,0,1);

    fprintf(spfp,"\n");
}

void do_sp_compute_checksum_test(char *file, int with_checksum, int corrupt, int test_seek){
    SP_FILE *sp;
    SP_INTEGER fchksum;
    SP_CHECKSUM cchksum;    
    int ret;
    
    fprintf(spfp,"---- Testing sp_compute_checksum() on file '%s'\n",file);
    
    if ((sp=sp_open(file,"r")) == SPNULL) {
	fprintf(spfp,"Error: sp_open of file '%s' failed.\n",file);
	goto FATAL_EXIT;
    }
    if (sp_h_get_field(sp,"sample_checksum",
		       T_INTEGER,(void *)&fchksum)!=0){
	if (with_checksum){
	    fprintf(spfp,"Error: Can't get checksum from file '%s'\n",file);
	    goto FATAL_EXIT;
	}
    }    
    if (test_seek){
	if (sp_seek(sp,1034,0) != 0){
	    fprintf(spfp,"Error: attempt to perform an initial seek failed");
	    goto FATAL_EXIT;
	}
    }
    ret = sp_compute_checksum(sp, &cchksum);
    if (!corrupt){
	if (cchksum == 0 && ret > 0){
	    sp_print_return_status(spfp);
	    goto FATAL_EXIT;
	}
	if (with_checksum && (fchksum != cchksum)){
	    fprintf(spfp,"Error: Computed checksum %d != header %d\n",
		    (int)cchksum,(int)fchksum);
	    goto FATAL_EXIT;
	}
    } else {
	if (ret == 0){
	    fprintf(spfp,"Error: successfull execution on corrupt file\n");
	    goto FATAL_EXIT;
	}
    }

    if (test_seek){
	if (sp_tell(sp) != 1034){
	    fprintf(spfp,"File was not returned to it's seek'd position of"
		    " %d, but rather %d\n",1034,sp_tell(sp));
	    goto FATAL_EXIT;
	}
    }

    if (sp_close(sp) != 0){
	fprintf(spfp,"Error: sp_close failed\n");
	goto FATAL_EXIT;
    }
    return;

  FATAL_EXIT:
    if (sp != SPNULL) sp_close(sp);
    exit(1);
}


void seek_test(int test)
{
    fprintf(spfp,"Test %2d: Waveform sp_seek() and sp_tell() tests:\n",test);
    perform_seek_test(EX1_01);
    perform_seek_test(EX1_01_SHORTEN);
    perform_seek_test(EX6);

    fprintf(spfp,"\n");
}

void perform_seek_test(char *file){
    SP_FILE *sp;
    void *orig_data = (void *)0, *seek_data = (void *)0;
    int origin, offset;
    SP_INTEGER nsamp, nchan, nsnb;
    int loc, size, i, n1;

    /* 1. open and read in a file.
       2. close the file
       3. re-open the file, randomly seeking through the 
       file verifying data as you go .
       */ 
    fprintf(spfp,"-- Seek testing on file '%s'.\n",file);
    if (sp_load_file(file, ""/*SDM*/, &nsamp, &nchan, &nsnb,
		     &orig_data) != 0){
	fprintf(spfp,"   sp_load_file for file '%s' failed\n",file);
	goto FATAL_QUIT;	    
    }
    
    if ((sp=sp_open(file,"r")) == SPNULL) {
	fprintf(spfp,"   sp_open of file '%s' failed.\n",file);
	goto FATAL_QUIT;	    
    }
    
    if ((seek_data = (void *)sp_data_alloc(sp,-1))== (void *)0){
	fprintf(spfp,"   seek data allocation failed\n");
	goto FATAL_QUIT;	    
    }
    
    for (origin = 0; origin < 3; origin ++){
	fprintf(spfp,"---- Origin: %d, Seek to: ",origin);
	/* start seeking through the data, comparing the data */
	for (i=0; i<5; i++){
	    size = (nsamp / 5);
	    if (size < 100) size = 100;
	    loc = ((i+1) * (int)(nsamp * 0.6)) % nsamp;
	    /* check the location, and make sure we don't read after EOF */
	    if (loc > (nsamp - 100)) loc = 100;
	    if (loc + size > nsamp) size = nsamp - loc;
	    
	    fprintf(spfp,"%d[%d] ",(int)loc,(int)size);
	    switch (origin) {
	      case 0: offset = loc;
		break;
	      case 1: offset = loc - sp->read_spifr->waveform->samples_read;
		break;
	      case 2: offset = loc - sp->read_spifr->status->file_sample_count;
		break;
	      default:
		goto FATAL_QUIT;
	    }
	    if (sp_seek(sp,offset,origin) != 0){
		fprintf(spfp,"\nsp_seek() failed.\n");
		goto FATAL_QUIT;
	    }
	    
	    n1 = sp_read_data(seek_data,nsnb,size,sp);
	    if (n1 != size){
		fprintf(spfp,"\nsp_read_data failed.\n");
		goto FATAL_QUIT;
	    }
	    if (memcmp((char *)seek_data,((char *)orig_data) + nsnb*nchan*loc,
		       size*nchan*nsnb)){
		fprintf(spfp,"\nError: byte-comparison failed\n");
		goto FATAL_EXIT;
	    }
	    if (do_sp_tell_check(sp,file) != 0)
		goto FATAL_EXIT;
	}
	fprintf(spfp,"\n");
    }
    goto CLEAN_UP;
    
  FATAL_QUIT:
    sp_print_return_status(spfp);
  FATAL_EXIT:
    exit (1);
    
  CLEAN_UP:
    if (sp != SPNULL) {
	if (seek_data != (void *)0) sp_data_free(sp,seek_data);
	sp_close(sp);
    }
    if (orig_data != (void *)0) mtrf_free(orig_data);
    return;
}

/* do the sp_tell check */
int do_sp_tell_check(SP_FILE *sp, char *file){
    int file_byte_loc, sptell_byte_loc, file_samples;
    SPIFR *spifr;

    if (sp->open_mode == SP_mode_read)
	spifr = sp->read_spifr;
    else if (sp->open_mode == SP_mode_write){
	spifr = sp->write_spifr;
	if (fob_fflush(spifr->waveform->sp_fob) != 0){
	    fprintf(spfp,"Error: do_sp_tell_check unable to flush the buffer"
		    "for file '%s'.",file);
	    return(1);
	}
    } else {
	fprintf(spfp,"Error: do_sp_tell_check called on file '%s' opened"
		" for something other that read or write",file);
	return(1);
    }
	    
    file_byte_loc = fob_ftell(spifr->waveform->sp_fob);
    if ((! fob_is_fp(spifr->waveform->sp_fob)) ||
	(fob_is_fp(spifr->waveform->sp_fob) && spifr->status->is_temp_file)) 
	file_byte_loc += spifr->waveform->header_data_size;

    file_samples = 
	(fob_ftell(spifr->waveform->sp_fob) -
	 ((fob_is_fp(spifr->waveform->sp_fob) && 
	   !(spifr->status->is_temp_file)) ?
	  spifr->waveform->header_data_size : 0)) / 
	 (spifr->status->file_sample_n_bytes * 
	  spifr->status->file_channel_count);
    
    if ((sptell_byte_loc = sp_tell(sp)) < 0){
	fprintf(spfp,"Error: sp_tell() failed\n");
	sp_print_return_status(spfp);
	return(1);
    }
    sptell_byte_loc = 
	(sptell_byte_loc * 
	 spifr->status->file_sample_n_bytes * 
	 spifr->status->file_channel_count) + 
	     spifr->waveform->header_data_size;

    if (file_byte_loc != sptell_byte_loc){
	fprintf(spfp,"\nError: sp_tell on '%s' failed to return a correct ",
		file);
	fprintf(spfp,"value. %d != expected %d\n",sp_tell(sp),file_samples);
	return(1);
    }
    return(0);
}

void rewind_tests(int test)
{
    fprintf(spfp,"Test %2d: Rewind waveform tests:\n",test);
       
    fprintf(spfp,"---- Rewind without data_mode modifications\n");
    rewind_file_compare(EX1_10,EX1_10,"","","");
    rewind_file_compare(EX1_10,EX1_01,"","","");
    rewind_file_compare(EX1_01,EX1_10,"","","");
    rewind_file_compare(EX5_2CHAN,EX5_2CHAN,"","","");
    rewind_file_compare(EX5_2CHAN,EX5_2CHAN,"","","");
    rewind_file_compare(EX5_2CHAN_SHORTEN,EX5_2CHAN,"","","");
    rewind_file_compare(EX5_2CHAN_WAVPACK,EX5_2CHAN,"","","");
    rewind_file_compare(EX1_10_SHORTEN,EX1_10,"","","");
    rewind_file_compare(EX1_10_WAVPACK,EX1_10,"","","");
    rewind_file_compare(EX1_10_SHORTPACK,EX1_10,"","","");

    fprintf(spfp,"---- Rewind with data_mode modifications\n");
    rewind_file_compare(EX5_2CHAN,EX5_CHAN12,"CH-1+2","","");
    if (get_natural_sbf(2) == SP_sbf_10){
	rewind_file_compare(EX1_01,EX1_10,"SBF-10","","");
	rewind_file_compare(EX1_01_SHORTEN,EX1_10,"SBF-10","","");
    } else {
	rewind_file_compare(EX1_10,EX1_10,"SBF-01","","");
	rewind_file_compare(EX1_10_SHORTEN,EX1_10,"SBF-01","","");
    }
    fprintf(spfp,"---- Rewind with 2 data_mode modifications\n");
    if (get_natural_sbf(2) == SP_sbf_10){
	rewind_file_compare(EX1_01,EX1_10,"SBF-10",EX4_ULAW,"SE-ULAW");
	rewind_file_compare(EX1_01,EX4_ULAW,"SE-ULAW",EX1_10,"SBF-10");
	rewind_file_compare(EX1_01_SHORTEN,EX4_ULAW,"SE-ULAW",EX1_10,"SBF-10");
    } else {
	rewind_file_compare(EX1_10,EX1_10,"SBF-01",EX4_ULAW,"SE-ULAW");
	rewind_file_compare(EX1_10,EX4_ULAW,"SE-ULAW",EX1_10,"SBF-01");
	rewind_file_compare(EX1_10_SHORTEN,EX4_ULAW,"SE-ULAW",EX1_10,"SBF-01");
    }
    rewind_file_compare(EX5_2CHAN,EX5_CHAN12,"CH-1+2",EX5_2CHAN,
			"SE-ORIG:CH-1,2");
    rewind_file_compare(EX5_2CHAN_SHORTEN,EX5_CHAN12,"CH-1+2",EX5_2CHAN,
			"SE-ULAW:CH-1,2");
    rewind_file_compare(EX5_2CHAN,EX5_2CHAN,"",EX5_CHAN12,"CH-1+2");

    fprintf(spfp,"---- Rewind of multi channel file\n");
    rewind_file_multi_channel(EX5_2CHAN,"",EX5_CHAN1,EX5_CHAN2); 
    rewind_file_multi_channel(EX5_2CHAN_PCM,"",EX5_CHAN1_PCM,EX5_CHAN2_PCM);
    rewind_file_multi_channel(EX5_2CHAN,"SE-PCM:",EX5_CHAN1_PCM,
			      EX5_CHAN2_PCM); 
    rewind_file_multi_channel(EX5_2CHAN_SHORTEN,"SE-PCM:",EX5_CHAN1_PCM,
			      EX5_CHAN2_PCM); 
    rewind_file_multi_channel(EX5_2CHAN_WAVPACK,"SE-PCM:",EX5_CHAN1_PCM,
			      EX5_CHAN2_PCM); 
    fprintf(spfp,"\n");
}

void rewind_file_multi_channel(char *file1, char *conv, char *file2, char *file3)
{
    SP_FILE *sp1, *sp2, *sp3, *sp4;
    char dm[50];
    
    fprintf(spfp,"------ Comparing file '%s' converted by '%s'\n",file1,conv);
    fprintf(spfp,"-------- Chan1 '%s' and Chan2 '%s':\n",file2,file3);
    
    if ((sp1=sp_open(file1,"r")) == SPNULL) {
	fprintf(spfp,"   sp_open: spopen failed on file '%s'\n",file1);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    if ((sp4=sp_open(file1,"r")) == SPNULL) {
	fprintf(spfp,"   sp_open: spopen failed on file '%s'\n",file1);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    if ((sp2=sp_open(file2,"r")) == SPNULL) {
	fprintf(spfp,"   spopen failed on file '%s'\n",file2);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    sprintf(dm,"%sCH-1",conv);
    if (sp_set_data_mode(sp1,dm) != 0){
	fprintf(spfp,"Error: sp_set_data_mode to ");
	fprintf(spfp,"'CH-1' failed on file '%s'\n",file1);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }

    if (diff_SP_FILE_waveforms(sp1, sp2, file1, file2, TRUE, stderr) > 0){
	fprintf(spfp,"Error: Waveforms differ on channel 1 compare\n");
	goto FATAL_QUIT;
    }
    if ((sp3=sp_open(file3,"r")) == SPNULL) {
	fprintf(spfp,"   spopen failed on file '%s'\n",file3);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    if (sp_rewind(sp1) > 0){
	fprintf(spfp,"   sp_rewind failed on file '%s'\n",file1);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    sprintf(dm,"%sCH-2",conv);
    if (sp_set_data_mode(sp1,dm) != 0){
	fprintf(spfp,"Error: sp_set_data_mode to ");
	fprintf(spfp,"'CH-2' failed on '%s'\n",file1);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }

    if (diff_SP_FILE_waveforms(sp1, sp3, file1, file3, TRUE, stderr) > 0){
	fprintf(spfp,
		"Error: Waveforms differ on channel 2 compare after rewind\n");
	goto FATAL_QUIT;
    }


    if (sp_rewind(sp1) > 0){
	fprintf(spfp,"   sp_rewind failed on file '%s'\n",file1);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    sprintf(dm,"CH-1,2");
    if (sp_set_data_mode(sp1,dm) != 0){
	fprintf(spfp,"Error: sp_set_data_mode to '%s' failed on '%s'\n",
		dm,file1);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    if (diff_SP_FILE_waveforms(sp1, sp4, file1, file1, TRUE, stderr) > 0){
	fprintf(spfp,"Error: Waveform differs when compared to itself\n");
	goto FATAL_QUIT;
    }

    goto CLEAN_UP;

  FATAL_QUIT:
    exit(1);

  CLEAN_UP:
    if (sp1 != SPNULL) sp_close(sp1);
    if (sp2 != SPNULL) sp_close(sp2);
    if (sp3 != SPNULL) sp_close(sp3);
    if (sp4 != SPNULL) sp_close(sp4);
    return;
}

void rewind_file_compare(char *file1, char *file2, char *conv2, char *file3, char *conv3)
{
    SP_FILE *sp1, *sp2;
    char *cfile, *cconv;
    int limit = 1 + ((strcmp(file3,"") == 0) ? 0 : 1), i;

    fprintf(spfp,"------ Comparing file '%s' (converted by '%s') to "
		"'%s':\n",file1,conv2,file2);

    /* open file 1 only once */
    if ((sp1=sp_open(file1,"r")) == SPNULL) {
	fprintf(spfp,"   sp_open: spopen failed on file '%s'\n",file1);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    for (i=0; i<limit; i++){
        if (i == 1)
	    fprintf(spfp,"------       AND file '%s' (converted by '%s') to "
		    "'%s':\n",file1,conv3,file3);
	switch (i){
	  case 0:
	    cfile = file2; cconv = conv2; break;
	  case 1: 
	    cfile = file3; cconv = conv3; break;
	  default:
	    fprintf(spfp,"Error: undefined limit\n");
	    goto FATAL_QUIT;
	}
	if ((sp2=sp_open(cfile,"r")) == SPNULL) {
	    fprintf(spfp,"   spopen failed on file '%s'\n",cfile);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}
	if (sp_set_data_mode(sp1,cconv) != 0){
	    fprintf(spfp,"Error: first sp_set_data_mode to ");
	    fprintf(spfp,"'%s' failed on file '%s'\n",cconv,file1);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}
	if (diff_SP_FILE_waveforms(sp1, sp2, file1,cfile,TRUE,stderr) > 0){
	    fprintf(spfp,"Error: Waveforms differ on initial compare\n");
	    goto FATAL_QUIT;
	}
	sp_close(sp2); sp2 = SPNULL;
	if ((sp2=sp_open(cfile,"r")) == SPNULL) {
	    fprintf(spfp,"   spopen failed on file '%s'\n",cfile);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}
	if (sp_rewind(sp1) > 0){
	    fprintf(spfp,"   first sp_rewind failed on file '%s'\n",file1);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}
	if (sp_set_data_mode(sp1,cconv) != 0){
	    fprintf(spfp,"Error: second sp_set_data_mode to ");
	    fprintf(spfp,"'%s' failed on file '%s'\n",cconv,file1);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}
	if (diff_SP_FILE_waveforms(sp1, sp2, file1, cfile, TRUE, stderr) > 0){
	    fprintf(spfp,"Error: Waveforms differ on compare after rewind\n");
	    goto FATAL_QUIT;
	}
	sp_close(sp2); sp2 = SPNULL;
	if (sp_rewind(sp1) > 0){
	    fprintf(spfp,"   second sp_rewind failed on file '%s'\n",file1);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}
    }
    goto CLEAN_UP;

  FATAL_QUIT:
    exit(1);

  CLEAN_UP:
    if (sp1 != SPNULL) sp_close(sp1);
    if (sp2 != SPNULL) sp_close(sp2);
    return;
}

void build_example_file_names(char *sphere_lib_dir)
{
    sprintf(EX1_10, "%s/%s", sphere_lib_dir,EX1_10_BASE);
    sprintf(EX1_10_CORRUPT, "%s/%s", sphere_lib_dir,EX1_10_CORRUPT_BASE);
    sprintf(EX1_01, "%s/%s", sphere_lib_dir,EX1_01_BASE);
    sprintf(EX1_10_WAVPACK, "%s/%s", sphere_lib_dir, EX1_10_WAVPACK_BASE);
    sprintf(EX1_10_SHORTEN, "%s/%s", sphere_lib_dir, EX1_10_SHORTEN_BASE);
    sprintf(EX1_10_SHORTEN_CORRUPT, "%s/%s", sphere_lib_dir, EX1_10_SHORTEN_CORRUPT_BASE);
    sprintf(EX1_10_SHORTPACK, "%s/%s", sphere_lib_dir, EX1_10_SHORTPACK_BASE);
    sprintf(EX1_01_WAVPACK, "%s/%s", sphere_lib_dir, EX1_01_WAVPACK_BASE);
    sprintf(EX1_01_SHORTEN, "%s/%s", sphere_lib_dir, EX1_01_SHORTEN_BASE);
    sprintf(EX1_01_SHORTPACK, "%s/%s", sphere_lib_dir, EX1_01_SHORTPACK_BASE);

    sprintf(EX2_10, "%s/%s", sphere_lib_dir,EX2_10_BASE);
    sprintf(EX2_10_WAVPACK, "%s/%s", sphere_lib_dir, EX2_10_WAVPACK_BASE);
    sprintf(EX2_10_SHORTEN, "%s/%s", sphere_lib_dir, EX2_10_SHORTEN_BASE);
    sprintf(EX2_01, "%s/%s", sphere_lib_dir,EX2_01_BASE);
    sprintf(EX2_01_WAVPACK, "%s/%s", sphere_lib_dir, EX2_01_WAVPACK_BASE);
    sprintf(EX2_01_SHORTEN, "%s/%s", sphere_lib_dir, EX2_01_SHORTEN_BASE);

    sprintf(EX4_ULAW, "%s/%s", sphere_lib_dir,EX4_ULAW_BASE);
    sprintf(EX4_ULAW_WAVPACK, "%s/%s", sphere_lib_dir, EX4_ULAW_WAVPACK_BASE);
    sprintf(EX4_ULAW_SHORTEN, "%s/%s", sphere_lib_dir, EX4_ULAW_SHORTEN_BASE);
    sprintf(EX4_ULAW_10, "%s/%s", sphere_lib_dir,EX4_ULAW_10_BASE);
    sprintf(EX4_ULAW_01, "%s/%s", sphere_lib_dir,EX4_ULAW_01_BASE);

    sprintf(EX5_2CHAN, "%s/%s", sphere_lib_dir,EX5_ULAW_2CHAN_BASE);
    sprintf(EX5_2CHAN_PCM, "%s/%s", sphere_lib_dir, EX5_ULAW_2CHAN_PCM_BASE);
    sprintf(EX5_2CHAN_PCM_01, "%s/%s", sphere_lib_dir,
	    EX5_ULAW_2CHAN_PCM_01_BASE);
    sprintf(EX5_2CHAN_PCM_SHORTEN, "%s/%s", sphere_lib_dir,
	    EX5_ULAW_2CHAN_PCM_SHORTEN_BASE);
    sprintf(EX5_2CHAN_PCM_WAVPACK, "%s/%s", sphere_lib_dir,
	    EX5_ULAW_2CHAN_PCM_WAVPACK_BASE);
    sprintf(EX5_2CHAN_WAVPACK, "%s/%s", sphere_lib_dir,
	    EX5_ULAW_2CHAN_WAVPACK_BASE);
    sprintf(EX5_2CHAN_SHORTEN, "%s/%s", sphere_lib_dir,
	    EX5_ULAW_2CHAN_SHORTEN_BASE); 
    sprintf(EX5_CHAN1, "%s/%s", sphere_lib_dir,EX5_ULAW_CHAN1_BASE);
    sprintf(EX5_CHAN1_BITREV, "%s/%s", sphere_lib_dir,
	    EX5_ULAW_CHAN1_BITREV_BASE);
    sprintf(EX5_CHAN1_PCM, "%s/%s", sphere_lib_dir,
	    EX5_ULAW_CHAN1_PCM_BASE);
    sprintf(EX5_CHAN2, "%s/%s", sphere_lib_dir,EX5_ULAW_CHAN2_BASE);
    sprintf(EX5_CHAN2_BITREV, "%s/%s", sphere_lib_dir,
	    EX5_ULAW_CHAN2_BITREV_BASE);
    sprintf(EX5_CHAN2_PCM, "%s/%s", sphere_lib_dir,
	    EX5_ULAW_CHAN2_PCM_BASE);
    sprintf(EX5_CHAN12, "%s/%s", sphere_lib_dir,EX5_ULAW_CHAN12_BASE);
    sprintf(EX5_CHAN12_PCM, "%s/%s", sphere_lib_dir,
	    EX5_ULAW_CHAN12_PCM_BASE);

    sprintf(EX6, "%s/%s", sphere_lib_dir,EX6_BASE);

    sprintf(EX7_PCULAW, "%s/%s", sphere_lib_dir,EX7_PCULAW_BASE);
    sprintf(EX7_PCULAW_WAVPACK, "%s/%s", sphere_lib_dir, EX7_PCULAW_WAVPACK_BASE);
    sprintf(EX7_PCULAW_SHORTEN, "%s/%s", sphere_lib_dir, EX7_PCULAW_SHORTEN_BASE);
    sprintf(EX7_PCULAW_10, "%s/%s", sphere_lib_dir,EX7_PCULAW_10_BASE);
    sprintf(EX7_PCULAW_01, "%s/%s", sphere_lib_dir,EX7_PCULAW_01_BASE);
}

int verify_checksum_computations(int test){
    char *proc="check_checksum " SPHERE_VERSION_STR;
    SP_CHECKSUM comp_chksm, comp_chksm1, sum_chksm;
    static SP_CHECKSUM ex_arr[100] = { 0, 17, 51, 102, 170, 255, 357, 476,
	612, 765, 935, 1122, 1326, 1547, 1785, 2040, 2312, 2601, 2907,
	3230, 3570, 3927, 4301, 4692, 5100, 5525, 5967, 6426, 6902,
	7395, 7905, 8432, 8976, 9537, 10115, 10710, 11322, 11951,
	12597, 13260, 13940, 14637, 15351, 16082, 16830, 17595, 18377,
	19176, 19992, 20825, 21675, 22542, 23426, 24327, 25245, 26180,
	27132, 28101, 29087, 30090, 31110, 32147, 33201, 34272, 35360,
	36465, 37587, 38726, 39882, 41055, 42245, 43452, 44676, 45917,
	47175, 48450, 49742, 51051, 52377, 53720, 55080, 56457, 57851,
	59262, 60690, 62135, 63597, 65076, 1036, 2549, 4079, 5626,
	7190, 8771, 10369, 11984, 13616, 15265, 16931, 18614};
    static SP_CHECKSUM sum_arr[100] = { 17, 68, 153, 272, 425, 612, 833,
	1088, 1377, 1700, 2057, 2448, 2873, 3332, 3825, 4352, 4913,
	5508, 6137, 6800, 7497, 8228, 8993, 9792, 10625, 11492, 12393,
	13328, 14297, 15300, 16337, 17408, 18513, 19652, 20825, 22032,
	23273, 24548, 25857, 27200, 28577, 29988, 31433, 32912, 34425,
	35972, 37553, 39168, 40817, 42500, 44217, 45968, 47753, 49572,
	51425, 53312, 55233, 57188, 59177, 61200, 63257, 65348, 1937,
	4096, 6289, 8516, 10777, 13072, 15401, 17764, 20161, 22592,
	25057, 27556, 30089, 32656, 35257, 37892, 40561, 43264, 46001,
	48772, 51577, 54416, 57289, 60196, 63137, 576, 3585, 6628,
	9705, 12816, 15961, 19140, 22353, 25600, 28881, 32196, 35545};
    short sh_arr[100];
    int i, failure=FALSE;

    fprintf(spfp,"Test %2d: Waveform checksum calculation tests:\n",test);

    if (sizeof(long) <= 2)
	fprintf(spfp,"Proc %s: Long int is only %ld bytes, >2 needed\n",
		proc,sizeof(long));
    if (sizeof(short) != 2)
	fprintf(spfp,"Proc %s: short int is only %ld bytes, != 2\n",
		proc,sizeof(short));

    for (i=0; i<100; i++)
	sh_arr[i] = i*17;
       
    for (i=0; i<100; i++){
	comp_chksm = sp_compute_short_checksum(sh_arr, i+1, FALSE);
	if (comp_chksm != ex_arr[i]){
	    fprintf(spfp,"Error: Computed checksum %d != Expected[%d] %d\n",
		    comp_chksm,i,ex_arr[i]);
	    failure=TRUE;
	}
    }
    for (i=0; i<99; i++){
	comp_chksm = sp_compute_short_checksum(sh_arr, i+1, FALSE);
	comp_chksm1 = sp_compute_short_checksum(sh_arr, i+2, FALSE);
	sum_chksm = sp_add_checksum(comp_chksm,comp_chksm1);
	if (sum_chksm != sum_arr[i]){
	    fprintf(spfp,"Error: Summed checksum %d -> %d+%d=%d != %d\n",
		    i,comp_chksm,comp_chksm1,sum_chksm,sum_arr[i]);
	    failure=TRUE;
	}
    }
    fprintf(spfp,"\n");
    return(0);
}

void array_access_tests(int test){
    SP_FILE *sp_inter=SPNULL, *sp_array=SPNULL;
    SP_INTEGER nchan, snb;
    void *inter_buf=(void *)0, **array_buf=(void **)0;
    char conversion[100], array_conversion[100];
    int frame_size=3000, ret, ret2, return_value;
    int s,c,fmt;

    fprintf(spfp,"Test %2d: Array access to waveform tests:\n",test);
    /* change the input data formats */
    /*     read in a 4-channel mux-ed file as interleaved data */
    /*     read in a 4-channel mux-ed file as an array         */
    /*     if that passes, */
    /*         change the data formats */
    /*            re-read the file as an array, writing it as an array */
    /*            do a data diff on the original and the new           */


    for (fmt=0; fmt < 6; fmt++){
	
	switch (fmt){
	  case 0: 
	    fprintf(spfp,"---- %d: No format changes\n",fmt+1);
	    strcpy(conversion,"");
	    sprintf(array_conversion,"DF-ARRAY");
	    break;
	  case 1:
	    fprintf(spfp,"---- %d: Changing to ULAW\n",fmt+1);
	    sprintf(conversion,"SE-ULAW:SBF-1");
	    sprintf(array_conversion,"SE-ULAW:SBF-1:DF-ARRAY");
	    break;
	  case 2:
	    fprintf(spfp,"---- %d: Changing to Channels 1,2\n",fmt+1);
	    sprintf(conversion,"CH-1,2");
	    sprintf(array_conversion,"CH-1,2:DF-ARRAY");
	    break;
	  case 3:
	    fprintf(spfp,"---- %d: Changing to Channels 2,1\n",fmt+1);
	    sprintf(conversion,"CH-2,1");
	    sprintf(array_conversion,"CH-2,1:DF-ARRAY");
	    break;
	  case 4:
	    fprintf(spfp,"---- %d: Changing to ULAW, Channels 1,2\n",fmt+1);
	    sprintf(conversion,"CH-1,2:SE-ULAW:SBF-1");
	    sprintf(array_conversion,"CH-1,2:SE-ULAW:SBF-1:DF-ARRAY");
	    break;
	  case 5:
	    fprintf(spfp,"---- %d: Changing to ULAW, Channels 2,1\n",fmt+1);
	    sprintf(conversion,"CH-2,1:SE-ULAW:SBF-1");
	    sprintf(array_conversion,"CH-2,1:SE-ULAW:SBF-1:DF-ARRAY");
	    break;
	}
	/* open both files */
	if ((sp_inter=sp_open(EX6,"r")) == SPNULL) {
	    fprintf(spfp,"   sp_open: spopen for interleaved read of ");
	    fprintf(spfp,"file '%s' failed\n",EX6);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}
	if ((sp_array=sp_open(EX6,"r")) == SPNULL) {
	    fprintf(spfp,"   sp_open: spopen for array read of ");
	    fprintf(spfp,"file '%s' failed\n",EX6);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	    
	}

	if (sp_set_data_mode(sp_inter,conversion) != 0){
	    fprintf(spfp,"Error: sp_set_data_mode to ");
	    fprintf(spfp,"'%s' failed interleaved file '%s'\n",conversion,EX6);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}
	if (sp_h_get_field(sp_inter,CHANNEL_COUNT_FIELD,
			   T_INTEGER,(void *)&nchan)!=0){
	    fprintf(spfp,"   Can't get the channel count from file '%s'\n",
		    EX6);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}    
	if (sp_h_get_field(sp_inter,CHANNEL_COUNT_FIELD,
			   T_INTEGER,(void *)&snb)!=0){
	    fprintf(spfp,"   Can't get the sample_n_bytes from file '%s'\n",
		    EX6);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}    

	/* convert the spfile to be read as an array */
	if (sp_set_data_mode(sp_array,array_conversion) != 0){
	    fprintf(spfp,"Error: sp_set_data_mode to ");
	    fprintf(spfp,"'%s' failed on file '%s'\n",array_conversion,EX6);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}

	/* alloc buffers for each file */
	if ((inter_buf = (void *)sp_data_alloc(sp_inter,frame_size))==
	    (void *)0){
	    fprintf(spfp,"Unable to allocate interleaved memory for ");
	    fprintf(spfp,"file '%s'\n",EX6);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}
	if ((array_buf = (void **)sp_data_alloc(sp_array,frame_size))==
	    (void **)0){
	    fprintf(spfp,"Unable to allocate array memory for ");
	    fprintf(spfp,"file '%s'\n",EX6);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}
	do {
	    ret=sp_read_data(inter_buf,2,frame_size,sp_inter);
	    if ((ret == 0) && (sp_error(sp_inter) > 0)){
		fprintf(spfp,"   Read failed on interleaved input file\n");
		sp_print_return_status(spfp);
		goto FATAL_QUIT;
	    }
	    if (ret > 0){
		ret2=sp_read_data(array_buf,2,ret,sp_array);
		if ((ret2 == 0) && (sp_error(sp_array) > 0)){
		    fprintf(spfp,"   Read failed on array input file\n");
		    sp_print_return_status(spfp);
		    goto FATAL_QUIT;
		}
		if (ret != ret2){
		    fprintf(spfp,"   Read %d samples from inteleaved file",
			    ret);
		    fprintf(spfp,", but only %d from array file",ret);
		    goto FATAL_QUIT;
		}
		{   unsigned char **carr=(unsigned char **)array_buf,
			          *cintr=(unsigned char *)inter_buf;
		    short **sarr = (short **)array_buf,
		          *sintr = (short *)inter_buf;
		    int val;
		    for (s=0; s<ret; s++){
			for (c=0; c<nchan; c++){
			    if (fmt == 0 || fmt == 2 || fmt == 3)
				val = (*(sintr + s*nchan +c) != sarr[c][s]);
			    else
				val = (*(cintr + s*nchan +c) != carr[c][s]);
			    if (val){
				fprintf(spfp,
 				    "Sample Values != at samp %d, chan %d\n",
					s,c);
				goto FATAL_QUIT;
			    }
			}
		    }
		}
	    }
	} while (ret > 0);
	if (sp_error(sp_inter) > 0 ){
	    fprintf(spfp,"Read failure on interleaved file %s\n",EX6);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}
	if (sp_error(sp_array) > 0 ){
	    fprintf(spfp,"Read failure on array file %s\n",EX6);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}
	if (sp_inter != SPNULL) {
	    if (inter_buf != (void *)0) sp_data_free(sp_inter,inter_buf);
	    if (sp_close(sp_inter) != 0){
		fprintf(spfp,"Error: Unable to close stereo file '%s'\n",EX6);
		sp_print_return_status(spfp);
		return_value = 200;
	    }
	    sp_inter = SPNULL; inter_buf = (void *)0;
	}
	if (sp_array != SPNULL) {
	    if (array_buf != (void **)0) sp_data_free(sp_array,array_buf);
	    if (sp_close(sp_array) != 0){
		fprintf(spfp,"Error: Unable to close stereo file '%s'\n",EX6);
		sp_print_return_status(spfp);
		return_value = 200;
	    }
	    sp_array = SPNULL; array_buf = (void **)0;
	}

	write_with_array_access(EX6, conversion, array_conversion, fmt);

	/* everything is ok */
	return_value = 0;
	goto CLEAN_UP;

      FATAL_QUIT:  /* Failed routine */
	return_value = 100;
	
      CLEAN_UP:
	if (sp_inter != SPNULL) {
	    if (inter_buf != (void *)0) sp_data_free(sp_inter,inter_buf);
	    if (sp_close(sp_inter) != 0){
		fprintf(spfp,"Error: Unable to close stereo file '%s'\n",EX6);
		sp_print_return_status(spfp);
		return_value = 200;
	    }
	    sp_inter = SPNULL; inter_buf = (void *)0;
	}
	if (sp_array != SPNULL) {
	    if (array_buf != (void **)0) sp_data_free(sp_array,array_buf);
	    if (sp_close(sp_array) != 0){
		fprintf(spfp,"Error: Unable to close stereo file '%s'\n",EX6);
		sp_print_return_status(spfp);
		return_value = 200;
	    }
	    sp_array = SPNULL; array_buf = (void **)0;
	}
	if (return_value != 0)
	    exit(-1);
    }
    fprintf(spfp,"\n");
}

int write_with_array_access(char *filein, char *conversion,
			    char *array_conversion, int fmt)
{
    SP_FILE *sp_inter=SPNULL, *sp_new=SPNULL;
    SP_INTEGER nchan;
    char *fileout="output.wav";
    int frame_size=3000, ret, ret2, c, s, return_value;
    void *inter_buf=(void *)0, **new_array_buf=(void **)0;

    fprintf(spfp,"------ Writing test into format '%s'\n",
	    array_conversion);
    
    /**** re open the input file, un-altered, writting it to the ****/
    /**** new file  ****/
    if ((sp_inter=sp_open(filein,"r")) == SPNULL) {
	fprintf(spfp,"   sp_open: spopen for interleaved read of ");
	fprintf(spfp,"file '%s' failed\n",filein);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    if ((sp_new=sp_open(fileout,"w")) == SPNULL) {
	fprintf(spfp,"   sp_open: spopen for array write of ");
	fprintf(spfp,"file '%s' failed\n",fileout);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    if (sp_copy_header(sp_inter, sp_new) > 0){
	fprintf(spfp,"Couldn't duplicate the header in file ");
	fprintf(spfp,"'%s' for file '%s'\n",filein,fileout);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    if (sp_set_data_mode(sp_new,array_conversion) != 0){
	fprintf(spfp,"Error: sp_set_data_mode to ");
	fprintf(spfp,"'%s' failed on file '%s'\n",array_conversion,
		fileout);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }

    if (sp_h_get_field(sp_inter,CHANNEL_COUNT_FIELD,
		       T_INTEGER,(void *)&nchan)!=0){
	fprintf(spfp,"   Can't get the channel count from file '%s'\n",
		filein);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }    

    /* alloc buffers for each file */
    if ((inter_buf = (void *)sp_data_alloc(sp_inter,frame_size))==
	(void *)0){
	fprintf(spfp,"Unable to allocate interleaved memory for ");
	fprintf(spfp,"file '%s'\n",filein);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    if ((new_array_buf = (void **)sp_data_alloc(sp_new,frame_size))==
	(void **)0){
	fprintf(spfp,"Unable to allocate array memory for new ");
	fprintf(spfp,"file '%s'\n",fileout);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }

    do {
	short *sintr = (short *)inter_buf;
	short **nsarr = (short **)new_array_buf;
	
	ret=sp_read_data(inter_buf,2,frame_size,sp_inter);
	if ((ret == 0) && (sp_error(sp_inter) > 0)){
	    fprintf(spfp,"   Read failed on interleaved input file\n");
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}
	if (ret > 0) {
	    /* copy the data into the output array data */
	    for (s=0; s<ret; s++)
		for (c=0; c<nchan; c++)
		    nsarr[c][s] = (*(sintr + s*nchan +c));
	    
	    if ((ret2=sp_write_data(new_array_buf,2,ret,sp_new)) != ret){
		fprintf(spfp,"Unable to write %d array samples to file\n",
			ret);
		sp_print_return_status(spfp);
		goto FATAL_QUIT;
	    }
	}
    } while (!sp_eof(sp_inter));
    if (sp_new != SPNULL) {
	if (new_array_buf != (void **)0) sp_data_free(sp_new,
						      new_array_buf);
	new_array_buf = (void **)0;
	if (sp_close(sp_new) != 0){
	    fprintf(spfp,"Error: Unable to close stereo file '%s'\n",
		    fileout);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}
	sp_new = SPNULL;
    }
    
    
    if (diff_waveforms(filein,fileout,conversion,"", 0, spfp) != 0){
	fprintf(spfp,"Write of Array Accessed passed, but diff failed\n");
	diff_waveforms(filein,fileout,conversion,"", 1, spfp);
	goto FATAL_QUIT;
    }
    
    /* everything is ok */
    return_value = 0;
    unlink(fileout);
    goto CLEAN_UP;
    
  FATAL_QUIT:  /* Failed routine */
    return_value = 100;
    
  CLEAN_UP:
    if (sp_inter != SPNULL) {
	if (inter_buf != (void *)0) sp_data_free(sp_inter,inter_buf);
	if (sp_close(sp_inter) != 0){
	    fprintf(spfp,"Error: Unable to close stereo file '%s'\n",filein);
	    sp_print_return_status(spfp);
	    return_value = 200;
	}
	sp_inter = SPNULL; inter_buf = (void *)0;
    }
    if (sp_new != SPNULL) {
	if (new_array_buf != (void **)0) sp_data_free(sp_new,
						      new_array_buf);
	new_array_buf = (void **)0;
	if (sp_close(sp_new) != 0){
	    fprintf(spfp,"Error: Unable to close stereo file '%s'\n",
		    fileout);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}
	sp_new = SPNULL;
    }
    if (return_value != 0)
	exit(-1);
    return(1);    
}

void selective_channel_test(int test){
    fprintf(spfp,"Test %2d: Selective Channel testing\n",test);

    fprintf(spfp,"---- Invalid read test\n");
    if (!do_selective_read_test(EX5_2CHAN,EX5_CHAN1,"CH-2","",0)){
	do_selective_read_test(EX5_2CHAN,EX5_CHAN1,"CH-2","",1);
	fprintf(spfp,"Error: Expecting an error, but none was detected\n");
	exit(-1);
    }

    fprintf(spfp,"---- Read selective channels\n");
    if (do_selective_read_test(EX5_2CHAN,EX5_CHAN1, "CH-1", "",1))
	exit(-1);
    if (do_selective_read_test(EX5_2CHAN,EX5_CHAN2, "CH-2", "",1))
	exit(-1);
    if (do_selective_read_test(EX5_2CHAN_PCM,EX5_CHAN1_PCM, "CH-1", "",1))
	exit(-1);
    if (do_selective_read_test(EX5_2CHAN_PCM,EX5_CHAN2_PCM,"CH-2", "",1))
	exit(-1);
    fprintf(spfp,"---- Read selective channels converting sample encodings\n");
    if (do_selective_read_test(EX5_2CHAN,EX5_CHAN1_PCM,
			       "SE-PCM:SBF-N:CH-1", "",1))
	exit(-1);
    if (do_selective_read_test(EX5_2CHAN,EX5_CHAN2_PCM,
			       "SE-PCM:SBF-N:CH-2", "",1))
	exit(-1);

    fprintf(spfp,"---- Read channels added together\n");
    if (do_selective_read_test(EX5_2CHAN_PCM,EX5_CHAN12_PCM,"CH-1+2", "",1))
	exit(-1);
    if (do_selective_read_test(EX5_2CHAN,EX5_CHAN12, "CH-1+2","",1))
	exit(-1);
    if (do_selective_read_test(EX5_2CHAN_PCM_01,EX5_CHAN12_PCM, 
			       "SBF-01:CH-1+2", "SBF-01",1))
	exit(-1);
    fprintf(spfp,
	    "---- Read channels added together converting sample types\n");
    if (do_selective_read_test(EX5_2CHAN,EX5_CHAN12_PCM, 
			       "SE-PCM:SBF-01:CH-1+2", "SBF-01",1))
	exit(-1);
    fprintf(spfp,"\n");

    fprintf(spfp,
	    "---- Write channels added together changing sample encodings\n");
    waveform_update(EX5_2CHAN,"SE-PCM:SBF-10:CH-1+2",EX5_CHAN12_PCM);

    fprintf(spfp,"---- Write channels added together\n");
    waveform_update(EX5_2CHAN,"CH-1+2", EX5_CHAN12);
    waveform_update(EX5_2CHAN_PCM,"SE-PCM:SBF-10:CH-1+2", EX5_CHAN12_PCM);

    fprintf(spfp,"---- Write selective channels changing sample encodings\n");
    waveform_update(EX5_2CHAN,"SE-PCM:SBF-10:CH-1", EX5_CHAN1_PCM);
    waveform_update(EX5_2CHAN,"SE-PCM:SBF-10:CH-2", EX5_CHAN2_PCM);

    fprintf(spfp,"---- Write selective channels\n");
    waveform_update(EX5_2CHAN_PCM,"SE-PCM:SBF-10:CH-1", EX5_CHAN1_PCM);
    waveform_update(EX5_2CHAN_PCM,"SE-PCM:SBF-10:CH-2", EX5_CHAN2_PCM);
    waveform_update(EX5_2CHAN,"SE-ULAW:SBF-1:CH-1",EX5_CHAN1);
    waveform_update(EX5_2CHAN,"SE-ULAW:SBF-1:CH-2",EX5_CHAN2);
    fprintf(spfp,"\n");
}

int do_selective_read_test(char *f1, char *f2, char *conv1, char *conv2,int v){
    if (v) fprintf(spfp,"------ File: %s conv '%s' Compared to %s conv '%s'\n",
		  f1,conv1,f2,conv2);
    if (diff_waveforms(f1,f2,conv1,conv2, 0, spfp) != 0){
	if (v) diff_waveforms(f1,f2,conv1,conv2, 1, spfp);
	return(1);
    }
    return(0);
}

#define MC_CHAN 4
#define MC_BUFS 500
void mult_channel_raw_data_test(int test){
    SP_FILE *sp;
    SP_INTEGER bps, nc=MC_CHAN, sc=MC_BUFS, sr=8000, r_nc, r_bps, r_sc;
    int c,s,cc,failure,rtn;
    char *out_file="testing.wav";
    double factor;
    int ntypes=5, cur_type;
    union{ 
	short  **b_short; int **b_int; long **b_long; float **b_float;
	double **b_double;
    } buf;
    union{
	short  *b_short; int *b_int; long *b_long; float *b_float;
	double *b_double;
    } time_samp;
    char *types[5];
    types[0] = "short";
    types[1] = "int";
    types[2] = "long";
    types[3] = "float";
    types[4] = "double";

    fprintf(spfp,"Test %2d: Write of a multichannel data with varying types\n",test);

    for (cur_type=0; cur_type<ntypes; cur_type++){
	switch (cur_type){
	  case 0: bps=sizeof(short);	   break;
	  case 1: bps=sizeof(int);	   break;
	  case 2: bps=sizeof(long);	   break; 
	  case 3: bps=sizeof(float);	   break; 
	  case 4: bps=sizeof(double);	   break;
	}	  
	fprintf(spfp,"---- using type '%s', %ld bytes ",types[cur_type],bps);
	fprintf(spfp,"per sample,  %ld channels, %ld samples\n",nc,sc);
	
	/* set the byte size and allocate memory for the buffers */
	switch (cur_type){
	  case 0: alloc_2dimarr(buf.b_short,nc,sc,short);
	    alloc_singarr(time_samp.b_short,nc,short);
	    break;
	  case 1: alloc_2dimarr(buf.b_int,nc,sc,int);
	    alloc_singarr(time_samp.b_int,nc,int);
	    break;
	  case 2: alloc_2dimarr(buf.b_long,nc,sc,long);
	    alloc_singarr(time_samp.b_long,nc,long);	
	    break; 
	  case 3: alloc_2dimarr(buf.b_float,nc,sc,float);
	    alloc_singarr(time_samp.b_float,nc,float);
	    break; 
	  case 4: alloc_2dimarr(buf.b_double,nc,sc,double);
	    alloc_singarr(time_samp.b_double,nc,double);
	    break;
	}	  
	
	/* load the values in the buffer */
	for (c=0; c<MC_CHAN; c++){
	    factor = 1.0 / ((c+1) * 10.0 );
	    switch (cur_type){
	      case 0: for (s=0; s<MC_BUFS; s++)
		  buf.b_short[c][s] = 
		      (short)((c+1) * 10 * cos( M_PI * 2.0 * s * factor));
		break;
	      case 1: for (s=0; s<MC_BUFS; s++)
		  buf.b_int[c][s] =
		      (int)((c+1) * 10 * cos( M_PI * 2.0 * s * factor));
		break;
	      case 2: for (s=0; s<MC_BUFS; s++)
		  buf.b_long[c][s] = 
		      (long)((c+1) * 10 * cos( M_PI * 2.0 * s * factor));
		break;
	      case 3: for (s=0; s<MC_BUFS; s++)
		  buf.b_float[c][s] =
		      (float)((c+1) * 10 * cos( M_PI * 2.0 * s * factor));
		break;
	      case 4: for (s=0; s<MC_BUFS; s++)
		  buf.b_double[c][s] = 
		      (double)((c+1) * 10 * cos( M_PI * 2.0 * s * factor));
		break;
	    }
	}
	
	/* open a sphere file to write out to */
	if ((sp=sp_open(out_file,"w")) == SPNULL) {
	    fprintf(spfp,"   sp_open failed for file '%s' opened for write\n",
		   out_file);
	    sp_print_return_status(spfp);
	    exit(-1);
	}
	
	/* set up the header */
	/* set the channel count */
	if (sp_h_set_field(sp,CHANNEL_COUNT_FIELD,T_INTEGER,(void *)&nc) != 0){
	    fprintf(spfp,"    sp_h_set_fielcase d: failed on field '%s'\n",
		   CHANNEL_COUNT_FIELD);
	    sp_print_return_status(spfp);
	    exit(-1);
	}    
	/* set the sample_n_bytes */
	if (sp_h_set_field(sp,SAMPLE_N_BYTES_FIELD,T_INTEGER,(void *)&bps)!=0){
	    fprintf(spfp,"    sp_h_set_field: failed on field '%s'\n",
		   SAMPLE_N_BYTES_FIELD);
	    sp_print_return_status(spfp);
	    exit(-1);
	}    
	/* set the sample_count */
	if (sp_h_set_field(sp,SAMPLE_COUNT_FIELD,T_INTEGER,(void *)&sc) != 0){
	    fprintf(spfp,"    sp_h_set_field: failed on field '%s'\n",
		   SAMPLE_COUNT_FIELD);
	    sp_print_return_status(spfp);
	    exit(-1);
	}    
	/* set the sample_rate */
	if (sp_h_set_field(sp,SAMPLE_RATE_FIELD,T_INTEGER,(void *)&sr) != 0){
	    fprintf(spfp,"    sp_h_set_field: failed on field '%s'\n",
		   SAMPLE_RATE_FIELD);
	    sp_print_return_status(spfp);
	    exit(-1);
	}    
	/* set the sample_coding */
	if (sp_h_set_field(sp,SAMPLE_CODING_FIELD,T_STRING,(void *)"raw") !=0){
	    fprintf(spfp,"    sp_h_set_field: failed on field '%s'\n",
		   SAMPLE_CODING_FIELD);
	    sp_print_return_status(spfp);
	    exit(-1);
	}    
	
	/* write the data to the file, INTERLEAVED!!!!! */
	for (s=0; s<MC_BUFS; s++){
	    switch (cur_type){
	      case 0:
		for (c=0; c<MC_CHAN; c++)  
		    time_samp.b_short[c] = buf.b_short[c][s];
		rtn = sp_write_data((void *)time_samp.b_short,bps,1,sp);
		break;
	      case 1:
		for (c=0; c<MC_CHAN; c++)  
		    time_samp.b_int[c] = buf.b_int[c][s];
		rtn = sp_write_data((void *)time_samp.b_int,bps,1,sp);
		break;
	      case 2:
		for (c=0; c<MC_CHAN; c++)  
		    time_samp.b_long[c] = buf.b_long[c][s];
		rtn = sp_write_data((void *)time_samp.b_long,bps,1,sp);
		break;
	      case 3:
		for (c=0; c<MC_CHAN; c++)  
		    time_samp.b_float[c] = buf.b_float[c][s];
		rtn = sp_write_data((void *)time_samp.b_float,bps,1,sp);
		break;
	      case 4:
		for (c=0; c<MC_CHAN; c++)  
		    time_samp.b_double[c] = buf.b_double[c][s];
		rtn = sp_write_data((void *)time_samp.b_double,bps,1,sp);
		break;
	      default:
		rtn = 0;
	    }
	    if (rtn != 1){
		fprintf(spfp,
			"Error: Multichannel write failed on time sample %d\n",
			s);
		sp_print_return_status(spfp);
		exit(-1);
	    }    
	}	
	
	/* Close the output file */
	if (sp_close(sp) != 0){
	    fprintf(spfp,"Erro: Close of multichannel file failed\n");
	    sp_print_return_status(spfp);
	    exit(-1);
	}    
	
	/*** NOW Read the file back in, comparing it to the form in memory ***/
	if ((sp=sp_open(out_file,"rv")) == SPNULL) {
	    fprintf(spfp,"   sp_open failed for file '%s' opened for read\n",
		   out_file);
	    sp_print_return_status(spfp);
	    exit(-1);
	}
	
	/* get the channel count */
	if (sp_h_get_field(sp,CHANNEL_COUNT_FIELD,T_INTEGER,(void *)&r_nc)!=0){
	    fprintf(spfp,"    sp_h_get_field: failed on field '%s'\n",
		   CHANNEL_COUNT_FIELD);
	    sp_print_return_status(spfp);
	    exit(-1);
	}    
	/* get the sample_n_bytes */
	if (sp_h_get_field(sp,SAMPLE_N_BYTES_FIELD,T_INTEGER,(void *)&r_bps) 
	    != 0){
	    fprintf(spfp,"    sp_h_get_field: failed on field '%s'\n",
		   SAMPLE_N_BYTES_FIELD);
	    sp_print_return_status(spfp);
	    exit(-1);
	}    
	/* get the sample_count */
	if (sp_h_get_field(sp,SAMPLE_COUNT_FIELD,T_INTEGER,(void *)&r_sc)!= 0){
	    fprintf(spfp,"    sp_h_get_field: failed on field '%s'\n",
		   SAMPLE_COUNT_FIELD);
	    sp_print_return_status(spfp);
	    exit(-1);
	}    
	/* verify the header fields */
	if (nc != r_nc){
	    fprintf(spfp,"Error: read channel_count (%ld) != expected (%ld)\n",
		    r_nc,nc);
	    exit(-1);
	}
	if (bps != r_bps){
	    fprintf(spfp,"Error: read sample_n_bytes (%ld) != expected (%ld)\n"
		    ,r_nc,nc);
	    exit(-1);
	}
	if (sc != r_sc){
	    fprintf(spfp,"Error: read sample_count (%ld) != expected (%ld)\n",
		    r_nc,nc);
	    exit(-1);
	}
	if (0)
	    switch (cur_type){
	      case 0: buf.b_short[1][2] = 32; break;
	      case 1: buf.b_int[1][2] = 32; break;
	      case 2: buf.b_long[1][2] = 32; break;
	      case 3: buf.b_float[1][2] = 32; break;
	      case 4: buf.b_double[1][2] = 32; break;
	      default: fprintf(spfp,
			       "type %d not defined\n",cur_type); exit(-1);
	    }
	
	/* loop through reading the data, verifying it against the memory  */
	/* version */
	for (s=0; s<MC_BUFS; s++){
	    char *mem, *file;
	    
	    switch (cur_type){
	      case 0: file=(char *)(time_samp.b_short); break;
	      case 1: file=(char *)(time_samp.b_int); break;
	      case 2: file=(char *)(time_samp.b_long); break;
	      case 3: file=(char *)(time_samp.b_float); break;
	      case 4: file=(char *)(time_samp.b_double); break;
	      default: fprintf(spfp,
			       "type %d not defined\n",cur_type); exit(-1);
	    }
	    if (sp_read_data((void *)file,r_bps,1,sp) != 1){
		fprintf(spfp,
			"Error: Multichannel read failed on time sample %d\n",
			s);
		sp_print_return_status(spfp);
		exit(-1);
	    }    	
	    for (c=0, failure=0; c<MC_CHAN; c++){
		switch (cur_type){
		  case 0: mem = (char *)&(buf.b_short[c][s]);
		    file=(char *)&(time_samp.b_short[c]); break;
		  case 1: mem = (char *)&(buf.b_int[c][s]); 
		    file=(char *)&(time_samp.b_int[c]); break;
		  case 2: mem = (char *)&(buf.b_long[c][s]);
		    file=(char *)&(time_samp.b_long[c]); break;
		  case 3: mem = (char *)&(buf.b_float[c][s]); 
		    file=(char *)&(time_samp.b_float[c]); break;
		  case 4: mem = (char *)&(buf.b_double[c][s]);
		    file=(char *)&(time_samp.b_double[c]); break;
		  default: fprintf(spfp,
				   "type %d not defined\n",cur_type); exit(-1);
		}
		if (memcmp(mem,file,bps))
		    failure = 1;
	    }
	    
	    if (failure){
		fprintf(spfp,"Error Multichannel failed to match");
		fprintf(spfp,"memory version on sample %d\n",s);
		fprintf(spfp,"Memory: ");
		switch (cur_type){
		  case 0: file=(char *)(time_samp.b_short); break;
		  case 1: file=(char *)(time_samp.b_int); break;
		  case 2: file=(char *)(time_samp.b_long); break;
		  case 3: file=(char *)(time_samp.b_float); break;
		  case 4: file=(char *)(time_samp.b_double); break;
		  default: fprintf(spfp,
				   "type %d not defined\n",cur_type); exit(-1);
		}
		for (c=0; c<MC_CHAN; c++){
		    switch (cur_type){
		      case 0: mem = (char *)&(buf.b_short[c][s]); break;
		      case 1: mem = (char *)&(buf.b_int[c][s]); break;
		      case 2: mem = (char *)&(buf.b_long[c][s]); break;
		      case 3: mem = (char *)&(buf.b_float[c][s]); break;
		      case 4: mem = (char *)&(buf.b_double[c][s]); break;
		      default: fprintf(spfp,"type %d not defined\n",cur_type);
			exit(-1);
		    }
		    for (cc=0; cc<bps; cc++)
			fprintf(spfp," %2x",
				*((unsigned char *)( mem + cc )));
		    fprintf(spfp," |");
		}
		fprintf(spfp,"\nFile:   ");
		for (c=0; c<MC_CHAN; c++){
		    for (cc=0; cc<bps; cc++)
			fprintf(spfp," %2x",
				*((unsigned char *)( file + c*bps + cc )));
		    fprintf(spfp," |");
		}
		fprintf(spfp,"\n");
		/*	    exit(-1); */
	    }
	}	
	/* free the memory associated with the type */
	switch (cur_type){
	  case 0: free_2dimarr(buf.b_short,nc,short);
	    free_singarr(time_samp.b_short,short);
	    break;
	  case 1: free_2dimarr(buf.b_int,nc,int);
	    free_singarr(time_samp.b_int,int);
	    break;
	  case 2: free_2dimarr(buf.b_long,nc,long);
	    free_singarr(time_samp.b_long,long);	
	    break; 
	  case 3: free_2dimarr(buf.b_float,nc,float);
	    free_singarr(time_samp.b_float,float);
	    break; 
	  case 4: free_2dimarr(buf.b_double,nc,double);
	    free_singarr(time_samp.b_double,double);
	    break;
	}	  
	if (sp != SPNULL) sp_close(sp);
    }
    fprintf(spfp,"\n");
}


/*****************************************************************************/
void header_test(int test)
{
    SP_FILE *sp;
    int rtn;
    char *str;
    long lint;
    double real;

    fprintf(spfp,"Test %2d: Write Mode header operations:\n",test);
    system("rm -f testing.wav");
    if ((sp=sp_open("testing.wav","w")) == SPNULL) {
	fprintf(spfp,"   sp_open: Valid write open failed\n");
	sp_print_return_status(spfp);
	exit(-1);
    }

    fprintf(spfp,"---- Testing the file header io:\n");
    fprintf(spfp,"------ Field creation:\n");
    if (sp_h_set_field(SPNULL,"field1",T_STRING,(void *)"char string 1") == 0)
	fprintf(spfp,"    sp_h_set_field: Null SPFILE pointer failed\n");
    if (sp_h_set_field(sp,CNULL,T_STRING,(void *)"char string 1") == 0)
	fprintf(spfp,"    sp_h_set_field: Null field name failed\n");
    if (sp_h_set_field(sp,"field1",4930,(void *)"char string 1") == 0)
	fprintf(spfp,"    sp_h_set_field: Invalid field type failed\n");
    if (sp_h_set_field(sp,"field1",T_STRING,(void *)CNULL) == 0)
	fprintf(spfp,"    sp_h_set_field: Null value failed\n");

    if (sp_h_set_field(sp,"field1",T_STRING,(void *)"string value1") != 0){
	fprintf(spfp,"    sp_h_set_field: valid STRING command failed\n");
	sp_print_return_status(spfp);
    }

    lint=1;
    if (sp_h_set_field(sp,"field2",T_INTEGER,(void *)&lint) != 0){
	fprintf(spfp,"    sp_h_set_field: valid INTEGER command failed\n");
	sp_print_return_status(spfp);
	exit(-1);
    }
    real=2.0;
    if (sp_h_set_field(sp,"field3",T_REAL,(void *)&real) != 0){
	fprintf(spfp,"    sp_h_set_field: valid REAL command failed\n");
	sp_print_return_status(spfp);
    }

    fprintf(spfp,"------ Field access:\n");
    if (sp_h_get_field(sp,"field1",T_STRING,(void *)&str) != 0){
	fprintf(spfp,"    sp_h_get_field: valid STRING command failed\n");
	sp_print_return_status(spfp);
    }
    lint=1;
    free(str);
    if (sp_h_get_field(sp,"field2",T_INTEGER,(void *)&lint) != 0){
	fprintf(spfp,"    sp_h_get_field: valid INTEGER command failed\n");
	sp_print_return_status(spfp);
	exit(-1);
    }
    real=2.0;
    if ((rtn=sp_h_get_field(sp,"field3",T_REAL,(void *)&real)) != 0){
      fprintf(spfp,
	    "    sp_h_get_field: valid REAL command returned %d and failed\n",
	     rtn);
	sp_print_return_status(spfp);
    }
	   
    fprintf(spfp,"------ Illegal Field access:\n");
    if ((rtn=sp_h_get_field(SPNULL,"field3",T_REAL,(void *)&real)) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid REAL command, SPNULL, ");
	fprintf(spfp,"returned %d and failed\n",rtn);
	sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_get_field(sp,CNULL,T_REAL,(void *)&real)) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid REAL command, Field NULL, ");
	fprintf(spfp,"returned %d and failed\n",rtn);
        sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_get_field(sp,"field3",6,(void *)&real)) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid REAL command, Bad Type, ");
	fprintf(spfp,"returned %d and failed\n",rtn);
	sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_get_field(sp,"field3",T_REAL,(void *)CNULL)) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid REAL command, Null value ");
	fprintf(spfp,"pointer, returned %d and failed\n",rtn);
	sp_print_return_status(spfp);
    }

    if ((rtn=sp_h_get_field(sp,"field3",T_INTEGER,(void *)&real)) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid REAL command Accessed as ");
	fprintf(spfp,"an INTEGER returned %d and failed\n",rtn);
	sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_get_field(sp,"field3",T_STRING,(void *)&real)) < 100){
	fprintf(spfp,
		"    sp_h_get_field: Invalid REAL command Accessed as an ");
	fprintf(spfp,"STRING returned %d and failed\n",rtn);
	sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_get_field(sp,"field2",T_STRING,(void *)&lint)) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid INTEGER command accessed ");
	fprintf(spfp,"as a STRING failed\n");
	sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_get_field(sp,"field2",T_REAL,(void *)&lint)) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid INTEGER command accessed ");
	fprintf(spfp,"as a REAL failed\n");
	sp_print_return_status(spfp);
    }
    if (sp_h_get_field(sp,"field1",T_REAL,(void *)&str) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid STRING command accessed ");
	fprintf(spfp,"as a REAL failed\n");
	sp_print_return_status(spfp);
    }
    if (sp_h_get_field(sp,"field1",T_INTEGER,(void *)&str) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid STRING command accessed ");
	fprintf(spfp,"as a INTEGER failed\n");
	sp_print_return_status(spfp);
   }

    fprintf(spfp,"------ Verifying 3 header fields\n");
    if ((sp->write_spifr->header->fc != 3) ||
	(sp->write_spifr->status->file_header->fc != 3)){
	fprintf(spfp,"***************************************************\n");
	fprintf(spfp,"*  The following header should have three fields  *\n");
	fprintf(spfp,"***************************************************\n");
	sp_file_dump(sp,spfp);
	exit(-1);
    }

    fprintf(spfp,"------ Illegal Field Deletion:\n");
    if ((rtn=sp_h_delete_field(SPNULL,"field1")) < 100){
	fprintf(spfp,
		"    sp_h_delete_field: Invalid Deletion, SPNULL, failed\n");
	sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_delete_field(sp,CNULL)) < 100){
	fprintf(spfp,"    sp_h_delete_field: Invalid Deletion, ");
	fprintf(spfp,"Field name NULL, failed\n");
	sp_print_return_status(spfp);
    }
    if (((rtn=sp_h_delete_field(sp,"field84")) == 0) && (rtn >= 100)){
	fprintf(spfp,"    sp_h_delete_field: Invalid Deletion, ");
	fprintf(spfp,"field already deleted, failed\n");
	sp_print_return_status(spfp);
    }

    fprintf(spfp,"------ Legal Field Deletion:\n");
    if ((rtn=sp_h_delete_field(sp,"field1")) != 0){
	fprintf(spfp,"    sp_h_delete_field: Valid STRING Deletion failed\n");
	sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_delete_field(sp,"field2")) != 0){
	fprintf(spfp,"    sp_h_delete_field: Valid INTEGER Deletion failed\n");
	sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_delete_field(sp,"field3")) != 0){
	fprintf(spfp,"    sp_h_delete_field: Valid REAL Deletion failed\n");
	sp_print_return_status(spfp);
    }
    fprintf(spfp,"------ Verifying an empty header\n");
    if ((sp->write_spifr->header->fc != 0) ||
	(sp->write_spifr->status->file_header->fc != 0)){
	fprintf(spfp,"***************************************************\n");
	fprintf(spfp,"*      The following header should be empty       *\n");
	fprintf(spfp,"***************************************************\n");
	sp_file_dump(sp,spfp);
	exit(-1);
    }
    
    sp_close(sp);
    system("rm -f testing.wav");
    fprintf(spfp,"\n");
    
    fprintf(spfp,"-- Read Mode header operations:\n");
    if ((sp=sp_open(EX1_10,"r")) == SPNULL) {
	fprintf(spfp,"   sp_open: Valid spopen for read of ");
	fprintf(spfp,"file '%s' failed\n",EX1_10);
	sp_print_return_status(spfp);
	exit(-1);
    }

    fprintf(spfp,"---- Testing the file header io:\n");
    fprintf(spfp,"------ Field creation:\n");
    if (sp_h_set_field(SPNULL,"field1",T_STRING,(void *)"char string 1") == 0)
	fprintf(spfp,"    sp_h_set_field: Null SPFILE pointer failed\n");
    if (sp_h_set_field(sp,CNULL,T_STRING,(void *)"char string 1") == 0)
	fprintf(spfp,"    sp_h_set_field: Null field name failed\n");
    if (sp_h_set_field(sp,"field1",4930,(void *)"char string 1") == 0)
	fprintf(spfp,"    sp_h_set_field: Invalid field type failed\n");
    if (sp_h_set_field(sp,"field1",T_STRING,(void *)CNULL) == 0)
	fprintf(spfp,"    sp_h_set_field: Null value failed\n");

    if (sp_h_set_field(sp,"field1",T_STRING,(void *)"string value1") != 0){
	fprintf(spfp,"    sp_h_set_field: valid STRING command failed\n");
	sp_print_return_status(spfp);
    }
    lint=1;
    if (sp_h_set_field(sp,"field2",T_INTEGER,(void *)&lint) != 0){
	fprintf(spfp,"    sp_h_set_field: valid INTEGER command failed\n");
	sp_print_return_status(spfp);
	exit(-1);
    }
    real=2.0;
    if (sp_h_set_field(sp,"field3",T_REAL,(void *)&real) != 0){
	fprintf(spfp,"    sp_h_set_field: valid REAL command failed\n");
	sp_print_return_status(spfp);
    }

    fprintf(spfp,"------ Field access:\n");
    if (sp_h_get_field(sp,"field1",T_STRING,(void *)&str) != 0){
	fprintf(spfp,"    sp_h_get_field: valid STRING command failed\n");
	sp_print_return_status(spfp);
    }
    mtrf_free(str);
    lint=1;
    if (sp_h_get_field(sp,"field2",T_INTEGER,(void *)&lint) != 0){
	fprintf(spfp,"    sp_h_get_field: valid INTEGER command failed\n");
	sp_print_return_status(spfp);
	exit(-1);
    }
    real=2.0;
    if ((rtn=sp_h_get_field(sp,"field3",T_REAL,(void *)&real)) != 0){
	fprintf(spfp,"    sp_h_get_field: valid REAL command ");
	fprintf(spfp,"returned %d and failed\n",rtn);
	sp_print_return_status(spfp);
    }
	   
    fprintf(spfp,"------ Illegal Field access:\n");
    if ((rtn=sp_h_get_field(SPNULL,"field3",T_REAL,(void *)&real)) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid REAL command, SPNULL, ");
	fprintf(spfp,"returned %d and failed\n",rtn);
	sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_get_field(sp,CNULL,T_REAL,(void *)&real)) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid REAL command, Field NULL, ");
	fprintf(spfp,"returned %d and failed\n",rtn);
	sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_get_field(sp,"field3",6,(void *)&real)) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid REAL command, Bad Type, ");
	fprintf(spfp,"returned %d and failed\n",rtn);
	sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_get_field(sp,"field3",T_REAL,(void *)CNULL)) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid REAL command, ");
	fprintf(spfp,"Null value pointer, returned %d and failed\n",rtn);
	sp_print_return_status(spfp);
    }

    if ((rtn=sp_h_get_field(sp,"field3",T_INTEGER,(void *)&real)) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid REAL command Accessed ");
	fprintf(spfp,"as an INTEGER returned %d and failed\n",rtn);
	sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_get_field(sp,"field3",T_STRING,(void *)&real)) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid REAL command Accessed as ");
	fprintf(spfp,"an STRING returned %d and failed\n",rtn);
	sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_get_field(sp,"field2",T_STRING,(void *)&lint)) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid INTEGER command accessed ");
	fprintf(spfp,"as a STRING failed\n");
	sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_get_field(sp,"field2",T_REAL,(void *)&lint)) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid INTEGER command accessed ");
	fprintf(spfp,"as a REAL failed\n");
	sp_print_return_status(spfp);
    }
    if (sp_h_get_field(sp,"field1",T_REAL,(void *)&str) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid STRING command accessed ");
	fprintf(spfp,"as a REAL failed\n");
	sp_print_return_status(spfp);
    }
    if (sp_h_get_field(sp,"field1",T_INTEGER,(void *)&str) < 100){
	fprintf(spfp,"    sp_h_get_field: Invalid STRING command accessed ");
	fprintf(spfp,"as a INTEGER failed\n");
	sp_print_return_status(spfp);
    }

    fprintf(spfp,"------ Illegal Field Deletion:\n");
    if ((rtn=sp_h_delete_field(SPNULL,"field1")) < 100){
     fprintf(spfp,"    sp_h_delete_field: Invalid Deletion, SPNULL, failed\n");
	sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_delete_field(sp,CNULL)) < 100){
	fprintf(spfp,"    sp_h_delete_field: Invalid Deletion, ");
	fprintf(spfp,"Field name NULL, failed\n");
	sp_print_return_status(spfp);
    }
    if (((rtn=sp_h_delete_field(sp,"field84")) == 0) && (rtn >= 100)){
	fprintf(spfp,"    sp_h_delete_field: Invalid Deletion, ");
	fprintf(spfp,"field already deleted, failed\n");
	sp_print_return_status(spfp);
    }

    fprintf(spfp,"------ Legal Field Deletion:\n");
    if ((rtn=sp_h_delete_field(sp,"field1")) != 0){
	fprintf(spfp,"    sp_h_delete_field: Valid STRING Deletion failed\n");
	sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_delete_field(sp,"field2")) != 0){
	fprintf(spfp,"    sp_h_delete_field: Valid INTEGER Deletion failed\n");
	sp_print_return_status(spfp);
    }
    if ((rtn=sp_h_delete_field(sp,"field3")) != 0){
	fprintf(spfp,"    sp_h_delete_field: Valid REAL Deletion failed\n");
	sp_print_return_status(spfp);
    }
    fprintf(spfp,"\n");

    sp_close(sp);
}


/*****************************************************************************/
void doc_example_2_test(int test)
{
    SP_FILE *sp=SPNULL;
    short *waveform;
    SP_INTEGER channel_count, sample_n_bytes, sample_count;
    int wave_byte_size, total_samples, samp_read;

    fprintf(spfp,"Test %2d: Documentation Example 2\n",test);
    if ((sp = sp_open(EX4_ULAW,"r")) == SPNULL) {
        fprintf(spfp,"Error: Unable to open SPHERE file %s\n",EX4_ULAW);
	sp_print_return_status(spfp);
	exit(-1);
    }

    if (sp_set_data_mode(sp, "SE-PCM-2:SBF-01") > 0){
	sp_print_return_status(spfp);
	sp_close(sp);
	exit(-1);
    }
    if (sp_h_get_field(sp,"channel_count",T_INTEGER,(void *)&channel_count)>0){
       fprintf(spfp,"Error: Unable to get the '%s' field\n","channel_count");
	sp_close(sp);
	exit(-1);
    }
    if (sp_h_get_field(sp,"sample_n_bytes",
		       T_INTEGER,(void *)&sample_n_bytes) > 0){
	fprintf(spfp,"Error: Unable to get the '%s' field\n",
		"sample_n_bytes");
	sp_close(sp);
	exit(-1);
    }
    if (sp_h_get_field(sp,"sample_count",T_INTEGER,(void *)&sample_count) > 0){
	fprintf(spfp,"Error: Unable to get the '%s' field\n","sample_count");
	sp_close(sp);
	exit(-1);
    }

    total_samples=sample_count * channel_count;
    wave_byte_size=sample_n_bytes * total_samples;

    fprintf(spfp,"---- Example 2: Expected channel_count=1,      ");
    fprintf(spfp,"Actually=%ld\n",channel_count);
    fprintf(spfp,"---- Example 2: Expected sample_n_bytes=2,     ");
    fprintf(spfp,"Actually=%ld\n",sample_n_bytes);
    fprintf(spfp,"---- Example 2: Expected sample_count=16000,   ");
    fprintf(spfp,"Actually=%ld\n",sample_count);
    fprintf(spfp,"---- Example 2: Expected total_samples=16000,  ");
    fprintf(spfp,"Actually=%d\n",total_samples);
    fprintf(spfp,"---- Example 2: Expected wave_byte_size=32000, ");
    fprintf(spfp,"Actually=%d\n",wave_byte_size);

    if ((waveform=(short *)mtrf_malloc(wave_byte_size)) == (short *)0){
        fprintf(spfp,"Error: Unable to allocate %d bytes for the waveform\n",
                       wave_byte_size);
	sp_close(sp);
	exit(-1);
    }
    
    if ((samp_read=sp_read_data(waveform,sample_n_bytes,total_samples,sp)) !=
                     total_samples){
        fprintf(spfp,"Error: Unable to read speech waveform, ");
	fprintf(spfp,"%d samples read\n",samp_read);
	sp_print_return_status(spfp);
	sp_close(sp);
	mtrf_free((char *)waveform);
	exit(-1);
    }
    mtrf_free((char *)waveform);
    sp_close(sp);
    fprintf(spfp,"\n");
}

void doc_example_4_test(int test)
{
    SP_FILE *spp;
    short *wavbuf;
    SP_INTEGER lint;
    SP_INTEGER buf_nsamp = 16000;
    SP_INTEGER nbyte = 2;
    SP_INTEGER srate = 16000;
    int stow, i;
    int samps_written=0, size=64000, status;
    char *name="example4.wav";
    double factor;	

    fprintf(spfp,"Test %2d: Documentation Example 4: File Creation example\n",test);
    fprintf(spfp,"---- filename: %s\n",name);


    if ((spp = sp_open(name, "w")) == SPNULL){
	fprintf(spfp,"Couldn't open NIST waveform file: %s\n", name);
	sp_print_return_status(spfp); 	exit(-1);
    }

    fprintf(spfp,"---- Setting header fields\n");
    lint = size;
    if (sp_h_set_field(spp, "sample_count", T_INTEGER,(void *) &lint) != 0){
	fprintf(spfp,"Error: unable to set sample_count field\n");
	sp_print_return_status(spfp); 	exit(-1);
    }
    if (sp_h_set_field(spp, "sample_rate", T_INTEGER,(void *) &srate)){
	fprintf(spfp,"Error: unable to set sample_rate field\n");
	sp_print_return_status(spfp); 	exit(-1);
    }
    if (sp_h_set_field(spp, "sample_n_bytes", T_INTEGER, (void *) &nbyte)){
	fprintf(spfp,"Error: unable to set sample_n_bytes field\n");
	sp_print_return_status(spfp); 	exit(-1);
    }
    if (sp_h_set_field(spp, "sample_byte_format", T_STRING, (void *)"10")){
	fprintf(spfp,"Error: unable to set sample_byte_format field\n");
	sp_print_return_status(spfp); 	exit(-1);
    }
    if (sp_h_set_field(spp, "sample_coding", T_STRING, (void *)"pcm")){
	fprintf(spfp,"Error: unable to set sample_coding field\n");
	sp_print_return_status(spfp); 	exit(-1);
    }
    lint = 1;
    if (sp_h_set_field(spp, "channel_count", T_INTEGER, (void *)&lint)){
	fprintf(spfp,"Error: unable to set channel_count field\n");
	sp_print_return_status(spfp); 	exit(-1);
    }

    if (sp_set_data_mode(spp,"SE-PCM-2") != 0){
	fprintf(spfp,"Error: sp_set_data_mode failed\n");
	sp_print_return_status(spfp);
	exit(-1);
    }

    fprintf(spfp,"---- Allocating a waveform buffer\n");
    if ((wavbuf=(short *)sp_data_alloc(spp,buf_nsamp)) == (short *)0){
	fprintf(spfp,"Unable to malloc memory for wav buffer\n");
	exit(-1);
    }

    factor = 1.0 / 100.0 ;
    for (i=0; i<buf_nsamp; i++)
	wavbuf[i] = (short)(1000 * cos( M_PI * 2.0 * i * factor));


    fprintf(spfp,"---- Writing the waveform\n");
    while (samps_written < size){
	stow = (samps_written + buf_nsamp) < size ? buf_nsamp :
	    size - samps_written;
	status = sp_write_data((void *)wavbuf, sizeof(short), stow, spp);
	if (status != stow){
	    fprintf(spfp,"Couldn't write NIST waveform file: %s\n", name);
	    sp_print_return_status(spfp);
	    status = sp_error(spp);
	    sp_print_return_status(spfp);
	    sp_close(spp);
	    (void) mtrf_free((char *)wavbuf);
	    exit(-1);
	}	
	samps_written += stow;
    }
    fprintf(spfp,"---- Closing the file\n");
    sp_data_free(spp,wavbuf);
    if (sp_close(spp) != 0) {
	fprintf(spfp,"SP_close failed\n");
	sp_print_return_status(spfp);
	exit(-1);
    }
    fprintf(spfp,"\n");

    unlink(name); 
}

void ulaw_test(int test)
{
    fprintf(spfp,"Test %2d: Ulaw Test conversions\n",test);
    fprintf(spfp,"---- Test creation from the original\n");
    waveform_update(EX1_10,"SE-ULAW:SBF-1",EX4_ULAW);
    waveform_update(EX1_01,"SE-ULAW:SBF-1",EX4_ULAW);

    fprintf(spfp,"---- Write as 2 Byte PCM file\n");
    waveform_update(EX4_ULAW,"SE-PCM-2:SBF-10",EX4_ULAW_10);
    waveform_update(EX4_ULAW,"SE-PCM-2:SBF-01",EX4_ULAW_01);
    waveform_update(EX4_ULAW,"SE-SHORTEN",EX4_ULAW_SHORTEN);
    waveform_update(EX4_ULAW,"SE-WAVPACK",EX4_ULAW_WAVPACK);
    waveform_update(EX4_ULAW_WAVPACK,"SE-ULAW",EX4_ULAW);
    waveform_update(EX4_ULAW_SHORTEN,"SE-ULAW",EX4_ULAW);
    waveform_update(EX4_ULAW_WAVPACK,"SE-PCM-2:SBF-10",EX4_ULAW_10);
    waveform_update(EX4_ULAW_SHORTEN,"SE-PCM-2:SBF-01",EX4_ULAW_01);

    fprintf(spfp,"---- Read as 2 Byte PCM file\n");
    converted_read_check(EX4_ULAW,EX4_ULAW_10,"SE-PCM-2:SBF-10");
    converted_read_check(EX4_ULAW,EX4_ULAW_10,"SE-PCM-2:SBF-01");
    converted_read_check(EX4_ULAW,EX4_ULAW_01,"SE-PCM-2:SBF-10");
    converted_read_check(EX4_ULAW,EX4_ULAW_01,"SE-PCM-2:SBF-01");

    fprintf(spfp,"---- Read as 1 Byte ULAW file\n");
    converted_read_check(EX1_10,EX4_ULAW,"SE-ULAW:SBF-1");
    converted_read_check(EX1_01,EX4_ULAW,"SE-ULAW:SBF-1");


    fprintf(spfp,"\n");
}

void pculaw_test(int test)
{
    fprintf(spfp,"Test %2d: Pculaw Test conversions\n",test);
    fprintf(spfp,"---- Test creation from the original\n");
    waveform_update(EX1_10,"SE-PCULAW:SBF-1",EX7_PCULAW);
    waveform_update(EX1_01,"SE-PCULAW:SBF-1",EX7_PCULAW);

    fprintf(spfp,"---- Write as 2 Byte PCM file\n");
    waveform_update(EX7_PCULAW,"SE-PCM-2:SBF-10",EX7_PCULAW_10);
    waveform_update(EX7_PCULAW,"SE-PCM-2:SBF-01",EX7_PCULAW_01);
    waveform_update(EX7_PCULAW,"SE-SHORTEN",EX7_PCULAW_SHORTEN);
    waveform_update(EX7_PCULAW,"SE-WAVPACK",EX7_PCULAW_WAVPACK);
    waveform_update(EX7_PCULAW_WAVPACK,"SE-PCULAW",EX7_PCULAW);
    waveform_update(EX7_PCULAW_SHORTEN,"SE-PCULAW",EX7_PCULAW);
    waveform_update(EX7_PCULAW_WAVPACK,"SE-PCM-2:SBF-10",EX7_PCULAW_10);
    waveform_update(EX7_PCULAW_SHORTEN,"SE-PCM-2:SBF-01",EX7_PCULAW_01);

    fprintf(spfp,"---- Read as 2 Byte PCM file\n");
    converted_read_check(EX7_PCULAW,EX7_PCULAW_10,"SE-PCM-2:SBF-10");
    converted_read_check(EX7_PCULAW,EX7_PCULAW_10,"SE-PCM-2:SBF-01");
    converted_read_check(EX7_PCULAW,EX7_PCULAW_01,"SE-PCM-2:SBF-10");
    converted_read_check(EX7_PCULAW,EX7_PCULAW_01,"SE-PCM-2:SBF-01");

    fprintf(spfp,"---- Read as 1 Byte PCULAW file\n");
    converted_read_check(EX1_10,EX7_PCULAW,"SE-PCULAW:SBF-1");
    converted_read_check(EX1_01,EX7_PCULAW,"SE-PCULAW:SBF-1");


    fprintf(spfp,"\n");
}

void converted_read_check(char *file1, char *file2, char *conv)
{
    fprintf(spfp,"------ File: %s converted by %s, compared to %s\n",
	   file1,conv,file2);
    if (diff_waveforms(file1,file2,conv,conv,1,spfp) > 0)
	exit(-1);
}

void two_channel_test(int test)
{
    fprintf(spfp,"Test %2d: Two Channel READ test:\n",test);

    fprintf(spfp,"---- PCM stereo files compared to both single channels:\n");
    if (perform_2_channel_read_test(EX5_2CHAN_PCM,"",
				    EX5_CHAN1_PCM,EX5_CHAN2_PCM) > 0) exit(-1);
    if (perform_2_channel_read_test(EX5_2CHAN_PCM_WAVPACK,"",
				    EX5_CHAN1_PCM,EX5_CHAN2_PCM) > 0) exit(-1);
    if (perform_2_channel_read_test(EX5_2CHAN_PCM_SHORTEN,"",
				    EX5_CHAN1_PCM,EX5_CHAN2_PCM) > 0) exit(-1);

    fprintf(spfp,"---- ULAW stereo files compared to both single channels:\n");
    if (perform_2_channel_read_test(EX5_2CHAN,"",
				    EX5_CHAN1,EX5_CHAN2) > 0) exit(-1);
    if (perform_2_channel_read_test(EX5_2CHAN_SHORTEN,"",
				    EX5_CHAN1,EX5_CHAN2) > 0) exit(-1);
    if (perform_2_channel_read_test(EX5_2CHAN_WAVPACK,"",
				    EX5_CHAN1,EX5_CHAN2) > 0) exit(-1);

    fprintf(spfp,"---- ULAW stereo files converted to PCM, ");
    fprintf(spfp,"compared to both single channels:\n");
    if (perform_2_channel_read_test(EX5_2CHAN,"SE-PCM",EX5_CHAN1_PCM,
				    EX5_CHAN2_PCM) > 0) exit(-1);
    if (perform_2_channel_read_test(EX5_2CHAN_SHORTEN,"SE-PCM",
				    EX5_CHAN1_PCM,EX5_CHAN2_PCM) > 0) exit(-1);
    if (perform_2_channel_read_test(EX5_2CHAN_WAVPACK,"SE-PCM",
				    EX5_CHAN1_PCM,EX5_CHAN2_PCM) > 0) exit(-1);

    fprintf(spfp,"---- ULAW stereo files converted to PCM stereo files:\n");
    converted_read_check(EX5_2CHAN,EX5_2CHAN_PCM,"SE-PCM:SBF-10");
    converted_read_check(EX5_2CHAN_WAVPACK,EX5_2CHAN_PCM,"SE-PCM:SBF-10");
    converted_read_check(EX5_2CHAN_SHORTEN,EX5_2CHAN_PCM,"SE-PCM:SBF-10");

    fprintf(spfp,"-- Two Channel WRITE test\n");
    fprintf(spfp,"---- 2 ULAW single channel files, creating a stereo file\n");
    if (perform_2_channel_write_test(EX5_2CHAN,"",
				     EX5_CHAN1,EX5_CHAN2) > 0) exit(-1);
    if (perform_2_channel_write_test(EX5_2CHAN_SHORTEN,"SE-SHORTEN",
				     EX5_CHAN1,EX5_CHAN2) > 0) exit(-1);
    if (perform_2_channel_write_test(EX5_2CHAN_WAVPACK,"SE-WAVPACK",
				     EX5_CHAN1,EX5_CHAN2) > 0) exit(-1);
    fprintf(spfp,"---- 2 PCM single channel files, creating a stereo file\n");
    if (perform_2_channel_write_test(EX5_2CHAN_PCM,"",EX5_CHAN1_PCM,
				     EX5_CHAN2_PCM) > 0) exit(-1);
    if (perform_2_channel_write_test(EX5_2CHAN_PCM_SHORTEN,"SE-SHORTEN",
				     EX5_CHAN1_PCM,EX5_CHAN2_PCM) >0) exit(-1);
    if (perform_2_channel_write_test(EX5_2CHAN_PCM_WAVPACK,"SE-WAVPACK",
				     EX5_CHAN1_PCM,EX5_CHAN2_PCM) >0) exit(-1);
    fprintf(spfp,
	    "---- 2 ULAW single channel files, creating a PCM stereo file\n");
    if (perform_2_channel_write_test(EX5_2CHAN_PCM,"SE-PCM",
				     EX5_CHAN1,EX5_CHAN2) > 0) exit(-1);

    fprintf(spfp,"\n");
}

int perform_2_channel_write_test(char *combined_wav, char *conversion,
				 char *chan1_wav, char *chan2_wav)
{
    SP_FILE *new_sp=SPNULL, *chan1_sp=SPNULL, *chan2_sp=SPNULL;
    char *new_buf=CNULL, *new_ptr, *chan1_buf=CNULL, *chan2_buf=CNULL;
    char *new_wav="output.wav";
    SP_INTEGER lint, chan1_snb, chan2_snb, new_snb;
    size_t s, i;
    size_t frame_size=160;
    int return_value;

    fprintf(spfp,"------ Creating stereo file from '%s' and ,",chan1_wav);
    fprintf(spfp,"'%s' conversions '%s', verifying to file '%s'\n",
	   chan2_wav, conversion ,combined_wav);

    /********   Channel 1 SP_FILE ****************/
    if ((chan1_sp = sp_open(chan1_wav, "rv")) == SPNULL) {
	fprintf(spfp,"Couldn't open for read-verified single ");
	fprintf(spfp,"channel file: %s", chan1_wav);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    if ((chan1_buf = (char *)sp_data_alloc(chan1_sp,frame_size)) == CNULL){
	fprintf(spfp,"Unable to allocate waveform data for ");
	fprintf(spfp,"file '%s'\n",chan1_wav);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    if (sp_h_get_field(chan1_sp,SAMPLE_N_BYTES_FIELD,T_INTEGER,
		       (void *)&chan1_snb) != 0){
	fprintf(spfp,"Unable to retieve %s field from file '%s'\n",
		SAMPLE_N_BYTES_FIELD,chan1_wav);
	goto FATAL_QUIT;
    }


    /********   Channel 2 SP_FILE ****************/
    if ((chan2_sp = sp_open(chan2_wav, "rv")) == SPNULL) {
	fprintf(spfp,"Couldn't open for read-verified single ");
	fprintf(spfp,"channel file: %s", chan2_wav);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    if ((chan2_buf = (char *)sp_data_alloc(chan2_sp,frame_size)) == CNULL){
	fprintf(spfp,"Unable to allocate waveform data for ");
	fprintf(spfp,"file '%s'\n",chan2_wav);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    if (sp_h_get_field(chan2_sp,SAMPLE_N_BYTES_FIELD,
		       T_INTEGER,(void *)&chan2_snb) != 0){
	fprintf(spfp,"Unable to retieve %s field from file '%s'\n",
		SAMPLE_N_BYTES_FIELD,chan1_wav);
	goto FATAL_QUIT;
    }



    if ((new_sp = sp_open(new_wav, "wv")) == SPNULL) {
	fprintf(spfp,"Couldn't open for write-verified 2 channel ");
	fprintf(spfp,"file: %s", new_wav);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    /* fill the header with fields from channel 1 */
    if (sp_copy_header(chan1_sp, new_sp) > 0){
	fprintf(spfp,"Couldn't duplicate the header in file ");
	fprintf(spfp,"'%s' for file '%s'\n",chan1_wav,new_wav);
	sp_print_return_status(spfp);
    }
    if (sp_h_get_field(new_sp,SAMPLE_N_BYTES_FIELD,
		       T_INTEGER,(void *)&new_snb) != 0){
	fprintf(spfp,"Unable to retieve %s",SAMPLE_N_BYTES_FIELD);
	fprintf(spfp," field from file '%s'\n",new_wav);
	goto FATAL_QUIT;
    }
    /* now reset the appropriate fields */
    /* set the channel count to 2 */
    lint = 2;
    if (sp_h_set_field(new_sp,CHANNEL_COUNT_FIELD,
		       T_INTEGER,(void *)&lint) != 0){
	fprintf(spfp,"Unable to retieve %s field from file '%s'\n",
		CHANNEL_COUNT_FIELD,new_wav);
	goto FATAL_QUIT;
    }
    sp_h_delete_field(new_sp,SAMPLE_CHECKSUM_FIELD);
    if ((new_buf = (char *)sp_data_alloc(new_sp,frame_size)) == CNULL){
	fprintf(spfp,"Unable to allocate waveform data for file '%s'\n",
		new_wav);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    if (! strsame(conversion,"") )
	if (sp_set_data_mode(new_sp,conversion) != 0){
	    fprintf(spfp,"Error: sp_set_data_mode to ");
	    fprintf(spfp,"'%s' failed on file '%s'\n",conversion,new_wav);
	    sp_print_return_status(spfp);
	}

    /* now read through the 2 channel files, interleaving them and writing  */
    /* them to the new_sp just cycle through the data checking to make      */
    /* sure the interleaving is correct */
    do {	
	s = sp_read_data(chan1_buf,chan1_snb,frame_size,chan1_sp);
	if ((sp_get_return_status() > 0) && (s == 0)) {
	    fprintf(spfp,"Unable to read samples from channel 1 file\n");
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT; 
	}
	if (sp_read_data(chan2_buf,chan2_snb,s,chan2_sp) != s){
	    fprintf(spfp,"Unable to read %ld samples from file %s\n",
		    s,chan2_wav);
 	    goto FATAL_QUIT;
	}

	new_ptr = new_buf;
	/* interleave the samples */
	for (i=0; i<s; i++) {
	    memcpy(new_ptr + i*2*chan1_snb,
		   chan1_buf + i*chan1_snb, chan1_snb);
	    memcpy(new_ptr + i*2*chan2_snb + chan2_snb,
		   chan2_buf + i*chan2_snb, chan2_snb);
	}
	/* write the buffer out */
	if (sp_write_data(new_buf,new_snb,s,new_sp) != s){
	    fprintf(spfp,"Error: Unable to write %ld samples to file '%s'\n",
		    s,new_wav);
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT;
	}	    
    } while (s > 0);

    /* everything is ok */
    return_value = 0;
    goto CLEAN_UP;

  FATAL_QUIT:  /* Failed routine */
    return_value = 100;

  CLEAN_UP:
    if (new_sp != SPNULL) {
	if (new_buf != CNULL) sp_data_free(new_sp,new_buf);
	if (sp_close(new_sp) != 0){
	    fprintf(spfp,"Error: Unable to close stereo file '%s'\n",
		    new_wav);
	    sp_print_return_status(spfp);
	    return_value = 200;
	}
    }
    if (chan1_sp != SPNULL) {
	if (chan1_buf != CNULL) sp_data_free(chan1_sp,chan1_buf);
	sp_close(chan1_sp);
    }
    if (chan2_sp != SPNULL) {
	if (chan2_buf != CNULL) sp_data_free(chan2_sp,chan2_buf);
	sp_close(chan2_sp);
    }

    if (return_value == 0){
	int rtn;
	if (diff_waveforms(combined_wav,new_wav,CNULL,CNULL,1,spfp) != 0){
	    fprintf(spfp,"Successful stereo file creation BUT failed ");
	    fprintf(spfp,"waveform verification\n");
	    exit(-1);
	}
	if ((rtn = diff_data(combined_wav,new_wav,1,spfp)) != 0){
	    fprintf(spfp,"Warning: files '%s' and '%s' decode to the",
		    combined_wav,new_wav);
	    if (rtn == 100) 
		fprintf(spfp," same form, but are not identical on disk\n");
	    else
		fprintf(spfp," same form, but are byte-swapped on disk\n");
	    warning++;
	}
	unlink(new_wav);
    }

    return(return_value);
}

int perform_2_channel_read_test(char *combined_wav, char *conversion,
				char *chan1_wav, char *chan2_wav)
{
    SP_FILE *comb_sp=SPNULL, *chan1_sp=SPNULL, *chan2_sp=SPNULL;
    char *comb_buf=CNULL, *chan1_buf=CNULL, *chan2_buf=CNULL;
    SP_INTEGER chan1_snb, chan2_snb, comb_snb;
    size_t s, i;
    int frame_size=160;
    int return_value;

    fprintf(spfp,
	    "------ File '%s', converted by '%s' ",combined_wav,conversion);
    fprintf(spfp,"compared to channel 1 '%s' and channel 2 '%s'\n",
	   chan1_wav, chan2_wav);
    if ((comb_sp = sp_open(combined_wav, "rv")) == SPNULL) {
	fprintf(spfp,"Couldn't open for read-verified 2 channel ");
	fprintf(spfp,"file: %s", combined_wav);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    /* convert the input format of the stereo file */
    if (! strsame(conversion,"") )
	if (sp_set_data_mode(comb_sp,conversion) != 0){
	    fprintf(spfp,"Error: sp_set_data_mode to ");
	    fprintf(spfp,"'%s' failed on file '%s'\n",
		    conversion,combined_wav);
	    sp_print_return_status(spfp);
	}
    if ((comb_buf = (char *)sp_data_alloc(comb_sp,frame_size)) == CNULL){
	fprintf(spfp,"Unable to allocate memory for ");
	fprintf(spfp,"file '%s'\n",combined_wav);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    if (sp_h_get_field(comb_sp,SAMPLE_N_BYTES_FIELD,
		       T_INTEGER,(void *)&comb_snb) != 0){
	fprintf(spfp,"Unable to retieve ");
	fprintf(spfp,"%s field from file '%s'\n",
		SAMPLE_N_BYTES_FIELD,combined_wav);	
	goto FATAL_QUIT;
    }
	
    /********   Channel 1 SP_FILE ****************/
    if ((chan1_sp = sp_open(chan1_wav, "rv")) == SPNULL) {
	fprintf(spfp,"Couldn't open for read-verified ");
	fprintf(spfp,"single channel file: %s", chan1_wav);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    if (sp_h_get_field(chan1_sp,SAMPLE_N_BYTES_FIELD,
		       T_INTEGER,(void *)&chan1_snb) != 0){
	fprintf(spfp,"Unable to retieve %s ",SAMPLE_N_BYTES_FIELD);
	fprintf(spfp,"field from file '%s'\n",chan1_wav);
	goto FATAL_QUIT;
    }
    if ((chan1_buf = (char *)sp_data_alloc(chan1_sp,frame_size)) == CNULL){
	fprintf(spfp,"Unable to allocate memory for file '%s'\n",chan1_wav);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }


    /********   Channel 2 SP_FILE ****************/
    if ((chan2_sp = sp_open(chan2_wav, "rv")) == SPNULL) {
	fprintf(spfp,"Couldn't open for read-verified single channel ");
	fprintf(spfp,"file: %s", chan2_wav);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    if ((chan2_buf = (char *)sp_data_alloc(chan2_sp,frame_size)) == CNULL){
	fprintf(spfp,"Unable to allocate memory for file '%s'\n",chan2_wav);
	sp_print_return_status(spfp);
	goto FATAL_QUIT;
    }
    if (sp_h_get_field(chan2_sp,SAMPLE_N_BYTES_FIELD,
		       T_INTEGER,(void *)&chan2_snb) != 0){
	fprintf(spfp,"Unable to retieve %s field from file '%s'\n",
		SAMPLE_N_BYTES_FIELD,chan1_wav);
	goto FATAL_QUIT;
    }

    /* just cycle through the data checking to make sure the interleaving  */
    /* is correct */
    do {	
	s = sp_read_data(comb_buf,comb_snb,frame_size,comb_sp);
	if ((sp_get_return_status() > 0) && (s == 0)) {
	    fprintf(spfp,"Unable to read samples from stereo file\n");
	    sp_print_return_status(spfp);
	    goto FATAL_QUIT; 
	}
	if (sp_read_data(chan1_buf,chan1_snb,s,chan1_sp) != s){
	    sp_print_return_status(spfp);
	    fprintf(spfp,"Unable to read %ld samples from file %s\n",
		    s,chan1_wav);
 	    goto FATAL_QUIT; 
	}
	if (sp_read_data(chan2_buf,chan2_snb,s,chan2_sp) != s){
	    fprintf(spfp,"Unable to read %ld samples from file %s\n",
		    s,chan2_wav);
 	    goto FATAL_QUIT;
	}

	/*	for (i=0; i<s; i++) 
		printf ("   %2x %2x %2x %2x-> %2x %2x %2x %2x\n",
		*(comb_buf + i*2*comb_snb),
		(comb_buf + i*2*comb_snb+1),
		*(comb_buf + i*2*comb_snb+2),*(comb_buf + i*2*comb_snb+3),
		*(chan1_buf + i*chan1_snb), *(chan1_buf + i*chan1_snb+1),
		*(chan2_buf + i*chan2_snb), *(chan2_buf + i*chan2_snb+1));*/

	for (i=0; i<s; i++) {
	    if (memcmp(comb_buf + i*2*comb_snb,chan1_buf+ i*chan1_snb,comb_snb)){
		int bn;
		fprintf(spfp,"Channel 1 differs from the stereo version ");
		fprintf(spfp,"at sample %ld\n",i);
		fprintf(spfp,"    Stereo Chan 1:  ");
		for (bn=0; bn < comb_snb; bn++)
		    fprintf(spfp,"%2x ",
			   *((unsigned char *)(comb_buf + i*2*comb_snb + bn)));
		fprintf(spfp,"\n");
		fprintf(spfp,"           Chan 1:  ");
		for (bn=0; bn < chan1_snb; bn++)
		    fprintf(spfp,"%2x ",
			   *((unsigned char *)(chan1_buf + i*chan1_snb + bn)));
		fprintf(spfp,"\n");
		goto FATAL_QUIT;
	    }
	    if (memcmp(comb_buf + i*2*comb_snb + (1*comb_snb),
		     chan2_buf+i*chan2_snb,comb_snb)){
		int bn;	
		fprintf(spfp,"Channel 2 differs from the stereo version ");
		fprintf(spfp,"at sample %ld\n",i);
		fprintf(spfp,"    Stereo Chan 2:  ");
		for (bn=0; bn < comb_snb; bn++)
		    fprintf(spfp,"%2x ",
			   *((unsigned char *)(comb_buf + i*2*comb_snb + bn)));
		fprintf(spfp,"\n");
		fprintf(spfp,"           Chan 2:  ");
		for (bn=0; bn < chan2_snb; bn++)
		    fprintf(spfp,"%2x ",
			   *((unsigned char *)(chan2_buf + i*chan2_snb + bn)));
		fprintf(spfp,"\n");
		goto FATAL_QUIT;
	    }
	}
    } while (s > 0);

    /* everything is ok */
    return_value = 0;
    goto CLEAN_UP;

  FATAL_QUIT:  /* Failed routine */
    return_value = 100;

  CLEAN_UP:
    if (comb_sp != SPNULL) {
	if (comb_buf != CNULL) sp_data_free(comb_sp,comb_buf);
	sp_close(comb_sp);
    }
    if (chan1_sp != SPNULL) {
	if (chan1_buf != CNULL) sp_data_free(chan1_sp,chan1_buf);
	sp_close(chan1_sp);
    }
    if (chan2_sp != SPNULL) {
	if (chan2_buf != CNULL) sp_data_free(chan2_sp,chan2_buf);
	sp_close(chan2_sp);
    }
    return(return_value);
}



void large_file_test(int test)
{
    char *large_file_name = "large.wav";
    char *large2_file_name = "large2.wav";
    
    fprintf(spfp,"Test %2d: Large File Handling and sp_tell() for write:\n",test);
    fprintf(spfp,"---- Building CONTROL data file, %s\n",large2_file_name);
    make_test_file(large2_file_name,160000,"");

    do_large_file_conversion(large_file_name, large2_file_name, 
			     "SE-PCM:SBF-01");
    do_large_file_conversion(large_file_name, large2_file_name, 
			     "SE-SHORTEN:SBF-10");
    do_large_file_conversion(large_file_name, large2_file_name, 
			     "SE-WAVPACK:SBF-01");

    unlink(large2_file_name);
    fprintf(spfp,"\n");
}

void do_large_file_conversion(char *modified, char *control, char *mode)
{
    fprintf(spfp,"------ Building Test file '%s' converted by ",modified);
    fprintf(spfp,"'%s', using control file %s\n",mode,control);

    make_test_file(modified,160000,mode);
    if (diff_waveforms(modified,control,CNULL,CNULL,1,spfp) != 0){
	fprintf(spfp,"Successful update failed waveform verification\n");
	exit(-1);
    }
    unlink(modified);
}


void make_test_file(char *name, int size, char *conversion)
{
    SP_FILE *spp;
    short *wavbuf;
    SP_INTEGER lint;
    SP_INTEGER buf_nsamp = 16000;
    SP_INTEGER nbyte = 2;
    SP_INTEGER srate = 16000;
    int stow, i;
    int samps_written=0, status;

    if ((wavbuf=(short *)mtrf_malloc(sizeof(short)*buf_nsamp)) == (short *)0){
	fprintf(spfp,"Unable to malloc memory for wav buffer\n");
	exit(-1);
    }

    for (i=0; i<buf_nsamp; i++)
	wavbuf[i] = 10 * ( i % (16 + (i / 100)));

    spp = sp_open(name, "w");
    if (spp == NULL) {
	fprintf(spfp,"Couldn't open NIST waveform file: %s\n", name);
	sp_print_return_status(spfp);
	exit(-1);
    }

    lint = size;
    sp_h_set_field(spp, "sample_count", T_INTEGER,(void *) &lint);
    sp_h_set_field(spp, "sample_rate", T_INTEGER,(void *) &srate);
    sp_h_set_field(spp, "sample_n_bytes", T_INTEGER, (void *) &nbyte);
    sp_h_set_field(spp, "sample_byte_format", T_STRING, (void *)"10");
    sp_h_set_field(spp, "sample_coding", T_STRING, (void *)"pcm");
    lint = 1; sp_h_set_field(spp, "channel_count", T_INTEGER, (void *)&lint);
    if (! strsame(conversion,"") )
	if (sp_set_data_mode(spp,conversion) != 0){
	    sp_print_return_status(spfp);
	    fprintf(spfp,"Error: sp_set_data_mode() failed on file '%s'\n",
		    name);
	    exit(-1);
	}


    while (samps_written < size){
	stow = (samps_written + buf_nsamp) < size ? buf_nsamp :
	    size - samps_written;
	status = sp_write_data((void *)wavbuf, sizeof(short), stow, spp);
	if (status != stow){
	    fprintf(spfp,"Couldn't write NIST waveform file: %s\n", name);
	    sp_print_return_status(spfp);
	    status = sp_error(spp);
	    sp_print_return_status(spfp);
	    sp_close(spp);
	    (void) mtrf_free((char *)wavbuf);
	    exit(-1);
	}	
	samps_written += stow;
	if (do_sp_tell_check(spp,name) != 0)
	    exit(-1);
    }
    if (sp_close(spp) != 0) {
	fprintf(spfp,"SP_close failed\n");
	sp_print_return_status(spfp);
	exit(-1);
    }
    (void) mtrf_free((char *)wavbuf);
}

void write_required_field_test(int test)
{
    SP_FILE *spp;
    char *outfilename="outputz.wav";
    short *wavbuf;
    SP_INTEGER lint;
    SP_INTEGER buf_nsamp = 16000;
    SP_INTEGER nbyte = 2;
    int i, status;

    fprintf(spfp,"Test %2d: Required Field Test:\n",test);

    if ((wavbuf=(short *)mtrf_malloc(sizeof(short)*buf_nsamp)) == (short *)0){
	fprintf(spfp,"Unable to malloc memory for wav buffer\n");
	exit(-1);
    }

    for (i=0; i<buf_nsamp; i++)
	wavbuf[i] = 10 * ( i % (16 + (i / 100)));

    spp = sp_open(outfilename, "wv");
    if (spp == NULL) {
	fprintf(spfp,"Couldn't open NIST waveform file: %s\n", outfilename);
	sp_print_return_status(spfp);
	exit(-1);
    }

    for (i=0; i<5; i++){
	switch (i) {
	  case 0: fprintf(spfp,
			  "---- Test %d: Missing sample_n_bytes\n",i+1); break;
	  case 1: fprintf(spfp,
			  "---- Test %d: Missing channel_count\n",i+1); break;
	  case 2:
	    fprintf(spfp,
		    "---- Test %d: sample_encoding 'ulaw' and missing ",i+1);
	    fprintf(spfp,"sample_rate\n"); break;
	  case 3: fprintf(spfp,"---- Test %d: sample_encoding 'pcm' and ",i+1);
	    fprintf(spfp,"missing sample_rate\n"); break;
	  case 4: fprintf(spfp,
			  "---- Test %d: All fields present\n",i+1); break;
	}

	lint = buf_nsamp; sp_h_set_field(spp, "sample_count",
					 T_INTEGER, (void *) &lint); 

	if (i > 0) sp_h_set_field(spp, "sample_n_bytes", 
				  T_INTEGER, (void *) &nbyte);
	if (i > 1) { lint = 1;  sp_h_set_field(spp, "channel_count",
					       T_INTEGER, (void *) &lint);}
	if (i > 2) { sp_h_set_field(spp, "sample_coding",
				    T_STRING, (void *) "ulaw"); }
	if (i > 3) { sp_h_set_field(spp, "sample_coding", 
				    T_STRING, (void *) "pcm"); }
	if (i > 3) { lint = 16000;  sp_h_set_field(spp, "sample_rate", 
						   T_INTEGER, (void *) &lint);}

	status = sp_write_data((void *)wavbuf, sizeof(short), buf_nsamp, spp);
	if ((i < 4) && (status > 0)){
	    fprintf(spfp,"************* TEST FAILED ****************\n");
	    sp_print_return_status(spfp);
	}
	else if ((i == 4) && (status != buf_nsamp)) {
	    fprintf(spfp,"************* TEST FAILED ****************\n");
	    sp_print_return_status(spfp);
	}
    }
    if (sp_close(spp) != 0) {
	fprintf(spfp,"SP_close failed\n");
	sp_print_return_status(spfp);
	exit(-1);
    }
    (void) mtrf_free((char *)wavbuf);
    unlink(outfilename);
    fprintf(spfp,"\n");
}


void write_check_adding_fields(char *comp_file, char *conversion_str)
{
    SP_FILE *spp;
    char *outfilename = "outputw.wav";
    short *wavbuf;
    int i, status;
    SP_INTEGER nsamp = 16000;
    SP_INTEGER nbyte = 2;
    SP_INTEGER srate = 16000;
    SP_CHECKSUM expected_checksum=61520;
    SP_INTEGER lchecksum=expected_checksum;
    SP_INTEGER channels=1;

    fprintf(spfp,"------ Data Mode string '%s'\n",conversion_str);
    fprintf(spfp,"------   Compared to file '%s'\n",comp_file);

    if ((wavbuf=(short *)mtrf_malloc(sizeof(short)*nsamp)) == (short *)0){
	fprintf(spfp,"Unable to malloc memory for wav buffer\n");
	exit(-1);
    }
    for (i=0; i<16000; i++)
	wavbuf[i] = 10 * ( i % (16 + (i / 100)));

    for (i=0; i<5; i++){
	switch (i) {
	  case 0: fprintf(spfp,
			  "-------- Test %d: All fields present\n",i+1); break;
	  case 1: fprintf(spfp,
			  "-------- Test %d: Missing sample_count\n",i+1); 
	    break;
	  case 2: fprintf(spfp,
			  "-------- Test %d: Missing sample_checksum\n",i+1);
	    break;
	  case 3: fprintf(spfp,
			  "-------- Test %d: Missing sample_coding\n",i+1);
	    break;
	  case 4: fprintf(spfp,
			  "-------- Test %d: Missing sample_count, ",i+1);
	    fprintf(spfp,"sample_checksum and sample_coding\n"); break;
	}
	spp = sp_open(outfilename, "wv");
	if (spp == NULL) {
	   fprintf(spfp,"Couldn't open NIST waveform file: %s", outfilename);
	    sp_print_return_status(spfp);
	    exit(-1);
	}

	/* FILL IN ONLY THE REQUIRED FIELDS */
	/*  i = 0  ->  All fields present */
	/*  i = 1  ->  missing sample_count */
	/*  i = 2  ->  missing checksum */
	/*  i = 3  ->  missing sample coding */
	/*  i = 4  ->  missing sample_count and checksum */
	if ((i == 0) || (i == 2) || (i == 3))
	    sp_h_set_field(spp, "sample_count", T_INTEGER, (void *) &nsamp);
	if ((i == 0) || (i == 1) || (i == 3))
	    sp_h_set_field(spp, "sample_checksum", T_INTEGER, 
			   (void *) &lchecksum);
	if ((i == 0) || (i == 1) || (i == 2))
	    sp_h_set_field(spp, "sample_coding", T_STRING, (void *)"pcm");
	sp_h_set_field(spp, "sample_rate", T_INTEGER, (void *) &srate);
	sp_h_set_field(spp, "sample_n_bytes", T_INTEGER, (void *) &nbyte);
	sp_h_set_field(spp, "sample_byte_format", T_STRING,
		       (void *)get_natural_byte_order(2));
	sp_h_set_field(spp, "channel_count", T_INTEGER, (void *)&channels);

	if (! strsame(conversion_str,"")){
	    if (sp_set_data_mode(spp,conversion_str) != 0){
		fprintf(spfp,"Set data mode failed\n");
		sp_print_return_status(spfp);
		exit(-1);
	    }
	}

	status = sp_write_data((void *)wavbuf, sizeof(short), nsamp, spp);
	if (status != nsamp)
	    {
		fprintf(spfp,"Couldn't write NIST waveform file: %s\n",
			outfilename);
		sp_print_return_status(spfp);
		status = sp_error(spp);
		sp_print_return_status(spfp);
		sp_close(spp);
	    }	
	if (sp_close(spp) != 0) {
	    fprintf(spfp,"SP_close failed\n");
	    sp_print_return_status(spfp);
	    exit(-1);
	}
	    

	if (diff_waveforms(outfilename,comp_file,CNULL,CNULL,1,spfp) != 0){
	    fprintf(spfp,"Successful write failed waveform verification\n");
	    exit(-1);
	}
	{ int chg, ins, del, fail=0;
	  if (diff_header(outfilename,comp_file,&chg,&ins,&del,0,spfp) != 0){
	      fprintf(spfp,"Successful write failed header verification\n");
	      fail=1;
	  }
	  if ((fail==1) && (chg > 0 || ins > 0 || del > 0)) {
	      diff_header(outfilename,comp_file,&chg,&ins,&del,1,spfp);
	      exit(-1);
	  }
        }

	unlink(outfilename);
    }
    (void) mtrf_free((char *)wavbuf);
}

void write_check_adding_fields_test(int test)
{
    fprintf(spfp,"Test %2d: Write Adding Fields Check:\n",test);
    write_check_adding_fields(EX2_10,"SBF-10");
    write_check_adding_fields(EX2_01,"SBF-01");
    write_check_adding_fields(EX2_10_SHORTEN,"SE-SHORTEN:SBF-10");
    write_check_adding_fields(EX2_10_WAVPACK,"SE-WAVPACK:SBF-10");
    write_check_adding_fields(EX2_10_SHORTEN,"SE-SHORTEN:SBF-10");
    write_check_adding_fields(EX2_01,"SBF-01");
    write_check_adding_fields(EX2_01_SHORTEN,"SE-SHORTEN:SBF-01");
    write_check_adding_fields(EX2_01_WAVPACK,"SE-WAVPACK:SBF-01");
    write_check_adding_fields(EX2_01_SHORTEN,"SE-SHORTEN:SBF-01");
    fprintf(spfp,"\n");
}

void update_test(int test)
{
    fprintf(spfp,"Test %2d: Update Tests\n",test);
    fprintf(spfp,"---- Update of header fields\n");
    header_update(EX1_10,0);
    header_update(EX1_01,0);
    header_update(EX1_10_WAVPACK,0);
    header_update(EX1_10_SHORTEN,0);
    header_update(EX1_01_WAVPACK,0);
    header_update(EX1_01_SHORTEN,0);
    fprintf(spfp,"---- Update of header fields, expanding the header\n");
    header_update(EX1_10,1);
    header_update(EX1_01,1);
    header_update(EX1_10_WAVPACK,1);
    header_update(EX1_10_SHORTEN,1);
    header_update(EX1_01_WAVPACK,1);
    header_update(EX1_01_SHORTEN,1);

    fprintf(spfp,"---- Update the waveform format\n");
    waveform_update(EX1_10,"SBF-10",EX1_10);
    waveform_update(EX1_10,"SBF-01",EX1_01);
    waveform_update(EX1_10,"SE-WAVPACK:SBF-10",EX1_10_WAVPACK);
    waveform_update(EX1_10,"SE-SHORTEN:SBF-10",EX1_10_SHORTEN);
    waveform_update(EX1_10,"SE-WAVPACK:SBF-01",EX1_01_WAVPACK);
    waveform_update(EX1_10,"SE-SHORTEN:SBF-01",EX1_01_SHORTEN);

    waveform_update(EX1_01,"SBF-01",EX1_01);
    waveform_update(EX1_01,"SBF-10",EX1_10);
    waveform_update(EX1_01,"SE-WAVPACK:SBF-01",EX1_01_WAVPACK);
    waveform_update(EX1_01,"SE-SHORTEN:SBF-01",EX1_01_SHORTEN);
    waveform_update(EX1_01,"SE-WAVPACK:SBF-10",EX1_10_WAVPACK);
    waveform_update(EX1_01,"SE-SHORTEN:SBF-10",EX1_10_SHORTEN);

    waveform_update(EX1_10_SHORTEN,"SE-SHORTEN:SBF-10",EX1_10_SHORTEN);
    waveform_update(EX1_10_SHORTEN,"SE-SHORTEN:SBF-01",EX1_01_SHORTEN);
    waveform_update(EX1_10_SHORTEN,"SE-WAVPACK:SBF-10",EX1_10_WAVPACK);
    waveform_update(EX1_10_SHORTEN,"SE-WAVPACK:SBF-01",EX1_01_WAVPACK);
    waveform_update(EX1_10_SHORTEN,"SE-PCM:SBF-10",EX1_10);
    waveform_update(EX1_10_SHORTEN,"SE-PCM:SBF-01",EX1_01);

    waveform_update(EX1_10_WAVPACK,"SE-WAVPACK:SBF-10",EX1_10_WAVPACK);
    waveform_update(EX1_10_WAVPACK,"SE-WAVPACK:SBF-01",EX1_01_WAVPACK);
    waveform_update(EX1_10_WAVPACK,"SE-SHORTEN:SBF-10",EX1_10_SHORTEN);
    waveform_update(EX1_10_WAVPACK,"SE-SHORTEN:SBF-01",EX1_01_SHORTEN);
    waveform_update(EX1_10_WAVPACK,"SE-PCM:SBF-10",EX1_10);
    waveform_update(EX1_10_WAVPACK,"SE-PCM:SBF-01",EX1_01);


    waveform_update(EX1_10_SHORTPACK,"SE-SHORTEN:SBF-10",EX1_10_SHORTEN);
    waveform_update(EX1_10_SHORTPACK,"SE-SHORTEN:SBF-01",EX1_01_SHORTEN);
    waveform_update(EX1_10_SHORTPACK,"SE-WAVPACK:SBF-10",EX1_10_WAVPACK);
    waveform_update(EX1_10_SHORTPACK,"SE-WAVPACK:SBF-01",EX1_01_WAVPACK);
    waveform_update(EX1_10_SHORTPACK,"SE-PCM:SBF-10",EX1_10);
    waveform_update(EX1_10_SHORTPACK,"SE-PCM:SBF-01",EX1_01);

    waveform_update(EX1_01_SHORTEN,"SE-SHORTEN:SBF-01",EX1_01_SHORTEN);
    waveform_update(EX1_01_SHORTEN,"SE-SHORTEN:SBF-10",EX1_10_SHORTEN);
    waveform_update(EX1_01_SHORTEN,"SE-WAVPACK:SBF-10",EX1_10_WAVPACK);
    waveform_update(EX1_01_SHORTEN,"SE-WAVPACK:SBF-01",EX1_01_WAVPACK);
    waveform_update(EX1_01_SHORTEN,"SE-PCM:SBF-10",EX1_10);
    waveform_update(EX1_01_SHORTEN,"SE-PCM:SBF-01",EX1_01);

    waveform_update(EX1_01_WAVPACK,"SE-WAVPACK:SBF-01",EX1_01_WAVPACK);
    waveform_update(EX1_01_WAVPACK,"SE-WAVPACK:SBF-10",EX1_10_WAVPACK);
    waveform_update(EX1_01_WAVPACK,"SE-SHORTEN:SBF-10",EX1_10_SHORTEN);
    waveform_update(EX1_01_WAVPACK,"SE-SHORTEN:SBF-01",EX1_01_SHORTEN);
    waveform_update(EX1_01_WAVPACK,"SE-PCM:SBF-10",EX1_10);
    waveform_update(EX1_01_WAVPACK,"SE-PCM:SBF-01",EX1_01);

    waveform_update(EX1_01_SHORTPACK,"SE-SHORTEN:SBF-01",EX1_01_SHORTEN);
    waveform_update(EX1_01_SHORTPACK,"SE-SHORTEN:SBF-10",EX1_10_SHORTEN);
    waveform_update(EX1_01_SHORTPACK,"SE-WAVPACK:SBF-10",EX1_10_WAVPACK);
    waveform_update(EX1_01_SHORTPACK,"SE-WAVPACK:SBF-01",EX1_01_WAVPACK);
    waveform_update(EX1_01_SHORTPACK,"SE-PCM:SBF-10",EX1_10);
    waveform_update(EX1_01_SHORTPACK,"SE-PCM:SBF-01",EX1_01);

    fprintf(spfp,"\n");
}

void header_update(char *file, int expand)
{
    char *target="speaking_mode";
    char *change_target="database_id";
    char *insert_target="insert_field";
    SP_FILE *sp;
    int chg, ins, del, rtn;
    int added_field=0, num_to_add=30, i;

    fprintf(spfp,"------ File %s\n",file);
        
    system(rsprintf("%s %s output.wav",COPY,file));
    if ((sp=sp_open("output.wav","u")) == 0){
	fprintf(spfp,"Unable to open copy base file '%s' called '%s'\n",
	       file,"output.wav");
	sp_print_return_status(spfp);
        exit(-1);
    }
    if (sp_h_delete_field(sp,target) != 0){
	fprintf(spfp,
		"Can't delete field '%s' in file %s\n",target,"output.wav");
	sp_print_return_status(spfp);
    }
    if (sp_h_set_field(sp,change_target,T_STRING,"foobar") != 0){
	fprintf(spfp,"Can't change field '%s' in file %s\n",change_target,
	       "output.wav");
	sp_print_return_status(spfp);
    }
    if (sp_h_set_field(sp,insert_target,T_STRING,"foobar") != 0){
	fprintf(spfp,"Can't insert field '%s' in file %s\n",insert_target,
	       "output.wav");
	sp_print_return_status(spfp);
    }
    if (expand){
	for (i=0; i<num_to_add; i++) {
	    if (sp_h_set_field(sp,rsprintf("dummy_field_%d",i),
			       T_STRING,"Dummy field string value") != 0){
		fprintf(spfp,"Can't isnert field '%s' in file %s\n",
		       rsprintf("dummy_field_%d",i),"output.wav");
		sp_print_return_status(spfp);
	    }
	    added_field ++;
	}
    }
	

    if (sp_close(sp) != 0) {
	fprintf(spfp,"      Failed to close\n");
	sp_print_return_status(spfp);	
	exit(-1);
    }

    if (diff_waveforms("output.wav",file,CNULL,CNULL,1,spfp) != 0){
	fprintf(spfp,
		"Update passed, but waveforms of files '%s' and '%s' differ\n",
		"output.wav",file);
	exit(-1);
    }	
    if ((rtn = diff_data("output.wav",file,1,spfp)) != 0){
	fprintf(spfp,
		"WARNING: files '%s' and '%s' decompress to the same form,\n",
		"output.wav",file);
	if (rtn == 100) 
	    fprintf(spfp,"         but are not identical on disk\n");
	else
	    fprintf(spfp,"         but are byte-swapped on disk\n");
	warning++;
    }
    if (diff_header(file,"output.wav",&chg,&ins,&del,0,spfp) != 0){
	fprintf(spfp,"Unable to compare headers of file '%s' and '%s'\n",
	       "output.wav",file);
	fprintf(spfp,"         but are not identical on disk\n");
	exit(-1);
    }
    if (! expand){
	if ((del != 1) || (chg != 1) || (ins != 1)){
	    fprintf(spfp,
		    "   There should have been one field deleted, inserted");
	    fprintf(spfp," and changed, but the actual status is\n");
	    fprintf(spfp,"   Del: %d   Ins: %d   Chg: %d\n",del,ins,chg);
	    diff_header(file,"output.wav",&chg,&ins,&del,1,spfp);
	    exit(-1);
	}
    } else {
	if ((del != 1) || (chg != 1) || (ins != 1 + added_field)){
	    fprintf(spfp,"   There should have been one field deleted and ");
	    fprintf(spfp,"changed, and %d fields inserted\n",1+added_field);
	    fprintf(spfp,"   but the actual status is\n");
	    fprintf(spfp,"   Del: %d   Ins: %d   Chg: %d\n",del,ins,chg);
	    diff_header(file,"output.wav",&chg,&ins,&del,1,spfp);
	    exit(-1);
	}
    }
}

void waveform_update(char *base_file, char *sdm_mode, char *compare_file)
{
    SP_FILE *sp;
    int rtn;

    fprintf(spfp,
	    "------ File: %-16s converted by: %-21s compared to file: %s\n",
	   base_file,sdm_mode,compare_file);
    system(rsprintf("%s %s output.wav",COPY,base_file));
    if ((sp=sp_open("output.wav","u")) == 0){
	fprintf(spfp,"Unable to open copy base file '%s' called '%s'\n",
	       base_file,"output.wav");
	sp_print_return_status(spfp);
        exit(-1);
    }
    if (sp_set_data_mode(sp,sdm_mode) != 0){
	fprintf(spfp,"Set data mode failed\n");
	sp_print_return_status(spfp);
        exit(-1);
    }
    if (sp_close(sp) != 0) {
	fprintf(spfp,"      Failed to close\n");
	sp_print_return_status(spfp);
	exit(-1);
    }

    if (diff_waveforms("output.wav",compare_file,CNULL,CNULL,1,spfp) != 0){
	fprintf(spfp,
		"Update passed, but waveforms of files '%s' and '%s' differ\n",
	       "output.wav",compare_file);
	exit(-1);
    }
	
    if ((rtn = diff_data("output.wav",compare_file,1,spfp)) != 0){
	fprintf(spfp,
		"WARNING: files '%s' and '%s' decompress to the same form,\n",
	       "output.wav",compare_file);
	if (rtn == 100) 
	    fprintf(spfp,"         but are not identical on disk\n");
	else
	    fprintf(spfp,"         but are byte-swapped on disk\n");
	warning++;
    }
    unlink("output.wav");


}

void checksum_pre_post_verification(int test) 
{
    fprintf(spfp,"Test %2d: Checksum verification tests\n",test);
    fprintf(spfp,"---- Pre-Read Check\n");
    pre_read_check(EX1_10,0);
    pre_read_check(EX1_01,0);
    pre_read_check(EX1_10_WAVPACK,0);
    pre_read_check(EX1_10_SHORTEN,0);
    pre_read_check(EX1_10_SHORTPACK,0);
    pre_read_check(EX1_01_WAVPACK,0);
    pre_read_check(EX1_01_SHORTEN,0);
    pre_read_check(EX1_01_SHORTPACK,0);

    fprintf(spfp,"---- Pre-Read Check on corrupt data\n");
    pre_read_check(EX1_10_CORRUPT,1);
    pre_read_check(EX1_10_SHORTEN_CORRUPT,1);

    fprintf(spfp,"---- Post-Write Check\n");
    post_write_check(EX1_10);
    post_write_check(EX1_01);
    post_write_check(EX1_10_WAVPACK);
    post_write_check(EX1_01_WAVPACK);
    post_write_check(EX1_10_SHORTEN);
    post_write_check(EX1_01_SHORTEN);

    fprintf(spfp,"\n");
}

void pre_read_check(char *file, int corrupt)
{
    SP_FILE *sp;
    short buff[20];

    fprintf(spfp,"------ File: %s\n",file);
    if ((sp = sp_open(file,"rv")) == SPNULL) {
	sp_print_return_status(spfp);
        fprintf(spfp,"Error: Unable to open file %s to reading\n",file);
        exit(-1);
    }
    if (sp_read_data(buff,2,10,sp) != 10){
	if (!corrupt){
	    sp_print_return_status(spfp);
	    fprintf(spfp,"Error: Unable to pre-verify checksum in file %s\n",
		    file);
	    exit(-1);
	}
    } else {
	if (corrupt){
	    fprintf(spfp,"Error: pre-verify of corrupt file '%s' passed.\n",
		    file);
	    sp_print_return_status(spfp);
	    exit(-1);
	}
    }
    sp_close(sp);
}

void post_write_check(char *file)
{
    SP_FILE *sp, *out_sp;
    
    fprintf(spfp,"------ File: %s\n",file);
    if ((sp = sp_open(file,"r")) == SPNULL) {
	sp_print_return_status(spfp);
        fprintf(spfp,"Error: Unable open input file %s\n",file);
        exit(-1);
    }

    if (sp_set_data_mode(sp,"SBF-ORIG") != 0){
	sp_print_return_status(spfp);
	fprintf(spfp,"Error: sp_set_data_mode() failed on file '%s'\n",file);
	exit(-1);
    }

    if ((out_sp = sp_open("output.wav","w")) == SPNULL) {
	sp_print_return_status(spfp);
        fprintf(spfp,"Error: Unable to open SPHERE file %s\n","output.wav");
        exit(-1);
    }

    if (sp_copy_header(sp,out_sp) != 0){
	sp_print_return_status(spfp);
	exit(-1);
    }
/* NO NEED TO DO THIS 
    if (sp->read_spifr->status->file_compress !=
	sp->read_spifr->status->user_compress){
	switch (sp->read_spifr->status->file_compress){
	  case SP_wc_shortpack:
	    if (sp_set_data_mode(out_sp,"SE-SHORTPACK:SBF-ORIG") != 0){
		sp_print_return_status(spfp);
		fprintf(spfp,"Error: sp_set_data_mode() failed on file '%s'\n",
			"output.wav");
		exit(-1);
	    }
	    break;
          case SP_wc_wavpack:
	    if (sp_set_data_mode(out_sp,"SE-WAVPACK:SBF-ORIG") != 0){
		sp_print_return_status(spfp);
		fprintf(spfp,"Error: sp_set_data_mode() failed on file '%s'\n",
			"output.wav");
		exit(-1);
	    }
	    break;
          case SP_wc_shorten:
	    if (sp_set_data_mode(out_sp,"SE-SHORTEN:SBF-ORIG")  != 0){
		sp_print_return_status(spfp);
		fprintf(spfp,"Error: sp_set_data_mode() failed on file '%s'\n",
			"output.wav");
		exit(-1);
	    }
	    break;
	  default:
	    ;
	}
    }
    */
    if (sp_set_data_mode(out_sp,"SE-ORIG:SBF-ORIG") >= 100){
	sp_print_return_status(spfp);
	fprintf(spfp,"Error: sp_set_data_mode failed on file '%s'\n",
		"output.wav");
	exit(-1);
    }

    { short buff[2048];
      int ret;

      do {
          ret=sp_read_data(buff,2,1024,sp);
	  if ((ret == 0) && (sp_error(sp) > 0)){
	    fprintf(spfp,"   Read failed on input file\n");
	    sp_print_return_status(spfp);
	    exit(-1);
	  }
	  if (sp_write_data(buff,2,ret,out_sp) != ret){
	    fprintf(spfp,"   Write failed on output to file '%s'\n",
		    "output.wav");
	    sp_print_return_status(spfp);
	    exit(-1);
	  }
      } while (ret > 0);
      if (sp_error(sp) > 0 ){
	  fprintf(spfp,"Read failure on file %s\n",file);
	  sp_print_return_status(spfp);
      }
      if (sp_error(out_sp) > 0 ){
	  fprintf(spfp,"Write failure on file %s\n","output.wav");
	  sp_print_return_status(spfp);
      }
    }
/*    sp_file_dump(sp,spfp);
    sp_file_dump(out_sp,spfp);*/
    if (sp_close(sp) != 0) {
	fprintf(spfp,"    Closing of file '%s' opened for read failed\n",
		file);
	sp_print_return_status(spfp);
	exit(-1);
    }

    if (sp_close(out_sp) != 0) {
	fprintf(spfp,"    Closing of file '%s' opened for write failed\n",
		"output.wav");
	sp_print_return_status(spfp);
	if (sp_error(out_sp) == 100)
	    fprintf(spfp,"    POST-WRITE CHECKSUM VERIFICATION FAILED\n");
	sp_print_return_status(spfp);
	exit(-1);
    }
    if (diff_waveforms(file,"output.wav",CNULL,CNULL,1,spfp) != 0){
	fprintf(spfp,"    Verification passed, but files were not ");
	fprintf(spfp,"reproduced identically\n");
	exit(-1);
    }
    unlink("output.wav");
}
