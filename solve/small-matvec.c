
#include "small-matvec.h"

/*
 * ----------------------------------------------------------------------------
 */

char *solver_name( int solver ) {
  switch( solver ) {
  case RICHARDSON:           return "Richardson";
  case STEEPEST_DESCENT:     return "steepest descent";
  case MINIMUM_RESIDUAL:     return "minimum residual";
  case RESIDUAL_NORM_SD:     return "residual norm steepest descent";
  case GMRES:                return "GMRES";
  case GAUSSIAN_ELIM:        return "Gaussian elimination";
  case CONJUGATE_GRAD:       return "conjugate gradient";
  case CONJUGATE_GRAD_SQ:    return "conjugate gradient squared";
  case CONJUGATE_GRAD_NR:    return "conjugate gradient NR";
  case CONJUGATE_GRAD_NE:    return "conjugate gradient NE";

  case AZ_GMRES:             return "Aztec GMRES";
  case AZ_CONJUGATE_GRAD:    return "Aztec conjugate gradient";
  case AZ_CONJUGATE_GRAD_SQ: return "Aztec conjugate gradient squared";
  case AZ_TFQMR:             return "Aztec TFQMR";
  case AZ_BICGSTAB:          return "Aztec BiCGSTAB";
  case AZ_LU:                return "Aztec LU";

  default: return "UNKNOWN";
  }
}

int is_normal_solver( int solver ) {
  if ( solver >= MIN_SOLVER && solver <= MAX_SOLVER ) {
    return 1;
  }
  return 0;
}

int is_aztec_solver( int solver ) {
  if ( solver >= AZ_MIN_SOLVER && solver <= AZ_MAX_SOLVER ) {
    return 1;
  }
  return 0;
}

/*
 * ============================================================================
 *
 * Vector and matrix routines
 *
 * ============================================================================
 */

void dump_matrix( matrix_t *A ) {
  int i, j;

  printf( "**** DUMPING A MATRIX ****\n" );
  printf( "rows=%d, cols=%d, nnz=%d\n", A->nrows, A->ncols, A->total_nz );

  printf( "ROWS\n" );
  for ( i=0; i<A->nrows; i++ ) {
    for ( j=0; j<A->rows[i].colcnt; j++ ) {
      printf( "  A[%d][%d] = %.9f\n", i,
	      A->rows[i].entries[j].col, 
	      A->rows[i].entries[j].entry );
    }
  }

}

matrix_t *basic_matrix_allocate( int rows, int cols ) {
  matrix_t *A;

  A = (matrix_t *)malloc( sizeof(matrix_t) );
  DIE_IF_NULL( A );
  A->nrows = rows;
  A->ncols = cols;

  /* allocate the row descriptors */
  A->rows = (rowcol_t *)malloc( sizeof(rowcol_t) * rows );
  DIE_IF_NULL( A->rows );
  memset( A->rows, 0, sizeof(rowcol_t) * rows );

  return A;
}

void nelts_matrix_allocate( matrix_t *A, int nelts ) {
  entry_t *row_entries;

  A->total_nz = nelts;

  row_entries = (entry_t *)malloc( sizeof(entry_t) * nelts );
  DIE_IF_NULL( row_entries );
  memset( row_entries, 0, sizeof(entry_t) * nelts );
  A->__row_entries = row_entries;  /* save this in case we want to dealloc */
}

void matrix_patch_entries( matrix_t *A ) {
  int i, row_total;
  entry_t *row_entries;

  row_entries = A->__row_entries;

  row_total = 0;
  for ( i=0; i<A->nrows; i++ ) {
    A->rows[i].entries = row_entries + row_total;
    row_total += A->rows[i].colcnt;
  }
}

void add_entry( matrix_t *A, rowcol_t *r, int rc, prec_t entry ) {
  int i;

  //  if ( rc == 0 && entry == 0 ) {
  //    fprintf( stderr, "WARGH! BAD CHECK!\n" );
    //    exit( 0 );
  //  }

  /* find the next empty entry */
  for ( i=0; i<r->colcnt; i++ ) {
    if ( r->entries[i].col == 0 && r->entries[i].entry == 0 ) {
      r->entries[i].col = rc;
      r->entries[i].entry = entry;
      return;
    }
  }
}

matrix_t *load_mtx_file( char *fn ) {
  FILE *fp;
  matrix_t *A;
  int rows, cols, nelts, row, col, lineno;
  prec_t entry;
  char buf[512];
  long pos;

  fp = fopen( fn, "rb" );
  DIE_IF_NULL( fp );

  /* skip comments */
  lineno = 0;
  while ( fgets( buf, 512, fp ) ) {
    lineno++;
    if ( buf[0] == '%' ) {
      continue;
    }
    break;
  }

  /* parse first line */
  sscanf( buf, "%d %d %d", &rows, &cols, &nelts );

  A = basic_matrix_allocate( rows, cols );
  nelts_matrix_allocate( A, nelts );

  if ( verbose ) {
    printf( "\n" );
    printf( "Loading a %d,%d matrix (%d non-zero entries)\n",
	    rows, cols, nelts );
  }

  pos = ftell( fp );

  /* parse the rest */
  while ( fgets( buf, 512, fp ) ) {
    lineno++;

#ifdef PREC_IS_FLOAT
    sscanf( buf, "%d %d %f", &row, &col, &entry );
#endif
#ifdef PREC_IS_DOUBLE
    sscanf( buf, "%d %d %lf", &row, &col, &entry );
#endif

    row--;
    col--;

    if ( row < 0 || row >= rows || col < 0 || col >= cols ) {
      fprintf( stderr, "Bad row/col: %d,%d (line %d)\n", row, col, lineno );
      exit( 0 );
    }

    A->rows[row].colcnt++;
  }

  matrix_patch_entries( A );

  /* parse it again */
  fseek( fp, pos, SEEK_SET );
  while ( fgets( buf, 512, fp ) ) {

#ifdef PREC_IS_FLOAT
    sscanf( buf, "%d %d %f", &row, &col, &entry );
#endif
#ifdef PREC_IS_DOUBLE
    sscanf( buf, "%d %d %lf", &row, &col, &entry );
#endif

    row--;
    col--;

    add_entry( A, &( A->rows[row] ), col, entry );
  }

  fclose( fp );

  if ( verbose ) {
    printf( "done.\n\n" );
  }

  return A;
}

