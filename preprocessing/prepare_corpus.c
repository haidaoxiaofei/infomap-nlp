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
  prepare_corpus.c

  converts a text corpus to usable format.
  Stefan Kaufmann, July 2000

  1.	Reads the corpus file by invoking functions in tokenizer.c
  2.	Writes the wordlist 
  3.	Writes the dictionary, sorted by frequency (descending)

  **************************************************/

#include "prepare_corpus.h"
#include "model_params.h"
#include <unistd.h>

/* Read the corpus, writing out the wordlist file (first version)
   and building the dictionary as you go along. */
int tokenize_corpus( NODE **word_tree, int *typecount, 
		     char *corpus_dir, char *model_data_dir,
		     char *corpus_filename, char *fnames_filename,
		     char *valid_chars_filename,
		     char *stoplist_filename );

/* Map the pointers of an array to the word nodes in the tree. */
void map_to_words( NODE **array, NODE *current, int *index);

/* For sorting with qsort(); compares word nodes based on frequency of word. */
int freqcmp();

int populate_corpus_format( CORPUS_FORMAT *corpus_format );

void usage_and_exit(char *prog_name, int exit_status);

/**************************************************
  Main.
  **************************************************/
int main(int argc, char *argv[]) {

  NODE *word_tree = NULL;	/* the root of the word tree */
  NODE **node_array;		/* the array pointing to tree
				   nodes, used for qsort */
  int i;			/* counters */
  int typecount = 0;		/* counting the types
				   thus the number of dictionary
				   entries */
  char *corpus_dir, *model_data_dir, *corpus_filename, *fnames_filename;
  char *stoplist_filename, *reports_filename, *valid_chars_filename;

  if ( argc != 15 ) usage_and_exit( argv[0], 1 );
  if ( strcmp( "-cdir", argv[1] ) != 0 ) usage_and_exit( argv[0], 1 );
  corpus_dir = argv[2];
  if ( strcmp( "-mdir", argv[3] ) != 0 ) usage_and_exit( argv[0], 1 );
  model_data_dir = argv[4];
  if ( strcmp( "-cfile", argv[5] ) != 0 ) usage_and_exit( argv[0], 1 );
  corpus_filename = argv[6];
  if ( strcmp( "-fnfile", argv[7] ) != 0 ) usage_and_exit( argv[0], 1 );
  fnames_filename = argv[8];
  if ( strcmp( "-chfile", argv[9] ) != 0 ) usage_and_exit( argv[0], 1 );
  valid_chars_filename = argv[10];
  if ( strcmp( "-slfile", argv[11] ) != 0 ) usage_and_exit( argv[0], 1 );
  stoplist_filename = argv[12];
  if ( strcmp( "-rptfile", argv[13] ) != 0 ) usage_and_exit( argv[0], 1 );
  reports_filename = argv[14];


  /* Initialize the reporting facility. */
  init_report( reports_filename );

  /* Read the corpus and send the words to processing. */
  if (! tokenize_corpus( &word_tree, &typecount, corpus_dir, model_data_dir,
			 corpus_filename, fnames_filename, valid_chars_filename, stoplist_filename ))
    exit(EXIT_FAILURE);

  sprintf( message_buffer, "Typecount = %d\n", typecount );
  message( message_buffer );

  /* Allocate space for the array used in sorting. */
  message( "Preparing to sort ... " );
  if( ( node_array = (NODE**) malloc( typecount * sizeof(NODE *) ) ) == NULL )
    die( "Couldn't build the word array.\n");
  
  /* Line up the wordinfo structures in node_array. */
  i = 0;
  map_to_words( node_array, word_tree, &i );

  /* Sort the words by frequency. */
  set_report( report_sorting );
  message( "Sorting ... ");
  qsort( (char*) node_array, typecount, sizeof( NODE*), &freqcmp);
  message( "Done.\n");

  /* Write the dictionary. */
  if( !write_dictionary( node_array, typecount, model_data_dir ))
    die( "Couldn't write dictionary entry.\n");
 
  if( !write_model_data( model_data_dir, corpus_dir,
			 corpus_filename, fnames_filename, typecount ) )
    die( "couldn't write model data" );


  exit( EXIT_SUCCESS);

}

