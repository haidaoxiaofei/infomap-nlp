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
  word_tree.c

   Stores words in a hierarchy of binary trees.
   Stefan Kaufmann, July 2000

**************************************************/

#include "word_tree.h"


/**************************************************
   new_node.

   Allocate memory for a new node,
   set its character to c, its frequency to 0, its tokennum to -1, 
   and initialize its pointers:  parent to parent, the others to NULL.

   Arguments:	1 c	the character to be entered
		parent	the invoking node (where the prefix ends)

   Return Values:	NULL	if the allocation failed
			the address of the new node if all goes well
			*/

NODE *new_node( char c, NODE *parent) {
  NODE *ret;

  /* Allocate memory. */
  if( ( ret = (NODE *) malloc( sizeof( NODE))) == NULL)
    return NULL;

  /* Initialize. */
  ret->c = c;
  ret->data = NULL;
  ret->leftdtr = ret->rightdtr = ret->next = NULL;
  ret->parent = parent;
  return ret;
}


/**************************************************
  insert_word()

  Traverse the tree to see if string has been processed before
  Create new nodes as needed as you go along.
  Recursive.

  Arguments:	1 current	the current node
		2 string	the word (or suffix) we are inserting
		3 parent	the caller (used for creating links
				in new_node)
  Return Values:	NULL	if we couldn't create a node we needed
			the address of the new node
			if all goes well.
			*/

NODE *insert_word( NODE **current, char *string, NODE *parent) {

  /* If we don't have a node here */
  if( *current == NULL)
    if( ( *current = new_node( string[0], parent)) == NULL)
      return NULL;
  
  /* If we've seen this prefix before */
  if( string[0] == (*current)->c) {
    /* If the string is used up... */
    if( strlen( string) == 1)
      return *current;
    /* If the string is not used up... */
    return insert_word( &((*current)->next), &(string[1]), *current);
  }
  /*  If we haven't seen this before... */
  if( string[0] < (*current)->c)
    return insert_word( &((*current)->rightdtr), string, *current);
  return insert_word( &((*current)->leftdtr), string, *current);
}


NODE *lookup_word( char *string, NODE *current) {
  if( current == NULL)
    return NULL;
  /* If we know this initial substring */
  if( string[0] == current->c) {
    /* If this is the end of the input word... */
    if( strlen( string) == 1) {
      /* If there are no data, this is not the end of a word. */
      if( current->data == NULL)
	return NULL;
      /* If there are, it is. */
      return current;
    }
    /* If it's not the end, go on. */
    return lookup_word( &(string[1]), current->next);
  }
  /* If the current node does not belong to the word, go on. */
  if( string[0] < current->c)
    return lookup_word( string, current->rightdtr);
  return lookup_word( string, current->leftdtr);
}

/**************************************************
  print_string.

  Given a node in the tree, print the string to a file.
  Recursive.
  That's why we need parent pointers:  we have only pointers to the nodes
  holding the *last* characters of words, so the function works its
  way up to the root and then comes back down, printing the characters.
  
  Arguments:	1 buffer:	the array on which the string is
				assembled
		2 index:	the current index in the buffer
		2 current:	the current node in the tree.
		3 caller:	the node from which we were called.
				if it is is our 'next', we know that our
				character was part of the word.

  Return values:	none (void).
  */
char *print_string2( char *buffer, int *index, NODE *current, NODE *caller) {
  /* If we are at the root */
  if( current == NULL)
    return buffer; /*before just return */
  /* Print the prefix up to the current character. */
  print_string2( buffer, index, current->parent, current);
  /* Print the current character if we are in the middle of the word. */
  if( current->next == caller)
    buffer[(*index)++] = current->c;
  /* Also terminate the string if we are done. */
  else
    if( current == caller) {
      buffer[(*index)++] = current->c;
      buffer[*index] = '\0';
    }
  return buffer;
}

char *print_string( char *buffer, NODE *root) {
  int index = 0;
  return print_string2( buffer, &index, root, root);
}

/**************************************************
  free_tree

  free up the memory occupied by the tree.
  Does _not_ free the structures pointed to by "data"!

  ARGUMENTS:	current:	the current node

  RETURN_VALUES:	void
  */
void free_tree( NODE *current) {
  if( current == NULL)
    return;
  free_tree( current->leftdtr);
  free_tree( current->next);
  free_tree( current->rightdtr);
  free( current);
  return;
}
