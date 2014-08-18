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
  tokenizer.c

   Reads a corpus and returns the words individually.
   Stefan Kaufmann, July 2000

 **************************************************/

#include "tokenizer.h"


/**************************************************
  GLOBAL VARIABLES
  **************************************************/
FILE /* *FptrCorpus = NULL,*/ *FptrFnames = NULL, *FptrCurrent = NULL; 
FILE *ValidChars = NULL;
char linebuffer[MAXLINELEN+1];
char *buffer_reader;
int  IntIntext, IntIndoc, IntTextbeg, IntTextend, IntDoccntr;
int  numFiles=0; /* number of files which comprise corpus */
char *ignore[] = IGNORE_TXT;
int CORPUS_TYPE = MANY_FILES;
char **fnameslist; /* names of corpus files in case of CORPUS_TYPE=MANY_FILES */
int valid_chars[256];
/*************************************************
  initialize_tokenizer / close_tokenizer.

  Set the locale (LC_CTYPE) to what is required in corpus_params
  ????
  Open the corpus file
  open and initialize valid characters and characters to break on
  initialize some buffers

  Arguments:		corpus_filename, fnames_filename, valid_chars_filename

  Return Values:	0 if the corpus file could not be opened
			1 if everything is going well
			*/
int initialize_tokenizer(char *corpus_filename, char *fnames_filename, char *valid_chars_filename ) {
  int i, j;
  char c;

  fprintf( stderr, "Locale set to %s.\n", setlocale( LC_CTYPE, "en_US"));


  if( my_fopen( &ValidChars, valid_chars_filename, "r") != NULL ) {
    for (i = 0; i<256; i++) { valid_chars[i] = 0; }
    while ( ( c = getc( ValidChars ) ) != EOF ) { valid_chars[c] = 1; }
  } else {
    fprintf(stderr, "Unable to open valid characters file \"%s\"\n", valid_chars_filename);
    return 0;
  }

  /* open the file containing filenames */
  if( !my_fopen( &FptrFnames, fnames_filename, "r")) {
    /* perror( "initialize_tokenizer"); 
    return 0; */
    numFiles=1;
    if( ((fnameslist = (char **) malloc( numFiles * sizeof(char *))) == NULL)){
      perror( "Allocating fnameslist memory");
      return 0;
    }
    if( ((fnameslist[0] = (char *) malloc( MAXWORDLEN*sizeof(char))) == NULL)){
        perror( "Allocating filename memory");
        return 0;
    }
    strcpy( fnameslist[0], corpus_filename); 
    CORPUS_TYPE = ONE_FILE;
    IntIntext = FALSE;  
  }
  else {
    for( i = 0; ( c = getc( FptrFnames)) != EOF; i++) {
      if( c == '\n') 
        numFiles++;
    }

    if( ((fnameslist = (char **) malloc( numFiles * sizeof(char *))) == NULL)){
      perror( "Allocating fnameslist memory");
      return 0;
    }

    /* move pointer back to beginning of the filenames file */
    fseek( FptrFnames,0, SEEK_SET);  

    for ( i=0; i<numFiles; i++){

      if( ((fnameslist[i] = (char *) malloc( MAXWORDLEN*sizeof(char))) == NULL)){
        perror( "Allocating filename memory");
        return 0;
      }

      for( j = 0; ( c = getc( FptrFnames)) != EOF; j++) {
        if( c == '\n'){
           fnameslist[i][j] = '\0';
           break;
        }
        else
          fnameslist[i][j] = c;
      }
    }

    if( !my_fclose( &FptrFnames))
      die( "Can't close filenames file.\n");
   
    CORPUS_TYPE = MANY_FILES;
    IntIntext = TRUE; /* necessary for next_token */
  }

  linebuffer[0] = '\0';
  buffer_reader = linebuffer;
  return 1;
}

/*int close_tokenizer() {
 if( !my_fclose( &FptrCorpus))
      die( "Can't close corpus file.\n");
 
 return 1;
 }*/
  
/***************************************************
  next_token.
  
  Get the next word in the corpus.
  
  Try to copy the next word from the line currently held
  in line_buffer to the outbuffer.
  If lin_buffer doesn't have any more words, 
  invoke next_line to replenish it.

  Arguments:	1 outbuffer	the target buffer
				to copy the next word to
		2 curr_pos	the current file location
				in the corpus (will be recorded
				in the wordlist file at
				document breaks)

  Return Values:	-1 if next_line returns -1 
				(i.e., document break)
			 0 if next_line returns 0 
				(i.e., EOF encountered)
			 1 when returning a word.
			 */
