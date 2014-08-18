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

#include "associate.h"
#include "search_utils.h"
#include "files.h"
#include "filenames.h"
#include "model_params.h"
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* Tasks. */
#define ASSOCIATE_WORDS      0
#define ASSOCIATE_DOCUMENTS  1
#define PRINT_QUERY_VECTOR   2

/***************************************************
  Main 
  **************************************************/


/** file-local functions **/
int check_args( char next_arg, int total_remaining_args );
int usage();
void print_query_vector( MATRIX_TYPE query_vector[], int num_elems );
/**************************/

static MODEL_PARAMS model_params;

int main(int argc, char **argv) {
  DBM *nu2na; /* hash table (only needed if corpus_type==MANY_FILES) 
                 key: index of corpus file, content: name of file */
  MATRIX_TYPE *queryvector;
  int Task = ASSOCIATE_WORDS;
  int Input = WORDS;
  int results = MAX_NEIGHBOR;
  int num_results_printed = PRINT_NEIGHBOR;
  char *corpus_tag = NULL;
  datum key, content, content2;
  char pathbuf[BUFSIZ];
  int output_vector_file = 0;          /* false; set to true if we
					* encounter -f command line
					* option */
  char *vector_output_filename = NULL; /* should be specified following
					* -f option */

  char *model_dir_parent = NULL;
  char *model_dir = NULL;
  char *model_path_val;

  int use_working_dir = 0;  /* Look for model dir in
			     * working dir, rather than model path
			     */
  int have_m_opt = 0;

  struct stat statbuf;

  /* Variable holding the linked list */
  LIST_MEMBER *list = NULL, *tmpmember;

  /* Variables holding all file pointers */
  FILES Files;  // Source files from which original query is built 
  FILE *vectorfile;

  /**********************************
  Parse command line options.  
  If multiple tasks are given, the last one.
  **********************************/
  while( (argc > 1) && (argv[1][0] == '-')) {
    switch( argv[1][1] ) {

    case 'w':
      Task = ASSOCIATE_WORDS;
      break;

    case 'd':
      Task = ASSOCIATE_DOCUMENTS;
      break;

    case 'q':
      Task = PRINT_QUERY_VECTOR;
      break;

    case 'i':
      if ( argc < 3 ) {
        fprintf( stderr, "\n-i option must be followed by space "
                 "and one of \n\td (for sequence of documents)\n\tw (for sequence of words)" )
; 
        usage();                        
      }                                 
      ++argv;
      --argc;                           
      if( strcmp( argv[1], "d") == 0) {
        Input = DOCUMENTS;
      }
      else {
        if(  strcmp( argv[1], "w") == 0) {
          Input = WORDS;
        }                    
        else {               
           fprintf(stderr, "\nOption -i has to be followed by one of \"d\" or \"w\".\n");
           usage();
        }
      }
      break;
      
    case 'n':
      if ( argc < 3 ) {
	fprintf( stderr, "\n-n option must be followed by space "
		 "and num neighbors to print.\n" );
	usage();
      }
      ++argv;
      --argc;
      if (atoi(argv[1]) <= 0) {
	fprintf(stderr, "\nPlease specify a positive number of neighbors.\n");
	usage();
      }

      if (atoi(argv[1]) <= MAX_NEIGHBOR) {
	num_results_printed = atoi(argv[1]);
      } else {
	num_results_printed = MAX_NEIGHBOR;
	fprintf( stderr, "Number of neighbors can be at most %d; printing %d.\n",
		 MAX_NEIGHBOR, MAX_NEIGHBOR );
	fprintf( stderr, "(Edit associate.h and recompile to change.)\n" );
      }
      break;

  /* For the 'corpus' flag, look at the next argument for the corpus stem */
    case 'c':
      if ( argc < 3 ) {
	fprintf( stderr, "\n-c option must be followed by space " 
		 "and corpus tag.\n" );
	usage();
      }
      /* check for enough args and properly formatted corpus name */
      if ( !check_args( argv[2][0], argc ) )
	usage();
      ++argv;
      --argc;
      corpus_tag = argv[1];
      break;

    case 'm':
      if ( argc < 3 ) {
	fprintf( stderr, "\n-m option must be followed " 
		 "by space and model directory name.\n" );
	usage();
      }
      if ( !check_args( argv[2][0], argc ) )
	usage();
      ++argv;
      --argc;
      have_m_opt = 1;
      model_dir_parent = argv[1];
      break;

    case 'f':
      if ( argc < 3 ) {
	fprintf( stderr, "\n-f option must be followed by space " 
		 "and vector output filename.\n" );
	usage();      
      }
      /* check for a bad file name */
      if ( !check_args( argv[2][0], argc ) )
	usage();
      /* advance to the next argument, which had better be our filename */
      ++argv;
      --argc;
      output_vector_file = 1;  /* true */
      vector_output_filename = argv[1];
      break;

    case 't':
      use_working_dir = 1;
      break;

    case 'h':
      usage();
      break;

    default:
      fprintf( stderr, "Bad option: %s\n", argv[1]);
      usage();
    }
    /* Move the argument list ahead. 
       The options don't count any longer as input terms. 
       This works only if all options precede the input terms! */
    ++argv;
    --argc;
  }


  /* Was I invoked without term list(s)? */
  if(argc <  2) {
    fprintf( stderr, "\nNo query terms specified.\n\n" );
    usage();
  }

  if (model_dir_parent == NULL) {
    if (use_working_dir) {
      model_dir_parent = getenv(WORKING_DIR_VAR);
      if (model_dir_parent == NULL) {
	fprintf( stderr, "-t specified, but %s "
		 "undefined.\n"
		 "Use -h option for help.\n", WORKING_DIR_VAR);
	exit( 2 );
      }
    } else {
      if ( corpus_tag != NULL ) {
	model_dir_parent = search_model_path( corpus_tag );
	if (model_dir_parent == NULL) {
	  fprintf( stderr, "Could not find model with tag \"%s\" in "
		   "Infomap model path.\n", corpus_tag );

	  if ((model_path_val = get_model_path()) != NULL) {
	    fprintf( stderr, "Using model path: %s\n", model_path_val );
	  }

	  fprintf( stderr, "Use -h option for help.\n" );
	  exit( 2 );
	}
      }
    }
  }

  /* If we still haven't found it, something is wrong. */
  if (model_dir_parent == NULL) {
    fprintf( stderr, "Could not find a model.  The -t, -c, and -m options and \n"
	     "the INFOMAP_MODEL_PATH and INFOMAP_WORKING_DIR environment \n"
	     "variables relate to finding models.\n"
	     "Use the -h option for a usage summary, or consult the \n"
	     "associate(1) manual page.\n");

    exit( 2 );
  }


  /****************************************
  Finished parsing command line options
  ****************************************/

  if (corpus_tag != NULL) {
    sprintf( pathbuf, "%s/%s", model_dir_parent, corpus_tag );
    model_dir = strdup( pathbuf );
  } else {
    if (have_m_opt) {
      model_dir = model_dir_parent;
    }
  }

  if (!((stat(model_dir, &statbuf) == 0) && (S_ISDIR(statbuf.st_mode)))) {

    if (have_m_opt) {
      if (corpus_tag == NULL) {
	fprintf( stderr, "Model directory \"%s\" given as argument to -m\n"
		 "option does not exist, or is not a directory.\n",
		 model_dir );
      } else {
	fprintf( stderr, "Can't find model %s in parent directory \"%s\".\n",
		 corpus_tag, model_dir_parent );
	
	fprintf( stderr, "Using parent dir given as argument to -m option.\n" );
      }
    } else if (use_working_dir) {
      if (corpus_tag == NULL) {
	fprintf( stderr, "When using the -t option, the -c option must be \n"
		 "used to specify a model tag.\n" );
      } else {
	fprintf( stderr, "Can't find model %s in parent directory \"%s\".\n",
		 corpus_tag, model_dir_parent );

	fprintf( stderr, "Using working directory from INFOMAP_WORKING_DIR.\n");
	fprintf( stderr, "(As requested by -t option.)\n" );
      }
    }

    exit( EXIT_FAILURE );
  }

  sprintf( pathbuf, "%s/%s", model_dir, MODEL_PARAMS_BIN_FILE );
  if ( !read_model_params( pathbuf, &model_params )) {
    fprintf( stderr, "Can't open model parameter file \"%s\".\n", pathbuf );
    exit( EXIT_FAILURE );
  }

  if ( (queryvector = malloc( model_params.singvals 
			      * sizeof(MATRIX_TYPE) )) == NULL ) {
    fprintf( stderr, 
	     "associate.c: Can't allocate memory for query vector.\n" );
    exit( EXIT_FAILURE );
  }

  /* Open all files */
  if( !open_files( &Files, model_dir )) {
    fprintf( stderr, "Can't open Files.\n" );
    exit( EXIT_FAILURE );
  }

  if( model_params.corpus_type == MANY_FILES ) {
    sprintf( pathbuf, "%s/%s", model_dir, NUMBER2NAME_FILE );
    if (( nu2na = dbm_open( pathbuf, O_RDONLY, 0644 ))
         == NULL ) {
      perror( "Can't open nu2na database" );
      exit( EXIT_FAILURE );
    }
  }

  /* Build a query vector out of the positive and negative input terms */
  if( !build_query_vector( Input, argc, argv, &Files, queryvector, model_params.singvals, model_params.corpus_type, model_dir )) {
    fprintf( stderr, "associate.c: Can't build query vector.\n" );
    exit( EXIT_FAILURE );
  }
  
  /* Find the nearest neighbors */
  switch( Task ) {
  case ASSOCIATE_WORDS:
    vectorfile = Files.wordvecbin;
    break;
  case ASSOCIATE_DOCUMENTS:
    vectorfile = Files.artvecbin;
    break;
  case PRINT_QUERY_VECTOR:
    print_query_vector( queryvector, model_params.singvals );
    exit( EXIT_SUCCESS );
    break;
  default:
    fprintf( stderr, "No task defined.\n");
    exit( EXIT_FAILURE );
  }

  if( !find_neighbors( vectorfile, &list, results, 
		       queryvector, model_params.singvals )) {
    fprintf( stderr, "Can't find nearest neighbors.\n");
    exit( EXIT_FAILURE );
  }

  /* Fill in information. */

  tmpmember = list;
  while( tmpmember != NULL ) {

    switch( Task ) {

    case ASSOCIATE_WORDS:
      key.dptr = (char *) &( ((NEIGHBOR_ITEM *) tmpmember->data)->offset);
      key.dsize = sizeof( ((NEIGHBOR_ITEM *) tmpmember->data)->offset);
      content = dbm_fetch( Files.o2w, key );
      memcpy( ((NEIGHBOR_ITEM *) tmpmember->data)->string, 
	      content.dptr, content.dsize );
      (((NEIGHBOR_ITEM *) tmpmember->data)->string)[content.dsize] = '\0';
      break;

    case ASSOCIATE_DOCUMENTS:
      key.dptr = (char *) &( ((NEIGHBOR_ITEM *) tmpmember->data)->offset);
      key.dsize = sizeof( ((NEIGHBOR_ITEM *) tmpmember->data)->offset);
      content = dbm_fetch( Files.o2a, key );

      /* if corpus is one big file */
      if( model_params.corpus_type == ONE_FILE ) {
        sprintf( ((NEIGHBOR_ITEM *) tmpmember->data)->string, "%d", 
	       *( (int*) content.dptr));
      }
      /* if corpus consists of many files */
      else {
        content2 = dbm_fetch( nu2na, content);
        memcpy( ((NEIGHBOR_ITEM *) tmpmember->data)->string, 
	          content2.dptr, content2.dsize);
        (((NEIGHBOR_ITEM *) tmpmember->data)->string)[content2.dsize] = '\0';
      }
      break;

    default:
      fprintf( stderr, "No task defined.\n");
      exit( EXIT_FAILURE);
    }
    tmpmember = tmpmember->next;
  }

  /* if -f option and filename were specified, output word vectors
   to the appropriate file 
  */
  if (output_vector_file) {
    if ( !output_array( &list, vector_output_filename, model_params.singvals )){
      fprintf( stderr, "Can't output vectors to file:  %s.\n",
	       vector_output_filename);
      exit( EXIT_FAILURE );
    }
  }

  /* Print the result. */
  if ( !print_list( list, num_results_printed )) {
    fprintf( stderr, "Can't print output.\n");
    exit( EXIT_FAILURE);
  }

  /* Wrap up */
  if ( !close_files( &Files )) {
    fprintf( stderr, "Can't close Files.\n" );
    exit( EXIT_FAILURE );
  }

  if ( model_params.corpus_type == MANY_FILES )
    dbm_close( nu2na );

  /* Retire */
  exit( EXIT_SUCCESS );
}

