/* ****************************************
   
   (c) Copyright 1993
   University of Tennessee
   All Rights Reserved                          

   Changes by Hinrich Schuetze (1994) and
   Scott Cederberg <cederber@csli.stanford.edu> (2003)

   The following notice added by Scott Cederberg 
   <cederber@csli.stanford.edu>, in preparation for
   releasing the Infomap NLP software with this file 
   included:

   This file is part of the SVDPACKC package for Singular Value
   Decomposition.  The entire package can be obtained from
   http://www.netlib.org/svdpack/.

   The use of this code in a commercial product **IS NOT ALLOWED** without 
   express permission from the University of Tennessee.  You should
   begin seeking such permission by contacting Michael Berry 
   <berry@cs.utk.edu>.

   Additionally, the above copyright notice must be retained in this
   file, and credit must be given to SVDPACKC and its developers in
   the documentation associated with this software.

   **************************************** */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/types.h>
#include <sys/time.h>
#include "fixed.h"
#include <errno.h>
#include "filenames.h"

#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#if 0
#include "las2.h"
#endif
#include "fixed.h"
#endif

#define KAPPA ((double) 1.0e-6)
#define ENDL ((double) -1.0e-30)
#define ENDR ((double) 1.0e-30)

/**************************************************************
 * Sparse svd via eigensystem of A'A matrix   		      *
 * The equivalent symmetric eigenvalue problem:               *
 *                                                            *
 *  B x = lambda x, where x' = (u',v'), lambda = sigma**2,    *
 *                                                            *
 *  B = A'A, and A is m (nrow) by n (ncol) (nrow >> ncol),    *
 *							      *
 *  so that {u, sqrt(lambda), v} is a singular triplet of A.  *
 *  (A' = transpose of A)				      *
 *                                                            *
 * global variables and common areas used by las2 and its     *
 * procedures.                                                *
 **************************************************************/

#define LMTNW   600000  /* max. size of working area allowed  */
#define NMAX    3000    /* bound on ncol, order of A          */

int    ierr,           /* error flag                         */
	j,              /* number of lanczos steps taken      */
	neig,           /* number of ritz values stabilized   */
	nsig,           /* number of accepted ritz values     *
			 * based on kappa (relative accuracy) */
    	ncol,           /* number of columns of A             */
    	nrow,           /* number of rows of A                */
	mxvcount = 0;

/**************************************************************
 * pointers to areas holding input matrix which is stored in  *
 * harwell-boeing format.                                     *
 **************************************************************/
int    *pointr = NULL, /* pointer to column start array      */
   	*rowind = NULL; /* pointer to row indices array       */
float  *value = NULL;  /* pointer to nonzero values array    */

double  rnm,            /* norm of the next residual vector   */
	anorm,
	tol,
	eps,            /* positive machine epsilon           */
	eps1,           /* roundoff estimate for dot product  *
			 * of two unit vector                 */
 	reps,
	eps34;

double  *xv1 = NULL,    /* temp arrays needed for computing   */
	*xv2 = NULL,    /* singular vectors                   */
	*ztemp = NULL,

        *a = NULL;      /* pointer to area used by user-      *
			 * supplied procedure store and holds *
		  	 * lanczos vectors                    */
  char *error[10] = {  /* error messages used by function    *
			* check_parameters                   */
    NULL,
    " SORRY, YOUR MATRIX IS TOO BIG ",
    " ***** ENDL MUST BE LESS THAN ENDR *****",
    " ***** MAXPRS CANNOT EXCEED LANMAX *****",
    " ***** N = NROW + NCOL MUST BE GREATER THAN ZERO *****",
    " ***** LANMAX (NUMBER OF LANCZOS STEPS) IS INVALID *****",
    " ***** MAXPRS (NUMBER OF IEGENPAIRS DESIRED) IS INVALID *****",
    " ***** 6*N+4*LANMAX+1 + LANMAX*LANMAX CANNOT EXCEED NW *****",
    " ***** 6*N+4*LANMAX+1 CANNOT EXCEED NW *****",
    NULL};

#if 0
long   landr           (long, long, long, long, double, double, long,
	                double, double *, double *, double *);
void   dscal           (long, double, double *,long);
double ddot            (long, double *,long, double *, long);
void   daxpy           (long, double, double *,long, double *, long);
void   opb             (long, double *, double *);
void   opa             (double *, double *);
void   write_data      (long, long, double, double, long, double,
		        char *,char *, long, long, long);
long   check_parameters(long, long, long, double, double, long,
		        long);
float  timer           (void);
#endif

static double random();

/***********************************************************************
 *                                                                     *
 *                        main()                                       *
 * Sparse SVD(A) via Eigensystem of A'A symmetric Matrix 	       *
 *                  (double precision)                                 *
 *                                                                     *
 ***********************************************************************/
/***********************************************************************
  
  Description
  -----------
  
  This sample program uses landr to compute singular triplets of A via
  the equivalent symmetric eigenvalue problem                         
  
  B x = lambda x, where x' = (u',v'), lambda = sigma**2,
  where sigma is a singular value of A,
  
  B = A'A , and A is m (nrow) by n (ncol) (nrow >> ncol),                
  
  so that {u,sqrt(lambda),v} is a singular triplet of A.        
  (A' = transpose of A)                                      
  
  User supplied routines: opa, opb, store, timer              
  
  opa(     x,y) takes an n-vector x and returns A*x in y.
  opb(ncol,x,y) takes an n-vector x and returns B*x in y.
  
  Based on operation flag isw, store(n,isw,j,s) stores/retrieves 
  to/from storage a vector of length n in s.                   
  
  User should edit timer() with an appropriate call to an intrinsic
  timing routine that returns elapsed user time.                      
  
  
  External parameters 
  -------------------
  
  Defined and documented in las2.h
  
  
  Local parameters 
  ----------------
  
  (input)
  endl     left end of interval containing unwanted eigenvalues of B
  endr     right end of interval containing unwanted eigenvalues of B
  kappa    relative accuracy of ritz values acceptable as eigenvalues
  of B
  vectors is not equal to 1
  r        work array
  n	    dimension of the eigenproblem for matrix B (ncol)
  maxprs   upper limit of desired number of singular triplets of A
  lanmax   upper limit of desired number of Lanczos steps
  nnzero   number of nonzeros in A
  vectors  1 indicates both singular values and singular vectors are 
  wanted and they can be found in output file lav2;
  0 indicates only singular values are wanted 
  
  (output)
  ritz	    array of ritz values
  bnd      array of error bounds
  d        array of singular values
  memory   total memory allocated in bytes to solve the B-eigenproblem
  
  
  Functions used
  --------------
  
  BLAS		daxpy, dscal, ddot
  USER		opa, opb, timer
  MISC		write_data, check_parameters
  LAS2		landr
  
  
  Precision
  ---------
  
  All floating-point calculations are done in double precision;
  variables are declared as long and double.
  
  
  LAS2 development
  ----------------
  
  LAS2 is a C translation of the Fortran-77 LAS2 from the SVDPACK
  library written by Michael W. Berry, University of Tennessee,
  Dept. of Computer Science, 107 Ayres Hall, Knoxville, TN, 37996-1301
  
  31 Jan 1992:  Date written 
  
  Theresa H. Do
  University of Tennessee
  Dept. of Computer Science
  107 Ayres Hall
  Knoxville, TN, 37996-1301
  internet: tdo@cs.utk.edu
  
  ***********************************************************************/

computencolnnzero(indxname,myncol,nnzero)
     char *indxname;
     int *myncol,*nnzero;
{
  int chuck,i,count = 0;
  FILE *indxfile;
  char zeile[MAXLINE];
  prepareread(indxname,"",&indxfile);
  for (i=0;;i++) {
    if (fgets(zeile,MAXLINE,indxfile)==NULL)
      break;
    chuck = BADINT;
    sscanf(zeile,"%d",&chuck);
    if (chuck<0 || chuck==BADINT)
      errormessage("chuck<0 || chuck==BADINT","",NON);
    count += chuck;
  }
  fclose(indxfile);
  *nnzero = count;
  *myncol = i;
}

readindx(indxname,myncol,field)
     char *indxname;
     int *myncol;
     int field[];
{
  int chuck,i,count = 0;
  FILE *indxfile;
  char zeile[MAXLINE];
  prepareread(indxname,"",&indxfile);
  for (i=0;;i++) {
    if (fgets(zeile,MAXLINE,indxfile)==NULL)
      break;
    if (i>=*myncol)
      errormessage("i>=myncol","",NON);
    chuck = BADINT;
    sscanf(zeile,"%d",&chuck);
    if (chuck<0 || chuck==BADINT)
      errormessage("chuck<0 || chuck==BADINT","",NON);
    field[i] = count;
    count += chuck;
  }
  fclose(indxfile);
  if (i!=*myncol)
    errormessage("i!=myncol","",NON);
  field[i] = count;
}

#if 0
printdata() {
  int i,k;
  for (i=0; i<ncol; i++) {
    printf("\n  COLUMN %d\n",i);
    for (k=pointr[i]; k<pointr[i+1]; k++)
      printf("%d:%f ",rowind[k],value[k]);
  }
}
#endif

checkdata() {
  int i,k;
  for (i=0; i<ncol; i++)
    for (k=pointr[i]; k<pointr[i+1]; k++)
      if (rowind[k]>=nrow)
	errormessage("%d>=nrow","",rowind[k]);
}

void readdata( char *datadir, int *myncol, int *nnzero )
{
  /* char *mymalloc(); */

  int i,chuck;
  float fchuck;
  FILE *collfile;
  char indxname[MAXLINE],collname[MAXLINE],zeile[MAXLINE];
  
  sprintf(indxname,"%s%s",datadir, INDEX_FILE);
  sprintf(collname,"%s%s",datadir, COLL_FILE);
  
  computencolnnzero(indxname,myncol,nnzero);
  
  pointr = (int *) mymalloc(sizeof(int)*(*myncol+1));
  readindx(indxname,myncol,pointr);
  if (pointr[*myncol]!=*nnzero)
    errormessage("pointr[*myncol]!=*nnzero","",NON);
  
  rowind = (int *) mymalloc(sizeof(int)**nnzero);
  value = (float *) mymalloc(sizeof(float)**nnzero);
  prepareread(collname,"",&collfile);
  for (i=0;;i++) {
    if (fgets(zeile,MAXLINE,collfile)==NULL)
      break;
    if (i>=*nnzero)
      errormessage("i>=*nnzero","",NON);
    fchuck = BADFLOAT;
    sscanf(zeile,"%d %f",&chuck,&fchuck);
    if (fchuck==BADFLOAT)
      errormessage("fchuck==BADFLOAT","",NON);
    rowind[i] = chuck;
    value[i] = fchuck;
  }
  fclose(collfile);
  if (i!=*nnzero)
    errormessage("i!=*nnzero","",NON);
}

