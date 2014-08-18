#include <stdio.h>

#include "env.h"
#include "model_params.h"

/* Dump word vectors to standard output. */

void usage(char* cmd);

int main(int argc, char *argv[]) {

  char pathbuf[BUFSIZ];
  char wordbuf[BUFSIZ];
  int copylen;
  int i, j;
  DBM *w2o_db;
  datum curr_key;
  datum curr_val;
  long curr_offset;
  MODEL_PARAMS model_params;
  FILE *wordvecfile;
  MATRIX_TYPE *curr_vector;
  size_t vecsize;

  if (argc != 2) {
    usage(argv[0]);
  }
  
  sprintf(pathbuf, "%s/%s", argv[1], WORD2OFFSET_FILE);
  w2o_db = dbm_open(pathbuf, O_RDONLY, 0);

  sprintf(pathbuf, "%s/%s", argv[1], MODEL_PARAMS_BIN_FILE);
  if ( !read_model_params( pathbuf, &model_params )) {
    fprintf( stderr, "Can't open model parameter file \"%s\".\n", pathbuf );
    exit( EXIT_FAILURE );
  }

  vecsize = sizeof(MATRIX_TYPE) * model_params.singvals;
  curr_vector = malloc(vecsize);
  if (curr_vector == NULL) {
    fprintf(stderr, "Couldn't allocated memory for curr_vector.\n");
    exit( EXIT_FAILURE );
  }

  sprintf(pathbuf, "%s/%s", argv[1], WORDVECBIN_FILE);
  wordvecfile = fopen(pathbuf, "r");
  if(wordvecfile == NULL) {
    fprintf(stderr,"Can't open word vector file \"%s\"\n", pathbuf);
    exit( EXIT_FAILURE );
  }


  for (curr_key = dbm_firstkey(w2o_db), i=0;
       curr_key.dptr != NULL;
       curr_key = dbm_nextkey(w2o_db), i++) {
    copylen = curr_key.dsize;
    if (curr_key.dsize + 1 > BUFSIZ) {
      fprintf(stderr, "Word %d too long for buffer; truncating.", i);
      copylen = BUFSIZ - 1;
    }
    strncpy(wordbuf, (char *)curr_key.dptr, copylen);
    wordbuf[copylen] = '\0';
    curr_val = dbm_fetch(w2o_db, curr_key);

    if (curr_val.dsize != sizeof(curr_offset)) {
      fprintf(stderr, "Size mismatch for word %d (\"%s\"); skipping.\n",
	      i, wordbuf);
      continue;
    }
    memcpy(&curr_offset, curr_val.dptr, curr_val.dsize);
    if (fseek(wordvecfile, curr_offset, SEEK_SET) != 0) {
      fprintf(stderr, "Couldn't seek to offset %d.  Skipping word %d (\"%s\").\n",
	      curr_offset, i, wordbuf);
      continue;
    }

    if (fread(curr_vector, sizeof(MATRIX_TYPE), model_params.singvals, wordvecfile) !=
	model_params.singvals) {
      fprintf(stderr, "Couldn't read word vector for word %d (\"%s\").  Skipping.\n",
	      i, wordbuf);
      continue;
    }
    
    printf("%s", wordbuf);
    for (j = 0; j < model_params.singvals; j++) {
      printf(" %.9f", curr_vector[j]);
    }
    printf("\n");
  }

  dbm_close(w2o_db);
  fclose(wordvecfile);
  
  return 0;

}

void usage(char *cmd) {
  fprintf(stderr, "Usage: %s <model_dir>\n", cmd);
  exit( EXIT_FAILURE );
}
