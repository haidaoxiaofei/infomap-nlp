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

#include "model_params.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void scancfparam( FILE *fp, char **field, const char *name );

/**
 *  Return 1 for success; 0 for failure
 */
int write_model_params(const char *filename, 
		       const MODEL_PARAMS *model_params) {
  size_t num_written;
  FILE *fp;
  
  fp = fopen( filename, "w" );
  if ( fp == NULL ) {
    perror( "model_params.c: write_model_params: can't open model params file" );
    return 0;
  }

  num_written = fwrite( model_params, sizeof( MODEL_PARAMS ), 1, fp );
  if ( num_written != 1 ) {
    perror( "model_param.c: write_model_params: write error" );
    return 0;
  }

  fclose( fp );  /* ignore errors */
  return 1;
}

/**
 *  1 indicates success; 0 failure.
 */
int read_model_params(const char *filename, MODEL_PARAMS *model_params) {
  size_t num_read;
  struct stat stats;
  FILE *fp;

  /** Before reading, we should check the size of the model params file
   *  to make sure it equals the size of a MODEL_PARAMS structure. If the
   *  definition of that structure has changed since the model params file
   *  was written, we run the risk of undefined behavior if we don't do this
   *  check.
   *
   *  This also goes to show that we should generate a text version of the
   *  parameters also, just in case.
   */  
  if ( stat( filename, &stats ) < 0 ) {
    perror( "model_params.c: read_model_params: can't stat model params file" );
    return 0;
  }
  
  if (stats.st_size != sizeof( MODEL_PARAMS )) {
    fprintf( stderr, "model_params.c: read_model_params: "
	     "Size of model params file \"%s\" doesn't match "
	     "size of MODEL_PARAMS structure type.\n"
	     "Has the MODEL_PARAMS type definition changed since "
	     "this file was written?\n", filename );
    return 0;
  }
  
  fp = fopen( filename, "r" );
  if ( fp == NULL ) {
    perror( "model_params.c: read_model_params: can't open model params file" );
    return 0;
  }

  num_read = fread( model_params, sizeof( MODEL_PARAMS ), 1, fp );
  if ( num_read != 1 ) {
    perror( "model_params.c: read_model_params: read error" );
    return 0;
  }

  fclose( fp );  /* ignore errors */
  return 1;

}

void print_model_params(FILE *fp, const MODEL_PARAMS *model_params,
			const MODEL_INFO *model_info,
			const CORPUS_FORMAT *corpus_format) {

  int i;

  if ( model_params != NULL ) {
    fprintf( fp, "\n" );
    fprintf( fp, "corpus type = " );
    if ( model_params->corpus_type == ONE_FILE ) {
      fprintf( fp, "one file\n" );
    } else if ( model_params->corpus_type == MANY_FILES ) {
      fprintf( fp, "many files\n" );
    } else {
      fprintf( fp, "undefined\n" );
    }
    
    fprintf( fp, "corpus dir = \"%s\"\n", model_params->corpus_dir );
    if ( model_params->corpus_type == ONE_FILE )
      fprintf( fp, "corpus file = \"" );
    else
      fprintf( fp, "filenames file = \"" );

    fprintf( fp, "%s\"\n", model_params->corpus_filename );
    fprintf( fp, "rows = %d\n", model_params->rows );
    fprintf( fp, "singvals = %d\n", model_params->singvals );
    
  }
   
  if ( model_info != NULL ) {
    fprintf( fp, "\n" );
    fprintf( fp, "columns = %d\n", model_info->columns );
    fprintf( fp, "start columns = %d\n", model_info->start_columns );
    fprintf( fp, "col_labels_from_file = %d\n", model_info->col_labels_from_file );
    fprintf( fp, "pre context size = %d\n", model_info->pre_context_size );
    fprintf( fp, "post context size = %d\n", model_info->post_context_size );
    fprintf( fp, "blocksize = %d\n", model_info->blocksize );
    fprintf( fp, "svd iterations = %d\n", model_info->svd_iter );
  }

  if ( corpus_format != NULL ) {
    fprintf( fp, "\n" );
    fprintf( fp, "document beginning tag = \"%s\"\n", 
	     corpus_format->b_doc_tag );
    fprintf( fp, "document ending tag = \"%s\"\n",
	     corpus_format->e_doc_tag );
    fprintf( fp, "text beginning tag = \"%s\"\n",
	     corpus_format->b_text_tag );
    fprintf( fp, "text ending tag = \"%s\"\n",
	     corpus_format->e_text_tag );

    fprintf( fp, "ignored text:" );
    for ( i = 0; corpus_format->ignore_text[i] != NULL; i++ ) {
      fprintf( fp, " %s", corpus_format->ignore_text[i] );
    }
    fprintf( fp, "\n" );

    fprintf( fp, "wordlist document beginning tag = \"%s\"\n",
	     corpus_format->wordlist_b_doc_tag );
    fprintf( fp, "wordlist document ending tag = \"%s\"\n",
	     corpus_format->wordlist_e_doc_tag );
    fprintf( fp, "wordlist text beginning tag = \"%s\"\n",
	     corpus_format->wordlist_b_text_tag );
    fprintf( fp, "wordlist text ending tag = \"%s\"\n",
	     corpus_format->wordlist_e_text_tag );
    fprintf( fp, "wordlist file location beginning tag = \"%s\"\n",
	     corpus_format->wordlist_b_floc_tag );
    fprintf( fp, "wordlist file location ending tag = \"%s\"\n",
	     corpus_format->wordlist_e_floc_tag );
    
  }
  
}

