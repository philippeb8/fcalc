/*! \file xlhtml.c
    \brief converts excel files to Html

   xlhtml generates HTML, XML, csv and tab-delimitted versions of Excel
   spreadsheets.
*/

/*
   Copyright 2002  Charles N Wyble  <jackshck@yahoo.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published  by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

 */


#if !(defined( __BORLANDC__ ) || defined( _WIN32 ))
#include "config.h"		/* Created by ./configure script */
#include "support.h"		/* Needs to be before internal.h */
#include "internal.h"		/* Needs to be before cole */
#include "cole.h"
#else
#include "config.h.in"		/* Created by ./configure script */
#include "support.h"		/* Needs to be before internal.h */
#include "internal.h"		/* Needs to be before cole */
#include "cole.h.in"
#include <io.h>			/* for umask */
//#include <dir.h>
#endif
#include <stdlib.h>		/* For atof(), calloc() */
#include <string.h>		/* For string functions */
#include <math.h>		/* For fabs() */
#include <ctype.h>		/* For isprint() */
#include <errno.h>
/***************************
*	The next couple of lines are "tunable". They are the amount that
*	various things get incremented by when we need more.
****************************/
#define XFORMATS_INCR		64		/*!< Increments to allocate extended formats */
#define FONTS_INCR	 	32 		/*!< Increments to allocate fonts */
#define WORKSHEETS_INCR		4		/*!< Increments to allocate worksheet pages */
#define COLS_INCR		(U16)24 	/*!< Increments to allocate Columns per Worksheet page */
#define ROWS_INCR 		(U32)128	/*!< Increments to allocate Rows per Worksheet page */
#define STRINGS_INCR 		256UL		/*!< Increments to allocate the string array - */

/* Used by packed string array Opcode: 0xFC */
#define HARD_MAX_ROWS_97	0x7FFE    /*!< Used in add_wb_array to prevent OOM */
#define HARD_MAX_ROWS_95	0x3FFF    /*!< Used in add_wb_array to prevent OOM */
#define HARD_MAX_COLS		256	  /*!< Used in add_wb_array to prevent OOM */

U16 HARD_MAX_ROWS = HARD_MAX_ROWS_97;
/**********************************
*
*	Don't change anything below here...
*
************************************/
#if defined( _WIN32 ) || defined( __BORLANDC__ )
#define MAXPATH 80
#include <direct.h>
#define fnsplit _splitpath
#define setdisk(n) _chdrive((n)+1)
#define VERSION  		"0.4.9.1"
#endif
#define PRGNAME 		"xlhtml"
#define WBUFF_SIZE 		8240	/*!< The working buffer. SB 522+10+4(header) bytes minimum = 536 */
#define MAX_COLORS		65	/*!< This is the size of the built-in color table */
#define EXCEL95		0x500		/*!< This is the file stamp for biff7 - Excel 5 & 95 */
#define EXCEL97		0x600		/*!< This is the file stamp for biff8 - Excel 97 & 2000 */

#ifndef S_IRUSR
#define S_IRUSR 0000400        /* read permission, owner */
#endif
#ifndef S_IRGRP
#define S_IRGRP 0000040        /* read permission, group */
#endif
#ifndef S_IROTH
#define S_IROTH 0000004        /* read permission, other */
#endif
#ifndef S_IWUSR
#define S_IWUSR 0000200        /* write permission, owner */
#endif
#ifndef S_IWGRP
#define S_IWGRP 0000020        /* write permission, group */
#endif
#ifndef S_IWOTH
#define S_IWOTH 0000002        /* write permission, other */
#endif
#ifndef S_IXUSR
#define S_IXUSR 0000100        /* execute/search permission, owner */
#endif
#ifndef S_IXGRP
#define S_IXGRP 0000010        /* execute/search permission, group */
#endif
#ifndef S_IXOTH
#define S_IXOTH 0000001        /* execute/search permission, other */
#endif
#ifndef S_IRWXU
#define S_IRWXU 0000700        /* read, write, execute permissions, owner */
#endif
#ifndef S_IRWXG
#define S_IRWXG 0000070        /* read, write, execute permissions, group */
#endif
#ifndef S_IRWXO
#define S_IRWXO 0000007        /* read, write, execute permissions, other */
#endif
#ifndef S_ISBLK
#define S_ISBLK(mode)  (((mode) & (0170000)) == (0060000))
#endif
#ifndef S_ISCHR
#define S_ISCHR(mode)  (((mode) & (0170000)) == (0020000))
#endif
#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & (0170000)) == (0040000))
#endif
#ifndef S_ISFIFO
#define S_ISFIFO(mode) (((mode) & (0170000)) == (0010000))
#endif
#ifndef S_ISREG
#define S_ISREG(mode)  (((mode) & (0170000)) == (0100000))
#endif

#if !(defined( _WIN32 ) || defined( __BORLANDC__ ))
#include <sys/stat.h>
#define GLOBAL_UMASK (S_IXUSR|S_IWGRP|S_IRGRP|S_IXGRP|S_IWOTH|S_IROTH|S_IXOTH)
#else
#define GLOBAL_UMASK (2)
#endif

static char SectionName[2][12] =	/* The section of the Excel Stream where the workbooks are kept */
{
	"/Workbook",		/*!< Excel 97 & 2000 */
	"/Book"			/*!< Everything else ? */
};


typedef struct		/*!< This encapsulates the Unicode String	*/
{
	U8 uni;		/*!< Unicode String: 0==ASCII/8859-1, 1==windows-1252, 2==utf-8 */
	U8 *str;	/*!< Characters of string */
	U16 len;	/*!< Length of string */
	U8 *fmt_run;	/*!< formatting run, short pairs: offset, index to font */
	U8 crun_cnt;	/*!< The count of format runs */
}uni_string;

typedef struct 		/*!< This is everything we need for a cell */
{
	U16 xfmt;	/*!< The high bit will tell us which version 0 =< 2; 1 == 2+ */
	U16 type;	/*!< This will record the record type that generated the cell */
	U16 spanned;		/*!< If 1 don't output */
	uni_string ustr;	/*!< The cell's displayed contents */
	U16 rowspan;		/*!< rows to span */
	U16 colspan;		/*!< columns to span */
	uni_string h_link;	/*!< If a hyperlinked cell, this is the link*/
}cell;

typedef struct	/*!< This encapsulates some information about each worksheet */
{
	U32 first_row;
	S32 biggest_row;
	U32 max_rows;
	U16 first_col;
	S16 biggest_col;
	U16 max_cols;
	uni_string ws_title;
	cell **c_array;
	U16 spanned;
}work_sheet;

typedef struct	/*!< This is everything we need to know about fonts */
{
	U16 size;
	U16 attr;
	U16 c_idx;
	U16 bold;
	U16 super;
	U8 underline;
	uni_string name;
}font_attr;

typedef struct
{
	uni_string *name;
	U16 cnt;
}fnt_cnt;

typedef struct		/*!< This covers the Extended Format records */
{
	U16 fnt_idx;
	U16 fmt_idx;
	U16 gen;
	U16 align;
	U16 indent;
	U16 b_style;
	U16 b_l_color;
	U32  b_t_color;
	U16 cell_color;
}xf_attr;

typedef struct		/*!< HTML Attribute */
{
	int fflag;		/*!< Font Flag */
	int bflag;		/*!< Bold Flag */
	int iflag;		/*!< Itallic Flag */
	int sflag;		/*!< Strike thru flag */
	int uflag;		/*!< Underline flag */
	int sbflag;		/*!< Subscript */
	int spflag;		/*!< Superscript */
}html_attr;

static int numCustomColors = 0;
static U8 **customColors = 0;
static char colorTab[MAX_COLORS][8] =
{
	"000000",	/* FIXME: Need to find these first 8 colors! */
	"FFFFFF",
	"FFFFFF",
	"FFFFFF",
	"FFFFFF",
	"FFFFFF",
	"FFFFFF",
	"FFFFFF",
	"FFFFFF",	/*0x08 - This one's Black, too ??? */
	"FFFFFF", /* This one's normal */
	"red",	/*	"FF0000", */
	"lime",	/*	"00FF00", */
	"blue",	/*	"0000FF", */
	"FFFF00",
	"FF00FF",
	"aqua",	/*	"00FFFF", */
	"800000",	/* 0x10 */
	"green",	/*	"008000", */
	"navy",	/*	"000080", */
	"808000",
	"800080",
	"teal",	/*	"008080", */
	"C0C0C0",
	"gray",	/*	"808080", */
	"9999FF",	/* 0x18 */
	"993366",
	"FFFFCC",
	"CCFFFF",
	"660066",
	"FF8080",
	"0066CC",
	"CCCCFF",
	"000080",
	"FF00FF",	/* 0x20 */
	"FFFF00",
	"00FFFF",
	"800080",
	"800000",
	"008080",
	"0000FF",
	"00CCFF",	/* 0x28 */
	"CCFFFF",
	"CCFFCC",
	"FFFF99",
	"99CCFF",
	"FF99CC",
	"CC99FF",
	"FFCC99",
	"3366FF",	/* 0x30 */
	"33CCCC",
	"99CC00",
	"FFCC00",
	"FF9900",
	"FF6600",
	"666699",
	"969696",
	"003366",	/* 0x38 */
	"339966",
	"003300",
	"333300",
	"993300",
	"993366",
	"333399",
	"333333",
	"FFFFFF"	/* 0x40 */
};

/* FIXME: Support major languages here...not just English */
static const char month_abbr[12][5] = {	"Jan", "Feb", "Mar", "Apr", "May", "June",
					"July", "Aug", "Sep", "Oct", "Nov", "Dec" };
