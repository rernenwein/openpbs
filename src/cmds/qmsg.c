/*
 * Copyright (C) 1994-2016 Altair Engineering, Inc.
 * For more information, contact Altair at www.altair.com.
 *  
 * This file is part of the PBS Professional ("PBS Pro") software.
 * 
 * Open Source License Information:
 *  
 * PBS Pro is free software. You can redistribute it and/or modify it under the
 * terms of the GNU Affero General Public License as published by the Free 
 * Software Foundation, either version 3 of the License, or (at your option) any 
 * later version.
 *  
 * PBS Pro is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.
 *  
 * You should have received a copy of the GNU Affero General Public License along 
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *  
 * Commercial License Information: 
 * 
 * The PBS Pro software is licensed under the terms of the GNU Affero General 
 * Public License agreement ("AGPL"), except where a separate commercial license 
 * agreement for PBS Pro version 14 or later has been executed in writing with Altair.
 *  
 * Altair’s dual-license business model allows companies, individuals, and 
 * organizations to create proprietary derivative works of PBS Pro and distribute 
 * them - whether embedded or bundled with other software - under a commercial 
 * license agreement.
 * 
 * Use of Altair’s trademarks, including but not limited to "PBS™", 
 * "PBS Professional®", and "PBS Pro™" and Altair’s logos is subject to Altair's 
 * trademark licensing policies.
 *
 */
/**
 * @file	qmsg.c
 * @brief
 * 	qmsg - (PBS) send message to batch job
 *
 * @author	Terry Heidelberg
 * 			Livermore Computing
 *
 * @author	Bruce Kelly
 * 			National Energy Research Supercomputer Center
 *
 * @author	Lawrence Livermore National Laboratory
 * 			University of California
 */

#include "cmds.h"
#include "pbs_ifl.h"
#include <pbs_config.h>   /* the master config generated by configure */
#include <pbs_version.h>


int
main(int argc, char **argv, char **envp) /* qmsg */
{
	int c;
	int to_file;
	int errflg=0;
	int any_failed=0;

	char job_id[PBS_MAXCLTJOBID];       /* from the command line */

	char job_id_out[PBS_MAXCLTJOBID];
	char server_out[MAXSERVERNAME];
	char rmt_server[MAXSERVERNAME];

#define MAX_MSG_STRING_LEN 256
	char msg_string[MAX_MSG_STRING_LEN+1];

#define GETOPT_ARGS "EO"

	/*test for real deal or just version and exit*/

	execution_mode(argc, argv);

#ifdef WIN32
	winsock_init();
#endif

	msg_string[0]='\0';
	to_file = 0;

	while ((c = getopt(argc, argv, GETOPT_ARGS)) != EOF)
		switch (c) {
			case 'E':
				to_file |= MSG_ERR;
				break;
			case 'O':
				to_file |= MSG_OUT;
				break;
			default :
				errflg++;
		}
	if (to_file == 0) to_file = MSG_ERR;	/* default */

		if (errflg || ((optind+1) >= argc)) {
			static char usage[]=
				"usage: qmsg [-O] [-E] msg_string job_identifier...\n";
			static char usag2[]=
				"       qmsg --version\n";
			fprintf(stderr, usage);
			fprintf(stderr, usag2);
			exit(2);
		}

	strcpy(msg_string, argv[optind]);

	/*perform needed security library initializations (including none)*/

	if (CS_client_init() != CS_SUCCESS) {
		fprintf(stderr, "qmsg: unable to initialize security library.\n");
		exit(2);
	}

	for (optind++; optind < argc; optind++) {
		int connect;
		int stat=0;
		int located = FALSE;

		strcpy(job_id, argv[optind]);
		if (get_server(job_id, job_id_out, server_out)) {
			fprintf(stderr, "qmsg: illegally formed job identifier: %s\n", job_id);
			any_failed = 1;
			continue;
		}
cnt:
		connect = cnt2server(server_out);
		if (connect <= 0) {
			fprintf(stderr, "qmsg: cannot connect to server %s (errno=%d)\n",
				pbs_server, pbs_errno);
			any_failed = pbs_errno;
			continue;
		}

		stat = pbs_msgjob(connect, job_id_out, to_file, msg_string, NULL);
		if (stat && (pbs_errno != PBSE_UNKJOBID)) {
			prt_job_err("qmsg", connect, job_id_out);
			any_failed = pbs_errno;
		} else if (stat && (pbs_errno == PBSE_UNKJOBID) && !located) {
			located = TRUE;
			if (locate_job(job_id_out, server_out, rmt_server)) {
				pbs_disconnect(connect);
				strcpy(server_out, rmt_server);
				goto cnt;
			}
			prt_job_err("qmsg", connect, job_id_out);
			any_failed = pbs_errno;
		}

		pbs_disconnect(connect);
	}

	/*cleanup security library initializations before exiting*/
	CS_close_app();

	exit(any_failed);
}