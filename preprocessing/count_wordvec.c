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

  count_wordvec.c

  Goes through an integer-encoded corpus file and compiles
  the co-occurrence vector for all words in the dictionary.

  1. Reads the dictionary file to determine which words are
     row and column labels.

  2. Fills the arrays row_indices and col_indices to map row and
     column indices to the words that label them.
     row_indices[j] is the index in env.word_array of the word
     that labels row j.  If row_indices[j] = i, then 
     env.word_array[i].row = j.

  3. Invokes functions in process_wordlist to read chunks from the
     wordlist file and process regions in them.

  4. For each row label in the corpus, information is extracted
     by (i) building a "region" around it, and (ii) extracting 
     collocational information from that region.  The "region" may
     be, and currently is, a window determined by word count.
     This can be changed easily to use other text units, such
     as sentences, as long as they are detectable in the wordlist
     (which they currently are not.)  The window shape (weight of the
     words in the window) is set in the function term_weight.

  5. Writes out the cooccurrence matrix in a format digestible for
     svdinterface (column-major).

  Stefan Kaufmann, August 2000

*/

#include "count_wordvec.h"
#include "matrix.h"
#include "model_params.h"
#include "preprocessing_env.h"

int transform_matrix( MATRIX_TYPE ** matrix, int rows, int columns );
void usage_and_exit( char *prog_name, int exit_status );

int numDocs = 0; /* number of documents (articles) in the corpus */

static int pre_context_size;
static int post_context_size;