void save_mtx_file( matrix_t *A, char *fn ) {
  int i, j, rowcnt, colcnt;
  FILE *fp;

  fp = fopen( fn, "wb" );
  DIE_IF_NULL( fp );

  fprintf( fp, "%%%%MatrixMarket matrix coordinate real general\n" );
  fprintf( fp, "%d %d %d\n", A->nrows, A->ncols, A->total_nz );

  rowcnt = A->nrows;
  for ( i=0; i<rowcnt; i++ ) {
    colcnt = A->rows[i].colcnt;
    for ( j=0; j<colcnt; j++ ) {
      fprintf( fp, "%d %d %.9f\n", i+1, A->rows[i].entries[j].col+1,
	       A->rows[i].entries[j].entry );
    }
  }

  fclose( fp );
}

prec_t **alloc_twod_prect( int rows, int cols ) {
  int i;
  prec_t **a;

  a = (prec_t **)malloc( sizeof(prec_t *) * rows );
  DIE_IF_NULL( a );

  for ( i=0; i<rows; i++ ) {
    a[i] = (prec_t *)malloc( sizeof(prec_t) * cols );
    DIE_IF_NULL( a[i] );
  }

  return a;
}

void dealloc_twod_prect( prec_t **d, int rows ) {
  int i;

  for ( i=0; i<rows; i++ ) {
    free( d[i] );
  }

  free( d );
}

/*
 * ----------------------------------------------------------------------------
 */

prec_t get_entry( matrix_t *A, int r, int c ) {
  int i, cnt;

  cnt = A->rows[r].colcnt;
  for ( i=0; i<cnt; i++ ) {
    if ( A->rows[r].entries[i].col == c ) {
      return A->rows[r].entries[i].entry;
    }
  }

  return 0;
}

/*
 * ----------------------------------------------------------------------------
 */

void vec_save( char *fn, vec_t *a ) {
  FILE *fp;
  int i;

  fp = fopen( fn, "wb" );
  if ( fp == NULL ) {
    fprintf( stderr, "Error opening file %s!\n", fn );
    return;
  }

  for ( i=0; i<a->nelts; i++ ) {
    fprintf( fp, "%.9f\n", a->elts[i] );
  }

  fclose( fp );
}

void vec_load( char *fn, vec_t *a ) {
  FILE *fp;
  int rows, cols, i;
  char buf[512];
  prec_t val;

  fp = fopen( fn, "rb" );
  DIE_IF_NULL( fp );

  /* skip comments */
  while ( fgets( buf, 512, fp ) ) {
    if ( buf[0] == '%' ) {
      continue;
    }
    break;
  }

  /* parse first line */
  sscanf( buf, "%d %d", &rows, &cols );

  if ( cols != 1 ) {
    fprintf( stderr, "Whoa! was expecting one column; file says %d\n",
	     cols );
    exit( 0 );
  }
  if ( rows != a->nelts ) {
    fprintf( stderr, "Whoa! mismatch loading vec file: file specifies %d elts, was expecting %d\n", rows, a->nelts );
    exit( 0 );
  }

  /* parse the rest */
  i = 0;
  while ( fgets( buf, 512, fp ) ) {

#ifdef PREC_IS_FLOAT
    sscanf( buf, "%f", &val );
#endif
#ifdef PREC_IS_DOUBLE
    sscanf( buf, "%lf", &val );
#endif

    a->elts[i] = val;
    i++;
    if ( i > a->nelts ) {
      fprintf( stderr, "too many items in file. skipping the remainder...\n" );
      break;
    }
  }

  fclose( fp );

}

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

int is_matrix_sound( matrix_t *A ) {
  int i, ok;

  ok = 1;
  for ( i=0; i<A->nrows; i++ ) {
    if ( A->rows[i].colcnt == 0 ) {
      printf( "Underconstrained system: check entry %d\n", i );
      ok = 0;
    }
  }

  return ok;
}

int is_matrix_dangerous( matrix_t *A ) {
  int i;

  DIE_IF_FALSE( A->nrows == A->ncols );

  for ( i=0; i<A->nrows; i++ ) {
    if ( A->rows[i].colcnt == 0 ) {
      return 1;
    }
  }

  return 0;
}

prec_t matrix_max_elem( matrix_t *A ) {
  int i, j, cnt;
  prec_t max, t;

  /* find the first entry */
  max = 0;
  for ( i=0; i<A->nrows; i++ ) {
    if ( A->rows[i].colcnt != 0 ) {
      max = A->rows[i].entries[0].entry;
      break;
    }
  }
  if ( i == A->nrows ) {
    return 0;
  }

  for ( i=0; i<A->nrows; i++ ) {
    cnt = A->rows[i].colcnt;
    for ( j=0; j<cnt; j++ ) {
      t = fabs( A->rows[i].entries[j].entry );
      if ( t > max ) {
	max = t;
      }
    }
  }

  return max;
}

prec_t matrix_find_entry( matrix_t *A, int row, int col ) {
  int cnt, i;

  cnt = A->rows[row].colcnt;
  for ( i=0; i<cnt; i++ ) {
    if ( A->rows[row].entries[i].col == col ) {
      return A->rows[row].entries[i].entry;
    }
  }

  return 0;
}

/*
 * ----------------------------------------------------------------------------
 */

void vec_show( vec_t *v ) {
  int i, cnt;

  cnt = v->nelts;
  if ( cnt > 10 ) { cnt = 10; }

  for ( i=0; i<cnt; i++ ) {
    fprintf( stderr, "  %d: %.9f\n", i, v->elts[i] );
  }
}

void vec_show_at( vec_t *v, int ind ) {
  int i, start, end;

  start = ind - 5;
  if ( start < 0 ) { start = 0; }
  end = ind + 5;
  if ( end > v->nelts ) { end = v->nelts; }

  for ( i=start; i<end; i++ ) {
    fprintf( stderr, "  %d: %.9f\n", i, v->elts[i] );
  }
}

void vec_show_at_s( vec_t *v, int ind ) {
  int i;

  for ( i=ind; i<ind+10; i++ ) {
    fprintf( stderr, "  %d: %.9f\n", i, v->elts[i] );
  }
}


