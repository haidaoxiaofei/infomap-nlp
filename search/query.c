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

#include "query.h"
#include "associate.h"
#include "files.h"

/**********************************************************************
  build_query_vector

  parse the command-line input and retrieve the vectors of all query words.
  assembles all 'positive' query terms in 'queryvector', then makes the
  result pairwise orthogonal to all 'negative' query terms.

  ARGUMENTS:	1 argc		argc
		2 argv		argv
		3 Files		pointers to files
		4 queryvector	the end result

  RETURN VALUES:	0 if memory allocation fails
			  or a subroutine is unsuccessful
			1 if all goes well
  **********************************************************************/
int build_query_vector( int Input, int argc, char *argv[],
			FILES *Files, MATRIX_TYPE *queryvector,
			int num_singvals, int corpus_type, char *model_data_dir) {
  MATRIX_TYPE *tmpvector, **neg_keyword_array;
  int cnt, i, j, k, return_value, neg_keywordsnum, query_exists = 0;
  double dot_product;

  /* Initialize the vectors */
  if( ( tmpvector = 
	( MATRIX_TYPE *) malloc( sizeof( MATRIX_TYPE) * num_singvals))
      == NULL) {
    fprintf( stderr, "query.c: Can't allocate vector memory.\n");
    return(0);
  }

  for( i=0; i<num_singvals; i++)
    tmpvector[i] = queryvector[i] = 0.0;

  /* An array for the negative keywords */
  if( ( neg_keyword_array = 
	( MATRIX_TYPE **) malloc( sizeof( MATRIX_TYPE*) * (argc - 1)))
      == NULL) {
    fprintf( stderr, "Can't allocate neg_keyword vector pointers.\n");
    return(0);
  }
  /* Set them to NULL (for now) */
  for( i = 0; i < (argc-1); i++)
    neg_keyword_array[i] = NULL;
  
  /* Count number of positive and negative keywords and read them into the 
     array */
  query_exists = 0;
  /* Collect what's before "NOT" */
  for( cnt = 1; (cnt < argc) && ( strcmp( argv[cnt], "NOT")); cnt++) {
    if( Input == WORDS) {
      return_value = get_word_vector( Files, argv[cnt], &tmpvector, num_singvals);
    }
    else { /* Input = Documents */
      return_value = get_doc_vector( Files, argv[cnt], &tmpvector, num_singvals, corpus_type, model_data_dir);
    }
    /* switch( get_word_vector( Files, argv[cnt], &tmpvector, num_singvals)) {*/
    switch( return_value) {
    case 1: 
      query_exists = 1;
      for( i=0; i<num_singvals; i++)
	queryvector[i] += tmpvector[i];
      break;
    case 0:
      fprintf( stderr, "Can't process positive keywords/documents.\n");
      return(0);
    case -1:
      fprintf( stderr, "No word/document vector for \"%s\".\n", argv[cnt]);
      break;
    default: continue;
    }
  }
  /* Collect what's after "NOT" */
  for( cnt++, neg_keywordsnum = 0; cnt < argc; cnt++) {
    if( Input == WORDS) {
      return_value = get_word_vector( Files, argv[cnt], &(neg_keyword_array[neg_keywordsnum]), num_singvals);
    }
    else { /* Input = DOCUMENTS */
      return_value = get_doc_vector( Files, argv[cnt], &(neg_keyword_array[neg_keywordsnum]), num_singvals);
    }
    /* switch( get_word_vector( Files, argv[cnt], 
			     &(neg_keyword_array[neg_keywordsnum]), num_singvals)) { */
    switch( return_value) {
    case 1: 
      neg_keywordsnum++;
      break;
    case 0:
      fprintf( stderr, "Can't process negative keywords/documents.\n");
      return(0);
    default: continue;
    }
  }

  /* Normalize the query vector */
  normalize( queryvector, num_singvals );

  /* Now we've got our positive and negative arguments, 
     subtract all the negative i.e. just enough to 
     make the output irrelevant to EVERY negative argument */

  /* For ease of computing, let the query vector be the last element
     of the neg_keywords vector. */
  neg_keyword_array[neg_keywordsnum] = queryvector;

  /* The orthogonalize function takes a list of n vectors and (using
     a Gram-Schmidt process) makes them orthogonal to one another. 
     This achieves the purpose of making our  word_array[neg_keywordsnum]
     orthogonal to all the 
     negative arguments */ 

  if ( neg_keywordsnum ) {  
    i = j = k = 0;
    
    /* Go up through vectors in turn, parameterized by k */
    for( k = 0 ; k <= neg_keywordsnum ; k++ ){
      /* Go up to vector k, parameterized by j */
      for( j = 0; j < k ; j++ ){
	dot_product = 0.0;
	/* Compute vector_k * vector_j */
	for( i = 0 ; i < num_singvals ; i++ ) 
	  dot_product += neg_keyword_array[k][i] * neg_keyword_array[j][i];
	/* Subtract relevant amount from vectors[k] */
	for( i = 0 ; i < num_singvals ; i++ )
	  neg_keyword_array[k][i] -= dot_product * neg_keyword_array[j][i];
      }
      /* normalize the vector we're working on */     
      normalize( neg_keyword_array[k], num_singvals );  
    }
  }
  return 1;
}

