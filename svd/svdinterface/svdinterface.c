/*
Copyright (c) 2004, Stanford University
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.

    * Neither the name of Stanford University nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* ****************************************
   
   CALL LAS2
   
   Hinrich Schuetze, 1994
   
   **************************************** */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/types.h>
#include <sys/time.h>
#include "fixed.h"
#include "filenames.h"
#include "myutils.h"
#include "model_params.h"

void doit(void);
void usage_and_exit(char *prog_name, int exit_status);

FILE *singfile, *leftfile, *rghtfile;

int hashrow,hashcol,hashsing,hashstep;
char extension[MAXLINE];
float *sing;
float **left;
float **right;


int main( int argc, char *argv[] )
{
  MODEL_PARAMS model_params;
  MODEL_INFO model_info;

  fprintf(stderr,"\n  This is svdinterface.\n\n");
  if (argc != 5) usage_and_exit( argv[0], 2 );
  if ( strcmp( argv[1], "-singvals" ) != 0 )
    usage_and_exit( argv[0], 2 );
  hashsing = atoi( argv[2] );
  if ( strcmp( argv[3], "-iter" ) != 0 )
    usage_and_exit( argv[0], 2 );
  hashstep = atoi( argv[4] );

  if ( !read_model_params( MODEL_PARAMS_BIN_FILE, &model_params )) {
    fprintf( stderr, "svdinterface.c: can't read model params file.\n" );
    exit(3);
  }

  if ( !read_model_info( MODEL_INFO_BIN_FILE, &model_info )) {
    fprintf( stderr, "svdinterface.c: can't read model info file.\n" );
    exit(3);
  }

  model_params.singvals = hashsing;
  model_info.svd_iter = hashstep;

  hashrow = model_params.rows;
  hashcol = model_info.columns;

  sing = (float *) mymalloc(sizeof(float)*hashsing);
  left = Allocate_Float_Matrix(hashrow,hashsing);
  right = Allocate_Float_Matrix(hashcol,hashsing);
  doit(  );

  if ( !write_model_params( MODEL_PARAMS_BIN_FILE, &model_params )) {
    fprintf( stderr, "svdinterface.c: can't write model params file.\n" );
    exit(3);
  }

  if ( !write_model_info( MODEL_INFO_BIN_FILE, &model_info )) {
    fprintf( stderr, "svdinterface.c: can't write model info file.\n" );
    exit(3);
  }

  return(0);
}

void doit(void) {
  int n,i,hashcomp;
  openfile( WORDVEC_FILE, &leftfile );
  openfile( RIGHT_SINGVEC_FILE, &rghtfile );
  openfile( SINGVAL_FILE, &singfile );
  preparelandr( "", hashrow, hashstep, hashsing, left, right, sing, &hashcomp );
  if (hashcomp<hashsing)
    printf("\n  FEWER THAN EXPECTED SINGULAR VALUES\n");
  for (i=0; i<hashsing; i++)
    fprintf(singfile,"%f\n",sing[i]);
  fclose(singfile);
  for (n=0; n<hashrow; n++)
    for (i=0; i<hashsing; i++)
      fprintf(leftfile,"%f\n",left[n][i]);
  fclose(leftfile);
  for (n=0; n<hashcol; n++)
    for (i=0; i<hashsing; i++)
      fprintf(rghtfile,"%f\n",right[n][i]);
  fclose(rghtfile);
}


void usage_and_exit(char *prog_name, int exit_status) {
  fprintf(stderr, "Usage: %s " 
	  "-singvals <num_singvals> " 
	  "-iter <num_iter>\n", prog_name);

  exit( exit_status );
}



