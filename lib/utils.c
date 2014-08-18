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

/* utils.c
   Some routines to avoid cluttering up the programs.

   Stefan Kaufmann, July 1999
*/

#include "utils.h"

/**************************************************
  GLOBAL VARS
**************************************************/
MEM *memor = NULL;

/* This is a somewhat radical approach, in that it assumes
   ASCII for efficiency and will *break* with other character
   encodings.  */
int my_isalpha( int c) {  // configured to let underscore through for POS and tilda for indexing compounds
  return( ( c > 64 && c < 91) || ( c > 96 && c < 123) || ( c == '_') || c == '~');
}

int my_write( int filedes, void* buffer, int size) {
  int i, written;
  i = written = 0;
  while( ( i = write( filedes, (buffer + written), (size - written))) 
	 < ( size - written)) {
    if( i < 0) {
      perror( "my_write");
      return 0;
    }
    written += i;
  }
  return 1;
}

int my_fopen( FILE **fptr, char *fname, char *mode) {
  if( VERBOSE)
    fprintf( stderr, "Opening File for \"%s\":\n\t\"%s\"\n", mode, fname);
  if( ( *fptr = fopen( fname, mode)) == NULL) {
    perror( "my_fopen");
    return 0;
  }
  return 1;
}

int my_fclose( FILE **fptr) {
  if( fclose( *fptr) == EOF) {
    perror( "my_fclose");
    return 0;
  }
  return 1;
}

int my_opendir( DIR **dirptr, char *dirname) {
  if( VERBOSE)
    fprintf( stderr, "Opening Directory %s\n", dirname);
  if( ( *dirptr = opendir( dirname)) == NULL) {
    perror( "my_opendir");
    return 0;
  }
  return 1;
}

int my_closedir( DIR **dirptr) {
  if( closedir( *dirptr) == -1) {
    perror( "my_closedir");
    return 0;
  }
  return 1;
}





int die( char *str) {
  fprintf( stderr, "%s\n", str);
  exit( EXIT_FAILURE);
}

int message( char *str) {
  if( VERBOSE )
    fprintf( stderr, "%s", str );
  return 1;
}

/* Check whether string is in array. */
int in_array( char **array, char *str) {
  int i;
  for( i=0; array[i] != NULL; i++)
    if( !strcmp( array[i], str))
      return 1;
  return 0;
}

/* Memory management. */
char *mymalloc( int size) {
  char *ret;
  if( memor == NULL || memor->space < size) {
    if( ( memor = (MEM *) malloc( sizeof( MEM))) == NULL)
      return NULL;
    if( ( memor->ptr = malloc( MEM_CHUNK)) == NULL)
      return malloc( size);
    memor->space = MEM_CHUNK;
  }
  ret = memor->ptr;
  memor->ptr += size;
  memor->space -= size;
  return ret;
}
