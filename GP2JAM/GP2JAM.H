/*
** File   : gp2jam.h
** Author : tk
** Date   : 04/08/96
**
** $Header:   F:/TK/GP2/GP2JAM/VCS/GP2JAM.H__   1.2   29 Sep 1996 16:38:48   tk  $
**
** Convert GP2 .JAM file to .BMP and back again.
*/

#ifndef   gp2jam_h
#define   gp2jam_h

/*---------------------------------------------------------------------------
** Includes
*/

/*---------------------------------------------------------------------------
** Defines and Macros
*/

#define NUM_COLOURS_IN_PALETTE     256

#define FALSE					0
#define TRUE					1

/*---------------------------------------------------------------------------
** Typedefs
*/

typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned int	uint;
typedef unsigned long	ulong;

typedef int			BOOL;
typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;
typedef unsigned int	UINT;
typedef long			LONG;

typedef struct tagRGBQUAD
{
	BYTE		rgbBlue;
	BYTE		rgbGreen;
	BYTE		rgbRed;
	BYTE		rgbReserved;
} RGBQUAD;

/*---------------------------------------------------------------------------
** Data
*/

/*---------------------------------------------------------------------------
** Functions
*/


#endif    /* gp2jam_h */
/*---------------------------------------------------------------------------
** End of File
*/