/**************************************************
  FUNCTIONS
  **************************************************/


/**************************************************
  tokenize_corpus()

  Go through the corpus, gathering the dictionary and
  converting the corpus to a binary file of integers.
  The integers are assigned arbitrarily, as the types are
  encountered.  The will be converted to frequency scores later.

  Arguments:	1 word_tree		the root of the tree to
					store the dictionary
		2 typecount		counting the types
					thus the size of the dictionary.

  Return values:	1
  */
int tokenize_corpus( NODE **word_tree, int *typecount, 
		     char *corpus_dir, char *model_data_dir,
		     char *corpus_filename, char *fnames_filename,
		     char *valid_chars_filename,
		     char *stoplist_filename ) {
  
  FILE *wordlist_file;		/* wordlist file */
  NODE *nodeptr;		/* a movable pointer to
				   traverse the tree */
  WORDINFO *wordinfoptr;	/* pointing to wordinfo */
  char in_word[MAXWORDLEN];	/* buffer holding the strings
				   returned from next_token */
  char charbuf[BUFSIZ];
  int i, j, curr_pos;

  extern int CORPUS_TYPE;

  /** following variables are only used in the case of a corpus consisting of several files 
      (i.e. CORPUS_TYPE=MANY_FILES) **/
  DBM *nu2na;                   /* hash table - key: index of file, value: name of the file */  
  DBM *na2nu;                   /* hash table - key: name of the file, value: index of file */  
  char wdir[BUFSIZ];
  extern int numFiles;
  extern FILE *FptrCurrent;     /* points to corpus file currently processed */
  extern char **fnameslist;
  /*****/

  datum key, content;

  FILE *FptrNumDocs;   /* points to file which will only contains the number of docs in corpus; 
                           is needed in both prepare_corpus.c and count_wordvec.c 
                           and therefore written to file */

  /* Have the tokenizer open the file etc. */
  if( !initialize_tokenizer( corpus_filename, fnames_filename, valid_chars_filename ))
    die( "Couldn't initialize tokenizer.");

  /* Read the stoplist etc. */
  if( !initialize_stoplist( stoplist_filename ))
    die( "Couldn't initialize stoplist.");
 
  /* Prepare the output file. */
  sprintf( charbuf, "%s/%s", model_data_dir, WORDLIST_FILE );
  if( !my_fopen( &wordlist_file, charbuf, "w"))
    die( "Can't open wordlist file.\n");

  word_cntr = doc_cntr = curr_pos = 0;
  set_report( report_reading);
    
  if( CORPUS_TYPE == MANY_FILES) {
    sprintf( charbuf, "%s/%s", model_data_dir, NUMBER2NAME_FILE );
    if( ( nu2na = dbm_open( charbuf, O_RDWR | O_CREAT | O_TRUNC, 00644 )) 
         == NULL) {
      die("Can't open nu2na database");
    }
    sprintf( charbuf, "%s/%s", model_data_dir, NAME2NUMBER_FILE);
    if( ( na2nu = dbm_open( charbuf, O_RDWR | O_CREAT | O_TRUNC, 00644 ))
         == NULL) {
      die("Can't open na2nu database");
    }

    if ( getcwd(wdir, BUFSIZ) == NULL ) {
      perror("prepare_corpus.c: can't get current dir" );
      return 0;
    }
    
    if ( chdir(corpus_dir) < 0 ) {
      perror("prepare_corpus.c: can't chdir to corpus dir" );
      return 0;
    }
  }

 for( j=0; j<numFiles; j++ ) {

  if ( !my_fopen( &FptrCurrent, fnameslist[j], "r"))
       die("can't open current corpus file\n");

  if( CORPUS_TYPE == MANY_FILES){
    doc_cntr++;

    key.dptr = (char*) &j;
    key.dsize = sizeof( j);
    content.dptr = (char *) fnameslist[j];
    content.dsize = strlen(fnameslist[j]);

    if ( dbm_store( nu2na, key, content, DBM_REPLACE) != 0) 
       die("nu2na");

    key.dptr = (char *) fnameslist[j];
    key.dsize = strlen(fnameslist[j]);
    content.dptr = (char*) &j;
    content.dsize = sizeof( j);

    if ( dbm_store( na2nu, key, content, DBM_REPLACE) != 0)
      die("na2nu");

    fprintf( wordlist_file, "%s\n%s\n%d\n%s\n%s\n", 
             WORDLIST_B_DOC_TAG,
             WORDLIST_B_FLOC_TAG, j/*fnameslist[j]*/, WORDLIST_E_FLOC_TAG,
             WORDLIST_B_TEXT_TAG);
  }

  /* Get words from the tokenizer. */
  
  while( ( i = next_token( in_word, &curr_pos))) {

      switch( i) {
      
	/*** these cases should only occur if CORPUS_TYPE=ONE_FILE ***/

      /* If i == INT_B_DOC_TAG, we're at a document beginning. */
      case INT_B_DOC_TAG:
        doc_cntr++;
        fprintf( wordlist_file, "%s\n%s\n%d\n%s\n",
                 WORDLIST_B_DOC_TAG,
                 WORDLIST_B_FLOC_TAG, curr_pos, WORDLIST_E_FLOC_TAG);
        break;

       /* If i == INT_E_DOC_TAG, we're at a document ending. */
      case INT_E_DOC_TAG:
        fprintf( wordlist_file, "%s\n", WORDLIST_E_DOC_TAG);
        break;

      /* If i == INT_B_TEXT_TAG, we're at a text beginning. */
      case INT_B_TEXT_TAG:
        fprintf( wordlist_file, "%s\n", WORDLIST_B_TEXT_TAG);
        break;

      /* If i == INT_E_TEXT_TAG, we're at a text ending. */
      case INT_E_TEXT_TAG:
        fprintf( wordlist_file, "%s\n", WORDLIST_E_TEXT_TAG);
        break;

	/*****/

      /* If it's a word... */
      default:
        word_cntr++;
        /* Put it in the tree. */
        if( ( nodeptr = insert_word( word_tree, in_word, *word_tree)) == NULL)
	  die( "Couldn't allocate node memory.\n");
        if( ( wordinfoptr = 
	      increment_data( (WORDINFO **) &(nodeptr->data))) == NULL)
	  die( "Wordinfo data not allocated.\n");
        /* Some updates on the 'wordinfo' part of the node in which the
	   current word ended up. */
        /* If we've seen this word for the first time... */
        if( wordinfoptr->term_freq == 1) {
	  wordinfoptr->is_stop = is_stop( in_word);
	  (*typecount)++;
        }
        /* If this is the first occurrence in the current document... */
        if( wordinfoptr->doc_last_seen != doc_cntr) {
	  wordinfoptr->doc_last_seen = doc_cntr;
	  wordinfoptr->doc_freq++;
        }
        /* Write the word. */
        fprintf( wordlist_file, "%s\n", in_word);

      } /* end of switch */

  } /* end of while */

  if( CORPUS_TYPE == MANY_FILES)
    fprintf( wordlist_file, "%s\n%s\n", 
             WORDLIST_E_TEXT_TAG, WORDLIST_E_DOC_TAG);
  if( !my_fclose( &FptrCurrent))
    die( "Can't close current corpus file.\n");
  
 } /* end of for */

 if (CORPUS_TYPE == MANY_FILES) {
   if ( chdir(wdir) < 0 ) {
     perror( "prepare_corpus.c: can't return to original working dir" );
     return 0;
   }
 }

  if( !my_fclose( &wordlist_file))
    die( "Can't close wordlist file.\n");
  
  sprintf( charbuf, "%s/%s", model_data_dir, FNUM_FILE );
  if( !my_fopen( &FptrNumDocs, charbuf, "w"))
     die("can't open numDocs file");

  if( !fprintf( FptrNumDocs, "%ld", doc_cntr))
    die( "couldn't write numDocs"); 

  if( !my_fclose( &FptrNumDocs))
    die( "Can't close numDocs file.\n"); 
  
  /*close_tokenizer();*/
  
  if( CORPUS_TYPE == MANY_FILES) {
    dbm_close( nu2na);
    dbm_close( na2nu);
  }
  
  return 1;
}

