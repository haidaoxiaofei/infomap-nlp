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

#ifndef PREPROCESSING_ENV_H
#define PREPROCESSING_ENV_H  1

#define MAXWORDLEN BUFSIZ
#define MAXLINELEN (3 * BUFSIZ)

/* Start labeling columns from the
   START_COLUMNS'th non-stopped dictionary entry. */
#define START_COLUMNS 50

/* Buffer size for reading the wordlist file
   (must be larger than the wordcount in the longest document!.) */
#define BLOCKSIZE 1000000

/**************************************************
  CORPUS FILE INFO
  **************************************************/
/* Corpus Tags.  May list tags for different corpora, as long
   as they don't cause confusion.
   Given as strings. */
#define B_DOC_TAG  "<DOC>"
#define E_DOC_TAG  "</DOC>"
#define B_TEXT_TAG "<TEXT>"
#define E_TEXT_TAG "</TEXT>"
/* In-text stuff to be ignored. */
#define IGNORE_TXT  {"<p>", \
  "&MD;", "&LR;", "&UR;", "&QL;", "&QC;", "&QR;", "&TL;", \
    NULL}

#define WORDLIST_B_DOC_TAG "<d>"
#define WORDLIST_E_DOC_TAG "</d>"
#define WORDLIST_B_TEXT_TAG "<t>"
#define WORDLIST_E_TEXT_TAG "</t>"
#define WORDLIST_B_FLOC_TAG "<f>"
#define WORDLIST_E_FLOC_TAG "</f>"

/* Integers to be used by the tokenizer to signal the beginning and
   end of a document */
#define INT_B_DOC_TAG -1
#define INT_E_DOC_TAG -2
#define INT_B_TEXT_TAG -3
#define INT_E_TEXT_TAG -4
#define INT_B_FLOC_TAG -5
#define INT_E_FLOC_TAG -6

#endif /* !PREPROCESSING_ENV_H */
