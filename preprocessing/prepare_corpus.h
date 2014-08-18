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
  prepare_corpus.c

  converts a text corpus to usable format.
  Stefan Kaufmann, July 2000

**************************************************/

#ifndef PREPARE_CORPUS_H
#define PREPARE_CORPUS_H	1

#include "env.h"
#include "preprocessing_env.h"
#include "filenames.h"
#include "word_tree.h"
#include "tokenizer.h"
#include "stopper.h"
#include "utils.h"
#include "report.h"


/**************************************************
  DEFINITIONS
  **************************************************/

/**************************************************
  GLOBAL VARIABLES
  **************************************************/

long int word_cntr, doc_cntr = 0;	/* Used in reporting and to
					   calculate document frequency. */
char message_buffer[MAXLINELEN];

/**************************************************
  FUNCTION DECLARATIONS
  **************************************************/

/* Create or update the info slot kept in the word's node in the tree. */
WORDINFO *increment_data( WORDINFO **current);

/* Write an entry to the dictionary */
int write_dictionary( NODE **node_array, int typecount, char *model_data_dir );

/* For reporting. */
void report_reading( char *buffer);
void report_sorting( char *buffer);
void report_converting( char *buffer);

int write_model_data( const char *model_data_dir, const char *corpus_dir,
		      const char *corpus_file, const char *fnames_file,
		      int typecount );
#endif