/**************************************************
  increment_data

  Not all nodes in the tree represent words.  If one does, 
  this function is applied to either newly create or update 
  the 'data' part of the node.

  ARGUMENTS:	1 current	The wordinfo pointer to be updated.

  RETURN VALUES:	NULL	if a new structure was needed but
				could not be allocated
			the address of the updated structure 
			otherwise.
			*/
WORDINFO *increment_data( WORDINFO **current) {
  if( *current == NULL) {
    if( ( *current = (WORDINFO *) malloc( sizeof( WORDINFO))) == NULL)
      return NULL;
    (*current)->term_freq = 0;
    (*current)->doc_freq = 0;
    (*current)->is_stop = 0;
    (*current)->doc_last_seen = -1;
    (*current)->index = 0;
  }

  (*current)->term_freq++;
  return *current;
}


/**************************************************
  map_to_words.

  Map the pointers of an array to the word nodes in the tree.
  Recursive on the tree.
  Sets pointers in the array to all nodes which have a freqency count >= 1.

  Arguments:	1 array:	an array of pointers to type NODE,
				as many as there are dictionary entries.
		2 current:	the current node in the tree.
		3 index:	the current index in the array.

  Return values:	none (void).
  */
void map_to_words( NODE **array, NODE *current, int *index) {

  /* If we are at a leaf */
  if( current == NULL)
    return;
  /* Go recursively through the daughters rightdtr, next, and leftdtr. */
  map_to_words( array, current->rightdtr, index);
  if( current->data != NULL)
    array[(*index)++] = current;
  map_to_words( array, current->next, index);
  map_to_words( array, current->leftdtr, index);

  return;
}



