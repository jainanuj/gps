
#ifndef _SMALL_MATVEC_H
#define _SMALL_MATVEC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * ----------------------------------------------------------------------------
 */

extern int verbose;

/* epsilon for determining when a singular matrix is encountered */
#define EPS 10e-20

/* this is the default "m" parameter to GMRES */
#define DEFAULT_GMRES_M 15

#define DIE_IF_NULL(x) { \
  if ((x)==NULL) { \
    fprintf(stderr,"Null pointer at %s,%d!\n", __FILE__, __LINE__); \
    abort();\
  } \
}

#define DIE_IF_FALSE(x) { \
  if (!(x)) { \
    fprintf(stderr,"test failed at %s,%d!\n", __FILE__, __LINE__); \
    abort();\
  } \
}

/*
 * ----------------------------------------------------------------------------
 */

/* the precision type used throughout the code */
/* typedef float prec_t; */
/* #define PREC_IS_FLOAT */

typedef double prec_t;
#define PREC_IS_DOUBLE

typedef struct vec_t {
  int nelts;
  prec_t *elts;
} vec_t;

/* this describes one entry in the matrix */
typedef struct entry_t {
  int col;
  prec_t entry;
} entry_t;

/* this describes one row/column in the matrix */
typedef struct rowcol_t {
  int colcnt;
  entry_t *entries;
} rowcol_t;

typedef struct matrix_t {
  int nrows, ncols;
  rowcol_t *rows;
  entry_t *__row_entries;
  int total_nz;
  int add_identity;
} matrix_t;

/*
 * ----------------------------------------------------------------------------
 */

#define MIN_SOLVER            1

/* these are the generally available solvers */
#define RICHARDSON            1
#define STEEPEST_DESCENT      3
#define MINIMUM_RESIDUAL      4
#define RESIDUAL_NORM_SD      6
#define GMRES                 7
#define GAUSSIAN_ELIM         8
#define CONJUGATE_GRAD        9
#define CONJUGATE_GRAD_SQ     10
#define CONJUGATE_GRAD_NR     11
#define CONJUGATE_GRAD_NE     12
#define MAX_SOLVER            12

/* these are Aztec only solvers */
#define AZ_MIN_SOLVER        100
#define AZ_GMRES             100
#define AZ_CONJUGATE_GRAD    101
#define AZ_CONJUGATE_GRAD_SQ 102
#define AZ_TFQMR             103
#define AZ_BICGSTAB          104
#define AZ_LU                105
#define AZ_MAX_SOLVER        105

char *solver_name( int solver );
int is_normal_solver( int solver );
int is_aztec_solver( int solver );

/*
 * ============================================================================
 *
 * Vector and matrix routines
 *
 * ============================================================================
 */

void dump_matrix( matrix_t *A );
matrix_t *basic_matrix_allocate( int rows, int cols );
void nelts_matrix_allocate( matrix_t *A, int nelts );
void matrix_patch_entries( matrix_t *A );
void add_entry( matrix_t *A, rowcol_t *r, int rc, prec_t entry );
matrix_t *load_mtx_file( char *fn );
void save_mtx_file( matrix_t *A, char *fn );
prec_t **alloc_twod_prect( int rows, int cols );
void dealloc_twod_prect( prec_t **d, int rows );

/*
 * ----------------------------------------------------------------------------
 */

prec_t get_entry( matrix_t *A, int r, int c );
void vec_save( char *fn, vec_t *a );
void vec_load( char *fn, vec_t *a );

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

int is_matrix_sound( matrix_t *A );
int is_matrix_dangerous( matrix_t *A );
prec_t matrix_max_elem( matrix_t *A );
prec_t matrix_find_entry( matrix_t *A, int row, int col );

/*
 * ----------------------------------------------------------------------------
 */

void vec_show( vec_t *v );
void vec_show_at( vec_t *v, int ind );
void vec_show_at_s( vec_t *v, int ind );
vec_t *vec_create( int nelts );
void vec_free( vec_t *v );
vec_t *new_vec_from_matrix( matrix_t *A );
vec_t *new_vec_from_matrix_transpose( matrix_t *A );
void vec_copy( vec_t *a, vec_t *r );
vec_t *vec_clone( vec_t *a );

/*
 * ----------------------------------------------------------------------------
 */

void matrix_vec_mult( matrix_t *A, vec_t *b, vec_t *r );
void matrix_transpose_vec_mult( matrix_t *A, vec_t *b, vec_t *r );
prec_t matrixrow_vec_mult( matrix_t *A, vec_t *b, int row );
prec_t entries_vec_mult( entry_t *et, int cnt, vec_t *b );
void matrix_scale( matrix_t *A, prec_t c );

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

void const_vec( vec_t *a, prec_t val );
void rand_vec( vec_t *a );
void const_mult_vec( vec_t *a, prec_t c, vec_t *r );

/*
 * ----------------------------------------------------------------------------
 */

prec_t vec_two_norm( vec_t *a );
prec_t vec_max_norm( vec_t *a );
prec_t vec_dot( vec_t *a, vec_t *b );
void vec_sub( vec_t *a, vec_t *b, vec_t *r );
void vec_add( vec_t *a, vec_t *b, vec_t *r );
int vec_max_elem_index( vec_t *a );
prec_t vec_max_elem_val( vec_t *a );
void vec_mult( vec_t *a, vec_t *b, vec_t *r );

/*
 * ============================================================================
 *
 * Here is where we collect all of the solvers that we can use
 *
 * ============================================================================
 */

int richardson( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
		int max_it, prec_t tol,
		prec_t *error, int *iters );

int steepest_descent( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
		      int max_it, prec_t tol, prec_t *error, int *iters );

int minimum_residual( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
		      int max_it, prec_t tol, prec_t *error, int *iters );

int residual_norm_sd( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
		      int max_it, prec_t tol, prec_t *error, int *iters );

void GeneratePlaneRotation( prec_t dx, prec_t dy, prec_t *cs, prec_t *sn );
void ApplyPlaneRotation( prec_t *dx, prec_t *dy, prec_t cs, prec_t sn );
void Update( vec_t *x, int k, prec_t **h, vec_t *s, vec_t **v );

int gmres( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
	   int max_iter, prec_t tol, prec_t *error, int *iters,
	   int m );

int cg( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
	int max_iter, prec_t tol, prec_t *error, int *iters );

int cgs( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
	 int max_iter, prec_t tol, prec_t *error, int *iters );

int cgnr( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
	  int max_iter, prec_t tol, prec_t *error, int *iters );

int cgne( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
	  int max_iter, prec_t tol, prec_t *error, int *iters );

int direct_gaussian_solver( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
			    int max_it, prec_t tol,
			    prec_t *error, int *iters );

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

#endif /* ndef _SMALL_MATVEC_H */
