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

#include "report.h"

static char *my_reports_file;

/* Invoke the creator and write the result to the pipe. */
void report( int sig ) {
  int pipe;
  char buffer[MAXLINELEN];

  (*report_creator)( buffer);
  pipe = open( my_reports_file, O_WRONLY );
  if( pipe != -1) {
    write( pipe, buffer, strlen( buffer));
    close( pipe);
    return;
  }
  else return;
}

/* To be written if the program is not initialized. */
void default_creator( char *str) {
  sprintf( str, "No report message available.\n");
}

/* This is called from programs to tell report which function to invoke. */
int init_report(char *reports_file) {
  react.sa_handler = report;
  sigemptyset( &react.sa_mask);
  react.sa_flags   = 0;
  sigaction( SIGUSR1, &react, 0);

  report_creator = default_creator;
  my_reports_file = reports_file;
  return 1; /* wasn't there before */
}

int set_report( void (*new_creator)()) {
  report_creator = new_creator;
  return 1; /* wasn't there before */
}
