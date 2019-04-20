/*
** File   : gp2jam.cpp
** Author : TK
** Date   : 03/08/96
**
** $Header:   F:/TK/GP2/GP2JAM/VCS/GP2JAM.CP_   1.2   29 Sep 1996 16:38:46   tk  $
**
** Convert GP2 .JAM file to .BMP and back again.
**
** History:
**
** V1.0	18th Aug 1996
**
** V2.0	28th Sep 1996
**
**		Fixed bug in XOR routine which overwrote 4 bytes off the end of
**		the buffer.
**
**		Added support for detection of invalid pixel colour palette indexes.
**
**		Changed 'bool' to 'BOOL' for Visual C++ compiler ('bool' is now
**		a reserved word for future use).
**
**		Added support for helmets file.
**
**		Now a protected mode application as helmets file is >64Kb.
*/

#define VERSION     "V2.0 28th Sep 1996"

/*---------------------------------------------------------------------------
** Includes
*/

/*lint -elib(???) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <conio.h>
#include <ctype.h>
#include <math.h>
/*lint +elib(???) */
#include "gp2jam.h"
#include "jam.h"
#include "bmp.h"

#ifdef _COVER_
#include "dump.h"
#endif

/*---------------------------------------------------------------------------
** Defines and Macros
*/

#define PAL_MAX_CUSTOM_INDEX		192U

/*---------------------------------------------------------------------------
** Typedefs
*/

/*---------------------------------------------------------------------------
** Local function prototypes
*/

static void	Usage(void);
static void	Version(void);
static void	BadOption(const char *arg);
static BOOL	QueryOverwrite(const char *filename, BOOL auto_answer_yes);
static uchar	NearestPaletteEntry(RGBQUAD match_rgb);
static void	ShowSupportedJamFilenames(void);
static void	Nop(void);

/*---------------------------------------------------------------------------
** Data
*/

static const char title_msg[] =
     "@(#)"
     "GP2JAM " VERSION " - Grand Prix 2 JAM to BMP Converter\n"
     "Copyright (c) 1996 Trevor Kellaway (100331.2330@compuserve.com)\n\n";

#include "palette.i"

/*---------------------------------------------------------------------------
** Functions
*/