int write_model_info(const char *filename, const MODEL_INFO *model_info) {

  size_t num_written;
  FILE *fp;

  fp = fopen( filename, "w" );
  if ( fp == NULL ) {
    perror( "model_params.c: write_model_info: can't open model info file" );
    return 0;
  }

  num_written = fwrite( model_info, sizeof( MODEL_INFO ), 1, fp );
  if ( num_written != 1 ) {
    perror( "model_params.c: write_model_info: write error" );
    return 0;
  }

  fclose( fp );  /* ignore errors */
  return 1;  
}

int read_model_info(const char *filename, MODEL_INFO *model_info) {

  size_t num_read;
  struct stat stats;
  FILE *fp;

  /** Before reading, we should check the size of the model params file
   *  to make sure it equals the size of a MODEL_INFO structure. If the
   *  definition of that structure has changed since the model params file
   *  was written, we run the risk of undefined behavior if we don't do this
   *  check.
   *
   *  This also goes to show that we should generate a text version of the
   *  parameters also, just in case.
   */  
  if ( stat( filename, &stats ) < 0 ) {
    perror( "model_params.c: read_model_info: can't stat model info file" );
    return 0;
  }
  
  if (stats.st_size != sizeof( MODEL_INFO )) {
    fprintf( stderr, "model_params.c: read_model_info: "
	     "Size of model info file \"%s\" doesn't match "
	     "size of MODEL_INFO structure type.\n"
	     "Has the MODEL_INFO type definition changed since "
	     "this file was written?\n", filename );
    return 0;
  }
  
  fp = fopen( filename, "r" );
  if ( fp == NULL ) {
    perror( "model_params.c: read_model_info: can't open model info file" );
    return 0;
  }

  num_read = fread( model_info, sizeof( MODEL_INFO ), 1, fp );
  if ( num_read != 1 ) {
    perror( "model_params.c: read_model_info: read error" );
    return 0;
  }

  fclose( fp );  /* ignore errors */
  return 1;

  
}


int write_corpus_format(const char *filename, 
			const CORPUS_FORMAT *corpus_format) {
  
  FILE *fp;
  int i;

  if ( (fp = fopen( filename, "w" )) == NULL ) {
    perror( "model_params.c: write_corpus_format(): can't open file" );
    return 0;
  }

  fprintf( fp, "B_DOC_TAG: %s\n", corpus_format->b_doc_tag );
  fprintf( fp, "E_DOC_TAG: %s\n", corpus_format->e_doc_tag );
  fprintf( fp, "B_TEXT_TAG: %s\n", corpus_format->b_text_tag );
  fprintf( fp, "E_TEXT_TAG: %s\n", corpus_format->e_text_tag );

  for ( i = 0; corpus_format->ignore_text[i] != NULL; i++ )
    ;

  fprintf( fp, "IGNORE_TXT: %d\n", i );

  for ( i = 0; corpus_format->ignore_text[i] != NULL; i++ ) 
    fprintf( fp, "IGNORE_TXT: %s\n", corpus_format->ignore_text[i] );

  fprintf( fp, "WORDLIST_B_DOC_TAG: %s\n", 
	   corpus_format->wordlist_b_doc_tag );
  fprintf( fp, "WORDLIST_E_DOC_TAG: %s\n", 
	   corpus_format->wordlist_e_doc_tag );
  fprintf( fp, "WORDLIST_B_TEXT_TAG: %s\n", 
	   corpus_format->wordlist_b_text_tag );
  fprintf( fp, "WORDLIST_E_TEXT_TAG: %s\n", 
	   corpus_format->wordlist_e_text_tag );
  fprintf( fp, "WORDLIST_B_FLOC_TAG: %s\n", 
	   corpus_format->wordlist_b_floc_tag );
  fprintf( fp, "WORDLIST_E_FLOC_TAG: %s\n", 
	   corpus_format->wordlist_e_floc_tag );

  fclose( fp );
  return 1;
}


