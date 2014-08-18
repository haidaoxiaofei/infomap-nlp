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

#ifndef TOC_H
#define TOC_H   1

#define NUM_TOC_FIELDS  4

typedef struct toc {

  /** Short tag that can be used to 
      refer to this TOC entry  **/
  char *tag;

  /** Longer descriptive name of
      the model to which this
      entry corresponds.  **/
  char *name;

  /** Directory containing the 
      data for the model this
      TOC entry describes. **/
  char *dir;

  /** Freeform description of 
      the model. **/
  char *descrip;

  struct toc* next_entry;

} TOC;

/**
 *  Read the table of contents (TOC) file toc_filename, which should have 
 *  one or more entries in the following format:
 * 
 *  TAG: <tag>
 *  NAME: <name>
 *  DIR: <dir>
 *  DESCRIP: <descrip>
 *
 *  separated from each other by blank lines.  Create a linked
 *  list of TOC structures, with one struture for each entry
 *  in toc_filename; return a pointer to that list.
 *
 *  Returns NULL to indicate failure.
 */
TOC *read_toc(const char *toc_filename);

/**
 *  Read through toc looking for entry with model tag
 *  model_tag.  Return pointer to the model data dir
 *  (the DIR field) for that TOC entry.  Return
 *  NULL to indicate that no matching entry was found.
 */
char *find_model_in_toc( TOC *toc, char *model_tag );

#endif /* !TOC_H */


