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

#ifndef PREPROCESSING_TYPES_H
#define PREPROCESSING_TYPES_H 1

#include "env.h"

/* NODE	type, a structure holding stuff, arranged in a hierarchy of
   binary trees (word_tree.c).   */
typedef struct node {
  char c;		/* holds the current character */
  void *data;           /* in dictionary structures, points to
			 * WORDINFO structure for word corresp to this
			 * node */
  struct node *leftdtr, *rightdtr, *next, *parent;
} NODE;

typedef struct wordinfo {
  int term_freq;
  int doc_freq;
  int doc_last_seen;
  int is_stop;
  int index;
  int row;
  int col;
  char *string;
  NODE *nodeptr;
  struct wordinfo *wordinfoptr;
} WORDINFO;

typedef struct {
  WORDINFO *word_array;
  NODE *word_tree;
  int *row_indices; /* index of row labels in word_array */
  int *col_indices; /* index of column labels in word_array */
  MATRIX_TYPE **matrix;
} ENVIRONMENT;

#endif /* !PREPROCESSING_TYPES_H */
