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
  dict.c

  Functions for making dictionary information available to programs.

  Stefan Kaufmann, January 2001

  **************************************************/

#include "dict.h"
#include "utils.h"
#include "preprocessing_env.h"

/**************************************************
  read_dictionary

  Load all the information in the dictionary into memory.
  Words are stored in a binary tree (see word_tree.c).  Their
  data are stored in the array word_array, holding WORDINFO
  structures (cf. env.h)

  ARGUMENTS:	1 word_array	the array holding the WORDINFO
				structures with data from the 
				dictionary
		2 word_tree	the binary tree holding the
				words (in NODE structures) with
				pointers to the corresponding 
				elements in word_array

  RETURN VALUES:	0 if the dictionary could not be opened,
				closed or read,
				holds fewer than DIC_SIZE entries,
				or memory could not be allocated
			1 if all goes well

  **************************************************/

/* The number of types in the dictionary.  
   This is not compiled as a constant because the dictionary size
   is unknown until the dictionary is actually written.
   But I want the software to be fully compileable before 
   any of it is parts are run.  Call it excentric.
   DIC_SIZE is measured in 'read_dictionary' be counting
   the number of newlines (similarly to the shell command 'wc')
*/
int DIC_SIZE=0; 

int read_dictionary( WORDINFO **wordinfo_array, NODE **word_tree, 
		     char *model_data_dir ) {
  NODE *nodeptr;
  FILE *dict_file;
  int i;
  char buffer[MAXWORDLEN+1];
  char error_message[BUFSIZ];
  char pathbuf[BUFSIZ];

  /* Find out how many entries there are in the dictionary */
  sprintf( pathbuf, "%s/%s", model_data_dir, DIC_FILE );
  if( !my_fopen( &dict_file, pathbuf, "r")) {
    fprintf( stderr, "dict.c: read_dictionary(): can't open dict file \"%s\".\n",
	     pathbuf );
    return 0;
  }

  while( ( i = getc( dict_file)) != EOF)
    if( i == '\n')
      DIC_SIZE++;

  /* Rewind the file. */
  rewind( dict_file);

  if( ( *wordinfo_array = (WORDINFO *) malloc( DIC_SIZE * sizeof( WORDINFO))) 
      == NULL) {
    fprintf( stderr, "dict.c: read_dictionary(): " 
	     "can't allocate space for word info array.\n" );
    return 0;
  }
  
  for( i=0; i<DIC_SIZE; i++) {
    if( fscanf( dict_file, "%d %d %d %s",
		&((*wordinfo_array)[i].term_freq), 
		&((*wordinfo_array)[i].doc_freq),
		&((*wordinfo_array)[i].is_stop), 
		buffer) == EOF) {
      sprintf( error_message, "Premature EOF in dictionary file at line %d.\n", i);
      die(error_message);
    }

    if( ( (*wordinfo_array)[i].string = (char *) malloc( (strlen(buffer)+1) * sizeof(char)))
        == NULL)
      return 0;

    (*wordinfo_array)[i].index = i;
    strcpy( (*wordinfo_array)[i].string, buffer);
    
/* Store the string in the tree and record the index of its data */
    if( ( nodeptr = insert_word( word_tree, buffer, *word_tree)) == NULL)
      return 0;
    nodeptr->data = (void *) &((*wordinfo_array)[i]);
    (*wordinfo_array)[i].nodeptr = nodeptr;
    
    /* Set some other things to -1. These are later changed as needed. */
    (*wordinfo_array)[i].row = (*wordinfo_array)[i].col = -1;
    
  }
  
  if( !my_fclose( &dict_file))
    return 0;
  
  return 1;
}

/**************************************************
  initialize_row_indices(), 
  initialize_column_indices()

  Get information from the dictionary to assign row and column numbers
  to terms.

  ARGUMENTS:	1 word_array	Array of WORDINFO structures
				(read from the dictionary by
				read_dictionary)

                2 row_indices   Array of the row labels' indices in word_array

                3 num_rows      Number of rows for which to allocate space

  RETURN VALUES:	1   if everything went fine
                        0   if memory coudn't be allocated

  **************************************************/