int
main(
	int	argc,
	char	**argv
) {
	BOOL			fSeenFilename			= FALSE;
	BOOL			fSeenFilenameJamFirst	= FALSE;
	BOOL			fSeenFilenameBmpFirst	= FALSE;
	BOOL			fAutoAnswerYes			= FALSE;
	BOOL			fVerbose				= FALSE;
	const char	*pFilenameJam			= NULL;
	const char	*pFilenameBmp			= NULL;
	char			szTmpFilename[256];
	RGBQUAD		pal[256];
	JAM			jam;
	BMP			bmp;
	uint			i;
	uint			n;
	uint			x;
	uint			y;
	uint			nItems;
	uint			nPaletteOffset;

#ifdef _COVER_
	atexit(__tp_dump);
#endif

	fprintf(stderr, &title_msg[4]);

	if (argc == 1) {
		Usage();
		return 1;
	}

	for (--argc, ++argv; argc; argc--, argv++) {
		if (argv[0][0] == '-' || argv[0][0] == '/') {
			switch (argv[0][1]) {
			case 'h':
			case 'H':
			case '?':
				Usage();
				return 0;

			case 'v':
			case 'V':
				Version();
				return 0;

			case 'i':
			case 'I':
				fVerbose = TRUE;
				break;

			case 'j':
			case 'J':
				if (argv[0][2] == 0 || isspace(argv[0][2])) {
					printf(	"You must specify the JAM filename directly after the '-j'.\n"
							"\n"
							"E.g.,\n"
							"\n"
							"     -jFILENAME.JAM\n"
						);
					return 1;
				}

				pFilenameJam = &argv[0][2];
				if (!fSeenFilename) {
					fSeenFilename			= TRUE;
					fSeenFilenameJamFirst	= TRUE;
				}
				break;

			case 'b':
			case 'B':
				if (argv[0][2] == 0 || isspace(argv[0][2])) {
					printf(	"You must specify the BMP filename directly after the '-b'.\n"
							"\n"
							"E.g.,\n"
							"\n"
							"     -bFILENAME.BMP\n"
						);
					return 1;
				}

				pFilenameBmp = &argv[0][2];
				if (!fSeenFilename) {
					fSeenFilename			= TRUE;
					fSeenFilenameBmpFirst	= TRUE;
				}
				break;

			case 'f':
			case 'F':
				ShowSupportedJamFilenames();
				return 0;

			case 'y':
			case 'Y':
				fAutoAnswerYes = TRUE;
				break;

			default:
				BadOption(argv[0]);
				return 1;
			}
		}
		else {
			printf("Incorrect command line argument. Type 'GP2JAM -?' for a usage summary.\n");
			return 1;
		}
	}

	if (pFilenameJam == NULL && pFilenameBmp == NULL) {
		printf("You must specify at least one filename. Type 'GP2JAM -?' for a usage summary.\n");
		return 1;
	}

	if (fSeenFilenameBmpFirst && pFilenameJam == NULL) {
		printf(	"When converting back to a JAM you must specify both the BMP & JAM filenames.\n"
				"Type 'GP2JAM -?' for a usage summary.\n"
			);
		return 1;
	}

	if (fSeenFilenameJamFirst) {
		/*
		** Convert JAM to BMP
		*/
		if (!jam.Load(pFilenameJam)) {
			return 1;
		}

		if (!jam.IsJam()) {
			printf("The file '%s' does not appear to be a known JAM file.\n", pFilenameJam);
			return 1;
		}

		if (pFilenameBmp == NULL) {
			const JAM_ID_TABLE *pJamIdTable = NULL;

			pJamIdTable = jam.FindJam(jam.Hdr(), jam.FirstEntry());
			if (pJamIdTable->duplicate) {
				printf(	"GP2JAM can't be certain exactly which JAM file you have selected\n"
						"so it is unable to automatically generate the output bitmap filename.\n"
						"Please use the -b command line option to specify the bitmap filename.\n");
				return 1;
			}
			else {
				sprintf(szTmpFilename, "%s.bmp", pJamIdTable->szFilename);
				pFilenameBmp = szTmpFilename;
			}
		}

		printf("Converting '%s' (%s) to '%s'.\n",
				pFilenameJam,
				jam.JamDescription(jam.Hdr(), jam.FirstEntry()),
				pFilenameBmp
			);

		if (!QueryOverwrite(pFilenameBmp, fAutoAnswerYes)) {
			return 1;
		}

		/*
		** Map all colour palettes and images to our standard palette.
		*/
		BOOL	warn_no_mapping = FALSE;

		uchar	*pImageCopy;

		pImageCopy = new uchar[jam.ImageTotalSize()];
		if (pImageCopy == NULL) {
			printf("Out of memory trying to allocate image mask.\n");
			return 1;
		}

		if (fVerbose) {
			printf("Processing %u sub-image%s in the %u by %u image.\n",
					jam.NumItems(),
					jam.NumItems() == 1 ? " " : "s",
					0x100,
					jam.ImageTotalSize() / 0x100
				);
		}
		memset(pImageCopy, 0x00, jam.ImageTotalSize());
		nPaletteOffset	 = 0;
		for (nItems = 0; nItems < jam.NumItems(); nItems++) {
			/*
			** Build palette mapping table for this palette.
			*/
			uchar 	pal_mapping[256];
			uchar	new_pal[256];

			memset(pal_mapping, 0, sizeof(pal_mapping));
			memset(new_pal, 0, sizeof(new_pal));

			BOOL	found = TRUE;
			if (jam.Entry()[nItems].palette_size_div4 != 0) {
				for (i = 0; i < 256; i++) {
					if (i < PAL_MAX_CUSTOM_INDEX) {
						new_pal[i] 	= (uchar) i;
						pal[i] 		= RGB_Palette[i];
					}
					else {
						new_pal[i] 	= (uchar) 0;
						pal[i] 		= RGB_Palette[0];
					}
				}

				for (i = 0; i < jam.Entry()[nItems].palette_size_div4; i++) {
					found = FALSE;
					for (n = 0; !found && n < PAL_MAX_CUSTOM_INDEX; n++) {
						if (jam.Palette()[nPaletteOffset + i] == new_pal[n]) {
							pal_mapping[i] = (uchar) n;
							found = TRUE;
						}
					}

					if (!found) {
						pal_mapping[i] = NearestPaletteEntry(RGB_Palette[jam.Palette()[nPaletteOffset + i]]);
						warn_no_mapping = TRUE;
					}
				}
			}
			else {
				/*
				** Use full palette if no palette is present.
				*/
				for (i = 0; i < 256; i++) {
					new_pal[i] = (uchar) i;
					pal[i] 	 = RGB_Palette[i];
				}
			}

			/*
			** Iterate through all sub-images.
			*/
			for (y = jam.Entry()[nItems].y; y < jam.Entry()[nItems].height + (uint) jam.Entry()[nItems].y; y++) {
				for (x = jam.Entry()[nItems].x; x < jam.Entry()[nItems].width + (uint) jam.Entry()[nItems].x; x++) {
					if (jam.Entry()[nItems].palette_size_div4 != 0) {
						/*
						** Map pixel to new colour palette.
						*/
						jam.Image()[(y * 0x100) + x] = pal_mapping[jam.Image()[(y * 0x100) + x]];
					}

					/*
					** Copy sub-image to clean image (remove background garbage).
					*/
					pImageCopy[(y * 0x100) + x] = jam.Image()[(y * 0x100) + x];
				}
			}
			nPaletteOffset += jam.Entry()[nItems].palette_size_div4 * 4;
		}

		memcpy(jam.Image(), pImageCopy, jam.ImageTotalSize());
		delete[] pImageCopy;
		pImageCopy = NULL;

		if (warn_no_mapping) {
			printf(	"\n"
					"Warning: The image contains unique colours which can't be mapped to GP2JAM's\n"
					"standard palette, these will be mapped to the nearest colour available.\n\n"
				);
		}

		if (!bmp.Create(0x100, (ushort) (jam.ImageTotalSize() / 0x100), jam.Image())) {
			return 1;
		}
		bmp.SetPalette(pal);

		if (!bmp.Save(pFilenameBmp)) {
			return 1;
		}

		printf("\n'%s' converted to '%s'.\n", pFilenameJam, pFilenameBmp);
	}
	else if (fSeenFilenameBmpFirst) {
		/*
		** Convert BMP to JAM
		*/
		if (!bmp.Load(pFilenameBmp)) {
			return 1;
		}

		if (access(pFilenameJam, 0x00) != 0) {
			printf(	"The file '%s' does not exist.\n"
					"When converting from a BMP back to a JAM file the destination JAM file\n"
					"must already exist, this program will only update an existing game JAM file.\n",
					pFilenameJam
				);
			return 1;
		}

		for (i = 0; i < 256; i++) {
			if (i < PAL_MAX_CUSTOM_INDEX) {
				pal[i] = RGB_Palette[i];
			}
			else {
				pal[i] = RGB_Palette[0];
			}
		}

		if (memcmp(pal, bmp.Palette(), sizeof(pal)) != 0) {
			printf(	"\n"
					"Warning: It appears that you have modified a colour in the palette.\n"
					"The game uses a fixed palette which can't be modified externally,\n"
					"so don't change any of the colours in the palette.\n\n"
				);
		}

		if (!jam.Load(pFilenameJam)) {
			return 1;
		}

		if (!jam.IsJam()) {
			printf(	"The file '%s' does not appear to be a known JAM file.\n"
					"Type 'GP2JAM -f' for a list of the supported JAM files.\n",
					pFilenameJam
				);
			return 1;
		}

		if (bmp.ImageWidth() * bmp.ImageHeight() != (long) jam.ImageTotalSize()) {
			printf(	"The bitmap file '%s' is the wrong size, it is %lu by %lu,\n"
					"the destination JAM file size is %lu by %lu.\n",
					pFilenameBmp,
					bmp.ImageWidth(),
					bmp.ImageHeight(),
					0x100UL,
					jam.ImageTotalSize() / 0x100UL
				);
			return 1;
		}

		printf("Converting '%s' to '%s' (%s).\n",
				pFilenameBmp,
				pFilenameJam,
				jam.JamDescription(jam.Hdr(), jam.FirstEntry())
			);

		if (!QueryOverwrite(pFilenameJam, fAutoAnswerYes)) {
			return 1;
		}

		/*
		** Invert image and copy to JAM.
		*/
		bmp.InvertImage();
		memcpy(jam.Image(), bmp.Image(), (uint) jam.ImageTotalSize());

		if (fVerbose) {
			printf("Processing %u sub-image%s in the %u by %u image.\n",
					jam.NumItems(),
					jam.NumItems() == 1 ? " " : "s",
					0x100,
					jam.ImageTotalSize() / 0x100
				);
		}

		nPaletteOffset	 = 0;
		for (nItems = 0; nItems < jam.NumItems(); nItems++) {
			if (fVerbose) {
				printf("Processing palette for sub-image #%u at X=%u Y=%u (%u by %u) max %u colours.\n",
						nItems + 1,
						jam.Entry()[nItems].x,
						jam.Entry()[nItems].y,
						jam.Entry()[nItems].width,
						jam.Entry()[nItems].height,
						jam.Entry()[nItems].palette_size_div4
					);
			}

			/*
			** Calculate number of colours used.
			*/
			int 		nColours = 0;
			uint 	aColours[256];
			uchar 	iColours[256];
			uchar 	tColours[256];

			memset(aColours, 0, sizeof(aColours));
			memset(iColours, 0, sizeof(iColours));
			memset(tColours, 0, sizeof(tColours));
			for (y = jam.Entry()[nItems].y; y < jam.Entry()[nItems].y + jam.Entry()[nItems].height; y++) {
				for (x = jam.Entry()[nItems].x; x < jam.Entry()[nItems].x + jam.Entry()[nItems].width; x++) {
					if (aColours[jam.Image()[(y * 0x100) + x]] == 0) {
						++nColours;
					}
					++aColours[jam.Image()[(y * 0x100) + x]];
				}
			}

			/*
			** Build colours into palette index.
			*/
			memset(&jam.Palette()[nPaletteOffset], 0, jam.Entry()[nItems].palette_size_div4 * 4);
			if (jam.Entry()[nItems].palette_size_div4 != 0 && nColours > jam.Entry()[nItems].palette_size_div4) {
				printf(	"\nSorry the sub-image #%u at X=%u Y=%u contains too many colours (%u) for the number\n"
						"available for this item (%u). Try reducing the number of colours.\n",
							nItems + 1,
							jam.Entry()[nItems].x,
							jam.Entry()[nItems].y,
							nColours,
							jam.Entry()[nItems].palette_size_div4
						);
				return 1;
			}

			for (i = 0, n = 0; n < 256; n++) {
				if (aColours[n] != 0) {
					iColours[i] = (uchar) n;
					jam.Palette()[nPaletteOffset + i] = (uchar) n;
					++i;
				}
			}

			/*
			** Remap pixels in sub-image to use new palette indexes.
			*/
			for (n = 0; n < 256; n++) {
				tColours[iColours[n]] = (uchar) n;
			}

			for (y = jam.Entry()[nItems].y; y < jam.Entry()[nItems].y + jam.Entry()[nItems].height; y++) {
				for (x = jam.Entry()[nItems].x; x < jam.Entry()[nItems].x + jam.Entry()[nItems].width; x++) {
					jam.Image()[(y * 0x100) + x] = tColours[jam.Image()[(y * 0x100) + x]];
				}
			}

			/*
			** Build colour fade palette.
			*/
			for (i = 0; i < jam.Entry()[nItems].palette_size_div4; i++) {
				for (n = 1; n < 4; n++) {
					uchar 	ix;
					RGBQUAD 	rgb  = RGB_Palette[jam.Palette()[nPaletteOffset + i]];
					short	r	= rgb.rgbRed;
					short	g	= rgb.rgbGreen;
					short	b	= rgb.rgbBlue;
					r		-= (uchar) (5 * n);
					g		-= (uchar) (5 * n);
					b		-= (uchar) (5 * n);

					if (r >= 0 && g >= 0 && b >= 0) {
						rgb.rgbRed	= (uchar) r;
						rgb.rgbGreen	= (uchar) g;
						rgb.rgbBlue	= (uchar) b;
						ix = NearestPaletteEntry(rgb);
					}
					else {
						ix = jam.Palette()[nPaletteOffset + i];
					}
					jam.Palette()[nPaletteOffset + (jam.Entry()[nItems].palette_size_div4 * n) + i] = ix;
				}
			}

			nPaletteOffset += jam.Entry()[nItems].palette_size_div4 * 4;
		}

		if (!jam.Save(pFilenameJam)) {
			return 1;
		}

		printf("\n'%s' converted to '%s'.\n", pFilenameBmp, pFilenameJam);
	}
	return 0;
}