/**************************************************
  freqcmp.

  For sorting.
  Passed to qsort as the comparison function betwee nodes.

  Arguments:	1 left		pointer to one node
		2 right		pointer to the other node

  Return values:	0 if both have the same frequency
			positive if right's term_freq is higher than left's
			negative if left's term_freq is higher than right's
			*/
int freqcmp( NODE **left, NODE **right) {
  return( ( ((WORDINFO *) (*right)->data)->term_freq 
	    - ((WORDINFO *) (*left)->data)->term_freq));
}


/**************************************************
  write_dictionary

  Write the dictionary to the dictionary file.

  ARGUMENTS:	1 NODE		The node array pointing to nodes in
				the word tree
		2 typecount     the numbers of types (elements of
				node_array)

  RETURN VALUES:	0 if writing fails
			1 otherwise.

  **************************************************/

int write_dictionary( NODE **node_array, int typecount, char *model_data_dir ) {
  FILE *outfile;
  int i;
  WORDINFO *wordinfoptr;
  char char_buffer[MAXWORDLEN];
  char pathbuf[BUFSIZ];

  sprintf( pathbuf, "%s/%s", model_data_dir, DIC_FILE );
  if( !my_fopen( &outfile, pathbuf, "w"))
    die( "Couldn't open dictionary file.\n");

  for( i=0; i<typecount; i++) {
    wordinfoptr = (WORDINFO *) (node_array[i])->data;
    if( ( fprintf( outfile, "%d %d %d ", 
		   wordinfoptr->term_freq, 
		   wordinfoptr->doc_freq,
		   wordinfoptr->is_stop) < 0)
	||
	( fprintf( outfile, "%s\n", 
		   print_string( char_buffer, node_array[i])) < 0)) {
      perror( "Writing dictionary entry");
      return 0;
    }
  }

  if( !my_fclose( &outfile))
    die( "Couldn't close dictionary file.\n");
  return 1;
}



/**************************************************
  STATUS REPORTS.
**************************************************/
void report_reading( char *buffer ) {
  sprintf( buffer, 
	   "prepare_corpus: Creating wordlist, %ld words, %ld documents.\n", 
	   word_cntr, doc_cntr );
}

void report_sorting( char *buffer ) {
  sprintf( buffer, "prepare_corpus: Sorting dictionary.\n" );
}

/****************************************
  WRITE MODEL DATA
*****************************************/