clearfields(left,right,sing,maxprs,myncol,mynrow)
     float **left,**right,sing[];
     int maxprs,myncol,mynrow;
{
  int i,k;
  for (i=0; i<maxprs; i++)
    sing[i] = 0.0;
  for (k=0; k<mynrow; k++)
    for (i=0; i<maxprs; i++)
      left[k][i] = 0.0;
  for (k=0; k<myncol; k++)
    for (i=0; i<maxprs; i++)
      right[k][i] = 0.0;
}

/*******************************************************************
 * allocate memory                                                 *
 * pointr - column start array of harwell-boeing sparse matrix     *
 *          format                                       (ncol+1)  *
 * rowind - row indices array of harwell-boeing format   (nnzero)  *
 * value  - nonzero values array of harwell-boeing sparse matrix   *
 *          format                                       (nnzero)  *
 * r      - work array                                        (n)  *
 * ritz   - array of ritz values                              (n)  *
 * bnd    - array of error bounds                             (n)  *
 * d      - array of approximate singular values of matrix A  (n)  *
 * ztemp  - work array for user function opb	       (nrow)  *
 * a      - storage area for Lanczos vectors     (n * (lanmax+2))  *
 *******************************************************************/

void preparelandr( char *datadir, int mynrow, int lanmax,
		   int maxprs, float **left, float **right, float *sing,
		   int *hashcomp )
{
  float  timer();
/*
  char *mymalloc();
*/
  FILE *diagfile;
  int dsize = sizeof(double),i,nn,n,nnzero,memory;
  float exetime;
  double endl = ENDL, endr = ENDR, kappa = KAPPA;
  double *r, *ritz, *bnd, *d;
  openfile(SVD_DIAG_FILE, &diagfile);
  readdata(datadir,&ncol,&nnzero);
  clearfields(left,right,sing,maxprs,ncol,mynrow);
  n = ncol;
  nn = ncol + mynrow;
  nrow = mynrow;
  checkdata();
  write_data(lanmax,maxprs,endl,endr,kappa,mynrow,ncol,n,diagfile);
  check_parameters(maxprs,lanmax,n,endl,endr);
  r      = (double *) mymalloc(dsize*n);
  ritz   = (double *) mymalloc(dsize*n);
  bnd    = (double *) mymalloc(dsize*n);
  d      = (double *) mymalloc(dsize*n);
  ztemp  = (double *) mymalloc(dsize*mynrow);
  a      = (double *) mymalloc(dsize*n*(lanmax+2));
  memory = dsize*(6*n+lanmax*n+mynrow)
    +(sizeof(int)+sizeof(float))*nnzero
      +sizeof(int)*(ncol+1);
  /* initialize first n cells to zero for starting vector */
  for (i = 0; i < n; i++) r[i] = 0.;
  exetime = timer();
  /* make a lanczos run; see landr for meaning of parameters */
  landr(n,lanmax,maxprs,endl,endr,TRUE,kappa,ritz,bnd,r,right);
  exetime = timer() - exetime;
  /* memory allocated for xv1,xv2 and s in landr() */
  memory += sizeof(double) * (nn * (j+1) + n + (j+1) * (j+1));
  firstdiags(ierr,memory,exetime,neig,j,ritz,bnd,diagfile);
  computeleftvectors(n,nn,exetime,bnd,d,maxprs,left,diagfile);
  getsing(nsig,d,bnd,maxprs,sing,diagfile);
  free((char *) value); free((char *) pointr);
  free((char *) r); free((char *) ritz); free((char *) bnd);
  free((char *) d); free((char *) ztemp); free((char *) a);
  free((char *) xv1); free((char *) xv2);
  *hashcomp = nsig;
  if (nsig<maxprs)
    fprintf(diagfile,"FEWER THAN EXPECTED SVALUES: %d\n",nsig);
  fclose(diagfile);
}

/* extern int ncol,nrow; */
/* extern char *error[]; */
/* extern FILE *fp_out1; */
/***********************************************************************
 *								       *
 *		      check_parameters()			       *
 *								       *
 ***********************************************************************/
/***********************************************************************
  
  Description
  -----------
  Function validates input parameters and returns error code (long)  
  
  Parameters 
  ----------
  (input)
  maxprs   upper limit of desired number of eigenpairs of B           
  lanmax   upper limit of desired number of lanczos steps             
  n        dimension of the eigenproblem for matrix B               
  endl     left end of interval containing unwanted eigenvalues of B
  endr     right end of interval containing unwanted eigenvalues of B
  vectors  1 indicates both eigenvalues and eigenvectors are wanted 
  and they can be found in lav2; 0 indicates eigenvalues only
  nnzero   number of nonzero elements in input matrix (matrix A)      
  
  ***********************************************************************/

check_parameters(maxprs,lanmax,n,endl,endr)
     int maxprs;
     int lanmax;
     int n;
     double endl;
     double endr;
{
  int ncells;
  /* assuming that nrow >= ncol... */
  if (ncol >= NMAX)
    errormessage("ncol >= NMAX","",NON);
  if (endl >= endr)
    errormessage("endl >= endr","",NON);
  if (maxprs > lanmax)
    errormessage("maxprs > lanmax","",NON);
  if (n <= 0)
    errormessage("n <= 0","",NON);
  if (lanmax <= 0 || lanmax > n)
    errormessage("lanmax <= 0 || lanmax > n","",NON);
  if (maxprs <= 0 || maxprs > lanmax)
    errormessage("maxprs <= 0 || maxprs > lanmax","",NON);
  ncells = 6 * n + 4 * lanmax + 1 + lanmax * lanmax;
  if (ncells > LMTNW)
    errormessage("ncells > LMTNW","",NON);
}

/* extern FILE *fp_out1; */
/***********************************************************************
 *								       *
 *			  write_data()				       *
 *   Function writes out header of output file containing ritz values  *
 *								       *
 ***********************************************************************/

/* compute residual error when singular values and vectors are 
 * computed.  This is done only for singular values that are
 * within the relative accuracy (kappa) */

putinvector(thissing,hashsing,hashcomputed,myvalue,left,ldim)
     int thissing,hashsing,hashcomputed;
     double *myvalue;
     float **left;
{
  int i,reordered;
  reordered = (hashcomputed-1-thissing);
  if (reordered<0)
    errormessage("reordered<0","",NON);
  if (reordered<hashsing)
    for (i=0; i<ldim; i++)
      left[i][reordered] = myvalue[i];
}

computeleftvectors(n,nn,exetime,bnd,d,maxprs,left,diagfile)
     int n,nn;
     float exetime;
     double *bnd,*d;
     float **left;
     FILE *diagfile;
{
  double ddot();
  float  timer();
  int i,id,ida;
  float t0;  
  double tmp0,tmp1,tmp2,xnorm;
  t0 = timer();
  id = 0;
  for (i=0; i<nsig; i++) {
    /* multiply by matrix B first */
    opb(n, &xv1[id], xv2);
    tmp0 = ddot(n, &xv1[id], 1, xv2, 1);
    daxpy(n, -tmp0, &xv1[id], 1, xv2, 1);
    if (tmp0<0) {
      /*
	for (k=0; k<n; k++)
	printf("%f %f   ",*(&xv1[id]+k),*(xv2+k));
	printf("Setting %f to 0\n",tmp0);
	tmp0 = 0;
	*/
      printf("%f\n",tmp0);
      errormessage("sqrt tmp0 computeleftvectors","",NON);
    }
    tmp0 = sqrt(tmp0);
    tmp2 = ddot(n, xv2, 1, xv2, 1);
    if (tmp2<0) {
      printf("%f\n",tmp2);
      errormessage("sqrt tmp2 computeleftvectors","",NON);
    }
    xnorm = sqrt(tmp2);
    ida = id + ncol;
    /* multiply by matrix A to get (scaled) left s-vector */
    opa(&xv1[id], &xv1[ida]);
    tmp1 = 1.0 / tmp0;
    dscal(nrow, tmp1, &xv1[ida], 1);
    xnorm *= tmp1;
    bnd[i] = xnorm;
    d[i] = tmp0;
    putinvector(i,maxprs,nsig,&xv1[ida],left,nrow);
    id += nn;
  }
  exetime += (timer() - t0);
  seconddiags(mxvcount,nsig,exetime,diagfile);
}

firstdiags(myierr,memory,exetime,myneig,hashlan,ritz,bnd,diagfile)
     int myierr,memory;
     float exetime;
     int myneig,hashlan;
     double *ritz,*bnd;
     FILE *diagfile;
{
  int i;
  /* print error code if not zero */
  if (myierr) {
    fprintf(diagfile," ... RETURN FLAG = %9ld ...\n",myierr);
    for (i=0; i<10; i++)
      fprintf(diagfile,"%s\n",error[i]);
  }
  fprintf(diagfile,"\n");
  fprintf(diagfile," ...... ALLOCATED MEMORY (BYTES)= %10.2E\n",(float)memory);
  fprintf(diagfile," ...... LANSO EXECUTION TIME=%10.2E\n",exetime);
  fprintf(diagfile," ...... \n");
  fprintf(diagfile," ...... NUMBER OF LANCZOS STEPS = %3ld       NEIG = %3ld\n",hashlan+1,myneig);
  fprintf(diagfile," ...... \n");
  fprintf(diagfile," ......         COMPUTED RITZ VALUES  (ERROR BNDS)\n");
  fprintf(diagfile," ...... \n");
  for (i = 0; i <= hashlan; i++)
    fprintf(diagfile," ...... %3ld   %22.14E  (%11.2E)\n",
	    i + 1,ritz[i],bnd[i]);
} 

seconddiags(mymxvcount,mynsig,exetime,diagfile)
     int mymxvcount,mynsig;
     float exetime;
     FILE *diagfile;
{
  int count1,count2;
  count1=(mymxvcount-mynsig)/2 + mynsig;
  count2=(mymxvcount-mynsig)/2;
  fprintf(diagfile," ...... \n");
  fprintf(diagfile," ...... NO. MULTIPLICATIONS BY A  =%10ld\n",count1);
  fprintf(diagfile," ...... NO. MULT. BY TRANSPOSE(A) =%10ld\n",count2);
  fprintf(diagfile,"\n");
  fprintf(diagfile," ...... LASVD EXECUTION TIME=%10.2E\n",exetime);
  fprintf(diagfile," ...... \n");
  fprintf(diagfile," ......        NSIG = %4ld\n",mynsig);
  fprintf(diagfile," ...... \n");
  fprintf(diagfile," ......         COMPUTED S-VALUES     (RES. NORMS)\n");
  fprintf(diagfile," ...... \n");
}

