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

#ifndef MODEL_PARAMS_H
#define MODEL_PARAMS_H   1

#include <stdio.h>

#ifndef ONE_FILE
#define ONE_FILE    0
#endif

#ifndef MANY_FILES
#define MANY_FILES  1
#endif

typedef struct {

  int corpus_type;  /** ONE_FILE or MANY_FILES **/
  char corpus_dir[BUFSIZ]; 
  char corpus_filename[BUFSIZ]; /** should contain CORPUS_FILE if this is
				    a ONE_FILE corpus; FNAMES_FILE if
				    this is a MANY_FILES corpus **/
  int rows;
  int singvals;

} MODEL_PARAMS;

typedef struct {

  int columns;
  int start_columns;
  int col_labels_from_file;
  int pre_context_size;
  int post_context_size;
  int blocksize;
  int svd_iter;

} MODEL_INFO;

typedef struct {

  char *b_doc_tag;
  char *e_doc_tag;
  char *b_text_tag;
  char *e_text_tag;
  
  char **ignore_text;
  
  char *wordlist_b_doc_tag;
  char *wordlist_e_doc_tag;
  char *wordlist_b_text_tag;
  char *wordlist_e_text_tag;
  char *wordlist_b_floc_tag;
  char *wordlist_e_floc_tag;

} CORPUS_FORMAT;

/**
 *  Write the MODEL_PARAMS object pointed to by model_params
 *  to filename as raw data.
 *  Returns 1 to indicate success and 0 to indicate failure.
 */
int write_model_params(const char *filename, 
		       const MODEL_PARAMS *model_params);

/**
 *  Read the MODEL_PARAMS object stored in filename and store
 *  it at the address contained in model_params.
 *  Returns 1 to indicate success and 0 to indicate failure.
 */
int read_model_params(const char *filename, MODEL_PARAMS *model_params);

/**
 *  Print the model params stored in the MODEL_PARAMS object
 *  pointed to by model_params to the stdio stream fp in
 *  a textual format.
 *  If any of the last three arguments are "NULL", no attempt
 *  to print that structure will be made.
 */
void print_model_params(FILE *fp, const MODEL_PARAMS *model_params,
			const MODEL_INFO *model_info,
			const CORPUS_FORMAT *corpus_format);

/**
 *  Write the MODEL_INFO object pointed to by model_info
 *  to filename as raw data.
 *  Returns 1 to indicate success and 0 to indicate failure.
 */
int write_model_info(const char *filename, const MODEL_INFO *model_info);

/**
 *  Read the MODEL_INFO object stored in filename and store
 *  it at the address contained in model_info.
 *  Returns 1 to indicate success and 0 to indicate failure.
 */
int read_model_info(const char *filename, MODEL_INFO *model_info);

/**
 *  Write the CORPUS_FORMAT object pointed to by corpus_format
 *  to filename as raw data.
 *  Returns 1 to indicate success and 0 to indicate failure.
 */
int write_corpus_format(const char *filename, 
			const CORPUS_FORMAT *corpus_format);

/**
 *  Read the CORPUS_FORMAT object stored in filename and store
 *  it at the address contained in corpus_format.
 *  Returns 1 to indicate success and 0 to indicate failure.
 */
int read_corpus_format(const char *filename, CORPUS_FORMAT *corpus_format);

/**
 *  Frees the dynamically allocated storage associated with
 *  a CORPUS_FORMAT object.
 */
void free_corpus_format(CORPUS_FORMAT *corpus_format);

#endif /* !MODEL_PARAMS_H */
