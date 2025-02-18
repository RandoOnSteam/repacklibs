# repackaged libs
Single-file header-only libs written by other people that are public domain and modified by RandoOnSteam <randoonsteam@gmail.com>.

* [miniz.h](miniz.h) - The original public domain minizip modified for ancient compilers and libc removal. Also adds a light wrapper you can use ("Zip" for C++ and "ZIP" for C).

* [stb_textedit.h](stb_textedit.h) - The text control guts with some modifications:

	1. Some small modifications to allow setting width type (STB_TEXTEDIT_WIDTHTYPE) as it is normally a float which isn't good on old CPUs without a FPU and a few more type safety fixes.
	
	2. Some fixes to line by line calculations
	
	3. Abstractions and fixes to newline handling so it can handle CR and CRLF
	
	4. Abstraction to character type used
	
Here's a list of defines and functions you'll need to make before including the header

```c
#define STB_TEXTEDIT_KEYTYPE size_t
#define STB_TEXTEDIT_CHARTYPE WSMUTSTRATTRCHAR
#define STB_TEXTEDIT_POSITIONTYPE size_t
#define STB_TEXTEDIT_WIDTHTYPE size_t

STB_TEXTEDIT_WIDTHTYPE getwidth(WSMUTSTRATTR* str,
	STB_TEXTEDIT_POSITIONTYPE n, STB_TEXTEDIT_POSITIONTYPE i);
void layoutrow(struct StbTexteditRowStruct* row,
	WSMUTSTRATTR* str, STB_TEXTEDIT_POSITIONTYPE n);

#define KEY_LEFT VK_LEFT
#define KEY_RIGHT VK_RIGHT
#define KEY_UP VK_UP
#define KEY_DOWN VK_DOWN
#define KEY_PRIOR VK_PRIOR /*pgup*/
#define KEY_NEXT VK_NEXT /*pgdwn*/
#define KEY_HOME VK_HOME
#define KEY_END VK_END
#define KEY_DELETE VK_DELETE
#define KEY_BACK VK_BACK
#define KEY_SHIFT VK_SHIFT
#define KEY_CONTROL VK_CONTROL
#define KEY_INSERT VK_INSERT

#define STB_TEXTEDIT_memmove WSMemoryMove
#define STB_TEXTEDIT_UNDOSTATECOUNT (4096)
#define STB_TEXTEDIT_UNDOCHARCOUNT (256*1024)
#define STB_TEXTEDIT_STRING WSMUTSTRATTR
#define STB_TEXTEDIT_STRINGLEN(obj) WSMutStrAttr_GetLength(obj)
#define STB_TEXTEDIT_LAYOUTROW(r,obj,n) layoutrow(r,obj,n)
#define STB_TEXTEDIT_GETWIDTH(obj,n,i) getwidth(obj,n,i)
#define STB_TEXTEDIT_GETWIDTH_NEWLINE 0.0f
#define STB_TEXTEDIT_KEYTOTEXT(k, obj, pch) (KeyToAttrChar(k, obj, pch))
#define STB_TEXTEDIT_GETCHAR(obj,i) WSMutStrAttr_GetRawChar(obj, i)
#define STB_TEXTEDIT_IS_NEWLINE(ac) \
	(WSMutStrAttrGetCharInline(ac) == '\n' \
		|| WSMutStrAttrGetCharInline(ac) == '\r')
#define STB_TEXTEDIT_DELETECHARS(obj,i,n) WSMutStrAttr_EraseRangeGUI(obj,i,n)
#define STB_TEXTEDIT_INSERTCHARS(obj,i,c,n) \
	WSMutStrAttr_InsertCharsWithLengthGUI(obj,i,c,n), 1
#define STB_TEXTEDIT_K_SHIFT  256
#define STB_TEXTEDIT_K_CONTROL 512
#define STB_TEXTEDIT_K_LEFT KEY_LEFT
#define STB_TEXTEDIT_K_RIGHT KEY_RIGHT
#define STB_TEXTEDIT_K_UP KEY_UP
#define STB_TEXTEDIT_K_DOWN KEY_DOWN
#define STB_TEXTEDIT_K_PGUP KEY_PRIOR
#define STB_TEXTEDIT_K_PGDOWN KEY_NEXT
#define STB_TEXTEDIT_K_LINESTART KEY_HOME
#define STB_TEXTEDIT_K_LINEEND KEY_END
#define STB_TEXTEDIT_K_TEXTSTART \
	(STB_TEXTEDIT_K_LINESTART | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_TEXTEND \
	(STB_TEXTEDIT_K_LINEEND | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_DELETE KEY_DELETE
#define STB_TEXTEDIT_K_BACKSPACE KEY_BACK
#define STB_TEXTEDIT_K_UNDO ( 'Z' | STB_TEXTEDIT_K_CONTROL )
#define STB_TEXTEDIT_K_REDO ( 'Y' | STB_TEXTEDIT_K_CONTROL )
#define STB_TEXTEDIT_K_INSERT KEY_INSERT
#define STB_TEXTEDIT_IS_SPACE(ch) \
	( CharIsSpace(WSMutStrAttrGetCharInline(ch)) \
		&& WSMutStrAttrGetCharInline(ch) != '\t' )
#define STB_TEXTEDIT_K_WORDLEFT ( KEY_LEFT | STB_TEXTEDIT_K_CONTROL )
#define STB_TEXTEDIT_K_WORDRIGHT ( KEY_RIGHT | STB_TEXTEDIT_K_CONTROL )
```