/******************************************************
usage function in case args are wrong
******************************************************/

int usage() {

  /**

associate  [-w  |  -d  |  -q]  (  [-t] | [-m model_dir] ) 
       -c model_tag [-n num_neighbors] [-f vector_output_file] <pos_term_1>
       [pos_term_2 ... pos_term_n] [NOT neg_term_1 ... neg_term_n]

   **/

  fprintf( stderr, "\nUsage:\t`associate [-w | -d | -q] \n"
	   "\t\t[-i type_of_input(d or w)] [-f vector_output_file]\n"
	   "\t\t( [-t] | [-m model_dir] )\n"
	   "\t\t[-c <model_tag>]\n"
	   "\t\t[-n num_neighbors] [-f vector_output_file]\n"
	   "\t\t<pos_term_1> [pos_term_2 ... pos_term_n]\n"
	   "\t\t[NOT neg_term_1 ... neg_term_n]'\n\n"

  
	   "\tTask:\t-w\tassociate words (DEFAULT)\n"
	   "\t\t-d\tassociate documents\n"
	   "\t\t-q\tprint query vector\n\n");

  exit( EXIT_SUCCESS );
}

/******************************************
check_args function
check for a couple of things that would indicate
this option was not properly provided with a filename 
*******************************************/
int check_args( char next_arg, int total_remaining_args ) {
  if (next_arg == '-') {
    fprintf(stderr, "\n-c, -m, and -f options must be ");
    fprintf(stderr, "immediately followed by space and argument.\n");
    fprintf(stderr, "We do not allow option arguments beginning with '-'.\n\n");
    return 0;
  }
  return(1);
}


/******************************************
normalize function turns a vector into a unit 
vector by dividing by it's length
******************************************/

void normalize( MATRIX_TYPE *vector, int num_elems ){
  double length = 0.0;
  int i;

  for( i = 0 ; i < num_elems ; i++ ){
    length += vector[i] * vector[i];
  }
  
  length = sqrt( length);

  if( length != 0.0) {
    for( i = 0 ; i < num_elems ; i++ ){
      vector[i] /= length;
    }
  }
}

void print_query_vector( MATRIX_TYPE query_vector[], int num_elems ) {
  int i;

  /* Print out vector component by component */
  for( i=0; i < num_elems; i++ ){
    printf( "%f ", query_vector[i] );
  }
  printf( "\n" );
}


