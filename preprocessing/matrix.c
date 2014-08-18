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
  matrix.c

  Functions for initializing matrices and writing
  them to disk in useful formats.

  Used for word-word co-occurrence matrices.

  Stefan Kaufmann, January 2001
  Modified by Scott Cederberg (December 2003) and
    possibly by others (e.g. Beate Dorow and
    Dominic Widdows) in the intervening time.

  **************************************************/

#include "matrix.h"

/**************************************************
  initialize_matrix

  Allocate memory for a rows x columns matrix and set all values to zero.
  The type of the cells is set in the global variable MATRIX_TYPE;
  e.g., it can be 'int' or 'double'.

  ARGUMENTS:	1 matrix	The pointer which will hold the matrix
		2 rows		number of rows
		3 columns	number of columns

  **************************************************/

int initialize_matrix( MATRIX_TYPE ***matrix, int rows, int columns) {
  int i,j;

  /* Allocate memory for an array of pointers to vectors. */
  message( "Allocating matrix memory...");
  if( ( *matrix = 
	(MATRIX_TYPE **) malloc( rows * sizeof( MATRIX_TYPE *))) == NULL) {
    message( "Can't allocate row memory.\n");
    return 0;
  }
  /* Allocate vector memory for each pointers in the array. */
  for( i=0; i<rows; i++)
    if( ( (*matrix)[i] = 
	  (MATRIX_TYPE *) malloc( columns * sizeof( MATRIX_TYPE))) == NULL) {
      message( "Can't allocate row memory.\n");
      return 0;
    }
  message( "done.\n");
  
  /* Set all values to zero. */
  message( "Initializing matrix...");
  /* Initialize the matrix. */
  for( i=0; i<rows; i++)
    for( j=0; j<columns; j++)
      (*matrix)[i][j] = (MATRIX_TYPE) 0.0;

  message( "done.\n");

  return 1;
}

/**************************************************
  write_matrix_svd

  Writes the vectors in column-major format, for input to SVDpack.
  
*/
int write_matrix_svd(MATRIX_TYPE **matrix,
		     int rows, int columns,
		     char *model_data_dir) {

  FILE *coll_file, *index_file;
  int i, j, non_zero_cnt;
  char pathbuf[BUFSIZ];

  printf("Entering write_matrix_svd; rows = %d and columns = %d.\n",
	 rows, columns);

  /* Open the output files. */
  sprintf( pathbuf, "%s/%s", model_data_dir, COLL_FILE );
  if( !my_fopen( &coll_file, pathbuf, "w"))
    die( "Couldn't open word vector file.\n");
  
  sprintf( pathbuf, "%s/%s", model_data_dir, INDEX_FILE );
  if( !my_fopen( &index_file, pathbuf, "w" ))
    die( "Couldn't open index file.\n");
  
  /* Write the numbers. */
  for( j = 0; j < columns; j++) {
    /*printf("===== Column %d =====\n", j);*/
    
    /* Write the non-zero entries */
    for( i = non_zero_cnt = 0; i < rows; i++) {

      /*printf("matrix[%d][%d] = %" FORMAT_STRING" \n", i, j, matrix[i][j]);*/

      if( matrix[i][j] != (MATRIX_TYPE) 0.0) {
	non_zero_cnt++;
	/*printf("matrix.c: printing to coll_file\n" );*/
	if( fprintf( coll_file, "%d %f\n", 
		     i, (float) matrix[i][j]) < 0) {
	  perror( "Trying to write word vector file");
	  return 0;
	}
      }
    }

    /* Write the number of non-zero cells in this column 
       to the index file. */
    if( fprintf( index_file, "%d\n", non_zero_cnt) < 0) {
      perror( "Trying to write index file");
      return 0;
    }
  }

  my_fclose( &coll_file);
  my_fclose( &index_file);


  return 1;
}

/** 
 *  write_matrix_matlab()
 *  ---------------------
 *
 *  Write the contents of the rows by columns matrix "matrix" to
 *  the file MATLAB_FILE in model_data_dir, in a format
 *  readable by Matlab.
 *
 */
int write_matrix_matlab(MATRIX_TYPE **matrix,
			int rows, int columns,
			char *model_data_dir) {

  FILE *matlab_file = NULL;
  char pathbuf[BUFSIZ];
  int i, j;

  sprintf( pathbuf, "%s/%s", model_data_dir, MATLAB_FILE );
  if( !my_fopen( &matlab_file, pathbuf, "w" ))
    die( "Couldn't open matlab file.\n");

  
  for (j = 0; j < columns; j++) {
    for (i = 0; i < rows; i++) {

      if( fprintf( matlab_file, "%d %d %f\n", 
		   j, i, (float) matrix[i][j]) < 0) {
	perror( "Trying to write co-occurrence matrix in matlab format");
	return 0;
      }
    }
  }

  my_fclose( &matlab_file);
  return 1;

}








