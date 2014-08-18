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
  word_tree.h

   Stores words in a hierarchy of binary trees.
   Stefan Kaufmann, July 2000

**************************************************/

#ifndef WORD_TREE_H
#define WORD_TREE_H	1

#include <string.h>
#include "env.h"
#include "preprocessing_types.h"

/**************************************************
  FUNCTION DECLARATIONS
  **************************************************/

/* Create a new node. */
NODE *new_node( char c, NODE *parent);

/* Insert a word in the tree. */
NODE *insert_word( NODE **current, char *string, NODE *parent);

NODE *lookup_word( char *string, NODE *root);

/* Given a node in the tree, return the string 
   (that's why we need parent pointers.) */
char *print_string2( char *buffer, int *index, NODE *current, NODE *caller);
char *print_string( char *buffer, NODE *root);

void free_tree( NODE *current);

#endif /* !WORD_TREE_H */
