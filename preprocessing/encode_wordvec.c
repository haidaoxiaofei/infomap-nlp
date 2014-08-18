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
/**************************************************
  encode_wordvec

  Read the word vectors stored in a text file and write
  them to a binary file, creating two hash files to look up

  (i)  the string, given the location of its vector in the
       binary file (offset2word)

  (ii) the offset in the binary file, given the string
       (word2offset)

  both (i) and (ii) can be used e.g. in calculating 
  word similarity etc.

  Stefan Kaufmann, April 2001

  **************************************************/

#include "encode_wordvec.h"
#include "utils.h"
#include "model_params.h"

int get_next_vector( FILE *vecfile, MATRIX_TYPE *vector, int singvals);

int main(int argc, char *argv[]) {
  ENVIRONMENT env;
  FILE *vecfile, *binfile /*, *singvalfile */;
  DBM  *w2o, *o2w;
  datum key, content;
  int row, rows_done, bin_offset;
  char charbuf[BUFSIZ];
  MATRIX_TYPE *vector;
  int vectorsize;
  /* MATRIX_TYPE singvals[SINGVALS]; */
  char *model_data_dir;

  /* Declared in dict.c */
  extern int DIC_SIZE;

  MODEL_PARAMS model_params;

  env.word_array = NULL;
  env.word_tree = NULL;

  if ( argc != 3 ) {
    fprintf( stderr, "Usage: %s -m <model_data_dir>\n", argv[0] );
    exit( 1 );
  }
  model_data_dir = argv[2];

  sprintf( charbuf, "%s/%s", model_data_dir, MODEL_PARAMS_BIN_FILE );
  if ( !read_model_params( charbuf, &model_params )) {
    die( "encode_wordvec.c: Can't read model params file.\n" );
  }

  vectorsize = model_params.singvals * sizeof(MATRIX_TYPE);

  if ( (vector = malloc( vectorsize ))
       == NULL) {
    die( "encode_wordvec.c: can't malloc() space for vector.\n" );
  }
       

  sprintf( charbuf, "%s/%s", model_data_dir, WORDVEC_FILE );
  if( !my_fopen( &vecfile, charbuf, "r" )) {
    fprintf( stderr,"Can't Open %s\n", charbuf );
    exit( EXIT_FAILURE );
  }

  sprintf( charbuf, "%s/%s", model_data_dir, WORD2OFFSET_FILE );
  if( ( w2o = dbm_open( charbuf, O_RDWR | O_CREAT | O_TRUNC, 00644 )) 
      == NULL) {
    fprintf( stderr, "Can't Open %s\n", charbuf );
    perror( "Failed dbm_open()" );
    exit( EXIT_FAILURE );
  }
  
  sprintf( charbuf, "%s/%s", model_data_dir, OFFSET2WORD_FILE );
  if( ( o2w = dbm_open( charbuf, O_RDWR | O_CREAT | O_TRUNC, 00644 )) 
      == NULL ) {
    fprintf( stderr, "Can't Open %s\n", charbuf );
    exit( EXIT_FAILURE );
  }

  sprintf( charbuf, "%s/%s", model_data_dir, WORDVECBIN_FILE );
  if ( !my_fopen( &binfile, charbuf, "w" )) {
    fprintf( stderr, "Can't Open %s\n", charbuf );
    exit( EXIT_FAILURE );
  }

  /* Read the singular values. */
  /** For now we don't use the singular values for anything, 
   *  so don't bother reading them.
   */
  /**
  sprintf( charbuf, "%s/%s", model_data_dir, SINGVAL_FILE );
  if( !my_fopen( &singvalfile, charbuf, "r" )) {
    fprintf( stderr, "Can't Open %s\n", charbuf );
    exit( EXIT_FAILURE );
  }
  
  message( "Reading singular values...\n");
  if( !read_vector( singvalfile, singvals, SINGVALS ))
    die( "Can't read singval file.\n");
  my_fclose( &singvalfile);
  **/

  /* Read the dictionary. */
  message( "Reading the dictionary...\n");
  if( !read_dictionary( &(env.word_array), &(env.word_tree), model_data_dir ))
    die( "encode_wordvec.c: Can't read the dictionary.\n");

  /* We might be better off keeping track of the row indices the
   * first time we initialize them, rather than having to repeat 
   * it hear. */
  if( !initialize_row_indices( env.word_array, &(env.row_indices),
			       model_params.rows ))
    die( "Can't initialize word indices.\n");

  bin_offset = 0;
  for( row=0, rows_done=0; 
       (row < DIC_SIZE) && (rows_done <= model_params.rows); 
       row++) {
    /* If this is a row label in the matrix... */
    if( env.word_array[row].row != -1) {
      /* Get the vector */
      switch( get_next_vector( vecfile, vector, model_params.singvals )) {
	/* If there was an error */
      case 0:
	die( "Error reading vector file.\n");
	
	/* If the 'natural' EOF was reached */
      case -1:
	fprintf( stderr, "Vector file done.  %d vectors processed.\n", 
		 rows_done);
	exit( EXIT_SUCCESS );
	
	/* If we have a vector */
      case 1:
	
	/* Get the string. */
	print_string( charbuf, (env.word_array[row]).nodeptr);
	/* Write word-to-offset info */
	key.dptr	= charbuf;
	key.dsize	= strlen( charbuf);
	content.dptr	= (char*) &bin_offset;
	content.dsize	= sizeof( bin_offset);
	
	if( dbm_store( w2o, key, content, DBM_REPLACE) != 0) {
	  perror( "encode_wordvec.c: attempted dbm_store to word2offset");
	  exit( EXIT_FAILURE );
	}
	
	/* Write offset-to-word info */
	key.dptr	= (char*) &bin_offset;
	key.dsize	= sizeof( bin_offset);
	content.dptr	= charbuf;
	content.dsize	= strlen( charbuf);
	
	if( dbm_store( o2w, key, content, DBM_REPLACE) != 0) {
	  perror( "encode_wordvec.c: attempted dbm_store to offset2word");
	  exit( EXIT_FAILURE );
	}
	
	
	/* Scale the vector if you want to

	   Commented out by Dominic, June 2001, because the first
	   principal component only really seems to say that all the
	   data is in the positive quadrant; declare i if you want to
	   use these two lines of code.  (Also, comment back in the
	   code to declare the singvals array and to open SINGVAL_FILE
	   and read its contents into that array.)

	for( i=0; i<SINGVALS; i++)
	vector[i] *= singvals[i]; */	  

	/* Normalize(?) */
	normalize_vector( vector, model_params.singvals );
	
	/* Write the vector */
	if( fwrite( (char*) vector, 
		     vectorsize,
		     1, binfile) != 1) {
	  perror( "wordvectorbin");
	  exit( EXIT_FAILURE );
	}
	bin_offset += vectorsize;
      }
      rows_done++;
    }
  }
  free( vector );
  dbm_close(w2o);
  dbm_close(o2w);
  return EXIT_SUCCESS;
}

int get_next_vector( FILE *vecfile, MATRIX_TYPE *vector, int singvals ) {
  /* Read the values */
  clearerr( vecfile);
  if( !read_vector( vecfile, vector, singvals)) {
    if( ferror( vecfile)) {
      perror( "reading vector file");
      return 0;
    }
    else
      return -1;
  }
  
  return 1;
}
