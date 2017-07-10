
/* File: h_test.c */

/******************************************/
/* This program tests header functions    */
/* in an infinite loop that continually   */
/* inserts, retrieves, and deletes fields */
/* from a header.                         */
/* If there is a bug in there, we hope    */
/* that a function will eventually fail   */
/* due to a detected inconsistency or by  */
/* running out of memory.                 */
/******************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sp/sphere.h>

#define LONG1		27L
#define LONG2		-55L
#define DOUBLE1		205.17197
#define STR1		"foobar"

int main(int argc, char **argv)
{
    char *prog;
    struct header_t *h;
    int i, n, j, k;
    int f1size, f2size, f3size;
    char buf[BUFSIZ];
    char str_buf[BUFSIZ];
    char *str_cnt="1234567890abcdefghijklmnopqrstuvwxyz";

    prog = strrchr(argv[0],'/');
    prog = (prog == CNULL) ? argv[0] : (prog + 1);

    fprintf(spfp,"%s: %s\n",prog,sp_get_version());

    if (argc != 1) {
	(void) fprintf(spfp,"%s: no arguments expected\n",prog);
	exit(ERROR_EXIT_STATUS);
    }

    h = sp_create_header();
    if (h == HDRNULL) {
	(void) fprintf(spfp,"%s: Could not create header\n",prog);
	exit(ERROR_EXIT_STATUS);
    }

    for (i=0,j=0;;i++) {

	static double r = DOUBLE1;
	static long l = LONG1, l2 = LONG2;
	
	if (((i+1) % 1000) == 0) printf("Iteration %d\n",i+1);
	if ((n = sp_add_field(h,rsprintf("field1_%d",j),
			      T_INTEGER,(char *) &l)) >= 0){
	    if ((n = sp_add_field(h,rsprintf("field2_%d",j),
				  T_REAL,(char *) &r)) >= 0) {
		k = rand() % 35 + 1;
		strncpy(str_buf,str_cnt,k);
		str_buf[k] = '\0';
	        if ((n = sp_add_field(h,rsprintf("field3_%d",j),
				      T_STRING,str_buf)) < 0)
		    fprintf(spfp,"ADD of STRING field failed\n");
	    } else
		fprintf(spfp,"ADD of REAL field failed\n");
	} else
	    fprintf(spfp,"ADD of INTEGER field failed\n");
       
	if (n < 0) {
	    (void) fprintf(spfp,
		        "%s: iteration %d -- error adding fields to header\n",
			   prog,i);
	    (void) sp_print_lines(h,spfp);
	    exit(ERROR_EXIT_STATUS);
	}

	f1size = sizeof(long);
	f2size = sizeof(double);
	f3size = sizeof(STR1) - 1;
	if ((n = sp_get_data(h,rsprintf("field1_%d",j),buf,&f1size)) >= 0) {
	    if (*(long *)buf != LONG1){
		n = -1;
		if (n < 0) printf("Failed INTEGER value check %ld != %ld\n",
				  *(long *)buf,LONG1);
	    } else {
		if ((n=sp_get_data(h,rsprintf("field2_%d",j),buf,&f2size))>=0){
		    if (*(double *)buf != DOUBLE1) {
			n = -1;
			if (n < 0) printf("Failed REAL value check %f\n",
					  *(double *)buf);
		    } else {
			if ((n = sp_get_data(h,rsprintf("field3_%d",j),buf,
					     &f3size)) >= 0){
			    if (strncmp(buf,str_buf,f3size) != 0){
				n = -1;			
				if (n < 0) 
				    printf("Failed STRING value check %s\n",
					   buf);
			    }
			} else {
			    fprintf(spfp,"STRING Get failed\n");
			    n = -1;
			}
		    }
		} else {
		    fprintf(spfp,"DOUBLE Get failed\n");
		    n = -1;
		}
	    }
	} else {
	    fprintf(spfp,"INTEGER Get failed\n");
	    n = -1;
	}

	if (n < 0) {
	    (void) fprintf(spfp,
		       "%s: iteration %d -- error retrieving values\n",prog,i);
	    (void) fprintf(spfp,
			   "%s: retrived field '%s' string %s should be %s\n",
			   prog,rsprintf("field3_%d",j),buf,str_buf);
	    (void) sp_print_lines(h,spfp);
	    exit(ERROR_EXIT_STATUS);
	}
	
	if ((rand() % 10) == 5) {
	    n = sp_delete_field(h,rsprintf("field1_%d",j));
	    if (n >= 0) n = sp_delete_field(h,rsprintf("field2_%d",j));
	    if (n >= 0) n = sp_change_field(h,rsprintf("field3_%d",j),
					    T_INTEGER,(char *)&l2);
	    if (n >= 0) n = sp_delete_field(h,rsprintf("field3_%d",j));
	} else if ((rand () % 30) == 10) {
	    n =  sp_close_header(h);
	    h = sp_create_header();
	    if (h == HDRNULL) {
		(void) fprintf(spfp,"%s: Could not create header\n",prog);
		exit(ERROR_EXIT_STATUS);
	    }
	    j = 0;
	} else 
	    j++;

	if (n < 0) {
	    (void) fprintf(spfp,
		   "%s: iteration %d -- error deleting field(s) from header\n",
			   prog,i);
	    (void) sp_print_lines(h,spfp);
	    exit(ERROR_EXIT_STATUS);
	}
    }
}
