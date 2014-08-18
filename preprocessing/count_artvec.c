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

/**********************************************************************

  count_artvec.c

  Stefan Kaufmann, August 2000

  */

#include "count_artvec.h"
#include "model_params.h"
#include "preprocessing_env.h"

FILE *artvecbin;
DBM *a2o, *o2a;
int numDocs = 0; /* number of documents/articles in the corpus */

static int singvals = -1;

int transform_matrix( MATRIX_TYPE ** matrix, int rows, int columns); 

int main(int argc, char *argv[]) {
  
  FILE *FptrNumDocs;
  FILE *wordvecbin;
  ENVIRONMENT env;
  int i;
  char charbuf[BUFSIZ];
  char *model_data_dir;

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
    die( "count_artvec.c: can't read model params file\n" );
  }

  singvals = model_params.singvals;

  sprintf( charbuf, "%s/%s", model_data_dir, ARTVECBIN_FILE );
  if ( !my_fopen( &artvecbin, charbuf, "w" ))
    die( "Can't open artvec file.\n" );

  sprintf( charbuf, "%s/%s", model_data_dir, ART2OFFSET_FILE );
  if (( a2o = dbm_open( charbuf, O_RDWR | O_CREAT | O_TRUNC, 00644 )) 
      == NULL ) {
    fprintf( stderr, "Can't Open %s\n", charbuf );
    exit( EXIT_FAILURE );
  }
  
  sprintf( charbuf, "%s/%s", model_data_dir, OFFSET2ART_FILE );
  if (( o2a = dbm_open( charbuf, O_RDWR | O_CREAT | O_TRUNC, 00644 )) 
      == NULL ) {
    fprintf( stderr, "Can't Open %s\n", charbuf );
    exit( EXIT_FAILURE );
  }
  
  /*** read number of documents from file ***/
  sprintf( charbuf, "%s/%s", model_data_dir, FNUM_FILE );
  if ( !my_fopen( &FptrNumDocs, charbuf, "r" ))
    die( "couldn't open filenames file");

  if( !fscanf( FptrNumDocs, "%d", &numDocs ))
    die( "can't read numDocs" );

  if( !my_fclose( &FptrNumDocs ))
    die( "couldn't close numDocs file" );
  /*****/

  message( "Reading the dictionary... " );
  if( !read_dictionary( &(env.word_array), &(env.word_tree), model_data_dir ))
    die( "count_artvec.c: Can't read the dictionary.\n");
  
  /* Set some initial values in the matrix, arrays etc. */
  if( !initialize_row_indices( env.word_array, &(env.row_indices),
			       model_params.rows ))
    die( "Couldn't initialize row indices.\n");

  /* Allocate memory and set everything to zero.
     Defined in matrix.h */
  if( !initialize_matrix( (MATRIX_TYPE***) &(env.matrix), 
			  model_params.rows, model_params.singvals))
    die( "Can't initialize matrix.\n");


  /* Fill the matrix, using the numbers in WORDVECBIN_FILE */
  sprintf( charbuf, "%s/%s", model_data_dir, WORDVECBIN_FILE );
  if( !my_fopen( &wordvecbin, charbuf, "r"))
    die( "Couldn't open binary wordvector file.");

  printf ("count_artvec.c: about to read %d rows from wordvector file.\n",
	  model_params.rows);

  for( i=0; i < model_params.rows; i++)
    /* this line introduced to prevent choking on small corpora */
    if( env.word_array[i].row != -1) 
      if( !fread( (void *) (((MATRIX_TYPE **)
env.matrix)[env.word_array[i].row]),
		  (model_params.singvals * sizeof( MATRIX_TYPE)), 
		  1, wordvecbin))
	die( "Couldn't read binary wordvector file.");
  my_fclose( &wordvecbin);

  /* Go through the wordlist, applying process_region to all regions. */
  if( !process_wordlist( is_target, advance_target,
			 set_region_in, set_region_out, 
			 process_region , &env, model_data_dir ))
    die( "Couldn't process wordlist.\n" );
 
  my_fclose( &artvecbin);
  dbm_close( a2o);
  dbm_close( o2a);

  exit( EXIT_SUCCESS);
}



/**************************************************
  advance_target

  Get the next article if there is such a thing in the current buffer.
  This includes reading in a new buffer if necessary.

  ARGUMENTS:	1 buffer	the current wordlist chunk
		2 target	the current target word
		3 words_read	how many words are in the buffer?

  RETURN VALUES:	0 if we reached the end of the buffer
				AND the wordlist
			1 if all goes well
  **************************************************/
int advance_target( int *int_buffer, int words_read, int *target,
		    int *region_in, int *region_out) {
  int in_floc = FALSE;

  /* Targets are fileloc numbers. */
  while( ++(*target) <= words_read) {

    /* Floc end */
    if( int_buffer[*target] == INT_E_FLOC_TAG) {
      in_floc = FALSE;
      continue;
    }
    
    if( in_floc)
      return 1;

    /* Floc beginning */
    if( int_buffer[*target] == INT_B_FLOC_TAG)
      in_floc = TRUE;
  }
  return 0;
}



/**************************************************
  set_region_in

  ARGUMENTS:	int_buffer	the array holding the current
				chunk from the wordlist
		target		the word currently under
				scrutiny
		region_in	the index indicating the beginning
				of the region

  RETURN VALUES:	1
  */