vec_t *vec_create( int nelts ) {
  vec_t *v;

  v = (vec_t *)malloc( sizeof(vec_t) );
  DIE_IF_NULL( v );

  v->elts = (prec_t *)malloc( sizeof(prec_t)*nelts );
  DIE_IF_NULL( v->elts );
  memset( v->elts, 0, sizeof(prec_t)*nelts );

  v->nelts = nelts;

  return v;
}

void vec_free( vec_t *v ) {
  free( v->elts );
  free( v );
}

vec_t *new_vec_from_matrix( matrix_t *A ) {
  return vec_create( A->ncols );
}

vec_t *new_vec_from_matrix_transpose( matrix_t *A ) {
  return vec_create( A->nrows );
}

void vec_copy( vec_t *a, vec_t *r ) {
  DIE_IF_FALSE( a->nelts == r->nelts );
  memcpy( r->elts, a->elts, sizeof(prec_t)*a->nelts );
}

vec_t *vec_clone( vec_t *a ) {
  vec_t *bob;
  int i;

  bob = vec_create( a->nelts );
  for ( i=0; i<a->nelts; i++ ) {
    bob->elts[i] = a->elts[i];
  }

  return bob;
}


/*
 * ----------------------------------------------------------------------------
 */

void matrix_vec_mult( matrix_t *A, vec_t *b, vec_t *r ) {
  int i, j, rowcnt, colcnt;
  prec_t tmpr;
  entry_t *m;

  DIE_IF_FALSE( A->ncols == b->nelts );
  DIE_IF_FALSE( A->nrows == r->nelts );

  rowcnt = A->nrows;
  for ( i=0; i<rowcnt; i++ ) {
    tmpr = 0;
    m = A->rows[i].entries;
    colcnt = A->rows[i].colcnt;
    for ( j=0; j<colcnt; j++ ) {
      tmpr += m[j].entry * b->elts[m[j].col];
    }
    r->elts[i] = tmpr;
  }

  if ( A->add_identity ) {
    vec_add( r, b, r );
  }
}

void matrix_transpose_vec_mult( matrix_t *A, vec_t *b, vec_t *r ) {
  int i, j, rowcnt, colcnt;
  entry_t *m;

  DIE_IF_FALSE( A->nrows == b->nelts );
  DIE_IF_FALSE( A->ncols == r->nelts );

  memset( r->elts, 0, sizeof(prec_t) * r->nelts );

  rowcnt = A->nrows;
  for ( i=0; i<rowcnt; i++ ) {
    m = A->rows[i].entries;
    colcnt = A->rows[i].colcnt;
    for ( j=0; j<colcnt; j++ ) {
      r->elts[ m[j].col ] += m[j].entry * b->elts[i];
    }
  }

  if ( A->add_identity ) {
    vec_add( r, b, r );
  }
}

prec_t matrixrow_vec_mult( matrix_t *A, vec_t *b, int row ) {
  int j, colcnt;
  prec_t tmpr;
  entry_t *m;

  DIE_IF_FALSE( A->ncols == b->nelts );

  tmpr = 0;
  m = A->rows[row].entries;
  colcnt = A->rows[row].colcnt;
  for ( j=0; j<colcnt; j++ ) {
    tmpr += m[j].entry * b->elts[m[j].col];
  }

  return tmpr;
}

prec_t entries_vec_mult( entry_t *et, int cnt, vec_t *b ) {
  int j;
  prec_t tmpr;

  tmpr = 0;
  for ( j=0; j<cnt; j++ ) {
    tmpr += et[j].entry * b->elts[et[j].col];
  }

  return tmpr;
}

void matrix_scale( matrix_t *A, prec_t c ) {
  int i, cnt;
  entry_t *rowbase;

  cnt = A->total_nz;
  rowbase = A->__row_entries;
  for ( i=0; i<cnt; i++ ) {
    rowbase[i].entry *= c;
  }
}

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

void const_vec( vec_t *a, prec_t val ) {
  int i, cnt;

  cnt = a->nelts;
  for ( i=0; i<cnt; i++ ) {
    a->elts[i] = val;
  }
}

void rand_vec( vec_t *a ) {
  int i, cnt;

  cnt = a->nelts;
  for ( i=0; i<cnt; i++ ) {
    a->elts[i] = (prec_t)rand() / (RAND_MAX+1.0);
  }

}

void const_mult_vec( vec_t *a, prec_t c, vec_t *r ) {
  int i, cnt;

  DIE_IF_FALSE( a->nelts == r->nelts );

  cnt = a->nelts;
  for ( i=0; i<cnt; i++ ) {
    r->elts[i] = a->elts[i] * c;
  }
}

/*
 * ----------------------------------------------------------------------------
 */

prec_t vec_two_norm( vec_t *a ) {
  int i, cnt;
  prec_t r;

  cnt = a->nelts;
  r = 0;
  for ( i=0; i<cnt; i++ ) {
    r += a->elts[i] * a->elts[i];
  }

  return sqrt(r);
}

prec_t vec_max_norm( vec_t *a ) {
  int i, cnt;
  prec_t r, t;

  cnt = a->nelts;
  r = 0;
  for ( i=0; i<cnt; i++ ) {
    t = fabs( a->elts[i] );
    if ( t > r ) {
      r = t;
    }
  }

  return r;
}

/*
 * ----------------------------------------------------------------------------
 */

prec_t vec_dot( vec_t *a, vec_t *b ) {
  int i, cnt;
  prec_t r;

  DIE_IF_FALSE( a->nelts == b->nelts );

  r = 0;
  cnt = a->nelts;
  for ( i=0; i<cnt; i++ ) {
    r += a->elts[i] * b->elts[i];
  }

  return r;
}

void vec_sub( vec_t *a, vec_t *b, vec_t *r ) {
  int i, cnt;

  DIE_IF_FALSE( a->nelts == b->nelts );
  DIE_IF_FALSE( b->nelts == r->nelts );

  cnt = a->nelts;
  for ( i=0; i<cnt; i++ ) {
    r->elts[i] = a->elts[i] - b->elts[i];
  }
}


void vec_add( vec_t *a, vec_t *b, vec_t *r ) {
  int i, cnt;

  DIE_IF_FALSE( a->nelts == b->nelts );
  DIE_IF_FALSE( b->nelts == r->nelts );

  cnt = a->nelts;
  for ( i=0; i<cnt; i++ ) {
    r->elts[i] = a->elts[i] + b->elts[i];
  }
}

