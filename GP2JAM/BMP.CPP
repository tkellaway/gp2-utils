/*
** File   : bmp.cpp
** Author : tk
** Date   : 04/08/96
**
** $Header:   F:/TK/GP2/GP2JAM/VCS/BMP.CP_   1.2   29 Sep 1996 16:38:46   tk  $
**
** Bitmap class.
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
#include "bmp.h"

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

/*---------------------------------------------------------------------------
** Functions
*/

BMP::BMP(
	void
) {
	memset(&m_Hdr, 0, sizeof(m_Hdr));
	memset(&m_InfoHdr, 0, sizeof(m_InfoHdr));
	memset(m_Palette, 0, sizeof(m_Palette));
	m_pImage		= NULL;
	m_BmpImageSize	= 0;
}

BMP::~BMP(
	void
) {
	if (m_pImage) {
		delete[] m_pImage;
		m_pImage = NULL;
	}
}

BOOL
BMP::Load(
	const char *filename
) {
	FILE	*fp;
	BOOL	ok = TRUE;

	memset(&m_Hdr, 0, sizeof(m_Hdr));
	memset(&m_InfoHdr, 0, sizeof(m_InfoHdr));
	memset(m_Palette, 0, sizeof(m_Palette));
	if (m_pImage) {
		delete m_pImage;
		m_pImage = NULL;
	}

	if ((fp = fopen(filename, "rb")) == 0) {
		printf("Unable to open file '%s' for loading.\n", filename);
		return FALSE;
	}

	if (ok && fread(&m_Hdr, sizeof(m_Hdr), 1, fp) != 1) {
		printf("Read #1 failed from file '%s'\n", filename);
		ok = FALSE;
	}

	if (ok && !IsBmp()) {
		printf("The file '%s' is not a bitmap file.\n", filename);
		ok = FALSE;
	}

	if (ok && fread(&m_InfoHdr, sizeof(m_InfoHdr), 1, fp) != 1) {
		printf("Read #2 failed from file '%s'\n", filename);
		ok = FALSE;
	}

	/*
	** Calculate some of the fields that may not be filled in!
	*/
	if (ok && m_InfoHdr.biSizeImage == 0) {
		long size;

		size = (m_InfoHdr.biWidth * m_InfoHdr.biBitCount) / 8;
		if (size % 4) {
			size += 4 - (size % 4);
		}
		size *= m_InfoHdr.biHeight;
		m_InfoHdr.biSizeImage = (ulong) size;
	}

	if (ok && !IsBmpVariantOk()) {
		printf(	"The bitmap file '%s' is stored in the wrong bitmap sub-format.\n"
				"Make sure you save the bitmap with the following options:\n"
				"\n"
				"     256 colours.\n"
				"     Windows RGB encoded (not RLE or OS/2 encoded)\n"
				"\n",
				filename
			);
		ok = FALSE;
	}
	if (ok && fread(m_Palette, sizeof(m_Palette), 1, fp) != 1) {
		printf("Read #3 failed from file '%s'\n", filename);
		ok = FALSE;
	}

	if (ok) {
		m_pImage = new uchar[(uint) m_InfoHdr.biSizeImage];
		if (m_pImage == NULL) {
			printf("Out of memory trying to allocate a buffer for '%s'.\n", filename);
			ok = FALSE;
		}
		else {
			memset(m_pImage, 0, (uint) m_InfoHdr.biSizeImage);
		}
	}

	/*lint -esym(668,fread) Ignore possible null pointer*/
	if (ok && fread(m_pImage, (uint) m_InfoHdr.biSizeImage, 1, fp) != 1) {
		printf("Read #4 failed from file '%s'\n", filename);
		ok = FALSE;
	}
	/*lint +esym(668,m_pImage) */

	fclose(fp);
	return ok;
}

