#include "license_pbs.h" /* See here for the software license */
/*
 *
 * qdel - (PBS) delete batch job
 *
 * Authors:
 *      Terry Heidelberg
 *      Livermore Computing
 *
 *      Bruce Kelly
 *      National Energy Research Supercomputer Center
 *
 *      Lawrence Livermore National Laboratory
 *      University of California
 */

#include "cmds.h"
#include "net_cache.h"
#include <pbs_config.h>   /* the master config generated by configure */
#include "../lib/Libifl/lib_ifl.h"



bool is_array(

  char *job_id)

  {
  char *bracket_ptr;

  if ((bracket_ptr = strchr(job_id,'[')) != NULL)
    {
    /* Make sure the next character is ']' */
    if (*(++bracket_ptr) == ']')
      {
      return(true);
      }
    }
    
  return(false);
  } /* END is_array() */



/* qdel */

int main(

  int    argc,
  char **argv)

  {
  int c;
  int errflg = 0;
  int any_failed = 0;
  int purge_completed = FALSE;
  int located = FALSE;
  char *pc;
  bool  dash_t = false;

  char job_id[PBS_MAXCLTJOBID]; /* from the command line */

  char job_id_out[PBS_MAXCLTJOBID];
  char server_out[MAXSERVERNAME] = "";
  char rmt_server[MAXSERVERNAME] = "";

  char extend[1024];

#define GETOPT_ARGS "acm:pW:t:"

  extend[0] = '\0';

  while ((c = getopt(argc, argv, GETOPT_ARGS)) != EOF)
    {
    switch (c)
      {

      case 'a': /* Async job deletion */

        if (extend[0] != '\0')
          {
          errflg++;

          break;
          }

        snprintf(extend, sizeof(extend), "%s", DELASYNC);

        break;

      case 'c':

        if (extend[0] != '\0')
          {
          errflg++;

          break;
          }

        snprintf(extend,sizeof(extend),"%s%ld",PURGECOMP,(long)(time(NULL)));
        purge_completed = TRUE;

        break;

      case 'm':

        /* add delete message */

        if (extend[0] != '\0')
          {
          /* extension option already specified */

          errflg++;

          break;
          }

        snprintf(extend, sizeof(extend), "%s", optarg);

        break;

      case 'p':

        if (extend[0] != '\0')
          {
          errflg++;

          break;
          }

        snprintf(extend, sizeof(extend), "%s1", DELPURGE);

        break;

      case 't':

        dash_t = true;

        if (extend[0] != '\0')
          {
          errflg++;

          break;
          }
        
        pc = optarg;

        if (strlen(pc) == 0)
          {
          fprintf(stderr, "qdel: illegal -t value (array range cannot be zero length)\n");

          errflg++;

          break;
          }

        snprintf(extend,sizeof(extend),"%s%s",
          ARRAY_RANGE,
          pc);

        break;

      case 'W':

        if (extend[0] != '\0')
          {
          errflg++;

          break;
          }

        pc = optarg;

        if (strlen(pc) == 0)
          {
          fprintf(stderr, "qdel: illegal -W value\n");

          errflg++;

          break;
          }

        while (*pc != '\0')
          {
          if (!isdigit(*pc))
            {
            fprintf(stderr, "qdel: illegal -W value\n");

            errflg++;

            break;
            }

          pc++;
          }

        snprintf(extend, sizeof(extend), "%s%s", DELDELAY, optarg);

        break;

      default:
    	
        errflg++;

        break;
      }
    }    /* END while (c) */

  if (purge_completed)
    {
    snprintf(server_out, sizeof(server_out), "%s", pbs_default());
    goto cnt;
    }
  
  if ((errflg != 0) || (optind >= argc))
    {
    static char usage[] = "usage: qdel [{ -a | -c | -p | -t | -W delay | -m message}] [<JOBID>[<JOBID>]|'all'|'ALL']...\n";

    fprintf(stderr, "%s", usage);

    fprintf(stderr, "       -a -c, -m, -p, -t, and -W are mutually exclusive\n");

    exit(2);
    }

  for (;optind < argc;optind++)
    {
    int connect;
    int stat;

    /* check to see if user specified 'all' to delete all jobs */

   snprintf(job_id, sizeof(job_id), "%s", argv[optind]);
   
   if ((dash_t == true) && 
       is_array(job_id) == false)
     {
     fprintf(stderr, "qdel: Error: job id '%s' isn't a job array but -t was specified.\n",
       job_id);

     any_failed = 1;

     exit(any_failed);
     }
   
   if (get_server(job_id, job_id_out, sizeof(job_id_out), server_out, sizeof(server_out)))
     {
     fprintf(stderr, "qdel: illegally formed job identifier: %s\n",
             job_id);
      
     any_failed = 1;

     exit(any_failed);
     }

cnt:

    connect = cnt2server(server_out);

    if (connect <= 0)
      {
      any_failed = -1 * connect;

      if(server_out[0] != 0)
        fprintf(stderr, "qdel: cannot connect to server %s (errno=%d) %s\n",
              server_out,
              any_failed,
              pbs_strerror(any_failed));
      else
        fprintf(stderr, "qdel: cannot connect to server %s (errno=%d) %s\n",
              pbs_server,
              any_failed,
              pbs_strerror(any_failed));

      continue;
      }

    stat = pbs_deljob_err(connect, job_id_out, extend, &any_failed);

    if (stat &&
        (any_failed != PBSE_UNKJOBID))
      {
      if (!located)
        {
        located = TRUE;

        if (locate_job(job_id_out, server_out, rmt_server))
          {
          pbs_disconnect(connect);

          strcpy(server_out, rmt_server);

          goto cnt;
          }

        }
        
      prt_job_err((char *)"qdel", connect, job_id_out);
      }
    
    if (!located && any_failed != 0)
      {
      fprintf(stderr, "qdel: nonexistent job id: %s\n", job_id);
      // need to change to PBSE_ERROR.db
      }

    pbs_disconnect(connect);
    }

  exit(any_failed);
  }  /* END main() */