int write_model_data( const char *model_data_dir, const char *corpus_dir,
		      const char *corpus_file, const char *fnames_file,
		      int typecount ) {
  MODEL_PARAMS model_params;
  MODEL_INFO model_info;
  CORPUS_FORMAT corpus_format;

  extern int CORPUS_TYPE;
  char pathbuf[BUFSIZ];

  model_params.corpus_type = CORPUS_TYPE;
  strcpy( model_params.corpus_dir, corpus_dir );  
  if ( model_params.corpus_type == ONE_FILE ) {
    strcpy( model_params.corpus_filename, corpus_file );
  } else if ( model_params.corpus_type == MANY_FILES ) {
    strcpy( model_params.corpus_filename, fnames_file );
  }
  model_params.rows = typecount;
  model_params.singvals = -1;

  sprintf( pathbuf, "%s/%s", model_data_dir, MODEL_PARAMS_BIN_FILE );
  if ( !write_model_params( pathbuf, &model_params )) {
    fprintf( stderr, "prepare_corpus.c: Can't write model params file.\n" );
    return 0;
  }

  model_info.columns = -1;
  model_info.start_columns = -1;
  model_info.pre_context_size = -1;
  model_info.post_context_size = -1;
  model_info.blocksize = -1;
  model_info.svd_iter = -1;

  sprintf( pathbuf, "%s/%s", model_data_dir, MODEL_INFO_BIN_FILE );
  if ( !write_model_info( pathbuf, &model_info )) {
    fprintf( stderr, "prepare_corpus.c: Can't write model info file.\n" );
    return 0;
  }

  if ( !populate_corpus_format( &corpus_format )) {
    fprintf( stderr, "prepare_corpus.c: Couldn't create CORPUS_FORMAT "
	     "object.\n" );
    return 0;
  }
  sprintf( pathbuf, "%s/%s", model_data_dir, CORPUS_FORMAT_DATA_FILE );
  if ( !write_corpus_format( pathbuf, &corpus_format )) {
    fprintf( stderr, "prepare_corpus.c: Can't write corpus format file.\n" );
    free_corpus_format( &corpus_format );
    return 0;
  }

  free_corpus_format( &corpus_format );
  return 1;
}

