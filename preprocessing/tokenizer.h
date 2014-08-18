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
  tokenizer.h

  Belongs to tokenizer.c
  Reads a corpus and returns the words individually.
  Stefan Kaufmann, July 2000
  
  **************************************************/

#ifndef TOKENIZER_H
#define TOKENIZER_H	1

#include <locale.h>
#include "env.h"
#include "preprocessing_env.h"
#include "utils.h"

/**************************************************
  GLOBAL VARIABLES

  are in tokenizer.c
  **************************************************/

/**************************************************
  FUNCTION DECLARATIONS
  **************************************************/

/* Open the file to be read, set the locale, prepare the buffers. */
int initialize_tokenizer(char *corpus_filename, char *fnames_filename, char * valid_chars_filename );

/* Return the next word. */
int next_token( char *outbuffer, int *curr_pos );

/* Read the next line.  Invoked by next_token. */
int next_line( char *linebuffer, int *curr_pos );

/* Read a line.  Invoked by next_line. */
int read_line( FILE *fptr, char *linebuffer, int *curr_pos );

/* Close the file etc. */
/*int close_tokenizer( );*/

#endif
