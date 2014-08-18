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
  neighbors.c

  Find nearest neighbors in a vector space.
  Vectors are (currently) either word or document vectors.

  Stefan Kaufmann, March 2001
  **************************************************/

#include "neighbors.h"

void neighbor_item_free( void *neighbor_item );

/**************************************************
  find_neighbors

  Find the nearest neighbors to the queryvector.

  ARGUMENTS:	1 Files	structure holding file pointers
		2 list	ptr to linked list; the list will
		        be filled in with NEIGHBOR_ITEM
			structures
		3 queryvector	the queryvector

  RETURN VALUES:	1 for success
                        0 for failure
  **************************************************/
int find_neighbors( FILE *vectorfile,
		    LIST_MEMBER **list, int depth, 
		    MATRIX_TYPE *queryvector, int num_singvals ) {
  MATRIX_TYPE *tmpvector;
  MATRIX_TYPE length;
  int tmpoffset = 0; 
  int i;
  LIST_MEMBER *last = NULL;
  int list_length=0;

  NEIGHBOR_ITEM neighbor_item;
  MATRIX_TYPE *tmp_ni_vector;

  int vector_size = num_singvals * sizeof(MATRIX_TYPE);

  if ( (tmpvector = malloc(vector_size) ) == NULL) {
    fprintf( stderr, "neighbors.c: can't malloc vector memory.\n" );
    return 0;
  }
  
  if ( (tmp_ni_vector = malloc(vector_size) ) == NULL) {
    fprintf( stderr, "neighbors.c: can't malloc neighbor item vector.\n" );
    free( tmpvector );
    return 0;
  }
  neighbor_item.vector = tmp_ni_vector;
  
  /* Read vectors one at a time (in one go, from the binary file) */
  fseek( vectorfile, 0, SEEK_SET );
  while( fread( neighbor_item.vector, 
		vector_size, 1, vectorfile )) {

    neighbor_item.num_elems = num_singvals;
    neighbor_item.score = 0.0;
    for( i=0; i < num_singvals; i++ ){
      neighbor_item.score += (neighbor_item.vector)[i] * queryvector[i];
    } 
    
    neighbor_item.offset = tmpoffset;
    
    if( neighbor_item.score > threshold) {

      list_insert( list, &last, depth, 
		   (void *) &neighbor_item, sizeof( NEIGHBOR_ITEM ),
		   neighbor_item_cmp, neighbor_item_free );
      
      list_length++;
      
      /* Set the threshold to the lowest value in the list */
      if( (last != NULL) && (list_length >= depth))
	threshold = ((NEIGHBOR_ITEM *) (last->data))->score;
    }
    tmpoffset += vector_size;

    if ( (tmp_ni_vector = malloc(vector_size)) == NULL ) {
      fprintf( stderr, "neighbors.c: can't allocate vector memory.\n" );
      free_tail( list, neighbor_item_free );
      return 0;
    }
    neighbor_item.vector = tmp_ni_vector;
  }
  free( tmpvector );
  free( tmp_ni_vector );
  return 1;
}

/**************************************************
  neighbor_item_cmp

  Comparison function to be passed as a parameter to list_insert.

  ARGUMENTS:	1 left	left argument
		2 right right argument

  RETURN VALUES:	 1 if left is less than right
			-1 if right is less than left
			 0 otherwise
			 */
int neighbor_item_cmp( void *left, void *right) {
  MATRIX_TYPE tmp = 
    ((NEIGHBOR_ITEM *) left)->score - 
    ((NEIGHBOR_ITEM *) right)->score;
  if( tmp > 0.0)
    return 1;
  if( tmp < 0.0)
    return -1;
  return 0;
}

/*
 *  Free dynamically allocated memory associated
 *  with a NEIGHBOR_ITEM object.  So far,
 *  this memory is just the "vector" element.
 */
void neighbor_item_free( void *neighbor_item ) {

  if ( ((NEIGHBOR_ITEM *)neighbor_item)->vector != NULL ) {
    free( ((NEIGHBOR_ITEM *)neighbor_item)->vector );
  }

}

/**************************************************
  print_list

  Prints the linked list in the format 'word:similarity'
  for words and 'artloc:similarity' for documents.

  ARGUMENTS:		1 top	the head of the list
			2 depth	(max) number of items to be printed
  RETURN VALUES:	1
  */
int print_list( LIST_MEMBER *top, int depth) {
  NEIGHBOR_ITEM *tmp = NULL;

  while( top != NULL && depth) {
    tmp = (NEIGHBOR_ITEM *) (top->data);
    printf( "%s:%"FORMAT_STRING"\n", tmp->string, tmp->score );
    top = top->next;
    depth--;
  }
  
  return 1;
}