int main(int argc, char *argv[]) {
  FILE *FptrNumDocs;
  ENVIRONMENT env;
  char *model_data_dir;
  char pathbuf[BUFSIZ];
  char *col_label_file;
  int write_matlab;
  int col_labels_from_file;

  int rows, columns;
  MODEL_PARAMS model_params;
  MODEL_INFO model_info;

  env.word_array = NULL;
  env.word_tree = NULL;

  if ( argc != 17 ) usage_and_exit( argv[0], 1 );
  if ( strcmp( argv[1], "-mdir" ) != 0 ) usage_and_exit( argv[0], 1 );
  model_data_dir = argv[2];
  if ( strcmp( argv[3], "-matlab" ) != 0 ) usage_and_exit( argv[0], 1 );
  write_matlab = atoi( argv[4] );
  if ( strcmp( argv[5], "-precontext" ) != 0 ) usage_and_exit( argv[0], 1 );
  pre_context_size = atoi( argv[6] );
  if ( strcmp( argv[7], "-postcontext" ) != 0 ) usage_and_exit( argv[0], 1 );
  post_context_size = atoi( argv[8] );
  if ( strcmp( argv[9], "-rows" ) != 0 ) usage_and_exit( argv[0], 1 );
  rows = atoi( argv[10] );
  if ( strcmp( argv[11], "-columns" ) != 0 ) usage_and_exit( argv[0], 1 );
  columns = atoi( argv[12] );
  if ( strcmp( argv[13], "-col_labels_from_file" ) != 0 ) usage_and_exit( argv[0], 1 );
    col_labels_from_file = atoi( argv[14] );
  if ( strcmp( argv[15], "-col_label_file" ) != 0 ) usage_and_exit( argv[0], 1 );
    col_label_file = argv[16];  
  
  fprintf( stderr, "model data dir is \"%s\".\n", model_data_dir );

  /** Read in current model params **/
  sprintf( pathbuf, "%s/%s", model_data_dir, MODEL_PARAMS_BIN_FILE );
  if ( !read_model_params( pathbuf, &model_params )) {
    die( "count_wordvec.c: couldn't read model data file\n" );
  }

  sprintf( pathbuf, "%s/%s", model_data_dir, MODEL_INFO_BIN_FILE );
  if ( !read_model_info( pathbuf, &model_info )) {
    die( "count_wordvec.c: couldn't read model info file\n" );
  }

  if (model_params.rows < rows) {
    rows = model_params.rows;
  } else {
    model_params.rows = rows;
  }

  printf("count_wordvec.c: looking for %d rows\n", rows);
  printf("which had better match %d\n", model_params.rows);
  model_info.columns = columns;
  model_info.col_labels_from_file = col_labels_from_file;
  model_info.pre_context_size = pre_context_size;
  model_info.post_context_size = post_context_size;
  model_info.blocksize = BLOCKSIZE;
  model_info.start_columns = START_COLUMNS;

  message( "Reading the dictionary... ");
  if( !read_dictionary( &(env.word_array), &(env.word_tree), model_data_dir ))
    die( "count_wordvec.c: Can't read the dictionary.\n");

  /*** read number of ducuments from file ***/
  
  sprintf( pathbuf, "%s/%s", model_data_dir, FNUM_FILE );
  if ( !my_fopen( &FptrNumDocs, pathbuf, "r" ))
    die( "couldn't open filenames file" );

  if( !fscanf( FptrNumDocs, "%d", &numDocs ))
    die( "can't read numDocs" );

  if( !my_fclose( &FptrNumDocs ))
    die( "couldn't close numDocs file" );
  /*****/
  
  /* Set some initial values in the matrix, arrays etc. */
  if( !initialize_row_indices( env.word_array, &(env.row_indices), 
			       rows ))
    die( "Couldn't initialize row indices.\n");
  if( !initialize_column_indices( env.word_array, &(env.col_indices),
				  columns, col_labels_from_file, col_label_file, &(env.word_tree) ))
    die( "Couldn't initialize column indices.\n");

  /* Allocate memory and set everything to zero.
     Defined in matrix.h */
  if( !initialize_matrix( (MATRIX_TYPE***) &(env.matrix), rows, columns))
    die( "Can't initialize matrix.\n");

  /* Go through the wordlist, applying process_region to all regions. */
  fprintf( stderr, "model data dir is \"%s\".\n", model_data_dir );
  fprintf( stderr, "count_wordvec.c: about to call process_wordlist\n" );
  if( !process_wordlist( is_target, advance_target,
			 set_region_in, set_region_out, 
			 process_region , &env, model_data_dir))
    die( "Couldn't process wordlist.\n");

  /* Perform some conversion on the matrix.
     E.g. some kind of normalization.  We traditionally take the square root
     of all entries. */
  if( !transform_matrix( (MATRIX_TYPE **) (env.matrix), rows, columns))
    die( "Couldn't transform matrix.\n");
  
  /* Write the co-occurrence matrix. */
  message( "Writing the co-occurrence matrix.\n");

  if( !write_matrix_svd((MATRIX_TYPE **) (env.matrix),
                         rows, columns, model_data_dir ))
    die( "count_wordvec.c: couldn't write co-occurrence "
	 "matrix in SVD input format.\n" );

  if ( write_matlab ) {
    if ( !write_matrix_matlab( (MATRIX_TYPE **)(env.matrix),
			       rows, columns, model_data_dir ))
      die( "count_wordvec.c: couldn't write co-occurrence "
	   "matrix in Matlab input format.\n" );

  }

  sprintf( pathbuf, "%s/%s", model_data_dir, MODEL_PARAMS_BIN_FILE );
  if ( !write_model_params( pathbuf, &model_params )) {
    die( "count_wordvec.c: couldn't write model params file\n" );
  }

  sprintf( pathbuf, "%s/%s", model_data_dir, MODEL_INFO_BIN_FILE );
  if ( !write_model_info( pathbuf, &model_info )) {
    die( "count_wordvec.c: couldn't write model info file\n" );
  }

  exit( EXIT_SUCCESS);
}



/**************************************************
  advance_target

  Get the next word if there is such a thing in the current buffer.
  This includes reading in a new buffer if necessary.

  ARGUMENTS:	1 buffer	the current wordlist chunk
		2 target	the current target word
		3 words_read	how many words are in the buffer?

  RETURN VALUES:	0 if we reached the end of the buffer
				AND the wordlist
			1 if all goes well
  **************************************************/
int advance_target( int *buffer, int words_read, int *target) {
  int in_text = TRUE;

  while( ++(*target) <= words_read) {
    /* Tags.  (Need to be careful not to step on the fileloc number!) */
    if( buffer[*target] < 0 || buffer[(*target)+1] == INT_E_FLOC_TAG)
      in_text = FALSE;
    
    if( in_text) {
      return 1;
    }
    
    /* Beginning of text */
    if( buffer[*target] == INT_B_TEXT_TAG)
      in_text = TRUE;
  }
  
  return 0;
}