int populate_corpus_format( CORPUS_FORMAT *corpus_format ) {

  int num_ignore, i;
  char *ignore_text[] = IGNORE_TXT;
  
  corpus_format->b_doc_tag = NULL;
  corpus_format->e_doc_tag = NULL;
  corpus_format->b_text_tag = NULL;
  corpus_format->e_text_tag = NULL;
  corpus_format->ignore_text = NULL;
  corpus_format->wordlist_b_doc_tag = NULL;
  corpus_format->wordlist_e_doc_tag = NULL;
  corpus_format->wordlist_b_text_tag = NULL;
  corpus_format->wordlist_e_text_tag = NULL;
  corpus_format->wordlist_b_floc_tag = NULL;
  corpus_format->wordlist_e_floc_tag = NULL;

  if ( (corpus_format->b_doc_tag = malloc( strlen(B_DOC_TAG) + 1 )) == NULL ) {
    fprintf( stderr, "prepare_corpus.c: Can't malloc b_doc_tag.\n" );
    free_corpus_format( corpus_format );
    return 0;
  }
  strcpy( corpus_format->b_doc_tag, B_DOC_TAG );

  if ( (corpus_format->e_doc_tag = malloc( strlen(E_DOC_TAG) + 1 )) == NULL ) {
    fprintf( stderr, "prepare_corpus.c: Can't malloc e_doc_tag.\n" );
    free_corpus_format( corpus_format );
    return 0;
  }
  strcpy( corpus_format->e_doc_tag, E_DOC_TAG );

  if ( (corpus_format->b_text_tag = 
	malloc( strlen(B_TEXT_TAG) + 1 )) == NULL ) {
    fprintf( stderr, "prepare_corpus.c: Can't malloc b_text_tag.\n" );
    free_corpus_format( corpus_format );
    return 0;
  }
  strcpy( corpus_format->b_text_tag, B_TEXT_TAG );
  
  if ( (corpus_format->e_text_tag = 
	malloc( strlen(E_TEXT_TAG) + 1 )) == NULL ) {
    fprintf( stderr, "prepare_corpus.c: Can't malloc e_text_tag.\n" );
    free_corpus_format( corpus_format );
    return 0;
  }
  strcpy( corpus_format->e_text_tag, E_TEXT_TAG );

  for ( i = 0; ignore_text[i] != NULL; i++ )
    ;
  num_ignore = i + 1;

  if ( (corpus_format->ignore_text =
	malloc( (i+1) * sizeof(char *) )) == NULL ) {
    fprintf( stderr, "prepare_corpus.c: Can't malloc ignore_text array.\n" );
    free_corpus_format( corpus_format );
    return 0;
  }

  for ( i = 0; i < num_ignore; i++ ) {
    corpus_format->ignore_text[i] = NULL;
  }

  for ( i = 0; i < num_ignore - 1; i++ ) {
    if ( (corpus_format->ignore_text[i] =
	  malloc( strlen(ignore_text[i]) + 1 )) == NULL ) {
      fprintf( stderr, "prepare_corpus.c: Can't malloc "
	       "ignore_text string.\n" );
      free_corpus_format( corpus_format );
      return 0;
    }
    strcpy( corpus_format->ignore_text[i], ignore_text[i] );
  }

  if ( (corpus_format->wordlist_b_doc_tag =
	malloc( strlen(WORDLIST_B_DOC_TAG) + 1 )) == NULL ) {
    fprintf( stderr, "prepare_corpus.c: Can't malloc "
	     "wordlist_b_doc_tag.\n" );
    free_corpus_format( corpus_format );
    return 0;
  }
  strcpy( corpus_format->wordlist_b_doc_tag, WORDLIST_B_DOC_TAG );

  if ( (corpus_format->wordlist_e_doc_tag =
	malloc( strlen(WORDLIST_E_DOC_TAG) + 1 )) == NULL ) {
    fprintf( stderr, "prepare_corpus.c: Can't malloc "
	     "wordlist_e_doc_tag.\n" );
    free_corpus_format( corpus_format );
    return 0;
  }
  strcpy( corpus_format->wordlist_e_doc_tag, WORDLIST_E_DOC_TAG );
  
  if ( (corpus_format->wordlist_b_text_tag = 
	malloc( strlen(WORDLIST_B_TEXT_TAG) + 1 )) == NULL ) {
    fprintf( stderr, "prepare_corpus.c: Can't malloc "
	     "wordlist_b_text_tag.\n" );
    free_corpus_format( corpus_format );
    return 0;
  }
  strcpy( corpus_format->wordlist_b_text_tag, WORDLIST_B_TEXT_TAG );

  if ( (corpus_format->wordlist_e_text_tag =
	malloc( strlen(WORDLIST_E_TEXT_TAG) + 1 )) == NULL ) {
    fprintf( stderr, "prepare_corpus.c: Can't malloc "
	     "wordlist_e_text_tag.\n" );
    free_corpus_format( corpus_format );
    return 0;
  }
  strcpy( corpus_format->wordlist_e_text_tag, WORDLIST_E_TEXT_TAG );

  if ( (corpus_format->wordlist_b_floc_tag =
	malloc( strlen(WORDLIST_B_FLOC_TAG) + 1 )) == NULL ) {
    fprintf( stderr, "prepare_corpus.c: Can't malloc "
	     "wordlist_b_floc_tag.\n" );
    free_corpus_format( corpus_format );
    return 0;
  }
  strcpy( corpus_format->wordlist_b_floc_tag, WORDLIST_B_FLOC_TAG );

  if ( (corpus_format->wordlist_e_floc_tag =
	malloc( strlen(WORDLIST_E_FLOC_TAG) + 1 )) == NULL ) {
    fprintf( stderr, "prepare_corpus.c: Can't malloc "
	     "wordlist_e_floc_tag.\n" );
    free_corpus_format( corpus_format );
    return 0;
  }
  strcpy( corpus_format->wordlist_e_floc_tag, WORDLIST_E_FLOC_TAG );
  
  return 1;

}

void usage_and_exit(char *prog_name, int exit_status) {
  fprintf( stderr, "Usage: %s -cdir <corpus_dir> -mdir <model_data_dir>\n"
	   "\t-cfile <corpus_filename> -fnfile <file_names_filename>\n"
	   "\t-chfile <valid_chars_filename> -slfile <stoplist_filename>\n"
	   "-rptfile <reports_filename>\n", prog_name );
  fprintf( stderr, "\nNote arguments must appear in this exact order.\n" );

  exit( exit_status );
}