static void
Usage(
	void
) {
	Nop();
	printf(	"Usage: GP2JAM [-h?v] [-y] [-bFILENAME.BMP] [-jFILENAME.JAM] [-f]\n"
			"\n"
			"     -h|-?         Display this usage message.\n"
			"     -v            Display the version number & contact information.\n"
			"     -i            Display information about conversion process.\n"
			"\n"
			"     -jFNAME.JAM	Set the JAM filename to be 'FNAME.JAM'.\n"
			"     -bFNAME.BMP	Set the BMP filename to be 'FNAME.BMP'.\n"
			"\n"
			"     -y            Automatically answer all questions with 'Yes'.\n"
			"     -f            Display all the JAM filenames GP2JAM currently supports.\n"
			"\n"
			"E.g., To create a BMP (with the default name) for the WILLIAMS.JAM:\n"
			"\n"
			"     GP2JAM -jWILLIAMS.JAM\n"
			"\n"
			"To convert the BMP back to a JAM:\n"
			"\n"
			"     GP2JAM -bWILLIAMS.BMP -jWILLIAMS.JAM\n"
		);
}

static void
Version(
	void
) {
	Nop();
	printf(	"This version of GP2JAM is %s.\n"
			"\n"
			"If you have any comments, suggestions or bug reports please contact the\n"
			"author (make sure you include the version of GP2JAM you are using) at:\n"
			"\n"
			"     Internet:    Trevor Kellaway  100331.2330@compuserve.com\n"
			"     CompuServe:  Trevor Kellaway  [100331,2330]\n"
			"\n"
			"Check the web page for the latest version of GP2JAM:\n"
			"\n"
			"     Web Page:    http://ourworld.compuserve.com/homepages/tkellaway/\n"
			"\n"
			, VERSION
		);
}

