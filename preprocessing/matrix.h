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
  matrix.h

  Functions for initializing matrices and writing
  them to disk in useful formats.
  
  Used for word-word co-occurrence matrices.

  Stefan Kaufmann, January 2001
  Modified by Scott Cederberg (December 2003) and
    possibly by others (e.g. Beate Dorow and
    Dominic Widdows) in the intervening time.

  **************************************************/

#ifndef MATRIX_H
#define MATRIX_H	1

#include "env.h"
#include "utils.h"
#include "vector.h"

/**
 *  Dynamically allocate memory for a matrix with "rows" rows
 *  and "columns" columns and elements of MATRIX_TYPE.  
 *  Initialize all matrix elements to 0.
 */
int initialize_matrix( MATRIX_TYPE ***matrix, int rows, int columns);

/**
 *  Write the matrix "matrix" (which has "rows" rows and "columns" columns) 
 *  to the COLL_FILE and INDEX_FILE in model_data_dir, in a
 *  format that can be used as input by the svdinterface code.
 */
int write_matrix_svd(MATRIX_TYPE **matrix,
		     int rows, int columns,
		     char *model_data_dir);

/**
 *  Write the matrix "matrix" (which has "rows" rows and "columns" columns)
 *  to the MATLAB_FILE in model_data_dir, in a format
 *  that can be used as input by Matlab.
 */
int write_matrix_matlab(MATRIX_TYPE **matrix,
			int rows, int columns,
			char *model_data_dir);

#endif /* !MATRIX_H */

