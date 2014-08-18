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

#include "toc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READING_BUFSIZ  (BUFSIZ * NUM_TOC_FIELDS)
#define TAG_PREFIX "TAG:"
#define NAME_PREFIX "NAME:"
#define DIR_PREFIX "DIR:"
#define DESCRIP_PREFIX "DESCRIP:"

static FILE *toc_file = NULL;
int get_next_record(char *record_buf);
int parse_record(char *record_buf, char *tag_buf, char *name_buf,
		 char *dir_buf, char *descrip_buf);
int skip_space(int i, const char *buffer);
int populate_buffer(int offset, char *dest_buf, const char *source_buf);

/**
 *  See toc.h for comments.
 */
TOC *read_toc(const char *toc_filename) {

  /* buffer to hold a single record before parsing
   * it to fill tag_buf, name_buf, dir_buf, and descrip_buf.
   */
  char record_buf[BUFSIZ * NUM_TOC_FIELDS + 1];

  /* these four buffers are filled by parsing
   * record_buf; their contents are then used
   * to create the TOC structure last_rec. */
  char tag_buf[BUFSIZ];
  char name_buf[BUFSIZ];
  char dir_buf[BUFSIZ];
  char descrip_buf[BUFSIZ];

  /* pointer to the linked list we return */
  TOC *retval = NULL;
  /* TOC structure we are in the process of filling,
   * or have just finished filling. */
  TOC *last_rec = NULL;

  toc_file = fopen(toc_filename, "r");
  if ( toc_file == NULL ) {
    perror("toc.c: read_toc: can't open TOC file");
    return NULL;
  }

  /*
   * 1. fill record_buf with next record from toc_filename.
   * 2. parse record_buf to fill tag_buf, name_buf, dir_buf,
   *    and descrip_buf
   * 3. allocate, fill, and add the next TOC element to 
   *    the linked list.
   */
  while (get_next_record(record_buf)) {

    if (parse_record(record_buf, tag_buf, name_buf, dir_buf, descrip_buf) != 0)
      return NULL;

    if (last_rec == NULL) {
      /* allocate the first elem of the linked list. */
      retval = last_rec = malloc( sizeof (TOC) );
    } else {
      /* add new entry to end of linked list. */
      last_rec->next_entry = malloc( sizeof (TOC) );
      last_rec = last_rec->next_entry;
    }

    last_rec->tag = strdup(tag_buf);
    last_rec->name = strdup(name_buf);
    last_rec->dir = strdup(dir_buf);
    last_rec->descrip = strdup(descrip_buf);
    last_rec->next_entry = NULL;
  }
  
  fclose(toc_file);
  return retval;
}

/**
 *  get_next_record(char *record_buf)
 *  ---------------------------------
 *
 *     Fill record_buf with the next record in the file
 *  pointed to by the external static variable toc_file,
 *  where records are separated by blank lines.  We rely
 *  on toc_file already having been opened.
 */
int get_next_record(char *record_buf) {

  static char reading_buf[READING_BUFSIZ];
  static size_t reading_buf_index = 0;
  static size_t bytes_read = 0;

  size_t record_buf_index = 0;
  int on_newline = 0, have_record = 0;

  while (1) {

    while (reading_buf_index < bytes_read) {
      record_buf[record_buf_index] = reading_buf[reading_buf_index++];
      if (record_buf[record_buf_index] == '\n') {
	if (on_newline) {
	  have_record = 1;
	  /** skip other blank lines. **/
	  while (reading_buf[reading_buf_index] == '\n')
	    reading_buf_index++;
	  break;
	}
	else on_newline = 1;
      } else on_newline = 0;
      record_buf_index++;
    }

    if (have_record) break;

    bytes_read = fread(reading_buf, 1, READING_BUFSIZ, toc_file);

    if (bytes_read <= 0) {
      break;
    }
    
  }

  if (record_buf_index > 0) have_record = 1;
  record_buf[record_buf_index] = '\0';
  return have_record;
}

/**
 *  int parse_record(char *record_buf, char *tag_buf, char *name_buf,
 *                   char *dir_buf, char *descrip_buf)
 *  -----------------------------------------------------------------
 *
 *  Parse the record stored in record_buf, placing the appropriate
 *  contents in tag_buf, name_buf, dir_buf, and descrip_buf.
 *
 *  Return 0 to indicate success; non-zero value to indicate failure.
 */
