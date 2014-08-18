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
  list.c

  Routines to handle linked lists.  Currently only accumulation
  of members (no removal, no search) implemented.

  Stefan Kaufmann, March 2001
  **************************************************/

#include "list.h"


/**************************************************
  list_insert

  Insert a new member into the list at the appropriate position.
  We make the address of the last list member available to the
  calling function because we use 'thresholds' to determine whether
  to call this function in the first place (it is called only if the
  new member has a chance of being admitted).

  The desired length of the list is given in the argument 'depth';
  if this is zero, nothing is done.  If it is negative, no restriction
  on the length of the list is imposed.

  The appropriate position is determined by the 'data_cmp' function,
  which is defined in the calling code and passed to list_insert
  as an argument.

  ARGUMENTS:	1 top		head of the list
		2 last		last element of the list
		3 depth		desired length of the list
		4 new_data	data to be held by the new member
		5 data_size	size (in bytes) of the data structure
		6 data_cmp	comparison function

  RETURN_VALUES:	0 if the member could not be created
			1 if all went well
			*/
int list_insert( LIST_MEMBER **top, LIST_MEMBER **last, int depth, 
		 void *new_data, int data_size,
		 int (*data_cmp)( void*, void*),
		 void (*data_free)(void *)) {
  LIST_MEMBER *tmp, *pre, *current;
  int i;	/* counts the length of the list */

  /* Do nothing if the desired depth is zero */
  if( depth == 0 )
    return 1;
  
  /* Be prepared to count if the desired depth is positive */
  if( depth > 0 )
    i = depth;
  /* If the depth is negative, we don't count. */
  else
    i = -1;
  
  current = *top;
  pre = current;

  /* if the new data become head of the list */
  if( ( *top == NULL)
      || ( data_cmp( new_data, (void *) ((*top)->data)) > 0)) {
    current = *top;
    if( ( *top = create_member( new_data, data_size)) == NULL)
      return 0;
    pre = *top;
    pre->next = current;
    i--;
  }

  /* otherwise, look for the appropriate slot */
  else {
    while( i
	   && ( current != NULL)
	   && ( data_cmp( new_data, (void *) (current->data)) <= 0)) {
      pre = current;
      current = current->next;
      i--;
    }
    /* insert the new data */
    if( i ) {
      if( ( tmp = create_member( new_data, data_size)) == NULL)
	return 0;
      pre->next = tmp;
      pre = tmp;
      pre->next = current;
      i--;
    }
  }

  /* Move on to the end of the list to remove the last member if
     the list has grown too long */
  while( i && ( current != NULL)) {
    pre = current;
    current = current->next;
    i--;
  }

  /* Erase the dangling last members if there are any. */
  free_tail( (void *) &(pre->next), data_free );

  /* Set the tail pointer to the last member */
  *last = pre;
  
  return 1;
}

/**************************************************
  create_member

  Allocate memory for a new list member and its 'data' element
  and initialize the 'data' element.

  ARGUMENTS:	new_data	the data to be held by the new member
		data_size	the size of the data structure

  RETURN VALUES:	a pointer to the new member if memory could
				be allocated
			NULL	otherwise
			*/

LIST_MEMBER *create_member( void *new_data, int data_size) {
  LIST_MEMBER *ret;

  /* Allocate member memory */
  if( ( ret = (LIST_MEMBER *) malloc( 101 ) ) == NULL) {
    fprintf( stderr, "Couldn't allocate list member memory.\n");
    return NULL;
  }

  /* Allocate data memory */
  if( ( ret->data = malloc( data_size)) == NULL) {
    fprintf( stderr, "Couldn't allocate list data memory.\n");
    return NULL;
  }

  /* Initialize the member */
  memcpy( ret->data, new_data, data_size);
  ret->next = NULL;

  return ret;
}

/**************************************************
  free_tail

  Free up all the members of the list including and beyond 'current'.
  Set 'current' to NULL.

  ARGUMENTS:		current		the first member to be erased
  RETURN VALUES:	1
  */
int free_tail( LIST_MEMBER **current, void (*data_free)(void *)) {
  if( *current == NULL)
    return 1;
  free_tail( &((*current)->next), data_free );
  data_free( (*current)->data);
  free( (*current)->data);
  free( *current);
  *current = NULL;
  return 1;
}