/**************************************************
  set_region_in

  Given a target word (as an index in the buffer holding the
  wordlist chunk), find the beginning of the region around it.
  Counts backwards to halfwindow size, stopping at boundaries.
  Could also disregard any fixed size, for counting within
  documents, phrases etc.

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
  for( *region_in = *target; *region_in >= 0; (*region_in)--) {
    /* Beginning of text */
    if( *region_in > 0 && int_buffer[(*region_in)-1] == INT_B_TEXT_TAG)
      return 1;
    /* Left half window */
    if( ( *target - *region_in) >= pre_context_size)
      return 1;
  }
  fprintf( stderr, "Warning: No B_TEXT_TAG at beginning of buffer!\n");
  return 1;
}

/**************************************************
  set_region_out

  Given the target word, finds the end of the region.
  Pretty much parallel to set_region_in.

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
  for( *region_out = *target; *region_out <= words_read; (*region_out)++) {
    /* End of text */
    if( int_buffer[*region_out+1] == INT_E_TEXT_TAG)
      return 1;
    /* Right half window */
    if( ( *region_out - *target) >= post_context_size)
      return 1;
  }
  fprintf( stderr, "Warning: No E_TEXT_TAG at end of buffer!\n");
  return 1;
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
		    int region_in, int region_out, 
		    ENVIRONMENT *env) {
  int row, column, cursor;
  MATRIX_TYPE **matrix = (MATRIX_TYPE **) env->matrix;

  /* Which row are we in? 
     (We know it's a row label, otherwise we wouldn't be here) - 
     that has been checked by 'is_target' */
  row = ((env->word_array)[int_buffer[target]]).row;

  for( cursor = region_in; cursor <= region_out; cursor++) {
    /* If it's the middle of the window... */
    if( cursor == target)
      continue;
    /* If it's not a column label... */

    if( ( column = ((env->word_array)[int_buffer[cursor]]).col) == -1)
      continue;
    /*if (!((column < COLUMNS)&&(column>=0))){
      printf("col:%d int_buf:%d\n", column, int_buffer[cursor]); 
      printf("region_in:%d cursor:%d region_out:%d\n", region_in, cursor, region_out);   
    }*/
    /* Otherwise, increment the cooccurrence count. */
    /*printf("%d %d\n", row, column);*/
    matrix[row][column] += term_weight( int_buffer, target, cursor, env);
  }
  return 1;
}

/**************************************************
  is_target

  Check whether the input word is one for which we want a word vector.
  Identifies the row labels.

  ARGUMENTS:	1 candidate	the numerical identifier of the word
		2 env		the structure holding information

  RETURN VALUES:	1 if the candidate is a row label
			0 if it is not.


  **************************************************/
int is_target( int candidate, ENVIRONMENT *env) {
  /* If the target is not a row label... */
  if( (env->word_array[candidate]).row == -1)
    return 0;
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
  extern int numDocs;


  /** use tf*idf of concepts (column labels) as term weight **/
  tf = env->word_array[buffer[cursor]].term_freq;
  df = env->word_array[buffer[cursor]].doc_freq;

  idf = log((double)numDocs + (double)1) - log((double)df);

  return (MATRIX_TYPE) (tf*idf);
    
  /*return (MATRIX_TYPE) 1;*/
  
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
  int i,j;
  
  errno = 0;
  for( i=0; i<rows; i++)
    for( j=0; j<columns; j++) { 
      matrix[i][j] = (MATRIX_TYPE) sqrt( (double) matrix[i][j]);
      if( errno)
	perror( "Transforming matrix for output");
    }
  
  return 1;
}


void usage_and_exit(char *prog_name, int exit_status) {
  fprintf( stderr, "Usage: %s -mdir <model_data_dir>"
	   " -matlab <write_matlab?>"
	   " -precontext <pre_context_size>"
	   " -postcontext <post_context_size>"
	   " -rows <num_rows>"
	   " -columns <num_columns>"
	   " -col_labels_from_file <read_col_labels_from_file?>"
	   " -col_label_file <file_containing_col_labels>"
	   "\n",
	   prog_name );

  exit( exit_status );

}
