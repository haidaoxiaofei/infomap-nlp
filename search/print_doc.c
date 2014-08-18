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

/**
 *  print_doc.c
 *  -----------
 *     The print_doc program takes identification of
 *  a model (using TOC files and the -m option
 *  in the same way that associate does) and one
 *  or more  document ID's;
 *  it prints the corresponding documents on standard out.
 */

#include "associate.h"
#include "filenames.h"
#include "model_params.h"
#include "search_utils.h"
#include "env.h"
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>

/*** Function prototypes ***/
void usage_and_exit( char *prog_name, int exit_status );
void print_opt_req_arg( const char *opt );
/***************************/

int main( int argc, char *argv[] ) {

  /** string to print in between documents,
   *  if we are retrieving more than one. */
  char *doc_sep_string = "\n===\n\n";
  int num_docs_printed = 0;

  char *model_tag = NULL;
  char *model_data_dir = NULL;
  char *model_dir_parent = NULL;
  int use_working_dir = 0;
  int have_m_opt = 0;
  
  MODEL_PARAMS model_params;
  CORPUS_FORMAT corpus_format;

  FILE *corpus_file;
  off_t curr_doc_id;

  char *buf, curr_doc[BUFSIZ];
  int buf_size;
  char buf2[BUFSIZ];
  char pathbuf[BUFSIZ];
  char *tempbuf;

  if ( (buf = malloc(BUFSIZ)) == NULL) {
    fprintf( stderr, "Can't malloc() initial buffer.\n");
    exit(5);
  }
  buf_size = BUFSIZ;

  if (argc < 2)
    usage_and_exit( argv[0], 2 );

  while( (argc > 1) && (argv[1][0] == '-') ) {

    switch( argv[1][1] ) {

    case 't' : 
      use_working_dir = 1;
      break;

    case 'c' : 
      if (argc < 3) {
	print_opt_req_arg("-c");
	usage_and_exit( argv[0], 2 );
      }
      ++argv;
      --argc;
      model_tag = argv[1];
      break;

    case 'm' : 
      if (argc < 3) {
	print_opt_req_arg("-m");
	usage_and_exit( argv[0], 2 );
      }
      ++argv;
      --argc;
      model_dir_parent = argv[1];
      have_m_opt = 1;
      break;

    default : 
      fprintf( stderr, "\nUnknown option: %s\n\n", argv[1] );
      usage_and_exit( argv[0], 2 );

    } /* switch( argv[1][1] ) */

    /* Move the argument list ahead.
       The options don't count any longer as input terms.
       This works only if all options precede the input terms! */
    ++argv;
    --argc;
  
  } /* while( (argc > 1) && (argv[1][0] == '-') ) */

  if (model_dir_parent == NULL) {
    if (use_working_dir) {
      model_dir_parent = getenv(WORKING_DIR_VAR);
      if (model_dir_parent == NULL) {
	fprintf( stderr, "-t specified, but %s undefined.\n",
		 WORKING_DIR_VAR );
	exit( 2 );
      } 
    } else {
	if ( model_tag != NULL ) {
	  model_dir_parent = search_model_path( model_tag );
	  if (model_dir_parent == NULL) {
	    fprintf( stderr, "Could not find model with tag \"%s\" in "
		     "Infomap model path.\n", model_tag );
	    exit( 2 );
	  }
	}
    }
  }
  
  
  /** at this point we have the appropriate model_data_dir,
   *  however it was specified */

  if (model_tag != NULL) {
    sprintf( pathbuf, "%s/%s", model_dir_parent, model_tag );
    model_data_dir = strdup( pathbuf );
  }
  else {
    if (have_m_opt) {
      model_data_dir = model_dir_parent;
    }
  }
  sprintf( buf, "%s/%s", model_data_dir, MODEL_PARAMS_BIN_FILE );
  if ( !read_model_params( buf, &model_params )) {
    fprintf( stderr, "Can't read model params file \"%s\"", buf );
    exit(3);
  }

  sprintf( buf, "%s/%s", model_data_dir, CORPUS_FORMAT_DATA_FILE );
  if ( !read_corpus_format( buf, &corpus_format )) {
    fprintf( stderr, "Can't read corpus format file \"%s\"", buf );
    exit(3);
  }

  if (model_params.corpus_type == MANY_FILES) {
    /**
     *  in this case, we read entire files
     */
    
    while (argc > 1) {
      memcpy( curr_doc, argv[1], strlen(argv[1])); 
      ++argv;
      --argc;
      
      if ( (corpus_file = fopen( curr_doc, "r" )) == NULL ) {
	fprintf( stderr, "Can't open document file \"%s\".\n"
		 "Skipping.\n", curr_doc );
	continue;
      }
      if (num_docs_printed > 0) printf( "%s", doc_sep_string );
      while ( fgets( buf, buf_size, corpus_file ) != NULL ) {
	printf( "%s", buf );
      }
      num_docs_printed++;
      if ( ferror( corpus_file )) {
	perror("Error reading document file");
	fclose( corpus_file );
	continue;
      }
      fclose( corpus_file );
    }
    
  } else if (model_params.corpus_type == ONE_FILE) {
    /**
     *  in this case, we read from the offsets specified
     *  up to the end of doc tag.
     */
    if ( (corpus_file = fopen( model_params.corpus_filename, "r" )) ==
	 NULL ) {
      fprintf( stderr, "Can't open corpus file \"%s\".\n", 
	       model_params.corpus_filename );
      exit(3);
    }

    while (argc > 1) {
      curr_doc_id = atol(argv[1]);
      ++argv;
      --argc;

      if ( fseek( corpus_file, curr_doc_id, SEEK_SET ) != 0 ) {
	fprintf( stderr, "Can't seek to document id %ld "
		 "in corpus file.\n"
		 "Skipping this document.\n", curr_doc_id);
	continue;
      }
      
      if (num_docs_printed > 0) printf( "%s", doc_sep_string );
      while ( fgets( buf, buf_size, corpus_file ) != NULL ) {
	printf( "%s", buf);
	sprintf( buf2, "%s\n", corpus_format.e_doc_tag );
	if ( strcmp(buf, buf2) == 0) break;
      }

      num_docs_printed++;
      if (ferror(corpus_file)) {
	perror("Error reading corpus file.\n");
	exit(3);
      }
      
    }
    fclose(corpus_file);
    free_corpus_format( &corpus_format );

  } else {
    fprintf( stderr, "Unrecognized corpus type in model params object.\n" );
    exit(4);
    
  }
  exit(0);
}


void usage_and_exit( char *prog_name, int exit_status ) {

  FILE *out_stream;

  if ( exit_status == 0 )
    out_stream = stdout;
  else 
    out_stream = stderr;

  fprintf( out_stream, "Usage: %s ( [-t] | [-m model_data_dir] ) "
	   "-c <model_tag> "
	   "<doc_id_1> [doc_id_2 ... doc_id_n]\n",
	   prog_name );

  exit( exit_status );

}

void print_opt_req_arg( const char *opt ) {
  fprintf( stderr, "\nThe %s option must be followed by "
	   "a space and an argument.\n", opt );
}

