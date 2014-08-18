#ifndef SEARCH_UTILS_H
#define SEARCH_UTILS_H    1

#define MODEL_PATH_VAR  "INFOMAP_MODEL_PATH"

/*
 *  Return first entry in Infomap model path containing
 *  a subdirectory whose name is target_dir.
 *  Return NULL if no such directory is found.
 */
char *search_model_path( char *target_dir );

/*
 *  Return the value of MODEL_PATH_VAR.
 */
char *get_model_path(void);

#endif /* ! SEARCH_UTILS_H */
