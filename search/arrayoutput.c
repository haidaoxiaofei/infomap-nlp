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

# include "associate.h"

/****************************************************************
file arrayoutput.c takes the word array found by neighbors.c and outputs it 
to a file called ArrayOutput/wordarray.data                    

This can be used by Splus, matlab etc. for principal component analysis,
 cluster analysis, etc. Minor modifications may be necessary.

     Arguments 1. The queryvector, to be output first
               2. The array, in order of similarity to the query   
     Returns 1 for success */


int output_array( LIST_MEMBER **top, char* outfileName, int vector_length ){
  FILE *outfile;
  LIST_MEMBER *current = *top; 
  int i, j = 0;

  outfile = fopen ( outfileName, "w" );

  /* print first line indicating number of words and 
     number of singular values given for each word.
     Makes file format acceptable to the TableReader Java class
     used in the WordSpectra web demo. 
  */
  fprintf( outfile, "%d %d\n", MAX_OUTPUT, vector_length);


  /* as current pointer goes through word list */  
  while(( j++ < MAX_OUTPUT ) && ( current != NULL )){
    /* output the current word ....*/
    fprintf( outfile, "%s ", ((NEIGHBOR_ITEM *) current->data)->string );
    /* followed by the word vector coordinates*/
    for( i=0; i < vector_length; i++ ){
      fprintf( outfile, "%f ", (((NEIGHBOR_ITEM *) current->data)->vector)[i] );
    }
    /* output a new line character and move on */
    fprintf( outfile, "\n" );
    current = current->next;
  }

  fclose( outfile );
  return(1);
}





