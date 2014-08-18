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
  stopper.c

  Reads a stop list and tells, given a word, whether
  it is a stopword.
  Stefan Kaufmann, August 2000
  
  **************************************************/

#include "stopper.h"


/**************************************************
  initialize_stoplist

  Open the stoplist file and read the list into a binary tree,
  stored in the global variable 'stop_tree'

  Arguments:		none.
 
  Return values:	0 if the file can't be read or
				the tree can'b be built.
			1 if all goes well.
			*/
int initialize_stoplist(char *stoplist_filename) {
  FILE *stoplist;
  char buffer[MAXWORDLEN];

  if( !my_fopen( &stoplist, stoplist_filename, "r"))
    return 0;

  while( ( fscanf( stoplist, "%s", buffer)) != EOF)
    if( !insert_stopword( buffer, &stop_tree))
      return 0;

  my_fclose( &stoplist);
  return 1;
}

/**************************************************
  is_stop

  Check whether the input string is in the stopword tree.
  Not recursive.

  Arguments:	1 str	the input string

  Return values:	1 if the string is in the tree
			0 otherwise.
			*/
int is_stop( char *str) {
  STOPNODE *current = stop_tree;
  int cmp;
  /* string with pos-tag stripped off */
  char str_no_pos[MAXWORDLEN];
  int j;

  /* copy str into str_no_pos, cutting off at the underscore */
  for( j=0; j < MAXWORDLEN; j++ ){
    if( ( str[j] == '_') || ( str[j] == '\0') ){
      str_no_pos[j] = '\0';
      break;
    }
    str_no_pos[j] = str[j];
  }



  /* Until we reach a dead end... */
  while( current != NULL) {
    /* If we found the word... */
    if( !( cmp = strcmp( str_no_pos, current->string)))
      return 1;

    /* Otherwise, traverse the tree. */
    if( cmp < 0)
      current = current->leftdtr;
    else
      current = current->rightdtr;
  }
  /* If we did reach a dead end... */
  return 0;
}


/**************************************************
  create_stopnode

  Allocate a new node for a new stopword.

  Arguments:	1 str	the input string

  Return values:	NULL if memory allocation failed
				either for the node or for the string
			a pointer to the new node if all went well.
			*/
STOPNODE *create_stopnode( char *str) {
  STOPNODE *ret;

  /* Allocate node memory. */
  if( ( ret = (STOPNODE *) malloc( sizeof( STOPNODE))) == NULL)
    return NULL;

  /* Allocate string memory. */
  if( ( ret->string = (char *) malloc( strlen( str) + 1)) == NULL)
    return NULL;

  /* Initialize the node and return its address. */
  strcpy( ret->string, str);
  ret->leftdtr = ret->rightdtr = NULL;
  return ret;
}

/**************************************************
  insert_stopword

  Traverse the stopword tree to find or else insert a word.
  Recursive.

  Arguments:	1 str		the string to be inserted
		2 current	the current node in the tree.

  Return values:	0 if the string couldn't be inserted
				(because create_stopnode failed)
			1 if all went well.
			*/
int insert_stopword( char *str, STOPNODE **current) {
  int cmp;

  /* If we are at the end of a branch, add a node. */
  if( *current == NULL) {
    if( ( *current = create_stopnode( str)) == NULL)
      return 0;
    return 1;
  }
  
  /* If we found the word at the current position... */
  if( !( cmp = strcmp( str, (*current)->string)))
    return 1;
  
  /* Else, keep traversing. */
  if( cmp < 0)
    return insert_stopword( str, &((*current)->leftdtr));
  
  return insert_stopword( str, &((*current)->rightdtr));
}
