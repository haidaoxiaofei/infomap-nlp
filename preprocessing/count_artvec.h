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
  count_artvec.h

  Goes with count_artvec.c
  Some definitions and declarations.

  Stefan Kaufmann, August 2000
  **************************************************/

#ifndef COUNT_ARTVEC_H
#define COUNT_ARTVEC_H	1

#include "env.h"
#include "dict.h"
#include "matrix.h"
#include "utils.h"
#include "wordlist.h"
#include "word_tree.h"
#include "vector.h"

/**************************************************
  DEFINITIONS
  **************************************************/

/**************************************************
  DECLARATIONS
  **************************************************/
int is_target( int candidate_index, ENVIRONMENT *env);
int advance_target(	int *int_buffer,int words_read,	int *target,
			int *region_in, int *region_out);
int set_region_in(	int *int_buffer,int words_read,	int *target,
			int *region_in, int *region_out);
int set_region_out(	int *int_buffer,int words_read,	int *target,
			int *region_in, int *region_out);
int process_region(	int *int_buffer,int words_read,	int target, 
			int region_in,	int region_out, ENVIRONMENT *env);

double output_transformation( MATRIX_TYPE arg);
MATRIX_TYPE term_weight( int *buffer, int target, int cursor,
			 ENVIRONMENT *env);
#endif