write_data(lanmax,maxprs,endl,endr,kappa,mynrow,myncol,n,diagfile)
     int lanmax;
     int maxprs;
     double endl;
     double endr;
     double kappa;
     int mynrow;
     int myncol;
     int n;
     FILE *diagfile;
{
  fprintf(diagfile," ... \n");
  fprintf(diagfile," ... SOLVE THE [A^TA]   EIGENPROBLEM\n");
  fprintf(diagfile," ... NO. OF EQUATIONS          =%5ld\n",n);
  fprintf(diagfile," ... MAX. NO. OF LANCZOS STEPS =%5ld\n",lanmax);
  fprintf(diagfile," ... MAX. NO. OF EIGENPAIRS    =%5ld\n",maxprs);
  fprintf(diagfile," ... LEFT  END OF THE INTERVAL =%10.2E\n",endl);
  fprintf(diagfile," ... RIGHT END OF THE INTERVAL =%10.2E\n",endr);
  fprintf(diagfile," ... KAPPA                     =%10.2E\n",kappa);
  fprintf(diagfile," ... NO. OF DOCUMENTS (ROWS)   = %8ld\n",mynrow);
  fprintf(diagfile," ... NO. OF TERMS     (COLS)   = %8ld\n",myncol);
  fprintf(diagfile," ... ORDER OF MATRIX A         = %8ld\n",n);
  fprintf(diagfile," ... \n");
}

getsing(mynsig,d,bnd,maxprs,sing,diagfile)
     int mynsig;
     double *d,*bnd;
     int maxprs;
     float *sing;
     FILE *diagfile;
{
  int i,k;
  float max = -BIGFLOAT;
  for (i=mynsig-1,k=0; i>=0 && k<maxprs; i--,k++) {
    if (bnd[i]>=max)
      max = bnd[i];    
    fprintf(diagfile,"%22.14E  (%11.2E) %d\n",d[i],bnd[i],mynsig-(i+1));
    sing[k] = d[i];
  }
  for (; k<maxprs; k++)
    sing[k] = 0.0;
  fprintf(diagfile,"WORST ERROR BOUND: %f\n",max);
}


/* extern double eps, eps1, reps, eps34, *xv1, *xv2; */
#if 0
/* extern long nrow, ncol, j; */
void   machar(long *, long *, long *, long *, long *);
long   check_parameters(long, long, long, double, double, long,
			long);
double dmax(double, double);
void   lanso(long, long, long, double, double, double *, double *,
	     double *[]);
void   ritvec(long, double, double *, double *, double *, double *,
	      double *, double *);
#endif

/***********************************************************************
 *                                                                     *
 *				landr()				       *
 *        Lanczos algorithm with selective orthogonalization           *
 *                    Using Simon's Recurrence                         *
 *                       (double precision)                            *
 *                                                                     *
 ***********************************************************************/
/***********************************************************************
  
  Description
  -----------
  
  landr() is the LAS2 driver routine that, upon entry,
  (1)  checks for the validity of input parameters of the 
  B-eigenproblem 
  (2)  determines several machine constants
  (3)  makes a Lanczos run
  (4)  calculates B-eigenvectors (singular vectors of A) if requested 
  by user
  
  
  arguments
  ---------
  
  (input)
  n        dimension of the eigenproblem for A'A
  lanmax   upper limit of desired number of Lanczos steps
  maxprs   upper limit of desired number of eigenpairs
  nnzero   number of nonzeros in matrix A
  endl     left end of interval containing unwanted eigenvalues of B
  endr     right end of interval containing unwanted eigenvalues of B
  vectors  1 indicates both eigenvalues and eigenvectors are wanted
  and they can be found in output file lav2; 
  0 indicates only eigenvalues are wanted
  kappa    relative accuracy of ritz values acceptable as eigenvalues
  of B (singular values of A)
  r        work array
  
  (output)
  j        number of Lanczos steps actually taken                     
  neig     number of ritz values stabilized                           
  ritz     array to hold the ritz values                              
  bnd      array to hold the error bounds
  
  
  External parameters
  -------------------
  
  Defined and documented in las2.h
  
  
  local parameters
  -------------------
  
  ibeta    radix for the floating-point representation
  it       number of base ibeta digits in the floating-point significand
  irnd     floating-point addition rounded or chopped
  machep   machine relative precision or round-off error
  negeps   largest negative integer
  wptr	    array of pointers each pointing to a work space
  
  
  Functions used
  --------------
  
  MISC         dmax, machar, check_parameters
  LAS2         ritvec, lanso
  
  ***********************************************************************/

landr(n,lanmax,maxprs,endl,endr,vectors,kappa,ritz,bnd,r,right)
     int n;
     int lanmax;
     int maxprs;
     double endl;
     double endr;
     int vectors;
     double kappa;
     double *ritz;
     double *bnd;
     double *r;
     float **right;
{
  double dmax();
  int i, size, ibeta, it, irnd, machep, negep;
  double *wptr[10], *tptr, *tptr2;
  
  /* data validation */
  check_parameters(maxprs, lanmax, n, endl, endr);
  
  /* Compute machine precision */ 
  machar(&ibeta, &it, &irnd, &machep, &negep);

    if (n<0)
      errormessage("sqrt n landr","",NON);
  
  eps1 = eps * sqrt( (double) n );
    if (eps<0)
      errormessage("sqrt eps landr","",NON);
  reps = sqrt(eps);
  eps34 = reps * sqrt(reps);
  
  /* allocate work area and initialize pointers         *
   * ptr             symbolic name         size         *
   * wptr[0]             r                  n           *
   * wptr[1]             q		     n           *
   * wptr[2]             q_previous         n           *
   * wptr[3]             p		     n           *
   * wptr[4]             p_previous         n           *
   * wptr[5]             wrk                n           *
   * wptr[6]             alf              lanmax        *
   * wptr[7]             eta              lanmax        *
   * wptr[8]             oldeta           lanmax        *
   * wptr[9]             bet              lanmax+1      */
  
  size = 5 * n + (lanmax * 4 + 1);
  tptr = NULL;
  if (!(tptr = (double *) mymalloc(size * sizeof(double))))
    errormessage("FIRST MALLOC FAILED in LANDR()","",NON);
  tptr2 = tptr;
  wptr[0] = r;
  for (i = 1; i <= 5; i++) {
    wptr[i] = tptr;
    tptr += n;
  }
  for (i = 6; i <= 9; i++) {
    wptr[i] = tptr;
    tptr += lanmax;
  }
  
  lanso(n, lanmax, maxprs, endl, endr, ritz, bnd, wptr);
  
  /* compute eigenvectors */
  if (vectors) {
    if (!(xv1 = (double *) mymalloc((nrow+ncol)*(j+1)*sizeof(double))) ||
	!(xv2 = (double *) mymalloc(ncol * sizeof(double)))) 
      errormessage("SECOND MALLOC FAILED in LANDR()","",NON);
    kappa = dmax(fabs(kappa), eps34);
    ritvec(n,kappa,ritz,bnd,wptr[6],wptr[9],wptr[4],wptr[5],right,maxprs);
  }
  
  free((char *) tptr2);
}

#define STORQ 1
#define RETRQ 2
#define STORP 3
#define RETRP 4

/* extern int nrow, ierr, j, nsig, neig; */
/* extern double *xv1; */

#if 0
void   dscal(long, double, double *,long);
void   dcopy(long, double *, long, double *, long);
void   daxpy(long, double, double *,long, double *, long);
void   store(long, long, long, double *);
void   imtql2(long, long, double *, double *, double *);
#endif

/***********************************************************************
 *                                                                     *
 *                        ritvec()                                     *
 * 	    Function computes the singular vectors of matrix A	       *
 *                                                                     *
 ***********************************************************************/
/***********************************************************************
  
  Description
  -----------
  
  This function is invoked by landr() only if eigenvectors of the A'A
  eigenproblem are desired.  When called, ritvec() computes the 
  singular vectors of A and writes the result to an unformatted file.
  
  
  Parameters
  ----------
  
  (input)
  nrow       number of rows of A
  j	      number of Lanczos iterations performed
  fp_out2    pointer to unformatted output file
  n	      dimension of matrix A
  kappa      relative accuracy of ritz values acceptable as 
  eigenvalues of A'A
  ritz       array of ritz values
  bnd        array of error bounds
  alf        array of diagonal elements of the tridiagonal matrix T
  bet        array of off-diagonal elements of T
  w1, w2     work space
  
  (output)
  xv1        array of eigenvectors of A'A (right singular vectors of A)
  ierr	      error code
  0 for normal return from imtql2()
  k if convergence did not occur for k-th eigenvalue in
  imtql2()
  nsig       number of accepted ritz values based on kappa
  
  (local)
  s	      work array which is initialized to the identity matrix
  of order (j + 1) upon calling imtql2().  After the call,
  s contains the orthonormal eigenvectors of the symmetric 
  tridiagonal matrix T
  
  Functions used
  --------------
  
  BLAS		dscal, dcopy, daxpy
  USER		store
  imtql2
  
  ***********************************************************************/

