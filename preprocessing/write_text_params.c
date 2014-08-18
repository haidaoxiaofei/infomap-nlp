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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filenames.h"
#include "model_params.h"

void usage_and_exit( char *prog_name, int exit_status );

int main( int argc, char *argv[] ) {

  char *model_data_dir;
  char pathbuf[BUFSIZ];
  
  FILE *outfile;

  MODEL_PARAMS model_params;
  MODEL_INFO model_info;
  CORPUS_FORMAT corpus_format;

  int have_error = 0;

  if ( argc != 3 ) {
    usage_and_exit( argv[0], 1 );
  }
  if ( strcmp( argv[1], "-mdir" ) != 0 ) {
    usage_and_exit( argv[0], 1 );
  }
  model_data_dir = argv[2];

  sprintf( pathbuf, "%s/%s", model_data_dir, MODEL_PARAMS_BIN_FILE );
  if ( !read_model_params( pathbuf, &model_params )) {
    fprintf( stderr, "write_text_params: can't read model params bin file "
	     "\"%s\".\n", pathbuf );
    have_error++;
  } else {
    sprintf( pathbuf, "%s/%s", model_data_dir, MODEL_PARAMS_TEXT_FILE );
    if (( outfile = fopen( pathbuf, "w" )) == NULL ) {
      perror( "write_text_params: opening model params text file" );
      have_error++;
    } else {
      print_model_params( outfile, &model_params, NULL, NULL );
      fclose( outfile );
    }
  }

  sprintf( pathbuf, "%s/%s", model_data_dir, MODEL_INFO_BIN_FILE );
  if ( !read_model_info( pathbuf, &model_info )) {
    fprintf( stderr, "write_text_params: can't read model info bin file "
	     "\"%s\".\n", pathbuf );
    have_error++;
  } else {
    sprintf( pathbuf, "%s/%s", model_data_dir, MODEL_INFO_TEXT_FILE );
    if (( outfile = fopen( pathbuf, "w" )) == NULL ) {
      perror( "write_text_params: opening model info text file" );
      have_error++;
    } else {
      print_model_params( outfile, NULL, &model_info, NULL );
      fclose( outfile );
    }
  }

  sprintf( pathbuf, "%s/%s", model_data_dir, CORPUS_FORMAT_DATA_FILE );
  if ( !read_corpus_format( pathbuf, &corpus_format )) {
    fprintf( stderr, "write_text_params: can't read corpus format bin file "
	     "\"%s\".\n", pathbuf );
    have_error++;
  } else {
    sprintf( pathbuf, "%s/%s", model_data_dir, CORPUS_FORMAT_TEXT_FILE );
    if (( outfile = fopen( pathbuf, "w" )) == NULL ) {
      perror( "write_text_params: opening corpus format text file" );
      have_error++;
    } else {
      print_model_params( outfile, NULL, NULL, &corpus_format );
      fclose( outfile );
    }
  }

  exit( have_error );
}

void usage_and_exit( char *prog_name, int exit_status ) {
  fprintf( stderr, "Usage: %s -mdir <model_data_dir>\n", prog_name );
  exit( exit_status );
}

