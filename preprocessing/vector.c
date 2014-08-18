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

#include "vector.h"

MATRIX_TYPE *vector_sum( MATRIX_TYPE *left, MATRIX_TYPE *right, 
			 MATRIX_TYPE *sum, int dimensions) {
  int i;

  clear_vector( sum, dimensions);

  for( i=0; i<dimensions; i++)
    sum[i] = left[i] + right[i];

  return sum;
}

MATRIX_TYPE *dot_product( MATRIX_TYPE *left, MATRIX_TYPE *right,
			  MATRIX_TYPE *product, int dimensions) {
  int i;
  clear_vector( product, dimensions);
  for( i=0; i<dimensions; i++)
    product[i] = left[i] * right[i];
  return product;
}


MATRIX_TYPE *clear_vector( MATRIX_TYPE *vector, int dimensions) {
  int i;

  for( i=0; i<dimensions; i++)
    vector[i] = (MATRIX_TYPE) 0.0;
  return vector;
}

MATRIX_TYPE *normalize_vector( MATRIX_TYPE *vector, int dimensions) {
  int i;
  MATRIX_TYPE length = 0.0;
  
  length = vector_length( vector, dimensions);
  /*fprintf( stderr, "vector.c: normalizing vector of length %"
    FORMAT_STRING "\n", length ); */
  if( !length)
    return vector;
  else
    for( i=0; i<dimensions; i++) {
      /*fprintf ( stderr, " %.2"FORMAT_STRING, vector[i] );*/
      /*fprintf ( stderr, "=>" );*/
      vector[i] /= length;
      /*printf ( stderr, "%.2"FORMAT_STRING, vector[i] );*/
    }
      
  return vector;
}

MATRIX_TYPE vector_length( MATRIX_TYPE *vector, int dimensions) {
  int i;
  MATRIX_TYPE ret = 0.0;
  for( i=0; i<dimensions; i++)
    ret += ( vector[i] * vector[i]);
  return (MATRIX_TYPE) sqrt( (double) ret);
}

int write_vector( FILE *output, MATRIX_TYPE *vector, int dimensions) {
  int i;
  for( i=0; i<dimensions; i++)
    if( fprintf( output, "%"FORMAT_STRING"\n", vector[i]) < 0) {
      perror( "Writing vector");
      return 0;
    }
  return 1;
}

int read_vector( FILE *infile, MATRIX_TYPE *vector, int dimensions) {
  int i;
  for( i=0; i<dimensions; i++)
    if( fscanf( infile, "%" FORMAT_STRING, &(vector[i])) == EOF)
      return 0;
  return 1;
}