int vec_max_elem_index( vec_t *a ) {
  int i, cnt, max_index;
  prec_t max_val, t;

  cnt = a->nelts;
  max_val = fabs( a->elts[0] );
  max_index = 0;

  for ( i=0; i<cnt; i++ ) {
    t = fabs( a->elts[i] );
    if ( t > max_val  ) {
      max_val = t;
      max_index = i;
    }
  }

  return max_index;
}

prec_t vec_max_elem_val( vec_t *a ) {
  int i, cnt, max_index;
  prec_t max_val, t;

  cnt = a->nelts;
  max_val = fabs( a->elts[0] );
  max_index = 0;

  for ( i=0; i<cnt; i++ ) {
    t = fabs( a->elts[i] );
    if ( t > max_val  ) {
      max_val = t;
      max_index = i;
    }
  }

  return max_val;
}


void vec_mult( vec_t *a, vec_t *b, vec_t *r ) {
  int i, cnt;

  DIE_IF_FALSE( a->nelts == b->nelts );
  DIE_IF_FALSE( b->nelts == r->nelts );

  cnt = a->nelts;
  for ( i=0; i<cnt; i++ ) {
    r->elts[i] = a->elts[i] * b->elts[i];
  }
}

/*
 * ============================================================================
 *
 * Here is where we collect all of the solvers that we can use
 *
 * ============================================================================
 */

void add_residual_gs( matrix_t *A, vec_t *x, vec_t *b, vec_t *r ) {
  int i, rowcnt;
  prec_t tmpr;

  if ( A->add_identity ) {
    rowcnt = A->nrows;
    for ( i=0; i<rowcnt; i++ ) {
      tmpr = matrixrow_vec_mult( A, x, i );
      r->elts[i] = b->elts[i] - tmpr - x->elts[i];
      x->elts[i] = b->elts[i] - tmpr;
    }

  } else {
    rowcnt = A->nrows;
    for ( i=0; i<rowcnt; i++ ) {
      tmpr = matrixrow_vec_mult( A, x, i );
      r->elts[i] = b->elts[i] - tmpr;
      x->elts[i] += b->elts[i] - tmpr;
    }
  }
}

int richardson( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
		int max_it, prec_t tol,
		prec_t *error, int *iters ) {
  int i, rval;
  vec_t *tmp;

  rval = 0;
  tmp = new_vec_from_matrix( A );

  for ( i=0; i<max_it; i++ ) {

    /* JACOBI */
/*     matrix_vec_mult( A, x, tmp );  /\* tmp = A*x *\/ */
/*     vec_sub( b, tmp, r );          /\* r = b - tmp *\/ */
/*     vec_add( x, r, x );            /\* x = x + r *\/ */

    /* GAUSS-SEIDEL */
    add_residual_gs( A, x, b, r );

    //    *error = vec_two_norm( r );
    *error = vec_max_norm( r );

    if ( *error <= tol ) {
      rval = 1;
      break;
    }
    //    if ( verbose && i % 10 == 0 ) {
    //      printf( "  e = %.5f\n", *error );
    //    }
    if ( isnan(*error) || isinf(*error) ) {
      if ( verbose ) {
	printf( "  e = %.5f\n", *error );
	printf( "  breaking early...\n" );
      }
      break;
    }
  }

  *iters = i+1;

  /* compute the final residual */
  matrix_vec_mult( A, x, tmp );  /* tmp = A*x */
  vec_sub( b, tmp, r );          /* r = b - tmp */

  //  *error = vec_two_norm( r );
  *error = vec_max_norm( r );

  vec_free( tmp );

  return rval;
}

/*
 * ----------------------------------------------------------------------------
 */

int steepest_descent( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
		      int max_it, prec_t tol, prec_t *error, int *iters ) {
  int i, rval;
  vec_t *tmp;
  prec_t alpha, num, denom;

  rval = 0;
  tmp = new_vec_from_matrix( A );

  matrix_vec_mult( A, x, tmp );  /* tmp = A*x */
  vec_sub( b, tmp, r );          /* r = b - tmp */
  *error = vec_two_norm( r );
  printf( "  Initial error = %.5f\n", *error );

  for ( i=0; i<max_it; i++ ) {
    matrix_vec_mult( A, x, tmp );  /* tmp = A*x */
    vec_sub( b, tmp, r );          /* r = b - tmp */

    matrix_vec_mult( A, r, tmp );  /* tmp = A*r */

    num = vec_dot( r, r );
    denom = vec_dot( r, tmp );
    alpha = num / denom;

    const_mult_vec( r, alpha, r ); /* r = alpha * r */
    vec_add( x, r, x );            /* x = x + r */

    //    *error = vec_two_norm( r );
    *error = vec_max_norm( r );

    if ( *error <= tol ) {
      rval = 1;
      break;
    }
/*     if ( verbose && i % 1000 == 0 ) { */
/*       printf( "  e = %.5f\n", *error ); */
/*     } */
    if ( isnan(*error) || isinf(*error) ) {
      if ( verbose ) {
	printf( "  e = %.5f\n", *error );
	printf( "  breaking early...\n" );
      }
      break;
    }

  }

  *iters = i+1;

  vec_free( tmp );

  return rval;
}

/*
 * ----------------------------------------------------------------------------
 */

int minimum_residual( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
		      int max_it, prec_t tol, prec_t *error, int *iters ) {
  int i, rval;
  vec_t *tmp;
  prec_t alpha, num, denom;

  rval = 0;
  tmp = new_vec_from_matrix( A );

  for ( i=0; i<max_it; i++ ) {
    matrix_vec_mult( A, x, tmp );  /* tmp = A*x */
    vec_sub( b, tmp, r );          /* r = b - tmp */

    matrix_vec_mult( A, r, tmp );  /* tmp = A*r */

    num = vec_dot( tmp, r );       /* <Ar,r>  */
    denom = vec_dot( tmp, tmp );   /* <Ar,Ar> */
    alpha = num / denom;

    const_mult_vec( r, alpha, r ); /* r = alpha * r */
    vec_add( x, r, x );            /* x = x + r */

    //    *error = vec_two_norm( r );
    *error = vec_max_norm( r );

    if ( *error <= tol ) {
      rval = 1;
      break;
    }
/*     if ( verbose && i % 1000 == 0 ) { */
/*       printf( "  e = %.5f\n", *error ); */
/*     } */
    if ( isnan(*error) || isinf(*error) ) {
      if ( verbose ) {
	printf( "  e = %.5f\n", *error );
	printf( "  breaking early...\n" );
      }
      break;
    }
  }

  *iters = i+1;

  vec_free( tmp );

  return rval;
}