int next_token( char *outbuffer, int *curr_pos) {
  int i;
  int in_word = FALSE;
  int my_isalpha( int c) { return valid_chars[c]; };

  while( !in_word) {
    /* Go to the beginning of the next word. */
    while( (buffer_reader[0] != '\0')  &&
	   !(my_isalpha( (int) *buffer_reader))) {

      if( CORPUS_TYPE == ONE_FILE) {

        /* End of a document; return INT_E_DOC_TAG to indicate this. */
        if( !strncmp( buffer_reader, E_DOC_TAG,
		      strlen(E_DOC_TAG))) {
          buffer_reader += strlen( E_DOC_TAG);
          IntIndoc = FALSE;
          return INT_E_DOC_TAG;
        }
        /* End of text */
        if( !strncmp( buffer_reader, E_TEXT_TAG,
		      strlen(E_TEXT_TAG))) {
          buffer_reader += strlen( E_TEXT_TAG);
          IntIntext = FALSE;
          return INT_E_TEXT_TAG;
        }
        /* Beginning of a document; return INT_B_DOC_TAG to indicate this. */
        if( !strncmp( buffer_reader, B_DOC_TAG,
		      strlen(B_DOC_TAG))) {
          buffer_reader += strlen( B_DOC_TAG);
          IntIndoc = TRUE;
          IntDoccntr++;
          return INT_B_DOC_TAG;
        }
        /* Beginning of text */
        if( !strncmp( buffer_reader, B_TEXT_TAG,
		      strlen(B_TEXT_TAG))) {
          buffer_reader += strlen( B_TEXT_TAG);
          IntIntext = TRUE;
          return INT_B_TEXT_TAG;
        }
      }

      buffer_reader++;

    }

    /* Do we have a word? */
    if( IntIntext && my_isalpha( (int) *buffer_reader)) {
      in_word = TRUE;
    }
    
    /* Otherwise we need to read a fresh line. */
    else {
      /* buffer_reader = linebuffer;*/
      if( !next_line( linebuffer, curr_pos))
	return 0;
      buffer_reader = linebuffer;
    }
  }
    
  /* Copy the next word to the return buffer. */
  for( i=0; my_isalpha( (int) *buffer_reader); buffer_reader++) {
    /* Truncate monster words. */
    if( i >= MAXWORDLEN) {
      outbuffer[MAXWORDLEN] = '\0';
      /*fprintf( stderr, "Word near fpos %d exceeding buffer size;"
	       "truncated:\n%s\n",
	       *curr_pos, outbuffer);*/
      return 1;
    }

    /* Copy along. */
    outbuffer[i++] = tolower( *buffer_reader);
    
    /* Use this to keep word-internal apostrophes. */
    if( buffer_reader[1] == '\'' && my_isalpha( (int) buffer_reader[2])) {
      outbuffer[i++] = '\'';
      buffer_reader++;
    }
  } 
  
  outbuffer[i] = '\0';
  return 1;
}

  
/**************************************************
  next_line.

  Get the next line in the corpus file.
  Read a line, but return it only if it is part of the text.
  Markup and document headers are filtered out here.
  This is heavily corpus-dependent.
  Tags are defined in preprocessing_env.h, but they may not generalize
  easily to other corpora.

  Arguments:	1 linebuffer	the buffer to which the line 
				is written
		2 curr_pos	the current file location

  Return Values:	-1 if a tag indicating a new document
				is encountered
			 0 if we don't have a line.
			 1 if everything is going fine.
			 */
int next_line( char *linebuffer, int *curr_pos) {
  int i;
  char *str;

  /* Do we have a new line? */
  while( read_line( FptrCurrent, linebuffer, curr_pos)) {
    
    /* Inspect the newly read line: */
    /* is it empty? */

    if( strlen( linebuffer) == 0) {
      continue;
    }
    /* filter out ignorable stuff */
    for( i=0; ignore[i] != NULL; i++) 
      while( ( str = strstr( linebuffer, ignore[i])) != NULL)
	memset( (void *) str, ' ', strlen( ignore[i]));
    
    /* If all else goes through, deliver the line. */
    return 1;
  }
  /* If we are at EOF */
  return 0;
}


/**************************************************
  read_line.

  Read one line from the corpus.
  Breaks if the corpus has lines longer than MAXLINELEN.
  Considers EOF and \n as line delimiters,
  could be generalized to others.
  Reads lines one character at a time.

  Arguments:	1 infile	file pointer
		2 INPUT_LINE	the buffer where the line is
				to be recorded
		3 curr_pos	the current file location

  Return Values:	0 if the line is empty or
				or we reached EOF
			1 if all goes well.
			*/
int read_line( FILE *infile, char *INPUT_LINE, int *curr_pos) {
  int i, cnt;
  char c;

  *curr_pos = ftell(infile);
  for( i = 0; ( c = getc( infile)) != EOF; i++) {
    if( c == '\n') {
      INPUT_LINE[i] = '\0';
      return 1;
    }
    else
      INPUT_LINE[i] = c;

    /* Truncate monster lines. */
    if( i >= MAXLINELEN) {
      /*for( cnt=0; 
	   ( my_isalpha( (int) INPUT_LINE[i] )
	     || INPUT_LINE[i] == '\'' ); 
	   cnt++, i--);
      fseek( infile, -1 * (cnt * sizeof( char)), SEEK_CUR);
      */

      INPUT_LINE[i] = '\0';
      /*fprintf( stderr, "Line near %d exceeding buffer size; truncated:\n%s\n",
       *curr_pos, INPUT_LINE);*/
      return 1;
    }
  }

  /* If we reached EOF and the line is non-empty */
  if( i) {
    INPUT_LINE[i] = '\0';
    return 1;
  }
  /* if we reached EOF and the line is empty */
  else
    return 0;
}