ritvec(n,kappa,ritz,bnd,alf,bet,w1,w2,right,maxprs)
     int n;
     double kappa;
     double *ritz;
     double *bnd;
     double *alf;
     double *bet;
     double *w1;
     double *w2;
     float **right;
     int maxprs;
{
/*
  void exit();
*/
  int js, jsq, i, k, id, id2, tmp;
  double *s;
  
  js = j + 1;
  jsq = js * js;
/*
  size = sizeof(double) * n;
*/  
  if(!(s = (double *) mymalloc (jsq * sizeof(double))))
    errormessage("MALLOC FAILED in RITVEC()","",NON);
  
  /* initialize s to an identity matrix */
  for (i = 0; i < jsq; i++) s[i] = 0.0;
  for (i = 0; i < jsq; i+= (js+1)) s[i] = 1.0;
  dcopy(js, alf, 1, w1, -1);
  dcopy(j, &bet[1], 1, &w2[1], -1);
  
  /* on return from imtql2(), w1 contains eigenvalues in ascending 
   * order and s contains the corresponding eigenvectors */
  imtql2(js, js, w1, w2, s);
  if (ierr) return;
  
#if 0
  write(fp_out2, (char *)&n, sizeof(n));
  write(fp_out2, (char *)&js, sizeof(js));
  write(fp_out2, (char *)&kappa, sizeof(kappa));
#endif
  
  id = 0;
  nsig = 0;
  id2 = jsq - js;
  for (k = 0; k < js; k++) {
    tmp = id2;
    if (bnd[k] <= kappa * fabs(ritz[k]) && k > js-neig-1) {
      for (i = 0; i < n; i++) w1[i] = 0.0;
      for (i = 0; i < js; i++) {
	store(n, RETRQ, i, w2);
	daxpy(n, s[tmp], w2, 1, w1, 1);
	tmp -= js;
      }
      putinvector(k,maxprs,js,w1,right,n);
      /*
	write(fp_out2, (char *)w1, size);
	*/      
      /* store the w1 vector row-wise in array xv1;   
       * size of xv1 is (j+1) * (nrow+ncol) elements 
       * and each vector, even though only ncol long,
       * will have (nrow+ncol) elements in xv1.      
       * It is as if xv1 is a 2-d array (j+1) by     
       * (nrow+ncol) and each vector occupies a row  */
      
      for (i = 0; i < n; i++) xv1[id++] = w1[i];
      id += nrow;
      nsig += 1;
    }
    id2++;
  }
  free((char *) s);
  return;
}

/* extern int ncol, nrow,mxvcount; */
/* extern int *pointr, *rowind; */
/* extern float *value; */
/* extern double *ztemp; */

/**************************************************************
 * multiplication of matrix B by vector x, where B = A'A,     *
 * and A is nrow by ncol (nrow >> ncol). Hence, B is of order *
 * n = ncol (y stores product vector).		              *
 **************************************************************/

opb(n,x,y)
     int n;
     double *x;
     double *y;
{
  int i, k, end;
  
  mxvcount += 2;
  for (i = 0; i < n; i++) y[i] = 0.0;
  for (i = 0; i < nrow; i++) ztemp[i] = 0.0;
  
  for (i = 0; i < ncol; i++) {
    end = pointr[i+1];
    for (k = pointr[i]; k < end; k++) 
      ztemp[rowind[k]] += value[k] * (*x); 
    x++;
  }
  for (i = 0; i < ncol; i++) {
    end = pointr[i+1];
    for (k = pointr[i]; k < end; k++) 
      *y += value[k] * ztemp[rowind[k]];
    y++;
  }
  return;
}

/***********************************************************
 * multiplication of matrix A by vector x, where A is 	   *
 * nrow by ncol (nrow >> ncol).  y stores product vector.  *
 ***********************************************************/

opa(x,y)
     double *x;
     double *y;
{
  int end, i, k;
  
  mxvcount += 1;
  for (i = 0; i < nrow; i++) y[i] = 0.0;
  
  for (i = 0; i < ncol; i++) {
    end = pointr[i+1];
    for (k = pointr[i]; k < end; k++)
      y[rowind[k]] += value[k] * x[i]; 
  }
  return;
}
#define STORQ 1
#define RETRQ 2
#define STORP 3
#define RETRP 4

/* extern double rnm, anorm, tol, eps, eps1, reps, eps34; */
/* extern int ierr, j, neig; */
#if 0
void   stpone(long, double *[]);
void   error_bound(long *, double, double, double *, double *);
void   lanczos_step(long, long, long, double *[], double *,
                    double *, double *, double *, long *, long *);
long   imin(long, long);
long   imax(long, long);
void   dsort2(long, long, double *, double *);
void   imtqlb(long n, double d[], double e[], double bnd[]);
#endif

/***********************************************************************
 *                                                                     *
 *                          lanso()                                    *
 *                                                                     *
 ***********************************************************************/
/***********************************************************************
  
  Description
  -----------
  
  Function determines when the restart of the Lanczos algorithm should 
  occur and when it should terminate.
  
  Arguments 
  ---------
  
  (input)
  n         dimension of the eigenproblem for matrix B
  lanmax    upper limit of desired number of lanczos steps           
  maxprs    upper limit of desired number of eigenpairs             
  endl      left end of interval containing unwanted eigenvalues
  endr      right end of interval containing unwanted eigenvalues
  ritz      array to hold the ritz values                       
  bnd       array to hold the error bounds                          
  wptr      array of pointers that point to work space:            
  wptr[0]-wptr[5]  six vectors of length n		
  wptr[6] array to hold diagonal of the tridiagonal matrix T
  wptr[9] array to hold off-diagonal of T	
  wptr[7] orthogonality estimate of Lanczos vectors at 
  step j
  wptr[8] orthogonality estimate of Lanczos vectors at 
  step j-1
  
  (output)
  j         number of Lanczos steps actually taken
  neig      number of ritz values stabilized
  ritz      array to hold the ritz values
  bnd       array to hold the error bounds
  ierr      (globally declared) error flag
  ierr = 8192 if stpone() fails to find a starting vector
  ierr = k if convergence did not occur for k-th eigenvalue
  in imtqlb()
  ierr = 0 otherwise
  
  
  Functions used
  --------------
  
  LAS		stpone, error_bound, lanczos_step
  MISC		dsort2
  UTILITY	imin, imax
  
  ***********************************************************************/

lanso(n,lanmax,maxprs,endl,endr,ritz,bnd,wptr)
     int n;
     int lanmax;
     int maxprs;
     double endl;
     double endr;
     double *ritz;
     double *bnd;
     double *wptr[];     
{
  double *alf, *eta, *oldeta, *bet, *wrk;
  int ll, first, last, ENOUGH, id1, id2, id3, i, l;
  
/*
  double *r;
  r = wptr[0];
*/
  alf = wptr[6];
  eta = wptr[7];
  oldeta = wptr[8];
  bet = wptr[9];
  wrk = wptr[5];
  j = 0;
  
  /* take the first step */
  stpone(n, wptr);
  if (!rnm || ierr) return;
  eta[0] = eps1;
  oldeta[0] = eps1;
  ll = 0;
  first = 1;
  last = imin(maxprs + imax(8,maxprs), lanmax);
  ENOUGH = FALSE;
  id1 = 0;
  while (id1 < maxprs && !ENOUGH) {
    if (rnm <= tol) rnm = 0.0;
    
    /* the actual lanczos loop */
    lanczos_step(n, first, last, wptr, alf, eta, oldeta, bet, &ll,
		 &ENOUGH);
    if (ENOUGH) j = j - 1;
    else j = last - 1;
    first = j + 1;
    bet[j+1] = rnm;
    
    /* analyze T */
    l = 0;
    for (id2 = 0; id2 < j; id2++) {
      if (l > j) break;
      for (i = l; i <= j; i++) if (!bet[i+1]) break;
      if (i > j) i = j;
      
      /* now i is at the end of an unreduced submatrix */
      dcopy(i-l+1, &alf[l],   1, &ritz[l],  -1);
      dcopy(i-l,   &bet[l+1], 1, &wrk[l+1], -1);
      
      imtqlb(i-l+1, &ritz[l], &wrk[l], &bnd[l]);
      
      if (ierr) {
	printf("IMTQLB FAILED TO CONVERGE (IERR = %d)\n",
	       ierr);
	printf("L = %d    I = %d\n", l, i);
	for (id3 = l; id3 <= i; id3++) 
	  printf("%d  %lg  %lg  %lg\n",
		 id3, ritz[id3], wrk[id3], bnd[id3]);
      }
      for (id3 = l; id3 <= i; id3++) 
	bnd[id3] = rnm * fabs(bnd[id3]);
      l = i + 1;
    }
    
    /* sort eigenvalues into increasing order */
    dsort2((j+1) / 2, j + 1, ritz, bnd);
    
    /* massage error bounds for very close ritz values */
    error_bound(&ENOUGH, endl, endr, ritz, bnd);
    
    /* should we stop? */
    if (neig < maxprs) {
      if (!neig) last = first + 9;
      else last = first + imax(3, 1 + ((j-5) * (maxprs-neig)) / neig);
      last = imin(last, lanmax);
    }
    else ENOUGH = TRUE;
    ENOUGH = ENOUGH || first >= lanmax;
    id1++;
  }
  store(n, STORQ, j, wptr[1]);
  return;
}
#define STORQ 1
#define RETRQ 2
#define STORP 3
#define RETRP 4
#define MAXLL 2

/* extern double rnm, anorm, tol, eps, eps1, reps, eps34; */
/* extern int ierr, j; */
#if 0
double ddot(long, double *,long, double *, long);
void   dscal(long, double, double *,long);
void   daxpy(long, double, double *,long, double *, long);
void   datx(long, double, double *,long, double *, long);
void   dcopy(long, double *, long, double *, long);
void   purge(long, long, double *, double *, double *, double *,
	     double *, double *, double *);
void   ortbnd(double *, double *, double *, double *);
double startv(long, double *[]);
void   store(long, long, long, double *);
long   imin(long, long);
long   imax(long, long);
#endif

/***********************************************************************
 *                                                                     *
 *			lanczos_step()                                 *
 *                                                                     *
 ***********************************************************************/
/***********************************************************************
  
  Description
  -----------
  
  Function embodies a single Lanczos step
  
  Arguments 
  ---------
  
  (input)
  n        dimension of the eigenproblem for matrix B
  first    start of index through loop				      
  last     end of index through loop				     
  wptr	    array of pointers pointing to work space		    
  alf	    array to hold diagonal of the tridiagonal matrix T
  eta      orthogonality estimate of Lanczos vectors at step j   
  oldeta   orthogonality estimate of Lanczos vectors at step j-1
  bet      array to hold off-diagonal of T                     
  ll       number of intitial Lanczos vectors in local orthog. 
  (has value of 0, 1 or 2)			
  enough   stop flag			
  
  Functions used
  --------------
  
  BLAS		ddot, dscal, daxpy, datx, dcopy
  USER		store
  LAS		purge, ortbnd, startv
  UTILITY	imin, imax
  
  ***********************************************************************/