/*
 * ----------------------------------------------------------------------------
 */

int residual_norm_sd( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
		      int max_it, prec_t tol, prec_t *error, int *iters ) {
  int i, rval;
  vec_t *tmp, *tmp2, *v;
  prec_t alpha, num, denom, old_error;

  rval = 0;
  v = new_vec_from_matrix( A );
  tmp = new_vec_from_matrix( A );
  tmp2 = new_vec_from_matrix( A );

  matrix_vec_mult( A, x, tmp );  /* tmp = Ax */
  vec_sub( b, tmp, r );          /* r = b - Ax */

  old_error = -1;

  for ( i=0; i<max_it; i++ ) {

    matrix_transpose_vec_mult( A, r, v );  /* v = A'r */

    matrix_vec_mult( A, v, tmp );  /* tmp = Av */

    /* why aren't we doing this???
    num = vec_dot( tmp, v );       // <Av,v>
    */
    num = vec_dot( v, v );         /* <v,v>   */
    denom = vec_dot( tmp, tmp );   /* <Av,Av> */
    alpha = num / denom;

    const_mult_vec( v, alpha, tmp2 ); /* tmp2 = alpha*v */
    vec_add( x, tmp2, x );            /* x = x + alpha*v */

    const_mult_vec( tmp, alpha, tmp ); /* tmp = tmp*alpha */
    vec_sub( r, tmp, r );            /* r = r - alpha*Av */

    //    *error = vec_two_norm( r );
    *error = vec_max_norm( r );

    if ( *error <= tol ) {
      rval = 1;
      break;
    }

    if ( old_error != -1 && old_error - *error < tol*tol ) {
      break;
    }
    old_error = *error;

/*     if ( verbose && i % 1000 == 0 ) { */
/*       printf( "  e = %.5f (alpha=%.5f)\n", *error, alpha ); */
/*     } */
    if ( isnan(*error) || isinf(*error) ) {
      if ( verbose ) {
	printf( "  e = %.5f\n", *error );
	printf( "  breaking early...\n" );
      }
      break;
    }
  }

  *iters = i+1;

  vec_free( v );
  vec_free( tmp );
  vec_free( tmp2 );

  return rval;
}

/*
 * ----------------------------------------------------------------------------
 */

//*****************************************************************
// Iterative template routine -- GMRES
//
// GMRES solves the unsymmetric linear system Ax = b using the 
// Generalized Minimum Residual method
//
// GMRES follows the algorithm described on p. 20 of the 
// SIAM Templates book.
//
// The return value indicates convergence within max_iter (input)
// iterations (0), or no convergence within max_iter iterations (1).
//
// Upon successful return, output arguments have the following values:
//  
//        x  --  approximate solution to Ax = b
// max_iter  --  the number of iterations performed before the
//               tolerance was reached
//      tol  --  the residual after the final iteration
//  
//*****************************************************************

void GeneratePlaneRotation( prec_t dx, prec_t dy, prec_t *cs, prec_t *sn ) {
  prec_t temp;

  if (dy == 0.0) {
    *cs = 1.0;
    *sn = 0.0;
  } else if (fabs(dy) > fabs(dx)) {
    temp = dx / dy;
    *sn = 1.0 / sqrt( 1.0 + temp*temp );
    *cs = temp * (*sn);
  } else {
    temp = dy / dx;
    *cs = 1.0 / sqrt( 1.0 + temp*temp );
    *sn = temp * (*cs);
  }
}

void ApplyPlaneRotation( prec_t *dx, prec_t *dy, prec_t cs, prec_t sn ) {
  prec_t temp;

  temp =  cs * (*dx) + sn * (*dy);
  (*dy) = -sn * (*dx) + cs * (*dy);
  (*dx) = temp;
}

void Update( vec_t *x, int k, prec_t **h, vec_t *s, vec_t **v ) {
  static vec_t *y=NULL, *tmp=NULL;
  static int y_elts=-1, tmp_elts=-1;
  int i, j;

  /* try to avoid allocating and deallocating a lot */
  if ( y_elts != s->nelts ) {
    if ( y != NULL ) vec_free( y );
    y = NULL;
  }
  if ( tmp_elts != x->nelts ) {
    if ( tmp != NULL ) vec_free( tmp );
    tmp = NULL;
  }
  if ( y == NULL ) {
    y = vec_create( s->nelts );
    y_elts = s->nelts;
  }
  if ( tmp == NULL ) {
    tmp = vec_create( x->nelts );
    tmp_elts = x->nelts;
  }

  vec_copy( s, y );

  // Backsolve:  
  for ( i = k; i >= 0; i-- ) {
    (y->elts[i]) /= h[i][i];
    for ( j = i - 1; j >= 0; j-- )
      (y->elts[j]) -= h[j][i] * y->elts[i];
  }

  for ( j = 0; j <= k; j++ ) {
    const_mult_vec( v[j], y->elts[j], tmp );
    vec_add( x, tmp, x );
  }


}