static void
BadOption(
	const char	*arg
) {
	Nop();
	printf(	"The command line argument '%s' isn't supported.\n"
			"Type 'GP2JAM -?' for a usage summary.\n",
				arg
		);
}

static BOOL
QueryOverwrite(
	const char	*filename,
	BOOL			auto_answer_yes
) {
	BOOL	ok = TRUE;

	/*
	** Does the file exist?
	*/
	if (access(filename, 0x00) == 0) {
		if (access(filename, 0x02) == 0) {
			/*
			** We have write access to it.
			*/
			if (!auto_answer_yes) {
				fprintf(	stderr,
						"\n"
						"Are you sure you want to overwrite/update '%s'?\n"
						"Press 'Y' for Yes, 'N' or any other key for No: ",
						filename
					);

				int c;

				c = getch();
				if (c != 'y' && c != 'Y') {
					fprintf(stderr, "No\n");
					fprintf(stderr, "Aborting due to your request not to overwrite '%s'.\n", filename);
					ok = FALSE;
				}
				else {
					fprintf(stderr, "Yes\n");
				}
			}
		}
		else {
			/*
			** We don't have write access.
			*/
			printf(	"\n"
					"You don't have write access/permission to '%s'.\n"
					"Make sure the file isn't on read only media like the CD-ROM.\n"
					"If it is on writeable media (like the hard disk) then make it\n"
					"writeable by typing the following at the command prompt:\n"
					"\n"
					"     ATTRIB -r %s\n"
					"\n",
					filename, filename
				);
			ok = FALSE;
		}
	}

	return ok;
}