/***************************************************
  get_word_vector

  Takes a word, puts coordinates into vector

  ARGUMENTS:	1 Filenames
                2 string	the target word
		3 vector	pointer to the target vector
		4 vector_length	
  RETURN VALUES:	-1 if the word is not in the database
			 0 if memory could not be allocated
			 1 if all goes well
  **************************************************/
int get_word_vector( FILES *Files, char *string, MATRIX_TYPE **vector,
		     int vector_length) {
  datum key, content;
  int offset,i,cnt,flg=0;

  key.dptr = string;
  key.dsize = strlen( string);
  
  content = dbm_fetch( Files->w2o, key);

  /* If we don't have that word */
  if( content.dptr == NULL)
    return(-1);

  /* Make a vector if the input isn't one already */
  if( *vector == NULL)
    if( ( *vector = ( MATRIX_TYPE *) 
	  malloc( sizeof( MATRIX_TYPE) * vector_length))
	== NULL) {
      fprintf( stderr, "Can't allocate vector memory.\n");
      return(0);
    }
 
  /* Read the vector */
  memcpy( &offset , content.dptr , sizeof(offset));

  if ( fseek( Files->wordvecbin, offset , SEEK_SET ) < 0 ) {
    perror( "query.c: get_word_vector: fseek()" );
  }
  if ( (fread( *vector , sizeof( MATRIX_TYPE) * vector_length, 1,
	     Files->wordvecbin)) != 1) {
    if (ferror(Files->wordvecbin)) {
      fprintf(stderr, "Error reading file.\n");
    }

    fprintf( stderr, "query.c: get_word_vector: can't read word vector for "
	     "\"%s\".\n", string );
    return 0;
  }

  return(1);
}


/***************************************************
  get_doc_vector

  Takes a document, puts coordinates into vector

  ARGUMENTS:    1 Files          structure containing pointers to files in data directory
                2 string         the target doc
                3 vector         pointer to the target vector
                4 vector_length 
  RETURN VALUES:        -1 if the doc is not in the database
                         0 if memory could not be allocated
                         1 if all goes well
  **************************************************/
int get_doc_vector( FILES *Files, char *string, MATRIX_TYPE **vector, int vector_length, int corpus_type, char *model_data_dir) {
  datum key, key2, content;
  int offset,i,cnt,flg=0;
  char pathbuf[BUFSIZ];
  DBM *a2o, *na2nu;
  FILE *artvecbin;


  /* if corpus consists of many files */
  if( corpus_type == MANY_FILES ) {

    key.dptr = string; 
	/* file name (if corpus consists of many files) or offset in wordlist (if corpus is one big file) */
    key.dsize = strlen( string);

    /* Open the database mapping file names to file ids */
    sprintf( pathbuf, "%s/%s", model_data_dir, NAME2NUMBER_FILE );
    if( ( na2nu = dbm_open( pathbuf, O_RDONLY, 0644 )) == NULL) {
      fprintf(stderr,"Can't open %s\n", pathbuf);
      return 0;
    }
    key = dbm_fetch( na2nu, key); /* file id */
    /* If we don't have that doc */
    if( key.dptr == NULL) {
      return(-1);
    }
    dbm_close(na2nu);
  }
  else {
    i = atoi(string);
    key.dptr = (char*) &i;
    key.dsize = sizeof(i);
  }

  content = dbm_fetch( Files->a2o, key ); /* offset in artvecbin */
  /* If we don't have that doc */
  if( content.dptr == NULL) {
    return(-1);
  }


  /* Make a vector if the input isn't one already */
  if( *vector == NULL)
    if( ( *vector = ( MATRIX_TYPE *)
          malloc( sizeof( MATRIX_TYPE) * vector_length))
        == NULL) {
      fprintf( stderr, "Can't allocate vector memory.\n");
      return(0);
    }

  /* Read the vector */
  memcpy( &offset , content.dptr , content.dsize);
  if( fseek( Files->artvecbin, offset , 0 ) < 0) 
    perror( "query.c: get_doc_vector: fseek()" ); 

  if ( (fread( (char *) *vector , sizeof( MATRIX_TYPE) * vector_length, 1,
         Files->artvecbin)) != 1) {
    if( ferror( Files->artvecbin)) 
      fprintf(stderr, "Error reading file.\n");

    fprintf( stderr, "query.c: get_word_vector: can't read word vector for "
             "\"%s\".\n", string );
    return(0);
  }
  return(1);
}