int gmres( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
	   int max_iter, prec_t tol, prec_t *error, int *iters,
	   int m ) {
  prec_t resid, beta;
  int i, j, k;
  vec_t **v;
  vec_t *s, *cs, *sn, *w, *tmp;
  prec_t **H;

  j = 1;

  tmp = new_vec_from_matrix( A );

  matrix_vec_mult( A, x, tmp );  /* tmp = A*x */
  vec_sub( b, tmp, r );          /* r = b - tmp */
  beta = vec_two_norm( r );
  
/*   if ( normb == 0.0 ) { */
/*     normb = 1; */
/*   } */
/*   normb = vec_two_norm( b ); */
  /* XXX basic twonorm XXX */
  /*  resid = beta / normb; */

/*   resid = beta; */
  resid = vec_max_norm( r );
  if ( resid < tol ) {
    *error = resid;
    *iters = 0;
    vec_free( tmp );
    return 0;
  }

  H = alloc_twod_prect( m+1, m+1 );
  s = vec_create( m+1 );
  cs = vec_create( m+1 );
  sn = vec_create( m+1 );
  w = new_vec_from_matrix( A );

  v = (vec_t **)malloc( sizeof(vec_t*) * (m+1) );
  DIE_IF_NULL( v );
  for ( i=0; i<m+1; i++ ) {
    v[i] = vec_create( r->nelts );
  }

  while ( j <= max_iter ) {
    const_mult_vec( r, 1.0/beta, v[0] );
    const_vec( s, 0 );
    s->elts[0] = beta;
    
/*     if ( verbose && j % 3 == 0 ) { */
/*       printf( "  e = %.5f\n", resid ); */
/*     } */

    for (i = 0; i < m && j <= max_iter; i++, j++) {
      matrix_vec_mult( A, v[i], w );
      for (k = 0; k <= i; k++) {
        H[k][i] = vec_dot( w, v[k] );
	const_mult_vec( v[k], H[k][i], tmp );
	vec_sub( w, v[k], w );
      }
      H[i+1][i] = vec_two_norm( w );
      const_mult_vec( w, 1.0/H[i+1][i], v[i+1] );

      for (k = 0; k < i; k++) {
        ApplyPlaneRotation( &(H[k][i]), &(H[k+1][i]),
			    cs->elts[k], sn->elts[k] );
      }
      
      GeneratePlaneRotation( H[i][i], H[i+1][i],
			     &(cs->elts[i]), &(sn->elts[i]) );
      ApplyPlaneRotation( &(H[i][i]), &H[i+1][i], cs->elts[i], sn->elts[i] );
      ApplyPlaneRotation( &(s->elts[i]), &(s->elts[i+1]),
			  cs->elts[i], sn->elts[i] );
      
      /* XXX basic twonorm XXX */
      /* resid = fabs( s->elts[i+1] / normb );*/
      /* resid = fabs( s->elts[i+1] ); */
      resid = vec_max_norm( r );
      if ( resid < tol ) {
        Update( x, i, H, s, v );
	goto cleanup;
      }
    }

    Update( x, m - 1, H, s, v );
    matrix_vec_mult( A, x, tmp );  /* tmp = A*x */
    vec_sub( b, tmp, r );          /* r = b - tmp */

    beta = vec_two_norm( r );
    /* XXX basic twonorm XXX */
    /*    resid = beta / normb; */
/*     if ( verbose && j % 10 == 0 ) { */
/*       printf( "  e = %.5f\n", resid ); */
/*     } */
/*     resid = beta; */
    resid = vec_max_norm( r );
    if ( resid < tol ) {
      goto cleanup;
    }
  }
  
  /* CLEAN UP */
 cleanup:

  *error = resid;
  *iters = j;
  dealloc_twod_prect( H, m+1 );
  for ( i=0; i<m+1; i++ ) {
    vec_free( v[i] );
  }
  free( v );
  vec_free( s );
  vec_free( cs );
  vec_free( sn );
  vec_free( w );
  vec_free( tmp );

  return 1;
}



//*****************************************************************
// Iterative template routine -- CG
//
// CG solves the symmetric positive definite linear
// system Ax=b using the Conjugate Gradient method.
//
// CG follows the algorithm described on p. 15 in the 
// SIAM Templates book.
//
// The return value indicates convergence within max_iter (input)
// iterations (0), or no convergence within max_iter iterations (1).
//
// Upon successful return, output arguments have the following values:
//  
//        x  --  approximate solution to Ax = b
// max_iter  --  the number of iterations performed before the
//               tolerance was reached
//      tol  --  the residual after the final iteration
//  
//*****************************************************************

int cg( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
	int max_iter, prec_t tol, prec_t *error, int *iters ) {
  prec_t resid;
  vec_t *p, *q, *tmp;
  prec_t alpha, beta, rho, rho_1;
  int i;

  /* silence a compiler warning */
  rho_1 = 0;

  p = new_vec_from_matrix( A );
  q = new_vec_from_matrix( A );
  tmp = new_vec_from_matrix( A );

  matrix_vec_mult( A, x, tmp );  /* tmp = A*x */
  vec_sub( b, tmp, r );          /* r = b - tmp */
  //  resid = vec_two_norm( r );
  resid = vec_max_norm( r );

  if ( resid <= tol ) {
    *error = resid;
    *iters = 0;
    vec_free( p );
    vec_free( q );
    vec_free( tmp );
    return 0;
  }

  for ( i=1; i <= max_iter; i++ ) {
    //    z = M.solve(r);
    //    rho = vec_dot( r, z );
    rho = vec_dot( r, r );
    
    if (i == 1) {
      vec_copy( r, p );
    } else {
      beta = rho / rho_1;
      // p = r + beta * p;
      const_mult_vec( p, beta, p );
      vec_add( p, r, p );
    }
    
    // q = A*p;
    matrix_vec_mult( A, p, q );

    alpha = rho / vec_dot( p, q );
    
    // x += alpha * p;
    // r -= alpha * q;
    const_mult_vec( p, alpha, tmp );
    vec_add( x, tmp, x );
    const_mult_vec( q, alpha, tmp );
    vec_sub( r, tmp, r );

    //    resid = vec_two_norm( r );
    resid = vec_max_norm( r );

    if ( resid <= tol ) {
      *error = resid;
      *iters = i;
      vec_free( p );
      vec_free( q );
      vec_free( tmp );
      return 0;     
    }

    rho_1 = rho;
  }
  
  *error = resid;
  *iters = i;
  vec_free( p );
  vec_free( q );
  vec_free( tmp );
  return 1;
}


//*****************************************************************
// Iterative template routine -- CGS
//
// CGS solves the unsymmetric linear system Ax = b 
// using the Conjugate Gradient Squared method
//
// CGS follows the algorithm described on p. 26 of the 
// SIAM Templates book.
//
// The return value indicates convergence within max_iter (input)
// iterations (0), or no convergence within max_iter iterations (1).
//
// Upon successful return, output arguments have the following values:
//  
//        x  --  approximate solution to Ax = b
// max_iter  --  the number of iterations performed before the
//               tolerance was reached
//      tol  --  the residual after the final iteration
//  
//*****************************************************************

