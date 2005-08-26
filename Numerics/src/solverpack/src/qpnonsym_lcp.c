#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*!\file qpnonsym_lcp.c


This subroutine allows the resolution of LCP (Linear Complementary Problem).
Try \f$(z,w)\f$ such that:

\f$
\left\lbrace
\begin{array}{l}
M z- w=q\\
0 \le z \perp w \ge 0\\
\end{array}
\right.
\f$

here M is an n by n  matrix, q an n-dimensional vector, w an n-dimensional  vector and z an n-dimensional vector.
*/




/*!\fn  qpnonsym_lcp(double vec[],double *qq,int *nn,int * itermax, double * tol,double z[],double w[],int *it_end,double * res,int *info)

qp_lcp use a quadratic programm formulation for solving an LCP with a non symmetric matrix

\param vec On enter a pointer over doubles containing the components of the double matrix with a fortran90 allocation.
\param qq On enter a pointer over doubles containing the components of the double vector.
\param nn On enter a pointer over integers, the dimension of the second member.
\param itermax On enter a pointer over integers, the maximum iterations required.
\param tol On enter a pointer over doubles, the tolerance required.
\param it_end On enter a pointer over integers, the number of iterations carried out.
\param res On return a pointer over doubles, the error value.
\param z On return double vector, the solution of the problem.
\param w On return double vector, the solution of the problem.
\param info On return a pointer over integers, the termination reason (0 is successful otherwise 1).

\author Vincent Acary
*/

double ddot_(int *, double [], int *, double [], int*);
void ql0001_(int * m, int * me, int *mmax, int *n, int *nmax, int *mnn, double *c, double *d, double *a, double *b, double *xl, double *xu,
             double *x, double *u, int *iout, int *ifail, int *iprint, double *war, int *lwar, int *iwar, int *liwar, double *eps);

void qpnonsym_lcp(double vec[], double *qq, int *nn, double * tol, double z[], double w[], int *info)
{
  int i, j;

  int n = *nn, nmax;
  int m, me, mmax, mnn;

  double *Q, *A;
  double *p, *b, *xl, *xu;

  double *lambda;

  int lwar, liwar, iout, inform, un;
  int *iwar;
  double *war;

  // m :        total number of constraints.
  m = n;
  // me :       number of equality constraints.
  me = 0;
  //  mmax :     row dimension of a. mmax must be at least one and greater than m.
  mmax = m + 1;
  //n :        number of variables.
  //nmax :     row dimension of C. nmax must be greater or equal to n.
  nmax = n;
  //mnn :      must be equal to m + n + n.
  mnn = m + n + n;

  for (i = 0; i < n; i++)
  {
    z[i] = 0.0;
    w[i] = 0.0;
  }



  // Creation of objective function matrix Q and the the constant vector of the objective function p

  Q = (double *)malloc(nmax * nmax * sizeof(double));
  for (i = 0; i < n; i++)
  {
    for (j = 0; j < n; j++) Q[j * n + i] = (vec[j * n + i] + vec[i * n + j]);
  }
  //for (i=0;i<n*n;i++) printf("Q[%i] = %g\n",i,Q[i]);

  p = (double *)malloc(nmax * sizeof(double));
  for (i = 0; i < n; i++)
    p[i] = -qq[i] ;
  //for (i=0;i<n;i++) printf("p[%i] = %g\n",i,p[i]);

  // Creation of the data matrix of the linear constraints, A and  the constant data of the linear constraints b
  A = (double *)malloc(mmax * nmax * sizeof(double));
  for (i = 0; i < m; i++)
  {
    for (j = 0; j < n; j++) A[j * mmax + i] = vec[j * n + i];
  }

  //for (i=0;i<mmax*mmax;i++) printf("A[%i] = %g\n",i,A[i]);

  b = (double *)malloc(mmax * sizeof(double));
  for (i = 0; i < m; i++) b[i] = -qq[i] ;

  //for (i=0;i<m;i++) printf("b[%i] = %g\n",i,b[i]);

  // Creation of the the lower and upper bounds for the variables.
  xu = (double *)malloc(n * sizeof(double));
  for (i = 0; i < n; i++) xu[i] = 1e300 ;
  xl = (double *)malloc(n * sizeof(double));
  for (i = 0; i < n; i++) xl[i] = 0.0 ;

  // on return, lambda contains the lagrange multipliers.
  lambda = (double *)malloc(mnn * sizeof(double));

  //   integer indicating the desired output unit number,
  iout = 6;

  //   output control.
  un = 1;

  // real working array.
  lwar = 3 * nmax * nmax / 2 + 10 * nmax + 2 * mmax;
  war = (double *)malloc(lwar * sizeof(double));
  // integer working array.
  liwar = n ;
  iwar = (int *)malloc(liwar * sizeof(int));
  iwar[0] = 1;


  // call ql0001_
  ql0001_(&m, &me, &mmax, &n, &nmax, &mnn, Q, p, A, b, xl, xu,
          z, lambda, &iout, &inform, &un, war, &lwar, iwar, &liwar, tol);

  //    printf("tol = %10.4e\n",*tol);
  // for (i=0;i<mnn;i++)printf("lambda[%i] = %g\n",i,lambda[i]);
  // for (i=0;i<n;i++)printf("z[%i] = %g\n",i,z[i]);

  // getting the multiplier due to the lower bounds
  for (i = 0; i < n; i++) w[i] = lambda[m + i] ;


  switch (inform)
  {
  case 0 :
    printf("   Minimization sucessfull\n");
    *info = 0;
    break;
  case 1 :
    printf("   Too Many iterations\n");
    break;
  case 2 :
    printf("   Accuracy insuficient to satisfy convergence criterion\n");
    break;
  case 3 :
  case 4 :
  case 5 :
    printf("   Length of working array insufficient\n");
    break;
  default :
    printf("   The constraints are inconstent\n");
    break;

  }

  // memory freeing
  free(Q);
  free(p);
  free(A);
  free(b);
  free(xu);
  free(xl);
  free(lambda);
}
