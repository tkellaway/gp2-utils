/*
** File   : jam.cpp
** Author : tk
** Date   : 04/08/96
**
** $Header:   F:/TK/GP2/GP2JAM/VCS/JAM.CP_   1.3   29 Sep 1996 16:38:46   tk  $
**
** JAM class.
*/

/*---------------------------------------------------------------------------
** Includes
*/

/*lint -elib(???) */
//#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*lint +elib(???) */
#include "gp2jam.h"
#include "jam.h"

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

#include "jamids.i"

/*---------------------------------------------------------------------------
** Functions
*/

JAM::JAM(
	void
) {
	memset(&m_Hdr, 0, sizeof(m_Hdr));
	memset(&m_Entry, 0, sizeof(m_Entry));
	m_pBuff		= NULL;
	m_pPalette	= NULL;
	m_pImage		= NULL;
	m_BuffSize	= 0;
	m_pJamEntry	= NULL;
}

JAM::~JAM(
	void
) {
	if (m_pBuff) {
		delete[] m_pBuff;
		m_pBuff 	= NULL;
	}
	m_pPalette	= NULL;
	m_pImage		= NULL;
	m_pJamEntry	= NULL;
}

BOOL
JAM::Load(
	const char *filename
) {
	FILE		*fp 		= NULL;
	long		file_size	= 0;
	uint		nPaletteSize;
	uint		n;

	if ((fp = fopen(filename, "rb")) == 0) {
		printf("Unable to open file '%s' for loading.\n", filename);
		return FALSE;
	}

	(void) fseek(fp, 0L, SEEK_END);
	file_size = ftell(fp);
	(void) fseek(fp, 0L, SEEK_SET);

	if ((ulong) file_size > 0x00040000UL || (ulong) file_size < sizeof(JAM_HDR) + sizeof(JAM_ENTRY)) {
		fclose(fp);
		printf("The file size of '%s' is not supported.\n", filename);
		return FALSE;
	}
	m_BuffSize = (uint) file_size;

	if (m_pBuff) {
		delete[] m_pBuff;
		m_pBuff = NULL;
	}

	m_pBuff = new uchar[m_BuffSize];
	if (m_pBuff == NULL) {
		fclose(fp);
		printf("Out of memory trying to allocate a palette for '%s'.\n", filename);
		return FALSE;
	}

	if (fread(m_pBuff, m_BuffSize, 1, fp) != 1) {
		fclose(fp);
		printf("Read #1 from '%s' failed.\n", filename);
		return FALSE;
	}
	fclose(fp);

	UnJam(m_pBuff, m_BuffSize);

	m_pJamEntry	= (JAM_ENTRY *) &m_pBuff[sizeof(JAM_HDR)];
	memcpy(&m_Hdr,   &m_pBuff[0], sizeof(m_Hdr));
	memcpy(&m_Entry, m_pJamEntry, sizeof(m_Entry));

	if (NumItems() == 0 || NumItems() > 0x0100) {
		printf("The file '%s' does not appear to be a known JAM file.\n", filename);
		return FALSE;
	}

	m_pPalette = &m_pBuff[sizeof(JAM_HDR) + (sizeof(JAM_ENTRY) * NumItems())];

	nPaletteSize = 0;
	for (n = 0; n < NumItems(); n++) {
		nPaletteSize += m_pJamEntry[n].palette_size_div4 * 4;
	}
	m_pImage = &m_pBuff[sizeof(JAM_HDR) + (sizeof(JAM_ENTRY) * NumItems()) + nPaletteSize];

	return TRUE;
}

BOOL
JAM::Save(
	const char *filename
) {
	FILE	*fp 	= NULL;
	BOOL	ok	= TRUE;

	if ((fp = fopen(filename, "rb+")) == 0) {
		printf("Unable to open file '%s' for saving.\n", filename);
		return FALSE;
	}

	UnJam(m_pBuff, m_BuffSize);
	if (fwrite(m_pBuff, m_BuffSize, 1, fp) != 1) {
		printf("Write to '%s' failed, disk full?\n", filename);
		ok = FALSE;
	}
	UnJam(m_pBuff, m_BuffSize);

	fclose(fp);

	return ok;
}

BOOL
JAM::IsJam(
	void
) {
	return (FindJam(&m_Hdr, &m_Entry) != NULL);
}

void
JAM::UnJam(
	void 	*ptr,
	uint		len
) {
	ulong	x;
	uint		n;
	uchar	*pc;
	ulong	*pl;

	x = 0xb082f164UL;
	n = len / 4;
	x |= 1;
	pl = (ulong *) ptr;

	while (n--) {
		*pl ^= x;
		++pl;
		x = x + (x * 4);
	}

	n = len & (4 - 1);
	if (n != 0) {
		pc = (uchar *) pl;
		*pc ^= x & 0xff;
		++pc;
		x >>= 8;
	}
}

const char *
JAM::JamDescription(
	const JAM_HDR		*pHdr,
	const JAM_ENTRY     *pEntry
) {
	const JAM_ID_TABLE	*pJamIdTable	= NULL;
	const char		*ptr			= NULL;

	pJamIdTable = FindJam(pHdr, pEntry);
	if (pJamIdTable) {
		if (pJamIdTable->duplicate) {
			return "????????";
		}
		else {
			return pJamIdTable->szFilename;
		}
	}

	return ptr;
}

const JAM_ID_TABLE *
JAM::FindJam(
	const JAM_HDR		*pHdr,
	const JAM_ENTRY     *pEntry
) {
	const JAM_ID_TABLE	*pJamIdTable	= JamIdTable;

	for (; pJamIdTable->szFilename != NULL; pJamIdTable++) {
		if (	memcmp(	pHdr,
					&pJamIdTable->hdr_bytes[0],
					sizeof(*pHdr)
				) == 0 &&
			memcmp(	pEntry,
					&pJamIdTable->hdr_bytes[sizeof(*pHdr)],
					sizeof(pJamIdTable->hdr_bytes) - sizeof(*pHdr)
				) == 0
		) {
			return pJamIdTable;
		}
	}

	return NULL;
}

/*---------------------------------------------------------------------------
** End of File
*/