int set_region_in( int *int_buffer, int words_read, int *target, 
		   int *region_in, int *region_out) {
  int in_text = FALSE;

  /* Set region_in on the first word (after the fileloc number) */
  *region_in = *target;
  while( ++(*region_in) <= words_read) {

    if( in_text)
      return 1;

    if( int_buffer[*region_in] == INT_B_TEXT_TAG)
      in_text = TRUE;
  }
  return 0;
}

/**************************************************
  set_region_out

  ARGUMENTS:	int_buffer	the array holding the current
				chunk from the wordlist
		words_read	the highest index to be used
				on the array
		target		the word currently under
				scrutiny
		region_out	the index indicating the end
				of the region

  RETURN VALUES:	1
  */
int set_region_out( int *int_buffer, int words_read, int *target, 
		    int *region_in, int *region_out) {
  for( *region_out = *region_in; *region_out <= words_read; (*region_out)++) {
    /* Stop at a end of text */
    if( int_buffer[*region_out] == INT_E_TEXT_TAG) {
      (*region_out)--;
      return 1;
    }
  }
  return 0;
}

/**************************************************
  process_region

  Given the beginning and end of a region and the target word
  within it, do whatever needs to be done (counting, most likely.)
  Keeps the variable 'cursor', which sweeps once between the
  region boundaries.

  ARGUMENTS:	1 *int_buffer	the array holding the chunk
				from the wordlist
		2 target	the index (in the buffer) of
				the word whose collocational
				behavior is under scrutiny
				(e.g. the middle between the 
				half-windows)
		3 region_in	the index of the beginning
		4 region_out	the index of the end
		5 env		the global structure holding
				the matrix and some other
				information.

  RETURN VALUES: 1
  **************************************************/
int process_region( int *int_buffer, int words_read, int target, 
		    int region_in, int region_out, ENVIRONMENT *env) {
  int cursor = region_in,
    row, i, bin_offset;
  datum key, content;
  MATRIX_TYPE *tmpvector;

  if ( (tmpvector = malloc( singvals * sizeof( MATRIX_TYPE ))) == NULL ) {
    fprintf( stderr, "count_artvec.c: process_region(): can't malloc "
	     "space for tmpvector\n" );
    exit(4);
  }

  /* Initialize the vector */
  clear_vector( tmpvector, singvals);
  
  /* Add the vectors up */
  while( cursor <= region_out) {
    /* If this is a row label... */
    if( ( row = ((env->word_array)[int_buffer[cursor]]).row) >= 0)
      for( i=0; i < singvals; i++)
	tmpvector[i] += (env->matrix)[row][i];
    cursor++;
  }
  
  /* Normalize the sum */
  normalize_vector( tmpvector, singvals );
  
  bin_offset = ftell( artvecbin);
  
  /* Write offset-to-art info */
  i = int_buffer[target];
  key.dptr	= (char*) &bin_offset;
  key.dsize	= sizeof( bin_offset);
  content.dptr	= (char*) &i;
  content.dsize	= sizeof( i);
  
  if( dbm_store( o2a, key, content, DBM_REPLACE) != 0) {
    perror( "offset2art");
    exit( EXIT_FAILURE);
  }
  
  /* Write art-to-offset info 
     (the article is identified by its fileloc, which target is set to) */
  key.dptr	= (char*) &i;
  key.dsize	= sizeof( i);
  content.dptr	= (char*) &bin_offset;
  content.dsize	= sizeof( bin_offset);
  
  if( dbm_store( a2o, key, content, DBM_REPLACE) != 0) {
    perror( "art2offset");
    exit( EXIT_FAILURE);
  }

  /* Write the article vector */
  if( !fwrite( (char*) tmpvector, singvals * sizeof(MATRIX_TYPE), 
	       1, artvecbin)) {
    perror( "artvecbin");
    exit( EXIT_FAILURE);
  }

  free( tmpvector );
  return 1;
}


/**************************************************
  is_target

  ARGUMENTS:	1 candidate	the numerical identifier of the word
		2 env		the structure holding information

  RETURN VALUES:	0 if the candidate is a row label
			1 if it is not.


  **************************************************/
int is_target( int candidate, ENVIRONMENT *env) {
  return 1;
}



/**************************************************
  term_weight

  This is used to put a weight on the co-occurrence count.
  The weight may depend only on the current column label (e.g. tf.idf),
  or on both it and the target (e.g. distance for window shape, gramma-
  tical relation etc.)

  Theoretically, the term weight should be thought of as being multi-
  plied with 1, the increase in count.

  ARGUMENTS:	1 buffer	the buffer holding the current
				wordlist chunk
		2 target	the current target (row label)
		3 cursor	the current cursor (column label)
		4 env		see above.

  RETURN_VALUES:	the weight.
  */
MATRIX_TYPE term_weight( int *buffer, int target, int cursor,
			 ENVIRONMENT *env) {

  int tf, df;
  double idf;

  /*return (MATRIX_TYPE) 1;*/ 

  /** use tf*idf of concepts (column labels) as term weight **/
  tf = env->word_array[buffer[cursor]].term_freq;
  df = env->word_array[buffer[cursor]].doc_freq;
  idf = log((double)numDocs) - log((double)df);

  return (MATRIX_TYPE) (tf*idf);
}


/**************************************************
  transform_matrix

  Perform any kind of transformation on the co-occurrence matrix
  before it is written out.
  Transformations may include some kind of vector normalization.
  Or simply taking the square root of all the numbers.

  ARGUMENTS:	1 matrix	The matrix
		2 rows		number of rows
		3 columns	number of columns

  RETURN VALUES:	1 
		only puts out warnings if the transformation fails.

  **************************************************/

int transform_matrix( MATRIX_TYPE ** matrix, int rows, int columns) {
  return 1;
}