static uchar
NearestPaletteEntry(
	RGBQUAD	match_rgb
) {
	RGBQUAD	rgb;
	ushort	i;
	ushort	best_index;
	uchar	pal[256];
	long		r;
	long		g;
	long		b;
	long		d;
	long		d_min;

	memset(pal, 0, sizeof(pal));
	for (i = 0; i < PAL_MAX_CUSTOM_INDEX; i++) {
		pal[i] = (uchar) i;
	}

	rgb = match_rgb;

	d_min 		= 0x7fffffff;
	best_index	= 1;
	for (i = 0; i < 256; i++) {
		r = rgb.rgbRed 	- RGB_Palette[pal[i]].rgbRed;
		g = rgb.rgbGreen 	- RGB_Palette[pal[i]].rgbGreen;
		b = rgb.rgbBlue	- RGB_Palette[pal[i]].rgbBlue;

		r *= r;
		g *= g;
		b *= b;

		d = (long) sqrt((double) (r + g + b));
		if (d < d_min) {
			d_min = d;
			best_index = i;
		}
	}

	return (uchar) best_index;
}

static void
ShowSupportedJamFilenames(
	void
) {
	JAM_ID_TABLE	*pJamIdTable = JamIdTable;
	uint			n;
	char			buff[32];

	printf("JAM files supported by GP2JAM (see \\GP2\\GAMEJAMS and sub-directories):\n\n");

	for (n = 0; pJamIdTable->szFilename != NULL; pJamIdTable++) {
		sprintf(buff, "%s.jam%c",
				pJamIdTable->szFilename,
				pJamIdTable->duplicate ? '*' : ' '
			);
		printf("  %-13s", buff);
		if (++n >= 5) {
			n = 0;
			printf("\n");
		}
	}

	if (n != 0) {
		printf("\n\n* GP2JAM can't uniquely determine these files identities (duplicate file/names).\n");
	}
}

static void
Nop(
	void
) {
	;
}

/*---------------------------------------------------------------------------
** End of File
*/
