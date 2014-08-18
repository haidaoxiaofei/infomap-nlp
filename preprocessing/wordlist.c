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
  wordlist

  Contains functions
  (i) read wordlist material into a buffer in memory
  (ii) go through the words in that buffer
  (iii) create "regions" around those words.

  "Regions" are contexts.  They may be defined by a fixed-size window,
  by document boundaries, or grammatical relations.  Currently only
  the fixed window approach is implemented.  Other approaches would
  require additional tagging of the wordlist, which the functions here
  are currently not sensitive to.

  Stefan Kaufmann, August 2000

**************************************************/

#include "wordlist.h"
#include "preprocessing_env.h"
#include <stdio.h>
#include <stdlib.h>

int initialize_wordlist( int *buffer, int *words_read, int *target, char *model_data_dir );
int close_wordlist( void );
int read_datablock( int *int_buffer, NODE *word_tree );

FILE *wordlist_file;


/**************************************************
  initialize_wordlist
  close_wordlist

  Open and close the wordlist file.
  The pointer to the open file is kept in the global variable
  "wordlist_file", declared in wordlist.h

  ARGUMENTS:	1 buffer	the buffer holding input chunks 
				from the wordlist
		2 words_read	how many words are in the buffer
		3 target	the index of the "target" word

  RETURN VALUES:	1 if all goes well;
			0 otherwise.
  **************************************************/
int initialize_wordlist( int *buffer, int *words_read, int *target, char *model_data_dir ) {
  char charbuf[BUFSIZ];

  sprintf( charbuf, "%s/%s", model_data_dir, WORDLIST_FILE );
  if( !my_fopen( &wordlist_file, charbuf, "r" ))
    return 0;
  *words_read = *target = 0;
  return 1;
}
/**************************************************/
int close_wordlist() {
  if( !my_fclose( &wordlist_file))
    return 0;
  return 1;
}
/**************************************************/


/**************************************************
  process_wordlist

  Load large chunks of the wordlist by reading the words in the file
  and storing their index (= position in the order by frequency) in
  the file int_buffer.  When stopping in mid-document, chop off the 
  beginning of the incomplete argument and reset the file pointer so
  that next time it is read again.

  Go through the buffer, identifying each target word (row label),
  building a region around around it and processing the region.
  The functions for setting regions are supplied by the calling program.

  ARGUMENTS:	1 is_target		the characteristic function 
					of the row labels
		2 advance_target	move on to the next target
		3 set_region_in		set the beginning of the region
		4 set_region_out	set the end of the region
		5 process_region	process the region
		6 env			the structure holding some
					information

  RETURN VALUES:	

  **************************************************/
int process_wordlist( int (*is_target)(), int (*advance_target)(),
		      int (*set_region_in)(),
		      int (*set_region_out)(), int (*process_region)(), 
		      ENVIRONMENT *env, char *model_data_dir ) {

  int *int_buffer;		/* the block holding chunks of
					   the wordlist */
  int words_read;			/* keeps track of how many words
					   are in int_buffer */
  int target_idx, region_in, region_out;/* used to delimit the region
					   around the target word */

  if ( (int_buffer = malloc(BLOCKSIZE * sizeof(int))) == NULL) {
    fprintf( stderr, "wordlist.c: process_wordlist: can't malloc "
	     "buffer for wordlist chunks\n" );
    return 0;
  }
  fprintf( stderr, "Entering process_wordlist.\n" );
  fprintf( stderr, "About to call initialize_wordlist.\n" );

  /* Open the wordlist. */
  if( !initialize_wordlist( int_buffer, &words_read, &target_idx, model_data_dir ))
    die( "Couldn't initialize wordlist.\n");
  fprintf( stderr, "Returned from initialize_wordlist.\n" );
  
  while( ( words_read = read_datablock( int_buffer, env->word_tree))) {
    target_idx = 0;
    while( advance_target( int_buffer, words_read, &target_idx)) {
      if( is_target( int_buffer[target_idx], env)) {
	/* Create a region around the target and process it. */
	set_region_in ( int_buffer, words_read, &target_idx, 
			&region_in, &region_out);
	set_region_out( int_buffer, words_read, &target_idx, 
			&region_in, &region_out);
	process_region( int_buffer, words_read, target_idx, 
			region_in, region_out, env);
      }
  }
}
  /* Close the wordlist. */
  if( !close_wordlist())
    die( "Couldn't close wordlist.\n");
  
  free( int_buffer );
  return 1;
}