lanczos_step(n,first,last,wptr,alf,eta,oldeta,bet,ll,enough)
     int n;
     int first;
     int last;
     double *wptr[];
     double *alf;
     double *eta;
     double *oldeta;
     double *bet;
     int *ll;
     int *enough;
{
  double ddot();
  double t, *mid,tmp1;
  int i;
  double startv();
  
  for (j=first; j<last; j++) {
    mid     = wptr[2];
    wptr[2] = wptr[1];
    wptr[1] = mid;
    mid     = wptr[3];
    wptr[3] = wptr[4];
    wptr[4] = mid;
    
    store(n, STORQ, j-1, wptr[2]);
    if (j-1 < MAXLL) store(n, STORP, j-1, wptr[4]);
    bet[j] = rnm;
    
    /* restart if invariant subspace is found */
    if (!bet[j]) {
      rnm = startv(n, wptr);
      if (ierr) return;
      if (!rnm) *enough = TRUE;
    }
    if (*enough) break;
    
    /* take a lanczos step */
    t = 1.0 / rnm;
    datx(n, t, wptr[0], 1, wptr[1], 1);
    dscal(n, t, wptr[3], 1);
    opb(n, wptr[3], wptr[0]);
    daxpy(n, -rnm, wptr[2], 1, wptr[0], 1);
    alf[j] = ddot(n, wptr[0], 1, wptr[3], 1);
    daxpy(n, -alf[j], wptr[1], 1, wptr[0], 1);
    
    /* orthogonalize against initial lanczos vectors */
    if (j <= MAXLL && (fabs(alf[j-1]) > 4.0 * fabs(alf[j])))
      *ll = j;  
    for (i=0; i < imin(*ll, j-1); i++) {
      store(n, RETRP, i, wptr[5]);
      t = ddot(n, wptr[5], 1, wptr[0], 1);
      store(n, RETRQ, i, wptr[5]);
      daxpy(n, -t, wptr[5], 1, wptr[0], 1);
      eta[i] = eps1;
      oldeta[i] = eps1;
    }
    
    /* extended local reorthogonalization */
    t = ddot(n, wptr[0], 1, wptr[4], 1);
    daxpy(n, -t, wptr[2], 1, wptr[0], 1);
    if (bet[j] > 0.0) bet[j] = bet[j] + t;
    t = ddot(n, wptr[0], 1, wptr[3], 1);
    daxpy(n, -t, wptr[1], 1, wptr[0], 1);
    alf[j] = alf[j] + t;
    dcopy(n, wptr[0], 1, wptr[4], 1);
    tmp1 = ddot(n, wptr[0], 1, wptr[4], 1);
    if (tmp1<0)
      errormessage("sqrt tmp1 lanczos_step","",NON);
    rnm = sqrt(tmp1);
    anorm = bet[j] + fabs(alf[j]) + rnm;
    tol = reps * anorm;
    
    /* update the orthogonality bounds */
    ortbnd(alf, eta, oldeta, bet);
    
    /* restore the orthogonality state when needed */
    purge(n,*ll,wptr[0],wptr[1],wptr[4],wptr[3],wptr[5],eta,oldeta);
    if (rnm <= tol) rnm = 0.0;
  }
  return;
}
/* extern double rnm, eps, eps1, reps, eps34; */
/* extern int j; */
#if 0
void   dswap(long, double *, long, double *, long);
#endif
/***********************************************************************
 *                                                                     *
 *                          ortbnd()                                   *
 *                                                                     *
 ***********************************************************************/
/***********************************************************************
  
  Description
  -----------
  
  Funtion updates the eta recurrence
  
  Arguments 
  ---------
  
  (input)
  alf      array to hold diagonal of the tridiagonal matrix T         
  eta      orthogonality estimate of Lanczos vectors at step j        
  oldeta   orthogonality estimate of Lanczos vectors at step j-1     
  bet      array to hold off-diagonal of T                          
  n        dimension of the eigenproblem for matrix B		    
  j        dimension of T					  
  rnm	    norm of the next residual vector			 
  eps1	    roundoff estimate for dot product of two unit vectors
  
  (output)
  eta      orthogonality estimate of Lanczos vectors at step j+1     
  oldeta   orthogonality estimate of Lanczos vectors at step j        
  
  
  Functions used
  --------------
  
  BLAS		dswap
  
  ***********************************************************************/

ortbnd(alf,eta,oldeta,bet)
     double *alf;
     double *eta;
     double *oldeta;
     double *bet;
{
  int i;
  if (j < 1) return;
  if (rnm) {
    if (j > 1) {
      oldeta[0] = (bet[1] * eta[1] + (alf[0]-alf[j]) * eta[0] -
		   bet[j] * oldeta[0]) / rnm + eps1;
    }
    for (i=1; i<=j-2; i++) 
      oldeta[i] = (bet[i+1] * eta[i+1] + (alf[i]-alf[j]) * eta[i] +
		   bet[i] * eta[i-1] - bet[j] * oldeta[i])/rnm + eps1;
  }
  oldeta[j-1] = eps1;
  dswap(j, oldeta, 1, eta, 1);  
  eta[j] = eps1;
  return;
}
#define STORQ 1
#define RETRQ 2
#define STORP 3
#define RETRP 4

/* extern double tol, rnm, eps, eps1, reps, eps34; */
/* extern int j; */
#if 0
void   store(long, long, long, double *);
void   daxpy(long, double, double *, long, double *, long);
void   dcopy(long, double *, long, double *, long);
long   idamax(long, double *, long);
double ddot(long, double *, long, double *, long);
#endif

/***********************************************************************
 *                                                                     *
 *				purge()                                *
 *                                                                     *
 ***********************************************************************/
/***********************************************************************
  
  Description
  -----------
  
  Function examines the state of orthogonality between the new Lanczos
  vector and the previous ones to decide whether re-orthogonalization 
  should be performed
  
  
  Arguments 
  ---------
  
  (input)
  n        dimension of the eigenproblem for matrix B		       
  ll       number of intitial Lanczos vectors in local orthog.       
  r        residual vector to become next Lanczos vector            
  q        current Lanczos vector			           
  ra       previous Lanczos vector
  qa       previous Lanczos vector
  wrk      temporary vector to hold the previous Lanczos vector
  eta      state of orthogonality between r and prev. Lanczos vectors 
  oldeta   state of orthogonality between q and prev. Lanczos vectors
  j        current Lanczos step				     
  
  (output)
  r	    residual vector orthogonalized against previous Lanczos 
  vectors
  q        current Lanczos vector orthogonalized against previous ones
  
  
  Functions used
  --------------
  
  BLAS		daxpy,  dcopy,  idamax,  ddot
  USER		store
  
  ***********************************************************************/

purge(n,ll,r,q,ra,qa,wrk,eta,oldeta)
     int n;
     int ll;
     double *r;
     double *q;
     double *ra;
     double *qa;
     double *wrk;
     double *eta;
     double *oldeta;
{
  double ddot();
  double t, tq, tr, reps1,tmp1;
  int k, iteration, flag, i;
  
  if (j < ll+2) return; 
  
  k = idamax(j - (ll+1), &eta[ll], 1) + ll;
  if (fabs(eta[k]) > reps) {
    reps1 = eps1 / reps;
    iteration = 0;
    flag = TRUE;
    while (iteration < 2 && flag) {
      if (rnm > tol) {
	
	/* bring in a lanczos vector t and orthogonalize both 
	 * r and q against it */
	tq = 0.0;
	tr = 0.0;
	for (i = ll; i < j; i++) {
	  store(n,  RETRQ,  i,  wrk);
	  t   = -ddot(n, qa, 1, wrk, 1);
	  tq += fabs(t);
	  daxpy(n,  t,  wrk,  1,  q,  1);
	  t   = -ddot(n, ra, 1, wrk, 1);
	  tr += fabs(t);
	  daxpy(n, t, wrk, 1, r, 1);
	}
	dcopy(n, q, 1, qa, 1);
	t   = -ddot(n, r, 1, qa, 1);
	tr += fabs(t);
	daxpy(n, t, q, 1, r, 1);
	dcopy(n, r, 1, ra, 1);
	tmp1 = ddot(n, ra, 1, r, 1);
	if (tmp1<0)
	  errormessage("sqrt tmp1 purge","",NON);
	rnm = sqrt(tmp1);

	if (tq <= reps1 && tr <= reps1 * rnm) flag = FALSE;
      }
      iteration++;
    }
    for (i = ll; i <= j; i++) { 
      eta[i] = eps1;
      oldeta[i] = eps1;
    }
  }
  return;
}

/* extern double rnm, anorm, tol, eps, reps; */
/* extern int j, ierr; */
#if 0
void   daxpy(long, double, double *,long, double *, long);
void   datx(long, double, double *,long, double *, long);
void   dcopy(long, double *, long, double *, long);
double ddot(long, double *,long, double *, long);
void   dscal(long, double, double *,long);
double startv(long, double *[]);
void   opb(long, double *, double *);
void   store(long, long, long, double *);
#endif

/***********************************************************************
 *                                                                     *
 *                         stpone()                                    *
 *                                                                     *
 ***********************************************************************/
/***********************************************************************
  
  Description
  -----------
  
  Function performs the first step of the Lanczos algorithm.  It also
  does a step of extended local re-orthogonalization.
  
  Arguments 
  ---------
  
  (input)
  n      dimension of the eigenproblem for matrix B
  
  (output)
  ierr   error flag
  wptr   array of pointers that point to work space that contains
  wptr[0]             r[j]
  wptr[1]             q[j]
  wptr[2]             q[j-1]
  wptr[3]             p
  wptr[4]             p[j-1]
  wptr[6]             diagonal elements of matrix T 
  
  
  Functions used
  --------------
  
  BLAS		daxpy, datx, dcopy, ddot, dscal
  USER		store, opb
  LAS		startv
  
  ***********************************************************************/

stpone(n,wrkptr)
     int n;
     double *wrkptr[];
     
{
  double startv(),ddot();
  double t, *alf,tmp1;
  alf = wrkptr[6];
  
  /* get initial vector; default is random */
  rnm = startv(n, wrkptr);
  if (rnm == 0.0 || ierr != 0) return;
  
  /* normalize starting vector */
  t = 1.0 / rnm;
  datx(n, t, wrkptr[0], 1, wrkptr[1], 1);
  dscal(n, t, wrkptr[3], 1);
  
  /* take the first step */
  opb(n, wrkptr[3], wrkptr[0]);
  alf[0] = ddot(n, wrkptr[0], 1, wrkptr[3], 1);
  daxpy(n, -alf[0], wrkptr[1], 1, wrkptr[0], 1);
  t = ddot(n, wrkptr[0], 1, wrkptr[3], 1);
  daxpy(n, -t, wrkptr[1], 1, wrkptr[0], 1);
  alf[0] += t;
  dcopy(n, wrkptr[0], 1, wrkptr[4], 1);
  tmp1 = ddot(n, wrkptr[0], 1, wrkptr[4], 1);
    if (tmp1<0)
      errormessage("sqrt tmp1 stpone","",NON);
  rnm = sqrt(tmp1);
  anorm = rnm + fabs(alf[0]);
  tol = reps * anorm;
  return;
}
#define STORQ 1
#define RETRQ 2
#define STORP 3
#define RETRP 4
/* extern int j, ierr; */
/* extern double eps; */
#if 0
double ddot(long, double *,long, double *, long);
void   daxpy(long, double, double *,long, double *, long);
void   dcopy(long, double *, long, double *, long);
static double random(long *);
void   store(long, long, long, double *);
void   opb(long, double *, double *);
#endif
/***********************************************************************
 *                                                                     *
 *                         startv()                                    *
 *                                                                     *
 ***********************************************************************/
