#include "search_utils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *  Return first entry in INFOMAP_MODEL_PATH containing
 *  a subdirectory whose name is target_dir.
 *  Return NULL if no such directory is found.
 */
char *search_model_path( char *target_dir ) {
  
  char pathbuf[BUFSIZ];
  char *model_path = getenv(MODEL_PATH_VAR);
  struct stat statbuf;
  char *tokstr, *tok;

  if (model_path == NULL) {
    fprintf(stderr, "model path variable %s is not set.\n", MODEL_PATH_VAR);
    return NULL;
  }
  tokstr = strdup( model_path );
  tok = strtok( tokstr, ":" );
  while (tok != NULL) {
    sprintf( pathbuf, "%s/%s", tok, target_dir );

    if ( (stat(pathbuf, &statbuf) == 0) && S_ISDIR(statbuf.st_mode) ) {
      return tok;
    }
    
    tok = strtok( NULL, ":" );
  }

  return NULL;
}

char *get_model_path(void) {
  char *model_path = getenv(MODEL_PATH_VAR);

  return model_path;
}