and here's a version of getwidth() and layoutrow() that will handle
all types of newlines:

```c
#define tab_width 4
STB_TEXTEDIT_WIDTHTYPE getwidth( WSMUTSTRATTR* str,
	STB_TEXTEDIT_POSITIONTYPE n, STB_TEXTEDIT_POSITIONTYPE i )
{
	const WSMUTSTRATTRCHAR* s = WSMutStrAttr_GetCString(str) + n;
	const WSMUTSTRATTRCHAR* sorigin = WSMutStrAttr_GetCString(str);
	const WSMUTSTRATTRCHAR* end = s + i;
	size_t x = 0;
	while(s < end)
	{
		if(WSMutStrAttrGetChar(s) == '\t')
		{
			size_t ofs = tab_width - ( x % tab_width );
			x += ( ofs - 1 );
		}
		x += WSMutStrAttr_GetAttrChar(str, s - sorigin)->width;
		++s;
	}
	if(WSMutStrAttrGetChar(s) == '\t' )
	{
		size_t ofs = tab_width - ( x % tab_width );
		x += ofs;
		return (STB_TEXTEDIT_WIDTHTYPE)x;
	}
	else if(WSMutStrAttrGetChar(s) < ' ')
		return 0;
	else
		return WSMutStrAttr_GetAttrChar(str, s - sorigin)->width;
}


void layoutrow( struct StbTexteditRowStruct* row, WSMUTSTRATTR* str,
	STB_TEXTEDIT_POSITIONTYPE n )
{
	const WSMUTSTRATTRCHAR* s = WSMutStrAttr_GetCString(str) + n;
	const WSMUTSTRATTRCHAR* sorigin = WSMutStrAttr_GetCString(str);
	size_t len = 0;
	size_t x = 0;
	/* skip over windows /r/n */
	if(s - sorigin
		&& WSMutStrAttrGetChar(s) == '\n'
		&& WSMutStrAttrGetChar(s - 1) == '\r')
		++s;
	while(WSMutStrAttrGetChar(s))
	{
		wsbool isnewline;
		if(WSMutStrAttrGetChar(s) == '\t')
		{
			size_t ofs = tab_width - ( x % tab_width );
			x += ( ofs - 1 );
		}
		x += WSMutStrAttr_GetAttrChar(str, s - sorigin)->width;
		isnewline = WSMutStrAttrGetChar(s) == '\n'
			|| WSMutStrAttrGetChar(s) == '\r';
		++s;
		if(isnewline)
			break;
		++len;
	}
	row->x0 = 0;
	row->x1 = (STB_TEXTEDIT_WIDTHTYPE)x;
	row->baseline_y_delta = WSMutStrAttr_GetAttrChar(str, s - sorigin)->height;
	row->ymin = 0;
	row->ymax = row->baseline_y_delta;
	row->num_chars = (size_t)( (s - sorigin) ) - n;
}
```

* [stb_vorbis.h](stb_vorbis.h) - The ogg vorbis reader with the following modifications:

	1. Modified to use more standardized data types
	2. Modified to give an option to remove the C runtime
	3. Modified to give an option use callbacks instead of memory/file reading.
	
	For #3, the declarations are of the form:
	```c
	#define STB_VORBIS_FROMCURRENT	1 /* SEEK_CUR */
	#define STB_VORBIS_FROMSTART	0 /* SEEK_SET */
	#define STB_VORBIS_FROMEND		2 /* SEEK_END */
	typedef struct stb_vorbis_funcs
	{
		 unsigned int (*read)(void* context, void* buf, unsigned int size);
		 int (*seek)(void* context, int offset, int whence);
		 unsigned int (*tell)(void* context);
		 void* context;
	} stb_vorbis_funcs;
	extern stb_vorbis * stb_vorbis_open_funcs(stb_vorbis_funcs *funcs,
		unsigned int size, int *error, const stb_vorbis_alloc *alloc);
	``` 	
	
	I also tried to merge in SDL's modifications as well for their RWOPS interface...
  	not sure how successful I was with that. According to SDL those modifications (all guarded by \[XXX\]SDL\[XXX\] #ifdefs) are public domain.
