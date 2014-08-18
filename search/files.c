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
  files.c

  Routines to open and close files and (dbm) hash files.
  We currently hold all files pointers in a single structure.
  This is neither required nor particularly efficient, but
  very neat and easy to handle.

  Stefan Kaufmann, March 2001
  **************************************************/

#include "files.h"
#include "filenames.h"
#include <stdio.h>
#include <sys/fcntl.h>

/**************************************************
  open_files

  Open all the files.

  ARGUMENTS:	1 Files		   the structure holding the pointers
		2 model_data_dir   the name of the model data directory

  RETURN VALUES:	0	if any of the files could not be opened
			1	if all goes well
			*/
int open_files( FILES *Files, char *model_data_dir) {
  char pathbuf[BUFSIZ];

  /* Open the database mapping words to their indices
     (which is their line number in the dictionary) */
  sprintf( pathbuf, "%s/%s", model_data_dir, WORD2OFFSET_FILE );
  if (( Files->w2o = dbm_open( pathbuf, O_RDONLY, 0644 )) == NULL ) {
    fprintf( stderr,"Can't Open %s\n", pathbuf );
    return 0;
  }
  
  /* Open the database mapping word indices to words */
  sprintf( pathbuf, "%s/%s", model_data_dir, OFFSET2WORD_FILE );
  if( ( Files->o2w = dbm_open( pathbuf, O_RDONLY, 0644 )) == NULL) {
    fprintf(stderr,"Can't open %s\n", pathbuf);
    return 0;
  }
  
  /* The binary file holding word vectors
     (must be recompiled for use by different OS's.) */
  sprintf( pathbuf, "%s/%s", model_data_dir, WORDVECBIN_FILE );
  if( ( Files->wordvecbin = fopen( pathbuf, "r")) == NULL) {
    fprintf(stderr,"Can't Open %s\n", pathbuf);
    return 0;
  }

  /* The binary file holding article vectors
     (must also be recompiled for use by different OS's.)  */
  sprintf( pathbuf, "%s/%s", model_data_dir, ARTVECBIN_FILE );
  if( ( Files->artvecbin = fopen( pathbuf, "r")) == NULL) {
    fprintf( stderr, "Can't open %s\n", pathbuf);
    return 0;
  }
  
  /* The database mapping article indices to their offsets in the corpus */
  sprintf( pathbuf, "%s/%s", model_data_dir, ART2OFFSET_FILE );
  if( ( Files->a2o = dbm_open( pathbuf, O_RDONLY, 0644 )) == NULL) { 
    fprintf( stderr, "Can't open %s\n", pathbuf);
    return 0;
  }

  /* Open the database mapping article vector offsets to article ID's */
  sprintf( pathbuf, "%s/%s", model_data_dir, OFFSET2ART_FILE );
  if( ( Files->o2a = dbm_open( pathbuf, O_RDONLY, 0644 )) == NULL) {
    fprintf(stderr,"Can't open %s\n", pathbuf);
    return 0;
  }
  
  
  
  return 1;
}


/**************************************************
  close_files

  Close all the files

  ARGUMENTS:	1 FILES		the structure holding all the pointers
  RETURN VALUES:	1
  */
int close_files( FILES *Files) {
  dbm_close( Files->w2o);
  dbm_close( Files->o2w);
  dbm_close( Files->a2o);
  dbm_close( Files->o2a);
  fclose( Files->wordvecbin);
  fclose( Files->artvecbin);
  return 1;
}
