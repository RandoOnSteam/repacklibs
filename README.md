# repackaged libs
Single-file header-only libs written by other people that are public domain and modified by me. For libraries made by me visit www.github.com/RandoOnSteam/libs.

* [miniz.h](miniz.h) - The original public domain minizip modified for ancient compilers and libc removal

* [stb_textedit.h](stb_textedit.h) - The text control guts with some small modifications to allow setting width type (STB_TEXTEDIT_WIDTHTYPE) as it is normally a float which isn't good on old CPUs without a FPU and a few more type safety fixes.

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
