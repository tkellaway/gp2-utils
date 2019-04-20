/*
** File   : jam.h
** Author : tk
** Date   : 04/08/96
**
** $Header:   F:/TK/GP2/GP2JAM/VCS/JAM.H__   1.2   29 Sep 1996 16:38:48   tk  $
**
** JAM class.
*/

#ifndef   jam_h
#define   jam_h

/*---------------------------------------------------------------------------
** Includes
*/

/*---------------------------------------------------------------------------
** Defines and Macros
*/

#define JAM_CAR_WIDTH 			0x0100UL
#define JAM_CAR_HEIGHT			0x00a4UL
#define JAM_CAR_ITEMS			0x01

#define JAM_HELMET_WIDTH 		0x0100UL
#define JAM_HELMET_HEIGHT		0x00b4UL
#define JAM_HELMET_ITEMS			0x1c

#define JAM_ADVERTS1_WIDTH 		0x0100UL
#define JAM_ADVERTS1_HEIGHT		0x00a0UL
#define JAM_ADVERTS1_ITEMS		0x0b

#define JAM_ADVERTS2_WIDTH 		0x0100UL
#define JAM_ADVERTS2_HEIGHT		0x0060UL
#define JAM_ADVERTS2_ITEMS		0x03

#define JAM_ID_CAR_FIRST   		JAM_ID_CAR_WILLIAMS
#define JAM_ID_CAR_LAST			JAM_ID_CAR_PACIFIC

#define JAM_ID_CAR_WILLIAMS		0x0213
#define JAM_ID_CAR_TYRRELL		0x0214
#define JAM_ID_CAR_BENETTON		0x0215
#define JAM_ID_CAR_MCLAREN		0x0216
#define JAM_ID_CAR_FOOTWORK		0x0217
#define JAM_ID_CAR_LOTUS			0x0218
#define JAM_ID_CAR_JORDAN		0x0219
#define JAM_ID_CAR_LAROUSSE		0x021a
#define JAM_ID_CAR_MINARDI		0x021b
#define JAM_ID_CAR_LIGIER		0x021c
#define JAM_ID_CAR_FERRARI		0x021d
#define JAM_ID_CAR_SAUBER		0x021e
#define JAM_ID_CAR_SIMTEK		0x021f
#define JAM_ID_CAR_PACIFIC		0x0220

#define JAM_ID_HELMET_FIRST		0x0221

#define JAM_ID_ADVERTS1_FIRST		0x00ad
#define JAM_ID_ADVERTS2_FIRST		0x00b8

/*---------------------------------------------------------------------------
** Typedefs
*/

typedef struct {
     ushort    num_items;
     ushort    image_total_size;
} JAM_HDR;

typedef struct {
     uchar	x;
     uchar	y;
	ushort	idx_02;
     ushort    width;
     ushort    height;
     ushort    idx_08;
     ushort    idx_0A;
     ushort    image_ptr; //TODO: long ptr?
     ushort    idx_0E;
     ushort    palette_size_div4;
     ushort    jam_id;
     ushort    idx_14;
     uchar     idx_16;
     uchar     idx_17;
     uchar     idx_18[8];
} JAM_ENTRY;

typedef struct {
	uchar	hdr_bytes[36];
	BOOL		duplicate;
	char		*szFilename;
} JAM_ID_TABLE;

class JAM {
public:
	JAM();
	~JAM();
	BOOL		Load(const char *filename);
	BOOL		Save(const char *filename);
	BOOL		IsJam();

	ushort	NumItems() 			const { return m_Hdr.num_items; 		}
	ulong	ImageTotalSize() 		const { return m_Hdr.image_total_size * 0x0100UL; 	}

	ushort	CurrentJamId()			const { return m_Entry.jam_id;		}

	const JAM_HDR		*Hdr()		const { return &m_Hdr;				}
	const JAM_ENTRY	*FirstEntry()	const { return &m_Entry;				}
	JAM_ENTRY			*Entry()		const { return m_pJamEntry;			}
	uchar			*Palette()	const { return m_pPalette; 			}
	uchar 			*Image()		const { return m_pImage; 			}

	const char *JamDescription(const JAM_HDR *pHdr, const JAM_ENTRY *pEntry);
	const JAM_ID_TABLE *FindJam(const JAM_HDR *pHdr, const JAM_ENTRY *pEntry);

private:
	static void	UnJam(void *ptr, uint len);

	JAM_HDR		m_Hdr;
	JAM_ENTRY 	m_Entry;
	JAM_ENTRY		*m_pJamEntry;
	uchar		*m_pBuff;
	uint			m_BuffSize;
	uchar		*m_pPalette;
	uchar		*m_pImage;
};

/*---------------------------------------------------------------------------
** Data
*/

extern JAM_ID_TABLE JamIdTable[];

/*---------------------------------------------------------------------------
** Functions
*/


#endif    /* jam_h */
/*---------------------------------------------------------------------------
** End of File
*/