int cgs( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
	 int max_iter, prec_t tol, prec_t *error, int *iters ) {
  prec_t resid;
  prec_t rho_1, rho_2, alpha, beta;
  vec_t *p, *q, *qhat, *vhat, *u, *uhat, *rtilde, *tmp;
  int i;

  // silence a compiler warning
  rho_2 = 0;

  p = new_vec_from_matrix( A );
  q = new_vec_from_matrix( A );
  qhat = new_vec_from_matrix( A );
  vhat = new_vec_from_matrix( A );
  u = new_vec_from_matrix( A );
  uhat = new_vec_from_matrix( A );
  rtilde = new_vec_from_matrix( A );
  tmp = new_vec_from_matrix( A );

  matrix_vec_mult( A, x, tmp );  /* tmp = A*x */
  vec_sub( b, tmp, r );          /* r = b - tmp */

  //  rtilde = r;
  vec_copy( r, rtilde );

  //  resid = vec_two_norm( r );
  resid = vec_max_norm( r );
  if ( resid <= tol ) {
    vec_free( p );
    vec_free( q );
    vec_free( qhat );
    vec_free( vhat );
    vec_free( u );
    vec_free( uhat );
    vec_free( rtilde );
    vec_free( tmp );
    *error = resid;
    *iters = 0;
    return 0;
  }

  for ( i=1; i <= max_iter; i++ ) {

    rho_1 = vec_dot( rtilde, r );

    if ( rho_1 == 0 ) {
      vec_free( p );
      vec_free( q );
      vec_free( qhat );
      vec_free( vhat );
      vec_free( u );
      vec_free( uhat );
      vec_free( rtilde );
      vec_free( tmp );
      //      *error = vec_two_norm( r );
      *error = vec_max_norm( r );
      *iters = i;
      return 2;
    }

    if (i == 1) {
      vec_copy( r, u );
      vec_copy( u, p );
    } else {
      beta = rho_1 / rho_2;

      //      u = r + beta * q;
      const_mult_vec( q, beta, tmp );
      vec_add( r, tmp, u );

      //      p = u + beta * (q + beta * p);
      const_mult_vec( p, beta, tmp );
      vec_add( tmp, q, tmp );
      const_mult_vec( tmp, beta, tmp );
      vec_add( u, tmp, p );

    }

    //    phat = M.solve(p);
    //    vhat = A*phat;
    //    alpha(0) = rho_1(0) / dot(rtilde, vhat);
    matrix_vec_mult( A, p, vhat );
    alpha = rho_1 / vec_dot( rtilde, vhat );

    //    q = u - alpha * vhat;
    const_mult_vec( vhat, alpha, tmp );
    vec_sub( u, tmp, q );

    //    uhat = M.solve(u + q);
    //    x += alpha * uhat;
    vec_add( u, q, uhat );
    const_mult_vec( uhat, alpha, tmp );
    vec_add( x, tmp, x );
    
    //    qhat = A * uhat;
    //    r -= alpha * qhat;
    matrix_vec_mult( A, uhat, tmp );
    const_mult_vec( tmp, alpha, tmp );
    vec_sub( r, tmp, r );

    rho_2 = rho_1;

    //    resid = vec_two_norm( r );
    resid = vec_max_norm( r );
    if ( resid < tol ) {
      vec_free( p );
      vec_free( q );
      vec_free( qhat );
      vec_free( vhat );
      vec_free( u );
      vec_free( uhat );
      vec_free( rtilde );
      vec_free( tmp );
      *error = resid;
      *iters = i;
      return 0;
    }
  }

  vec_free( p );
  vec_free( q );
  vec_free( qhat );
  vec_free( vhat );
  vec_free( u );
  vec_free( uhat );
  vec_free( rtilde );
  vec_free( tmp );
  *error = resid;
  *iters = i;
  return 1;
}

int cgnr( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
	  int max_iter, prec_t tol, prec_t *error, int *iters ) {
  int i;
  vec_t *p, *q, *w, *z, *tmp_r, *tmp_c;
  prec_t alpha, beta, wnorm, znorm_i, znorm_ipo, old_error;
/*   char save_fn[256]; */

  DIE_IF_FALSE( A->ncols == x->nelts );
  DIE_IF_FALSE( A->nrows == b->nelts );
  DIE_IF_FALSE( A->nrows == r->nelts );

  p = new_vec_from_matrix( A );
  q = new_vec_from_matrix( A );
  w = new_vec_from_matrix_transpose( A );
  z = new_vec_from_matrix( A );

  /* tmp_c is a vector that has as many elements as A has columns */
  /* tmp_r is a vector that has as many elements as A has rows */

  tmp_c = new_vec_from_matrix( A );
  tmp_r = new_vec_from_matrix_transpose( A );

  /* r_0 = b-Ax_0 */
  matrix_vec_mult( A, x, tmp_r );
  vec_sub( b, tmp_r, r );

/*   printf( "INITIAL R\n" ); */
/*   vec_show( r ); */

  /* z_0 = A^Tr_0 */
  matrix_transpose_vec_mult( A, r, z );

/*   printf( "Z\n" ); */
/*   vec_show( z ); */

  /* p_0 = z_0 */
  vec_copy( z, p );

  /* znorm_i = \|z\|^2 */
  znorm_i = vec_dot( z, z );

  old_error = -1;

  for ( i=1; i <= max_iter; i++ ) {

/*     sprintf( save_fn, "/tmp/GVMOVIE/params-%09d", i ); */
/*     vec_save( save_fn, x ); */
/*     printf( "P\n" ); */
/*     vec_show( p ); */

    /* w_i = Ap_i */
    matrix_vec_mult( A, p, w );

/*     printf( "W\n" ); */
/*     vec_show( w ); */

    /* alpha_i = \|z_i\|^2 / \|w_i\|_2^2 */
    wnorm = vec_dot( w, w );
    alpha = znorm_i / wnorm;

/*     printf( "wnorm: %.2f, alpha: %.2f\n", wnorm, alpha ); */

    /* x_{i+1} = x_i + alpha_i p_i */
    const_mult_vec( p, alpha, tmp_c );
    vec_add( x, tmp_c, x );

    const_mult_vec( w, alpha, tmp_r );
/*     printf( "R\n" ); */
/*     vec_show( r ); */
/*     printf( "tmp_r\n" ); */
/*     vec_show( tmp_r ); */
    vec_sub( r, tmp_r, r );

    matrix_transpose_vec_mult( A, r, z );    

    znorm_ipo = vec_dot( z, z );
    beta = znorm_ipo / znorm_i;
    znorm_i = znorm_ipo;

    const_mult_vec( p, beta, tmp_c );
    vec_add( z, tmp_c, p );

/*     vec_show( r ); */
//    *error = vec_two_norm( r );
    *error = vec_max_norm( r );

    if ( *error <= tol ) {
      break;
    }

/*     fprintf( stderr, "." ); */
/*     fprintf( stderr, "%.9f %.9f %.9f\n", */
/* 	     old_error, *error, old_error - *error ); */
/*     fprintf( stdout, "%.9f\n", *error ); */
/*     if ( old_error != -1 && old_error - *error < tol*tol ) { */
/*       break; */
/*     } */
    old_error = *error;
  }

  *iters = i;
  vec_free( p );
  vec_free( q );
  vec_free( w );
  vec_free( z );
  vec_free( tmp_c );
  vec_free( tmp_r );
  return 1;
}