int read_corpus_format(const char *filename, CORPUS_FORMAT *corpus_format) {

  /**
   *  We expect to see the fields making up this
   *  structure written one string per line, with
   *  the ignore_text field being written on multiple
   *  lines and preceded by a line indicating the
   *  number of ignore_text entries to follow.
   */
  
  FILE *fp;
  int num_ignore_text;
  int i;
  int scanfret;

  fp = fopen( filename, "r" );
  if ( fp == NULL ) {
    perror( "model_params.c: read_corpus_format(): can't open file" );
    return 0;
  }

  scancfparam( fp, &(corpus_format->b_doc_tag), "B_DOC_TAG" );
  scancfparam( fp, &(corpus_format->e_doc_tag), "E_DOC_TAG" );
  scancfparam( fp, &(corpus_format->b_text_tag), "B_TEXT_TAG" );
  scancfparam( fp, &(corpus_format->e_text_tag), "E_TEXT_TAG" );

  scanfret = fscanf( fp, "IGNORE_TXT: %d\n", &num_ignore_text );
  if ( scanfret != 1 ) {
    fprintf( stderr, "model_params.c: read_corpus_format(): "
	    "error reading IGNORE_TXT number spec.\n" );
  } else {
    if ( (corpus_format->ignore_text = malloc( (num_ignore_text + 1) *
					       sizeof(char *) )) == NULL) {
      fprintf( stderr, "model_params.c: read_corpus_format(): "
	       "can't malloc() ignore text memory\n" );
      return 0;
    }
    for ( i = 0; i <= num_ignore_text; i++ ) {
      corpus_format->ignore_text[i] = NULL;
    }

    for ( i = 0; i < num_ignore_text; i++ ) {
      scancfparam( fp, &(corpus_format->ignore_text[i]), "IGNORE_TXT" );
    }
  }

  scancfparam( fp, &(corpus_format->wordlist_b_doc_tag), "WORDLIST_B_DOC_TAG" );
  scancfparam( fp, &(corpus_format->wordlist_e_doc_tag), "WORDLIST_E_DOC_TAG" );
  scancfparam( fp, &(corpus_format->wordlist_b_text_tag), "WORDLIST_B_TEXT_TAG" );
  scancfparam( fp, &(corpus_format->wordlist_e_text_tag), "WORDLIST_E_TEXT_TAG" );
  scancfparam( fp, &(corpus_format->wordlist_b_floc_tag), "WORDLIST_B_FLOC_TAG" );
  scancfparam( fp, &(corpus_format->wordlist_e_floc_tag), "WORDLIST_E_FLOC_TAG" );

  fclose( fp );
  return 1;
}

void scancfparam( FILE *fp, char **field, const char *name ) {
  int scanfret;
  char scanfbuf[BUFSIZ];
  char formatbuf[BUFSIZ];

  sprintf( formatbuf, "%s: %%s\n", name );

  scanfret = fscanf( fp, formatbuf, scanfbuf );

  if ( scanfret != 1 ) {
    fprintf( stderr, "model_params.c: error reading %s.\n", name );
    fprintf( stderr, "scanfret = %d, but should be 1.\n", scanfret );
    return;
  }
  
  *field = strdup( scanfbuf );
  return;
}

void free_corpus_format(CORPUS_FORMAT *corpus_format) {

  int i;

  if ( corpus_format->b_doc_tag != NULL ) 
    free( corpus_format->b_doc_tag );

  if ( corpus_format->e_doc_tag != NULL )
    free( corpus_format->e_doc_tag );

  if ( corpus_format->b_text_tag != NULL )
    free( corpus_format->b_text_tag );
  
  if ( corpus_format->e_text_tag != NULL )
    free( corpus_format->e_text_tag );
  
  if ( corpus_format->ignore_text != NULL ) {
    for ( i = 0; corpus_format->ignore_text[i] != NULL; i++ ) {
      free( corpus_format->ignore_text[i] );
    }
    free( corpus_format->ignore_text );
  }

  if ( corpus_format->wordlist_b_doc_tag != NULL )
    free( corpus_format->wordlist_b_doc_tag );

  if ( corpus_format->wordlist_e_doc_tag != NULL )
    free( corpus_format->wordlist_e_doc_tag );

  if ( corpus_format->wordlist_b_text_tag != NULL )
    free( corpus_format->wordlist_b_text_tag );

  if ( corpus_format->wordlist_e_text_tag != NULL )
    free( corpus_format->wordlist_e_text_tag );

  if ( corpus_format->wordlist_b_floc_tag != NULL )
    free( corpus_format->wordlist_b_floc_tag );

  if ( corpus_format->wordlist_e_floc_tag != NULL ) 
    free( corpus_format->wordlist_e_floc_tag );
}










