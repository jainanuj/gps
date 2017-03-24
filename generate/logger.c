
#include "logger.h"

/* where we log information */
FILE *logfile = NULL;

/*
 * ============================================================================
 */

void wlog( int level, const char *format, ... ) {
  va_list ap;

  va_start( ap, format );

  if ( logfile == NULL ) {
    fprintf( stderr, "LOG FILE NOT YET OPEN!\n" );
    vfprintf( stderr, format, ap );    

  } else {
    vfprintf( logfile, format, ap );
  }

  va_end( ap );

#ifdef LOGGER_ALWAYS_FLUSH
  fflush( logfile );
#endif

}

/*
 * ----------------------------------------------------------------------------
 */

int open_logfile_mpi( char *proc_name, int my_proc_num, int num_procs ) {
  char fn[1024];
  sprintf( fn, "log-%s-%d_%d", proc_name, my_proc_num, num_procs );
  return open_logfile( fn );
}

int open_logfile_default( void ) {
  return open_logfile( "log" );
}

int open_logfile( char *fn ) {
  logfile = fopen( fn, "wb" );
  if ( logfile == NULL ) {
    fprintf( stderr, "Couldn't open %s for writing!\n", fn );
    return 0;
  }
  return 1;
}

int open_logfile_stdout( void ) {
  logfile = stdout;
  return 1;
}

/*
 * ----------------------------------------------------------------------------
 */

void wlog_flush( void ) {
  fflush( logfile );
}

/*
 * ----------------------------------------------------------------------------
 */