int cgne( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
	  int max_iter, prec_t tol, prec_t *error, int *iters ) {
  int i;
  vec_t *p, *tmp_r, *tmp_c;
  prec_t alpha, beta, old_error;
  prec_t r_inorm, r_iponorm;

  DIE_IF_FALSE( A->ncols == x->nelts );
  DIE_IF_FALSE( A->nrows == b->nelts );
  DIE_IF_FALSE( A->nrows == r->nelts );

  p = new_vec_from_matrix( A );

  /* tmp_c is a vector that has as many elements as A has columns */
  /* tmp_r is a vector that has as many elements as A has rows */

  tmp_c = new_vec_from_matrix( A );
  tmp_r = new_vec_from_matrix_transpose( A );

  /* r_0 = b-Ax_0 */
  matrix_vec_mult( A, x, tmp_r );
  vec_sub( b, tmp_r, r );

  /* p_0 = A^Tr_0 */
  matrix_transpose_vec_mult( A, r, p );

  r_inorm = vec_dot( r, r );

  old_error = -1;

  for ( i=1; i <= max_iter; i++ ) {

    alpha = r_inorm / vec_dot( p, p );

    const_mult_vec( p, alpha, tmp_c );
    vec_add( x, tmp_c, x );

    matrix_vec_mult( A, p, tmp_r );
    const_mult_vec( tmp_r, alpha, tmp_r );
    vec_sub( r, tmp_r, r );

    r_iponorm = vec_dot( r, r );
    beta = r_iponorm / r_inorm;
    r_inorm = r_iponorm;

    const_mult_vec( p, beta, p );
    matrix_transpose_vec_mult( A, r, tmp_c );
    vec_add( p, tmp_c, p );

    //    *error = vec_two_norm( r );
    *error = vec_max_norm( r );

    if ( *error <= tol ) {
      break;
    }

/*     fprintf( stderr, "." ); */
/*     fprintf( stderr, "%.9f %.9f %.9f\n", */
/* 	     old_error, *error, old_error - *error ); */
/*     fprintf( stdout, "%.9f\n", *error ); */
/*     if ( old_error != -1 && old_error - *error < tol*tol ) { */
/*       break; */
/*     } */
    old_error = *error;
  }

  *iters = i;
  vec_free( p );
  vec_free( tmp_c );
  vec_free( tmp_r );
  return 1;
}


/*
 * ----------------------------------------------------------------------------
 */

/*
   Solve a system of n equations in n unknowns using Gaussian Elimination
   Solve an equation in matrix form Ax = b
   The 2D array a is the matrix A with an additional column b.
   This is often written (A:b)

   A0,0    A1,0    A2,0    ....  An-1,0     b0
   A0,1    A1,1    A2,1    ....  An-1,1     b1
   A0,2    A1,2    A2,2    ....  An-1,2     b2
   :       :       :             :          :
   :       :       :             :          :
   A0,n-1  A1,n-1  A2,n-1  ....  An-1,n-1   bn-1

   The result is returned in x, otherwise the function returns FALSE
   if the system of equations is singular.
*/

/*
   Got this code from
     http://astronomy.swin.edu.au/~pbourke/analysis/gausselim/
*/

int direct_gaussian_solver( matrix_t *A, vec_t *x, vec_t *b, vec_t *r,
			    int max_it, prec_t tol,
			    prec_t *error, int *iters ) {
  int i, j, k, maxrow;
  int rc_cnt;
  prec_t tmp, **a;

  DIE_IF_FALSE( A->nrows == A->ncols );

  rc_cnt = A->nrows;

  /* allocate an augmented dense matrix */
  a = alloc_twod_prect( rc_cnt+1, rc_cnt );
  for ( i=0; i<rc_cnt+1; i++ ) {
    memset( a[i], 0, sizeof(prec_t)*rc_cnt );
  }

  /* initialize the dense matrix with the sparse entries */
  for ( i=0; i<rc_cnt; i++ ) {
    for ( j=0; j<A->rows[i].colcnt; j++ ) {
      k = A->rows[i].entries[j].col;
      a[k][i] = A->rows[i].entries[j].entry;
    }
    a[rc_cnt][i] = b->elts[i];
  }


  /* NOTATION: a[column][row] */

  for ( i=0; i<rc_cnt; i++ ) {

    /* Find the row with the largest first value */
    maxrow = i;
    for ( j=i+1; j<rc_cnt; j++ ) {
      if ( fabs(a[i][j]) > fabs(a[i][maxrow]) ) {
	maxrow = j;
      }
    }

    /* Swap the maxrow and ith row */
    for ( k=i; k<rc_cnt+1; k++) {
      tmp = a[k][i];
      a[k][i] = a[k][maxrow];
      a[k][maxrow] = tmp;
    }

    /* Singular matrix? */
    if ( fabs(a[i][i]) < EPS ) {
      fprintf( stderr, "SINGULAR MATRIX!\n" );
      return 0;
    }

    /* Eliminate the ith element of the jth row */
    for ( j=i+1; j<rc_cnt; j++ ) {
      for ( k=rc_cnt; k>=i; k-- ) {
	a[k][j] -= a[k][i] * a[i][j] / a[i][i];
      }
    }
  }

  /* Do the back substitution */
  for ( j=rc_cnt-1; j>=0; j-- ) {
    tmp = 0;
    for ( k=j+1; k<rc_cnt; k++ ) {
      tmp += a[k][j] * x->elts[k];
    }
    x->elts[j] = (a[rc_cnt][j] - tmp) / a[j][j];
  }

  dealloc_twod_prect( a, rc_cnt+1 );

  *iters = A->nrows;

  return 1;
}

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */
