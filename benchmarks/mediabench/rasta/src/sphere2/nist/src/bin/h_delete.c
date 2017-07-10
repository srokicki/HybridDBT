/* File: h_edit.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sp/sphere.h>
#include <util/hsgetopt.h>


int main (int argc, char **argv){
    return(wav_edit(argc, argv, "F", "D:F:fo:uv"));
}