static const int ndays[]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static const int ldays[]={31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


/**************************************************************************
*	Functions and data are declared static to prevent name collisions.
***************************************************************************/
/* Function Prototypes */
static void print_version(void);
COLE_LOCATE_ACTION_FUNC scan_file;
static void OutputPartialTableAscii(void);
static void OutputTableHTML(void);
static void OutputTableXML(void);
static void do_cr(void);
static void main_line_processor(U16, U16, U32, U16, U8);
static void SetupExtraction(void);
static void output_header(void);
static void output_footer(void);
static U16 getShort(U8 *);
static S32 getLong(U8 *);
static void getDouble(U8 *, F64 *);
static void RKtoDouble(S32, F64 *);
static void decodeBoolErr(U16, U16, char *);
static int IsCellNumeric(cell *);
static int IsCellSafe(cell *);
static int IsCellFormula(cell *);
static void output_cell(cell *, int);
static void output_formatted_data(uni_string *, U16, int, int);
static void PrintFloatComma(char *, int, F64);
static void print_as_fraction(F64, int);
static void NumToDate(long, int *, int *, int *);
static void FracToTime(U8 *, int *, int *, int *, int *);
static void trim_sheet_edges(unsigned int);
static void update_default_font(unsigned int);
static void incr_f_cnt(uni_string *);
static int get_default_font(void);
static void update_default_alignment(unsigned int, int);
static int null_string(U8 *);
static void OutputString(uni_string *);
static void OutputCharCorrected(U8);
static void update_crun_info(U16 *loc, U16 *fnt_idx, U16 crun_cnt, U8 *fmt_run);
static void put_utf8(U16);
static void print_utf8(U16);
static void uni_string_clear(uni_string *);
static int uni_string_comp(uni_string *, uni_string *);
static void output_start_html_attr(html_attr *h, unsigned int, int);
static void output_end_html_attr(html_attr *h);
static void html_flag_init(html_attr *h);
static void output_start_font_attribute(html_attr *h, U16 fnt_idx);

/* The array update functions */
static int ws_init(int);
static int add_more_worksheet_ptrs(void);
static int resize_c_array(work_sheet *, U32, U16);
static void add_wb_array(U16, U16, U16, U16, U8, U8 *, U16, U16, U8 *);
static void update_cell_xf(U16, U16, U16);
static void update_cell_hyperlink(U16 r, U16 c, U8 *hyperlink, int len, U16 type);
static void add_str_array(U8, U8 *, U16, U8 *, U8);
static void add_font(U16, U16, U16, U16, U16, U8, U16, U8 *, U16);
static void add_ws_title(U16, U8 *, U16);
static void add_xf_array(U16 fnt_idx, U16 fmt_idx, U16 gen, U16 align,
	U16 indent, U16 b_style, U16 b_l_color, U32  b_t_color, U16 cell_color);


/* Global data */
static FILE * pinfile = 0;
static FILE * poutfile = 0;
static char filename[128];
static int file_version = 0;
static U32 next_string=0;
static unsigned int next_font=0, next_ws_title=0, next_xf=0;
static U8 working_buffer[WBUFF_SIZE];
static unsigned int bufidx, buflast;	/*!< Needed for working buffer */
static U8 grbit=0;			/*!< Needed by the SST Opcode FC */
static U16 crun=0, cch=0;               /*!< Needed by the SST Opcode FC */
static U32 extrst=0;			/*!< Needed by the SST Opcode FC */
static U16 nonascii = 0;		/*!< Needed by the SST Opcode FC */
static int sheet_count=-2;		/*!< Number of worksheets found */
static U16 last_opcode = -1;		/*!< Used for the continue command */
static unsigned int cont_grbit=0, cont_str_array=0;
static uni_string default_font;		/*!< Font for table */
static int default_fontsize = 3;	/*!< Default font size for table */
static char *default_alignment = 0;	/*!< Alignment for table */
static int first_sheet = 0;		/*!< First worksheet to display */
static int last_sheet = WORKSHEETS_INCR-1;	/*!< The last worksheet to display */
static S16 xp=0, xr1=-1, xr2=-1, xc1=-1, xc2=-1; /*!< Extraction info... */
static int currency_symbol = '$';		/*!< What to use for currency */
static U16 str_formula_row = 0;		/*!< Data holders for string formulas */
static U16 str_formula_col = 0;         /*!< Data holders for string formulas */
static U16 str_formula_format = 0;	/*!< Data holders for string formulas */

/* Limits */
static unsigned int max_fonts = FONTS_INCR;
static unsigned int max_xformats = XFORMATS_INCR;
static unsigned long max_strings = STRINGS_INCR;
static unsigned int max_worksheets = WORKSHEETS_INCR;

/* Global arrays */
static xf_attr **xf_array;
static work_sheet **ws_array;
static uni_string **str_array;
static font_attr **font_array;
static fnt_cnt *f_cnt;
static int fnt_size_cnt[7];		/*!< Html has only 7 sizes... */
static uni_string author;
static char *title = 0;
static char *lastUpdated = 0;

/* Command Line flags */
static int use_colors = 1;		/*!< Whether or not to use colors in output */
static int aggressive = 0;		/*!< Aggressive html optimization */
static int formula_warnings = 1;	/*!< Whether or not to suppress formula warnings */
static int center_tables = 0;		/*!< Whether or not to center justify tables or leave it left */
static int trim_edges = 0;		/*!< Whether or not to trim the edges of columns or rows */
static char *default_text_color = "000000";
static char *default_background_color="FFFFFF";
static char *default_image=NULL;	/*!< Point to background image */
static int Ascii = 0;			/*!< Whether or not to out ascii instaed of html */
static int Csv = 0;			/*!< Whether or not to out csv instaed of html */
static int OutputXML = 0;		/*!< Output as xml */
static int DumpPage = 0;		/*!< Dump page count & max cols & rows */
static int Xtract = 0;			/*!< Extract a range on a page. */
static int MultiByte = 0;		/*!< Output as multibyte */
static int NoHeaders = 0;		/*!< Dont output html header */


/* Some Global Flags */
static int notAccurate = 0;	/*!< Flag used to indicate that stale data was used */
static int NoFormat = 0;	/*!< Flag used to indicated unimplemented format */
static int NotImplemented = 0;	/*!< Flag to print unimplemented cell type message */
static int Unsupported = 0;	/*!< Flag to print unsupported cell type message */
static int DatesR1904 = 0;	/*!< Flag that the dates are based on McIntosh Dates system */
static int MaxPalExceeded = 0;
static int MaxXFExceeded = 0;
static int MaxFormatsExceeded = 0;
static int MaxColExceeded = 0;
static int MaxRowExceeded = 0;
static int MaxWorksheetsExceeded = 0;
static int MaxStringsExceeded = 0;
static int MaxFontsExceeded = 0;
static int UnicodeStrings = 0;	/*!< 0==ASCII, 1==windows-1252, 2==uft-8 */
static int CodePage = 0;			/*!< Micosoft CodePage as specified in the Excel file. */


void init()
{
	/* Global data */
	pinfile = 0;
	poutfile = 0;
	filename[128];
	file_version = 0;
	next_string=0;
	next_font=0, next_ws_title=0, next_xf=0;
	working_buffer[WBUFF_SIZE];
	bufidx, buflast;	/*!< Needed for working buffer */
	grbit=0;			/*!< Needed by the SST Opcode FC */
	crun=0, cch=0;               /*!< Needed by the SST Opcode FC */
	extrst=0;			/*!< Needed by the SST Opcode FC */
	nonascii = 0;		/*!< Needed by the SST Opcode FC */
	sheet_count=-2;		/*!< Number of worksheets found */
	last_opcode = -1;		/*!< Used for the continue command */
	cont_grbit=0, cont_str_array=0;
	default_font;		/*!< Font for table */
	default_fontsize = 3;	/*!< Default font size for table */
	default_alignment = 0;	/*!< Alignment for table */
	first_sheet = 0;		/*!< First worksheet to display */
	last_sheet = WORKSHEETS_INCR-1;	/*!< The last worksheet to display */
	xp=0, xr1=-1, xr2=-1, xc1=-1, xc2=-1; /*!< Extraction info... */
	currency_symbol = '$';		/*!< What to use for currency */
	str_formula_row = 0;		/*!< Data holders for string formulas */
	str_formula_col = 0;         /*!< Data holders for string formulas */
	str_formula_format = 0;	/*!< Data holders for string formulas */

	/* Limits */
	max_fonts = FONTS_INCR;
	max_xformats = XFORMATS_INCR;
	max_strings = STRINGS_INCR;
	max_worksheets = WORKSHEETS_INCR;

	/* Global arrays */
	xf_array;
	ws_array;
	str_array;
	font_array;
	f_cnt;
	fnt_size_cnt[7];		/*!< Html has only 7 sizes... */
	author;
	title = 0;
	lastUpdated = 0;

	/* Command Line flags */
	use_colors = 1;		/*!< Whether or not to use colors in output */
	aggressive = 0;		/*!< Aggressive html optimization */
	formula_warnings = 1;	/*!< Whether or not to suppress formula warnings */
	center_tables = 0;		/*!< Whether or not to center justify tables or leave it left */
	trim_edges = 0;		/*!< Whether or not to trim the edges of columns or rows */
	default_text_color = "000000";
	default_background_color="FFFFFF";
	default_image=NULL;	/*!< Point to background image */
	Ascii = 0;			/*!< Whether or not to out ascii instaed of html */
	Csv = 0;			/*!< Whether or not to out csv instaed of html */
	OutputXML = 0;		/*!< Output as xml */
	DumpPage = 0;		/*!< Dump page count & max cols & rows */
	Xtract = 0;			/*!< Extract a range on a page. */
	MultiByte = 0;		/*!< Output as multibyte */
	NoHeaders = 0;		/*!< Dont output html header */


	/* Some Global Flags */
	notAccurate = 0;	/*!< Flag used to indicate that stale data was used */
	NoFormat = 0;	/*!< Flag used to indicated unimplemented format */
	NotImplemented = 0;	/*!< Flag to print unimplemented cell type message */
	Unsupported = 0;	/*!< Flag to print unsupported cell type message */
	DatesR1904 = 0;	/*!< Flag that the dates are based on McIntosh Dates system */
	MaxPalExceeded = 0;
	MaxXFExceeded = 0;
	MaxFormatsExceeded = 0;
	MaxColExceeded = 0;
	MaxRowExceeded = 0;
	MaxWorksheetsExceeded = 0;
	MaxStringsExceeded = 0;
	MaxFontsExceeded = 0;
	UnicodeStrings = 0;	/*!< 0==ASCII, 1==windows-1252, 2==uft-8 */
	CodePage = 0;			/*!< Micosoft CodePage as specified in the Excel file. */
}

int convert_xls_csv(char const * fin, char const * fout)
{
	int i, f_ptr = 0;
	U16 k;
	U32 j;
	COLEFS * cfs;
	COLERRNO colerrno;

	init();

	//freopen("fcalc-temp.csv", "w", stdout);
	pinfile = fopen(fin, "r");
	if (!pinfile)
	{
		return 0;
	}

	poutfile = fopen(fout, "w");
	if (!poutfile)
	{
		fclose(pinfile);
		return 0;
	}

	strncpy(filename, fin, 124);
	filename[124] = 0;

	Ascii = 1;
	Csv = 1;

	Xtract = 1;

	xp = (S16)0;

	xc1 = (S16)-1;
	xc2 = (S16)-1;

	xr2 = (S16)-1;
	xr1 = (S16)-1;

	if (Xtract)
		trim_edges = 0;		/* No trimming when extracting... */

	/* Init arrays... */
	ws_array = (work_sheet **)malloc(max_worksheets * sizeof(work_sheet*));
	for (i=0; i<(int)max_worksheets; i++)
		ws_array[i] = 0;

	str_array = (uni_string **)malloc(max_strings*sizeof(uni_string *));
	for (i=0; i<(int)max_strings; i++)
		str_array[i] = 0;

	font_array = (font_attr **)malloc(max_fonts * sizeof(font_attr *));
	f_cnt = (fnt_cnt *)malloc(max_fonts * sizeof(fnt_cnt));
	for (i=0; i<(int)max_fonts; i++)
	{	/* I assume these won't fail since we are just starting up... */
		font_array[i] = 0;
		f_cnt[i].name = 0;
	}
	xf_array = (xf_attr **)malloc(max_xformats * sizeof(xf_attr *));
	for (i=0; i<(int)max_xformats; i++)
		xf_array[i] = 0;

	uni_string_clear(&author);
	uni_string_clear(&default_font);
	umask(GLOBAL_UMASK);

#if defined( _WIN32 ) || defined( __BORLANDC__ )
	{
		char *ptr = strchr(filename, ':');
		if (ptr)
		{
			int len;
			char new_drive[MAXPATH];
			fnsplit(filename, new_drive, 0, 0, 0);
			if (new_drive[0] >= 'a')
				setdisk(new_drive[0] - 'a');
			else
				setdisk(new_drive[0] - 'A');
			ptr++;	/* Get past the colon */
			len = strlen(ptr);
			memmove(filename, ptr, len);
			filename[len] = 0;
		}
	}
#endif
	/* If successful, this calls scan_file to extract the work book... */
	cfs = cole_mount(filename, &colerrno);
	if (cfs == NULL)
	{
		cole_perror (NULL, colerrno);
		return 0;
	}
	while (cole_locate_filename (cfs, SectionName[f_ptr], NULL, scan_file, &colerrno))
	{
		if (f_ptr)
		{	/* Two strikes...we're out! */
			cole_perror (PRGNAME, colerrno);
			if (colerrno == COLE_EFILENOTFOUND)
				fprintf(stderr, "Section: Workbook\n");
			break;
		}
		else
			f_ptr++;
	}

	if (cole_umount (cfs, &colerrno))
	{
		cole_perror (PRGNAME, colerrno);
		return 0;
	}

	/* For some reason, this loop core dumps on some */
	/* files from Central Europe...so it's commented out for now */
/*	for (i=0; i<max_strings; i++)
	{
		if (str_array[i])
		{
			if (str_array[i]->str)
				free(str_array[i]->str);
			free(str_array[i]);
		}
	}	*/

	for (i=0; i<(int)max_fonts; i++)
	{
		if (font_array[i])
		{
			if (font_array[i]->name.str)
				free(font_array[i]->name.str);
			free(font_array[i]);
			if (f_cnt[i].name)
			{
				if (f_cnt[i].name->str)
					free(f_cnt[i].name->str);
				free(f_cnt[i].name);
			}
		}
	}
	free(font_array);
	free(f_cnt);
	for (i=0; i<(int)max_worksheets; i++)
	{
		if (ws_array[i])
		{
			if (ws_array[i]->ws_title.str)
				free(ws_array[i]->ws_title.str);
			if (ws_array[i]->c_array)
			{
				for (j=0; j<ws_array[i]->max_rows; j++)
				{
					for (k=0; k<ws_array[i]->max_cols; k++)
					{
						if (ws_array[i]->c_array[(j*ws_array[i]->max_cols)+k])
						{
							if (ws_array[i]->c_array[(j*ws_array[i]->max_cols)+k]->ustr.str)
								free(ws_array[i]->c_array[(j*ws_array[i]->max_cols)+k]->ustr.str);
							if (ws_array[i]->c_array[(j*ws_array[i]->max_cols)+k]->ustr.fmt_run)
								free(ws_array[i]->c_array[(j*ws_array[i]->max_cols)+k]->ustr.fmt_run);
							if (ws_array[i]->c_array[(j*ws_array[i]->max_cols)+k]->h_link.str)
								free(ws_array[i]->c_array[(j*ws_array[i]->max_cols)+k]->h_link.str);
							free(ws_array[i]->c_array[(j*ws_array[i]->max_cols)+k]);
						}
					}
				}
				free(ws_array[i]->c_array);
			}
			free(ws_array[i]);
		}
	}

	for (i=0; i<(int)max_xformats; i++)
	{
		if (xf_array[i])
			free(xf_array[i]);
	}
	free(xf_array);

	if (numCustomColors)
	{
		for (i=0; i<numCustomColors; i++)
			free(customColors[i]);
		free(customColors);
	}

	if (default_font.str)
		free(default_font.str);
	if (author.str)
		free(author.str);
	if (title)
		free(title);
	if (lastUpdated)
		free(lastUpdated);

	fclose(pinfile);
	fclose(poutfile);

	return 1;
}

static void print_version(void)
{
	fprintf(poutfile, "xlhtml %s\nCopyright (c) 1999-2002, Charles Wyble\n" 
	"Released under GPL.\n", VERSION);
	exit(1);
}

void scan_file(COLEDIRENT *cde, void *_info)
{
	U32 count = 0;
	U16 length=0, target=0, opcode=0, version=0;
	U8 buf[16];
	COLEFILE *cf;
	COLERRNO err;

	cf = cole_fopen_direntry(cde, &err);
	if (cf == 0)
	{	/* error abort processing */
		cole_perror (PRGNAME, err);
		return;
	}

	/* Read & process the file... */
	while (cole_fread(cf, buf, 1, &err))
	{
		if (count > 3)
			main_line_processor(opcode, version, count-4, target, buf[0]);
		else if (count == 0)
		{	/* Init everything */
			length = 0;
			opcode = (U16)buf[0];
			target = 80;	/* ficticious number */
		}
		else if (count == 1)
			version = (U16)buf[0];
		else if (count == 2)
			length = (U16)buf[0];
		else if (count == 3)
		{
			length |= (U16)(buf[0]<<8);
			target = length;
		}

		if (count == (U32)(target+3))
			count = 0;
		else
			count++;
		if (MaxColExceeded || MaxRowExceeded || MaxWorksheetsExceeded)
			break;	/* We're outta memory and therefore...done */

	}
	cole_fclose(cf, &err);

	if (Ascii)
	{
		if (DumpPage)
		{	/* Output the XLS Parameters */
			int i;
			fprintf(poutfile, "There are %d pages total.\n", sheet_count+1);
			for (i=0; i<=sheet_count; i++)
			{
				fprintf(poutfile, "Page:%d Name:%s MaxRow:%ld MaxCol:%d\n", i,
					ws_array[i]->ws_title.str ? (char *)ws_array[i]->ws_title.str : "(Unknown Page)",
					ws_array[i]->biggest_row, ws_array[i]->biggest_col);
			}
		}
		else if (Xtract)
			OutputPartialTableAscii();
	}
}

static void OutputPartialTableAscii(void)
{
	int i, j, k;

	SetupExtraction();

	/* Here's where we dump the Html Page out */
	for (i=first_sheet; i<=last_sheet; i++)	/* For each worksheet */
	{
		if (ws_array[i] == 0)
			continue;
		if ((ws_array[i]->biggest_row == -1)||(ws_array[i]->biggest_col == -1))
			continue;
		if (ws_array[i]->c_array == 0)
			continue;

		/* Now dump the table */
		for (j=ws_array[i]->first_row; j<=ws_array[i]->biggest_row; j++)
		{
			for (k=ws_array[i]->first_col; k<=ws_array[i]->biggest_col; k++)
			{
				int safe, numeric=0;
				cell *c = ws_array[i]->c_array[(j*ws_array[i]->max_cols)+k]; /* This stuff happens for each cell... */

				if (c)
				{
					numeric = IsCellNumeric(c);
//					if (!numeric && Csv)
//						fprintf(poutfile, "\"");
					safe = IsCellSafe(c);

					if (c->ustr.str)
					{
						if (safe)
							output_formatted_data(&(c->ustr), xf_array[c->xfmt]->fmt_idx, numeric, IsCellFormula(c));
						else
							OutputString(&(c->ustr));
					}
					else if (!Csv)
						fprintf(poutfile, " ");	/* Empty cell... */
				}
				else
				{   	/* Empty cell... */
					if (!Csv)
						fprintf(poutfile, " ");
//					else
//						fprintf(poutfile, "\"");
				}
				if (ws_array[i]->c_array[(j*ws_array[i]->max_cols)+k])	/* Honor Column spanning ? */
				{
					if (ws_array[i]->c_array[(j*ws_array[i]->max_cols)+k]->colspan != 0)
						k += ws_array[i]->c_array[(j*ws_array[i]->max_cols)+k]->colspan-1;
				}
//				if (!numeric && Csv)
//					fprintf(poutfile, "\"");

				if (Csv && (k < ws_array[i]->biggest_col))
				{	/* big cheat here: quoting everything! */
					fputc(',', poutfile);	/* Csv Cell Separator */
//					putchar(';');	/* Csv Cell Separator */
				}
//				else
//				{
//					if (( !Csv )&&( k != ws_array[i]->biggest_col ))
//						putchar('\t');	/* Ascii Cell Separator */
//				}
			}
			if (Csv)
//				fprintf(poutfile, "\r\n");
				fprintf(poutfile, "\n");
//			else
//				putchar(0x0A);		/* Row Separator */
		}
		if (!Csv)
			fprintf(poutfile, "\n\n");		 	/* End of Table 2 LF-CR */
	}
}

static void do_cr(void)
{
	if (!aggressive)
//		putchar('\n');
		fputc('\n', poutfile);
}

static void SetupExtraction(void)
{
	if (Xtract)
	{	/* Revise the page settings... */
/*		fprintf(poutfile, "-%d %d %d %d %d<br>\n", xp, xr1, xr2, xc1, xc2); */
		if ((xp >= first_sheet)&&(xp <= last_sheet)&&(xp <= sheet_count))
		{
			first_sheet = xp;
			last_sheet = xp;
			if (xr1 < 0)
			{
				xr1 = (S16)ws_array[xp]->first_row;
				xr2 = (S16)ws_array[xp]->biggest_row;
			}
			else if ((xr1 >= ws_array[xp]->first_row)&&(xr1 <= ws_array[xp]->biggest_row)
				&&(xr2 >= ws_array[xp]->first_row)&&(xr2 <= ws_array[xp]->biggest_row))
			{
				ws_array[xp]->first_row = xr1;
				ws_array[xp]->biggest_row = xr2;

				if (xc1 < 0)
				{
					xc1 = ws_array[xp]->first_col;
					xc2 = ws_array[xp]->biggest_col;
				}
				else if((xc1 >= ws_array[xp]->first_col)&&(xc1 <= ws_array[xp]->biggest_col)
					&&(xc2 >= ws_array[xp]->first_col)&&(xc2 <= ws_array[xp]->biggest_col))
				{
					ws_array[xp]->first_col = xc1;
					ws_array[xp]->biggest_col = xc2;
				}
				else
				{
					if (Ascii)
						fprintf(stderr, "Error - Col not in range during extraction"
						    " (%d or %d not in [%d..%d])\n", xc1, xc2, ws_array[xp]->first_col, ws_array[xp]->biggest_col);
					return;
				}
			}
			else
			{
				if (Ascii)
					fprintf(stderr, "Error - Row not in range during extraction"
					    " (%d or %d not in [%ld..%ld])\n", xr1, xr2, ws_array[xp]->first_row, ws_array[xp]->biggest_row);
				return;
			}
		}
		else
		{
			if (Ascii)
				fprintf(stderr, "Error - Page not in range during extraction.");
			return;
		}
	}
}

/*!******************************************************************
*	\param count	the absolute count in the record
*	\param last	the size of the record
*	\param bufidx	the index into the working buffer
*	\param buflast	the expected length of the working buffer
********************************************************************/
static void main_line_processor(U16 opcode, U16 version, U32 count, U16 last, U8 data)
{
	U16 cont_opcode = 0;
	
	/* If first pass, reset stuff. */
	if (count == 0)
	{
		if (opcode != 0x3C)	/* continue command */
/*		{
			fprintf(poutfile, "\n* * * * * * CONTINUE * * * * * * * * *\n\n");
		}
		else */
		{	/* Normal path... */
			last_opcode = opcode;
			bufidx = 0;
			buflast = 0;
			cont_str_array = 0;
			memset(working_buffer, 0, WBUFF_SIZE);
		}
	}
	if (opcode == 0x3C)
	{
		opcode = last_opcode;
		cont_opcode = 1;
	}

	/* Abort processing if too big. Next opcode will reset everything. */
	if (bufidx >= WBUFF_SIZE)
	{
		/*fprintf(poutfile, "OC:%02X C:%04X I:%04X BL:%04X cch:%04X gr:%04X\n", opcode, count, bufidx, buflast, cch, grbit); */
		/*abort(); */
		return;
	}

	/* no chart processing for now. */
	if (version == 0x0010)
		return;

	switch (opcode)
	{
		case 0x09:	/* BOF */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{
				if (file_version == 0)
				{	/* File version info can be gathered here...
					 *    4 = Excel version 4
					 * 1280 = Excel version 5
					 * 0500 = Excel 95
					 * 1536 = Excel 97 */
					if (version == 8)
						file_version = getShort(&working_buffer[0]);
					else
						file_version = version;
					if (file_version == EXCEL95)
					{
						use_colors = 0;
						HARD_MAX_ROWS = HARD_MAX_ROWS_95;
					}
/*					fprintf(poutfile, "Biff:%X\n", file_version); */
				}
				sheet_count++;
				if (sheet_count >= (int)max_worksheets)
					add_more_worksheet_ptrs();
			}
			break;
		case 0x01:	/* Blank */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{
				U16 r, c, f;

				r = getShort(&working_buffer[0]);
				c = getShort(&working_buffer[2]);
				if (version == 2)
					f = getShort(&working_buffer[4]);
				else
					f = 0;
				add_wb_array(r, c, f, opcode, (U16)0, (U8 *)0, 0, (U16)0, 0);
			}
			break;
		case 0x02:	/* Integer */
			working_buffer[bufidx++] = data;		
			if (bufidx == last)
			{
				U16 r, c, i, f;
				char temp[32];

				r = getShort(&working_buffer[0]);
				c = getShort(&working_buffer[2]);
				if (version == 2)
				{
					f = getShort(&working_buffer[4]);
					i = getShort(&working_buffer[7]);
					sprintf(temp, "%d", i);
				}
				else
				{
					f = 0;
					Unsupported++;
//					strcpy(temp, OutputXML ? "<Unsupported/>INT" : "****INT");
					strcpy(temp, OutputXML ? "<Unsupported/>INT" : "INT");
				}
				add_wb_array(r, c, f, opcode, (U16)0, (U8 *)temp, (U16)strlen(temp), 0, NULL);
			}		
			break;
		case 0x03:	/* Number - Float */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{
				U16 r, c, f;
				F64 d;
				char temp[64];

				r = getShort(&working_buffer[0]);
				c = getShort(&working_buffer[2]);
				if (version == 2)
				{
					f = getShort(&working_buffer[4]);
					getDouble(&working_buffer[6], &d);
					sprintf(temp, "%.15g", d);
				}
				else
				{	/* Who knows what the future looks like */
					f = 0;
					Unsupported = 1;
//					sprintf(temp, "****FPv:%d", version);
					sprintf(temp, "FPv:%d", version);
				}
				add_wb_array(r, c, f, opcode, (U16)0, (U8 *)temp, (U16)strlen(temp), 0, 0);
			}
			break;
		case 0xD6:	/* RString */
			working_buffer[bufidx++] = data;		
			if ((bufidx == 8)&&(buflast == 0))
				buflast = 8 + getShort(&working_buffer[6]);
			if (buflast)
			{
				if (bufidx == buflast)
				{
					U16 r, c, l, f;

					r = getShort(&working_buffer[0]);
					c = getShort(&working_buffer[2]);
					f = getShort(&working_buffer[4]);
					l = getShort(&working_buffer[6]);
					working_buffer[8+l] = 0;

					add_wb_array(r, c, f, opcode, (U16)0, &working_buffer[8],
							(U16)strlen((char *)&working_buffer[8]), 0, 0);
				}
			}
			break;
		case 0x04:	/* Label - UNI */
			working_buffer[bufidx++] = data;
			if (file_version == EXCEL95)
			{
				if (bufidx == last)
				{
					U16 r, c, f;

					r = getShort(&working_buffer[0]);
					c = getShort(&working_buffer[2]);
					f = getShort(&working_buffer[4]);
					working_buffer[bufidx] = 0;

					add_wb_array(r, c, f, opcode, (U16)0, &working_buffer[8],
							(U16)strlen((char *)&working_buffer[8]), 0, 0);
				}
			}
			else if (file_version == EXCEL97)
			{    /* Remember, bufidx is 1 more than it should be */
				if ((bufidx == 8)&&(buflast == 0))
				{	/* buflast = working_buffer[7]; */
					cch = getShort(&working_buffer[6]);
					buflast = cch + 9;
				}
				if (bufidx == 9)
				{
					if (working_buffer[8] == 1)
						buflast = (cch << 1) + 9;
				}
				if (buflast)
				{
					if (bufidx == buflast)
					{
						U16 r, c, f;
						U16 len;

						r = getShort(&working_buffer[0]);
						c = getShort(&working_buffer[2]);
						if (version == 2)
							f = getShort(&working_buffer[4]);
						else	/* Unknown version */
							f = 0;
						working_buffer[bufidx] = 0;

						len = (U16)strlen((char *)&working_buffer[8]);
						if (working_buffer[8] == 1)
						{
							UnicodeStrings = 2;
							add_wb_array(r, c, f, opcode, (U16)2, &working_buffer[9], (U16)(cch << 1), 0, 0);
						}
						else
							add_wb_array(r, c, f, opcode, (U16)0, &working_buffer[8], len, 0, 0);
					}
				}
			}
			break;
		case 0x05:	/* Boolerr */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{
				U16 r, c, f;
				char temp[16];

				r = getShort(&working_buffer[0]);
				c = getShort(&working_buffer[2]);
				if (version == 2)
				{
					f = getShort(&working_buffer[4]);
					decodeBoolErr(working_buffer[6], working_buffer[7], temp);
					add_wb_array(r, c, f, opcode, (U16)0, (U8 *)temp, (U16)strlen(temp), 0, 0);
				}
				else
				{
					f = 0;
					Unsupported = 1;
//					strcpy(temp, "****Bool");
					strcpy(temp, "Bool");
					add_wb_array(r, c, f, opcode, (U16)0, (U8 *)temp, (U16)strlen(temp), 0, 0);
				}
			}
			break;
			/************
			*	This function has 2 entry points. 1 is the mainline FC opcode.
			*	In this event there are several bytes that setup the type of
			*	strings that will follow. Then there is the continue entry
			*	point which is immediate - e.g location 0.
			*************/
		case 0xFC:	/* Packed String Array A.K.A. SST Shared String Table...UNI */
			if ((count > 7)||(cont_opcode == 1)) /* Skip the 1st 8 locations they are bs */
			{
/*				if ((count == 0)&&(data == 0)&&(buflast))	*/
				if ((count == 0)&&(cont_opcode == 1)&&(buflast))
				{
/*					fprintf(poutfile, "Adjusting...\n"); */
/*					fprintf(poutfile, "I:%04X BL:%04X\n", bufidx, buflast); */
					cont_str_array = 1;
					cont_grbit = data;
					return;
				}

				working_buffer[bufidx] = data;
				bufidx++;

				if((cont_str_array)&&(grbit & 0x01)&& !(cont_grbit & 0x01))
				{	/* ASCII -> unicode */
					working_buffer[bufidx] = 0;
					bufidx++;
				}

				if (buflast == 0)	/* Header processor */
				{
					if (bufidx == 0x03)  /* After 3 locations we have length */
					{				    /* and type of chars... */
						cch = getShort(&working_buffer[0]);
						grbit = working_buffer[2];

						if (grbit < 0x04)	/* Normal run */
						{
							nonascii = 0;
							bufidx = 0;
							crun = 0;
							extrst = 0;
							buflast = cch << (grbit & 0x01);

							/* special case for empty strings */
							if (!cch && !buflast)
								add_str_array(0, (U8 *)0, 0, 0, 0);
							else
								memset(working_buffer, 0, WBUFF_SIZE);
						}
					}
					else if (bufidx == 0x05)
					{
						if ((grbit & 0x0C) == 0x08)	/* Rich string only */
						{
							nonascii = 0;
							bufidx = 0;
							crun = getShort(&working_buffer[3]);
							extrst = 0;
							buflast = (cch << (grbit & 0x01)) + (crun*4);
/*							fprintf(poutfile, "rtbuflast:%X cch%X grbit:%X extrst:%X crun:%X last:%X\n",
										buflast, cch, grbit, extrst, crun, last);
							fprintf(poutfile, "%02X %02X %02X %02X %02X %02X\n",
							working_buffer[0], working_buffer[1], working_buffer[2],
							working_buffer[3], working_buffer[4], working_buffer[5]); */
							memset(working_buffer, 0, WBUFF_SIZE);
						}
					}
					else if (bufidx == 0x07)
					{
						if ((grbit & 0x0C) == 0x04)	/* Extended string only */
						{
							nonascii = 0;
							bufidx = 0;
							crun = 0;
							extrst = getLong(&working_buffer[3]);
							buflast = (cch << (grbit & 0x01)) + extrst;
/*							fprintf(poutfile, "esbuflast:%X cch%X grbit:%X extrst:%X last:%X\n",
										buflast, cch, grbit, extrst, last);
							fprintf(poutfile, "%02X %02X %02X %02X %02X %02X\n",
							working_buffer[0], working_buffer[1], working_buffer[2],
							working_buffer[3], working_buffer[4], working_buffer[5]); */
							memset(working_buffer, 0, WBUFF_SIZE);
						}
					}
					else if (bufidx == 0x09)
					{
						if ((grbit & 0x0C) == 0x0C)
						{
							/* Rich String + Extended String **/
							nonascii = 0;
							bufidx = 0;
							crun = getShort(&working_buffer[3]);
							extrst = getLong(&working_buffer[5]);
							buflast = (cch << (grbit & 0x01)) + extrst + (crun*4);
/*							fprintf(poutfile, "xrtbuflast:%X cch%X grbit:%X extrst:%X crun:%X last:%X\n",
										buflast, cch, grbit, extrst, crun, last);
							fprintf(poutfile, "%02X %02X %02X %02X %02X %02X\n",
							working_buffer[0], working_buffer[1], working_buffer[2],
							working_buffer[3], working_buffer[4], working_buffer[5]); */
							memset(working_buffer, 0, WBUFF_SIZE);
						}
					}
/*					fprintf(poutfile, "*%02X ", data); */
				}
				else	/* payload processor */
				{
/*					if (cont_opcode == 1)
						fprintf(poutfile, " %02X", data); */
					if (data > 127)
						nonascii = 1;
					if (bufidx == buflast)
					{
						U8 uni;
						U16 len = (U16)(cch << (grbit & 0x01));
/*						int i;	*/

						if (grbit & 01)
						{
							uni = 2;
							UnicodeStrings = 2;
						}
						else
							uni = nonascii;
						working_buffer[bufidx] = 0;
/*  						fprintf(stderr,":buflast-"); */
/*                                                  { int i; */
/*  						for (i=0; i<buflast; i++) */
/*                                                    putchar(working_buffer[i]); */
/*  						fprintf(stderr,"\nNext String:%d\n", next_string); */
/*                                                  } */

						if (crun)
							add_str_array(uni, working_buffer, len, working_buffer+len, crun);
						else
							add_str_array(uni, working_buffer, len, 0, 0);
						if (uni > UnicodeStrings)	/* Try to "upgrade" charset */
							UnicodeStrings = uni;
						bufidx = 0;
						buflast = 0;
						cch = 0;
						cont_str_array = 0;
						memset(working_buffer, 0, WBUFF_SIZE);
					}
				}
			}
			break;
		case 0xFD:	/* String Array Index A.K.A. LABELSST */
			working_buffer[count] = data;
			if (count == (last - 1))
			{
				U32 i;
				U16 r, c, f;

				/* This is byte reversed... */
				r = getShort(&working_buffer[0]);
				c = getShort(&working_buffer[2]);
				f = getShort(&working_buffer[4]);
				i = getLong(&working_buffer[6]);
				if (i < next_string)
				{
/*					fprintf(poutfile, "String used:%d\n", (int)i); */
					if (str_array[i])
					{
						if (str_array[i]->str)
							add_wb_array(
								r, c, f, opcode,
								str_array[i]->uni, str_array[i]->str,
								str_array[i]->len, str_array[i]->crun_cnt, str_array[i]->fmt_run);
					}
					else	/* Error, so just set it empty */
						add_wb_array( r, c, f, opcode,
								(U16)0, (U8 *)"String Table Error", 18, 0, 0);
				}
				else
					MaxStringsExceeded = 1;
			}
			break;
		case 0x31:	/* Font */
			working_buffer[bufidx++] = data;
			if (bufidx > 14) /* Address 14 has length in unicode chars */
			{
				if ((file_version == EXCEL95)&&(bufidx == last))
				{	/* Microsoft doesn't stick to their documentation. Excel 97 is supposed
					   to be 0x0231...but its not. Have to use file_version to separate them. */
					unsigned int i;
					U16 size, attr, c_idx, b, su;
					U8 u;

					size = getShort(&working_buffer[0]);
					attr = getShort(&working_buffer[2]);
					c_idx = getShort(&working_buffer[4]);
					b = getShort(&working_buffer[6]);
					su = getShort(&working_buffer[8]);
					u = working_buffer[10];
					buflast = working_buffer[14];
					for (i=0; i<buflast; i++)
						working_buffer[i] = working_buffer[i+15];

					working_buffer[buflast] = 0;
/*					fprintf(poutfile, "S:%04X A:%04X C:%04X B:%04X SU:%04X U:%02X\n",
							size, attr,c_idx,b,su,u);
					fprintf(poutfile, "f:%s\n", working_buffer); */
					add_font(size, attr, c_idx, b, su, u, 0, &working_buffer[0], 0);
				}
				else if ((file_version == EXCEL97)&&(bufidx == last))
				{	/* Microsoft doesn't stick to their documentation. Excel 97 is supposed
					   to be 0x0231...but its not. Have to use file_version to separate them. */
					unsigned int i;
					U16 len;
					U16 size, attr, c_idx, b, su;
					U8 u, uni=0;

					size = getShort(&working_buffer[0]);
					attr = getShort(&working_buffer[2]);
					c_idx = getShort(&working_buffer[4]);
					b = getShort(&working_buffer[6]);
					su = getShort(&working_buffer[8]);
					u = working_buffer[10];
					buflast = working_buffer[14];

					for (i=0; i<(buflast-2); i++)
					{	/* This looks at the 2nd byte to see if its unicode... */
						if (working_buffer[(i<<1)+17] != 0)
							uni = 2;
					}

					if (uni == 2)
						len = (U16)(buflast<<1);
					else
						len = (U16)buflast;

					if (uni == 0)
					{	
						for (i=0; i<len; i++)
						{
							working_buffer[i] = working_buffer[(i<<1)+16];
							if ((working_buffer[i] > 0x0080U) && (uni == 0))
								uni = 1;
						}
					}
					else
					{
						for (i=0; i<len; i++)
							working_buffer[i] = working_buffer[i+16];
					}

					working_buffer[len] = 0;

/*					fprintf(poutfile, "S:%04X A:%04X C:%04X B:%04X SU:%04X U:%02X\n",
							size, attr,c_idx,b,su,u);
					fprintf(poutfile, "BL:%d L:%d Uni:%d\n", buflast, len, uni);
					fprintf(poutfile, "%X %X %X %X\n", working_buffer[15], working_buffer[16], working_buffer[17], working_buffer[18]);
					fprintf(poutfile, "f:%s\n", working_buffer); */
					add_font(size, attr, c_idx, b, su, u, uni, &working_buffer[0], len);
				}
			}
			break;
		case 0x14:	/* Header */
			break;
		case 0x15:	/* Footer */
			break;
		case 0x06:	/* Formula */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{
				U16 r, c, f;
				U8 calc_val[64];

				r = getShort(&working_buffer[0]);
				c = getShort(&working_buffer[2]);
				f = getShort(&working_buffer[4]);
				if ((working_buffer[12] == 0xFF)&&(working_buffer[13] == 0xFF))
				{	/* Formula evaluates to Bool, Err, or String */
					if (working_buffer[6] == 1)				/* Boolean */
					{
						decodeBoolErr(working_buffer[8], 0, (char *)calc_val);
						opcode = 0x0105;
					}
					else if (working_buffer[6] == 2)			/* Err */
					{
						decodeBoolErr(working_buffer[8], 1, (char *)calc_val);
						opcode = 0x0105;
					}
					else
					{							/* String UNI */
						str_formula_row = r;
						str_formula_col = c;
						str_formula_format = f;
						break;
					}
				}
				else
				{	/* Otherwise...this is a number */
					F64 n;
					getDouble(&working_buffer[6], &n);
					sprintf((char *)calc_val, "%.15g", n);
					opcode = 0x0103;	/* To fix up OutputCellFormatted... */
				}
				add_wb_array(r, c, f, opcode, (U16)0, calc_val, (U16)strlen((char *)calc_val), 0, 0);
			}
			break;
		case 0x07:	/* String Formula Results */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{
				U8 *str;
				U8 uni = 0;
				U16 len = getShort(&working_buffer[0]);
				if (len > (last-3))
					len = (U16)(last-3);
				if (file_version == EXCEL97)
				{
					/* Check for unicode. Terminate the buffer at 2x len
						since unicode is 2bytes per char. Then see if
						strlen is short...upperbyte is usually 0 in
						western chararcter sets. */
					int t = len << 1;
					if ((t+3) < WBUFF_SIZE)
						working_buffer[t+3] = 0;
					else
						working_buffer[len+3] = 0;
					if ((len+3) < last)
					{
						uni = 2;
						len = (U16)t;
					}
					str = &working_buffer[3];
				}
				else if (file_version == EXCEL95)
				{
					str = &working_buffer[2];
					working_buffer[len+2] = 0;
				}
				else
				{
					if (OutputXML)
						str = (U8*)"<NotImplemented/>String Formula";
					else
//						str = (U8*)"***String Formula";
						str = (U8*)"String Formula";
					len = (U16)strlen((char*)str);
					NotImplemented++;
				}
				add_wb_array(str_formula_row, str_formula_col, str_formula_format, opcode, uni, str, len, 0, 0);
			}
			break;
		case 0x5C:	/* Author's name A.K.A. WRITEACCESS */
			working_buffer[bufidx++] = data;
			if ((bufidx == last)&&(author.str == 0))
			{
				if (file_version == EXCEL97)
				{
					author.len = getShort(&working_buffer[0]);
					if ((int)working_buffer[2] & 0x01)
					{
						author.len *= (U16)2;
						author.uni = 2;
					}
					else
						author.uni = 0;
					if (author.len > (last-2))
						author.len = (U16)(last-2);
					author.str = (U8 *)malloc(author.len+1);
					if (author.str)
					{
						memcpy(author.str, &working_buffer[3], author.len);
						author.str[author.len] = 0;
					}
				}
				else if (file_version == EXCEL95)
				{
					author.len = working_buffer[0];
					author.str = (U8 *)malloc(author.len+1);
					if (author.str)
					{
						memcpy(author.str, &working_buffer[1], author.len);
						author.str[author.len] = 0;
					}
					author.uni = 0;
				}
			}
			break;
		case 0x08:	/* Row Data */
			/* There's actually some other interesting things
			   here that we're not collecting. For now, we'll
			   Just get the dimensions of the sheet. */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{
				/* question...what is the actual limit?
					This can go as high as 64K. Is this really OK? */
				U16 i, r, fc, lc, d, xf;
				r = getShort(&working_buffer[0]);
				fc = getShort(&working_buffer[2]);
				lc = (U16)(getShort(&working_buffer[4]) - (U16)1);
				d = getShort(&working_buffer[12]);
				xf = getShort(&working_buffer[14]);

				if (ws_array[sheet_count] == 0)
					if (ws_init(sheet_count))
						return;

				if (r > ws_array[sheet_count]->biggest_row)
				{
					if (r < ws_array[sheet_count]->max_rows)
						ws_array[sheet_count]->biggest_row = r;
					else
					{	/* Resize the array... */
						if (MaxRowExceeded == 0)
						{
							int diff = (r/ROWS_INCR) + 1;
							if(resize_c_array(ws_array[sheet_count], ROWS_INCR*diff, 0))
							{
								ws_array[sheet_count]->biggest_row = ws_array[sheet_count]->max_rows - 1;
								MaxRowExceeded = 1;
								return;
							}
							else
								ws_array[sheet_count]->biggest_row = r;
						}
						else
							return;
					}
				}

				if (lc > ws_array[sheet_count]->biggest_col)
				{
					if (lc < ws_array[sheet_count]->max_cols)
						ws_array[sheet_count]->biggest_col = lc;
					else
					{	/* Resize array... */
						if (MaxColExceeded == 0)
						{
							int diff = (lc/COLS_INCR) + 1;
							if (resize_c_array(ws_array[sheet_count], 0, (U16)(COLS_INCR*diff)))
							{
								ws_array[sheet_count]->biggest_col = (S16)(ws_array[sheet_count]->max_cols - 1);
								MaxColExceeded = 1;
								lc = ws_array[sheet_count]->max_cols;
							}
							else
								ws_array[sheet_count]->biggest_col = lc;
						}
						else
							lc = ws_array[sheet_count]->max_cols;
					}
				}
				if ((fc < ws_array[sheet_count]->max_cols)&&(d & 0x0080))	/* fGhostDirty flag */
				{
					for (i=fc; i<lc; i++)
					{	/* Set the default attr... */
						update_cell_xf(r, i, xf);
					}
				}
			}
			break;
		case 0x22:	/* 1904 Flag - MacIntosh Dates or PC Dates */
			working_buffer[bufidx++] = data;
			if (bufidx == 2)
				DatesR1904 = getShort(&working_buffer[0]);
			break;
		case 0x085:	/* BoundSheet */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{	/* This is based on Office 97 info... */
				if ((working_buffer[4] & 0x0F) == 0)
				{	/* Worksheet as opposed to chart, etc */
					U16 len;
					U8 uni=0;
					if (file_version == EXCEL97)
					{
						len = (U16)working_buffer[6];		/* FIXME: Check this !!! Was GetShort */
						if (working_buffer[7] & 0x01)
						{
							uni = 2;
							len = (U16)(len<<1);
						}
						if (len != 0)
						{
							working_buffer[8 + len + 1] = 0;
							add_ws_title(uni, &working_buffer[8], len);
						}
					}
					else
					{
						len = working_buffer[6];
						if (len != 0)
						{
							working_buffer[7 + len + 1] = 0;
							add_ws_title(uni, &working_buffer[7], len);
						}
					}
				}
			}
			break;
		case 0x7E:	/* RK Number */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{	/* This is based on Office 97 info... */
				U16 r, c, f;
				U32 t;
				S32 n, n2;		/* Must be signed long !!! */
				F64 d;
				char temp[64];

				r = getShort(&working_buffer[0]);
				c = getShort(&working_buffer[2]);
				f = getShort(&working_buffer[4]);
				n = getLong(&working_buffer[6]);
				t = n & 0x03;
				n2 = n>>2;
				switch (t)
				{
					case 0:
						RKtoDouble(n2, &d);
						sprintf(temp, "%.15g", d);
						break;
					case 1:
						RKtoDouble(n2, &d);
						sprintf(temp, "%.15g", d / 100.0);
						break;
					case 2:
						sprintf(temp, "%ld", (S32)n2);
						break;
					default:
						d = (F64) n2;
						sprintf(temp, "%.15g", d / 100.0 );
						break;
				}
				add_wb_array(r, c, f, opcode, (U16)0, (U8 *)temp, (U16)strlen(temp), 0, 0);
			}
			break;
		case 0xBC:		/* Shared Formula's */
/*			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{
				int fr, lr, fc, lc, i, j;
				fr = getShort(&working_buffer[0]);
				lr = getShort(&working_buffer[2]);
				fc = working_buffer[4];
				lc = working_buffer[5];
				for (i=fr; i<=lr; i++)
				{
					for (j=fc; j<=lc; j++)
						add_wb_array(i, j, (U16)0, opcode, 0, "***SHRFORMULA", 13);
				}
				NotImplemented = 1;
			}	*/
			break;
		case 0x21:		/* Arrays */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{
				U16 fr, lr, fc, lc, i, j;
				fr = getShort(&working_buffer[0]);
				lr = getShort(&working_buffer[2]);
				fc = working_buffer[4];
				lc = working_buffer[5];
				for (i=fr; i<=lr; i++)
				{
					for (j=fc; j<=lc; j++)
//						add_wb_array(i, j, (U16)0, opcode, 0, (U8 *)"***Array", 8, 0, 0);
						add_wb_array(i, j, (U16)0, opcode, 0, (U8 *)"Array", 8, 0, 0);
				}
				NotImplemented = 1;
			}
			break;
		case 0xBD:		/* MULRK */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{
				U16 r, fc, lc;
				int i;
				r = getShort(&working_buffer[0]);
				fc = getShort(&working_buffer[2]);
				lc = getShort(&working_buffer[last-2]);
				for (i=0; i<=(lc-fc); i++)
				{
					U32 t;
					S32 n2, n;		/* Must be signed long !!! */
					U16 f;
					F64 d;
					char temp[64];

					f = getShort(&working_buffer[4+(i*6)]);
					n = getLong(&working_buffer[6+(i*6)]);
					t = n & 0x03;
					n2 = n>>2;
					switch (t)
					{
						case 0:
							RKtoDouble(n2, &d);
							sprintf(temp, "%.15g", d);
							break;
						case 1:
							RKtoDouble(n2, &d);
							sprintf(temp, "%.15g", d / 100.0);
							break;
						case 2:
							sprintf(temp, " %ld", (S32)n2);
							break;
						default:
							d = (F64) n2;
							sprintf(temp, "%.15g",  d / 100.0 );
							break;
					}
/*					fprintf(poutfile, "%08X %02X %s  %d  %d\n", n2, t, temp, r, fc+i); */
					add_wb_array(r, fc+i, f, opcode, (U16)0, (U8 *)temp, (U16)strlen(temp), 0, 0);
				}
			}
			break;
		case 0xBE:		/* MULBLANK */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{
				U16 r, fc, lc, j, f;
				r = getShort(&working_buffer[0]);
				fc = getShort(&working_buffer[2]);
				lc = getShort(&working_buffer[last-2]);
				for (j=0; j<=(lc-fc); j++)
				{	/* This just stores format strings... */
					f = getShort(&working_buffer[4+(j*2)]);
					add_wb_array(r, fc+j, f, opcode, (U16)0, (U8 *)0, (U16)0, 0, 0);
				}
			}
			break;
		case 0x18:		/* Name UNI */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{
				char *ptr;
				working_buffer[bufidx] = 0;
				ptr = (char *)strstr((char *)&working_buffer[15], "LastUpdate");
				if (ptr)
				{
					ptr += 13;
					lastUpdated = (char *)malloc(strlen(ptr)+1);
					if (lastUpdated)
						strcpy(lastUpdated, ptr);
				}
				else
				{
					ptr = (char *)strstr((char *)&working_buffer[15], "Title");
					if (ptr)
					{
						ptr += 8;
						title = (char *)malloc(strlen(ptr)+1);
						if (title)
							strcpy(title, ptr);
					}
				}
			}
			break;
		case 0xE0:		/* Extended format */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{
				U16 fnt_idx;
				U16 fmt_idx;
				U16 gen;
				U16 align;
				U16 indent;
				U16 b_style;
				U16 b_l_color;
				U32  b_t_color;
				U16 cell_color;

				fnt_idx = getShort(&working_buffer[0]);
				fmt_idx = getShort(&working_buffer[2]);
				gen = getShort(&working_buffer[4]);
				align  = getShort(&working_buffer[6]);
				indent  = getShort(&working_buffer[8]);
				b_style  = getShort(&working_buffer[10]);
				if (file_version == EXCEL95)
				{
					b_l_color  = 0;
					b_t_color = 0;
					cell_color  = (U16)(getShort(&working_buffer[12]) & (U16)0x1FFF);
				}
				else	/* Excel 97 + */
				{
					b_l_color  = getShort(&working_buffer[12]);
					b_t_color = getLong(&working_buffer[14]);
					cell_color  = getShort(&working_buffer[18]);
				}

				/* fprintf(poutfile, "XF:%02X FG:%02X BG:%02X\n", next_xf, cell_color&0x007F, (cell_color&0x1F80)>>7); */
				/* fprintf(poutfile, "XF:%02X M:%02X b_t:%04X<br>\n", next_xf, indent, b_t_color); */
				add_xf_array(fnt_idx, fmt_idx, gen, align, indent, b_style,
					b_l_color, b_t_color, cell_color);
			}
			break;
		case 0xE5:		/* CELL MERGE INSTRUCTIONS */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{
				U16 num, fr, lr, fc, lc, i, j, k;
				ws_array[sheet_count]->spanned = 1;
				num = getShort(&working_buffer[0]);
				if (ws_array[sheet_count]->c_array == 0)
					return;

				for (i=0; i<num; i++)
				{
					cell *c;
					fr = getShort(&working_buffer[2+(i*8)]);
					lr = getShort(&working_buffer[4+(i*8)]);
					fc = getShort(&working_buffer[6+(i*8)]);
					lc = getShort(&working_buffer[8+(i*8)]);
					if (sheet_count < (int)max_worksheets)
					{
						if (ws_array[sheet_count] == 0)
						{
							if (ws_init(sheet_count))
								return;
						}
						if (ws_array[sheet_count]->c_array)
						{
							if ((fr > lr)||(fr > ws_array[sheet_count]->biggest_row)||(lr > ws_array[sheet_count]->biggest_row))
								lr = (U16)ws_array[sheet_count]->biggest_row;
							if ((fc > lc)||(fc > ws_array[sheet_count]->biggest_col)||(lc > ws_array[sheet_count]->biggest_col))
								lc = ws_array[sheet_count]->biggest_col;
							for(j=fr; j<=lr; j++)
							{	/* For each row */
								for(k=fc; k<=lc; k++)
								{	/* for each column */
									c = ws_array[sheet_count]->c_array[(j*ws_array[sheet_count]->max_cols)+k];
									if (c != 0)
									{
										c->spanned = 1;
										c->rowspan = 0;
										if (k == fc)
											c->colspan = (U16)((lc-fc)+1);
										else
											c->colspan = 0;
									}
/*									else
									{	/ Need to create one...
										fprintf(poutfile, "Bad One at:%d %d %d<br>\n", sheet_count, j, k);
									} */
								}
							}
						}
						/* Now reset the first one... */
/*						fprintf(poutfile, "s:%d fr:%d fc:%d lr:%d lc:%d<br>\n", sheet_count, fr, fc, lr, lc); */
						c = ws_array[sheet_count]->c_array[(fr*ws_array[sheet_count]->max_cols)+fc];
						if (c != 0)
						{
							c->spanned = 0;
							c->rowspan = (U16)(lr-fr);
							c->colspan = (U16)(lc-fc);
							if (c->rowspan)
								c->rowspan++;
							if (c->colspan)
								c->colspan++;
						}
					}
				}
			}
			break;
		case 0xB8:	/* Hyperlink */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{	/* This is based on Office 97 info... */
				U16 r, c, uni_type, off;
				U32 len;

				r = getShort(&working_buffer[0]);
				c = getShort(&working_buffer[4]);
				if (working_buffer[32] == 0xE0)
				{	/* Unicode format */
					len = getLong(&working_buffer[48]);
					off = 52;
					uni_type = 2;
				}
				else
				{	/* Ascii format */
					len = getLong(&working_buffer[50]);
					off = 54;
					uni_type = 0;
				}
				if (len > (U32)(bufidx - off))
				{	/* correct misidentified links */
					if (uni_type == 0)
					{
						off = 36;
						uni_type = 2;
						len = getLong(&working_buffer[32]) * 2;
					}
					else
						len = bufidx - off;	/* safety measure to make sure it doen't blow up */
				}
				update_cell_hyperlink(r, c, &working_buffer[off], len, uni_type);
			}
			break;
		case 0x92:	/* Color Palette */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{	/* This is based on Office 97 info... */
				int i;
				U8 red, green, blue;
				U16 cnt = getShort(&working_buffer[0]);
				numCustomColors = cnt;
				customColors = (U8 **)calloc(cnt+1,  sizeof(char *));
				for (i=0; i<cnt; i++)
				{
					char color_string[8];
					red = (unsigned char)working_buffer[(4*i)+2];
					green = (unsigned char)working_buffer[(4*i)+3];
					blue = (unsigned char)working_buffer[(4*i)+4];
					/* fprintf(poutfile, "%02X%02X%02X\n", (int)red, (int)green, (int)blue); */
					sprintf(color_string, "%02X%02X%02X", (int)red, (int)green, (int)blue);
					customColors[i] = (U8 *)strdup(color_string);
				}
			}
			break;
		case 0x42:	/* CodePage */
			working_buffer[bufidx++] = data;
			if (bufidx == last)
			{
				CodePage = getShort(&working_buffer[0]);
				if (CodePage == 1200)
					CodePage = 0;	/* Unicode inside, old behavior is OK */
			}
			break;
		default:
			break;
	}
}

static U16 getShort(U8 *ptr)
{
	if (ptr == 0)
		return (U16)0;

	return (U16)((*(ptr+1)<<8)+*ptr);
}

/*! This is used in the RK number, so signedness counts */
static S32 getLong(U8 *ptr)
{
	if (ptr == 0)
		return (S32)0;

	return (S32)(*(ptr+3)<<24)+(*(ptr+2)<<16)+(*(ptr+1)<<8)+*ptr;
}

#ifndef WORDS_BIGENDIAN				/* Defined in <config.h> */
/*! Little Endian - 0x86 family */
static void getDouble(U8 *ptr, F64 *d)
{
	size_t i;
	F64 dd;
	U8 *t = (U8 *)&dd;

	for (i=0; i<sizeof(F64); i++)
		*(t+i) = *(ptr+i);

	*d = (F64)dd;
}
#else
/*! Big Endian version - UltraSparc's, etc. */
static void getDouble (U8 *ptr, F64 *d)
{
	size_t i;
	F64 dd;
	U8 *t = (U8 *)&dd;

	for (i=0; i<sizeof(F64); i++)
		*(t+i) = *(ptr+sizeof(F64) - 1 - i);

	*d = (F64)dd;
}
#endif

#ifndef WORDS_BIGENDIAN				/*! Defined in <config.h> */
/*! Little Endian - 0x86 family */
static void RKtoDouble(S32 n, F64 *d)
{
	*((S32 *)d) = 0;
	*((S32 *)d+1) = n << 2;
}
#else
/*! Big Endian version - UltraSparc's, etc. */
static void RKtoDouble(S32 n, F64 *d)
{
	F64 dd;
	S32 tmp;
	U8 *ptr = (U8 *)&n;

	tmp = (*(ptr+0)<<24)+(*(ptr+1)<<16)+(*(ptr+2)<<8)+(*(ptr+3));

	*((S32 *)&dd+1) = 0;
	*((S32 *)&dd) = tmp << 2;

	*d = dd;
}
#endif

/*! returns 1 on error, 0 on success */
static int ws_init(int i)
{
	U32 j;
	U16 k;
	if (i >= (int)max_worksheets)
		return 1;

	ws_array[i] = (work_sheet *)malloc(sizeof(work_sheet));
	if (ws_array[i])
	{
		ws_array[i]->spanned = 0;
		ws_array[i]->first_row = 0;
		ws_array[i]->biggest_row = -1;
		ws_array[i]->max_rows = ROWS_INCR;
		ws_array[i]->first_col = 0;
		ws_array[i]->biggest_col = -1;
		ws_array[i]->max_cols = COLS_INCR;
		uni_string_clear(&ws_array[i]->ws_title);
		ws_array[i]->c_array = (cell **)malloc(ROWS_INCR*COLS_INCR*sizeof(cell *));
		if (ws_array[i]->c_array == 0)
			return 1;
		for (j=0; j<ROWS_INCR; j++)
			for (k=0; k<COLS_INCR; k++)
				ws_array[i]->c_array[(j*ws_array[i]->max_cols)+k] = 0;
	}
	else
		return 1;

	return 0;
}

/*! returns 1 on error, 0 on success */
static int add_more_worksheet_ptrs(void)
{
	work_sheet **tws_array;
	int	pages;

	if (MaxWorksheetsExceeded)
		return 1;

	if (sheet_count > (int)max_worksheets)
		pages = (((sheet_count - max_worksheets)/WORKSHEETS_INCR) + 1) * WORKSHEETS_INCR;
	else
		pages = WORKSHEETS_INCR;
	tws_array = (work_sheet **)realloc(ws_array,
				(max_worksheets + pages) * sizeof(work_sheet *));

	if (tws_array == NULL)
	{
		MaxWorksheetsExceeded = 1;
		return 1;
	}
	else
	{	/* Next init the array... */
		unsigned int i;

		ws_array = tws_array;

		for (i=max_worksheets; i<max_worksheets+pages; i++)
			ws_array[i] = 0;

		max_worksheets += pages;
		last_sheet = max_worksheets - 1;
	}
	return 0;
}

static int resize_c_array(work_sheet *ws, U32 new_rows, U16 new_cols)
{
	cell **tc_array;
	if (ws == 0)
		return 1;
	if (ws->c_array == 0)
		return 1;

	tc_array = (cell **)malloc((ws->max_rows+new_rows)*(ws->max_cols+new_cols)*sizeof(cell *));

	if (tc_array == NULL)
		return 1;
	else
	{
		U32 j;
		U16 k;

		memset(tc_array, 0, (ws->max_rows+new_rows)*(ws->max_cols+new_cols)*sizeof(cell *));
		for (j=0; j<(ws->max_rows); j++)
		{
			for (k=0; k<ws->max_cols; k++)
				tc_array[(j*(ws->max_cols+new_cols))+k] = ws->c_array[(j*ws->max_cols)+k];
		}
		ws->max_cols += new_cols;
		ws->max_rows += new_rows;
		free(ws->c_array);
		ws->c_array = tc_array;
	}
	return 0;
}

static void add_wb_array(U16 r, U16 c, U16 xf, U16 type, U8 uni,
					U8 *str, U16 len, U16 crun_cnt, U8 *fmt_run)
{
	work_sheet *ws;

	if ((sheet_count < 0)||(r > HARD_MAX_ROWS)||(c > HARD_MAX_COLS))
		return;
	if (sheet_count >= (int)max_worksheets)
	{
		if (add_more_worksheet_ptrs())
			return;
	}
	if (ws_array[sheet_count] == 0)
	{
		if (ws_init(sheet_count))
			return;
	}
	ws = ws_array[sheet_count];
	if (r >= ws->max_rows)
	{
		if (MaxRowExceeded)
			return;
		else
		{
			int diff = ((r-ws->max_rows)/ROWS_INCR)+1;
			if(resize_c_array(ws, ROWS_INCR*diff, 0))
			{
				MaxRowExceeded = 1;
				return;
			}
		}
	}
	if (c >= ws->max_cols)
	{
		if (MaxColExceeded)
			return;
		else
		{
			U16 diff = (U16)(((c-ws->max_cols)/COLS_INCR)+1);
			if(resize_c_array(ws, 0, (U16)(COLS_INCR*diff)))
			{
				MaxColExceeded = 1;
				return;
			}
		}
	}
	if (ws->c_array[(r*ws->max_cols)+c] == 0)
	{
		if (r > ws_array[sheet_count]->biggest_row)
			ws_array[sheet_count]->biggest_row = r;
		if (c > ws_array[sheet_count]->biggest_col)
			ws_array[sheet_count]->biggest_col = c;
		ws->c_array[(r*ws->max_cols)+c] = (cell *)malloc(sizeof(cell));
		if (ws->c_array[(r*ws->max_cols)+c])
		{
			if (str)
			{
				ws->c_array[(r*ws->max_cols)+c]->ustr.str = (U8 *)malloc(len+1);
				if (ws->c_array[(r*ws->max_cols)+c]->ustr.str)
				{
					memcpy(ws->c_array[(r*ws->max_cols)+c]->ustr.str, str, len);
					ws->c_array[(r*ws->max_cols)+c]->ustr.str[len] = 0;
				}
				ws->c_array[(r*ws->max_cols)+c]->ustr.uni = uni;
				ws->c_array[(r*ws->max_cols)+c]->ustr.len = len;
				if (fmt_run && crun_cnt)
				{
					int rlen = crun_cnt*4;

					ws->c_array[(r*ws->max_cols)+c]->ustr.fmt_run = malloc(rlen);
					if (ws->c_array[(r*ws->max_cols)+c]->ustr.fmt_run)
					{
						memcpy(ws->c_array[(r*ws->max_cols)+c]->ustr.fmt_run, fmt_run, rlen);
						ws->c_array[(r*ws->max_cols)+c]->ustr.crun_cnt = crun_cnt;
					}
					else
						ws->c_array[(r*ws->max_cols)+c]->ustr.crun_cnt = 0;
				}
				else
				{
					ws->c_array[(r*ws->max_cols)+c]->ustr.fmt_run = 0;
					ws->c_array[(r*ws->max_cols)+c]->ustr.crun_cnt = 0;
				}
			}
			else
				uni_string_clear(&ws->c_array[(r*ws->max_cols)+c]->ustr);

			ws->c_array[(r*ws->max_cols)+c]->xfmt = xf;
			ws->c_array[(r*ws->max_cols)+c]->type = type;
			ws->c_array[(r*ws->max_cols)+c]->spanned = 0;
			ws->c_array[(r*ws->max_cols)+c]->rowspan = 0;
			ws->c_array[(r*ws->max_cols)+c]->colspan = 0;
			uni_string_clear(&ws->c_array[(r*ws->max_cols)+c]->h_link);
		}
	}
	else	/* Default attributes already copied */
	{
		if (r > ws_array[sheet_count]->biggest_row)
			ws_array[sheet_count]->biggest_row = r;
		if (c > ws_array[sheet_count]->biggest_col)
			ws_array[sheet_count]->biggest_col = c;
		if (str)
		{	/* Check if a place holder is there and free it */
			if (ws->c_array[(r*ws->max_cols)+c]->ustr.str != 0)
				free(ws->c_array[(r*ws->max_cols)+c]->ustr.str);

			ws->c_array[(r*ws->max_cols)+c]->ustr.str = (U8 *)malloc(len+1);
			if (ws->c_array[(r*ws->max_cols)+c]->ustr.str)
			{
				memcpy(ws->c_array[(r*ws->max_cols)+c]->ustr.str, str, len);
				ws->c_array[(r*ws->max_cols)+c]->ustr.str[len] = 0;
			}
			ws->c_array[(r*ws->max_cols)+c]->ustr.len = len;
			ws->c_array[(r*ws->max_cols)+c]->ustr.uni = uni;
			if (fmt_run && crun_cnt)
			{
				int rlen = crun_cnt*4;

				ws->c_array[(r*ws->max_cols)+c]->ustr.fmt_run = malloc(rlen);
				if (ws->c_array[(r*ws->max_cols)+c]->ustr.fmt_run)
				{
					memcpy(ws->c_array[(r*ws->max_cols)+c]->ustr.fmt_run, fmt_run, rlen);
					ws->c_array[(r*ws->max_cols)+c]->ustr.crun_cnt = crun_cnt;
				}
				else
					ws->c_array[(r*ws->max_cols)+c]->ustr.crun_cnt = 0;
			}
			else
			{
				ws->c_array[(r*ws->max_cols)+c]->ustr.fmt_run = 0;
				ws->c_array[(r*ws->max_cols)+c]->ustr.crun_cnt = 0;
			}
		}
		else
		{
			if (ws->c_array[(r*ws->max_cols)+c]->ustr.str == 0)
			{
				ws->c_array[(r*ws->max_cols)+c]->ustr.len = 0;
				ws->c_array[(r*ws->max_cols)+c]->ustr.uni = 0;
				ws->c_array[(r*ws->max_cols)+c]->ustr.fmt_run = 0;
				ws->c_array[(r*ws->max_cols)+c]->ustr.crun_cnt = 0;
			}
		}
		ws->c_array[(r*ws->max_cols)+c]->xfmt = xf;
		ws->c_array[(r*ws->max_cols)+c]->type = type;
		ws->c_array[(r*ws->max_cols)+c]->spanned = 0;
		ws->c_array[(r*ws->max_cols)+c]->rowspan = 0;
		ws->c_array[(r*ws->max_cols)+c]->colspan = 0;
	}
}

static void update_cell_xf(U16 r, U16 c, U16 xf)
{
	work_sheet *ws;

	if ((sheet_count < 0)||(r > HARD_MAX_ROWS)||(c > HARD_MAX_COLS))
		return;
	if (sheet_count >= (int)max_worksheets)
	{
		if (add_more_worksheet_ptrs())
			return;
	}
	if (ws_array[sheet_count] == 0)
	{
		if (ws_init(sheet_count))
			return;
	}
	if (r >= ws_array[sheet_count]->max_rows)
	{
		if (MaxRowExceeded)
			return;
		else
		{
			int diff = ((r-ws_array[sheet_count]->max_rows)/ROWS_INCR)+1;
			if(resize_c_array(ws_array[sheet_count], ROWS_INCR*diff, 0))
			{
				MaxRowExceeded = 1;
				return;
			}
		}
	}
	if (c >= ws_array[sheet_count]->max_cols)
	{
		if (MaxColExceeded)
			return;
		else
		{
			int diff = ((c-ws_array[sheet_count]->max_cols)/COLS_INCR)+1;
			if (resize_c_array(ws_array[sheet_count], 0, (U16)(COLS_INCR*diff)))
			{
				MaxColExceeded = 1;
				return;
			}
		}
	}

	ws = ws_array[sheet_count];
	if (ws->c_array[(r*ws->max_cols)+c] == 0)
	{
		ws->c_array[(r*ws->max_cols)+c] = (cell *)malloc(sizeof(cell));
		if (ws->c_array[(r*ws->max_cols)+c])
		{
			uni_string_clear(&ws->c_array[(r*ws->max_cols)+c]->ustr);
			ws->c_array[(r*ws->max_cols)+c]->xfmt = xf;
			ws->c_array[(r*ws->max_cols)+c]->type = 1;	/* This is the Blank Cell type */

			if (r > ws_array[sheet_count]->biggest_row)
				ws_array[sheet_count]->biggest_row = r;
			if (c > ws_array[sheet_count]->biggest_col)
				ws_array[sheet_count]->biggest_col = c;
			ws->c_array[(r*ws->max_cols)+c]->spanned = 0;
			ws->c_array[(r*ws->max_cols)+c]->rowspan = 0;
			ws->c_array[(r*ws->max_cols)+c]->colspan = 0;
			uni_string_clear(&ws->c_array[(r*ws->max_cols)+c]->h_link);
		}
	}
/*	else
	{
		fprintf(poutfile, "R:%02X C:%02X XF:%02X is:%02X\n",
			r, c, xf, ws->c_array[r][c]->xfmt);
	} */
}

static void update_cell_hyperlink(U16 r, U16 c, U8 *hyperlink, int len, U16 uni)
{
	work_sheet *ws;

	if (sheet_count < 0)	/* Used to do a "0 <" check on r & c */
		return;
	if (sheet_count >= (int)max_worksheets)
	{
		if (add_more_worksheet_ptrs())
			return;
	}
	if (ws_array[sheet_count] == 0)
	{
		if (ws_init(sheet_count))
			return;
	}
	if (r >= ws_array[sheet_count]->max_rows)
	{
		if (MaxRowExceeded)
			return;
		else
		{
			int diff = ((r-ws_array[sheet_count]->max_rows)/ROWS_INCR)+1;
			if(resize_c_array(ws_array[sheet_count], ROWS_INCR*diff, 0))
			{
				MaxRowExceeded = 1;
				return;
			}
		}
	}
	if (c >= ws_array[sheet_count]->max_cols)
	{
		if (MaxColExceeded)
			return;
		else
		{
			int diff = ((c-ws_array[sheet_count]->max_cols)/COLS_INCR)+1;
			if(resize_c_array(ws_array[sheet_count], 0, (U16)(COLS_INCR*diff)))
			{
				MaxColExceeded = 1;
				return;
			}
		}
	}

	ws = ws_array[sheet_count];
	if (ws->c_array[(r*ws->max_cols)+c] == 0)
	{	/* should not get here, but just in case */
		return;
	}
	if (ws->c_array[(r*ws->max_cols)+c]->h_link.str == 0)
	{
		ws->c_array[(r*ws->max_cols)+c]->h_link.str = (U8 *)malloc(len);
		if (ws->c_array[(r*ws->max_cols)+c]->h_link.str)
			memcpy(ws->c_array[(r*ws->max_cols)+c]->h_link.str, hyperlink, len);
		ws->c_array[(r*ws->max_cols)+c]->h_link.uni = uni;
		if (len)
		{
			if (uni < 2)
				ws->c_array[(r*ws->max_cols)+c]->h_link.len = (U16)(len-1);
			else
				ws->c_array[(r*ws->max_cols)+c]->h_link.len = (U16)(len-2);
		}
	}
/*	else
	{
		fprintf(poutfile, "R:%02X C:%02X XF:%02X is:%s\n",
			r, c, xf, ws->c_array[r][c]->h_link.str);
	} */
}

static void add_str_array(U8 uni, U8 *str, U16 len, U8 *fmt_run, U8 crun_cnt)
{

	if ((str == 0)||(len == 0))
	{
		next_string++; /* increment for empty strings, too */
		return;
	}
	if (next_string >= max_strings)
	{
		uni_string **tstr_array;
		size_t new_size = (max_strings + STRINGS_INCR) * sizeof(uni_string *);

		tstr_array = (uni_string **)realloc(str_array, new_size);

		if (tstr_array == NULL)
		{
			MaxStringsExceeded = 1;
/*			fprintf(stderr, "%s: cannot allocate %d bytes for string storage %d: %s",
			    PRGNAME, new_size, errno, strerror(errno));	*/
			return;
		}
		else
		{
			unsigned long i;

			str_array = tstr_array;

			/* Clear the new string slots */
			for (i=max_strings; i<(max_strings + STRINGS_INCR); i++)
				str_array[i] = 0;
			
			max_strings += STRINGS_INCR;
		}
	}
	
	if (str_array[next_string] == 0)
	{
		str_array[next_string] = (uni_string *)malloc(sizeof(uni_string));
		if (str_array[next_string])
		{
			str_array[next_string]->str = (U8 *)malloc(len+1);
			if (str_array[next_string]->str)
			{
				memcpy(str_array[next_string]->str, str, len);
				str_array[next_string]->str[len] = 0;
				str_array[next_string]->len = len;
				str_array[next_string]->uni = uni;
				if (fmt_run && crun_cnt)
				{
					int rlen = crun_cnt*4;

					str_array[next_string]->fmt_run = malloc(rlen);
					if (str_array[next_string]->fmt_run)
					{
						memcpy(str_array[next_string]->fmt_run, fmt_run, rlen);
						str_array[next_string]->crun_cnt = crun_cnt;
					}
					else
						str_array[next_string]->crun_cnt = 0;
				}
				else
				{
					str_array[next_string]->fmt_run = 0;
					str_array[next_string]->crun_cnt = 0;
				}
			}
		}
	}
	next_string++;
}

static void add_font(U16 size, U16 attr, U16 c_idx, U16 bold, U16 super, U8 underline,
	U16 uni, U8 *n, U16 len)
{
	if (n == 0)
		return;
	if (next_font >= max_fonts)
	{
		font_attr **tfont_array;
		fnt_cnt *tf_cnt;
		tfont_array = (font_attr **)realloc(font_array, (max_fonts * FONTS_INCR) * sizeof(font_attr *));
		tf_cnt = (fnt_cnt *)realloc(f_cnt, (max_fonts * FONTS_INCR) * sizeof(fnt_cnt));
		
		if ((tf_cnt == NULL) || (tfont_array == NULL))
		{
			MaxFontsExceeded = 1;
			return;
		}
		else
		{	/* Next init the array... */
			unsigned int i;
			
			font_array = tfont_array;
			f_cnt = tf_cnt;
			
			for (i=max_fonts; i<max_fonts+FONTS_INCR; i++)
			{
				font_array[i] = 0;
				f_cnt[i].name = 0;
			}
			max_fonts += FONTS_INCR;
		}
	}

	if (font_array[next_font] == 0)
	{
		font_array[next_font] = (font_attr *)malloc(sizeof(font_attr));
		if (font_array[next_font])
		{
			font_array[next_font]->name.str = (U8 *)malloc(len+1);
			if (font_array[next_font]->name.str)
			{
				font_array[next_font]->attr = attr;
				font_array[next_font]->c_idx = c_idx;
				font_array[next_font]->bold = bold;
				font_array[next_font]->super = super;
				font_array[next_font]->underline = underline;
				font_array[next_font]->name.uni = uni;
				memcpy(font_array[next_font]->name.str, n, len);
				font_array[next_font]->name.str[len] = 0;	
				font_array[next_font]->name.len = len;
				font_array[next_font]->name.fmt_run = 0;
				font_array[next_font]->name.crun_cnt = 0;

				/* We will "pre-digest" the font size.. */
				if (size >= 0x02D0)		/* 36 pts */
					font_array[next_font]->size = 7;
				else if (size >= 0x01E0)	/* 24 pts */
					font_array[next_font]->size = 6;
				else if (size >= 0x0168)	/* 18 pts */
					font_array[next_font]->size = 5;
				else if (size >= 0x00F0)	/* 12 pts */
					font_array[next_font]->size = 4;
				else if (size >= 0x00C8)	/* 10 pts */
					font_array[next_font]->size = 3;
				else if (size >= 0x00A0)	/* 8 pts */
					font_array[next_font]->size = 2;
				else
					font_array[next_font]->size = 1;
			}
		}
	}
	next_font++;
	if (next_font == 4)		/* Per the doc's - number 4 doesn't exist. */
		next_font++;
}

static void add_ws_title(U16 uni, U8 *n, U16 len)
{
	if (n == 0)
		return;

	if (next_ws_title >= max_worksheets)
	{
		if (add_more_worksheet_ptrs())
			return;
	}

	if (ws_array[next_ws_title] == 0)
	{
		if (ws_init(next_ws_title))
			return;
	}
	if (ws_array[next_ws_title]->ws_title.str == 0)
	{
		ws_array[next_ws_title]->ws_title.str = (U8 *)malloc(len+1);
		if (ws_array[next_ws_title]->ws_title.str)
		{
			ws_array[next_ws_title]->ws_title.uni = uni;
			memcpy(ws_array[next_ws_title]->ws_title.str, n, len);
			ws_array[next_ws_title]->ws_title.str[len] = 0;
			ws_array[next_ws_title]->ws_title.len = len;
			ws_array[next_ws_title]->ws_title.crun_cnt = 0;
			ws_array[next_ws_title]->ws_title.fmt_run = 0;
		}
	}
	next_ws_title++;
}

static void add_xf_array(U16 fnt_idx, U16 fmt_idx, U16 gen, U16 align,
	U16 indent, U16 b_style, U16 b_l_color, U32  b_t_color, U16 cell_color)
{
	if (next_xf >= max_xformats)
	{
		xf_attr **txf_array;
		
		txf_array = (xf_attr **)realloc(xf_array, (max_xformats + XFORMATS_INCR) * sizeof(xf_attr *));
		if (txf_array == NULL)
		{
			MaxXFExceeded = 1;
			return;
		}
		else
		{
			unsigned int i;
			
			xf_array = txf_array;
			
			for (i=max_xformats; i<(max_xformats + XFORMATS_INCR); i++)
				xf_array[i] = 0;
			
			max_xformats += XFORMATS_INCR;
		}
	}

	if (xf_array[next_xf] == 0)
	{
		xf_array[next_xf] = (xf_attr *)malloc(sizeof(xf_attr));
		if (xf_array[next_xf])
		{
			xf_array[next_xf]->fnt_idx = fnt_idx;
			xf_array[next_xf]->fmt_idx = fmt_idx;
			xf_array[next_xf]->gen = gen;
			xf_array[next_xf]->align = align;
			xf_array[next_xf]->indent = indent;
			xf_array[next_xf]->b_style = b_style;
			xf_array[next_xf]->b_l_color = b_l_color;
			xf_array[next_xf]->b_t_color = b_t_color;
			xf_array[next_xf]->cell_color = cell_color;
		}
		next_xf++;
	}
}

static void decodeBoolErr(U16 value, U16 flag, char *str)
{
	if (str == 0)
		return;
		
	if (flag == 0)
	{
		if (value == 1)
			strcpy(str, "TRUE");
		else
			strcpy(str, "FALSE");		
	}
	else
	{
		switch(value)
		{	
			case 0x00:
				strcpy(str, "#NULL!");		
				break;
			case 0x07:
				strcpy(str, "#DIV/0!");
				break;
			case 0x0F:
				strcpy(str, "#VALUE!");
				break;
			case 0x17:
				strcpy(str, "#REF!");
				break;
			case 0x1D:
				strcpy(str, "#NAME?");
				break;
			case 0x24:
				strcpy(str, "#NUM!");
				break;
			case 0x2A:
				strcpy(str, "#N/A");
				break;
			default:
				strcpy(str, "#ERR");		
				break;
		}
	}
}

static int IsCellNumeric(cell *c)
{
	int ret_val = 0;
	
	switch (c->type & 0x00FF)
	{
		case 0x02:	/* Int */
		case 0x03:	/* Float */
	/*	case 0x06: */	/* Formula */
	/*	case 0x08: */
		case 0x7E:	/* RK */
	/*	case 0xBC: */
	/*	case 0x21: */
		case 0xBD:	/* MulRK */
			ret_val = 1;
			break;
 		default:
			break;
	}
	return ret_val;
}

/*!	\retval 0 not safe at all.
	\retval 1 extended format is OK
	\retval 2 Fonts OK */
static int IsCellSafe(cell *c)
{
	int safe = 0;
	
	if (c->xfmt < next_xf)
	{
		if (xf_array[c->xfmt])
		{
			safe = 1;
			if (xf_array[c->xfmt]->fnt_idx < next_font)
			{
				if (font_array[xf_array[c->xfmt]->fnt_idx])
					safe = 2;
			}
		}
	}
	return safe;
}

static int IsCellFormula(cell *c)
{
	if ((c->type > 0x0100)||(c->type == 0x0006))
		return 1;
	else
		return 0;
}

static void output_formatted_data(uni_string *u, U16 idx, int numeric, int formula)
{
	if ((idx < max_xformats)&&(u->str))
	{
		if ((formula_warnings)&&(formula))
		{
			if( OutputXML )
				fprintf(poutfile,  "<NotAccurate/>" );
//			else
//				fprintf(poutfile, "** ");
			notAccurate++;
		}
		if (numeric)
		{
			int year, month, date;
			long num;
			F64 dnum;
			int hr, minu, sec, msec;

/*			fprintf(poutfile, "idx:%d ", idx);	*/
			switch (idx)
			{
				case 0x00:	/* General */
					dnum = atof((char *)u->str);
					fprintf(poutfile, "%.15g", dnum);
					break;
				case 0x01:	/* Number 0 */
					dnum = atof((char *)u->str);
					fprintf(poutfile, "%.0f", dnum);
					break;
				case 0x02:	/* Number	0.00 */
					dnum = atof((char *)u->str);
					fprintf(poutfile, "%.2f", dnum);
					break;
				case 0x03:	/* Number w/comma	0,000 */
					PrintFloatComma("%.0f", 0, (F64)atof((char *)u->str));
					break;
				case 0x04:	/* Number w/comma	0,000.00 */
					PrintFloatComma("%.2f", 0, (F64)atof((char *)u->str));
					break;
				case 0x05:	/* Currency, no decimal */
					PrintFloatComma("%.0f", 1, (F64)atof((char *)u->str));
					break;
				case 0x06:	/* Currency, no decimal Red on Neg */
					PrintFloatComma("%.0f", 1, (F64)atof((char *)u->str));
					break;
				case 0x07:	/* Currency with decimal */
					PrintFloatComma("%.2f", 1, (F64)atof((char *)u->str));
					break;
				case 0x08:	/* Currency with decimal Red on Neg */
					PrintFloatComma("%.2f", 1, (F64)atof((char *)u->str));
					break;
				case 0x09:	/* Percent 0% */
//					if (Csv)
//						fprintf(poutfile, "\"");
					dnum = 100.0*atof((char *)u->str);
					fprintf(poutfile, "%.0f%%", dnum);
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x0A:	/* Percent 0.00% */
//					if (Csv)
//						fprintf(poutfile, "\"");
					dnum = 100.0*atof((char *)u->str);
					fprintf(poutfile, "%.2f%%", dnum);
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x0B:	/* Scientific 0.00+E00 */
//					if (Csv)
//						fprintf(poutfile, "\"");
					dnum = atof((char *)u->str);
					fprintf(poutfile, "%.2E", dnum);
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x0C:	/* Fraction 1 number  e.g. 1/2, 1/3 */
//					if (Csv)
//						fprintf(poutfile, "\"");
					dnum = atof((char *)u->str);
					print_as_fraction(dnum, 1);
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x0D:	/* Fraction 2 numbers  e.g. 1/50, 25/33 */
//					if (Csv)
//						fprintf(poutfile, "\"");
					dnum = atof((char *)u->str);
					print_as_fraction(dnum, 2);
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x0E:	/* Date: m-d-y */
//					if (Csv)
//						fprintf(poutfile, "\"");
					num = atol((char *)u->str);
					NumToDate(num, &year, &month, &date);
					fprintf(poutfile, "%d-%d-%02d", month, date, year);
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x0F:	/* Date: d-mmm-yy */
//					if (Csv)
//						fprintf(poutfile, "\"");
					num = atol((char *)u->str);
					NumToDate(num, &year, &month, &date);
					fprintf(poutfile, "%d-%s-%02d", date, month_abbr[month-1], year);
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x10:	/* Date: d-mmm */
//					if (Csv)
//						fprintf(poutfile, "\"");
					num = atol((char *)u->str);
					NumToDate(num, &year, &month, &date);
					fprintf(poutfile, "%d-%s", date, month_abbr[month-1]);
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x11:	/* Date: mmm-yy */
//					if (Csv)
//						fprintf(poutfile, "\"");
					num = atol((char *)u->str);
					NumToDate(num, &year, &month, &date);
					fprintf(poutfile, "%s-%02d", month_abbr[month-1], year);
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x12:	/* Time: h:mm AM/PM */
//					if (Csv)
//						fprintf(poutfile, "\"");
					FracToTime(u->str, &hr, &minu, 0, 0);
					if (hr == 0)
						fprintf(poutfile, "12:%02d AM", minu);
					else if (hr < 12)
						fprintf(poutfile, "%d:%02d AM", hr, minu);
					else if (hr == 12)
						fprintf(poutfile, "12:%02d PM", minu);
					else
						fprintf(poutfile, "%d:%02d PM", hr-12, minu);
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x13:	/* Time: h:mm:ss AM/PM */
//					if (Csv)
//						fprintf(poutfile, "\"");
					FracToTime(u->str, &hr, &minu, &sec, 0);
					if (hr == 0)
						fprintf(poutfile, "12:%02d:%02d AM", minu, sec);
					else if (hr < 12)
						fprintf(poutfile, "%d:%02d:%02d AM", hr, minu, sec);
					else if (hr == 12)
						fprintf(poutfile, "12:%02d:%02d PM", minu, sec);
					else
						fprintf(poutfile, "%d:%02d:%02d PM", hr-12, minu, sec);
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x14:	/* Time: h:mm */
//					if (Csv)
//						fprintf(poutfile, "\"");
					FracToTime(u->str, &hr, &minu, 0, 0);
					fprintf(poutfile, "%d:%02d", hr, minu);
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x15:	/* Time: h:mm:ss */
//					if (Csv)
//						fprintf(poutfile, "\"");
					FracToTime(u->str, &hr, &minu, &sec, 0);
					if ((hr != 0)||(minu != 0)||(sec != 0))
						fprintf(poutfile, "%d:%02d:%02d", hr, minu, sec);
					else
					{
						if (Ascii)
//							putchar(' ');
							fputc(' ', poutfile);
						else
							fprintf(poutfile, OutputXML ? "" : "&nbsp;");
					}
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x25:	/* Number with comma, no decimal */
//					if (Csv)
//						fprintf(poutfile, "\"");
					PrintFloatComma("%.0f", 0, (F64)atof((char *)u->str));
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x26:	/* Number with comma, no decimal, red on negative */
//					if (Csv)
//						fprintf(poutfile, "\"");
					PrintFloatComma("%.0f", 0, (F64)atof((char *)u->str));
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x27:	/* Number with comma & decimal */
//					if (Csv)
//						fprintf(poutfile, "\"");
					PrintFloatComma("%.2f", 0, (F64)atof((char *)u->str));
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x28:	/* Number with comma & decimal, red on negative */
//					if (Csv)
//						fprintf(poutfile, "\"");
					PrintFloatComma("%.2f", 0, (F64)atof((char *)u->str));
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x29:	/* Number with comma, no decimal */
//					if (Csv)
//						fprintf(poutfile, "\"");
					PrintFloatComma("%.2f", 0, (F64)atof((char *)u->str));
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x2a:	/* Currency, no decimal */
//					if (Csv)
//						fprintf(poutfile, "\"");
					PrintFloatComma("%.0f", 1, (F64)atof((char *)u->str));
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x2B:	/* Number w/comma & decimal	0,000.00 */
//					if (Csv)
//						fprintf(poutfile, "\"");
					PrintFloatComma("%.2f", 0, (F64)atof((char *)u->str));
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x2C:	/* Accounting Currency	$0,000.00 */
				{
					F64 acc_val = atof((char *)u->str);
//					if (Csv)
//						fprintf(poutfile, "\"");
					if (acc_val < 0.0)
						PrintFloatComma(" (%.2f)", 1, fabs(acc_val));
					else
						PrintFloatComma(" %.2f", 1, acc_val);
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				}
				case 0x2D:	/* Time: mm:ss */
//					if (Csv)
//						fprintf(poutfile, "\"");
					FracToTime(u->str, &hr, &minu, &sec, 0);
					fprintf(poutfile, "%02d:%02d", minu, sec);
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x2E:	/* Time: [h]:mm:ss */
//					if (Csv)
//						fprintf(poutfile, "\"");
					FracToTime(u->str, &hr, &minu, &sec, 0);
					if (hr)
						fprintf(poutfile, "%d:%02d:%02d", hr, minu, sec);
					else
						fprintf(poutfile, "%02d:%02d", minu, sec);
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x2F:	/* Time: mm:ss.0 */
//					if (Csv)
//						fprintf(poutfile, "\"");
					FracToTime(u->str, &hr, &minu, &sec, &msec);
					fprintf(poutfile, "%02d:%02d.%01d", minu, sec, msec);
//					if (Csv)
//						fprintf(poutfile, "\"");
					break;
				case 0x31:	/* Text - if we are here...its a number */
					dnum = atof((char *)u->str);
					fprintf(poutfile, "%g", dnum);
					break;
				default:	/* Unsupported...but, if we are here, its a number */
					{
						char *ptr = strchr((char *)u->str, '.');
						if( OutputXML )
							fprintf(poutfile,  "<NoFormat/>" );
						dnum = atof((char *)u->str);
						if (ptr)
						{
							if (Csv)
//								fprintf(poutfile, "\"%.15g\"", dnum );
								fprintf(poutfile, "%.15g", dnum );
							else if (OutputXML)
								fprintf(poutfile, "%.15g", dnum );
							else
								fprintf(poutfile, "%.15g *", dnum );
						}
						else
						{
							if (Csv)
//								fprintf(poutfile, "\"%.0g\"", dnum );
								fprintf(poutfile, "%.0g", dnum );
							else if (OutputXML)
								fprintf(poutfile, "%.0g", dnum );
							else
								fprintf(poutfile, "%.0g *", dnum );
						}

/*						fprintf(poutfile, " F:%02X", idx); */
						NoFormat++ ;
					}
					break;
			}
		}
		else	/* Text data */
			OutputString(u);
	}
	else	/* Error handling just dump it. */
		OutputString(u);
}

static void NumToDate(long num, int *year, int *month, int *day)
{
	int t, i, y = 0;

	num = num%36525L;	/* Trim century */
	while (num > (((y%4) == 0) ? 366 : 365))
		num -= ((y++%4) == 0) ? 366 : 365;

	*year = y;
	t = num;
	if (DatesR1904)
		*year += 4;		/* Adjust for McIntosh... */
	if ((*year%4) == 0)
	{	/* Leap Year */
		for (i=0; i<12; i++)
		{
			if (t <= ldays[i])
				break;
			t -= ldays[i];
		}
	}
	else
	{
		for (i=0; i<12; i++)
		{
			if (t <= ndays[i])
				break;
			t -= ndays[i];
		}
	}
	/* Some fixups... */
	*month = 1+i;
	if (t == 0)
		t = 1;
	*day = t;
	*year = *year % 100;
}

static void FracToTime(U8 *cnum, int *hr, int *minut, int *sec, int *msec)
{
	int Hr, Min, Sec, Msec;
	F64 fnum, tHr, tMin, tSec, tMsec;

	if (msec)
		fnum = atof((char *)&cnum[0])+(0.05 / 86400.0);	/* Round off to 1/10th seconds */
	else if (sec)
		fnum = atof((char *)&cnum[0])+(0.5 / 86400.0);	/* Round off to seconds */
	else
		fnum = atof((char *)&cnum[0])+(30 / 86400.0);	/* Round off to minutes */
	tHr = 24.0 * fnum;
	Hr = (int)tHr;
	tMin = (tHr - (F64)Hr) * 60.0;
	Min = (int)tMin;
	tSec = (tMin - (F64)Min) * 60.0;
	Sec = (int)tSec;
	tMsec = (tSec - (F64)Sec) * 10.0;
	Msec = (int)tMsec;

	Hr = Hr%24;	/* Fix roll-overs */
	if (hr)
		*hr = Hr;
	if (minut)
		*minut = Min;
	if (sec)
		*sec = Sec;
	if (msec)
		*msec = Msec;
}

static void PrintFloatComma(char *fformat, int is_currency, F64 d)
{
	int len, int_len, dec_len;
	char *ptr2, buf[64];

	sprintf(buf, fformat, fabs(d));
	len = strlen(buf);
	ptr2 = strchr(buf, '.');
	if (ptr2)
	{
		int_len = ptr2 - buf;
		dec_len = len - int_len;
		if (isdigit(buf[0]) == 0)
		{
			char *ptr = &buf[0];
			while (isdigit(*ptr) == 0)
			{
				int_len--;
				ptr++;
				if (*ptr == 0)
					break;
			}
		}
	}
	else
	{
		int_len = len;
		dec_len = 0;
	}

	if (int_len > 3)
	{	/* we have to do it the hard way... */
		char rbuf[64], buf2[64];
		int neg, i, j, count=0;

		if (d < 0.0)
			neg = 1;
		else
			neg = 0;

		/* reverse the string. Its easier to work this way. */
		for (i=0, j=len-1; i<len;i++, j--)
			rbuf[i] = buf[j];
		rbuf[len] = 0;
		if (ptr2)
		{
			memcpy(buf2, rbuf, dec_len);
			i = dec_len;
			j = dec_len;
		}
		else
		{
			i = 0;
			j = 0;
		}

		for ( ;i<len;i++, j++)
		{
			buf2[j] = rbuf[i];
			count++;
			if ((count%3)==0)
			{
				if (isdigit(rbuf[i+1]))
					buf2[++j] = ',';
			}
		}
		if (neg)
			buf2[j++] = '-';
		if (is_currency)
			buf2[j++] = (char)currency_symbol;
		buf2[j] = 0;

		len = strlen(buf2);
		for (i=0, j=len-1; i<len;i++, j--)
			buf[i] = buf2[j];
		buf[len] = 0;
//		if (Csv)
//			fprintf(poutfile, "\"%s\"", buf);
//		else
			fprintf(poutfile, "%s", buf);
	}
	else	/* too short for commas, just output it as is. */
	{
		if (is_currency)
//			putchar((char)currency_symbol);
			fputc((char)currency_symbol, poutfile);
		fprintf(poutfile, fformat, d);
	}
}

static void print_as_fraction(F64 d, int digits)
{
	F64 i, j, w, r, closest=1.0, lim = 9.0;
	int ci=1, cj=1;

	/* Take care of negative sign */
	if (digits == 2)
		lim = 99.0;
	if (d < 0)
//		putchar('-');
		fputc('-', poutfile);

	/** take care of whole number part */
	w = fabs(d);
	if (w >= 1.0)
	{
		int n = (int)w;
		fprintf(poutfile, "%d ", n);
		r = w - (F64)n;
	}
	else
		r = w;

	/* Get closest fraction - brute force */
	for (j=lim; j>0.0; j--)
	{
		for (i=lim; i>=0.0; i--)
		{
			if ( fabs((i/j)-r) <= closest)
			{
				closest = fabs((i/j)-r);
				ci = (int)i;
				cj = (int)j;
			}
		}
	}

	/* Done, print it... */
	if (ci != 0)
		fprintf(poutfile, "%d/%d", ci, cj);
}

static void trim_sheet_edges(unsigned int sheet)
{
	cell *ce;
	int not_done = 1;
	S32 r;
	U16 c;

	if ((sheet >= max_worksheets)||(ws_array[sheet] == 0)||
			(trim_edges == 0)||(ws_array[sheet]->spanned))
		return;
	if (ws_array[sheet]->c_array == 0)
		return;
	if (	(ws_array[sheet]->biggest_row == -1) ||
			(ws_array[sheet]->biggest_col == -1) )
		return;

	/* First find top edge */
	for (r=ws_array[sheet]->first_row; r<=ws_array[sheet]->biggest_row; r++)
	{
		for (c=ws_array[sheet]->first_col; c<=ws_array[sheet]->biggest_col; c++)
		{	/* This stuff happens for each cell... */
			ce = ws_array[sheet]->c_array[(r*ws_array[sheet]->max_cols)+c];
			if (ce)
			{
				if (ce->ustr.str)
				{
					if (!null_string(ce->ustr.str))
					{
						not_done = 0;
						break;
					}
				}
			}
		}
		if (!not_done)
			break;
	}
	if (not_done)
		ws_array[sheet]->first_row = ws_array[sheet]->biggest_row;
	else
		ws_array[sheet]->first_row = r;

	/* Second Find bottom edge */
	not_done = 1;
	for (r=ws_array[sheet]->biggest_row; r>(S32)ws_array[sheet]->first_row; r--)
	{
		for (c=ws_array[sheet]->first_col; c<=ws_array[sheet]->biggest_col; c++)
		{	/* This stuff happens for each cell... */
			ce = ws_array[sheet]->c_array[(r*ws_array[sheet]->max_cols)+c];
			if (ce)
			{
				if (ce->ustr.str)
				{
					if (!null_string(ce->ustr.str))
					{
						not_done = 0;
						break;
					}
				}
			}
		}
		if (!not_done)
			break;
	}
	ws_array[sheet]->biggest_row = r;

	/* Third find left edge */
	not_done = 1;
	for (c=ws_array[sheet]->first_col; c<=ws_array[sheet]->biggest_col; c++)
	{
		for (r=ws_array[sheet]->first_row; r<=ws_array[sheet]->biggest_row; r++)
		{	/* This stuff happens for each cell... */
			ce = ws_array[sheet]->c_array[(r*ws_array[sheet]->max_cols)+c];
			if (ce)
			{
				if (ce->ustr.str)
				{
					if (!null_string(ce->ustr.str))
					{
						not_done = 0;
						break;
					}
				}
			}
		}
		if (!not_done)
			break;
	}
	if (not_done)
		ws_array[sheet]->first_col = ws_array[sheet]->biggest_col;
	else
		ws_array[sheet]->first_col = c;

	/* Last, find right edge */
	not_done = 1;
	for (c=ws_array[sheet]->biggest_col; c>ws_array[sheet]->first_col; c--)
	{
		for (r=ws_array[sheet]->first_row; r<=ws_array[sheet]->biggest_row; r++)
		{	/* This stuff happens for each cell... */
			ce = ws_array[sheet]->c_array[(r*ws_array[sheet]->max_cols)+c];
			if (ce)
			{
				if (ce->ustr.str)
				{
					if (!null_string(ce->ustr.str))
					{
						not_done = 0;
						break;
					}
				}
			}
		}
		if (!not_done)
			break;
	}
	ws_array[sheet]->biggest_col = c;
}

/***************
*! Figures out the best font & alignment for the current table.
* Also sets the default_font and default_alignment.
****************/
static void update_default_font(unsigned int sheet)
{
	cell *ce;
	int r, c, f;

	if ((sheet >= max_worksheets)||(ws_array[sheet] == 0))
		return;
	if (ws_array[sheet]->c_array == 0)
		return;

	/* Clear the book-keeping info... */
	for (r=0; r<FONTS_INCR; r++)
	{
		f_cnt[r].cnt = 0;
		if (f_cnt[r].name)
		{
			if (f_cnt[r].name->str)
				free(f_cnt[r].name->str);
			free(f_cnt[r].name);
			f_cnt[r].name = 0;
		}
	}
	if (default_font.str)
		free(default_font.str);
	for (r=0; r<7; r++)
		fnt_size_cnt[r] = 0;

	/* Now check each cell to see what its using. */
	for (r=ws_array[sheet]->first_row; r<=ws_array[sheet]->biggest_row; r++)
	{
		for (c=ws_array[sheet]->first_col; c<=ws_array[sheet]->biggest_col; c++)
		{	/* This stuff happens for each cell... */
			ce = ws_array[sheet]->c_array[(r*ws_array[sheet]->max_cols)+c];
			if (ce)
			{
				if ((ce->xfmt < next_xf)&&(ce->ustr.str))
				{
					if (strcmp((char *)ce->ustr.str, "&nbsp;"))
					{
						if (ce->xfmt < next_xf)
						{
							if (xf_array[ce->xfmt])
							{
								unsigned int fn = xf_array[ce->xfmt]->fnt_idx;
								if (fn < next_font)
								{
									if (font_array[fn])
									{
										if (font_array[fn]->name.str)
										{
											/* Here's where we check & increment count... */
											incr_f_cnt(&(font_array[fn]->name));
											if ((font_array[fn]->size < 8)&&(font_array[fn]->size))
												fnt_size_cnt[font_array[fn]->size-1]++;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	f = get_default_font();
	if (f == -1)
	{
		default_font.str = (U8 *)malloc(6);
		if (default_font.str)
		{
			strcpy((char *)default_font.str, "Arial");
			default_font.uni = 0;
			default_font.len = 5;
		}
	}
	else
	{
		default_font.str = (U8 *)malloc(f_cnt[f].name->len+1);
		if (default_font.str)
		{
			memcpy(default_font.str, f_cnt[f].name->str, f_cnt[f].name->len);
			default_font.str[f_cnt[f].name->len] = 0;
			default_font.uni = f_cnt[f].name->uni;
			default_font.len = f_cnt[f].name->len;
		}
	}

	/* Find the font size with the most counts...
	   Just re-using variables, c - max cnt, f = position of max cnt */
	c = 0;
	f = 3;
	for (r=0; r<7; r++)
	{
		if (fnt_size_cnt[r] > c)
		{
			c = fnt_size_cnt[r];
			f = r;
		}
	}
	if (fnt_size_cnt[2] == c)		/* favor size 3... */
		default_fontsize = 3;
	else
		default_fontsize = f+1;

	for (r=0; r<FONTS_INCR; r++)
	{
		if (f_cnt[r].name != 0)
		{
			if (f_cnt[r].name->str != 0)
				free(f_cnt[r].name->str);
			free(f_cnt[r].name);
			f_cnt[r].name= 0;
		}
	}
}

static void incr_f_cnt(uni_string *name)
{
	int i;

	if ((name == 0)||(name->str == 0)||(name->str[0] == 0))
		return;

	for (i=0; i<FONTS_INCR; i++)
	{
		if (f_cnt[i].name)
		{
			if (uni_string_comp(name, f_cnt[i].name) == 0)
				f_cnt[i].cnt++;
		}
		else
		{
			f_cnt[i].name = (uni_string *)malloc(sizeof(uni_string));
			if (f_cnt[i].name)
			{
				f_cnt[i].name->str = (U8 *)malloc(name->len+1);
				if (f_cnt[i].name->str)
				{
					memcpy(f_cnt[i].name->str, name->str, name->len);
					f_cnt[i].name->str[name->len] = 0;
					f_cnt[i].name->uni = name->uni;
					f_cnt[i].name->len = name->len;
					f_cnt[i].cnt = 1;
					break;
				}
			}
		}
	}
}

static int get_default_font(void)
{
	int i, m = -1;

	for (i=0; i<FONTS_INCR; i++)
	{
		if (f_cnt[i].name)
		{
			if (f_cnt[i].name->str)
			{
				if (f_cnt[i].cnt > m)
					m = i;
			}
		}
	}
	return m;
}

static void update_default_alignment(unsigned int sheet, int row)
{
	int i, left = 0, center = 0, right = 0;
	cell *c;

	if ((sheet >= max_worksheets)||(ws_array[sheet] == 0))
		return;
	if (ws_array[sheet]->c_array == 0)
		return;

	for (i=ws_array[sheet]->first_col; i<=ws_array[sheet]->biggest_col; i++)
	{	/* This stuff happens for each cell... */
		c = ws_array[sheet]->c_array[(row*ws_array[sheet]->max_cols)+i];
		if (c)
		{
			int numeric = IsCellNumeric(c);
			if (c->xfmt == 0)
			{	/* Unknown format... */
				left++;
			}
			else
			{	/* Biff 8 stuff... */
				if ((c->xfmt < next_xf)&&(c->ustr.str))
				{
					if (strcmp((char *)c->ustr.str, "&nbsp;"))
					{
						if (xf_array[c->xfmt])
						{
							switch(xf_array[c->xfmt]->align & 0x0007)
							{	/* Override default table alignment when needed */
								case 2:
								case 6:		/* Center across selection */
									center++;
									break;
								case 0:		/* General alignment */
									if (numeric)						/* Numbers */
										right++;
									else if ((c->type & 0x00FF) == 0x05)	/* Boolean */
										center++;
									else
										left++;
									break;
								case 3:
									right++;
									break;
								case 1:			/* fall through... */
								default:
									left++;
									break;
							}
						}
					}
				}
			}
		}
	}
	if ((center == 0)&&(left == 0)&&(right == 0))
		default_alignment = "";
	else if ((center >= left)&&(center >= right))		/* Favor center since its the longest word */
		default_alignment = "center";
	else if ((right >= center)&&(right >= left))
		default_alignment = "right";			/* Favor right since its second longest */
	else
		default_alignment = "left";
}

static int null_string(U8 *str)
{	/* FIXME: This function may not be unicode safe */
	U8 *ptr;
	if ((str == NULL)||(*str == 0))
		return 1;

	ptr = str;
	while (*ptr != 0)
	{
		if (*ptr++ != ' ')
			return 0;
	}
	return 1;
}

static void OutputString(uni_string *u)
{
	unsigned int i;

	if (u == 0)
		return;

	if (u->uni < 2)
	{
		if (null_string(u->str))
		{
			if (Ascii == 0)
				fprintf(poutfile, OutputXML ? "" : "&nbsp;");
			else if (!Csv)
				fprintf(poutfile, " ");
		}
		else
		{
			if (Ascii)	/* If Ascii output requested, simply output the string */
			{	/* These are broken up for performance */
				if (Csv)
				{
					for (i=0; i<u->len; i++)
					{
//						if (u->str[i] == 0x22)
//							fprintf(poutfile, "\"\"");
//						else
//							putchar(u->str[i]);
						switch (u->str[i])
						{
//							case 0x22:
//								fprintf(poutfile, "\\\"");
//								break;

//							case 0x2C:
//								fprintf(poutfile, "\\,");
//								break;

							default:
//								putchar(u->str[i]);
								fputc(u->str[i], poutfile);
								break;
						}
					}
				}
				else
				{
					for (i=0; i<u->len; i++)
//						putchar(u->str[i]);
						fputc(u->str[i], poutfile);
				}
				return;
			}
		}
	}
	else
	{
		if (u->len == 0)
		{
			if (Ascii)
				fprintf(poutfile, " ");
			else
				fprintf(poutfile, OutputXML ? "" : "&nbsp;");
		}
		else
		{
			if (u->len == 2)
			{
				if (memcmp(u->str, "& ", 2) == 0)
//					fprintf(poutfile, "&#8230;");
					fprintf(poutfile, "...");
				else
				{
					for (i=0; i<u->len; i+=2)
						print_utf8(getShort(&(u->str[i])));
				}
			}
			else
			{
				for (i=0; i<u->len; i+=2)
					print_utf8(getShort(&(u->str[i])));
			}
		}
	}
}

static void OutputCharCorrected(U8 c)
{
	if (MultiByte && (c & 0x80)) 
	{
//		putchar(c);
		fputc(c, poutfile);
		return;
	}
	switch (c)
	{	/* Special char handlers here... */
		case 0x3C:
			fprintf(poutfile, "&lt;");
			break;
		case 0x3E:
			fprintf(poutfile, "&gt;");
			break;
		case 0x26:
			fprintf(poutfile, "&amp;");
			break;
		case 0x22:
			fprintf(poutfile, "&quot;");
			break;
		/* Also need to cover 128-159 since MS uses this area... */
		case 0x80:		/* Euro Symbol */
			fprintf(poutfile, "&#8364;");
			break;
		case 0x82:		/* baseline single quote */
			fprintf(poutfile, "&#8218;");
			break;
		case 0x83:		/* florin */
			fprintf(poutfile, "&#402;");
			break;
		case 0x84:		/* baseline double quote */
			fprintf(poutfile, "&#8222;");
			break;
		case 0x85:		/* ellipsis */
			fprintf(poutfile, "&#8230;");
			break;
		case 0x86:		/* dagger */
		    fprintf(poutfile, "&#8224;");
		    break;
		case 0x87:		/* double dagger */
		    fprintf(poutfile, "&#8225;");
		    break;
		case 0x88:		/* circumflex accent */
		    fprintf(poutfile, "&#710;");
		    break;
		case 0x89:		/* permile */
		    fprintf(poutfile, "&#8240;");
		    break;
		case 0x8A:		/* S Hacek */
		    fprintf(poutfile, "&#352;");
		    break;
		case 0x8B:		/* left single guillemet */
		    fprintf(poutfile, "&#8249;");
		    break;
		case 0x8C:		/* OE ligature */
		    fprintf(poutfile, "&#338;");
		    break;
		case 0x8E:		/*  #LATIN CAPITAL LETTER Z WITH CARON */
			fprintf(poutfile, "&#381;");
			break;
		case 0x91:		/* left single quote ? */
		    fprintf(poutfile, "&#8216;");
		    break;
		case 0x92:		/* right single quote ? */
		    fprintf(poutfile, "&#8217;");
		    break;
		case 0x93:		/* left double quote */
		    fprintf(poutfile, "&#8220;");
		    break;
		case 0x94:		/* right double quote */
		    fprintf(poutfile, "&#8221;");
		    break;
		case 0x95:		/* bullet */
		    fprintf(poutfile, "&#8226;");
		    break;
		case 0x96:		/* endash */
		    fprintf(poutfile, "&#8211;");
		    break;
		case 0x97:		/* emdash */
		    fprintf(poutfile, "&#8212;");
		    break;
		case 0x98:		/* tilde accent */
		    fprintf(poutfile, "&#732;");
		    break;
		case 0x99:		/* trademark ligature */
		    fprintf(poutfile, "&#8482;");
		    break;
		case 0x9A:		/* s Haceks Hacek */
		    fprintf(poutfile, "&#353;");
		    break;
		case 0x9B:		/* right single guillemet */
		    fprintf(poutfile, "&#8250;");
		    break;
		case 0x9C:		/* oe ligature */
		    fprintf(poutfile, "&#339;");
		    break;
		case 0x9F:		/* Y Dieresis */
		    fprintf(poutfile, "&#376;");
		    break;
		default:
//			putchar(c);
			fputc(c, poutfile);
			break;
	}
}

static void update_crun_info(U16 *loc, U16 *fmt_idx, U16 crun_cnt, U8 *fmt_run)
{
	U16 tloc, tfmt_idx;
	U16 offset = (U16)(crun_cnt*4);

	tloc = getShort(&fmt_run[offset]);
	tfmt_idx = getShort(&fmt_run[offset+2]);
	*loc = tloc;
	*fmt_idx = tfmt_idx;
}

static void put_utf8(U16 c)
{
//	putchar((int)0x0080 | (c & 0x003F));
	fputc((int)0x0080 | (c & 0x003F), poutfile);
}

static void print_utf8(U16 c)
{
	if (c < 0x80)
		OutputCharCorrected(c);
	else if (c < 0x800)
	{
//		putchar(0xC0 | (c >> 6));
		fputc(0xC0 | (c >> 6), poutfile);
		put_utf8(c);
	}
	else
	{
//		putchar(0xE0 | (c >> 12));
		fputc(0xE0 | (c >> 12), poutfile);
		put_utf8((U16)(c >> 6));
		put_utf8(c);
	}
}

static void uni_string_clear(uni_string *str)
{
	if (str == 0)
		return;

	str->str = 0;
	str->uni = 0;
	str->len = 0;
	str->fmt_run = 0;
	str->crun_cnt = 0;
}

static int uni_string_comp(uni_string *s1, uni_string *s2)
{
	if ((s1 == 0)||(s2 == 0))
		return -1;
	if ((s1->str == 0)||(s2->str == 0))
		return -1;

	if ((s1->uni == s2->uni) && (s1->len == s2->len))
		return memcmp(s1->str, s2->str, s1->len);
	else
		return -1;
}

