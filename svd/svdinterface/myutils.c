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

#include <math.h>
#include <stdio.h>
/*#include <values.h>*/
#include <stdlib.h>
#include "fixed.h"
#include "myutils.h"

char infoname[MAXZEILE];

int doesexist( char *filename1, char *filename2 )
{
  char filename[MAXZEILE];
  FILE *myfile;
  int myflag;
  sprintf(filename,"%s%s",filename1,filename2);
  /* printf("|%s|\n",filename); */
  if ((myfile = fopen(filename,"r")) == NULL)
    myflag = FALSE;
  else
    myflag = TRUE;
  fclose(myfile);
  return myflag;
}

void openfile( char *filename, FILE **myfile )
{
  if ((*myfile = fopen(filename,"r")) != NULL)
    errormessage("%s already exists.",filename,NON);
  else if ((*myfile = fopen(filename,"w")) == NULL)
    errormessage("Can't open %s.",filename,NON);
  fprintf(stderr,"  Writing to: %s\n",filename);
}

void prepareread( char *filename1, char *filename2, FILE **myfile )
{
  char filename[MAXZEILE];
  sprintf(filename,"%s%s",filename1,filename2);
  /* printf("|%s|\n",filename); */
  if ((*myfile = fopen(filename,"r")) == NULL)
    errormessage("Can't open %s.",filename,NON);
  fprintf(stderr,"  Reading: %s\n",filename);
}

void errormessage( char *mainstring, char *auxstring, int auxnumber )
{
  void exit();
  int printstring,printnumber;
  printstring=(strlen(auxstring)!=0);
  printnumber=(auxnumber!=NON);
  /* printf("%s",mainstring);
     printf("%s",auxstring);
     printf("%d\n",auxnumber); */
  fprintf(stderr,"\n\n  ERROR:\n\n  ");
  if (printnumber&&printstring)
    fprintf(stderr,mainstring,auxstring,auxnumber);
  else if (printnumber)
    fprintf(stderr,mainstring,auxnumber);
  else if (printstring)
    fprintf(stderr,mainstring,auxstring);
  else
    fprintf(stderr,mainstring);
  fprintf(stderr,"\n\n");
  exit(1);
}

void infomessage( char *mainstring, char *auxstring, int auxnumber )
{
  FILE *infofile;
  int printstring,printnumber;
  /* printf("Entering infofile\n"); */
  if ((infofile = fopen(infoname,"a")) == NULL)
    errormessage("Can't open %s.",infoname,NON); 
  printstring=(strlen(auxstring)!=0);
  printnumber=(auxnumber!=NON && auxnumber!=NOCR);
  if (auxnumber!=NOCR) {
    fprintf(stderr,"  ");
    fprintf(infofile,"  ");
  };
  if (printnumber&&printstring) {
    fprintf(stderr,mainstring,auxstring,auxnumber);
    fprintf(infofile,mainstring,auxstring,auxnumber);
  } else if (printnumber) {
    fprintf(stderr,mainstring,auxnumber);
    fprintf(infofile,mainstring,auxnumber);
  } else if (printstring) {
    fprintf(stderr,mainstring,auxstring);
    fprintf(infofile,mainstring,auxstring);
  } else {
    fprintf(stderr,mainstring);
    fprintf(infofile,mainstring);
  };
  if (auxnumber!=NOCR) {
    fprintf(stderr,"\n");
    fprintf(infofile,"\n");
  };
  /* printf("Leaving infofile\n"); */
  fclose(infofile);
}

void makebeep( int count, int step, char *fstring, int superstep ) 
{
  if (count%step==0) {
    if (count%(superstep*step)==0) fprintf(stderr,"\n  ");
    fprintf(stderr,fstring,count);
    fflush(stdout);
  };
}

int samestart( char *word1, char *word2 )
{
  int l1, l2, shorterone;
  l1 = strlen( word1 );
  l2 = strlen( word2 );
  shorterone = ( (l1<l2) ? l1 : l2 );
  return (!strncmp( word1, word2, shorterone ));
}

/* Changed May 2005 to allow for 64 bit systems. */
float **Allocate_Float_Matrix( int N, int M ) {
  float *workp, **p;
  int i, j;

  if(!(p = malloc(N * sizeof(float *))))
errormessage("allocate_float","",NON);
 
  for(i=0; i<N; i++) {
    if(!(p[i] = malloc(M*sizeof(float))))
errormessage("allocate_float","",NON); 
  }
   return p;
}

char *mymalloc( int mysize )
{
  char *mypointer;
  mypointer = (char *) malloc((unsigned) mysize);
  if (mypointer==NULL)
    errormessage("allocation request failed in mymalloc","",NON);
  return mypointer;
}

void cheatlint( void ) {
  char *huch;
  float **tmp2;
  FILE *myfile;
  errormessage("Don't call cheatlint.","",NON);
  cheatlint();
  openfile("",&myfile);
  prepareread("","",&myfile);
  infomessage("","",0);
  if (samestart("",""))
    makebeep(0,0,"",0);
  if (doesexist("",""));
  huch = mymalloc(5*sizeof(char));
  tmp2 = Allocate_Float_Matrix(5,5);
  printf(huch,tmp2);
}