/***********************************************************************
  
  Description
  -----------
  
  Function delivers a starting vector in r and returns |r|; it returns 
  zero if the range is spanned, and ierr is non-zero if no starting 
  vector within range of operator can be found.
  
  Parameters 
  ---------
  
  (input)
  n      dimension of the eigenproblem matrix B
  wptr   array of pointers that point to work space
  j      starting index for a Lanczos run
  eps    machine epsilon (relative precision)
  
  (output)
  wptr   array of pointers that point to work space that contains
  r[j], q[j], q[j-1], p[j], p[j-1]
  ierr   error flag (nonzero if no starting vector can be found)
  
  Functions used
  --------------
  
  BLAS		ddot, dcopy, daxpy
  USER		opb, store
  MISC		random
  
  ***********************************************************************/

double startv(n,wptr)
     int n;
     double *wptr[];
{
  double ddot();
  double rnm2, *r, t;
  int irand;
  int id, i;
  
  /* get initial vector; default is random */
  rnm2 = ddot(n, wptr[0], 1, wptr[0], 1);
  irand = 918273 + j;
  r = wptr[0];
  for (id = 0; id < 3; id++) {
    if (id > 0 || j > 0 || rnm2 == 0) 
      for (i = 0; i < n; i++) r[i] = random(&irand);
    dcopy(n, wptr[0], 1, wptr[3], 1);
    
    /* apply operator to put r in range (essential if m singular) */
    opb(n, wptr[3], wptr[0]);
    dcopy(n, wptr[0], 1, wptr[3], 1);
    rnm2 = ddot(n, wptr[0], 1, wptr[3], 1);
    if (rnm2 > 0.0) break;
  }
  
  /* fatal error */
  if (rnm2 <= 0.0) {
    ierr = 8192;
    return(-1);
  }
  if (j > 0) {
    for (i = 0; i < j; i++) {
      store(n, RETRQ, i, wptr[5]);
      t = -ddot(n, wptr[3], 1, wptr[5], 1);
      daxpy(n, t, wptr[5], 1, wptr[0], 1);
    }
    
    /* make sure q[j] is orthogonal to q[j-1] */
    t = ddot(n, wptr[4], 1, wptr[0], 1);
    daxpy(n, -t, wptr[2], 1, wptr[0], 1);
    dcopy(n, wptr[0], 1, wptr[3], 1);
    t = ddot(n, wptr[3], 1, wptr[0], 1);
    if (t <= eps * rnm2) t = 0.0;
    rnm2 = t;
  }
if (rnm2<0)
      errormessage("sqrt rnm2 startv","",NON);
  return(sqrt(rnm2));
}

/***********************************************************************
 *                                                                     *
 *				random()                               *
 *                        (double precision)                           *
 ***********************************************************************/
/***********************************************************************
  
  Description
  -----------
  
  This is a translation of a Fortran-77 uniform random number
  generator.  The code is based  on  theory and suggestions  given in
  D. E. Knuth (1969),  vol  2.  The argument to the function should 
  be initialized to an arbitrary integer prior to the first call to 
  random.  The calling program should  not  alter  the value of the
  argument between subsequent calls to random.  Random returns values
  within the the interval (0,1).
  
  
  Arguments 
  ---------
  
  (input)
  iy	   an integer seed whose value must not be altered by the caller
  between subsequent calls
  
  (output)
  random  a double precision random number between (0,1)
  
  ***********************************************************************/
static double random(iy)
     int *iy;     
{
  static int m2 = 0;
  static int ia, ic, mic;
  static double halfm, s;
  
  /* If first entry, compute (max int) / 2 */
  if (!m2) {
    m2 = 1 << (8 * (int)sizeof(int) - 2); 
    halfm = m2;
    
    /* compute multiplier and increment for linear congruential 
     * method */
    ia = 8 * (int)(halfm * atan(1.0) / 8.0) + 5;
    ic = 2 * (int)(halfm * (0.5 - sqrt(3.0)/6.0)) + 1;
    mic = (m2-ic) + m2;
    
    /* s is the scale factor for converting to floating point */
    s = 0.5 / halfm;
  }
  
  /* compute next random number */
  *iy = *iy * ia;
  
  /* for computers which do not allow integer overflow on addition */
  if (*iy > mic) *iy = (*iy - m2) - m2;
  
  *iy = *iy + ic;
  
  /* for computers whose word length for addition is greater than
   * for multiplication */
  if (*iy / 2 > m2) *iy = (*iy - m2) - m2;
  
  /* for computers whose integer overflow affects the sign bit */
  if (*iy < 0) *iy = (*iy + m2) + m2;
  
  return((double)(*iy) * s);
}

#if 0
double dmax(double, double);
double dmin(double, double);
#endif

/************************************************************** 
 *							      *
 * Function finds sqrt(a^2 + b^2) without overflow or         *
 * destructive underflow.				      *
 *							      *
 **************************************************************/ 
/************************************************************** 
  
  Funtions used
  -------------
  
  UTILITY	dmax, dmin
  
  **************************************************************/ 

double pythag(mya,b)
     double mya,b;
{
  double dmax(),dmin();
  double p, r, s, t, u, temp;
  
  p = dmax(fabs(mya), fabs(b));
  if (p != 0.0) {
    temp = dmin(fabs(mya), fabs(b)) / p;
    r = temp * temp; 
    t = 4.0 + r;
    while (t != 4.0) {
      s = r / t;
      u = 1.0 + 2.0 * s;
      p *= u;
      temp = s / u;
      r *= temp * temp;
      t = 4.0 + r;
    }
  }
  return(p);
}
/* extern double tol, eps34, eps; */
/* extern int j, neig; */
#if 0
long   idamax(long, double *, long);
double dmin(double, double);
#endif

/***********************************************************************
 *                                                                     *
 *			error_bound()                                  *
 *                                                                     *
 ***********************************************************************/
/***********************************************************************
  
  Description
  -----------
  
  Function massages error bounds for very close ritz values by placing 
  a gap between them.  The error bounds are then refined to reflect 
  this.
  
  
  Arguments 
  ---------
  
  (input)
  endl     left end of interval containing unwanted eigenvalues
  endr     right end of interval containing unwanted eigenvalues
  ritz     array to store the ritz values
  bnd      array to store the error bounds
  enough   stop flag
  
  
  Functions used
  --------------
  
  BLAS		idamax
  UTILITY	dmin
  
  ***********************************************************************/

error_bound(enough,endl,endr,ritz,bnd)
     int *enough;
     double endl;
     double endr;
     double *ritz;
     double *bnd;
{
  double dmin();
  int mid, i;
  double gapl, gap;
  
  /* massage error bounds for very close ritz values */
  mid = idamax(j + 1, bnd, 1);
  
  for (i=((j+1) + (j-1)) / 2; i >= mid + 1; i -= 1)
    if (fabs(ritz[i-1] - ritz[i]) < eps34 * fabs(ritz[i])) 
      if (bnd[i] > tol && bnd[i-1] > tol) {
	bnd[i-1] = sqrt(bnd[i] * bnd[i] + bnd[i-1] * bnd[i-1]);
	bnd[i] = 0.0;
      }
  
  
  for (i=((j+1) - (j-1)) / 2; i <= mid - 1; i +=1 ) 
    if (fabs(ritz[i+1] - ritz[i]) < eps34 * fabs(ritz[i])) 
      if (bnd[i] > tol && bnd[i+1] > tol) {
	bnd[i+1] = sqrt(bnd[i] * bnd[i] + bnd[i+1] * bnd[i+1]);
	bnd[i] = 0.0;
      }
  
  /* refine the error bounds */
  neig = 0;
  gapl = ritz[j] - ritz[0];
  for (i = 0; i <= j; i++) {
    gap = gapl;
    if (i < j) gapl = ritz[i+1] - ritz[i];
    gap = dmin(gap, gapl);
    if (gap > bnd[i]) bnd[i] = bnd[i] * (bnd[i] / gap);
    if (bnd[i] <= 16.0 * eps * fabs(ritz[i])) {
      neig += 1;
      if (!*enough) *enough = endl < ritz[i] && ritz[i] < endr;
    }
  }   
  return;
}
/* extern int ierr; */
#if 0
double pythag(double, double);
double fsign(double, double);
#endif
/***********************************************************************
 *                                                                     *
 *				imtqlb()			       *
 *                                                                     *
 ***********************************************************************/
/***********************************************************************
  
  Description
  -----------
  
  imtqlb() is a translation of a Fortran version of the Algol
  procedure IMTQL1, Num. Math. 12, 377-383(1968) by Martin and 
  Wilkinson, as modified in Num. Math. 15, 450(1970) by Dubrulle.  
  Handbook for Auto. Comp., vol.II-Linear Algebra, 241-248(1971).  
  See also B. T. Smith et al, Eispack Guide, Lecture Notes in 
  Computer Science, Springer-Verlag, (1976).
  
  The function finds the eigenvalues of a symmetric tridiagonal
  matrix by the implicit QL method.
  
  
  Arguments 
  ---------
  
  (input)
  n      order of the symmetric tridiagonal matrix                   
  d      contains the diagonal elements of the input matrix           
  e      contains the subdiagonal elements of the input matrix in its
  last n-1 positions.  e[0] is arbitrary	             
  
  (output)
  d      contains the eigenvalues in ascending order.  if an error
  exit is made, the eigenvalues are correct and ordered for
  indices 0,1,...ierr, but may not be the smallest eigenvalues.
  e      has been destroyed.					    
  ierr   set to zero for normal return, j if the j-th eigenvalue has
  not been determined after 30 iterations.		    
  
  Functions used
  --------------
  
  UTILITY	fsign
  MISC		pythag
  
  ***********************************************************************/