/**************************************************
  read_datablock

  Read BLOCKSIZE entries from the wordlist file into 
  the buffer.  Then cuts off the end to discard partial
  documents.  If the current document is longer than
  BLOCKSIZE words, we will run into an infinite loop!

  ARGUMENTS:	int_buffer	the array holding the
				current wordlist chunk.
		word_tree	the word tree

  RETURN VALUES:	0 if there was a reading problem
			  or we hit EOF
			1 if all went well.
			*/
int read_datablock( int *int_buffer, NODE *word_tree) {

  int 
    in_text = FALSE,
    in_doc = FALSE,
    in_floc = FALSE,
    i;  
  char buffer[MAXWORDLEN];
  int last_doc_beg = 0;

  /* int_buffer has BLOCKSIZE-1 integer slots.
     It's one less than BLOCKSIZE because in certain cases
     we want to read one additional item in the loop. */
  for( i=0; i<BLOCKSIZE-1; i++) {
    if( fscanf( wordlist_file, "%s", buffer) == EOF) {
      /* If we haven't read anything */
      if( i == 0 )
	return 0;
      /* Otherwise, process what we've read 
	 Since we haven't read anyone, i is already one plus the
	 the highest index. That's why we don't need i+1 as below. */
      else
	return i;
    }

    /* If we're at a document end */
    if( strcmp( buffer, WORDLIST_E_DOC_TAG ) == 0 ) {
      in_doc = FALSE;
      int_buffer[i] = INT_E_DOC_TAG;
      /* Remember the position in case we can't fit the next document
	 in the remainder of the buffer */
      last_doc_beg = ftell( wordlist_file );
      continue;
    }

    /* If we're at a document beginning, read the file location too */
    if( strcmp( buffer, WORDLIST_B_DOC_TAG ) == 0 ) {
      in_doc = TRUE;
      int_buffer[i] = INT_B_DOC_TAG;
      continue;
    }

    /* If we're at a text end */
    if( strcmp( buffer, WORDLIST_E_TEXT_TAG ) == 0 ) {
      in_text = FALSE;
      int_buffer[i] = INT_E_TEXT_TAG;
      continue;
    }
    
    /* If we're at a text beginning */
    if( strcmp( buffer, WORDLIST_B_TEXT_TAG ) == 0 ) {
      in_text = TRUE;
      int_buffer[i] = INT_B_TEXT_TAG;
      continue;
    }

    /* If we're at the floc number */
    if( strcmp( buffer, WORDLIST_B_FLOC_TAG ) == 0 ) {
      in_floc = TRUE;
      int_buffer[i++] = INT_B_FLOC_TAG;
      if( fscanf( wordlist_file, "%d", &(int_buffer[i])) == EOF) {
	message( "Fileloc number missing after document beginning\n");
      }
      continue;
    }

    /* If we're at the end of the floc number */
    if( strcmp( buffer, WORDLIST_E_FLOC_TAG ) == 0 ) {
      in_floc = FALSE;
      int_buffer[i] = INT_E_FLOC_TAG;
      continue;
    }
    
    /* Default... */
    int_buffer[i] = word_index( buffer, word_tree );
  }
  
  /* Reset the file pointer to the beginning of the last document and
     look for the last document beginning in the buffer. */
  while( int_buffer[i] != INT_B_DOC_TAG)
    i--;
  fseek( wordlist_file, last_doc_beg, SEEK_SET);

  return( i+1);
}








