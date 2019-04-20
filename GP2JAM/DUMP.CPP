/*
** File   : dump.c
** Author : TK
** Date   : 18/04/96
**
** $Header:   F:/TK/GP2/GP2JAM/VCS/DUMP.CP_   1.2   29 Sep 1996 16:38:46   tk  $
**
** PC debug code coverage support.
*/

/*****************************************************************************
**                                                                          **
** This program is the confidential and proprietary product of Application  **
** Solutions Limited. Any unauthorised use, reproduction or transfer of     **
** this program is strictly prohibited.                                     **
** Copyright 1996 Application Solutions Limited. (Subject to limited        **
** distribution and restricted disclosure only.) All rights reserved.       **
**                                                                          **
*****************************************************************************/

/*---------------------------------------------------------------------------
** Includes
*/

#ifdef _COVER_

/*lint -elib(???) */
#include <stdio.h>
#include <time.h>
/*lint +elib(???) */
#include "dump.h"

#include "dump1.i"
#include "dump2.i"

#endif

/*---------------------------------------------------------------------------
** Defines and Macros
*/

/*---------------------------------------------------------------------------
** Typedefs
*/

/*---------------------------------------------------------------------------
** Local function prototypes
*/

/*---------------------------------------------------------------------------
** Data
*/

#ifdef _COVER_
/*lint -esym(528,what_cover) Ignore unreferenced */
static const char what_cover[] = "@(#)Code coverage enabled (_COVER_)";
#endif

/*---------------------------------------------------------------------------
** Functions
*/

/*codechk -w */
/*---------------------------------------------------------------------------
** Purpose:    Dump trace point data.
**
** Returns:    Nothing.
**
** Notes  :
*/
void
__tp_dump(
     void
) {
#ifdef _COVER_
	FILE           *fp = NULL;
	unsigned short i;
	unsigned short j;
	time_t		t;

	(void) fprintf(stderr, "Dumping code coverage trace points to DUMP.TP\n");
	(void) time(&t);
	fp = fopen("dump.tp", "a+");
	if (fp) {
		for (i = 0; i < sizeof(tp_dump_table) / sizeof(tp_dump_table[0]); i++) {
			for (j = 0; j < *tp_dump_table[i].size; j++) {
				(void) fprintf(fp, "%s %u %u %lu\n",
						tp_dump_table[i].filename,
						j,
					    	tp_dump_table[i].table[j],
						(j == 0) ? (unsigned long) t : 0UL
					);
			}
		}
		(void) fprintf(stderr, "Dump completed\n");
		(void) fclose(fp);
	}
#endif
	return;
}

/*codechk +w */
/*---------------------------------------------------------------------------
** End of File
*/