imtqlb(n,d,e,bnd)
     int n;
     double d[];
     double e[];
     double bnd[];
{
  double fsign();
  int last, l, m, i, iteration;
  
  /* various flags */
  int exchange, convergence, underflow;	
  
  double b, test, g, r, s, c, p, f;
  
  if (n == 1) return;
  ierr = 0;
  bnd[0] = 1.0;
  last = n - 1;
  for (i = 1; i < n; i++) {
    bnd[i] = 0.0;
    e[i-1] = e[i];
  }
  e[last] = 0.0;
  for (l = 0; l < n; l++) {
    iteration = 0;
    while (iteration <= 30) {
      for (m = l; m < n; m++) {
	convergence = FALSE;
	if (m == last) break;
	else {
	  test = fabs(d[m]) + fabs(d[m+1]);
	  if (test + fabs(e[m]) == test) convergence = TRUE;
	}
	if (convergence) break;
      }
      p = d[l]; 
      f = bnd[l]; 
      if (m != l) {
	if (iteration == 30) {
	  ierr = l;
	  return;
	}
	iteration += 1;
	/*........ form shift ........*/
	g = (d[l+1] - p) / (2.0 * e[l]);
	r = pythag(g, 1.0);
	g = d[m] - p + e[l] / (g + fsign(r, g));
	s = 1.0;
	c = 1.0;
	p = 0.0;
	underflow = FALSE;
	i = m - 1;
	while (underflow == FALSE && i >= l) {
	  f = s * e[i];
	  b = c * e[i];
	  r = pythag(f, g);
	  e[i+1] = r;
	  if (r == 0.0) underflow = TRUE;
	  else {
	    s = f / r;
	    c = g / r;
	    g = d[i+1] - p;
	    r = (d[i] - g) * s + 2.0 * c * b;
	    p = s * r;
	    d[i+1] = g + p;
	    g = c * r - b;
	    f = bnd[i+1];
	    bnd[i+1] = s * bnd[i] + c * f;
	    bnd[i] = c * bnd[i] - s * f;
	    i--;
	  }
	}       /* end while (underflow != FALSE && i >= l) */
	/*........ recover from underflow .........*/
	if (underflow) {
	  d[i+1] -= p;
	  e[m] = 0.0;
	}
	else {
	  d[l] -= p;
	  e[l] = g;
	  e[m] = 0.0;
	}
      } 		       		   /* end if (m != l) */
      else {
	
	/* order the eigenvalues */
	exchange = TRUE;
	if (l != 0) {
	  i = l;
	  while (i >= 1 && exchange == TRUE) {
	    if (p < d[i-1]) {
	      d[i] = d[i-1];
	      bnd[i] = bnd[i-1];
	      i--;
	    }
	    else exchange = FALSE;
	  }
	}
	if (exchange) i = 0;
	d[i] = p;
	bnd[i] = f; 
	iteration = 31;
      }
    }			       /* end while (iteration <= 30) */
  }				   /* end for (l=0; l<n; l++) */
  return;
}						  /* end main */
/* extern int ierr; */
#if 0
double fsign(double, double);
double pythag(double, double);
#endif
/***********************************************************************
 *                                                                     *
 *				imtql2()			       *
 *                                                                     *
 ***********************************************************************/
/***********************************************************************
  
  Description
  -----------
  
  imtql2() is a translation of a Fortran version of the Algol
  procedure IMTQL2, Num. Math. 12, 377-383(1968) by Martin and 
  Wilkinson, as modified in Num. Math. 15, 450(1970) by Dubrulle.  
  Handbook for Auto. Comp., vol.II-Linear Algebra, 241-248(1971).  
  See also B. T. Smith et al, Eispack Guide, Lecture Notes in 
  Computer Science, Springer-Verlag, (1976).
  
  This function finds the eigenvalues and eigenvectors of a symmetric
  tridiagonal matrix by the implicit QL method.
  
  
  Arguments
  ---------
  
  (input)                                                             
  nm     row dimension of the symmetric tridiagonal matrix           
  n      order of the matrix                                        
  d      contains the diagonal elements of the input matrix        
  e      contains the subdiagonal elements of the input matrix in its
  last n-1 positions.  e[0] is arbitrary	             
  z      contains the identity matrix				    
  
  (output)                                                       
  d      contains the eigenvalues in ascending order.  if an error
  exit is made, the eigenvalues are correct but unordered for
  for indices 0,1,...,ierr.				   
  e      has been destroyed.					  
  z      contains orthonormal eigenvectors of the symmetric   
  tridiagonal (or full) matrix.  if an error exit is made,
  z contains the eigenvectors associated with the stored 
  eigenvalues.					
  ierr   set to zero for normal return, j if the j-th eigenvalue has
  not been determined after 30 iterations.		    
  
  
  Functions used
  --------------
  UTILITY	fsign
  MISC		pythag
  
  ***********************************************************************/

imtql2(nm,n,d,e,z)
     int nm;
     int n;
     double d[];
     double e[];
     double z[];
{
  double fsign();
  int index, nnm, h, last, l, m, i, k, iteration, convergence, underflow;
  double b, test, g, r, s, c, p, f;
  if (n == 1) return;
  ierr = 0;
  last = n - 1;
  for (i = 1; i < n; i++) e[i-1] = e[i];
  e[last] = 0.0;
  nnm = n * nm;
  for (l = 0; l < n; l++) {
    iteration = 0;
    
    /* look for small sub-diagonal element */
    while (iteration <= 30) {
      for (m = l; m < n; m++) {
	convergence = FALSE;
	if (m == last) break;
	else {
	  test = fabs(d[m]) + fabs(d[m+1]);
	  if (test + fabs(e[m]) == test) convergence = TRUE;
	}
	if (convergence) break;
      }
      if (m != l) {
	
	/* set error -- no convergence to an eigenvalue after
	 * 30 iterations. */     
	if (iteration == 30) {
	  ierr = l;
	  return;
	}
	p = d[l]; 
	iteration += 1;
	
	/* form shift */
	g = (d[l+1] - p) / (2.0 * e[l]);
	r = pythag(g, 1.0);
	g = d[m] - p + e[l] / (g + fsign(r, g));
	s = 1.0;
	c = 1.0;
	p = 0.0;
	underflow = FALSE;
	i = m - 1;
	while (underflow == FALSE && i >= l) {
	  f = s * e[i];
	  b = c * e[i];
	  r = pythag(f, g);
	  e[i+1] = r;
	  if (r == 0.0) underflow = TRUE;
	  else {
	    s = f / r;
	    c = g / r;
	    g = d[i+1] - p;
	    r = (d[i] - g) * s + 2.0 * c * b;
	    p = s * r;
	    d[i+1] = g + p;
	    g = c * r - b;
	    
	    /* form vector */
	    for (k = 0; k < nnm; k += n) {
	      index = k + i;
	      f = z[index+1];
	      z[index+1] = s * z[index] + c * f;
	      z[index] = c * z[index] - s * f;
	    } 
	    i--;
	  }
	}   /* end while (underflow != FALSE && i >= l) */
	/*........ recover from underflow .........*/
	if (underflow) {
	  d[i+1] -= p;
	  e[m] = 0.0;
	}
	else {
	  d[l] -= p;
	  e[l] = g;
	  e[m] = 0.0;
	}
      }
      else break;
    }		/*...... end while (iteration <= 30) .........*/
  }		/*...... end for (l=0; l<n; l++) .............*/
  
  /* order the eigenvalues */
  for (l = 1; l < n; l++) {
    i = l - 1;
    k = i;
    p = d[i];
    for (h = l; h < n; h++) {
      if (d[h] < p) {
	k = h;
	p = d[h];
      }
    }
    /* ...and corresponding eigenvectors */
    if (k != i) {
      d[k] = d[i];
      d[i] = p;
      for (h = 0; h < nnm; h += n) {
	p = z[h+i];
	z[h+i] = z[h+k];
	z[h+k] = p;
      }
    }   
  }
  return;
}		/*...... end main ............................*/
/* extern double eps; */

/***********************************************************************
 *                                                                     *
 *				machar()			       *
 *                                                                     *
 ***********************************************************************/
/***********************************************************************
  
  Description
  -----------
  
  This function is a partial translation of a Fortran-77 subroutine 
  written by W. J. Cody of Argonne National Laboratory.
  It dynamically determines the listed machine parameters of the
  floating-point arithmetic.  According to the documentation of
  the Fortran code, "the determination of the first three uses an
  extension of an algorithm due to M. Malcolm, ACM 15 (1972), 
  pp. 949-951, incorporating some, but not all, of the improvements
  suggested by M. Gentleman and S. Marovich, CACM 17 (1974), 
  pp. 276-277."  The complete Fortran version of this translation is
  documented in W. J. Cody, "Machar: a Subroutine to Dynamically 
  Determine Determine Machine Parameters," TOMS 14, December, 1988.
  
  
  Parameters reported 
  -------------------
  
  ibeta     the radix for the floating-point representation       
  it        the number of base ibeta digits in the floating-point
  significand					 
  irnd      0 if floating-point addition chops		      
  1 if floating-point addition rounds, but not in the 
  ieee style					
  2 if floating-point addition rounds in the ieee style
  3 if floating-point addition chops, and there is    
  partial underflow				
  4 if floating-point addition rounds, but not in the
  ieee style, and there is partial underflow    
  5 if floating-point addition rounds in the ieee style,
  and there is partial underflow                   
  machep    the largest negative integer such that              
  1.0+float(ibeta)**machep .ne. 1.0, except that 
  machep is bounded below by  -(it+3)          
  negeps    the largest negative integer such that          
  1.0-float(ibeta)**negeps .ne. 1.0, except that 
  negeps is bounded below by  -(it+3)	       
  
  ***********************************************************************/

machar(ibeta,it,irnd,machep,negep)
     int *ibeta;
     int *it;
     int *irnd;
     int *machep;
     int *negep;
{
  
  double beta, betain, betah, mya, b, ZERO, ONE, TWO, temp, tempa, temp1;
  int i, itemp;
  
  ONE = (double) 1;
  TWO = ONE + ONE;
  ZERO = ONE - ONE;
  
  mya = ONE;
  temp1 = ONE;
  while (temp1 - ONE == ZERO) {
    mya = mya + mya;
    temp = mya + ONE;
    temp1 = temp - mya;
  }
  
  b = ONE;
  itemp = 0;
  while (itemp == 0) {
    b = b + b;
    temp = mya + b;
    itemp = (int)(temp - mya);
  }
  *ibeta = itemp;
  beta = (double) *ibeta;
  
  *it = 0;
  b = ONE;
  temp1 = ONE;
  while (temp1 - ONE == ZERO) {
    *it = *it + 1;
    b = b * beta;
    temp = b + ONE;
    temp1 = temp - b;
  }
  *irnd = 0; 
  betah = beta / TWO; 
  temp = mya + betah;
  if (temp - mya != ZERO) *irnd = 1;
  tempa = mya + beta;
  temp = tempa + betah;
  if ((*irnd == 0) && (temp - tempa != ZERO)) *irnd = 2;
  
  
  *negep = *it + 3;
  betain = ONE / beta;
  mya = ONE;
  for (i = 0; i < *negep; i++) mya = mya * betain;
  b = mya;
  temp = ONE - mya;
  while (temp-ONE == ZERO) {
    mya = mya * beta;
    *negep = *negep - 1;
    temp = ONE - mya;
  }
  *negep = -(*negep);
  
  *machep = -(*it) - 3;
  mya = b;
  temp = ONE + mya;
  while (temp - ONE == ZERO) {
    mya = mya * beta;
    *machep = *machep + 1;
    temp = ONE + mya;
  }
  eps = mya;
  return;
}
#define MAXLL 2
#define STORQ 1
#define RETRQ 2
#define STORP 3
#define RETRP 4
/* extern double *a; */
#if 0
void   dcopy(long, double *, long, double *, long);
#endif