int parse_record(char *record_buf, char *tag_buf, char *name_buf,
		 char *dir_buf, char *descrip_buf) {

  static char *tag_prefix = TAG_PREFIX;
  static char *name_prefix = NAME_PREFIX;
  static char *dir_prefix = DIR_PREFIX;
  static char *descrip_prefix = DESCRIP_PREFIX;

  int i = skip_space(0, record_buf);
  i = match_fixed_portion(i, tag_prefix, record_buf);
  if (i == -1) {
    fprintf(stderr, "toc.c: parse_record: Error parsing record.\n");
    return 1;
  }

  i = skip_space(i, record_buf);
  i = populate_buffer(i, tag_buf, record_buf);
  i = skip_space(i, record_buf);
  
  i = match_fixed_portion(i, name_prefix, record_buf);
  if (i == -1) {
    fprintf(stderr, "toc.c: parse_record: Error parsing record.\n");
    return 1;
  }
  i = skip_space(i, record_buf);
  i = populate_buffer(i, name_buf, record_buf);
  i = skip_space(i, record_buf);

  i = match_fixed_portion(i, dir_prefix, record_buf);
  if (i == -1) {
    fprintf(stderr, "toc.c: parse_record: Error parsing record.\n");
    return 1;
  }
  i = skip_space(i, record_buf);
  i = populate_buffer(i, dir_buf, record_buf);
  i = skip_space(i, record_buf);

  i = match_fixed_portion(i, descrip_prefix, record_buf);
  if (i == -1) {
    fprintf(stderr, "toc.c: parse_record: Error parsing record.\n");
    return 1;
  }
  i = skip_space(i, record_buf);
  i = populate_buffer(i, descrip_buf, record_buf);
  
  return 0;
  
}

/*
 *  int populate_buffer(int offset, char *dest_buf, const char *source_buf)
 *  -----------------------------------------------------------------------
 *  Copies the contents of source_buf from source_buf[offset] up to the
 *  first newline ('\n'), exclusive, and terminates dest_buf with a
 *  null zero ('\0').  Also stops copying if it encounters a null
 *  zero in source_buf.
 *
 *  Returns the offset of the character in source_buf immediately 
 *  following the last character copied.
 *
 *  Notice that disaster can strike if dest_buf contains no newline and
 *  is not null-terminated, or if the number of characters before the
 *  first newline or null zero in dest_buf is greater than or equal to
 *  the length of dest_buf minus one.
 */
int populate_buffer(int offset, char *dest_buf, const char *source_buf) {
  int i = 0;

  while ((source_buf[offset] != '\n') && (source_buf[offset] != '\0')) {
    dest_buf[i++] = source_buf[offset++];
  }

  dest_buf[i] = '\0';
  return offset;
}

/* int skip_space(int i, const char *buffer)
 * -----------------------------------------
 * Skips any whitespace characters in buffer beginning
 * with the character buffer[i]; returns the index
 * of the character immediately following the last character
 * skipped.
 */
#define MY_SPACE_CHARS "\n\t "

int skip_space(int i, const char *buffer) {
  
  while (strchr(MY_SPACE_CHARS, buffer[i]) != NULL) i++;
  return i;
  
}

/* 
 * int match_fixed_portion(int i, const char *template, const char *tested)
 * ------------------------------------------------------------------------
 * Checks the string beginning at tested[i] to see if it is has the
 * prefix stored in the string template.  Returns the index of the
 * first character in tested after the prefix stored in template, or
 * -1 to indicate that template does not match the beginning of tested[i].
 */
int match_fixed_portion(int i, const char *template, const char *tested) {

  int j = 0;

  while (template[j] != '\0') {
    if (template[j++] != tested[i++]) {
      fprintf(stderr, "toc.c: match_fixed_portion: Templates don't match!\n");
      fprintf(stderr, "TEMPLATE = \"%s\"\n", template);
      fprintf(stderr, "TESTED = \"%s\"\n", tested);
      return -1;
    }
  }

  return i;

}

char *find_model_in_toc( TOC *toc, char *model_tag ) {

  while (toc != NULL) {
    if ( strcmp( model_tag, toc->tag ) == 0 ) {
      return ( toc->dir );
    } 
    toc = toc->next_entry;
  }

  return NULL;
}