int initialize_row_indices( WORDINFO *word_array, int **row_indices,
			    int num_rows) { 
  int i, j;

  message( "Initializing row indices...");
  if( ( *row_indices = (int*) malloc( num_rows*sizeof(int))) == NULL) {
    message( "Can't allocate row_indices memory.\n");
    return 0;
  }
  /* Decorate the row label array. */
  for( i=j=0; i<DIC_SIZE && j<DIC_SIZE; i++) {
    /* Give non-stopped words non-negative row indices. 
       All words come with -1 from read_dictionary. */
    if( j >= num_rows)
      break;
    if( !( word_array[i].is_stop)) {
      word_array[i].row = j++;
      (*row_indices)[j-1] = i; /* index of row label in word_array */ 
    }
  }
  message( "Done.\n");
  
  return 1;
}

int initialize_column_indices( WORDINFO *word_array, int **col_indices,
                               int num_columns, int col_labels_from_file, char *col_label_file, NODE **word_tree) {
  int i, j, k;
  int num_labels_in_file=0;
  int index;
  char **col_labels;
  char c;
  FILE *fptr_col_labels=NULL;


  message( "Initializing column indices...");
  if( ( *col_indices = (int*) malloc( num_columns*sizeof(int))) == NULL) {
    message( "Can't allocate col_indices memory.\n");
    return 0;
  }

  if( col_labels_from_file) { /* read column labels from file */

    /* open the file containing the column labels */
    if( !my_fopen( &fptr_col_labels, col_label_file, "r")) {
      perror( "initialize_column_indices");
      return 0;
    }
    for( i=0; ( c=getc( fptr_col_labels)) != EOF; i++) {
      if( c=='\n')
        num_labels_in_file++; /* how many labels does the file contain? */
    }

    if( ((col_labels = (char **) malloc( num_labels_in_file * sizeof(char *))) == NULL)){
      perror( "Allocating col_labels memory");
      return 0;
    }

    /* move pointer back to beginning of the file */
    fseek( fptr_col_labels, 0, SEEK_SET);

    for ( i=0; i<num_labels_in_file; i++){ /* read column labels into array */

      if( ((col_labels[i] = (char *) malloc( MAXWORDLEN*sizeof(char))) == NULL)){
        perror( "Allocating col_labels memory");
        return 0;
      }

      for( j=0; ( c=getc( fptr_col_labels)) != EOF; j++) {
        if( c=='\n'){
          col_labels[i][j] = '\0';
          break;
        }
        else
          col_labels[i][j] = c;
      }
    }

    if( !my_fclose( &fptr_col_labels))
      die( "Can't close column label file.\n");

    j=0;
    for( i=0; i<num_labels_in_file; i++) {
      if( j >= num_columns)
        break;
      if( ( index = word_index( col_labels[i], *word_tree)) != -1) { /* the label appears in word_array */
        if( word_array[index].row != -1) { /* it's a row label */
          word_array[index].col = j++;
          (*col_indices)[j-1] = index;
        }
      }
    }
    if( j < num_columns) {
      printf( "WARNING: only %d column labels instead of %d\n", num_labels_in_file, num_columns);
    }

  }
  else { /* take the top 51 to (rows+50) most frequent words as column labels */
    /* Decorate the column label array. */
    for( i=j=k=0; i<DIC_SIZE && j<DIC_SIZE; i++) {
      /* Give non-stopped words qualifying for columnlabel-hood
         non-negative column indices.
         The START_COLUMNS most frequent non-stop words are ignored.
         All words come with -1 from read_dictionary. */
      if( j >= num_columns)
        break;
      if( k < START_COLUMNS) {
        k++;
        continue;
      }
      if( !( word_array[i].is_stop)) {
        word_array[i].col = j++;
        (*col_indices)[j-1] = i;
      }
    }
  }

  message( "Done.\n");

  return 1;
}


/**************************************************
  word_index

  Given a string, get the index recorded in the corresponding wordinfo
  in the wordarray. 
  */
int word_index( char *string, NODE *word_tree) {
  NODE *node;
  if( ( node = lookup_word( string, word_tree)) == NULL)
    return -1;
  return ( (WORDINFO *) node->data)->index;
}