/***********************************************************************
 *                                                                     *
 *                     store()                                         *
 *                                                                     *
 ***********************************************************************/
/***********************************************************************
  
  Description
  -----------
  
  store() is a user-supplied function which, based on the input
  operation flag, stores to or retrieves from memory a vector.
  
  
  Arguments 
  ---------
  
  (input)
  n       length of vector to be stored or retrieved
  isw     operation flag:
  isw = 1 request to store j-th Lanczos vector q(j)
  isw = 2 request to retrieve j-th Lanczos vector q(j)
  isw = 3 request to store q(j) for j = 0 or 1
  isw = 4 request to retrieve q(j) for j = 0 or 1
  s	   contains the vector to be stored for a "store" request 
  
  (output)
  s	   contains the vector retrieved for a "retrieve" request 
  
  Functions used
  --------------
  
  BLAS		dcopy
  
  ***********************************************************************/

store(n,isw,k,s)
     int n;
     int isw;
     int k;
     double *s;     
{
  switch(isw) {
  case STORQ:	dcopy(n, s, 1, &a[(k+MAXLL) * n], 1);
    break;
  case RETRQ:	dcopy(n, &a[(k+MAXLL) * n], 1, s, 1);
    break;
  case STORP:	if (k >= MAXLL) {
    fprintf(stderr,"store: (STORP) k >= MAXLL \n");
    break;
  }
    dcopy(n, s, 1, &a[k*n], 1);
    break;
  case RETRP:	if (k >= MAXLL) {
    fprintf(stderr,"store: (RETRP) k >= MAXLL \n");
    break;
  }
    dcopy(n, &a[k*n], 1, s, 1);
    break;
  }
  return;
}

double fsign(mya,b)
     double mya;
     double b;
     /************************************************************** 
      * returns |a| if b is positive; else fsign returns -|a|      *
      **************************************************************/ 
{
  
  if ((mya>=0.0 && b>=0.0) || (mya<0.0  && b<0.0))
    return(mya);
  if ((mya<0.0  && b>=0.0) || (mya>=0.0 && b<0.0))
    return(-mya);
  errormessage("fsign","",NON);
  return(0.0);
}

double dmax(mya,b)
     double mya; 
     double b;
     /************************************************************** 
      * returns the larger of two double precision numbers         *
      **************************************************************/ 
{
  
  if (mya > b) return(mya);
  else return(b);
}

double dmin(mya,b)
     double mya;
     double b;
     /************************************************************** 
      * returns the smaller of two double precision numbers        *
      **************************************************************/ 
{
  
  if (mya < b) return(mya);
  else return(b);
}

int imin(mya,b)
     int mya,b;
     /************************************************************** 
      * returns the smaller of two integers                        *
      **************************************************************/ 
{
  
  if (mya < b) return(mya);
  else return(b);
}

/************************************************************** 
 * returns the larger of two integers                         *
 **************************************************************/ 

int imax(mya,b)
     int mya,b;
{
  
  if (mya > b) return(mya);
  else return(b);
}

/************************************************************** 
 * Constant times a vector plus a vector     		      *
 * Based on Fortran-77 routine from Linpack by J. Dongarra    *
 **************************************************************/ 

daxpy(n,da,dx,incx,dy,incy)
     int n;
     double da;
     double *dx;
     int incx;
     double *dy;
     int incy;
{
  int i;
  
  if (n <= 0 || incx == 0 || incy == 0 || da == 0.0) return;
  if (incx == 1 && incy == 1) 
    for (i=0; i < n; i++) {
      *dy = *dy + da * (*dx++);
      dy++;
    }
  else {
    if (incx < 0) dx += (-n+1) * incx;
    if (incy < 0) dy += (-n+1) * incy;
    for (i=0; i < n; i++) {
      *dy = *dy + da * (*dx);
      dx += incx;
      dy += incy;
    }
  }
  return;
}

/************************************************************** 
 * Function forms the dot product of two vectors.      	      *
 * Based on Fortran-77 routine from Linpack by J. Dongarra    *
 **************************************************************/ 

double ddot(n,dx,incx,dy,incy)
     int n;
     double *dx;
     int incx;
     double *dy;
     int incy;
{
  int i;
  double dot_product;
  
  if (n <= 0 || incx == 0 || incy == 0) return(0.0);
  dot_product = 0.0;
  if (incx == 1 && incy == 1) 
    for (i=0; i < n; i++) dot_product += (*dx++) * (*dy++);
  else {
    if (incx < 0) dx += (-n+1) * incx;
    if (incy < 0) dy += (-n+1) * incy;
    for (i=0; i < n; i++) {
      dot_product += (*dx) * (*dy);
      dx += incx;
      dy += incy;
    }
  }
  return(dot_product);
}
/************************************************************** 
 * function scales a vector by a constant.	     	      *
 * Based on Fortran-77 routine from Linpack by J. Dongarra    *
 **************************************************************/ 

datx(n,da,dx,incx,dy,incy)
     int n;double da;double *dx;int incx;double *dy;int incy;
{
  int i;
  
  if (n <= 0 || incx == 0 || incy == 0 || da == 0.0) return;
  if (incx == 1 && incy == 1) 
    for (i=0; i < n; i++) *dy++ = da * (*dx++);
  
  else {
    if (incx < 0) dx += (-n+1) * incx;
    if (incy < 0) dy += (-n+1) * incy;
    for (i=0; i < n; i++) {
      *dy = da * (*dx);
      dx += incx;
      dy += incy;
    }
  }
  return;
}
/********************************************************************* 
 * Function sorts array1 and array2 into increasing order for array1 *
 *********************************************************************/

dsort2(igap,n,array1,array2)
     int igap;int n;double *array1;double *array2;
{
  double temp;
  int i, k, index;
  
  if (!igap) return;
  else {
    for (i = igap; i < n; i++) {
      k = i - igap;
      index = i;
      while (k >= 0 && array1[k] > array1[index]) {
	temp = array1[k];
	array1[k] = array1[index];
	array1[index] = temp;
	temp = array2[k];
	array2[k] = array2[index];
	array2[index] = temp;
	k -= igap;
	index = k + igap;
      }
    } 
  }
  dsort2(igap/2,n,array1,array2);
}
/************************************************************** 
 * Function interchanges two vectors		     	      *
 * Based on Fortran-77 routine from Linpack by J. Dongarra    *
 **************************************************************/ 

dswap(n,dx,incx,dy,incy)
     int n;double *dx;int incx;double *dy;int incy;
{
  int i;
  double dtemp;
  
  if (n <= 0 || incx == 0 || incy == 0) return;
  if (incx == 1 && incy == 1) {
    for (i=0; i < n; i++) {
      dtemp = *dy;
      *dy++ = *dx;
      *dx++ = dtemp;
    }	
  }
  else {
    if (incx < 0) dx += (-n+1) * incx;
    if (incy < 0) dy += (-n+1) * incy;
    for (i=0; i < n; i++) {
      dtemp = *dy;
      *dy = *dx;
      *dx = dtemp;
      dx += incx;
      dy += incy;
    }
  }
}

/***************************************************************** 
 * Function finds the index of element having max. absolute value*
 * based on FORTRAN 77 routine from Linpack by J. Dongarra       *
 *****************************************************************/ 

int idamax(n,dx,incx)
     int n;double *dx;int incx;
{
  int ix,i,myimax;
  double dtemp, mydmax;
  
  if (n < 1) return(-1);
  if (n == 1) return(0);
  if (incx == 0) return(-1);
  
  if (incx < 0) ix = (-n+1) * incx;
  else ix = 0;
  myimax = ix;
  dx += ix;
  mydmax = fabs(*dx);
  for (i=1; i < n; i++) {
    ix += incx;
    dx += incx;
    dtemp = fabs(*dx);
    if (dtemp > mydmax) {
      mydmax = dtemp;
      myimax = ix;
    }
  }
  return(myimax);
}
/************************************************************** 
 * Function scales a vector by a constant.     		      *
 * Based on Fortran-77 routine from Linpack by J. Dongarra    *
 **************************************************************/ 

dscal(n,da,dx,incx)
     int n;double da;double *dx;int incx;
{
  int i;
  
  if (n <= 0 || incx == 0) return;
  if (incx < 0) dx += (-n+1) * incx;
  for (i=0; i < n; i++) {
    *dx *= da;
    dx += incx;
  }
  return;
}
/************************************************************** 
 * Function copies a vector x to a vector y	     	      *
 * Based on Fortran-77 routine from Linpack by J. Dongarra    *
 **************************************************************/ 

dcopy(n,dx,incx,dy,incy)
     int n;double *dx;int incx;double *dy;int incy;     
{
  int i;
  
  if (n <= 0 || incx == 0 || incy == 0) return;
  if (incx == 1 && incy == 1) 
    for (i=0; i < n; i++) *dy++ = *dx++;
  
  else {
    if (incx < 0) dx += (-n+1) * incx;
    if (incy < 0) dy += (-n+1) * incy;
    for (i=0; i < n; i++) {
      *dy = *dx;
      dx += incx;
      dy += incy;
    }
  }
  return;
}

#if 0
float timer() {
  long elapsed_time;
  struct rusage mytime;
  getrusage(RUSAGE_SELF,&mytime);
  
  /* convert elapsed time to milliseconds */
  elapsed_time = (mytime.ru_utime.tv_sec * 1000 + 
		  mytime.ru_utime.tv_usec / 1000);
  
  /* return elapsed time in seconds */
  return((float)elapsed_time/1000.);
}
#endif

float timer() {
  return(0.0);
}
