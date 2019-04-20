/*
** File   : dump.h
** Author : TK
** Date   : 18/04/96
**
** $Header:   F:/TK/GP2/GP2JAM/VCS/DUMP.H__   1.2   29 Sep 1996 16:38:46   tk  $
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

#ifndef   dump_h
#define   dump_h

/*---------------------------------------------------------------------------
** Includes
*/

/*---------------------------------------------------------------------------
** Defines and Macros
*/

/*---------------------------------------------------------------------------
** Typedefs
*/

typedef struct {
     char           *table;
     unsigned short *size;
     char           *filename;
     char           *tp_name;
} TP_DUMP_TABLE;

/*---------------------------------------------------------------------------
** Data
*/

/*---------------------------------------------------------------------------
** Functions
*/

void __tp_dump(void);

#endif    /* dump_h */
/*---------------------------------------------------------------------------
** End of File
*/

