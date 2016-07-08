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
 * @file	enc_JobObit.c
 * @brief
 * encode_DIS_JobObit() - encode a Job Obituary Batch Request (Notice)
 *
 *	This request is used by the server ONLY; its input is a server
 *	batch request structure.
 *
 * @par Data items are:
 * 			string		job id
 *			signed int	status
 *			list of		svrattrl
 */

#include <pbs_config.h>   /* the master config generated by configure */

#include <sys/types.h>
#include "libpbs.h"
#include "list_link.h"
#include "server_limits.h"
#include "attribute.h"
#include "credential.h"
#include "batch_request.h"
#include "dis.h"

/**
 * @brief
 *      - encode a Job Obituary Batch Request (Notice)
 *
 * @par Functionality
 *              This request is used by the server ONLY.
 *          	its input is a server batch request structure.
 *
 * @par   Data items are:\n
 *              string          job id\n
 *              signed int      status\n
 *              list of         svrattrl
 *
 * @param[in] sock - socket descriptor
 * @param[out] preq - pointer to batch_request structure
 *
 * @return      int
 * @retval      DIS_SUCCESS(0)  success
 * @retval      error code      error
 *
 */
int
encode_DIS_JobObit(int sock, struct batch_request *preq)
{
	int   rc;
	struct svrattrl *psvrl;

	psvrl = (struct svrattrl *)GET_NEXT(preq->rq_ind.rq_jobobit.rq_attr);

	if ((rc = diswst(sock, preq->rq_ind.rq_jobobit.rq_jid) != 0) ||
		(rc = diswsi(sock, preq->rq_ind.rq_jobobit.rq_status) != 0) ||
		(rc = encode_DIS_svrattrl(sock, psvrl) != 0))
			return rc;

	return 0;
}