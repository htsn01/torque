/*
*         OpenPBS (Portable Batch System) v2.3 Software License
*
* Copyright (c) 1999-2000 Veridian Information Solutions, Inc.
* All rights reserved.
*
* ---------------------------------------------------------------------------
* For a license to use or redistribute the OpenPBS software under conditions
* other than those described below, or to purchase support for this software,
* please contact Veridian Systems, PBS Products Department ("Licensor") at:
*
*    www.OpenPBS.org  +1 650 967-4675                  sales@OpenPBS.org
*                        877 902-4PBS (US toll-free)
* ---------------------------------------------------------------------------
*
* This license covers use of the OpenPBS v2.3 software (the "Software") at
* your site or location, and, for certain users, redistribution of the
* Software to other sites and locations.  Use and redistribution of
* OpenPBS v2.3 in source and binary forms, with or without modification,
* are permitted provided that all of the following conditions are met.
* After December 31, 2001, only conditions 3-6 must be met:
*
* 1. Commercial and/or non-commercial use of the Software is permitted
*    provided a current software registration is on file at www.OpenPBS.org.
*    If use of this software contributes to a publication, product, or
*    service, proper attribution must be given; see www.OpenPBS.org/credit.html
*
* 2. Redistribution in any form is only permitted for non-commercial,
*    non-profit purposes.  There can be no charge for the Software or any
*    software incorporating the Software.  Further, there can be no
*    expectation of revenue generated as a consequence of redistributing
*    the Software.
*
* 3. Any Redistribution of source code must retain the above copyright notice
*    and the acknowledgment contained in paragraph 6, this list of conditions
*    and the disclaimer contained in paragraph 7.
*
* 4. Any Redistribution in binary form must reproduce the above copyright
*    notice and the acknowledgment contained in paragraph 6, this list of
*    conditions and the disclaimer contained in paragraph 7 in the
*    documentation and/or other materials provided with the distribution.
*
* 5. Redistributions in any form must be accompanied by information on how to
*    obtain complete source code for the OpenPBS software and any
*    modifications and/or additions to the OpenPBS software.  The source code
*    must either be included in the distribution or be available for no more
*    than the cost of distribution plus a nominal fee, and all modifications
*    and additions to the Software must be freely redistributable by any party
*    (including Licensor) without restriction.
*
* 6. All advertising materials mentioning features or use of the Software must
*    display the following acknowledgment:
*
*     "This product includes software developed by NASA Ames Research Center,
*     Lawrence Livermore National Laboratory, and Veridian Information
*     Solutions, Inc.
*     Visit www.OpenPBS.org for OpenPBS software support,
*     products, and information."
*
* 7. DISCLAIMER OF WARRANTY
*
* THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT
* ARE EXPRESSLY DISCLAIMED.
*
* IN NO EVENT SHALL VERIDIAN CORPORATION, ITS AFFILIATED COMPANIES, OR THE
* U.S. GOVERNMENT OR ANY OF ITS AGENCIES BE LIABLE FOR ANY DIRECT OR INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* This license will be governed by the laws of the Commonwealth of Virginia,
* without reference to its choice of law rules.
*/






/* pbs_submit.c
 *
 * The Submit Job request.
*/

#include <pbs_config.h>   /* the master config generated by configure */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "libpbs.h"
#include "u_hash_map_structs.h"
#include "server_limits.h"

int pbs_submit_hash(

  int                socket,
  job_data_container *job_attr,
  job_data_container *res_attr,
  char              *script,
  char              *destination,
  char              *extend,  /* (optional) */
  char              **return_jobid,
  char              **msg)

  {
  int rc = PBSE_NONE;
/*  struct attropl *pal; */
  /* first be sure that the script is readable if specified ... */
  
  if ((socket < 0) || 
      (socket >= PBS_NET_MAX_CONNECTIONS))
    {
    return(PBSE_IVALREQ);
    }

  if ((script != NULL) && (*script != '\0'))
    {
    if (access(script, R_OK) != 0)
      {
      rc = PBSE_BADSCRIPT;

      return rc;
      }
    }

  /* initiate the queueing of the job */

/*  for (pal = attrib;pal != NULL;pal = pal->next) */
/*    pal->op = SET;  *//* force operator to SET */

  /* Queue job with null string for job id */

  rc = PBSD_QueueJob2_hash(socket, (char *)"", destination, job_attr, res_attr, extend, return_jobid, msg);

  if (rc != PBSE_NONE)
    {
    return rc;
    }

  /* send script across */

  if ((script != NULL) && (*script != '\0'))
    {
    if (PBSD_jscript2(socket, script, (*return_jobid==NULL)?NULL:(const char *)*return_jobid) != 0)
      {
      rc = PBSE_BADSCRIPT;
      if (connection[socket].ch_errtxt != NULL)
        {
        *msg = strdup(connection[socket].ch_errtxt);
        }

      return rc;
      }
    }

#ifdef AUTORUN_JOBS
  if ((rc = PBSD_commit2(socket, *return_jobid)) != 0)
    {
      if (connection[socket].ch_errtxt != NULL)
        {
        *msg = strdup(connection[socket].ch_errtxt);
        }
    return rc;
    }
#endif


  return(rc);

  }  /* END pbs_submit_hash() */

int pbs_submit_hash_ext(
  int                socket,
  void               *job_attr,
  void              *res_attr,
  char              *script,
  char              *destination,
  char              *extend,  /* (optional) */
  char              **return_jobid,
  char              **msg)
  {
  return pbs_submit_hash(socket,
      (job_data_container *)job_attr,
      (job_data_container *)res_attr,
      script,destination,extend,return_jobid,msg);
  }

/* END pbsD_submit.c */