BOOL
BMP::Create(
	ushort	width,
	ushort 	height,
	uchar	*pImage
) {
	ulong	iw;

	iw = width;
	if ((iw & 0x03) != 0) {
		iw += 4;
		iw &= ~0x03;
	}

	m_BmpImageSize	= iw * height;

	m_Hdr.bfType		= 0x4d42;
	m_Hdr.bfSize		= sizeof(m_Hdr) + sizeof(m_InfoHdr) + sizeof(m_Palette) + m_BmpImageSize;
	m_Hdr.bfReserved1	= 0;
	m_Hdr.bfReserved2	= 0;
	m_Hdr.bfOffBits	= sizeof(m_Hdr) + sizeof(m_InfoHdr) + sizeof(m_Palette);

	m_InfoHdr.biSize            = sizeof(m_InfoHdr);
	m_InfoHdr.biWidth           = (long) width;
	m_InfoHdr.biHeight          = (long) height;
	m_InfoHdr.biPlanes          = 1;
	m_InfoHdr.biBitCount        = 8;
	m_InfoHdr.biCompression     = BI_RGB;
	m_InfoHdr.biSizeImage       = m_BmpImageSize;
	m_InfoHdr.biXPelsPerMeter   = 0;
	m_InfoHdr.biYPelsPerMeter   = 0;
	m_InfoHdr.biClrUsed         = 256;
	m_InfoHdr.biClrImportant    = 256;

	m_pImage = new uchar[(uint) m_BmpImageSize];
	if (m_pImage == NULL) {
		printf("Out of memory trying to allocate a buffer for BMP file.\n");
		return FALSE;
	}
	memset(m_pImage, 0, (uint) m_BmpImageSize);

	if (pImage) {
		uint	y;

		for (y = 0; y < height; y++) {
			memcpy(&m_pImage[y * iw], &pImage[y * width], width);
		}
	}

	return TRUE;
}

BOOL
BMP::Save(
	const char *filename
) {
	FILE	*fp;
	BOOL	ok;

	if ((fp = fopen(filename, "wb")) == 0) {
		printf("Unable to open file '%s' for saving.\n", filename);
		return FALSE;
	}

	ok = TRUE;
	if (fwrite(&m_Hdr, sizeof(m_Hdr), 1, fp) != 1) {
		ok = FALSE;
	}

	if (ok && fwrite(&m_InfoHdr, sizeof(m_InfoHdr), 1, fp) != 1) {
		ok = FALSE;
	}

	if (ok && fwrite(m_Palette, sizeof(m_Palette), 1, fp) != 1) {
		ok = FALSE;
	}

	InvertImage();
	if (ok && fwrite(m_pImage, (uint) m_BmpImageSize, 1, fp) != 1) {
		ok = FALSE;
	}
	InvertImage();

	fclose(fp);
	if (!ok) {
		printf("Write to '%s' failed, disk full?\n", filename);
		return FALSE;
	}
	return TRUE;
}

void
BMP::SetPalette(
	RGBQUAD *pPalette
) {
	memcpy(m_Palette, pPalette, sizeof(m_Palette));
}

BOOL
BMP::IsBmp(
	void
) {
	return (m_Hdr.bfType == 0x4d42);
}

BOOL
BMP::IsBmpVariantOk(
	void
) {
	return (	IsBmp() 					    	&&
			m_InfoHdr.biPlanes 		== 1	    	&&
			m_InfoHdr.biBitCount 	== 8	    	&&
			m_InfoHdr.biCompression 	== BI_RGB
		);
}

void
BMP::InvertImage(
	void
) {
	uchar	*pTmpBuff	= NULL;
	ushort	iw;
	ushort	i;
	ushort	n;

	iw = (ushort) m_InfoHdr.biWidth;

	if ((iw & 0x03) != 0) {
		iw += 4;
		iw &= ~0x03;
	}

	pTmpBuff = new uchar[(uint) iw];
	if (pTmpBuff == NULL) {
		printf("Out of memory trying to allocate invert buffer.\n");
		exit(1);
	}

	for (i = 0, n = (ushort) m_InfoHdr.biHeight - 1; i < (ushort) m_InfoHdr.biHeight / 2; i++, n--) {
		memcpy(pTmpBuff, 			&m_pImage[i * iw], 		iw);
		memcpy(&m_pImage[i * iw], 	&m_pImage[n * iw],		iw);
		memcpy(&m_pImage[n * iw], 	pTmpBuff,				iw);
	}

	delete[] pTmpBuff;
}

/*---------------------------------------------------------------------------
** End of File
*/
