/**
@file miniz.h

###Abstract###

miniz.c v1.15 - public domain deflate/inflate, zlib-subset, ZIP reading/writing/appending, PNG writing
See "unlicense" statement at the end of this file.
Rich Geldreich <richgel99@gmail.com>, last updated Oct. 13, 2013
Implements RFC 1950: http://www.ietf.org/rfc/rfc1950.txt and RFC 1951: http://www.ietf.org/rfc/rfc1951.txt

Most API's defined in miniz.c are optional. For example, to disable the archive related functions just define
MINIZ_NO_ARCHIVE_APIS, or to get rid of all stdio usage define MINIZ_NO_STDIO (see the list below for more macros).

###Modifications###

1) mz_[u]int[##] are changed to the proper C types.

2) When system.h is included beforehand WS time functions are used.

3) MINIZ_NO_STDIO is defined as C stdio is unsupported on legacy systems.

4) CRC32 is changed to a no-table variation (code of others are kept).


###Change History###

10/13/13 v1.15 r4 - Interim bugfix release while I work on the next major release with Zip64 support (almost there!):
- Critical fix for the MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY bug (thanks kahmyong.moon@hp.com) which could cause locate files to not find files. This bug
would only have occured in earlier versions if you explicitly used this flag, OR if you used mz_zip_extract_archive_file_to_heap() or mz_zip_add_mem_to_archive_file_in_place()
(which used this flag). If you can't switch to v1.15 but want to fix this bug, just remove the uses of this flag from both helper funcs (and of course don't use the flag).
- Bugfix in mz_zip_reader_extract_to_mem_no_alloc() from kymoon when pUser_read_buf is not NULL and compressed size is > uncompressed size
- Fixing mz_zip_reader_extract_*() funcs so they don't try to extract compressed data from directory entries, to account for weird zipfiles which contain zero-size compressed data on dir entries.
Hopefully this fix won't cause any issues on weird zip archives, because it assumes the low 16-bits of zip external attributes are DOS attributes (which I believe they always are in practice).
- Fixing mz_zip_reader_is_file_a_directory() so it doesn't check the internal attributes, just the filename and external attributes
- mz_zip_reader_init_file() - missing MZ_FCLOSE() call if the seek failed
- Added cmake support for Linux builds which builds all the examples, tested with clang v3.3 and gcc v4.6.
- Clang fix for tdefl_write_image_to_png_file_in_memory() from toffaletti
- Merged MZ_FORCEINLINE fix from hdeanclark
- Fix <time.h> include before config #ifdef, thanks emil.brink
- Added tdefl_write_image_to_png_file_in_memory_ex(): supports Y flipping (super useful for OpenGL apps), and explicit control over the compression level (so you can
set it to 1 for real-time compression).
- Merged in some compiler fixes from paulharris's github repro.
- Retested this build under Windows (VS 2010, including static analysis), tcc  0.9.26, gcc v4.6 and clang v3.3.
- Added example6.c, which dumps an image of the mandelbrot set to a PNG file.
- Modified example2 to help test the MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY flag more.
- In r3: Bugfix to mz_zip_writer_add_file() found during merge: Fix possible src file fclose() leak if alignment bytes+local header file write faiiled
- In r4: Minor bugfix to mz_zip_writer_add_from_zip_reader(): Was pushing the wrong central dir header offset, appears harmless in this release, but it became a problem in the zip64 branch
5/20/12 v1.14 - MinGW32/64 GCC 4.6.1 compiler fixes: added MZ_FORCEINLINE, #include <time.h> (thanks fermtect).
5/19/12 v1.13 - From jason@cornsyrup.org and kelwert@mtu.edu - Fix mz_crc32() so it doesn't compute the wrong CRC-32's when mz_ulong is 64-bit.
- Temporarily/locally slammed in "typedef unsigned long mz_ulong" and re-ran a randomized regression test on ~500k files.
- Eliminated a bunch of warnings when compiling with GCC 32-bit/64.
- Ran all examples, miniz.c, and tinfl.c through MSVC 2008's /analyze (static analysis) option and fixed all warnings (except for the silly
"Use of the comma-operator in a tested expression.." analysis warning, which I purposely use to work around a MSVC compiler warning).
- Created 32-bit and 64-bit Codeblocks projects/workspace. Built and tested Linux executables. The codeblocks workspace is compatible with Linux+Win32/x64.
- Added miniz_tester solution/project, which is a useful little app derived from LZHAM's tester app that I use as part of the regression test.
- Ran miniz.c and tinfl.c through another series of regression testing on ~500,000 files and archives.
- Modified example5.c so it purposely disables a bunch of high-level functionality (MINIZ_NO_STDIO, etc.). (Thanks to corysama for the MINIZ_NO_STDIO bug report.)
- Fix ftell() usage in examples so they exit with an error on files which are too large (a limitation of the examples, not miniz itself).
4/12/12 v1.12 - More comments, added low-level example5.c, fixed a couple minor level_and_flags issues in the archive API's.
level_and_flags can now be set to MZ_DEFAULT_COMPRESSION. Thanks to Bruce Dawson <bruced@valvesoftware.com> for the feedback/bug report.
5/28/11 v1.11 - Added statement from unlicense.org
5/27/11 v1.10 - Substantial compressor optimizations:
- Level 1 is now ~4x faster than before. The L1 compressor's throughput now varies between 70-110MB/sec. on a
- Core i7 (actual throughput varies depending on the type of data, and x64 vs. x86).
- Improved baseline L2-L9 compression perf. Also, greatly improved compression perf. issues on some file types.
- Refactored the compression code for better readability and maintainability.
- Added level 10 compression level (L10 has slightly better ratio than level 9, but could have a potentially large
drop in throughput on some files).
5/15/11 v1.09 - Initial stable release.

###Low-level Deflate/Inflate implementation notes###

Compression: Use the "tdefl" API's. The compressor supports raw, static, and dynamic blocks, lazy or
greedy parsing, match length filtering, RLE-only, and Huffman-only streams. It performs and compresses
approximately as well as zlib.

Decompression: Use the "tinfl" API's. The entire decompressor is implemented as a single function
coroutine: see tinfl_decompress(). It supports decompression into a 32KB (or larger power of 2) wrapping buffer, or into a memory
block large enough to hold the entire file.

The low-level tdefl/tinfl API's do not make any use of dynamic memory allocation.

###zlib-style API notes###

miniz.c implements a fairly large subset of zlib. There's enough functionality present for it to be a drop-in
zlib replacement in many apps:
The z_stream struct, optional memory allocation callbacks
deflateInit/deflateInit2/deflate/deflateReset/deflateEnd/deflateBound
inflateInit/inflateInit2/inflate/inflateEnd
compress, compress2, compressBound, uncompress
CRC-32, Adler-32 - Using modern, minimal code size, CPU cache friendly routines.
Supports raw deflate streams or standard zlib streams with adler-32 checking.

Limitations:
The callback API's are not implemented yet. No support for gzip headers or zlib static dictionaries.
I've tried to closely emulate zlib's various flavors of stream flushing and return status codes, but
there are no guarantees that miniz.c pulls this off perfectly.

* PNG writing: See the tdefl_write_image_to_png_file_in_memory() function, originally written by
Alex Evans. Supports 1-4 bytes/pixel images.

###ZIP archive API notes###

The ZIP archive API's where designed with simplicity and efficiency in mind, with just enough abstraction to
get the job done with minimal fuss. There are simple API's to retrieve file information, read files from
existing archives, create new archives, append new files to existing archives, or clone archive data from
one archive to another. It supports archives located in memory or the heap, on disk (using stdio.h),
or you can specify custom file read/write callbacks.

- Archive reading: Just call this function to read a single file from a disk archive:

void *mz_zip_extract_archive_file_to_heap(const char *pZip_filename, const char *pArchive_name,
size_t *pSize, mz_uint zip_flags);

For more complex cases, use the "mz_zip_reader" functions. Upon opening an archive, the entire central
directory is located and read as-is into memory, and subsequent file access only occurs when reading individual files.

- Archives file scanning: The simple way is to use this function to scan a loaded archive for a specific file:

int mz_zip_reader_locate_file(mz_zip_archive *pZip, const char *pName, const char *pComment, mz_uint flags);

The locate operation can optionally check file comments too, which (as one example) can be used to identify
multiple versions of the same file in an archive. This function uses a simple linear search through the central
directory, so it's not very fast.

Alternately, you can iterate through all the files in an archive (using mz_zip_reader_get_num_files()) and
retrieve detailed info on each file by calling mz_zip_reader_file_stat().

- Archive creation: Use the "mz_zip_writer" functions. The ZIP writer immediately writes compressed file data
to disk and builds an exact image of the central directory in memory. The central directory image is written
all at once at the end of the archive file when the archive is finalized.

The archive writer can optionally align each file's local header and file data to any power of 2 alignment,
which can be useful when the archive will be read from optical media. Also, the writer supports placing
arbitrary data blobs at the very beginning of ZIP archives. Archives written using either feature are still
readable by any ZIP tool.

- Archive appending: The simple way to add a single file to an archive is to call this function:

mz_bool mz_zip_add_mem_to_archive_file_in_place(const char *pZip_filename, const char *pArchive_name,
const void *pBuf, size_t buf_size, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags);

The archive will be created if it doesn't already exist, otherwise it'll be appended to.
Note the appending is done in-place and is not an atomic operation, so if something goes wrong
during the operation it's possible the archive could be left without a central directory (although the local
file headers and file data will be fine, so the archive will be recoverable).

For more complex archive modification scenarios:
1. The safest way is to use a mz_zip_reader to read the existing archive, cloning only those bits you want to
preserve into a new archive using using the mz_zip_writer_add_from_zip_reader() function (which compiles the
compressed file data as-is). When you're done, delete the old archive and rename the newly written archive, and
you're done. This is safe but requires a bunch of temporary disk space or heap memory.

2. Or, you can convert an mz_zip_reader in-place to an mz_zip_writer using mz_zip_writer_init_from_reader(),
append new files as needed, then finalize the archive which will write an updated central directory to the
original archive. (This is basically what mz_zip_add_mem_to_archive_file_in_place() does.) There's a
possibility that the archive's central directory could be lost with this method if anything goes wrong, though.

- ZIP archive support limitations:
ZIP64 metadata is supported. Multidisk archives can be read with the multidisk init APIs when disk start offsets are supplied, and mz_zip_reader_init_file() automatically discovers standard .z01, .z02, ... split files. mz_zip_writer_init_file_split() creates split archives. Aggregate central directories can be stored or Deflated. Extraction functions can only handle unencrypted, stored or deflated files. Patched entries can be cloned or returned as raw compressed data, but applying patches is not supported.
Requires streams capable of seeking.

* This is a header file library, like stb_image.c. To get only a header file, either cut and paste the
below header, or create miniz.h, #define MINIZ_HEADER_FILE_ONLY, and then include miniz.c from it.

* Important: For best perf. be sure to customize the below macros for your target platform:
#define MINIZ_USE_UNALIGNED_LOADS_AND_STORES 1
#define MINIZ_LITTLE_ENDIAN 1
#define MINIZ_HAS_64BIT_REGISTERS 1

* On platforms using glibc, Be sure to "#define _LARGEFILE64_SOURCE 1" before including miniz.c to ensure miniz
uses the 64-bit variants: fopen64(), stat64(), etc. Otherwise you won't be able to process large files
(i.e. 32-bit stat() fails for me on files > 0x7FFFFFFF bytes).
*/

/*
------------------------------------------------------------------------------
	BEGIN miniz.c
------------------------------------------------------------------------------
*/

#ifndef MINIZ_HEADER_INCLUDED
#define MINIZ_HEADER_INCLUDED

#pragma warning( push )
#pragma warning( disable: 4619 ) // pragma warning : there is no warning number 'number'
#pragma warning( disable: 4244 ) // 'conversion' conversion from 'type1' to 'type2', possible loss of data
#pragma warning( disable: 4365 ) // 'action' conversion from 'type_1' to 'type_2', signed/unsigned mismatch
#pragma warning( disable: 4548 ) // expression before comma has no effect; expected expression with side-effect
#pragma warning( disable: 4668 ) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'

#include <stdlib.h>

#ifdef MINIZ_USE_LZMA_SDK
  #ifndef MINIZ_LZMA_SDK_HEADER
    #define MINIZ_LZMA_SDK_HEADER "lzmasdk/LzmaLib.h"
  #endif
  #ifndef MINIZ_LZMA_SDK_VERSION_HEADER
    #define MINIZ_LZMA_SDK_VERSION_HEADER "lzmasdk/7zVersion.h"
  #endif
  #include MINIZ_LZMA_SDK_HEADER
  #include MINIZ_LZMA_SDK_VERSION_HEADER
#endif

// Define MINIZ_USE_AES to enable reading/writing WinZip AES encrypted entries (ZIP method 99, extra 0x9901) and
// reading legacy PKWARE ZipCrypto entries. Everything it needs (SHA-1, HMAC-SHA1, PBKDF2, AES, ZipCrypto) is
// inlined below - no external files are required. It is fully independent of MINIZ_USE_LZMA_SDK (only ZIP
// method 14 / LZMA still needs the SDK).
#ifdef MINIZ_USE_AES

#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* ---------------- AES (encrypt only) + CTR ----------------
 * Self-contained, portable, table-free software AES-128/192/256 encryption. WinZip AES uses AES in CTR mode, and
 * CTR needs only the block *encrypt* transform for both encryption and decryption, so no decryption path is needed.
 * This replaces the LZMA SDK's Aes.c for the AES path, so MINIZ_USE_AES has no external object-file dependency.
 * Verified byte-identical to the SDK / WinZip against pyzipper- and 7-Zip-produced archives. */

static const uint8_t mz_aes_sbox[256] = {
  0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
  0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
  0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
  0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
  0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
  0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
  0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
  0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
  0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
  0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
  0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
  0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
  0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
  0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
  0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
  0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};

typedef struct { uint32_t rk[60]; int rounds; } mz_aes_ctx; /* up to AES-256: 15 round keys * 4 words = 60 */

static uint32_t mz_aes_sub_word(uint32_t w) {
  return ((uint32_t)mz_aes_sbox[w & 0xff]) |
         ((uint32_t)mz_aes_sbox[(w >> 8) & 0xff] << 8) |
         ((uint32_t)mz_aes_sbox[(w >> 16) & 0xff] << 16) |
         ((uint32_t)mz_aes_sbox[(w >> 24) & 0xff] << 24);
}
static uint32_t mz_aes_rot_word(uint32_t w) { return (w >> 8) | (w << 24); }

/* key_len is bytes: 16/24/32. Round keys are stored as little-endian words (matching the block byte order below). */
static void mz_aes_key_expand(mz_aes_ctx *c, const uint8_t *key, size_t key_len) {
  static const uint8_t rcon[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36};
  int nk = (int)(key_len / 4);      /* 4, 6, 8 */
  int i, total;
  c->rounds = nk + 6;               /* 10, 12, 14 */
  total = 4 * (c->rounds + 1);
  for (i = 0; i < nk; ++i)
    c->rk[i] = (uint32_t)key[4*i] | ((uint32_t)key[4*i+1] << 8) | ((uint32_t)key[4*i+2] << 16) | ((uint32_t)key[4*i+3] << 24);
  for (i = nk; i < total; ++i) {
    uint32_t t = c->rk[i-1];
    if ((i % nk) == 0)
      t = mz_aes_sub_word(mz_aes_rot_word(t)) ^ (uint32_t)rcon[i/nk];
    else if ((nk > 6) && ((i % nk) == 4))
      t = mz_aes_sub_word(t);
    c->rk[i] = c->rk[i-nk] ^ t;
  }
}

#define MZ_XTIME(x) ((uint8_t)(((x) << 1) ^ (((x) >> 7) * 0x1b)))

/* Encrypts one 16-byte block (in==out allowed). State bytes are column-major per the AES spec. */
static void mz_aes_encrypt_block(const mz_aes_ctx *c, const uint8_t in[16], uint8_t out[16]) {
  uint8_t s[16], t[16];
  int r, i;
  for (i = 0; i < 16; ++i) s[i] = in[i];
  /* AddRoundKey (round 0) */
  for (i = 0; i < 4; ++i) {
    uint32_t k = c->rk[i];
    s[4*i] ^= (uint8_t)k; s[4*i+1] ^= (uint8_t)(k>>8); s[4*i+2] ^= (uint8_t)(k>>16); s[4*i+3] ^= (uint8_t)(k>>24);
  }
  for (r = 1; r < c->rounds; ++r) {
    /* SubBytes */
    for (i = 0; i < 16; ++i) s[i] = mz_aes_sbox[s[i]];
    /* ShiftRows (row j shifted left by j; state laid out column-major: byte at col*4+row) */
    t[0]=s[0];  t[4]=s[4];  t[8]=s[8];   t[12]=s[12];
    t[1]=s[5];  t[5]=s[9];  t[9]=s[13];  t[13]=s[1];
    t[2]=s[10]; t[6]=s[14]; t[10]=s[2];  t[14]=s[6];
    t[3]=s[15]; t[7]=s[3];  t[11]=s[7];  t[15]=s[11];
    /* MixColumns */
    for (i = 0; i < 4; ++i) {
      uint8_t a0=t[4*i],a1=t[4*i+1],a2=t[4*i+2],a3=t[4*i+3];
      uint8_t x01=MZ_XTIME(a0),x12=MZ_XTIME(a1),x23=MZ_XTIME(a2),x33=MZ_XTIME(a3);
      s[4*i]   = (uint8_t)(x01 ^ (x12 ^ a1) ^ a2 ^ a3);
      s[4*i+1] = (uint8_t)(a0 ^ x12 ^ (x23 ^ a2) ^ a3);
      s[4*i+2] = (uint8_t)(a0 ^ a1 ^ x23 ^ (x33 ^ a3));
      s[4*i+3] = (uint8_t)((x01 ^ a0) ^ a1 ^ a2 ^ x33);
    }
    /* AddRoundKey */
    for (i = 0; i < 4; ++i) {
      uint32_t k = c->rk[4*r+i];
      s[4*i]^=(uint8_t)k; s[4*i+1]^=(uint8_t)(k>>8); s[4*i+2]^=(uint8_t)(k>>16); s[4*i+3]^=(uint8_t)(k>>24);
    }
  }
  /* Final round (no MixColumns) */
  for (i = 0; i < 16; ++i) s[i] = mz_aes_sbox[s[i]];
  t[0]=s[0];  t[4]=s[4];  t[8]=s[8];   t[12]=s[12];
  t[1]=s[5];  t[5]=s[9];  t[9]=s[13];  t[13]=s[1];
  t[2]=s[10]; t[6]=s[14]; t[10]=s[2];  t[14]=s[6];
  t[3]=s[15]; t[7]=s[3];  t[11]=s[7];  t[15]=s[11];
  for (i = 0; i < 4; ++i) {
    uint32_t k = c->rk[4*c->rounds+i];
    out[4*i]   = (uint8_t)(t[4*i]   ^ (uint8_t)k);
    out[4*i+1] = (uint8_t)(t[4*i+1] ^ (uint8_t)(k>>8));
    out[4*i+2] = (uint8_t)(t[4*i+2] ^ (uint8_t)(k>>16));
    out[4*i+3] = (uint8_t)(t[4*i+3] ^ (uint8_t)(k>>24));
  }
}

/* AES-CTR XOR in place, WinZip convention: 16-byte little-endian counter starting at 1 for the first block. */
static void mz_aes_ctr_xor_buf(const mz_aes_ctx *c, uint8_t *data, size_t n) {
  uint8_t ctr[16], ks[16];
  size_t i;
  uint64_t counter = 0;
  memset(ctr, 0, sizeof(ctr));
  while (n) {
    size_t blk = (n < 16) ? n : 16;
    int j;
    /* pre-increment: first block uses counter value 1 */
    ++counter;
    for (j = 0; j < 8; ++j) ctr[j] = (uint8_t)(counter >> (8*j)); /* low 64 bits LE; high 8 bytes stay 0 */
    mz_aes_encrypt_block(c, ctr, ks);
    for (i = 0; i < blk; ++i) data[i] ^= ks[i];
    data += blk; n -= blk;
  }
}

/* ---------------- SHA-1 ---------------- */

typedef struct {
  uint32_t h[5];
  uint64_t len;      /* message length in bytes */
  uint8_t  buf[64];
  size_t   buf_used;
} mz_sha1_ctx;

#define MZ_SHA1_DIGEST_SIZE 20
#define MZ_SHA1_BLOCK_SIZE  64

static void mz_sha1_init(mz_sha1_ctx *c) {
  c->h[0] = 0x67452301u; c->h[1] = 0xEFCDAB89u; c->h[2] = 0x98BADCFEu;
  c->h[3] = 0x10325476u; c->h[4] = 0xC3D2E1F0u;
  c->len = 0; c->buf_used = 0;
}

static uint32_t mz_sha1_rol(uint32_t v, unsigned b) { return (v << b) | (v >> (32 - b)); }

static void mz_sha1_block(mz_sha1_ctx *c, const uint8_t *p) {
  uint32_t w[80], a, b, d, e, f, k, t;
  int i;
  for (i = 0; i < 16; ++i)
    w[i] = ((uint32_t)p[i*4] << 24) | ((uint32_t)p[i*4+1] << 16) | ((uint32_t)p[i*4+2] << 8) | (uint32_t)p[i*4+3];
  for (i = 16; i < 80; ++i)
    w[i] = mz_sha1_rol(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
  a = c->h[0]; b = c->h[1]; { uint32_t cc = c->h[2]; uint32_t dd = c->h[3]; e = c->h[4];
  d = dd;
  {
    uint32_t cv = cc;
    for (i = 0; i < 80; ++i) {
      if (i < 20)      { f = (b & cv) | ((~b) & d); k = 0x5A827999u; }
      else if (i < 40) { f = b ^ cv ^ d;           k = 0x6ED9EBA1u; }
      else if (i < 60) { f = (b & cv) | (b & d) | (cv & d); k = 0x8F1BBCDCu; }
      else             { f = b ^ cv ^ d;           k = 0xCA62C1D6u; }
      t = mz_sha1_rol(a, 5) + f + e + k + w[i];
      e = d; d = cv; cv = mz_sha1_rol(b, 30); b = a; a = t;
    }
    c->h[0] += a; c->h[1] += b; c->h[2] += cv; c->h[3] += d; c->h[4] += e;
  }
  }
}

static void mz_sha1_update(mz_sha1_ctx *c, const void *data, size_t n) {
  const uint8_t *p = (const uint8_t *)data;
  c->len += n;
  if (c->buf_used) {
    while (n && c->buf_used < 64) { c->buf[c->buf_used++] = *p++; --n; }
    if (c->buf_used == 64) { mz_sha1_block(c, c->buf); c->buf_used = 0; }
  }
  while (n >= 64) { mz_sha1_block(c, p); p += 64; n -= 64; }
  while (n) { c->buf[c->buf_used++] = *p++; --n; }
}

static void mz_sha1_final(mz_sha1_ctx *c, uint8_t out[MZ_SHA1_DIGEST_SIZE]) {
  uint64_t bits = c->len * 8;
  uint8_t pad = 0x80;
  int i;
  mz_sha1_update(c, &pad, 1);
  pad = 0x00;
  while (c->buf_used != 56) mz_sha1_update(c, &pad, 1);
  for (i = 7; i >= 0; --i) { uint8_t byte = (uint8_t)(bits >> (i * 8)); mz_sha1_update(c, &byte, 1); }
  for (i = 0; i < 5; ++i) {
    out[i*4]   = (uint8_t)(c->h[i] >> 24);
    out[i*4+1] = (uint8_t)(c->h[i] >> 16);
    out[i*4+2] = (uint8_t)(c->h[i] >> 8);
    out[i*4+3] = (uint8_t)(c->h[i]);
  }
}

/* ---------------- HMAC-SHA1 ---------------- */

typedef struct {
  mz_sha1_ctx inner;
  uint8_t okey[MZ_SHA1_BLOCK_SIZE];
} mz_hmac_sha1_ctx;

static void mz_hmac_sha1_init(mz_hmac_sha1_ctx *c, const uint8_t *key, size_t key_len) {
  uint8_t ikey[MZ_SHA1_BLOCK_SIZE], kbuf[MZ_SHA1_DIGEST_SIZE];
  size_t i;
  if (key_len > MZ_SHA1_BLOCK_SIZE) {
    mz_sha1_ctx t; mz_sha1_init(&t); mz_sha1_update(&t, key, key_len); mz_sha1_final(&t, kbuf);
    key = kbuf; key_len = MZ_SHA1_DIGEST_SIZE;
  }
  for (i = 0; i < MZ_SHA1_BLOCK_SIZE; ++i) {
    uint8_t kb = (i < key_len) ? key[i] : 0;
    ikey[i]    = kb ^ 0x36;
    c->okey[i] = kb ^ 0x5c;
  }
  mz_sha1_init(&c->inner);
  mz_sha1_update(&c->inner, ikey, MZ_SHA1_BLOCK_SIZE);
}

static void mz_hmac_sha1_update(mz_hmac_sha1_ctx *c, const void *data, size_t n) {
  mz_sha1_update(&c->inner, data, n);
}

static void mz_hmac_sha1_final(mz_hmac_sha1_ctx *c, uint8_t out[MZ_SHA1_DIGEST_SIZE]) {
  uint8_t ihash[MZ_SHA1_DIGEST_SIZE];
  mz_sha1_ctx outer;
  mz_sha1_final(&c->inner, ihash);
  mz_sha1_init(&outer);
  mz_sha1_update(&outer, c->okey, MZ_SHA1_BLOCK_SIZE);
  mz_sha1_update(&outer, ihash, MZ_SHA1_DIGEST_SIZE);
  mz_sha1_final(&outer, out);
}

static void mz_hmac_sha1(const uint8_t *key, size_t key_len, const void *msg, size_t msg_len, uint8_t out[MZ_SHA1_DIGEST_SIZE]) {
  mz_hmac_sha1_ctx c;
  mz_hmac_sha1_init(&c, key, key_len);
  mz_hmac_sha1_update(&c, msg, msg_len);
  mz_hmac_sha1_final(&c, out);
}

/* ---------------- PBKDF2-HMAC-SHA1 ---------------- */
/* Derives dk_len bytes. WinZip AES uses 1000 iterations. */
static void mz_pbkdf2_hmac_sha1(const uint8_t *pw, size_t pw_len,
                                const uint8_t *salt, size_t salt_len,
                                uint32_t iters, uint8_t *dk, size_t dk_len) {
  uint32_t block = 1;
  size_t offset = 0;
  /* precompute HMAC with the password key once per block start (re-init cheap) */
  while (offset < dk_len) {
    uint8_t u[MZ_SHA1_DIGEST_SIZE], t[MZ_SHA1_DIGEST_SIZE];
    uint8_t be[4];
    mz_hmac_sha1_ctx c;
    uint32_t j;
    size_t k, take;
    be[0] = (uint8_t)(block >> 24); be[1] = (uint8_t)(block >> 16);
    be[2] = (uint8_t)(block >> 8);  be[3] = (uint8_t)(block);
    /* U1 = PRF(pw, salt || INT(block)) */
    mz_hmac_sha1_init(&c, pw, pw_len);
    mz_hmac_sha1_update(&c, salt, salt_len);
    mz_hmac_sha1_update(&c, be, 4);
    mz_hmac_sha1_final(&c, u);
    memcpy(t, u, MZ_SHA1_DIGEST_SIZE);
    for (j = 1; j < iters; ++j) {
      mz_hmac_sha1(pw, pw_len, u, MZ_SHA1_DIGEST_SIZE, u);
      for (k = 0; k < MZ_SHA1_DIGEST_SIZE; ++k) t[k] ^= u[k];
    }
    take = dk_len - offset;
    if (take > MZ_SHA1_DIGEST_SIZE) take = MZ_SHA1_DIGEST_SIZE;
    memcpy(dk + offset, t, take);
    offset += take;
    ++block;
  }
}

/* ---------------- WinZip AES parameters ---------------- */

/* strength: 1=AES-128, 2=AES-192, 3=AES-256 */
static size_t mz_aes_salt_len(int strength)   { return (strength == 1) ? 8 : (strength == 2) ? 12 : 16; }
static size_t mz_aes_key_len(int strength)    { return (strength == 1) ? 16 : (strength == 2) ? 24 : 32; }
#define MZ_AES_PW_VERIFY_LEN 2
#define MZ_AES_AUTH_CODE_LEN 10   /* HMAC-SHA1 truncated to 10 bytes */

/* Derive enc key || auth key || 2-byte verifier from password+salt.
   Buffers must be sized key_len, key_len, 2 respectively. */
static void mz_aes_derive_keys(const uint8_t *pw, size_t pw_len,
                               const uint8_t *salt, int strength,
                               uint8_t *enc_key, uint8_t *auth_key, uint8_t verify[2]) {
  size_t kl = mz_aes_key_len(strength);
  size_t total = kl * 2 + MZ_AES_PW_VERIFY_LEN;
  uint8_t dk[32 * 2 + 2];
  mz_pbkdf2_hmac_sha1(pw, pw_len, salt, mz_aes_salt_len(strength), 1000, dk, total);
  memcpy(enc_key, dk, kl);
  memcpy(auth_key, dk + kl, kl);
  verify[0] = dk[kl * 2]; verify[1] = dk[kl * 2 + 1];
}

/* ---------------- Legacy PKWARE ZipCrypto (decryption only) ----------------
 * Traditional ZIP encryption (general-purpose bit 0). Weak by modern standards; provided for READING old
 * archives only. Never use it to write. A 12-byte encryption header precedes the (still-compressed) data; the
 * final header byte checks against the CRC-32 high byte (or, for streamed entries, the DOS time high byte). */

typedef struct { uint32_t k0, k1, k2; } mz_zipcrypto_keys;

/* CRC-32 (IEEE, reflected) used by ZipCrypto's key update. Kept standalone to avoid ordering deps on mz_crc32. */
static uint32_t mz_zipcrypto_crc32(uint32_t crc, uint8_t b) {
  int i;
  crc ^= b;
  for (i = 0; i < 8; ++i)
    crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)(-(int32_t)(crc & 1)));
  return crc;
}

static void mz_zipcrypto_update(mz_zipcrypto_keys *k, uint8_t b) {
  k->k0 = mz_zipcrypto_crc32(k->k0, b);
  k->k1 = (k->k1 + (k->k0 & 0xff)) * 134775813u + 1u;
  k->k2 = mz_zipcrypto_crc32(k->k2, (uint8_t)(k->k1 >> 24));
}

static uint8_t mz_zipcrypto_decrypt_byte(const mz_zipcrypto_keys *k) {
  uint16_t t = (uint16_t)((k->k2 & 0xffff) | 2);
  return (uint8_t)((t * (t ^ 1)) >> 8);
}

static void mz_zipcrypto_init(mz_zipcrypto_keys *k, const uint8_t *pw, size_t pw_len) {
  size_t i;
  k->k0 = 305419896u; k->k1 = 591751049u; k->k2 = 878082192u;
  for (i = 0; i < pw_len; ++i)
    mz_zipcrypto_update(k, pw[i]);
}

/* Decrypts n bytes in place (keystream XOR + key update per byte). */
static void mz_zipcrypto_decrypt(mz_zipcrypto_keys *k, uint8_t *data, size_t n) {
  size_t i;
  for (i = 0; i < n; ++i) {
    uint8_t c = (uint8_t)(data[i] ^ mz_zipcrypto_decrypt_byte(k));
    mz_zipcrypto_update(k, c);
    data[i] = c;
  }
}

#endif // MINIZ_USE_AES

// Defines to completely disable specific portions of miniz.c:
// If all macros here are defined the only functionality remaining will be CRC-32, adler-32, tinfl, and tdefl.

// Define MINIZ_NO_STDIO to disable all usage and any functions which rely on stdio for file I/O.
//#define MINIZ_NO_STDIO

// Define MINIZ_USE_LZMA_SDK to enable ZIP method 14 and LZMA central-directory compression.
// This expects the public-domain LZMA SDK under lzmasdk by default; override the two header macros above for another layout.

// If MINIZ_NO_TIME is specified then the ZIP archive functions will not be able to get the current time, or
// get/set file times.
// The current downside is the times written to your archives will be from 1979.
//#define MINIZ_NO_TIME

// Define MINIZ_NO_ARCHIVE_APIS to disable all ZIP archive API's.
//#define MINIZ_NO_ARCHIVE_APIS

// Define MINIZ_NO_ARCHIVE_APIS to disable all writing related ZIP archive API's.
//#define MINIZ_NO_ARCHIVE_WRITING_APIS

// Define MINIZ_NO_ZLIB_APIS to remove all ZLIB-style compression/decompression API's.
//#define MINIZ_NO_ZLIB_APIS

// Define MINIZ_NO_ZLIB_COMPATIBLE_NAME to disable zlib names, to prevent conflicts against stock zlib.
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES

// Define MINIZ_NO_MALLOC to disable all calls to malloc, free, and realloc.
// Note if MINIZ_NO_MALLOC is defined then the user must always provide custom user alloc/free/realloc
// callbacks to the zlib and archive API's, and a few stand-alone helper API's which don't provide custom user
// functions (such as tdefl_compress_mem_to_heap() and tinfl_decompress_mem_to_heap()) won't work.
//#define MINIZ_NO_MALLOC

#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__)
// MINIZ_X86_OR_X64_CPU is only used to help set the below macros.
#define MINIZ_X86_OR_X64_CPU 1
#endif

#if (__BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__) || MINIZ_X86_OR_X64_CPU
// Set MINIZ_LITTLE_ENDIAN to 1 if the processor is little endian.
#define MINIZ_LITTLE_ENDIAN 1
#endif

#if MINIZ_X86_OR_X64_CPU
// Set MINIZ_USE_UNALIGNED_LOADS_AND_STORES to 1 on CPU's that permit efficient integer loads and stores from unaligned addresses.
#define MINIZ_USE_UNALIGNED_LOADS_AND_STORES 1
#endif

#if defined(_M_X64) || defined(_WIN64) || defined(__MINGW64__) || defined(_LP64) || defined(__LP64__) || defined(__ia64__) || defined(__x86_64__)
// Set MINIZ_HAS_64BIT_REGISTERS to 1 if operations on 64-bit integers are reasonably fast (and don't involve compiler generated calls to helper functions).
#define MINIZ_HAS_64BIT_REGISTERS 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ------------------- zlib-style API Definitions.

// For more compatibility with zlib, miniz.c uses unsigned long for some parameters/struct members. Beware: mz_ulong can be either 32 or 64-bits!
typedef unsigned long mz_ulong;

// mz_free() internally uses the MZ_FREE() macro (which by default calls free() unless you've modified the MZ_MALLOC macro) to release a block allocated from the heap.
void mz_free(void *p);

#define MZ_ADLER32_INIT (1)
// mz_adler32() returns the initial adler-32 value to use when called with ptr==NULL.
mz_ulong mz_adler32(mz_ulong adler, const unsigned char *ptr, size_t buf_len);

#define MZ_CRC32_INIT (0)
// mz_crc32() returns the initial CRC-32 value to use when called with ptr==NULL.
mz_ulong mz_crc32(mz_ulong crc, const unsigned char *ptr, size_t buf_len);

// Compression strategies.
enum { MZ_DEFAULT_STRATEGY = 0, MZ_FILTERED = 1, MZ_HUFFMAN_ONLY = 2, MZ_RLE = 3, MZ_FIXED = 4 };

// Method
#define MZ_DEFLATED 8
#define MZ_LZMA 14
// WinZip AES marker method: the ZIP header method field is 99; the real method is stored in the 0x9901 extra.
#define MZ_ZIP_AES 99

#ifndef MINIZ_NO_ZLIB_APIS

// Heap allocation callbacks.
// Note that mz_alloc_func parameter types purpsosely differ from zlib's: items/size is size_t, not unsigned long.
typedef void *(*mz_alloc_func)(void *opaque, size_t items, size_t size);
typedef void (*mz_free_func)(void *opaque, void *address);
typedef void *(*mz_realloc_func)(void *opaque, void *address, size_t items, size_t size);

#define MZ_VERSION          "9.1.15"
#define MZ_VERNUM           0x91F0
#define MZ_VER_MAJOR        9
#define MZ_VER_MINOR        1
#define MZ_VER_REVISION     15
#define MZ_VER_SUBREVISION  0

// Flush values. For typical usage you only need MZ_NO_FLUSH and MZ_FINISH. The other values are for advanced use (refer to the zlib docs).
enum { MZ_NO_FLUSH = 0, MZ_PARTIAL_FLUSH = 1, MZ_SYNC_FLUSH = 2, MZ_FULL_FLUSH = 3, MZ_FINISH = 4, MZ_BLOCK = 5 };

// Return status codes. MZ_PARAM_ERROR is non-standard.
enum { MZ_OK = 0, MZ_STREAM_END = 1, MZ_NEED_DICT = 2, MZ_ERRNO = -1, MZ_STREAM_ERROR = -2, MZ_DATA_ERROR = -3, MZ_MEM_ERROR = -4, MZ_BUF_ERROR = -5, MZ_VERSION_ERROR = -6, MZ_PARAM_ERROR = -10000 };

// Compression levels: 0-9 are the standard zlib-style levels, 10 is best possible compression (not zlib compatible, and may be very slow), MZ_DEFAULT_COMPRESSION=MZ_DEFAULT_LEVEL.
enum { MZ_NO_COMPRESSION = 0, MZ_BEST_SPEED = 1, MZ_BEST_COMPRESSION = 9, MZ_UBER_COMPRESSION = 10, MZ_DEFAULT_LEVEL = 6, MZ_DEFAULT_COMPRESSION = -1 };

// Window bits
#define MZ_DEFAULT_WINDOW_BITS 15

struct mz_internal_state;

// Compression/decompression stream struct.
typedef struct mz_stream_s
{
  const unsigned char *next_in;     // pointer to next byte to read
  unsigned int avail_in;            // number of bytes available at next_in
  mz_ulong total_in;                // total number of bytes consumed so far

  unsigned char *next_out;          // pointer to next byte to write
  unsigned int avail_out;           // number of bytes that can be written to next_out
  mz_ulong total_out;               // total number of bytes produced so far

  char *msg;                        // error msg (unused)
  struct mz_internal_state *state;  // internal state, allocated by zalloc/zfree

  mz_alloc_func zalloc;             // optional heap allocation function (defaults to malloc)
  mz_free_func zfree;               // optional heap free function (defaults to free)
  void *opaque;                     // heap alloc function user pointer

  int data_type;                    // data_type (unused)
  mz_ulong adler;                   // adler32 of the source or uncompressed data
  mz_ulong reserved;                // not used
} mz_stream;

typedef mz_stream *mz_streamp;

// Returns the version string of miniz.c.
const char *mz_version(void);

// mz_deflateInit() initializes a compressor with default options:
// Parameters:
//  pStream must point to an initialized mz_stream struct.
//  level must be between [MZ_NO_COMPRESSION, MZ_BEST_COMPRESSION].
//  level 1 enables a specially optimized compression function that's been optimized purely for performance, not ratio.
//  (This special func. is currently only enabled when MINIZ_USE_UNALIGNED_LOADS_AND_STORES and MINIZ_LITTLE_ENDIAN are defined.)
// Return values:
//  MZ_OK on success.
//  MZ_STREAM_ERROR if the stream is bogus.
//  MZ_PARAM_ERROR if the input parameters are bogus.
//  MZ_MEM_ERROR on out of memory.
int mz_deflateInit(mz_streamp pStream, int level);

// mz_deflateInit2() is like mz_deflate(), except with more control:
// Additional parameters:
//   method must be MZ_DEFLATED
//   window_bits must be MZ_DEFAULT_WINDOW_BITS (to wrap the deflate stream with zlib header/adler-32 footer) or -MZ_DEFAULT_WINDOW_BITS (raw deflate/no header or footer)
//   mem_level must be between [1, 9] (it's checked but ignored by miniz.c)
int mz_deflateInit2(mz_streamp pStream, int level, int method, int window_bits, int mem_level, int strategy);

// Quickly resets a compressor without having to reallocate anything. Same as calling mz_deflateEnd() followed by mz_deflateInit()/mz_deflateInit2().
int mz_deflateReset(mz_streamp pStream);

// mz_deflate() compresses the input to output, consuming as much of the input and producing as much output as possible.
// Parameters:
//   pStream is the stream to read from and write to. You must initialize/update the next_in, avail_in, next_out, and avail_out members.
//   flush may be MZ_NO_FLUSH, MZ_PARTIAL_FLUSH/MZ_SYNC_FLUSH, MZ_FULL_FLUSH, or MZ_FINISH.
// Return values:
//   MZ_OK on success (when flushing, or if more input is needed but not available, and/or there's more output to be written but the output buffer is full).
//   MZ_STREAM_END if all input has been consumed and all output bytes have been written. Don't call mz_deflate() on the stream anymore.
//   MZ_STREAM_ERROR if the stream is bogus.
//   MZ_PARAM_ERROR if one of the parameters is invalid.
//   MZ_BUF_ERROR if no forward progress is possible because the input and/or output buffers are empty. (Fill up the input buffer or free up some output space and try again.)
int mz_deflate(mz_streamp pStream, int flush);

// mz_deflateEnd() deinitializes a compressor:
// Return values:
//  MZ_OK on success.
//  MZ_STREAM_ERROR if the stream is bogus.
int mz_deflateEnd(mz_streamp pStream);

// mz_deflateBound() returns a (very) conservative upper bound on the amount of data that could be generated by deflate(), assuming flush is set to only MZ_NO_FLUSH or MZ_FINISH.
mz_ulong mz_deflateBound(mz_streamp pStream, mz_ulong source_len);

// Single-call compression functions mz_compress() and mz_compress2():
// Returns MZ_OK on success, or one of the error codes from mz_deflate() on failure.
int mz_compress(unsigned char *pDest, mz_ulong *pDest_len, const unsigned char *pSource, mz_ulong source_len);
int mz_compress2(unsigned char *pDest, mz_ulong *pDest_len, const unsigned char *pSource, mz_ulong source_len, int level);

// mz_compressBound() returns a (very) conservative upper bound on the amount of data that could be generated by calling mz_compress().
mz_ulong mz_compressBound(mz_ulong source_len);

// Initializes a decompressor.
int mz_inflateInit(mz_streamp pStream);

// mz_inflateInit2() is like mz_inflateInit() with an additional option that controls the window size and whether or not the stream has been wrapped with a zlib header/footer:
// window_bits must be MZ_DEFAULT_WINDOW_BITS (to parse zlib header/footer) or -MZ_DEFAULT_WINDOW_BITS (raw deflate).
int mz_inflateInit2(mz_streamp pStream, int window_bits);

// Decompresses the input stream to the output, consuming only as much of the input as needed, and writing as much to the output as possible.
// Parameters:
//   pStream is the stream to read from and write to. You must initialize/update the next_in, avail_in, next_out, and avail_out members.
//   flush may be MZ_NO_FLUSH, MZ_SYNC_FLUSH, or MZ_FINISH.
//   On the first call, if flush is MZ_FINISH it's assumed the input and output buffers are both sized large enough to decompress the entire stream in a single call (this is slightly faster).
//   MZ_FINISH implies that there are no more source bytes available beside what's already in the input buffer, and that the output buffer is large enough to hold the rest of the decompressed data.
// Return values:
//   MZ_OK on success. Either more input is needed but not available, and/or there's more output to be written but the output buffer is full.
//   MZ_STREAM_END if all needed input has been consumed and all output bytes have been written. For zlib streams, the adler-32 of the decompressed data has also been verified.
//   MZ_STREAM_ERROR if the stream is bogus.
//   MZ_DATA_ERROR if the deflate stream is invalid.
//   MZ_PARAM_ERROR if one of the parameters is invalid.
//   MZ_BUF_ERROR if no forward progress is possible because the input buffer is empty but the inflater needs more input to continue, or if the output buffer is not large enough. Call mz_inflate() again
//   with more input data, or with more room in the output buffer (except when using single call decompression, described above).
int mz_inflate(mz_streamp pStream, int flush);

// Deinitializes a decompressor.
int mz_inflateEnd(mz_streamp pStream);

// Single-call decompression.
// Returns MZ_OK on success, or one of the error codes from mz_inflate() on failure.
int mz_uncompress(unsigned char *pDest, mz_ulong *pDest_len, const unsigned char *pSource, mz_ulong source_len);

// Returns a string description of the specified error code, or NULL if the error code is invalid.
const char *mz_error(int err);

// Redefine zlib-compatible names to miniz equivalents, so miniz.c can be used as a drop-in replacement for the subset of zlib that miniz.c supports.
// Define MINIZ_NO_ZLIB_COMPATIBLE_NAMES to disable zlib-compatibility if you use zlib in the same project.
#ifndef MINIZ_NO_ZLIB_COMPATIBLE_NAMES
  typedef unsigned char Byte;
  typedef unsigned int uInt;
  typedef mz_ulong uLong;
  typedef Byte Bytef;
  typedef uInt uIntf;
  typedef char charf;
  typedef int intf;
  typedef void *voidpf;
  typedef uLong uLongf;
  typedef void *voidp;
  typedef void *const voidpc;
  #define Z_NULL                0
  #define Z_NO_FLUSH            MZ_NO_FLUSH
  #define Z_PARTIAL_FLUSH       MZ_PARTIAL_FLUSH
  #define Z_SYNC_FLUSH          MZ_SYNC_FLUSH
  #define Z_FULL_FLUSH          MZ_FULL_FLUSH
  #define Z_FINISH              MZ_FINISH
  #define Z_BLOCK               MZ_BLOCK
  #define Z_OK                  MZ_OK
  #define Z_STREAM_END          MZ_STREAM_END
  #define Z_NEED_DICT           MZ_NEED_DICT
  #define Z_ERRNO               MZ_ERRNO
  #define Z_STREAM_ERROR        MZ_STREAM_ERROR
  #define Z_DATA_ERROR          MZ_DATA_ERROR
  #define Z_MEM_ERROR           MZ_MEM_ERROR
  #define Z_BUF_ERROR           MZ_BUF_ERROR
  #define Z_VERSION_ERROR       MZ_VERSION_ERROR
  #define Z_PARAM_ERROR         MZ_PARAM_ERROR
  #define Z_NO_COMPRESSION      MZ_NO_COMPRESSION
  #define Z_BEST_SPEED          MZ_BEST_SPEED
  #define Z_BEST_COMPRESSION    MZ_BEST_COMPRESSION
  #define Z_DEFAULT_COMPRESSION MZ_DEFAULT_COMPRESSION
  #define Z_DEFAULT_STRATEGY    MZ_DEFAULT_STRATEGY
  #define Z_FILTERED            MZ_FILTERED
  #define Z_HUFFMAN_ONLY        MZ_HUFFMAN_ONLY
  #define Z_RLE                 MZ_RLE
  #define Z_FIXED               MZ_FIXED
  #define Z_DEFLATED            MZ_DEFLATED
  #define Z_DEFAULT_WINDOW_BITS MZ_DEFAULT_WINDOW_BITS
  #define alloc_func            mz_alloc_func
  #define free_func             mz_free_func
  #define internal_state        mz_internal_state
  #define z_stream              mz_stream
  #define deflateInit           mz_deflateInit
  #define deflateInit2          mz_deflateInit2
  #define deflateReset          mz_deflateReset
  #define deflate               mz_deflate
  #define deflateEnd            mz_deflateEnd
  #define deflateBound          mz_deflateBound
  #define compress              mz_compress
  #define compress2             mz_compress2
  #define compressBound         mz_compressBound
  #define inflateInit           mz_inflateInit
  #define inflateInit2          mz_inflateInit2
  #define inflate               mz_inflate
  #define inflateEnd            mz_inflateEnd
  #define uncompress            mz_uncompress
  #define crc32                 mz_crc32
  #define adler32               mz_adler32
  #define MAX_WBITS             15
  #define MAX_MEM_LEVEL         9
  #define zError                mz_error
  #define ZLIB_VERSION          MZ_VERSION
  #define ZLIB_VERNUM           MZ_VERNUM
  #define ZLIB_VER_MAJOR        MZ_VER_MAJOR
  #define ZLIB_VER_MINOR        MZ_VER_MINOR
  #define ZLIB_VER_REVISION     MZ_VER_REVISION
  #define ZLIB_VER_SUBREVISION  MZ_VER_SUBREVISION
  #define zlibVersion           mz_version
  #define zlib_version          mz_version()
#endif // #ifndef MINIZ_NO_ZLIB_COMPATIBLE_NAMES

#endif // MINIZ_NO_ZLIB_APIS

// ------------------- Types and macros (WikiCms exclusive here as well)

typedef unsigned char mz_uint8;
typedef signed short mz_int16;
typedef unsigned short mz_uint16;
typedef uint32_t mz_uint32;
typedef unsigned int mz_uint;
typedef int64_t mz_int64;
typedef uint64_t mz_uint64;
typedef int mz_bool;
typedef size_t mz_time;
#if defined(NOCRUNTIME)
	#define memset WSMemoryFill
	#define memcpy WSMemoryCopy
	#define MZ_ASSERT(x)
#endif

#define MZ_FALSE (0)
#define MZ_TRUE (1)
#define MZ_UINT16_MAX (0xFFFFU)
#define MZ_UINT32_MAX (0xFFFFFFFFU)

// An attempt to work around MSVC's spammy "warning C4127: conditional expression is constant" message.
#ifdef _MSC_VER
   #define MZ_MACRO_END while (0, 0)
#else
   #define MZ_MACRO_END while (0)
#endif

// ------------------- ZIP archive reading/writing

#ifndef MINIZ_NO_ARCHIVE_APIS

enum
{
  MZ_ZIP_MAX_IO_BUF_SIZE = 64*1024,
  MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE = 260,
  MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE = 256
};

typedef struct
{
  mz_uint32 m_file_index;
  mz_uint64 m_central_dir_ofs;
  mz_uint16 m_version_made_by;
  mz_uint16 m_version_needed;
  mz_uint16 m_bit_flag;
  mz_uint16 m_method;
#ifndef MINIZ_NO_TIME
  mz_time m_time;
#endif
  mz_uint32 m_crc32;
  mz_uint64 m_comp_size;
  mz_uint64 m_uncomp_size;
  mz_uint16 m_internal_attr;
  mz_uint32 m_external_attr;
  mz_uint64 m_local_header_ofs;
  mz_uint32 m_disk_index;
  mz_uint32 m_comment_size;
  // WinZip AES: when m_is_encrypted && m_aes_strength != 0 the entry is WinZip AES. m_method is then the real
  // compression method from the 0x9901 extra (0/8/14), NOT 99. m_aes_strength is 1/2/3, m_aes_version 1/2.
  mz_uint8 m_is_encrypted;
  mz_uint8 m_aes_strength;
  mz_uint8 m_aes_version;
  char m_filename[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];
  char m_comment[MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE];
} mz_zip_archive_file_stat;

typedef size_t (*mz_file_read_func)(void *pOpaque, mz_uint64 file_ofs, void *pBuf, size_t n);
typedef size_t (*mz_file_write_func)(void *pOpaque, mz_uint64 file_ofs, const void *pBuf, size_t n);

struct mz_zip_internal_state_tag;
typedef struct mz_zip_internal_state_tag mz_zip_internal_state;

typedef enum
{
  MZ_ZIP_MODE_INVALID = 0,
  MZ_ZIP_MODE_READING = 1,
  MZ_ZIP_MODE_WRITING = 2,
  MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED = 3
} mz_zip_mode;

typedef struct mz_zip_archive_tag
{
  mz_uint64 m_archive_size;
  mz_uint64 m_central_directory_file_ofs;
  mz_uint m_total_files;
  mz_zip_mode m_zip_mode;

  mz_uint m_file_offset_alignment;

  mz_alloc_func m_pAlloc;
  mz_free_func m_pFree;
  mz_realloc_func m_pRealloc;
  void *m_pAlloc_opaque;

  mz_file_read_func m_pRead;
  mz_file_write_func m_pWrite;
  void *m_pIO_opaque;

  mz_zip_internal_state *m_pState;

} mz_zip_archive;

typedef enum
{
  MZ_ZIP_FLAG_CASE_SENSITIVE                = 0x0100,
  MZ_ZIP_FLAG_IGNORE_PATH                   = 0x0200,
  MZ_ZIP_FLAG_COMPRESSED_DATA               = 0x0400,
  MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY = 0x0800,
  MZ_ZIP_FLAG_LZMA                          = 0x1000,
  // Forces ZIP64 local/central headers on an added entry regardless of its size (for testing, or to keep a stable layout). The archive's EOCD becomes ZIP64 as usual.
  MZ_ZIP_FLAG_WRITE_ZIP64                   = 0x2000,
  // Encrypt this entry with WinZip AES. Uses the per-entry password if one was passed, else the archive default set via mz_zip_writer_set_password().
  MZ_ZIP_FLAG_ENCRYPT                       = 0x4000
} mz_zip_flags;

// ZIP archive reading

// Inits a ZIP archive reader.
// These functions read and validate the archive's central directory.
mz_bool mz_zip_reader_init(mz_zip_archive *pZip, mz_uint64 size, mz_uint32 flags);
mz_bool mz_zip_reader_init_mem(mz_zip_archive *pZip, const void *pMem, size_t size, mz_uint32 flags);
// Multidisk variants expect pDisk_start_offsets[0] == 0 and each following entry to be the logical concatenated offset where that disk begins.
// For mz_zip_reader_init_multidisk(), pZip->m_pRead must read from that logical concatenated byte stream before the call.
mz_bool mz_zip_reader_init_multidisk(mz_zip_archive *pZip, mz_uint64 size, mz_uint32 num_disks, const mz_uint64 *pDisk_start_offsets, mz_uint32 flags);
mz_bool mz_zip_reader_init_mem_multidisk(mz_zip_archive *pZip, const void *pMem, size_t size, mz_uint32 num_disks, const mz_uint64 *pDisk_start_offsets, mz_uint32 flags);

#ifndef MINIZ_NO_STDIO
// Standard split archives are discovered automatically when pFilename ends in .zip.
mz_bool mz_zip_reader_init_file(mz_zip_archive *pZip, const char *pFilename, mz_uint32 flags);
#endif

// Returns the total number of files in the archive.
mz_uint mz_zip_reader_get_num_files(mz_zip_archive *pZip);

// Returns detailed information about an archive file entry.
mz_bool mz_zip_reader_file_stat(mz_zip_archive *pZip, mz_uint file_index, mz_zip_archive_file_stat *pStat);

// Determines if an archive file entry is a directory entry.
mz_bool mz_zip_reader_is_file_a_directory(mz_zip_archive *pZip, mz_uint file_index);
mz_bool mz_zip_reader_is_file_encrypted(mz_zip_archive *pZip, mz_uint file_index);

// Retrieves the filename of an archive file entry.
// Returns the number of bytes written to pFilename, or if filename_buf_size is 0 this function returns the number of bytes needed to fully store the filename.
mz_uint mz_zip_reader_get_filename(mz_zip_archive *pZip, mz_uint file_index, char *pFilename, mz_uint filename_buf_size);

// Attempts to locates a file in the archive's central directory.
// Valid flags: MZ_ZIP_FLAG_CASE_SENSITIVE, MZ_ZIP_FLAG_IGNORE_PATH
// Returns -1 if the file cannot be found.
int mz_zip_reader_locate_file(mz_zip_archive *pZip, const char *pName, const char *pComment, mz_uint flags);

// Extracts a archive file to a memory buffer using no memory allocation.
mz_bool mz_zip_reader_extract_to_mem_no_alloc(mz_zip_archive *pZip, mz_uint file_index, void *pBuf, size_t buf_size, mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size);
mz_bool mz_zip_reader_extract_file_to_mem_no_alloc(mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size);

// Extracts a archive file to a memory buffer.
mz_bool mz_zip_reader_extract_to_mem(mz_zip_archive *pZip, mz_uint file_index, void *pBuf, size_t buf_size, mz_uint flags);
mz_bool mz_zip_reader_extract_file_to_mem(mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, mz_uint flags);

// Extracts a archive file to a dynamically allocated heap buffer.
void *mz_zip_reader_extract_to_heap(mz_zip_archive *pZip, mz_uint file_index, size_t *pSize, mz_uint flags);
void *mz_zip_reader_extract_file_to_heap(mz_zip_archive *pZip, const char *pFilename, size_t *pSize, mz_uint flags);

// Extracts a archive file using a callback function to output the file's data.
mz_bool mz_zip_reader_extract_to_callback(mz_zip_archive *pZip, mz_uint file_index, mz_file_write_func pCallback, void *pOpaque, mz_uint flags);
mz_bool mz_zip_reader_extract_file_to_callback(mz_zip_archive *pZip, const char *pFilename, mz_file_write_func pCallback, void *pOpaque, mz_uint flags);

#ifndef MINIZ_NO_STDIO
// Extracts a archive file to a disk file and sets its last accessed and modified times.
// This function only extracts files, not archive directory records.
mz_bool mz_zip_reader_extract_to_file(mz_zip_archive *pZip, mz_uint file_index, const char *pDst_filename, mz_uint flags);
mz_bool mz_zip_reader_extract_file_to_file(mz_zip_archive *pZip, const char *pArchive_filename, const char *pDst_filename, mz_uint flags);
#endif

// Ends archive reading, freeing all allocations, and closing the input archive file if mz_zip_reader_init_file() was used.
mz_bool mz_zip_reader_end(mz_zip_archive *pZip);

// ZIP archive writing

#ifndef MINIZ_NO_ARCHIVE_WRITING_APIS

// Inits a ZIP archive writer.
mz_bool mz_zip_writer_init(mz_zip_archive *pZip, mz_uint64 existing_size);
mz_bool mz_zip_writer_init_heap(mz_zip_archive *pZip, size_t size_to_reserve_at_beginning, size_t initial_allocation_size);

#ifndef MINIZ_NO_STDIO
mz_bool mz_zip_writer_init_file(mz_zip_archive *pZip, const char *pFilename, mz_uint64 size_to_reserve_at_beginning);
// Creates standard .z01, .z02, ... split volumes, with pFilename used for the final .zip volume.
// split_size must be between 64 KiB and 0xFFFFFFFF bytes.
mz_bool mz_zip_writer_init_file_split(mz_zip_archive *pZip, const char *pFilename, mz_uint64 split_size);
#endif

// Enables aggregate central-directory compression. method must be 0, MZ_DEFLATED, or MZ_LZMA when MINIZ_USE_LZMA_SDK is defined.
//
// WARNING - NON-STANDARD, MINIZ-ONLY EXTENSION: When method != 0, the resulting archive is NOT interoperable.
// A compressed central directory is stored by repurposing the ZIP64 "End of Central Directory" *version 2* record
// (APPNOTE 4.3.14), whose compression-method / compressed-size / AlgId / Hash fields are defined by PKWARE's
// proprietary Strong Encryption Specification and only ever produced there in combination with encryption. Writing
// that record with AlgId = 0 (compressed but not encrypted) is a miniz-private convention: only miniz reads it back.
// 7-Zip, WinZip, Info-ZIP unzip and Python's zipfile all reject such archives ("Bad magic number for central directory"
// / "Headers Error"). Failure is closed (readers refuse the file rather than misread it), but do not use this for
// archives that must be opened by other tools. Leave method = 0 (the default) for portable output.
mz_bool mz_zip_writer_set_central_directory_compression(mz_zip_archive *pZip, mz_uint method, mz_uint level);

// WinZip AES encryption (requires MINIZ_USE_AES). Sets the archive-wide default password and parameters used for any entry
// added with MZ_ZIP_FLAG_ENCRYPT that does not carry its own password. pPassword may be NULL to clear it.
//   aes_strength: 1=AES-128, 2=AES-192, 3=AES-256 (0 -> default 3).
//   ae_version:   1=AE-1 (stores real CRC), 2=AE-2 (stores CRC 0). 0 -> default 2.
mz_bool mz_zip_writer_set_password(mz_zip_archive *pZip, const char *pPassword, mz_uint aes_strength, mz_uint ae_version);

// Per-entry encrypted add. Same as add_mem_ex but encrypts with the given password (overriding the archive default).
// Pass level_and_flags with MZ_ZIP_FLAG_ENCRYPT set. If pPassword is NULL the archive default password is used.
mz_bool mz_zip_writer_add_mem_ex_v2(mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf, size_t buf_size, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags, mz_uint64 uncomp_size, mz_uint32 uncomp_crc32, const char *pPassword, mz_uint aes_strength, mz_uint ae_version);

// Sets the password the reader uses to decrypt WinZip AES (or legacy ZipCrypto, read-only) entries. Pass NULL to clear.
mz_bool mz_zip_reader_set_password(mz_zip_archive *pZip, const char *pPassword);

// Converts a ZIP archive reader object into a writer object, to allow efficient in-place file appends to occur on an existing archive.
// For archives opened using mz_zip_reader_init_file, pFilename must be the archive's filename so it can be reopened for writing. If the file can't be reopened, mz_zip_reader_end() will be called.
// For archives opened using mz_zip_reader_init_mem, the memory block must be growable using the realloc callback (which defaults to realloc unless you've overridden it).
// Finally, for archives opened using mz_zip_reader_init, the mz_zip_archive's user provided m_pWrite function cannot be NULL.
// Note: In-place archive modification is not recommended unless you know what you're doing, because if execution stops or something goes wrong before
// the archive is finalized the file's central directory will be hosed.
mz_bool mz_zip_writer_init_from_reader(mz_zip_archive *pZip, const char *pFilename);

// Adds the contents of a memory buffer to an archive. These functions record the current local time into the archive.
// To add a directory entry, call this method with an archive name ending in a forwardslash with empty buffer.
// level_and_flags - compression level (0-10, see MZ_BEST_SPEED, MZ_BEST_COMPRESSION, etc.) logically OR'd with zero or more mz_zip_flags, or just set to MZ_DEFAULT_COMPRESSION.
mz_bool mz_zip_writer_add_mem(mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf, size_t buf_size, mz_uint level_and_flags);
mz_bool mz_zip_writer_add_mem_ex(mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf, size_t buf_size, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags, mz_uint64 uncomp_size, mz_uint32 uncomp_crc32);

#ifndef MINIZ_NO_STDIO
// Adds the contents of a disk file to an archive. This function also records the disk file's modified time into the archive.
// level_and_flags - compression level (0-10, see MZ_BEST_SPEED, MZ_BEST_COMPRESSION, etc.) logically OR'd with zero or more mz_zip_flags, or just set to MZ_DEFAULT_COMPRESSION.
mz_bool mz_zip_writer_add_file(mz_zip_archive *pZip, const char *pArchive_name, const char *pSrc_filename, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags);
#endif

// Adds a file to an archive by fully cloning the data from another archive.
// This function fully clones the source file's compressed data (no recompression), data descriptor, filename, non-ZIP64 extra data, and comment fields. ZIP64 metadata is rebuilt for the destination.
mz_bool mz_zip_writer_add_from_zip_reader(mz_zip_archive *pZip, mz_zip_archive *pSource_zip, mz_uint file_index);

// Finalizes the archive by writing the central directory records followed by the end of central directory record.
// After an archive is finalized, the only valid call on the mz_zip_archive struct is mz_zip_writer_end().
// An archive must be manually finalized by calling this function for it to be valid.
mz_bool mz_zip_writer_finalize_archive(mz_zip_archive *pZip);
mz_bool mz_zip_writer_finalize_heap_archive(mz_zip_archive *pZip, void **pBuf, size_t *pSize);

// Ends archive writing, freeing all allocations, and closing the output file if mz_zip_writer_init_file() was used.
// Note for the archive to be valid, it must have been finalized before ending.
mz_bool mz_zip_writer_end(mz_zip_archive *pZip);

// Misc. high-level helper functions:

// mz_zip_add_mem_to_archive_file_in_place() efficiently (but not atomically) appends a memory blob to a ZIP archive.
// level_and_flags - compression level (0-10, see MZ_BEST_SPEED, MZ_BEST_COMPRESSION, etc.) logically OR'd with zero or more mz_zip_flags, or just set to MZ_DEFAULT_COMPRESSION.
mz_bool mz_zip_add_mem_to_archive_file_in_place(const char *pZip_filename, const char *pArchive_name, const void *pBuf, size_t buf_size, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags);

// Reads a single file from an archive into a heap block.
// Returns NULL on failure.
void *mz_zip_extract_archive_file_to_heap(const char *pZip_filename, const char *pArchive_name, size_t *pSize, mz_uint zip_flags);

#endif // #ifndef MINIZ_NO_ARCHIVE_WRITING_APIS

#endif // #ifndef MINIZ_NO_ARCHIVE_APIS

// ------------------- Low-level Decompression API Definitions

// Decompression flags used by tinfl_decompress().
// TINFL_FLAG_PARSE_ZLIB_HEADER: If set, the input has a valid zlib header and ends with an adler32 checksum (it's a valid zlib stream). Otherwise, the input is a raw deflate stream.
// TINFL_FLAG_HAS_MORE_INPUT: If set, there are more input bytes available beyond the end of the supplied input buffer. If clear, the input buffer contains all remaining input.
// TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF: If set, the output buffer is large enough to hold the entire decompressed stream. If clear, the output buffer is at least the size of the dictionary (typically 32KB).
// TINFL_FLAG_COMPUTE_ADLER32: Force adler-32 checksum computation of the decompressed bytes.
enum
{
  TINFL_FLAG_PARSE_ZLIB_HEADER = 1,
  TINFL_FLAG_HAS_MORE_INPUT = 2,
  TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF = 4,
  TINFL_FLAG_COMPUTE_ADLER32 = 8
};

// High level decompression functions:
// tinfl_decompress_mem_to_heap() decompresses a block in memory to a heap block allocated via malloc().
// On entry:
//  pSrc_buf, src_buf_len: Pointer and size of the Deflate or zlib source data to decompress.
// On return:
//  Function returns a pointer to the decompressed data, or NULL on failure.
//  *pOut_len will be set to the decompressed data's size, which could be larger than src_buf_len on uncompressible data.
//  The caller must call mz_free() on the returned block when it's no longer needed.
void *tinfl_decompress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags);

// tinfl_decompress_mem_to_mem() decompresses a block in memory to another block in memory.
// Returns TINFL_DECOMPRESS_MEM_TO_MEM_FAILED on failure, or the number of bytes written on success.
#define TINFL_DECOMPRESS_MEM_TO_MEM_FAILED ((size_t)(-1))
size_t tinfl_decompress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags);

// tinfl_decompress_mem_to_callback() decompresses a block in memory to an internal 32KB buffer, and a user provided callback function will be called to flush the buffer.
// Returns 1 on success or 0 on failure.
typedef int (*tinfl_put_buf_func_ptr)(const void* pBuf, int len, void *pUser);
int tinfl_decompress_mem_to_callback(const void *pIn_buf, size_t *pIn_buf_size, tinfl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

struct tinfl_decompressor_tag; typedef struct tinfl_decompressor_tag tinfl_decompressor;

// Max size of LZ dictionary.
#define TINFL_LZ_DICT_SIZE 32768

// Return status.
typedef enum
{
  TINFL_STATUS_BAD_PARAM = -3,
  TINFL_STATUS_ADLER32_MISMATCH = -2,
  TINFL_STATUS_FAILED = -1,
  TINFL_STATUS_DONE = 0,
  TINFL_STATUS_NEEDS_MORE_INPUT = 1,
  TINFL_STATUS_HAS_MORE_OUTPUT = 2
} tinfl_status;

// Initializes the decompressor to its initial state.
#define tinfl_init(r) do { (r)->m_state = 0; } MZ_MACRO_END
#define tinfl_get_adler32(r) (r)->m_check_adler32

// Main low-level decompressor coroutine function. This is the only function actually needed for decompression. All the other functions are just high-level helpers for improved usability.
// This is a universal API, i.e. it can be used as a building block to build any desired higher level decompression API. In the limit case, it can be called once per every byte input or output.
tinfl_status tinfl_decompress(tinfl_decompressor *r, const mz_uint8 *pIn_buf_next, size_t *pIn_buf_size, mz_uint8 *pOut_buf_start, mz_uint8 *pOut_buf_next, size_t *pOut_buf_size, const mz_uint32 decomp_flags);

// Internal/private bits follow.
enum
{
  TINFL_MAX_HUFF_TABLES = 3, TINFL_MAX_HUFF_SYMBOLS_0 = 288, TINFL_MAX_HUFF_SYMBOLS_1 = 32, TINFL_MAX_HUFF_SYMBOLS_2 = 19,
  TINFL_FAST_LOOKUP_BITS = 10, TINFL_FAST_LOOKUP_SIZE = 1 << TINFL_FAST_LOOKUP_BITS
};

typedef struct
{
  mz_uint8 m_code_size[TINFL_MAX_HUFF_SYMBOLS_0];
  mz_int16 m_look_up[TINFL_FAST_LOOKUP_SIZE], m_tree[TINFL_MAX_HUFF_SYMBOLS_0 * 2];
} tinfl_huff_table;

#if MINIZ_HAS_64BIT_REGISTERS
  #define TINFL_USE_64BIT_BITBUF 1
#endif

#if TINFL_USE_64BIT_BITBUF
  typedef mz_uint64 tinfl_bit_buf_t;
  #define TINFL_BITBUF_SIZE (64)
#else
  typedef mz_uint32 tinfl_bit_buf_t;
  #define TINFL_BITBUF_SIZE (32)
#endif

struct tinfl_decompressor_tag
{
  mz_uint32 m_state, m_num_bits, m_zhdr0, m_zhdr1, m_z_adler32, m_final, m_type, m_check_adler32, m_dist, m_counter, m_num_extra, m_table_sizes[TINFL_MAX_HUFF_TABLES];
  tinfl_bit_buf_t m_bit_buf;
  size_t m_dist_from_out_buf_start;
  tinfl_huff_table m_tables[TINFL_MAX_HUFF_TABLES];
  mz_uint8 m_raw_header[4], m_len_codes[TINFL_MAX_HUFF_SYMBOLS_0 + TINFL_MAX_HUFF_SYMBOLS_1 + 137];
};

// ------------------- Low-level Compression API Definitions

// Set TDEFL_LESS_MEMORY to 1 to use less memory (compression will be slightly slower, and raw/dynamic blocks will be output more frequently).
#define TDEFL_LESS_MEMORY 0

// tdefl_init() compression flags logically OR'd together (low 12 bits contain the max. number of probes per dictionary search):
// TDEFL_DEFAULT_MAX_PROBES: The compressor defaults to 128 dictionary probes per dictionary search. 0=Huffman only, 1=Huffman+LZ (fastest/crap compression), 4095=Huffman+LZ (slowest/best compression).
enum
{
  TDEFL_HUFFMAN_ONLY = 0, TDEFL_DEFAULT_MAX_PROBES = 128, TDEFL_MAX_PROBES_MASK = 0xFFF
};

// TDEFL_WRITE_ZLIB_HEADER: If set, the compressor outputs a zlib header before the deflate data, and the Adler-32 of the source data at the end. Otherwise, you'll get raw deflate data.
// TDEFL_COMPUTE_ADLER32: Always compute the adler-32 of the input data (even when not writing zlib headers).
// TDEFL_GREEDY_PARSING_FLAG: Set to use faster greedy parsing, instead of more efficient lazy parsing.
// TDEFL_NONDETERMINISTIC_PARSING_FLAG: Enable to decrease the compressor's initialization time to the minimum, but the output may vary from run to run given the same input (depending on the contents of memory).
// TDEFL_RLE_MATCHES: Only look for RLE matches (matches with a distance of 1)
// TDEFL_FILTER_MATCHES: Discards matches <= 5 chars if enabled.
// TDEFL_FORCE_ALL_STATIC_BLOCKS: Disable usage of optimized Huffman tables.
// TDEFL_FORCE_ALL_RAW_BLOCKS: Only use raw (uncompressed) deflate blocks.
// The low 12 bits are reserved to control the max # of hash probes per dictionary lookup (see TDEFL_MAX_PROBES_MASK).
enum
{
  TDEFL_WRITE_ZLIB_HEADER             = 0x01000,
  TDEFL_COMPUTE_ADLER32               = 0x02000,
  TDEFL_GREEDY_PARSING_FLAG           = 0x04000,
  TDEFL_NONDETERMINISTIC_PARSING_FLAG = 0x08000,
  TDEFL_RLE_MATCHES                   = 0x10000,
  TDEFL_FILTER_MATCHES                = 0x20000,
  TDEFL_FORCE_ALL_STATIC_BLOCKS       = 0x40000,
  TDEFL_FORCE_ALL_RAW_BLOCKS          = 0x80000
};

// High level compression functions:
// tdefl_compress_mem_to_heap() compresses a block in memory to a heap block allocated via malloc().
// On entry:
//  pSrc_buf, src_buf_len: Pointer and size of source block to compress.
//  flags: The max match finder probes (default is 128) logically OR'd against the above flags. Higher probes are slower but improve compression.
// On return:
//  Function returns a pointer to the compressed data, or NULL on failure.
//  *pOut_len will be set to the compressed data's size, which could be larger than src_buf_len on uncompressible data.
//  The caller must free() the returned block when it's no longer needed.
void *tdefl_compress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags);

// tdefl_compress_mem_to_mem() compresses a block in memory to another block in memory.
// Returns 0 on failure.
size_t tdefl_compress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags);

// Compresses an image to a compressed PNG file in memory.
// On entry:
//  pImage, w, h, and num_chans describe the image to compress. num_chans may be 1, 2, 3, or 4.
//  The image pitch in bytes per scanline will be w*num_chans. The leftmost pixel on the top scanline is stored first in memory.
//  level may range from [0,10], use MZ_NO_COMPRESSION, MZ_BEST_SPEED, MZ_BEST_COMPRESSION, etc. or a decent default is MZ_DEFAULT_LEVEL
//  If flip is true, the image will be flipped on the Y axis (useful for OpenGL apps).
// On return:
//  Function returns a pointer to the compressed data, or NULL on failure.
//  *pLen_out will be set to the size of the PNG image file.
//  The caller must mz_free() the returned heap block (which will typically be larger than *pLen_out) when it's no longer needed.
void *tdefl_write_image_to_png_file_in_memory_ex(const void *pImage, int w, int h, int num_chans, size_t *pLen_out, mz_uint level, mz_bool flip);
void *tdefl_write_image_to_png_file_in_memory(const void *pImage, int w, int h, int num_chans, size_t *pLen_out);

// Output stream interface. The compressor uses this interface to write compressed data. It'll typically be called TDEFL_OUT_BUF_SIZE at a time.
typedef mz_bool (*tdefl_put_buf_func_ptr)(const void* pBuf, int len, void *pUser);

// tdefl_compress_mem_to_output() compresses a block to an output stream. The above helpers use this function internally.
mz_bool tdefl_compress_mem_to_output(const void *pBuf, size_t buf_len, tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

enum { TDEFL_MAX_HUFF_TABLES = 3, TDEFL_MAX_HUFF_SYMBOLS_0 = 288, TDEFL_MAX_HUFF_SYMBOLS_1 = 32, TDEFL_MAX_HUFF_SYMBOLS_2 = 19, TDEFL_LZ_DICT_SIZE = 32768, TDEFL_LZ_DICT_SIZE_MASK = TDEFL_LZ_DICT_SIZE - 1, TDEFL_MIN_MATCH_LEN = 3, TDEFL_MAX_MATCH_LEN = 258 };


// TDEFL_OUT_BUF_SIZE MUST be large enough to hold a single entire compressed output block (using static/fixed Huffman codes).
#if TDEFL_LESS_MEMORY
//enum { TDEFL_LZ_CODE_BUF_SIZE = 24 * 1024, TDEFL_OUT_BUF_SIZE = (TDEFL_LZ_CODE_BUF_SIZE * 13 ) / 10, TDEFL_MAX_HUFF_SYMBOLS = 288, TDEFL_LZ_HASH_BITS = 12, TDEFL_LEVEL1_HASH_SIZE_MASK = 4095, TDEFL_LZ_HASH_SHIFT = (TDEFL_LZ_HASH_BITS + 2) / 3, TDEFL_LZ_HASH_SIZE = 1 << TDEFL_LZ_HASH_BITS };
#define TDEFL_LZ_CODE_BUF_SIZE (24 * 1024)
#define TDEFL_OUT_BUF_SIZE ((TDEFL_LZ_CODE_BUF_SIZE * 13 ) / 10)
#define TDEFL_MAX_HUFF_SYMBOLS (288)
#define TDEFL_LZ_HASH_BITS (12)
#define TDEFL_LEVEL1_HASH_SIZE_MASK (4095)
#define TDEFL_LZ_HASH_SHIFT ((TDEFL_LZ_HASH_BITS + 2) / 3)
#define TDEFL_LZ_HASH_SIZE (1 << TDEFL_LZ_HASH_BITS)
#else
//enum { TDEFL_LZ_CODE_BUF_SIZE = 64 * 1024, TDEFL_OUT_BUF_SIZE = (TDEFL_LZ_CODE_BUF_SIZE * 13 ) / 10, TDEFL_MAX_HUFF_SYMBOLS = 288, TDEFL_LZ_HASH_BITS = 15, TDEFL_LEVEL1_HASH_SIZE_MASK = 4095, TDEFL_LZ_HASH_SHIFT = (TDEFL_LZ_HASH_BITS + 2) / 3, TDEFL_LZ_HASH_SIZE = 1 << TDEFL_LZ_HASH_BITS };
#define TDEFL_LZ_CODE_BUF_SIZE (64 * 1024)
#define TDEFL_OUT_BUF_SIZE ((TDEFL_LZ_CODE_BUF_SIZE * 13 ) / 10)
#define TDEFL_MAX_HUFF_SYMBOLS (288)
#define TDEFL_LZ_HASH_BITS (15)
#define TDEFL_LEVEL1_HASH_SIZE_MASK (4095)
#define TDEFL_LZ_HASH_SHIFT ((TDEFL_LZ_HASH_BITS + 2) / 3)
#define TDEFL_LZ_HASH_SIZE (1 << TDEFL_LZ_HASH_BITS)
#endif

// The low-level tdefl functions below may be used directly if the above helper functions aren't flexible enough. The low-level functions don't make any heap allocations, unlike the above helper functions.
typedef enum
{
  TDEFL_STATUS_BAD_PARAM = -2,
  TDEFL_STATUS_PUT_BUF_FAILED = -1,
  TDEFL_STATUS_OKAY = 0,
  TDEFL_STATUS_DONE = 1
} tdefl_status;

// Must map to MZ_NO_FLUSH, MZ_SYNC_FLUSH, etc. enums
typedef enum
{
  TDEFL_NO_FLUSH = 0,
  TDEFL_SYNC_FLUSH = 2,
  TDEFL_FULL_FLUSH = 3,
  TDEFL_FINISH = 4
} tdefl_flush;

// tdefl's compression state structure.
typedef struct
{
  tdefl_put_buf_func_ptr m_pPut_buf_func;
  void *m_pPut_buf_user;
  mz_uint m_flags, m_max_probes[2];
  int m_greedy_parsing;
  mz_uint m_adler32, m_lookahead_pos, m_lookahead_size, m_dict_size;
  mz_uint8 *m_pLZ_code_buf, *m_pLZ_flags, *m_pOutput_buf, *m_pOutput_buf_end;
  mz_uint m_num_flags_left, m_total_lz_bytes, m_lz_code_buf_dict_pos, m_bits_in, m_bit_buffer;
  mz_uint m_saved_match_dist, m_saved_match_len, m_saved_lit, m_output_flush_ofs, m_output_flush_remaining, m_finished, m_block_index, m_wants_to_finish;
  tdefl_status m_prev_return_status;
  const void *m_pIn_buf;
  void *m_pOut_buf;
  size_t *m_pIn_buf_size, *m_pOut_buf_size;
  tdefl_flush m_flush;
  const mz_uint8 *m_pSrc;
  size_t m_src_buf_left, m_out_buf_ofs;
  mz_uint8 m_dict[TDEFL_LZ_DICT_SIZE + TDEFL_MAX_MATCH_LEN - 1];
  mz_uint16 m_huff_count[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
  mz_uint16 m_huff_codes[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
  mz_uint8 m_huff_code_sizes[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
  mz_uint8 m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE];
  mz_uint16 m_next[TDEFL_LZ_DICT_SIZE];
  mz_uint16 m_hash[TDEFL_LZ_HASH_SIZE];
  mz_uint8 m_output_buf[TDEFL_OUT_BUF_SIZE];
} tdefl_compressor;

// Initializes the compressor.
// There is no corresponding deinit() function because the tdefl API's do not dynamically allocate memory.
// pBut_buf_func: If NULL, output data will be supplied to the specified callback. In this case, the user should call the tdefl_compress_buffer() API for compression.
// If pBut_buf_func is NULL the user should always call the tdefl_compress() API.
// flags: See the above enums (TDEFL_HUFFMAN_ONLY, TDEFL_WRITE_ZLIB_HEADER, etc.)
tdefl_status tdefl_init(tdefl_compressor *d, tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

// Compresses a block of data, consuming as much of the specified input buffer as possible, and writing as much compressed data to the specified output buffer as possible.
tdefl_status tdefl_compress(tdefl_compressor *d, const void *pIn_buf, size_t *pIn_buf_size, void *pOut_buf, size_t *pOut_buf_size, tdefl_flush flush);

// tdefl_compress_buffer() is only usable when the tdefl_init() is called with a non-NULL tdefl_put_buf_func_ptr.
// tdefl_compress_buffer() always consumes the entire input buffer.
tdefl_status tdefl_compress_buffer(tdefl_compressor *d, const void *pIn_buf, size_t in_buf_size, tdefl_flush flush);

tdefl_status tdefl_get_prev_return_status(tdefl_compressor *d);
mz_uint32 tdefl_get_adler32(tdefl_compressor *d);

// Can't use tdefl_create_comp_flags_from_zip_params if MINIZ_NO_ZLIB_APIS isn't defined, because it uses some of its macros.
#ifndef MINIZ_NO_ZLIB_APIS
// Create tdefl_compress() flags given zlib-style compression parameters.
// level may range from [0,10] (where 10 is absolute max compression, but may be much slower on some files)
// window_bits may be -15 (raw deflate) or 15 (zlib)
// strategy may be either MZ_DEFAULT_STRATEGY, MZ_FILTERED, MZ_HUFFMAN_ONLY, MZ_RLE, or MZ_FIXED
mz_uint tdefl_create_comp_flags_from_zip_params(int level, int window_bits, int strategy);
#endif // #ifndef MINIZ_NO_ZLIB_APIS

#ifdef __cplusplus
}
#endif

#endif // MINIZ_HEADER_INCLUDED

// ------------------- End of Header: Implementation follows. (If you only want the header, define MINIZ_HEADER_FILE_ONLY.)

#ifndef MINIZ_HEADER_FILE_ONLY

typedef unsigned char mz_validate_uint16[sizeof(mz_uint16)==2 ? 1 : -1];
typedef unsigned char mz_validate_uint32[sizeof(mz_uint32)==4 ? 1 : -1];
typedef unsigned char mz_validate_uint64[sizeof(mz_uint64)==8 ? 1 : -1];

#include <string.h>

#ifndef MZ_ASSERT
	#include <assert.h>
	#define MZ_ASSERT(x) assert(x)
#endif

#ifdef MINIZ_NO_MALLOC
  #define MZ_MALLOC(x) NULL
  #define MZ_FREE(x) (void)x, ((void)0)
  #define MZ_REALLOC(p, x) NULL
#else
  #define MZ_MALLOC(x) malloc(x)
  #define MZ_FREE(x) free(x)
  #define MZ_REALLOC(p, x) realloc(p, x)
#endif

#define MZ_MAX(a,b) (((a)>(b))?(a):(b))
#define MZ_MIN(a,b) (((a)<(b))?(a):(b))
#define MZ_CLEAR_OBJ(obj) memset(&(obj), 0, sizeof(obj))

#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN
  #define MZ_READ_LE16(p) *((const mz_uint16 *)(p))
  #define MZ_READ_LE32(p) *((const mz_uint32 *)(p))
#else
  #define MZ_READ_LE16(p) ((mz_uint32)(((const mz_uint8 *)(p))[0]) | ((mz_uint32)(((const mz_uint8 *)(p))[1]) << 8U))
  #define MZ_READ_LE32(p) ((mz_uint32)(((const mz_uint8 *)(p))[0]) | ((mz_uint32)(((const mz_uint8 *)(p))[1]) << 8U) | ((mz_uint32)(((const mz_uint8 *)(p))[2]) << 16U) | ((mz_uint32)(((const mz_uint8 *)(p))[3]) << 24U))
#endif
#define MZ_READ_LE64(p) (((mz_uint64)MZ_READ_LE32(p)) | (((mz_uint64)MZ_READ_LE32(((const mz_uint8 *)(p)) + 4)) << 32U))

#ifdef _MSC_VER
  #define MZ_FORCEINLINE __forceinline
#elif defined(__GNUC__)
  #define MZ_FORCEINLINE inline __attribute__((__always_inline__))
#else
  #define MZ_FORCEINLINE inline
#endif

#ifdef __cplusplus
  extern "C" {
#endif

// ------------------- zlib-style API's

mz_ulong mz_adler32(mz_ulong adler, const unsigned char *ptr, size_t buf_len)
{
  mz_uint32 i, s1 = (mz_uint32)(adler & 0xffff), s2 = (mz_uint32)(adler >> 16); size_t block_len = buf_len % 5552;
  if (!ptr) return MZ_ADLER32_INIT;
  while (buf_len) {
	for (i = 0; i + 7 < block_len; i += 8, ptr += 8) {
	  s1 += ptr[0], s2 += s1; s1 += ptr[1], s2 += s1; s1 += ptr[2], s2 += s1; s1 += ptr[3], s2 += s1;
	  s1 += ptr[4], s2 += s1; s1 += ptr[5], s2 += s1; s1 += ptr[6], s2 += s1; s1 += ptr[7], s2 += s1;
	}
	for ( ; i < block_len; ++i) s1 += *ptr++, s2 += s1;
	s1 %= 65521U, s2 %= 65521U; buf_len -= block_len; block_len = 5552;
  }
  return (s2 << 16) + s1;
}

// Karl Malbrain's compact CRC-32. See "A compact CCITT crc16 and crc32 C implementation that balances processor cache usage against speed": http://www.geocities.com/malbrain/
mz_ulong mz_crc32(mz_ulong crc, const mz_uint8 *ptr, size_t buf_len)
{
  static const mz_uint32 s_crc32[16] = { 0, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
	0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };
  mz_uint32 crcu32 = (mz_uint32)crc;
  if (!ptr) return MZ_CRC32_INIT;
  crcu32 = ~crcu32; while (buf_len--) { mz_uint8 b = *ptr++; crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b & 0xF)]; crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b >> 4)]; }
  return ~crcu32;
}
#if 0
uint crc32(byte *data, int size)
{
	uint r = ~0; byte *end = data + size;

	while(data < end)
	{
		r ^= *data++;

		for(int i = 0; i < 8; i++)
		{
			uint t = ~((r&1) - 1); r = (r>>1) ^ (0xEDB88320 & t);
		}
	}

	return ~r;
}
#endif

void mz_free(void *p)
{
  MZ_FREE(p);
}

#ifndef MINIZ_NO_ZLIB_APIS

static void *def_alloc_func(void *opaque, size_t items, size_t size) { (void)opaque, (void)items, (void)size; return MZ_MALLOC(items * size); }
static void def_free_func(void *opaque, void *address) { (void)opaque, (void)address; MZ_FREE(address); }
static void *def_realloc_func(void *opaque, void *address, size_t items, size_t size) { (void)opaque, (void)address, (void)items, (void)size; return MZ_REALLOC(address, items * size); }

const char *mz_version(void)
{
  return MZ_VERSION;
}

int mz_deflateInit(mz_streamp pStream, int level)
{
  return mz_deflateInit2(pStream, level, MZ_DEFLATED, MZ_DEFAULT_WINDOW_BITS, 9, MZ_DEFAULT_STRATEGY);
}

int mz_deflateInit2(mz_streamp pStream, int level, int method, int window_bits, int mem_level, int strategy)
{
  tdefl_compressor *pComp;
  mz_uint comp_flags = TDEFL_COMPUTE_ADLER32 | tdefl_create_comp_flags_from_zip_params(level, window_bits, strategy);

  if (!pStream) return MZ_STREAM_ERROR;
  if ((method != MZ_DEFLATED) || ((mem_level < 1) || (mem_level > 9)) || ((window_bits != MZ_DEFAULT_WINDOW_BITS) && (-window_bits != MZ_DEFAULT_WINDOW_BITS))) return MZ_PARAM_ERROR;

  pStream->data_type = 0;
  pStream->adler = MZ_ADLER32_INIT;
  pStream->msg = NULL;
  pStream->reserved = 0;
  pStream->total_in = 0;
  pStream->total_out = 0;
  if (!pStream->zalloc) pStream->zalloc = def_alloc_func;
  if (!pStream->zfree) pStream->zfree = def_free_func;

  pComp = (tdefl_compressor *)pStream->zalloc(pStream->opaque, 1, sizeof(tdefl_compressor));
  if (!pComp)
	return MZ_MEM_ERROR;

  pStream->state = (struct mz_internal_state *)pComp;

  if (tdefl_init(pComp, NULL, NULL, comp_flags) != TDEFL_STATUS_OKAY)
  {
	mz_deflateEnd(pStream);
	return MZ_PARAM_ERROR;
  }

  return MZ_OK;
}

int mz_deflateReset(mz_streamp pStream)
{
  if ((!pStream) || (!pStream->state) || (!pStream->zalloc) || (!pStream->zfree)) return MZ_STREAM_ERROR;
  pStream->total_in = pStream->total_out = 0;
  tdefl_init((tdefl_compressor*)pStream->state, NULL, NULL, ((tdefl_compressor*)pStream->state)->m_flags);
  return MZ_OK;
}

int mz_deflate(mz_streamp pStream, int flush)
{
  size_t in_bytes, out_bytes;
  mz_ulong orig_total_in, orig_total_out;
  int mz_status = MZ_OK;

  if ((!pStream) || (!pStream->state) || (flush < 0) || (flush > MZ_FINISH) || (!pStream->next_out)) return MZ_STREAM_ERROR;
  if (!pStream->avail_out) return MZ_BUF_ERROR;

  if (flush == MZ_PARTIAL_FLUSH) flush = MZ_SYNC_FLUSH;

  if (((tdefl_compressor*)pStream->state)->m_prev_return_status == TDEFL_STATUS_DONE)
	return (flush == MZ_FINISH) ? MZ_STREAM_END : MZ_BUF_ERROR;

  orig_total_in = pStream->total_in; orig_total_out = pStream->total_out;
  for ( ; ; )
  {
	tdefl_status defl_status;
	in_bytes = pStream->avail_in; out_bytes = pStream->avail_out;

	defl_status = tdefl_compress((tdefl_compressor*)pStream->state, pStream->next_in, &in_bytes, pStream->next_out, &out_bytes, (tdefl_flush)flush);
	pStream->next_in += (mz_uint)in_bytes; pStream->avail_in -= (mz_uint)in_bytes;
	pStream->total_in += (mz_uint)in_bytes; pStream->adler = tdefl_get_adler32((tdefl_compressor*)pStream->state);

	pStream->next_out += (mz_uint)out_bytes; pStream->avail_out -= (mz_uint)out_bytes;
	pStream->total_out += (mz_uint)out_bytes;

	if (defl_status < 0)
	{
	  mz_status = MZ_STREAM_ERROR;
	  break;
	}
	else if (defl_status == TDEFL_STATUS_DONE)
	{
	  mz_status = MZ_STREAM_END;
	  break;
	}
	else if (!pStream->avail_out)
	  break;
	else if ((!pStream->avail_in) && (flush != MZ_FINISH))
	{
	  if ((flush) || (pStream->total_in != orig_total_in) || (pStream->total_out != orig_total_out))
		break;
	  return MZ_BUF_ERROR; // Can't make forward progress without some input.
	}
  }
  return mz_status;
}

int mz_deflateEnd(mz_streamp pStream)
{
  if (!pStream) return MZ_STREAM_ERROR;
  if (pStream->state)
  {
	pStream->zfree(pStream->opaque, pStream->state);
	pStream->state = NULL;
  }
  return MZ_OK;
}

mz_ulong mz_deflateBound(mz_streamp pStream, mz_ulong source_len)
{
  (void)pStream;
  // This is really over conservative. (And lame, but it's actually pretty tricky to compute a true upper bound given the way tdefl's blocking works.)
  return MZ_MAX(128 + (source_len * 110) / 100, 128 + source_len + ((source_len / (31 * 1024)) + 1) * 5);
}

int mz_compress2(unsigned char *pDest, mz_ulong *pDest_len, const unsigned char *pSource, mz_ulong source_len, int level)
{
  int status;
  mz_stream stream;
  memset(&stream, 0, sizeof(stream));

  // In case mz_ulong is 64-bits (argh I hate longs).
  if ((source_len | *pDest_len) > 0xFFFFFFFFU) return MZ_PARAM_ERROR;

  stream.next_in = pSource;
  stream.avail_in = (mz_uint32)source_len;
  stream.next_out = pDest;
  stream.avail_out = (mz_uint32)*pDest_len;

  status = mz_deflateInit(&stream, level);
  if (status != MZ_OK) return status;

  status = mz_deflate(&stream, MZ_FINISH);
  if (status != MZ_STREAM_END)
  {
	mz_deflateEnd(&stream);
	return (status == MZ_OK) ? MZ_BUF_ERROR : status;
  }

  *pDest_len = stream.total_out;
  return mz_deflateEnd(&stream);
}

int mz_compress(unsigned char *pDest, mz_ulong *pDest_len, const unsigned char *pSource, mz_ulong source_len)
{
  return mz_compress2(pDest, pDest_len, pSource, source_len, MZ_DEFAULT_COMPRESSION);
}

mz_ulong mz_compressBound(mz_ulong source_len)
{
  return mz_deflateBound(NULL, source_len);
}

typedef struct
{
  tinfl_decompressor m_decomp;
  mz_uint m_dict_ofs, m_dict_avail, m_first_call, m_has_flushed; int m_window_bits;
  mz_uint8 m_dict[TINFL_LZ_DICT_SIZE];
  tinfl_status m_last_status;
} inflate_state;

int mz_inflateInit2(mz_streamp pStream, int window_bits)
{
  inflate_state *pDecomp;
  if (!pStream) return MZ_STREAM_ERROR;
  if ((window_bits != MZ_DEFAULT_WINDOW_BITS) && (-window_bits != MZ_DEFAULT_WINDOW_BITS)) return MZ_PARAM_ERROR;

  pStream->data_type = 0;
  pStream->adler = 0;
  pStream->msg = NULL;
  pStream->total_in = 0;
  pStream->total_out = 0;
  pStream->reserved = 0;
  if (!pStream->zalloc) pStream->zalloc = def_alloc_func;
  if (!pStream->zfree) pStream->zfree = def_free_func;

  pDecomp = (inflate_state*)pStream->zalloc(pStream->opaque, 1, sizeof(inflate_state));
  if (!pDecomp) return MZ_MEM_ERROR;

  pStream->state = (struct mz_internal_state *)pDecomp;

  tinfl_init(&pDecomp->m_decomp);
  pDecomp->m_dict_ofs = 0;
  pDecomp->m_dict_avail = 0;
  pDecomp->m_last_status = TINFL_STATUS_NEEDS_MORE_INPUT;
  pDecomp->m_first_call = 1;
  pDecomp->m_has_flushed = 0;
  pDecomp->m_window_bits = window_bits;

  return MZ_OK;
}

int mz_inflateInit(mz_streamp pStream)
{
   return mz_inflateInit2(pStream, MZ_DEFAULT_WINDOW_BITS);
}

int mz_inflate(mz_streamp pStream, int flush)
{
  inflate_state* pState;
  mz_uint n, first_call, decomp_flags = TINFL_FLAG_COMPUTE_ADLER32;
  size_t in_bytes, out_bytes, orig_avail_in;
  tinfl_status status;

  if ((!pStream) || (!pStream->state)) return MZ_STREAM_ERROR;
  if (flush == MZ_PARTIAL_FLUSH) flush = MZ_SYNC_FLUSH;
  if ((flush) && (flush != MZ_SYNC_FLUSH) && (flush != MZ_FINISH)) return MZ_STREAM_ERROR;

  pState = (inflate_state*)pStream->state;
  if (pState->m_window_bits > 0) decomp_flags |= TINFL_FLAG_PARSE_ZLIB_HEADER;
  orig_avail_in = pStream->avail_in;

  first_call = pState->m_first_call; pState->m_first_call = 0;
  if (pState->m_last_status < 0) return MZ_DATA_ERROR;

  if (pState->m_has_flushed && (flush != MZ_FINISH)) return MZ_STREAM_ERROR;
  pState->m_has_flushed |= (flush == MZ_FINISH);

  if ((flush == MZ_FINISH) && (first_call))
  {
	// MZ_FINISH on the first call implies that the input and output buffers are large enough to hold the entire compressed/decompressed file.
	decomp_flags |= TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF;
	in_bytes = pStream->avail_in; out_bytes = pStream->avail_out;
	status = tinfl_decompress(&pState->m_decomp, pStream->next_in, &in_bytes, pStream->next_out, pStream->next_out, &out_bytes, decomp_flags);
	pState->m_last_status = status;
	pStream->next_in += (mz_uint)in_bytes; pStream->avail_in -= (mz_uint)in_bytes; pStream->total_in += (mz_uint)in_bytes;
	pStream->adler = tinfl_get_adler32(&pState->m_decomp);
	pStream->next_out += (mz_uint)out_bytes; pStream->avail_out -= (mz_uint)out_bytes; pStream->total_out += (mz_uint)out_bytes;

	if (status < 0)
	  return MZ_DATA_ERROR;
	else if (status != TINFL_STATUS_DONE)
	{
	  pState->m_last_status = TINFL_STATUS_FAILED;
	  return MZ_BUF_ERROR;
	}
	return MZ_STREAM_END;
  }
  // flush != MZ_FINISH then we must assume there's more input.
  if (flush != MZ_FINISH) decomp_flags |= TINFL_FLAG_HAS_MORE_INPUT;

  if (pState->m_dict_avail)
  {
	n = MZ_MIN(pState->m_dict_avail, pStream->avail_out);
	memcpy(pStream->next_out, pState->m_dict + pState->m_dict_ofs, n);
	pStream->next_out += n; pStream->avail_out -= n; pStream->total_out += n;
	pState->m_dict_avail -= n; pState->m_dict_ofs = (pState->m_dict_ofs + n) & (TINFL_LZ_DICT_SIZE - 1);
	return ((pState->m_last_status == TINFL_STATUS_DONE) && (!pState->m_dict_avail)) ? MZ_STREAM_END : MZ_OK;
  }

  for ( ; ; )
  {
	in_bytes = pStream->avail_in;
	out_bytes = TINFL_LZ_DICT_SIZE - pState->m_dict_ofs;

	status = tinfl_decompress(&pState->m_decomp, pStream->next_in, &in_bytes, pState->m_dict, pState->m_dict + pState->m_dict_ofs, &out_bytes, decomp_flags);
	pState->m_last_status = status;

	pStream->next_in += (mz_uint)in_bytes; pStream->avail_in -= (mz_uint)in_bytes;
	pStream->total_in += (mz_uint)in_bytes; pStream->adler = tinfl_get_adler32(&pState->m_decomp);

	pState->m_dict_avail = (mz_uint)out_bytes;

	n = MZ_MIN(pState->m_dict_avail, pStream->avail_out);
	memcpy(pStream->next_out, pState->m_dict + pState->m_dict_ofs, n);
	pStream->next_out += n; pStream->avail_out -= n; pStream->total_out += n;
	pState->m_dict_avail -= n; pState->m_dict_ofs = (pState->m_dict_ofs + n) & (TINFL_LZ_DICT_SIZE - 1);

	if (status < 0)
	   return MZ_DATA_ERROR; // Stream is corrupted (there could be some uncompressed data left in the output dictionary - oh well).
	else if ((status == TINFL_STATUS_NEEDS_MORE_INPUT) && (!orig_avail_in))
	  return MZ_BUF_ERROR; // Signal caller that we can't make forward progress without supplying more input or by setting flush to MZ_FINISH.
	else if (flush == MZ_FINISH)
	{
	   // The output buffer MUST be large to hold the remaining uncompressed data when flush==MZ_FINISH.
	   if (status == TINFL_STATUS_DONE)
		  return pState->m_dict_avail ? MZ_BUF_ERROR : MZ_STREAM_END;
	   // status here must be TINFL_STATUS_HAS_MORE_OUTPUT, which means there's at least 1 more byte on the way. If there's no more room left in the output buffer then something is wrong.
	   else if (!pStream->avail_out)
		  return MZ_BUF_ERROR;
	}
	else if ((status == TINFL_STATUS_DONE) || (!pStream->avail_in) || (!pStream->avail_out) || (pState->m_dict_avail))
	  break;
  }

  return ((status == TINFL_STATUS_DONE) && (!pState->m_dict_avail)) ? MZ_STREAM_END : MZ_OK;
}

int mz_inflateEnd(mz_streamp pStream)
{
  if (!pStream)
	return MZ_STREAM_ERROR;
  if (pStream->state)
  {
	pStream->zfree(pStream->opaque, pStream->state);
	pStream->state = NULL;
  }
  return MZ_OK;
}

int mz_uncompress(unsigned char *pDest, mz_ulong *pDest_len, const unsigned char *pSource, mz_ulong source_len)
{
  mz_stream stream;
  int status;
  memset(&stream, 0, sizeof(stream));

  // In case mz_ulong is 64-bits (argh I hate longs).
  if ((source_len | *pDest_len) > 0xFFFFFFFFU) return MZ_PARAM_ERROR;

  stream.next_in = pSource;
  stream.avail_in = (mz_uint32)source_len;
  stream.next_out = pDest;
  stream.avail_out = (mz_uint32)*pDest_len;

  status = mz_inflateInit(&stream);
  if (status != MZ_OK)
	return status;

  status = mz_inflate(&stream, MZ_FINISH);
  if (status != MZ_STREAM_END)
  {
	mz_inflateEnd(&stream);
	return ((status == MZ_BUF_ERROR) && (!stream.avail_in)) ? MZ_DATA_ERROR : status;
  }
  *pDest_len = stream.total_out;

  return mz_inflateEnd(&stream);
}

const char *mz_error(int err)
{
  static struct { int m_err; const char *m_pDesc; } s_error_descs[] =
  {
	{ MZ_OK, "" }, { MZ_STREAM_END, "stream end" }, { MZ_NEED_DICT, "need dictionary" }, { MZ_ERRNO, "file error" }, { MZ_STREAM_ERROR, "stream error" },
	{ MZ_DATA_ERROR, "data error" }, { MZ_MEM_ERROR, "out of memory" }, { MZ_BUF_ERROR, "buf error" }, { MZ_VERSION_ERROR, "version error" }, { MZ_PARAM_ERROR, "parameter error" }
  };
  mz_uint i; for (i = 0; i < sizeof(s_error_descs) / sizeof(s_error_descs[0]); ++i) if (s_error_descs[i].m_err == err) return s_error_descs[i].m_pDesc;
  return NULL;
}

#endif //MINIZ_NO_ZLIB_APIS

// ------------------- Low-level Decompression (completely independent from all compression API's)

#define TINFL_MEMCPY(d, s, l) memcpy(d, s, l)
#define TINFL_MEMSET(p, c, l) memset(p, c, l)

#define TINFL_CR_BEGIN switch(r->m_state) { case 0:
#define TINFL_CR_RETURN(state_index, result) do { status = result; r->m_state = state_index; goto common_exit; case state_index:; } MZ_MACRO_END
#define TINFL_CR_RETURN_FOREVER(state_index, result) do { for ( ; ; ) { TINFL_CR_RETURN(state_index, result); } } MZ_MACRO_END
#define TINFL_CR_FINISH }

// TODO: If the caller has indicated that there's no more input, and we attempt to read beyond the input buf, then something is wrong with the input because the inflator never
// reads ahead more than it needs to. Currently TINFL_GET_BYTE() pads the end of the stream with 0's in this scenario.
#define TINFL_GET_BYTE(state_index, c) do { \
  if (pIn_buf_cur >= pIn_buf_end) { \
	for ( ; ; ) { \
	  if (decomp_flags & TINFL_FLAG_HAS_MORE_INPUT) { \
		TINFL_CR_RETURN(state_index, TINFL_STATUS_NEEDS_MORE_INPUT); \
		if (pIn_buf_cur < pIn_buf_end) { \
		  c = *pIn_buf_cur++; \
		  break; \
		} \
	  } else { \
		c = 0; \
		break; \
	  } \
	} \
  } else c = *pIn_buf_cur++; } MZ_MACRO_END

#define TINFL_NEED_BITS(state_index, n) do { mz_uint c; TINFL_GET_BYTE(state_index, c); bit_buf |= (((tinfl_bit_buf_t)c) << num_bits); num_bits += 8; } while (num_bits < (mz_uint)(n))
#define TINFL_SKIP_BITS(state_index, n) do { if (num_bits < (mz_uint)(n)) { TINFL_NEED_BITS(state_index, n); } bit_buf >>= (n); num_bits -= (n); } MZ_MACRO_END
#define TINFL_GET_BITS(state_index, b, n) do { if (num_bits < (mz_uint)(n)) { TINFL_NEED_BITS(state_index, n); } b = bit_buf & ((1 << (n)) - 1); bit_buf >>= (n); num_bits -= (n); } MZ_MACRO_END

// TINFL_HUFF_BITBUF_FILL() is only used rarely, when the number of bytes remaining in the input buffer falls below 2.
// It reads just enough bytes from the input stream that are needed to decode the next Huffman code (and absolutely no more). It works by trying to fully decode a
// Huffman code by using whatever bits are currently present in the bit buffer. If this fails, it reads another byte, and tries again until it succeeds or until the
// bit buffer contains >=15 bits (deflate's max. Huffman code size).
#define TINFL_HUFF_BITBUF_FILL(state_index, pHuff) \
  do { \
	temp = (pHuff)->m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]; \
	if (temp >= 0) { \
	  code_len = temp >> 9; \
	  if ((code_len) && (num_bits >= code_len)) \
	  break; \
	} else if (num_bits > TINFL_FAST_LOOKUP_BITS) { \
	   code_len = TINFL_FAST_LOOKUP_BITS; \
	   do { \
		  temp = (pHuff)->m_tree[~temp + ((bit_buf >> code_len++) & 1)]; \
	   } while ((temp < 0) && (num_bits >= (code_len + 1))); if (temp >= 0) break; \
	} TINFL_GET_BYTE(state_index, c); bit_buf |= (((tinfl_bit_buf_t)c) << num_bits); num_bits += 8; \
  } while (num_bits < 15);

// TINFL_HUFF_DECODE() decodes the next Huffman coded symbol. It's more complex than you would initially expect because the zlib API expects the decompressor to never read
// beyond the final byte of the deflate stream. (In other words, when this macro wants to read another byte from the input, it REALLY needs another byte in order to fully
// decode the next Huffman code.) Handling this properly is particularly important on raw deflate (non-zlib) streams, which aren't followed by a byte aligned adler-32.
// The slow path is only executed at the very end of the input buffer.
#define TINFL_HUFF_DECODE(state_index, sym, pHuff) do { \
  int temp; mz_uint code_len, c; \
  if (num_bits < 15) { \
	if ((pIn_buf_end - pIn_buf_cur) < 2) { \
	   TINFL_HUFF_BITBUF_FILL(state_index, pHuff); \
	} else { \
	   bit_buf |= (((tinfl_bit_buf_t)pIn_buf_cur[0]) << num_bits) | (((tinfl_bit_buf_t)pIn_buf_cur[1]) << (num_bits + 8)); pIn_buf_cur += 2; num_bits += 16; \
	} \
  } \
  if ((temp = (pHuff)->m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0) \
	code_len = temp >> 9, temp &= 511; \
  else { \
	code_len = TINFL_FAST_LOOKUP_BITS; do { temp = (pHuff)->m_tree[~temp + ((bit_buf >> code_len++) & 1)]; } while (temp < 0); \
  } sym = temp; bit_buf >>= code_len; num_bits -= code_len; } MZ_MACRO_END

tinfl_status tinfl_decompress(tinfl_decompressor *r, const mz_uint8 *pIn_buf_next, size_t *pIn_buf_size, mz_uint8 *pOut_buf_start, mz_uint8 *pOut_buf_next, size_t *pOut_buf_size, const mz_uint32 decomp_flags)
{
  static const int s_length_base[31] = { 3,4,5,6,7,8,9,10,11,13, 15,17,19,23,27,31,35,43,51,59, 67,83,99,115,131,163,195,227,258,0,0 };
  static const int s_length_extra[31]= { 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0 };
  static const int s_dist_base[32] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193, 257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};
  static const int s_dist_extra[32] = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
  static const mz_uint8 s_length_dezigzag[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
  static const int s_min_table_sizes[3] = { 257, 1, 4 };

  tinfl_status status = TINFL_STATUS_FAILED; mz_uint32 num_bits, dist, counter, num_extra; tinfl_bit_buf_t bit_buf;
  const mz_uint8 *pIn_buf_cur = pIn_buf_next, *const pIn_buf_end = pIn_buf_next + *pIn_buf_size;
  mz_uint8 *pOut_buf_cur = pOut_buf_next, *const pOut_buf_end = pOut_buf_next + *pOut_buf_size;
  size_t out_buf_size_mask = (decomp_flags & TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF) ? (size_t)-1 : ((pOut_buf_next - pOut_buf_start) + *pOut_buf_size) - 1, dist_from_out_buf_start;

  // Ensure the output buffer's size is a power of 2, unless the output buffer is large enough to hold the entire output file (in which case it doesn't matter).
  if (((out_buf_size_mask + 1) & out_buf_size_mask) || (pOut_buf_next < pOut_buf_start)) { *pIn_buf_size = *pOut_buf_size = 0; return TINFL_STATUS_BAD_PARAM; }

  num_bits = r->m_num_bits; bit_buf = r->m_bit_buf; dist = r->m_dist; counter = r->m_counter; num_extra = r->m_num_extra; dist_from_out_buf_start = r->m_dist_from_out_buf_start;
  TINFL_CR_BEGIN

  bit_buf = num_bits = dist = counter = num_extra = r->m_zhdr0 = r->m_zhdr1 = 0; r->m_z_adler32 = r->m_check_adler32 = 1;
  if (decomp_flags & TINFL_FLAG_PARSE_ZLIB_HEADER)
  {
	TINFL_GET_BYTE(1, r->m_zhdr0); TINFL_GET_BYTE(2, r->m_zhdr1);
	counter = (((r->m_zhdr0 * 256 + r->m_zhdr1) % 31 != 0) || (r->m_zhdr1 & 32) || ((r->m_zhdr0 & 15) != 8));
	if (!(decomp_flags & TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)) counter |= (((((uint64_t)1) << (8U + (r->m_zhdr0 >> 4))) > 32768U) || ((out_buf_size_mask + 1) < (size_t)(((uint64_t)1) << (8U + (r->m_zhdr0 >> 4)))));
	if (counter) { TINFL_CR_RETURN_FOREVER(36, TINFL_STATUS_FAILED); }
  }

  do
  {
	TINFL_GET_BITS(3, r->m_final, 3); r->m_type = r->m_final >> 1;
	if (r->m_type == 0)
	{
	  TINFL_SKIP_BITS(5, num_bits & 7);
	  for (counter = 0; counter < 4; ++counter) { if (num_bits) TINFL_GET_BITS(6, r->m_raw_header[counter], 8); else TINFL_GET_BYTE(7, r->m_raw_header[counter]); }
	  if ((counter = (r->m_raw_header[0] | (r->m_raw_header[1] << 8))) != (mz_uint)(0xFFFF ^ (r->m_raw_header[2] | (r->m_raw_header[3] << 8)))) { TINFL_CR_RETURN_FOREVER(39, TINFL_STATUS_FAILED); }
	  while ((counter) && (num_bits))
	  {
		TINFL_GET_BITS(51, dist, 8);
		while (pOut_buf_cur >= pOut_buf_end) { TINFL_CR_RETURN(52, TINFL_STATUS_HAS_MORE_OUTPUT); }
		*pOut_buf_cur++ = (mz_uint8)dist;
		counter--;
	  }
	  while (counter)
	  {
		size_t n; while (pOut_buf_cur >= pOut_buf_end) { TINFL_CR_RETURN(9, TINFL_STATUS_HAS_MORE_OUTPUT); }
		while (pIn_buf_cur >= pIn_buf_end)
		{
		  if (decomp_flags & TINFL_FLAG_HAS_MORE_INPUT)
		  {
			TINFL_CR_RETURN(38, TINFL_STATUS_NEEDS_MORE_INPUT);
		  }
		  else
		  {
			TINFL_CR_RETURN_FOREVER(40, TINFL_STATUS_FAILED);
		  }
		}
		n = MZ_MIN(MZ_MIN((size_t)(pOut_buf_end - pOut_buf_cur), (size_t)(pIn_buf_end - pIn_buf_cur)), counter);
		TINFL_MEMCPY(pOut_buf_cur, pIn_buf_cur, n); pIn_buf_cur += n; pOut_buf_cur += n; counter -= (mz_uint)n;
	  }
	}
	else if (r->m_type == 3)
	{
	  TINFL_CR_RETURN_FOREVER(10, TINFL_STATUS_FAILED);
	}
	else
	{
	  if (r->m_type == 1)
	  {
		mz_uint8 *p = r->m_tables[0].m_code_size; mz_uint i;
		r->m_table_sizes[0] = 288; r->m_table_sizes[1] = 32; TINFL_MEMSET(r->m_tables[1].m_code_size, 5, 32);
		for ( i = 0; i <= 143; ++i) *p++ = 8; for ( ; i <= 255; ++i) *p++ = 9; for ( ; i <= 279; ++i) *p++ = 7; for ( ; i <= 287; ++i) *p++ = 8;
	  }
	  else
	  {
		for (counter = 0; counter < 3; counter++) { TINFL_GET_BITS(11, r->m_table_sizes[counter], "\05\05\04"[counter]); r->m_table_sizes[counter] += s_min_table_sizes[counter]; }
		MZ_CLEAR_OBJ(r->m_tables[2].m_code_size); for (counter = 0; counter < r->m_table_sizes[2]; counter++) { mz_uint s; TINFL_GET_BITS(14, s, 3); r->m_tables[2].m_code_size[s_length_dezigzag[counter]] = (mz_uint8)s; }
		r->m_table_sizes[2] = 19;
	  }
	  for ( ; (int)r->m_type >= 0; r->m_type--)
	  {
		int tree_next, tree_cur; tinfl_huff_table *pTable;
		mz_uint i, j, used_syms, total, sym_index, next_code[17], total_syms[16]; pTable = &r->m_tables[r->m_type]; MZ_CLEAR_OBJ(total_syms); MZ_CLEAR_OBJ(pTable->m_look_up); MZ_CLEAR_OBJ(pTable->m_tree);
		for (i = 0; i < r->m_table_sizes[r->m_type]; ++i) total_syms[pTable->m_code_size[i]]++;
		used_syms = 0, total = 0; next_code[0] = next_code[1] = 0;
		for (i = 1; i <= 15; ++i) { used_syms += total_syms[i]; next_code[i + 1] = (total = ((total + total_syms[i]) << 1)); }
		if ((65536 != total) && (used_syms > 1))
		{
		  TINFL_CR_RETURN_FOREVER(35, TINFL_STATUS_FAILED);
		}
		for (tree_next = -1, sym_index = 0; sym_index < r->m_table_sizes[r->m_type]; ++sym_index)
		{
		  mz_uint rev_code = 0, l, cur_code, code_size = pTable->m_code_size[sym_index]; if (!code_size) continue;
		  cur_code = next_code[code_size]++; for (l = code_size; l > 0; l--, cur_code >>= 1) rev_code = (rev_code << 1) | (cur_code & 1);
		  if (code_size <= TINFL_FAST_LOOKUP_BITS) { mz_int16 k = (mz_int16)((code_size << 9) | sym_index); while (rev_code < TINFL_FAST_LOOKUP_SIZE) { pTable->m_look_up[rev_code] = k; rev_code += (1 << code_size); } continue; }
		  if (0 == (tree_cur = pTable->m_look_up[rev_code & (TINFL_FAST_LOOKUP_SIZE - 1)])) { pTable->m_look_up[rev_code & (TINFL_FAST_LOOKUP_SIZE - 1)] = (mz_int16)tree_next; tree_cur = tree_next; tree_next -= 2; }
		  rev_code >>= (TINFL_FAST_LOOKUP_BITS - 1);
		  for (j = code_size; j > (TINFL_FAST_LOOKUP_BITS + 1); j--)
		  {
			tree_cur -= ((rev_code >>= 1) & 1);
			if (!pTable->m_tree[-tree_cur - 1]) { pTable->m_tree[-tree_cur - 1] = (mz_int16)tree_next; tree_cur = tree_next; tree_next -= 2; } else tree_cur = pTable->m_tree[-tree_cur - 1];
		  }
		  tree_cur -= ((rev_code >>= 1) & 1); pTable->m_tree[-tree_cur - 1] = (mz_int16)sym_index;
		}
		if (r->m_type == 2)
		{
		  for (counter = 0; counter < (r->m_table_sizes[0] + r->m_table_sizes[1]); )
		  {
			mz_uint s; TINFL_HUFF_DECODE(16, dist, &r->m_tables[2]); if (dist < 16) { r->m_len_codes[counter++] = (mz_uint8)dist; continue; }
			if ((dist == 16) && (!counter))
			{
			  TINFL_CR_RETURN_FOREVER(17, TINFL_STATUS_FAILED);
			}
			num_extra = "\02\03\07"[dist - 16]; TINFL_GET_BITS(18, s, num_extra); s += "\03\03\013"[dist - 16];
			TINFL_MEMSET(r->m_len_codes + counter, (dist == 16) ? r->m_len_codes[counter - 1] : 0, s); counter += s;
		  }
		  if ((r->m_table_sizes[0] + r->m_table_sizes[1]) != counter)
		  {
			TINFL_CR_RETURN_FOREVER(21, TINFL_STATUS_FAILED);
		  }
		  TINFL_MEMCPY(r->m_tables[0].m_code_size, r->m_len_codes, r->m_table_sizes[0]); TINFL_MEMCPY(r->m_tables[1].m_code_size, r->m_len_codes + r->m_table_sizes[0], r->m_table_sizes[1]);
		}
	  }
	  for ( ; ; )
	  {
		mz_uint8 *pSrc;
		for ( ; ; )
		{
		  if (((pIn_buf_end - pIn_buf_cur) < 4) || ((pOut_buf_end - pOut_buf_cur) < 2))
		  {
			TINFL_HUFF_DECODE(23, counter, &r->m_tables[0]);
			if (counter >= 256)
			  break;
			while (pOut_buf_cur >= pOut_buf_end) { TINFL_CR_RETURN(24, TINFL_STATUS_HAS_MORE_OUTPUT); }
			*pOut_buf_cur++ = (mz_uint8)counter;
		  }
		  else
		  {
			int sym2; mz_uint code_len;
#if TINFL_USE_64BIT_BITBUF
			if (num_bits < 30) { bit_buf |= (((tinfl_bit_buf_t)MZ_READ_LE32(pIn_buf_cur)) << num_bits); pIn_buf_cur += 4; num_bits += 32; }
#else
			if (num_bits < 15) { bit_buf |= (((tinfl_bit_buf_t)MZ_READ_LE16(pIn_buf_cur)) << num_bits); pIn_buf_cur += 2; num_bits += 16; }
#endif
			if ((sym2 = r->m_tables[0].m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
			  code_len = sym2 >> 9;
			else
			{
			  code_len = TINFL_FAST_LOOKUP_BITS; do { sym2 = r->m_tables[0].m_tree[~sym2 + ((bit_buf >> code_len++) & 1)]; } while (sym2 < 0);
			}
			counter = sym2; bit_buf >>= code_len; num_bits -= code_len;
			if (counter & 256)
			  break;

#if !TINFL_USE_64BIT_BITBUF
			if (num_bits < 15) { bit_buf |= (((tinfl_bit_buf_t)MZ_READ_LE16(pIn_buf_cur)) << num_bits); pIn_buf_cur += 2; num_bits += 16; }
#endif
			if ((sym2 = r->m_tables[0].m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
			  code_len = sym2 >> 9;
			else
			{
			  code_len = TINFL_FAST_LOOKUP_BITS; do { sym2 = r->m_tables[0].m_tree[~sym2 + ((bit_buf >> code_len++) & 1)]; } while (sym2 < 0);
			}
			bit_buf >>= code_len; num_bits -= code_len;

			pOut_buf_cur[0] = (mz_uint8)counter;
			if (sym2 & 256)
			{
			  pOut_buf_cur++;
			  counter = sym2;
			  break;
			}
			pOut_buf_cur[1] = (mz_uint8)sym2;
			pOut_buf_cur += 2;
		  }
		}
		if ((counter &= 511) == 256) break;

		num_extra = s_length_extra[counter - 257]; counter = s_length_base[counter - 257];
		if (num_extra) { mz_uint extra_bits; TINFL_GET_BITS(25, extra_bits, num_extra); counter += extra_bits; }

		TINFL_HUFF_DECODE(26, dist, &r->m_tables[1]);
		num_extra = s_dist_extra[dist]; dist = s_dist_base[dist];
		if (num_extra) { mz_uint extra_bits; TINFL_GET_BITS(27, extra_bits, num_extra); dist += extra_bits; }

		dist_from_out_buf_start = pOut_buf_cur - pOut_buf_start;
		if ((dist > dist_from_out_buf_start) && (decomp_flags & TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF))
		{
		  TINFL_CR_RETURN_FOREVER(37, TINFL_STATUS_FAILED);
		}

		pSrc = pOut_buf_start + ((dist_from_out_buf_start - dist) & out_buf_size_mask);

		if ((MZ_MAX(pOut_buf_cur, pSrc) + counter) > pOut_buf_end)
		{
		  while (counter--)
		  {
			while (pOut_buf_cur >= pOut_buf_end) { TINFL_CR_RETURN(53, TINFL_STATUS_HAS_MORE_OUTPUT); }
			*pOut_buf_cur++ = pOut_buf_start[(dist_from_out_buf_start++ - dist) & out_buf_size_mask];
		  }
		  continue;
		}
#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES
		else if ((counter >= 9) && (counter <= dist))
		{
		  const mz_uint8 *pSrc_end = pSrc + (counter & ~7);
		  do
		  {
			((mz_uint32 *)pOut_buf_cur)[0] = ((const mz_uint32 *)pSrc)[0];
			((mz_uint32 *)pOut_buf_cur)[1] = ((const mz_uint32 *)pSrc)[1];
			pOut_buf_cur += 8;
		  } while ((pSrc += 8) < pSrc_end);
		  if ((counter &= 7) < 3)
		  {
			if (counter)
			{
			  pOut_buf_cur[0] = pSrc[0];
			  if (counter > 1)
				pOut_buf_cur[1] = pSrc[1];
			  pOut_buf_cur += counter;
			}
			continue;
		  }
		}
#endif
		do
		{
		  pOut_buf_cur[0] = pSrc[0];
		  pOut_buf_cur[1] = pSrc[1];
		  pOut_buf_cur[2] = pSrc[2];
		  pOut_buf_cur += 3; pSrc += 3;
		} while ((int)(counter -= 3) > 2);
		if ((int)counter > 0)
		{
		  pOut_buf_cur[0] = pSrc[0];
		  if ((int)counter > 1)
			pOut_buf_cur[1] = pSrc[1];
		  pOut_buf_cur += counter;
		}
	  }
	}
  } while (!(r->m_final & 1));
  if (decomp_flags & TINFL_FLAG_PARSE_ZLIB_HEADER)
  {
	TINFL_SKIP_BITS(32, num_bits & 7); for (counter = 0; counter < 4; ++counter) { mz_uint s; if (num_bits) TINFL_GET_BITS(41, s, 8); else TINFL_GET_BYTE(42, s); r->m_z_adler32 = (r->m_z_adler32 << 8) | s; }
  }
  TINFL_CR_RETURN_FOREVER(34, TINFL_STATUS_DONE);
  TINFL_CR_FINISH

common_exit:
  r->m_num_bits = num_bits; r->m_bit_buf = bit_buf; r->m_dist = dist; r->m_counter = counter; r->m_num_extra = num_extra; r->m_dist_from_out_buf_start = dist_from_out_buf_start;
  *pIn_buf_size = pIn_buf_cur - pIn_buf_next; *pOut_buf_size = pOut_buf_cur - pOut_buf_next;
  if ((decomp_flags & (TINFL_FLAG_PARSE_ZLIB_HEADER | TINFL_FLAG_COMPUTE_ADLER32)) && (status >= 0))
  {
	const mz_uint8 *ptr = pOut_buf_next; size_t buf_len = *pOut_buf_size;
	mz_uint32 i, s1 = r->m_check_adler32 & 0xffff, s2 = r->m_check_adler32 >> 16; size_t block_len = buf_len % 5552;
	while (buf_len)
	{
	  for (i = 0; i + 7 < block_len; i += 8, ptr += 8)
	  {
		s1 += ptr[0], s2 += s1; s1 += ptr[1], s2 += s1; s1 += ptr[2], s2 += s1; s1 += ptr[3], s2 += s1;
		s1 += ptr[4], s2 += s1; s1 += ptr[5], s2 += s1; s1 += ptr[6], s2 += s1; s1 += ptr[7], s2 += s1;
	  }
	  for ( ; i < block_len; ++i) s1 += *ptr++, s2 += s1;
	  s1 %= 65521U, s2 %= 65521U; buf_len -= block_len; block_len = 5552;
	}
	r->m_check_adler32 = (s2 << 16) + s1; if ((status == TINFL_STATUS_DONE) && (decomp_flags & TINFL_FLAG_PARSE_ZLIB_HEADER) && (r->m_check_adler32 != r->m_z_adler32)) status = TINFL_STATUS_ADLER32_MISMATCH;
  }
  return status;
}

// Higher level helper functions.
void *tinfl_decompress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags)
{
  tinfl_decompressor decomp; void *pBuf = NULL, *pNew_buf; size_t src_buf_ofs = 0, out_buf_capacity = 0;
  *pOut_len = 0;
  tinfl_init(&decomp);
  for ( ; ; )
  {
	size_t src_buf_size = src_buf_len - src_buf_ofs, dst_buf_size = out_buf_capacity - *pOut_len, new_out_buf_capacity;
	tinfl_status status = tinfl_decompress(&decomp, (const mz_uint8*)pSrc_buf + src_buf_ofs, &src_buf_size, (mz_uint8*)pBuf, pBuf ? (mz_uint8*)pBuf + *pOut_len : NULL, &dst_buf_size,
	  (flags & ~TINFL_FLAG_HAS_MORE_INPUT) | TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
	if ((status < 0) || (status == TINFL_STATUS_NEEDS_MORE_INPUT))
	{
	  MZ_FREE(pBuf); *pOut_len = 0; return NULL;
	}
	src_buf_ofs += src_buf_size;
	*pOut_len += dst_buf_size;
	if (status == TINFL_STATUS_DONE) break;
	new_out_buf_capacity = out_buf_capacity * 2; if (new_out_buf_capacity < 128) new_out_buf_capacity = 128;
	pNew_buf = MZ_REALLOC(pBuf, new_out_buf_capacity);
	if (!pNew_buf)
	{
	  MZ_FREE(pBuf); *pOut_len = 0; return NULL;
	}
	pBuf = pNew_buf; out_buf_capacity = new_out_buf_capacity;
  }
  return pBuf;
}

size_t tinfl_decompress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags)
{
  tinfl_decompressor decomp; tinfl_status status; tinfl_init(&decomp);
  status = tinfl_decompress(&decomp, (const mz_uint8*)pSrc_buf, &src_buf_len, (mz_uint8*)pOut_buf, (mz_uint8*)pOut_buf, &out_buf_len, (flags & ~TINFL_FLAG_HAS_MORE_INPUT) | TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
  return (status != TINFL_STATUS_DONE) ? TINFL_DECOMPRESS_MEM_TO_MEM_FAILED : out_buf_len;
}

int tinfl_decompress_mem_to_callback(const void *pIn_buf, size_t *pIn_buf_size, tinfl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags)
{
  int result = 0;
  tinfl_decompressor decomp;
  mz_uint8 *pDict = (mz_uint8*)MZ_MALLOC(TINFL_LZ_DICT_SIZE); size_t in_buf_ofs = 0, dict_ofs = 0;
  if (!pDict)
	return TINFL_STATUS_FAILED;
  tinfl_init(&decomp);
  for ( ; ; )
  {
	size_t in_buf_size = *pIn_buf_size - in_buf_ofs, dst_buf_size = TINFL_LZ_DICT_SIZE - dict_ofs;
	tinfl_status status = tinfl_decompress(&decomp, (const mz_uint8*)pIn_buf + in_buf_ofs, &in_buf_size, pDict, pDict + dict_ofs, &dst_buf_size,
	  (flags & ~(TINFL_FLAG_HAS_MORE_INPUT | TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)));
	in_buf_ofs += in_buf_size;
	if ((dst_buf_size) && (!(*pPut_buf_func)(pDict + dict_ofs, (int)dst_buf_size, pPut_buf_user)))
	  break;
	if (status != TINFL_STATUS_HAS_MORE_OUTPUT)
	{
	  result = (status == TINFL_STATUS_DONE);
	  break;
	}
	dict_ofs = (dict_ofs + dst_buf_size) & (TINFL_LZ_DICT_SIZE - 1);
  }
  MZ_FREE(pDict);
  *pIn_buf_size = in_buf_ofs;
  return result;
}

// ------------------- Low-level Compression (independent from all decompression API's)

// Purposely making these tables static for faster init and thread safety.
static const mz_uint16 s_tdefl_len_sym[256] = {
  257,258,259,260,261,262,263,264,265,265,266,266,267,267,268,268,269,269,269,269,270,270,270,270,271,271,271,271,272,272,272,272,
  273,273,273,273,273,273,273,273,274,274,274,274,274,274,274,274,275,275,275,275,275,275,275,275,276,276,276,276,276,276,276,276,
  277,277,277,277,277,277,277,277,277,277,277,277,277,277,277,277,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,
  279,279,279,279,279,279,279,279,279,279,279,279,279,279,279,279,280,280,280,280,280,280,280,280,280,280,280,280,280,280,280,280,
  281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,
  282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,
  283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,
  284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,285 };

static const mz_uint8 s_tdefl_len_extra[256] = {
  0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0 };

static const mz_uint8 s_tdefl_small_dist_sym[512] = {
  0,1,2,3,4,4,5,5,6,6,6,6,7,7,7,7,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,
  11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,
  13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,14,14,14,14,
  14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,
  14,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
  15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,16,16,16,16,16,16,16,16,16,16,16,16,16,
  16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
  16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
  16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
  17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
  17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
  17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17 };

static const mz_uint8 s_tdefl_small_dist_extra[512] = {
  0,0,0,0,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7 };

static const mz_uint8 s_tdefl_large_dist_sym[128] = {
  0,0,18,19,20,20,21,21,22,22,22,22,23,23,23,23,24,24,24,24,24,24,24,24,25,25,25,25,25,25,25,25,26,26,26,26,26,26,26,26,26,26,26,26,
  26,26,26,26,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,
  28,28,28,28,28,28,28,28,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29 };

static const mz_uint8 s_tdefl_large_dist_extra[128] = {
  0,0,8,8,9,9,9,9,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
  12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
  13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13 };

// Radix sorts tdefl_sym_freq[] array by 16-bit key m_key. Returns ptr to sorted values.
typedef struct { mz_uint16 m_key, m_sym_index; } tdefl_sym_freq;
static tdefl_sym_freq* tdefl_radix_sort_syms(mz_uint num_syms, tdefl_sym_freq* pSyms0, tdefl_sym_freq* pSyms1)
{
  mz_uint32 total_passes = 2, pass_shift, pass, i, hist[256 * 2]; tdefl_sym_freq* pCur_syms = pSyms0, *pNew_syms = pSyms1; MZ_CLEAR_OBJ(hist);
  for (i = 0; i < num_syms; i++) { mz_uint freq = pSyms0[i].m_key; hist[freq & 0xFF]++; hist[256 + ((freq >> 8) & 0xFF)]++; }
  while ((total_passes > 1) && (num_syms == hist[(total_passes - 1) * 256])) total_passes--;
  for (pass_shift = 0, pass = 0; pass < total_passes; pass++, pass_shift += 8)
  {
	const mz_uint32* pHist = &hist[pass << 8];
	mz_uint offsets[256], cur_ofs = 0;
	for (i = 0; i < 256; i++) { offsets[i] = cur_ofs; cur_ofs += pHist[i]; }
	for (i = 0; i < num_syms; i++) pNew_syms[offsets[(pCur_syms[i].m_key >> pass_shift) & 0xFF]++] = pCur_syms[i];
	{ tdefl_sym_freq* t = pCur_syms; pCur_syms = pNew_syms; pNew_syms = t; }
  }
  return pCur_syms;
}

// tdefl_calculate_minimum_redundancy() originally written by: Alistair Moffat, alistair@cs.mu.oz.au, Jyrki Katajainen, jyrki@diku.dk, November 1996.
static void tdefl_calculate_minimum_redundancy(tdefl_sym_freq *A, int n)
{
  int root, leaf, next, avbl, used, dpth;
  if (n==0) return; else if (n==1) { A[0].m_key = 1; return; }
  A[0].m_key += A[1].m_key; root = 0; leaf = 2;
  for (next=1; next < n-1; next++)
  {
	if (leaf>=n || A[root].m_key<A[leaf].m_key) { A[next].m_key = A[root].m_key; A[root++].m_key = (mz_uint16)next; } else A[next].m_key = A[leaf++].m_key;
	if (leaf>=n || (root<next && A[root].m_key<A[leaf].m_key)) { A[next].m_key = (mz_uint16)(A[next].m_key + A[root].m_key); A[root++].m_key = (mz_uint16)next; } else A[next].m_key = (mz_uint16)(A[next].m_key + A[leaf++].m_key);
  }
  A[n-2].m_key = 0; for (next=n-3; next>=0; next--) A[next].m_key = A[A[next].m_key].m_key+1;
  avbl = 1; used = dpth = 0; root = n-2; next = n-1;
  while (avbl>0)
  {
	while (root>=0 && (int)A[root].m_key==dpth) { used++; root--; }
	while (avbl>used) { A[next--].m_key = (mz_uint16)(dpth); avbl--; }
	avbl = 2*used; dpth++; used = 0;
  }
}

// Limits canonical Huffman code table's max code size.
enum { TDEFL_MAX_SUPPORTED_HUFF_CODESIZE = 32 };
static void tdefl_huffman_enforce_max_code_size(int *pNum_codes, int code_list_len, int max_code_size)
{
  int i; mz_uint32 total = 0; if (code_list_len <= 1) return;
  for (i = max_code_size + 1; i <= TDEFL_MAX_SUPPORTED_HUFF_CODESIZE; i++) pNum_codes[max_code_size] += pNum_codes[i];
  for (i = max_code_size; i > 0; i--) total += (((mz_uint32)pNum_codes[i]) << (max_code_size - i));
  while (total != (1UL << max_code_size))
  {
	pNum_codes[max_code_size]--;
	for (i = max_code_size - 1; i > 0; i--) if (pNum_codes[i]) { pNum_codes[i]--; pNum_codes[i + 1] += 2; break; }
	total--;
  }
}

static void tdefl_optimize_huffman_table(tdefl_compressor *d, int table_num, int table_len, int code_size_limit, int static_table)
{
  int i, j, l, num_codes[1 + TDEFL_MAX_SUPPORTED_HUFF_CODESIZE]; mz_uint next_code[TDEFL_MAX_SUPPORTED_HUFF_CODESIZE + 1]; MZ_CLEAR_OBJ(num_codes);
  if (static_table)
  {
	for (i = 0; i < table_len; i++) num_codes[d->m_huff_code_sizes[table_num][i]]++;
  }
  else
  {
	tdefl_sym_freq syms0[TDEFL_MAX_HUFF_SYMBOLS], syms1[TDEFL_MAX_HUFF_SYMBOLS], *pSyms;
	int num_used_syms = 0;
	const mz_uint16 *pSym_count = &d->m_huff_count[table_num][0];
	for (i = 0; i < table_len; i++) if (pSym_count[i]) { syms0[num_used_syms].m_key = (mz_uint16)pSym_count[i]; syms0[num_used_syms++].m_sym_index = (mz_uint16)i; }

	pSyms = tdefl_radix_sort_syms(num_used_syms, syms0, syms1); tdefl_calculate_minimum_redundancy(pSyms, num_used_syms);

	for (i = 0; i < num_used_syms; i++) num_codes[pSyms[i].m_key]++;

	tdefl_huffman_enforce_max_code_size(num_codes, num_used_syms, code_size_limit);

	MZ_CLEAR_OBJ(d->m_huff_code_sizes[table_num]); MZ_CLEAR_OBJ(d->m_huff_codes[table_num]);
	for (i = 1, j = num_used_syms; i <= code_size_limit; i++)
	  for (l = num_codes[i]; l > 0; l--) d->m_huff_code_sizes[table_num][pSyms[--j].m_sym_index] = (mz_uint8)(i);
  }

  next_code[1] = 0; for (j = 0, i = 2; i <= code_size_limit; i++) next_code[i] = j = ((j + num_codes[i - 1]) << 1);

  for (i = 0; i < table_len; i++)
  {
	mz_uint rev_code = 0, code, code_size; if ((code_size = d->m_huff_code_sizes[table_num][i]) == 0) continue;
	code = next_code[code_size]++; for (l = code_size; l > 0; l--, code >>= 1) rev_code = (rev_code << 1) | (code & 1);
	d->m_huff_codes[table_num][i] = (mz_uint16)rev_code;
  }
}

#define TDEFL_PUT_BITS(b, l) do { \
  mz_uint bits = b; mz_uint len = l; MZ_ASSERT(bits <= ((1U << len) - 1U)); \
  d->m_bit_buffer |= (bits << d->m_bits_in); d->m_bits_in += len; \
  while (d->m_bits_in >= 8) { \
	if (d->m_pOutput_buf < d->m_pOutput_buf_end) \
	  *d->m_pOutput_buf++ = (mz_uint8)(d->m_bit_buffer); \
	  d->m_bit_buffer >>= 8; \
	  d->m_bits_in -= 8; \
  } \
} MZ_MACRO_END

#define TDEFL_RLE_PREV_CODE_SIZE() { if (rle_repeat_count) { \
  if (rle_repeat_count < 3) { \
	d->m_huff_count[2][prev_code_size] = (mz_uint16)(d->m_huff_count[2][prev_code_size] + rle_repeat_count); \
	while (rle_repeat_count--) packed_code_sizes[num_packed_code_sizes++] = prev_code_size; \
  } else { \
	d->m_huff_count[2][16] = (mz_uint16)(d->m_huff_count[2][16] + 1); packed_code_sizes[num_packed_code_sizes++] = 16; packed_code_sizes[num_packed_code_sizes++] = (mz_uint8)(rle_repeat_count - 3); \
} rle_repeat_count = 0; } }

#define TDEFL_RLE_ZERO_CODE_SIZE() { if (rle_z_count) { \
  if (rle_z_count < 3) { \
	d->m_huff_count[2][0] = (mz_uint16)(d->m_huff_count[2][0] + rle_z_count); while (rle_z_count--) packed_code_sizes[num_packed_code_sizes++] = 0; \
  } else if (rle_z_count <= 10) { \
	d->m_huff_count[2][17] = (mz_uint16)(d->m_huff_count[2][17] + 1); packed_code_sizes[num_packed_code_sizes++] = 17; packed_code_sizes[num_packed_code_sizes++] = (mz_uint8)(rle_z_count - 3); \
  } else { \
	d->m_huff_count[2][18] = (mz_uint16)(d->m_huff_count[2][18] + 1); packed_code_sizes[num_packed_code_sizes++] = 18; packed_code_sizes[num_packed_code_sizes++] = (mz_uint8)(rle_z_count - 11); \
} rle_z_count = 0; } }

static mz_uint8 s_tdefl_packed_code_size_syms_swizzle[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

static void tdefl_start_dynamic_block(tdefl_compressor *d)
{
  int num_lit_codes, num_dist_codes, num_bit_lengths; mz_uint i, total_code_sizes_to_pack, num_packed_code_sizes, rle_z_count, rle_repeat_count, packed_code_sizes_index;
  mz_uint8 code_sizes_to_pack[TDEFL_MAX_HUFF_SYMBOLS_0 + TDEFL_MAX_HUFF_SYMBOLS_1], packed_code_sizes[TDEFL_MAX_HUFF_SYMBOLS_0 + TDEFL_MAX_HUFF_SYMBOLS_1], prev_code_size = 0xFF;

  d->m_huff_count[0][256] = 1;

  tdefl_optimize_huffman_table(d, 0, TDEFL_MAX_HUFF_SYMBOLS_0, 15, MZ_FALSE);
  tdefl_optimize_huffman_table(d, 1, TDEFL_MAX_HUFF_SYMBOLS_1, 15, MZ_FALSE);

  for (num_lit_codes = 286; num_lit_codes > 257; num_lit_codes--) if (d->m_huff_code_sizes[0][num_lit_codes - 1]) break;
  for (num_dist_codes = 30; num_dist_codes > 1; num_dist_codes--) if (d->m_huff_code_sizes[1][num_dist_codes - 1]) break;

  memcpy(code_sizes_to_pack, &d->m_huff_code_sizes[0][0], num_lit_codes);
  memcpy(code_sizes_to_pack + num_lit_codes, &d->m_huff_code_sizes[1][0], num_dist_codes);
  total_code_sizes_to_pack = num_lit_codes + num_dist_codes; num_packed_code_sizes = 0; rle_z_count = 0; rle_repeat_count = 0;

  memset(&d->m_huff_count[2][0], 0, sizeof(d->m_huff_count[2][0]) * TDEFL_MAX_HUFF_SYMBOLS_2);
  for (i = 0; i < total_code_sizes_to_pack; i++)
  {
	mz_uint8 code_size = code_sizes_to_pack[i];
	if (!code_size)
	{
	  TDEFL_RLE_PREV_CODE_SIZE();
	  if (++rle_z_count == 138) { TDEFL_RLE_ZERO_CODE_SIZE(); }
	}
	else
	{
	  TDEFL_RLE_ZERO_CODE_SIZE();
	  if (code_size != prev_code_size)
	  {
		TDEFL_RLE_PREV_CODE_SIZE();
		d->m_huff_count[2][code_size] = (mz_uint16)(d->m_huff_count[2][code_size] + 1); packed_code_sizes[num_packed_code_sizes++] = code_size;
	  }
	  else if (++rle_repeat_count == 6)
	  {
		TDEFL_RLE_PREV_CODE_SIZE();
	  }
	}
	prev_code_size = code_size;
  }
  if (rle_repeat_count) { TDEFL_RLE_PREV_CODE_SIZE(); } else { TDEFL_RLE_ZERO_CODE_SIZE(); }

  tdefl_optimize_huffman_table(d, 2, TDEFL_MAX_HUFF_SYMBOLS_2, 7, MZ_FALSE);

  TDEFL_PUT_BITS(2, 2);

  TDEFL_PUT_BITS(num_lit_codes - 257, 5);
  TDEFL_PUT_BITS(num_dist_codes - 1, 5);

  for (num_bit_lengths = 18; num_bit_lengths >= 0; num_bit_lengths--) if (d->m_huff_code_sizes[2][s_tdefl_packed_code_size_syms_swizzle[num_bit_lengths]]) break;
  num_bit_lengths = MZ_MAX(4, (num_bit_lengths + 1)); TDEFL_PUT_BITS(num_bit_lengths - 4, 4);
  for (i = 0; (int)i < num_bit_lengths; i++) TDEFL_PUT_BITS(d->m_huff_code_sizes[2][s_tdefl_packed_code_size_syms_swizzle[i]], 3);

  for (packed_code_sizes_index = 0; packed_code_sizes_index < num_packed_code_sizes; )
  {
	mz_uint code = packed_code_sizes[packed_code_sizes_index++]; MZ_ASSERT(code < TDEFL_MAX_HUFF_SYMBOLS_2);
	TDEFL_PUT_BITS(d->m_huff_codes[2][code], d->m_huff_code_sizes[2][code]);
	if (code >= 16) TDEFL_PUT_BITS(packed_code_sizes[packed_code_sizes_index++], "\02\03\07"[code - 16]);
  }
}

static void tdefl_start_static_block(tdefl_compressor *d)
{
  mz_uint i;
  mz_uint8 *p = &d->m_huff_code_sizes[0][0];

  for (i = 0; i <= 143; ++i) *p++ = 8;
  for ( ; i <= 255; ++i) *p++ = 9;
  for ( ; i <= 279; ++i) *p++ = 7;
  for ( ; i <= 287; ++i) *p++ = 8;

  memset(d->m_huff_code_sizes[1], 5, 32);

  tdefl_optimize_huffman_table(d, 0, 288, 15, MZ_TRUE);
  tdefl_optimize_huffman_table(d, 1, 32, 15, MZ_TRUE);

  TDEFL_PUT_BITS(1, 2);
}

static const mz_uint mz_bitmasks[17] = { 0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN && MINIZ_HAS_64BIT_REGISTERS
static mz_bool tdefl_compress_lz_codes(tdefl_compressor *d)
{
  mz_uint flags;
  mz_uint8 *pLZ_codes;
  mz_uint8 *pOutput_buf = d->m_pOutput_buf;
  mz_uint8 *pLZ_code_buf_end = d->m_pLZ_code_buf;
  mz_uint64 bit_buffer = d->m_bit_buffer;
  mz_uint bits_in = d->m_bits_in;

#define TDEFL_PUT_BITS_FAST(b, l) { bit_buffer |= (((mz_uint64)(b)) << bits_in); bits_in += (l); }

  flags = 1;
  for (pLZ_codes = d->m_lz_code_buf; pLZ_codes < pLZ_code_buf_end; flags >>= 1)
  {
	if (flags == 1)
	  flags = *pLZ_codes++ | 0x100;

	if (flags & 1)
	{
	  mz_uint s0, s1, n0, n1, sym, num_extra_bits;
	  mz_uint match_len = pLZ_codes[0], match_dist = *(const mz_uint16 *)(pLZ_codes + 1); pLZ_codes += 3;

	  MZ_ASSERT(d->m_huff_code_sizes[0][s_tdefl_len_sym[match_len]]);
	  TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][s_tdefl_len_sym[match_len]], d->m_huff_code_sizes[0][s_tdefl_len_sym[match_len]]);
	  TDEFL_PUT_BITS_FAST(match_len & mz_bitmasks[s_tdefl_len_extra[match_len]], s_tdefl_len_extra[match_len]);

	  // This sequence coaxes MSVC into using cmov's vs. jmp's.
	  s0 = s_tdefl_small_dist_sym[match_dist & 511];
	  n0 = s_tdefl_small_dist_extra[match_dist & 511];
	  s1 = s_tdefl_large_dist_sym[match_dist >> 8];
	  n1 = s_tdefl_large_dist_extra[match_dist >> 8];
	  sym = (match_dist < 512) ? s0 : s1;
	  num_extra_bits = (match_dist < 512) ? n0 : n1;

	  MZ_ASSERT(d->m_huff_code_sizes[1][sym]);
	  TDEFL_PUT_BITS_FAST(d->m_huff_codes[1][sym], d->m_huff_code_sizes[1][sym]);
	  TDEFL_PUT_BITS_FAST(match_dist & mz_bitmasks[num_extra_bits], num_extra_bits);
	}
	else
	{
	  mz_uint lit = *pLZ_codes++;
	  MZ_ASSERT(d->m_huff_code_sizes[0][lit]);
	  TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][lit], d->m_huff_code_sizes[0][lit]);

	  if (((flags & 2) == 0) && (pLZ_codes < pLZ_code_buf_end))
	  {
		flags >>= 1;
		lit = *pLZ_codes++;
		MZ_ASSERT(d->m_huff_code_sizes[0][lit]);
		TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][lit], d->m_huff_code_sizes[0][lit]);

		if (((flags & 2) == 0) && (pLZ_codes < pLZ_code_buf_end))
		{
		  flags >>= 1;
		  lit = *pLZ_codes++;
		  MZ_ASSERT(d->m_huff_code_sizes[0][lit]);
		  TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][lit], d->m_huff_code_sizes[0][lit]);
		}
	  }
	}

	if (pOutput_buf >= d->m_pOutput_buf_end)
	  return MZ_FALSE;

	*(mz_uint64*)pOutput_buf = bit_buffer;
	pOutput_buf += (bits_in >> 3);
	bit_buffer >>= (bits_in & ~7);
	bits_in &= 7;
  }

#undef TDEFL_PUT_BITS_FAST

  d->m_pOutput_buf = pOutput_buf;
  d->m_bits_in = 0;
  d->m_bit_buffer = 0;

  while (bits_in)
  {
	mz_uint32 n = MZ_MIN(bits_in, 16);
	TDEFL_PUT_BITS((mz_uint)bit_buffer & mz_bitmasks[n], n);
	bit_buffer >>= n;
	bits_in -= n;
  }

  TDEFL_PUT_BITS(d->m_huff_codes[0][256], d->m_huff_code_sizes[0][256]);

  return (d->m_pOutput_buf < d->m_pOutput_buf_end);
}
#else
static mz_bool tdefl_compress_lz_codes(tdefl_compressor *d)
{
  mz_uint flags;
  mz_uint8 *pLZ_codes;

  flags = 1;
  for (pLZ_codes = d->m_lz_code_buf; pLZ_codes < d->m_pLZ_code_buf; flags >>= 1)
  {
	if (flags == 1)
	  flags = *pLZ_codes++ | 0x100;
	if (flags & 1)
	{
	  mz_uint sym, num_extra_bits;
	  mz_uint match_len = pLZ_codes[0], match_dist = (pLZ_codes[1] | (pLZ_codes[2] << 8)); pLZ_codes += 3;

	  MZ_ASSERT(d->m_huff_code_sizes[0][s_tdefl_len_sym[match_len]]);
	  TDEFL_PUT_BITS(d->m_huff_codes[0][s_tdefl_len_sym[match_len]], d->m_huff_code_sizes[0][s_tdefl_len_sym[match_len]]);
	  TDEFL_PUT_BITS(match_len & mz_bitmasks[s_tdefl_len_extra[match_len]], s_tdefl_len_extra[match_len]);

	  if (match_dist < 512)
	  {
		sym = s_tdefl_small_dist_sym[match_dist]; num_extra_bits = s_tdefl_small_dist_extra[match_dist];
	  }
	  else
	  {
		sym = s_tdefl_large_dist_sym[match_dist >> 8]; num_extra_bits = s_tdefl_large_dist_extra[match_dist >> 8];
	  }
	  MZ_ASSERT(d->m_huff_code_sizes[1][sym]);
	  TDEFL_PUT_BITS(d->m_huff_codes[1][sym], d->m_huff_code_sizes[1][sym]);
	  TDEFL_PUT_BITS(match_dist & mz_bitmasks[num_extra_bits], num_extra_bits);
	}
	else
	{
	  mz_uint lit = *pLZ_codes++;
	  MZ_ASSERT(d->m_huff_code_sizes[0][lit]);
	  TDEFL_PUT_BITS(d->m_huff_codes[0][lit], d->m_huff_code_sizes[0][lit]);
	}
  }

  TDEFL_PUT_BITS(d->m_huff_codes[0][256], d->m_huff_code_sizes[0][256]);

  return (d->m_pOutput_buf < d->m_pOutput_buf_end);
}
#endif // MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN && MINIZ_HAS_64BIT_REGISTERS

static mz_bool tdefl_compress_block(tdefl_compressor *d, mz_bool static_block)
{
  if (static_block)
	tdefl_start_static_block(d);
  else
	tdefl_start_dynamic_block(d);
  return tdefl_compress_lz_codes(d);
}

static int tdefl_flush_block(tdefl_compressor *d, int flush)
{
  mz_uint saved_bit_buf, saved_bits_in;
  mz_uint8 *pSaved_output_buf;
  mz_bool comp_block_succeeded = MZ_FALSE;
  int n, use_raw_block = ((d->m_flags & TDEFL_FORCE_ALL_RAW_BLOCKS) != 0) && (d->m_lookahead_pos - d->m_lz_code_buf_dict_pos) <= d->m_dict_size;
  mz_uint8 *pOutput_buf_start = ((d->m_pPut_buf_func == NULL) && ((*d->m_pOut_buf_size - d->m_out_buf_ofs) >= TDEFL_OUT_BUF_SIZE)) ? ((mz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs) : d->m_output_buf;

  d->m_pOutput_buf = pOutput_buf_start;
  d->m_pOutput_buf_end = d->m_pOutput_buf + TDEFL_OUT_BUF_SIZE - 16;

  MZ_ASSERT(!d->m_output_flush_remaining);
  d->m_output_flush_ofs = 0;
  d->m_output_flush_remaining = 0;

  *d->m_pLZ_flags = (mz_uint8)(*d->m_pLZ_flags >> d->m_num_flags_left);
  d->m_pLZ_code_buf -= (d->m_num_flags_left == 8);

  if ((d->m_flags & TDEFL_WRITE_ZLIB_HEADER) && (!d->m_block_index))
  {
	TDEFL_PUT_BITS(0x78, 8); TDEFL_PUT_BITS(0x01, 8);
  }

  TDEFL_PUT_BITS(flush == TDEFL_FINISH, 1);

  pSaved_output_buf = d->m_pOutput_buf; saved_bit_buf = d->m_bit_buffer; saved_bits_in = d->m_bits_in;

  if (!use_raw_block)
	comp_block_succeeded = tdefl_compress_block(d, (d->m_flags & TDEFL_FORCE_ALL_STATIC_BLOCKS) || (d->m_total_lz_bytes < 48));

  // If the block gets expanded, forget the current contents of the output buffer and send a raw block instead.
  if ( ((use_raw_block) || ((d->m_total_lz_bytes) && ((d->m_pOutput_buf - pSaved_output_buf + 1U) >= d->m_total_lz_bytes))) &&
	   ((d->m_lookahead_pos - d->m_lz_code_buf_dict_pos) <= d->m_dict_size) )
  {
	mz_uint i; d->m_pOutput_buf = pSaved_output_buf; d->m_bit_buffer = saved_bit_buf, d->m_bits_in = saved_bits_in;
	TDEFL_PUT_BITS(0, 2);
	if (d->m_bits_in) { TDEFL_PUT_BITS(0, 8 - d->m_bits_in); }
	for (i = 2; i; --i, d->m_total_lz_bytes ^= 0xFFFF)
	{
	  TDEFL_PUT_BITS(d->m_total_lz_bytes & 0xFFFF, 16);
	}
	for (i = 0; i < d->m_total_lz_bytes; ++i)
	{
	  TDEFL_PUT_BITS(d->m_dict[(d->m_lz_code_buf_dict_pos + i) & TDEFL_LZ_DICT_SIZE_MASK], 8);
	}
  }
  // Check for the extremely unlikely (if not impossible) case of the compressed block not fitting into the output buffer when using dynamic codes.
  else if (!comp_block_succeeded)
  {
	d->m_pOutput_buf = pSaved_output_buf; d->m_bit_buffer = saved_bit_buf, d->m_bits_in = saved_bits_in;
	tdefl_compress_block(d, MZ_TRUE);
  }

  if (flush)
  {
	if (flush == TDEFL_FINISH)
	{
	  if (d->m_bits_in) { TDEFL_PUT_BITS(0, 8 - d->m_bits_in); }
	  if (d->m_flags & TDEFL_WRITE_ZLIB_HEADER) { mz_uint i, a = d->m_adler32; for (i = 0; i < 4; i++) { TDEFL_PUT_BITS((a >> 24) & 0xFF, 8); a <<= 8; } }
	}
	else
	{
	  mz_uint i, z = 0; TDEFL_PUT_BITS(0, 3); if (d->m_bits_in) { TDEFL_PUT_BITS(0, 8 - d->m_bits_in); } for (i = 2; i; --i, z ^= 0xFFFF) { TDEFL_PUT_BITS(z & 0xFFFF, 16); }
	}
  }

  MZ_ASSERT(d->m_pOutput_buf < d->m_pOutput_buf_end);

  memset(&d->m_huff_count[0][0], 0, sizeof(d->m_huff_count[0][0]) * TDEFL_MAX_HUFF_SYMBOLS_0);
  memset(&d->m_huff_count[1][0], 0, sizeof(d->m_huff_count[1][0]) * TDEFL_MAX_HUFF_SYMBOLS_1);

  d->m_pLZ_code_buf = d->m_lz_code_buf + 1; d->m_pLZ_flags = d->m_lz_code_buf; d->m_num_flags_left = 8; d->m_lz_code_buf_dict_pos += d->m_total_lz_bytes; d->m_total_lz_bytes = 0; d->m_block_index++;

  if ((n = (int)(d->m_pOutput_buf - pOutput_buf_start)) != 0)
  {
	if (d->m_pPut_buf_func)
	{
	  *d->m_pIn_buf_size = d->m_pSrc - (const mz_uint8 *)d->m_pIn_buf;
	  if (!(*d->m_pPut_buf_func)(d->m_output_buf, n, d->m_pPut_buf_user))
		return (d->m_prev_return_status = TDEFL_STATUS_PUT_BUF_FAILED);
	}
	else if (pOutput_buf_start == d->m_output_buf)
	{
	  int bytes_to_copy = (int)MZ_MIN((size_t)n, (size_t)(*d->m_pOut_buf_size - d->m_out_buf_ofs));
	  memcpy((mz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs, d->m_output_buf, bytes_to_copy);
	  d->m_out_buf_ofs += bytes_to_copy;
	  if ((n -= bytes_to_copy) != 0)
	  {
		d->m_output_flush_ofs = bytes_to_copy;
		d->m_output_flush_remaining = n;
	  }
	}
	else
	{
	  d->m_out_buf_ofs += n;
	}
  }

  return d->m_output_flush_remaining;
}

#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES
#define TDEFL_READ_UNALIGNED_WORD(p) *(const mz_uint16*)(p)
static MZ_FORCEINLINE void tdefl_find_match(tdefl_compressor *d, mz_uint lookahead_pos, mz_uint max_dist, mz_uint max_match_len, mz_uint *pMatch_dist, mz_uint *pMatch_len)
{
  mz_uint dist, pos = lookahead_pos & TDEFL_LZ_DICT_SIZE_MASK, match_len = *pMatch_len, probe_pos = pos, next_probe_pos, probe_len;
  mz_uint num_probes_left = d->m_max_probes[match_len >= 32];
  const mz_uint16 *s = (const mz_uint16*)(d->m_dict + pos), *p, *q;
  mz_uint16 c01 = TDEFL_READ_UNALIGNED_WORD(&d->m_dict[pos + match_len - 1]), s01 = TDEFL_READ_UNALIGNED_WORD(s);
  MZ_ASSERT(max_match_len <= TDEFL_MAX_MATCH_LEN); if (max_match_len <= match_len) return;
  for ( ; ; )
  {
	for ( ; ; )
	{
	  if (--num_probes_left == 0) return;
	  #define TDEFL_PROBE \
		next_probe_pos = d->m_next[probe_pos]; \
		if ((!next_probe_pos) || ((dist = (mz_uint16)(lookahead_pos - next_probe_pos)) > max_dist)) return; \
		probe_pos = next_probe_pos & TDEFL_LZ_DICT_SIZE_MASK; \
		if (TDEFL_READ_UNALIGNED_WORD(&d->m_dict[probe_pos + match_len - 1]) == c01) break;
	  TDEFL_PROBE; TDEFL_PROBE; TDEFL_PROBE;
	}
	if (!dist) break; q = (const mz_uint16*)(d->m_dict + probe_pos); if (TDEFL_READ_UNALIGNED_WORD(q) != s01) continue; p = s; probe_len = 32;
	do { } while ( (TDEFL_READ_UNALIGNED_WORD(++p) == TDEFL_READ_UNALIGNED_WORD(++q)) && (TDEFL_READ_UNALIGNED_WORD(++p) == TDEFL_READ_UNALIGNED_WORD(++q)) &&
				   (TDEFL_READ_UNALIGNED_WORD(++p) == TDEFL_READ_UNALIGNED_WORD(++q)) && (TDEFL_READ_UNALIGNED_WORD(++p) == TDEFL_READ_UNALIGNED_WORD(++q)) && (--probe_len > 0) );
	if (!probe_len)
	{
	  *pMatch_dist = dist; *pMatch_len = MZ_MIN(max_match_len, TDEFL_MAX_MATCH_LEN); break;
	}
	else if ((probe_len = ((mz_uint)(p - s) * 2) + (mz_uint)(*(const mz_uint8*)p == *(const mz_uint8*)q)) > match_len)
	{
	  *pMatch_dist = dist; if ((*pMatch_len = match_len = MZ_MIN(max_match_len, probe_len)) == max_match_len) break;
	  c01 = TDEFL_READ_UNALIGNED_WORD(&d->m_dict[pos + match_len - 1]);
	}
  }
}
#else
static MZ_FORCEINLINE void tdefl_find_match(tdefl_compressor *d, mz_uint lookahead_pos, mz_uint max_dist, mz_uint max_match_len, mz_uint *pMatch_dist, mz_uint *pMatch_len)
{
  mz_uint dist, pos = lookahead_pos & TDEFL_LZ_DICT_SIZE_MASK, match_len = *pMatch_len, probe_pos = pos, next_probe_pos, probe_len;
  mz_uint num_probes_left = d->m_max_probes[match_len >= 32];
  const mz_uint8 *s = d->m_dict + pos, *p, *q;
  mz_uint8 c0 = d->m_dict[pos + match_len], c1 = d->m_dict[pos + match_len - 1];
  MZ_ASSERT(max_match_len <= TDEFL_MAX_MATCH_LEN); if (max_match_len <= match_len) return;
  for ( ; ; )
  {
	for ( ; ; )
	{
	  if (--num_probes_left == 0) return;
	  #define TDEFL_PROBE \
		next_probe_pos = d->m_next[probe_pos]; \
		if ((!next_probe_pos) || ((dist = (mz_uint16)(lookahead_pos - next_probe_pos)) > max_dist)) return; \
		probe_pos = next_probe_pos & TDEFL_LZ_DICT_SIZE_MASK; \
		if ((d->m_dict[probe_pos + match_len] == c0) && (d->m_dict[probe_pos + match_len - 1] == c1)) break;
	  TDEFL_PROBE; TDEFL_PROBE; TDEFL_PROBE;
	}
	if (!dist) break; p = s; q = d->m_dict + probe_pos; for (probe_len = 0; probe_len < max_match_len; probe_len++) if (*p++ != *q++) break;
	if (probe_len > match_len)
	{
	  *pMatch_dist = dist; if ((*pMatch_len = match_len = probe_len) == max_match_len) return;
	  c0 = d->m_dict[pos + match_len]; c1 = d->m_dict[pos + match_len - 1];
	}
  }
}
#endif // #if MINIZ_USE_UNALIGNED_LOADS_AND_STORES

#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN
static mz_bool tdefl_compress_fast(tdefl_compressor *d)
{
  // Faster, minimally featured LZRW1-style match+parse loop with better register utilization. Intended for applications where raw throughput is valued more highly than ratio.
  mz_uint lookahead_pos = d->m_lookahead_pos, lookahead_size = d->m_lookahead_size, dict_size = d->m_dict_size, total_lz_bytes = d->m_total_lz_bytes, num_flags_left = d->m_num_flags_left;
  mz_uint8 *pLZ_code_buf = d->m_pLZ_code_buf, *pLZ_flags = d->m_pLZ_flags;
  mz_uint cur_pos = lookahead_pos & TDEFL_LZ_DICT_SIZE_MASK;

  while ((d->m_src_buf_left) || ((d->m_flush) && (lookahead_size)))
  {
	const mz_uint TDEFL_COMP_FAST_LOOKAHEAD_SIZE = 4096;
	mz_uint dst_pos = (lookahead_pos + lookahead_size) & TDEFL_LZ_DICT_SIZE_MASK;
	mz_uint num_bytes_to_process = (mz_uint)MZ_MIN(d->m_src_buf_left, TDEFL_COMP_FAST_LOOKAHEAD_SIZE - lookahead_size);
	d->m_src_buf_left -= num_bytes_to_process;
	lookahead_size += num_bytes_to_process;

	while (num_bytes_to_process)
	{
	  mz_uint32 n = MZ_MIN(TDEFL_LZ_DICT_SIZE - dst_pos, num_bytes_to_process);
	  memcpy(d->m_dict + dst_pos, d->m_pSrc, n);
	  if (dst_pos < (TDEFL_MAX_MATCH_LEN - 1))
		memcpy(d->m_dict + TDEFL_LZ_DICT_SIZE + dst_pos, d->m_pSrc, MZ_MIN(n, (TDEFL_MAX_MATCH_LEN - 1) - dst_pos));
	  d->m_pSrc += n;
	  dst_pos = (dst_pos + n) & TDEFL_LZ_DICT_SIZE_MASK;
	  num_bytes_to_process -= n;
	}

	dict_size = MZ_MIN(TDEFL_LZ_DICT_SIZE - lookahead_size, dict_size);
	if ((!d->m_flush) && (lookahead_size < TDEFL_COMP_FAST_LOOKAHEAD_SIZE)) break;

	while (lookahead_size >= 4)
	{
	  mz_uint cur_match_dist, cur_match_len = 1;
	  mz_uint8 *pCur_dict = d->m_dict + cur_pos;
	  mz_uint first_trigram = (*(const mz_uint32 *)pCur_dict) & 0xFFFFFF;
	  mz_uint hash = (first_trigram ^ (first_trigram >> (24 - (TDEFL_LZ_HASH_BITS - 8)))) & TDEFL_LEVEL1_HASH_SIZE_MASK;
	  mz_uint probe_pos = d->m_hash[hash];
	  d->m_hash[hash] = (mz_uint16)lookahead_pos;

	  if (((cur_match_dist = (mz_uint16)(lookahead_pos - probe_pos)) <= dict_size) && ((*(const mz_uint32 *)(d->m_dict + (probe_pos &= TDEFL_LZ_DICT_SIZE_MASK)) & 0xFFFFFF) == first_trigram))
	  {
		const mz_uint16 *p = (const mz_uint16 *)pCur_dict;
		const mz_uint16 *q = (const mz_uint16 *)(d->m_dict + probe_pos);
		mz_uint32 probe_len = 32;
		do { } while ( (TDEFL_READ_UNALIGNED_WORD(++p) == TDEFL_READ_UNALIGNED_WORD(++q)) && (TDEFL_READ_UNALIGNED_WORD(++p) == TDEFL_READ_UNALIGNED_WORD(++q)) &&
		  (TDEFL_READ_UNALIGNED_WORD(++p) == TDEFL_READ_UNALIGNED_WORD(++q)) && (TDEFL_READ_UNALIGNED_WORD(++p) == TDEFL_READ_UNALIGNED_WORD(++q)) && (--probe_len > 0) );
		cur_match_len = ((mz_uint)(p - (const mz_uint16 *)pCur_dict) * 2) + (mz_uint)(*(const mz_uint8 *)p == *(const mz_uint8 *)q);
		if (!probe_len)
		  cur_match_len = cur_match_dist ? TDEFL_MAX_MATCH_LEN : 0;

		if ((cur_match_len < TDEFL_MIN_MATCH_LEN) || ((cur_match_len == TDEFL_MIN_MATCH_LEN) && (cur_match_dist >= 8U*1024U)))
		{
		  cur_match_len = 1;
		  *pLZ_code_buf++ = (mz_uint8)first_trigram;
		  *pLZ_flags = (mz_uint8)(*pLZ_flags >> 1);
		  d->m_huff_count[0][(mz_uint8)first_trigram]++;
		}
		else
		{
		  mz_uint32 s0, s1;
		  cur_match_len = MZ_MIN(cur_match_len, lookahead_size);

		  MZ_ASSERT((cur_match_len >= TDEFL_MIN_MATCH_LEN) && (cur_match_dist >= 1) && (cur_match_dist <= TDEFL_LZ_DICT_SIZE));

		  cur_match_dist--;

		  pLZ_code_buf[0] = (mz_uint8)(cur_match_len - TDEFL_MIN_MATCH_LEN);
		  *(mz_uint16 *)(&pLZ_code_buf[1]) = (mz_uint16)cur_match_dist;
		  pLZ_code_buf += 3;
		  *pLZ_flags = (mz_uint8)((*pLZ_flags >> 1) | 0x80);

		  s0 = s_tdefl_small_dist_sym[cur_match_dist & 511];
		  s1 = s_tdefl_large_dist_sym[cur_match_dist >> 8];
		  d->m_huff_count[1][(cur_match_dist < 512) ? s0 : s1]++;

		  d->m_huff_count[0][s_tdefl_len_sym[cur_match_len - TDEFL_MIN_MATCH_LEN]]++;
		}
	  }
	  else
	  {
		*pLZ_code_buf++ = (mz_uint8)first_trigram;
		*pLZ_flags = (mz_uint8)(*pLZ_flags >> 1);
		d->m_huff_count[0][(mz_uint8)first_trigram]++;
	  }

	  if (--num_flags_left == 0) { num_flags_left = 8; pLZ_flags = pLZ_code_buf++; }

	  total_lz_bytes += cur_match_len;
	  lookahead_pos += cur_match_len;
	  dict_size = MZ_MIN(dict_size + cur_match_len, TDEFL_LZ_DICT_SIZE);
	  cur_pos = (cur_pos + cur_match_len) & TDEFL_LZ_DICT_SIZE_MASK;
	  MZ_ASSERT(lookahead_size >= cur_match_len);
	  lookahead_size -= cur_match_len;

	  if (pLZ_code_buf > &d->m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE - 8])
	  {
		int n;
		d->m_lookahead_pos = lookahead_pos; d->m_lookahead_size = lookahead_size; d->m_dict_size = dict_size;
		d->m_total_lz_bytes = total_lz_bytes; d->m_pLZ_code_buf = pLZ_code_buf; d->m_pLZ_flags = pLZ_flags; d->m_num_flags_left = num_flags_left;
		if ((n = tdefl_flush_block(d, 0)) != 0)
		  return (n < 0) ? MZ_FALSE : MZ_TRUE;
		total_lz_bytes = d->m_total_lz_bytes; pLZ_code_buf = d->m_pLZ_code_buf; pLZ_flags = d->m_pLZ_flags; num_flags_left = d->m_num_flags_left;
	  }
	}

	while (lookahead_size)
	{
	  mz_uint8 lit = d->m_dict[cur_pos];

	  total_lz_bytes++;
	  *pLZ_code_buf++ = lit;
	  *pLZ_flags = (mz_uint8)(*pLZ_flags >> 1);
	  if (--num_flags_left == 0) { num_flags_left = 8; pLZ_flags = pLZ_code_buf++; }

	  d->m_huff_count[0][lit]++;

	  lookahead_pos++;
	  dict_size = MZ_MIN(dict_size + 1, TDEFL_LZ_DICT_SIZE);
	  cur_pos = (cur_pos + 1) & TDEFL_LZ_DICT_SIZE_MASK;
	  lookahead_size--;

	  if (pLZ_code_buf > &d->m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE - 8])
	  {
		int n;
		d->m_lookahead_pos = lookahead_pos; d->m_lookahead_size = lookahead_size; d->m_dict_size = dict_size;
		d->m_total_lz_bytes = total_lz_bytes; d->m_pLZ_code_buf = pLZ_code_buf; d->m_pLZ_flags = pLZ_flags; d->m_num_flags_left = num_flags_left;
		if ((n = tdefl_flush_block(d, 0)) != 0)
		  return (n < 0) ? MZ_FALSE : MZ_TRUE;
		total_lz_bytes = d->m_total_lz_bytes; pLZ_code_buf = d->m_pLZ_code_buf; pLZ_flags = d->m_pLZ_flags; num_flags_left = d->m_num_flags_left;
	  }
	}
  }

  d->m_lookahead_pos = lookahead_pos; d->m_lookahead_size = lookahead_size; d->m_dict_size = dict_size;
  d->m_total_lz_bytes = total_lz_bytes; d->m_pLZ_code_buf = pLZ_code_buf; d->m_pLZ_flags = pLZ_flags; d->m_num_flags_left = num_flags_left;
  return MZ_TRUE;
}
#endif // MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN

static MZ_FORCEINLINE void tdefl_record_literal(tdefl_compressor *d, mz_uint8 lit)
{
  d->m_total_lz_bytes++;
  *d->m_pLZ_code_buf++ = lit;
  *d->m_pLZ_flags = (mz_uint8)(*d->m_pLZ_flags >> 1); if (--d->m_num_flags_left == 0) { d->m_num_flags_left = 8; d->m_pLZ_flags = d->m_pLZ_code_buf++; }
  d->m_huff_count[0][lit]++;
}

static MZ_FORCEINLINE void tdefl_record_match(tdefl_compressor *d, mz_uint match_len, mz_uint match_dist)
{
  mz_uint32 s0, s1;

  MZ_ASSERT((match_len >= TDEFL_MIN_MATCH_LEN) && (match_dist >= 1) && (match_dist <= TDEFL_LZ_DICT_SIZE));

  d->m_total_lz_bytes += match_len;

  d->m_pLZ_code_buf[0] = (mz_uint8)(match_len - TDEFL_MIN_MATCH_LEN);

  match_dist -= 1;
  d->m_pLZ_code_buf[1] = (mz_uint8)(match_dist & 0xFF);
  d->m_pLZ_code_buf[2] = (mz_uint8)(match_dist >> 8); d->m_pLZ_code_buf += 3;

  *d->m_pLZ_flags = (mz_uint8)((*d->m_pLZ_flags >> 1) | 0x80); if (--d->m_num_flags_left == 0) { d->m_num_flags_left = 8; d->m_pLZ_flags = d->m_pLZ_code_buf++; }

  s0 = s_tdefl_small_dist_sym[match_dist & 511]; s1 = s_tdefl_large_dist_sym[(match_dist >> 8) & 127];
  d->m_huff_count[1][(match_dist < 512) ? s0 : s1]++;

  if (match_len >= TDEFL_MIN_MATCH_LEN) d->m_huff_count[0][s_tdefl_len_sym[match_len - TDEFL_MIN_MATCH_LEN]]++;
}

static mz_bool tdefl_compress_normal(tdefl_compressor *d)
{
  const mz_uint8 *pSrc = d->m_pSrc; size_t src_buf_left = d->m_src_buf_left;
  tdefl_flush flush = d->m_flush;

  while ((src_buf_left) || ((flush) && (d->m_lookahead_size)))
  {
	mz_uint len_to_move, cur_match_dist, cur_match_len, cur_pos;
	// Update dictionary and hash chains. Keeps the lookahead size equal to TDEFL_MAX_MATCH_LEN.
	if ((d->m_lookahead_size + d->m_dict_size) >= (TDEFL_MIN_MATCH_LEN - 1))
	{
	  mz_uint dst_pos = (d->m_lookahead_pos + d->m_lookahead_size) & TDEFL_LZ_DICT_SIZE_MASK, ins_pos = d->m_lookahead_pos + d->m_lookahead_size - 2;
	  mz_uint hash = (d->m_dict[ins_pos & TDEFL_LZ_DICT_SIZE_MASK] << TDEFL_LZ_HASH_SHIFT) ^ d->m_dict[(ins_pos + 1) & TDEFL_LZ_DICT_SIZE_MASK];
	  mz_uint num_bytes_to_process = (mz_uint)MZ_MIN(src_buf_left, TDEFL_MAX_MATCH_LEN - d->m_lookahead_size);
	  const mz_uint8 *pSrc_end = pSrc + num_bytes_to_process;
	  src_buf_left -= num_bytes_to_process;
	  d->m_lookahead_size += num_bytes_to_process;
	  while (pSrc != pSrc_end)
	  {
		mz_uint8 c = *pSrc++; d->m_dict[dst_pos] = c; if (dst_pos < (TDEFL_MAX_MATCH_LEN - 1)) d->m_dict[TDEFL_LZ_DICT_SIZE + dst_pos] = c;
		hash = ((hash << TDEFL_LZ_HASH_SHIFT) ^ c) & (TDEFL_LZ_HASH_SIZE - 1);
		d->m_next[ins_pos & TDEFL_LZ_DICT_SIZE_MASK] = d->m_hash[hash]; d->m_hash[hash] = (mz_uint16)(ins_pos);
		dst_pos = (dst_pos + 1) & TDEFL_LZ_DICT_SIZE_MASK; ins_pos++;
	  }
	}
	else
	{
	  while ((src_buf_left) && (d->m_lookahead_size < TDEFL_MAX_MATCH_LEN))
	  {
		mz_uint8 c = *pSrc++;
		mz_uint dst_pos = (d->m_lookahead_pos + d->m_lookahead_size) & TDEFL_LZ_DICT_SIZE_MASK;
		src_buf_left--;
		d->m_dict[dst_pos] = c;
		if (dst_pos < (TDEFL_MAX_MATCH_LEN - 1))
		  d->m_dict[TDEFL_LZ_DICT_SIZE + dst_pos] = c;
		if ((++d->m_lookahead_size + d->m_dict_size) >= TDEFL_MIN_MATCH_LEN)
		{
		  mz_uint ins_pos = d->m_lookahead_pos + (d->m_lookahead_size - 1) - 2;
		  mz_uint hash = ((d->m_dict[ins_pos & TDEFL_LZ_DICT_SIZE_MASK] << (TDEFL_LZ_HASH_SHIFT * 2)) ^ (d->m_dict[(ins_pos + 1) & TDEFL_LZ_DICT_SIZE_MASK] << TDEFL_LZ_HASH_SHIFT) ^ c) & (TDEFL_LZ_HASH_SIZE - 1);
		  d->m_next[ins_pos & TDEFL_LZ_DICT_SIZE_MASK] = d->m_hash[hash]; d->m_hash[hash] = (mz_uint16)(ins_pos);
		}
	  }
	}
	d->m_dict_size = MZ_MIN(TDEFL_LZ_DICT_SIZE - d->m_lookahead_size, d->m_dict_size);
	if ((!flush) && (d->m_lookahead_size < TDEFL_MAX_MATCH_LEN))
	  break;

	// Simple lazy/greedy parsing state machine.
	len_to_move = 1; cur_match_dist = 0; cur_match_len = d->m_saved_match_len ? d->m_saved_match_len : (TDEFL_MIN_MATCH_LEN - 1); cur_pos = d->m_lookahead_pos & TDEFL_LZ_DICT_SIZE_MASK;
	if (d->m_flags & (TDEFL_RLE_MATCHES | TDEFL_FORCE_ALL_RAW_BLOCKS))
	{
	  if ((d->m_dict_size) && (!(d->m_flags & TDEFL_FORCE_ALL_RAW_BLOCKS)))
	  {
		mz_uint8 c = d->m_dict[(cur_pos - 1) & TDEFL_LZ_DICT_SIZE_MASK];
		cur_match_len = 0; while (cur_match_len < d->m_lookahead_size) { if (d->m_dict[cur_pos + cur_match_len] != c) break; cur_match_len++; }
		if (cur_match_len < TDEFL_MIN_MATCH_LEN) cur_match_len = 0; else cur_match_dist = 1;
	  }
	}
	else
	{
	  tdefl_find_match(d, d->m_lookahead_pos, d->m_dict_size, d->m_lookahead_size, &cur_match_dist, &cur_match_len);
	}
	if (((cur_match_len == TDEFL_MIN_MATCH_LEN) && (cur_match_dist >= 8U*1024U)) || (cur_pos == cur_match_dist) || ((d->m_flags & TDEFL_FILTER_MATCHES) && (cur_match_len <= 5)))
	{
	  cur_match_dist = cur_match_len = 0;
	}
	if (d->m_saved_match_len)
	{
	  if (cur_match_len > d->m_saved_match_len)
	  {
		tdefl_record_literal(d, (mz_uint8)d->m_saved_lit);
		if (cur_match_len >= 128)
		{
		  tdefl_record_match(d, cur_match_len, cur_match_dist);
		  d->m_saved_match_len = 0; len_to_move = cur_match_len;
		}
		else
		{
		  d->m_saved_lit = d->m_dict[cur_pos]; d->m_saved_match_dist = cur_match_dist; d->m_saved_match_len = cur_match_len;
		}
	  }
	  else
	  {
		tdefl_record_match(d, d->m_saved_match_len, d->m_saved_match_dist);
		len_to_move = d->m_saved_match_len - 1; d->m_saved_match_len = 0;
	  }
	}
	else if (!cur_match_dist)
	  tdefl_record_literal(d, d->m_dict[MZ_MIN(cur_pos, sizeof(d->m_dict) - 1)]);
	else if ((d->m_greedy_parsing) || (d->m_flags & TDEFL_RLE_MATCHES) || (cur_match_len >= 128))
	{
	  tdefl_record_match(d, cur_match_len, cur_match_dist);
	  len_to_move = cur_match_len;
	}
	else
	{
	  d->m_saved_lit = d->m_dict[MZ_MIN(cur_pos, sizeof(d->m_dict) - 1)]; d->m_saved_match_dist = cur_match_dist; d->m_saved_match_len = cur_match_len;
	}
	// Move the lookahead forward by len_to_move bytes.
	d->m_lookahead_pos += len_to_move;
	MZ_ASSERT(d->m_lookahead_size >= len_to_move);
	d->m_lookahead_size -= len_to_move;
	d->m_dict_size = MZ_MIN(d->m_dict_size + len_to_move, TDEFL_LZ_DICT_SIZE);
	// Check if it's time to flush the current LZ codes to the internal output buffer.
	if ( (d->m_pLZ_code_buf > &d->m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE - 8]) ||
		 ( (d->m_total_lz_bytes > 31*1024) && (((((mz_uint)(d->m_pLZ_code_buf - d->m_lz_code_buf) * 115) >> 7) >= d->m_total_lz_bytes) || (d->m_flags & TDEFL_FORCE_ALL_RAW_BLOCKS))) )
	{
	  int n;
	  d->m_pSrc = pSrc; d->m_src_buf_left = src_buf_left;
	  if ((n = tdefl_flush_block(d, 0)) != 0)
		return (n < 0) ? MZ_FALSE : MZ_TRUE;
	}
  }

  d->m_pSrc = pSrc; d->m_src_buf_left = src_buf_left;
  return MZ_TRUE;
}

static tdefl_status tdefl_flush_output_buffer(tdefl_compressor *d)
{
  if (d->m_pIn_buf_size)
  {
	*d->m_pIn_buf_size = d->m_pSrc - (const mz_uint8 *)d->m_pIn_buf;
  }

  if (d->m_pOut_buf_size)
  {
	size_t n = MZ_MIN(*d->m_pOut_buf_size - d->m_out_buf_ofs, d->m_output_flush_remaining);
	memcpy((mz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs, d->m_output_buf + d->m_output_flush_ofs, n);
	d->m_output_flush_ofs += (mz_uint)n;
	d->m_output_flush_remaining -= (mz_uint)n;
	d->m_out_buf_ofs += n;

	*d->m_pOut_buf_size = d->m_out_buf_ofs;
  }

  return (d->m_finished && !d->m_output_flush_remaining) ? TDEFL_STATUS_DONE : TDEFL_STATUS_OKAY;
}

tdefl_status tdefl_compress(tdefl_compressor *d, const void *pIn_buf, size_t *pIn_buf_size, void *pOut_buf, size_t *pOut_buf_size, tdefl_flush flush)
{
  if (!d)
  {
	if (pIn_buf_size) *pIn_buf_size = 0;
	if (pOut_buf_size) *pOut_buf_size = 0;
	return TDEFL_STATUS_BAD_PARAM;
  }

  d->m_pIn_buf = pIn_buf; d->m_pIn_buf_size = pIn_buf_size;
  d->m_pOut_buf = pOut_buf; d->m_pOut_buf_size = pOut_buf_size;
  d->m_pSrc = (const mz_uint8 *)(pIn_buf); d->m_src_buf_left = pIn_buf_size ? *pIn_buf_size : 0;
  d->m_out_buf_ofs = 0;
  d->m_flush = flush;

  if ( ((d->m_pPut_buf_func != NULL) == ((pOut_buf != NULL) || (pOut_buf_size != NULL))) || (d->m_prev_return_status != TDEFL_STATUS_OKAY) ||
		(d->m_wants_to_finish && (flush != TDEFL_FINISH)) || (pIn_buf_size && *pIn_buf_size && !pIn_buf) || (pOut_buf_size && *pOut_buf_size && !pOut_buf) )
  {
	if (pIn_buf_size) *pIn_buf_size = 0;
	if (pOut_buf_size) *pOut_buf_size = 0;
	return (d->m_prev_return_status = TDEFL_STATUS_BAD_PARAM);
  }
  d->m_wants_to_finish |= (flush == TDEFL_FINISH);

  if ((d->m_output_flush_remaining) || (d->m_finished))
	return (d->m_prev_return_status = tdefl_flush_output_buffer(d));

#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN
  if (((d->m_flags & TDEFL_MAX_PROBES_MASK) == 1) &&
	  ((d->m_flags & TDEFL_GREEDY_PARSING_FLAG) != 0) &&
	  ((d->m_flags & (TDEFL_FILTER_MATCHES | TDEFL_FORCE_ALL_RAW_BLOCKS | TDEFL_RLE_MATCHES)) == 0))
  {
	if (!tdefl_compress_fast(d))
	  return d->m_prev_return_status;
  }
  else
#endif // #if MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN
  {
	if (!tdefl_compress_normal(d))
	  return d->m_prev_return_status;
  }

  if ((d->m_flags & (TDEFL_WRITE_ZLIB_HEADER | TDEFL_COMPUTE_ADLER32)) && (pIn_buf))
	d->m_adler32 = (mz_uint32)mz_adler32(d->m_adler32, (const mz_uint8 *)pIn_buf, d->m_pSrc - (const mz_uint8 *)pIn_buf);

  if ((flush) && (!d->m_lookahead_size) && (!d->m_src_buf_left) && (!d->m_output_flush_remaining))
  {
	if (tdefl_flush_block(d, flush) < 0)
	  return d->m_prev_return_status;
	d->m_finished = (flush == TDEFL_FINISH);
	if (flush == TDEFL_FULL_FLUSH) { MZ_CLEAR_OBJ(d->m_hash); MZ_CLEAR_OBJ(d->m_next); d->m_dict_size = 0; }
  }

  return (d->m_prev_return_status = tdefl_flush_output_buffer(d));
}

tdefl_status tdefl_compress_buffer(tdefl_compressor *d, const void *pIn_buf, size_t in_buf_size, tdefl_flush flush)
{
  MZ_ASSERT(d->m_pPut_buf_func); return tdefl_compress(d, pIn_buf, &in_buf_size, NULL, NULL, flush);
}

tdefl_status tdefl_init(tdefl_compressor *d, tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags)
{
  d->m_pPut_buf_func = pPut_buf_func; d->m_pPut_buf_user = pPut_buf_user;
  d->m_flags = (mz_uint)(flags); d->m_max_probes[0] = 1 + ((flags & 0xFFF) + 2) / 3; d->m_greedy_parsing = (flags & TDEFL_GREEDY_PARSING_FLAG) != 0;
  d->m_max_probes[1] = 1 + (((flags & 0xFFF) >> 2) + 2) / 3;
  if (!(flags & TDEFL_NONDETERMINISTIC_PARSING_FLAG)) MZ_CLEAR_OBJ(d->m_hash);
  d->m_lookahead_pos = d->m_lookahead_size = d->m_dict_size = d->m_total_lz_bytes = d->m_lz_code_buf_dict_pos = d->m_bits_in = 0;
  d->m_output_flush_ofs = d->m_output_flush_remaining = d->m_finished = d->m_block_index = d->m_bit_buffer = d->m_wants_to_finish = 0;
  d->m_pLZ_code_buf = d->m_lz_code_buf + 1; d->m_pLZ_flags = d->m_lz_code_buf; d->m_num_flags_left = 8;
  d->m_pOutput_buf = d->m_output_buf; d->m_pOutput_buf_end = d->m_output_buf; d->m_prev_return_status = TDEFL_STATUS_OKAY;
  d->m_saved_match_dist = d->m_saved_match_len = d->m_saved_lit = 0; d->m_adler32 = 1;
  d->m_pIn_buf = NULL; d->m_pOut_buf = NULL;
  d->m_pIn_buf_size = NULL; d->m_pOut_buf_size = NULL;
  d->m_flush = TDEFL_NO_FLUSH; d->m_pSrc = NULL; d->m_src_buf_left = 0; d->m_out_buf_ofs = 0;
  memset(&d->m_huff_count[0][0], 0, sizeof(d->m_huff_count[0][0]) * TDEFL_MAX_HUFF_SYMBOLS_0);
  memset(&d->m_huff_count[1][0], 0, sizeof(d->m_huff_count[1][0]) * TDEFL_MAX_HUFF_SYMBOLS_1);
  return TDEFL_STATUS_OKAY;
}

tdefl_status tdefl_get_prev_return_status(tdefl_compressor *d)
{
  return d->m_prev_return_status;
}

mz_uint32 tdefl_get_adler32(tdefl_compressor *d)
{
  return d->m_adler32;
}

mz_bool tdefl_compress_mem_to_output(const void *pBuf, size_t buf_len, tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags)
{
  tdefl_compressor *pComp; mz_bool succeeded; if (((buf_len) && (!pBuf)) || (!pPut_buf_func)) return MZ_FALSE;
  pComp = (tdefl_compressor*)MZ_MALLOC(sizeof(tdefl_compressor)); if (!pComp) return MZ_FALSE;
  succeeded = (tdefl_init(pComp, pPut_buf_func, pPut_buf_user, flags) == TDEFL_STATUS_OKAY);
  succeeded = succeeded && (tdefl_compress_buffer(pComp, pBuf, buf_len, TDEFL_FINISH) == TDEFL_STATUS_DONE);
  MZ_FREE(pComp); return succeeded;
}

typedef struct
{
  size_t m_size, m_capacity;
  mz_uint8 *m_pBuf;
  mz_bool m_expandable;
} tdefl_output_buffer;

static mz_bool tdefl_output_buffer_putter(const void *pBuf, int len, void *pUser)
{
  tdefl_output_buffer *p = (tdefl_output_buffer *)pUser;
  size_t new_size = p->m_size + len;
  if (new_size > p->m_capacity)
  {
	size_t new_capacity = p->m_capacity; mz_uint8 *pNew_buf; if (!p->m_expandable) return MZ_FALSE;
	do { new_capacity = MZ_MAX(128U, new_capacity << 1U); } while (new_size > new_capacity);
	pNew_buf = (mz_uint8*)MZ_REALLOC(p->m_pBuf, new_capacity); if (!pNew_buf) return MZ_FALSE;
	p->m_pBuf = pNew_buf; p->m_capacity = new_capacity;
  }
  memcpy((mz_uint8*)p->m_pBuf + p->m_size, pBuf, len); p->m_size = new_size;
  return MZ_TRUE;
}

void *tdefl_compress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags)
{
  tdefl_output_buffer out_buf; MZ_CLEAR_OBJ(out_buf);
  if (!pOut_len) return MZ_FALSE; else *pOut_len = 0;
  out_buf.m_expandable = MZ_TRUE;
  if (!tdefl_compress_mem_to_output(pSrc_buf, src_buf_len, tdefl_output_buffer_putter, &out_buf, flags)) return NULL;
  *pOut_len = out_buf.m_size; return out_buf.m_pBuf;
}

size_t tdefl_compress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags)
{
  tdefl_output_buffer out_buf; MZ_CLEAR_OBJ(out_buf);
  if (!pOut_buf) return 0;
  out_buf.m_pBuf = (mz_uint8*)pOut_buf; out_buf.m_capacity = out_buf_len;
  if (!tdefl_compress_mem_to_output(pSrc_buf, src_buf_len, tdefl_output_buffer_putter, &out_buf, flags)) return 0;
  return out_buf.m_size;
}

#ifndef MINIZ_NO_ZLIB_APIS
static const mz_uint s_tdefl_num_probes[11] = { 0, 1, 6, 32,  16, 32, 128, 256,  512, 768, 1500 };

// level may actually range from [0,10] (10 is a "hidden" max level, where we want a bit more compression and it's fine if throughput to fall off a cliff on some files).
mz_uint tdefl_create_comp_flags_from_zip_params(int level, int window_bits, int strategy)
{
  mz_uint comp_flags = s_tdefl_num_probes[(level >= 0) ? MZ_MIN(10, level) : MZ_DEFAULT_LEVEL] | ((level <= 3) ? TDEFL_GREEDY_PARSING_FLAG : 0);
  if (window_bits > 0) comp_flags |= TDEFL_WRITE_ZLIB_HEADER;

  if (!level) comp_flags |= TDEFL_FORCE_ALL_RAW_BLOCKS;
  else if (strategy == MZ_FILTERED) comp_flags |= TDEFL_FILTER_MATCHES;
  else if (strategy == MZ_HUFFMAN_ONLY) comp_flags &= ~TDEFL_MAX_PROBES_MASK;
  else if (strategy == MZ_FIXED) comp_flags |= TDEFL_FORCE_ALL_STATIC_BLOCKS;
  else if (strategy == MZ_RLE) comp_flags |= TDEFL_RLE_MATCHES;

  return comp_flags;
}
#endif //MINIZ_NO_ZLIB_APIS

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4204) // nonstandard extension used : non-constant aggregate initializer (also supported by GNU C and C99, so no big deal)
#endif

// Simple PNG writer function by Alex Evans, 2011. Released into the public domain: https://gist.github.com/908299, more context at
// http://altdevblogaday.org/2011/04/06/a-smaller-jpg-encoder/.
// This is actually a modification of Alex's original code so PNG files generated by this function pass pngcheck.
void *tdefl_write_image_to_png_file_in_memory_ex(const void *pImage, int w, int h, int num_chans, size_t *pLen_out, mz_uint level, mz_bool flip)
{
  // Using a local copy of this array here in case MINIZ_NO_ZLIB_APIS was defined.
  static const mz_uint s_tdefl_png_num_probes[11] = { 0, 1, 6, 32,  16, 32, 128, 256,  512, 768, 1500 };
  tdefl_compressor *pComp = (tdefl_compressor *)MZ_MALLOC(sizeof(tdefl_compressor)); tdefl_output_buffer out_buf; int i, bpl = w * num_chans, y, z; mz_uint32 c; *pLen_out = 0;
  if (!pComp) return NULL;
  MZ_CLEAR_OBJ(out_buf); out_buf.m_expandable = MZ_TRUE; out_buf.m_capacity = 57+MZ_MAX(64, (1+bpl)*h); if (NULL == (out_buf.m_pBuf = (mz_uint8*)MZ_MALLOC(out_buf.m_capacity))) { MZ_FREE(pComp); return NULL; }
  // write dummy header
  for (z = 41; z; --z) tdefl_output_buffer_putter(&z, 1, &out_buf);
  // compress image data
  tdefl_init(pComp, tdefl_output_buffer_putter, &out_buf, s_tdefl_png_num_probes[MZ_MIN(10, level)] | TDEFL_WRITE_ZLIB_HEADER);
  for (y = 0; y < h; ++y) { tdefl_compress_buffer(pComp, &z, 1, TDEFL_NO_FLUSH); tdefl_compress_buffer(pComp, (mz_uint8*)pImage + (flip ? (h - 1 - y) : y) * bpl, bpl, TDEFL_NO_FLUSH); }
  if (tdefl_compress_buffer(pComp, NULL, 0, TDEFL_FINISH) != TDEFL_STATUS_DONE) { MZ_FREE(pComp); MZ_FREE(out_buf.m_pBuf); return NULL; }
  // write real header
  *pLen_out = out_buf.m_size-41;
  {
	static const mz_uint8 chans[] = {0x00, 0x00, 0x04, 0x02, 0x06};
	mz_uint8 pnghdr[41]={0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
	  0,0,(mz_uint8)(w>>8),(mz_uint8)w,0,0,(mz_uint8)(h>>8),(mz_uint8)h,8,chans[num_chans],0,0,0,0,0,0,0,
	  (mz_uint8)(*pLen_out>>24),(mz_uint8)(*pLen_out>>16),(mz_uint8)(*pLen_out>>8),(mz_uint8)*pLen_out,0x49,0x44,0x41,0x54};
	c=(mz_uint32)mz_crc32(MZ_CRC32_INIT,pnghdr+12,17); for (i=0; i<4; ++i, c<<=8) ((mz_uint8*)(pnghdr+29))[i]=(mz_uint8)(c>>24);
	memcpy(out_buf.m_pBuf, pnghdr, 41);
  }
  // write footer (IDAT CRC-32, followed by IEND chunk)
  if (!tdefl_output_buffer_putter("\0\0\0\0\0\0\0\0\x49\x45\x4e\x44\xae\x42\x60\x82", 16, &out_buf)) { *pLen_out = 0; MZ_FREE(pComp); MZ_FREE(out_buf.m_pBuf); return NULL; }
  c = (mz_uint32)mz_crc32(MZ_CRC32_INIT,out_buf.m_pBuf+41-4, *pLen_out+4); for (i=0; i<4; ++i, c<<=8) (out_buf.m_pBuf+out_buf.m_size-16)[i] = (mz_uint8)(c >> 24);
  // compute final size of file, grab compressed data buffer and return
  *pLen_out += 57; MZ_FREE(pComp); return out_buf.m_pBuf;
}
void *tdefl_write_image_to_png_file_in_memory(const void *pImage, int w, int h, int num_chans, size_t *pLen_out)
{
  // Level 6 corresponds to TDEFL_DEFAULT_MAX_PROBES or MZ_DEFAULT_LEVEL (but we can't depend on MZ_DEFAULT_LEVEL being available in case the zlib API's where #defined out)
  return tdefl_write_image_to_png_file_in_memory_ex(pImage, w, h, num_chans, pLen_out, 6, MZ_FALSE);
}

#ifdef _MSC_VER
#pragma warning (pop)
#endif

// ------------------- .ZIP archive reading

#ifndef MINIZ_NO_ARCHIVE_APIS

#ifdef MINIZ_NO_STDIO
  #define MZ_FILE void *
#else
  #include <stdio.h>
  #include <sys/stat.h>

  #if ( defined(_MSC_VER) && _MSC_VER >= 1400 ) || defined(__MINGW64__)
	static FILE *mz_fopen(const char *pFilename, const char *pMode)
	{
	  FILE* pFile = NULL;
	  fopen_s(&pFile, pFilename, pMode);
	  return pFile;
	}
	static FILE *mz_freopen(const char *pPath, const char *pMode, FILE *pStream)
	{
	  FILE* pFile = NULL;
	  if (freopen_s(&pFile, pPath, pMode, pStream))
		return NULL;
	  return pFile;
	}
	#define MZ_FILE FILE
	#define MZ_FOPEN mz_fopen
	#define MZ_FCLOSE fclose
	#define MZ_FREAD fread
	#define MZ_FWRITE fwrite
	#define MZ_FTELL64 _ftelli64
	#define MZ_FSEEK64 _fseeki64
	#define MZ_FILE_STAT_STRUCT _stat
	#define MZ_FILE_STAT _stat
	#define MZ_FFLUSH fflush
	#define MZ_FREOPEN mz_freopen
	#define MZ_DELETE_FILE remove
  #elif defined(_MSC_VER) && _MSC_VER < 1400
	#define MZ_FILE FILE
	#define MZ_FOPEN(f, m) fopen(f, m)
	#define MZ_FCLOSE fclose
	#define MZ_FREAD fread
	#define MZ_FWRITE fwrite
	#define MZ_FTELL64 ftell
	#define MZ_FSEEK64 fseek
	#define MZ_FILE_STAT_STRUCT _stat
	#define MZ_FILE_STAT _stat
	#define MZ_FFLUSH fflush
	#define MZ_FREOPEN(f, m, s) freopen(f, m, s)
	#define MZ_DELETE_FILE remove
  #elif defined(__MINGW32__)
	#define MZ_FILE FILE
	#define MZ_FOPEN(f, m) fopen(f, m)
	#define MZ_FCLOSE fclose
	#define MZ_FREAD fread
	#define MZ_FWRITE fwrite
	#define MZ_FTELL64 ftello64
	#define MZ_FSEEK64 fseeko64
	#define MZ_FILE_STAT_STRUCT _stat
	#define MZ_FILE_STAT _stat
	#define MZ_FFLUSH fflush
	#define MZ_FREOPEN(f, m, s) freopen(f, m, s)
	#define MZ_DELETE_FILE remove
  #elif defined(__TINYC__)
	#define MZ_FILE FILE
	#define MZ_FOPEN(f, m) fopen(f, m)
	#define MZ_FCLOSE fclose
	#define MZ_FREAD fread
	#define MZ_FWRITE fwrite
	#define MZ_FTELL64 ftell
	#define MZ_FSEEK64 fseek
	#define MZ_FILE_STAT_STRUCT stat
	#define MZ_FILE_STAT stat
	#define MZ_FFLUSH fflush
	#define MZ_FREOPEN(f, m, s) freopen(f, m, s)
	#define MZ_DELETE_FILE remove
  #elif defined(__GNUC__) && _LARGEFILE64_SOURCE
	#define MZ_FILE FILE
	#define MZ_FOPEN(f, m) fopen64(f, m)
	#define MZ_FCLOSE fclose
	#define MZ_FREAD fread
	#define MZ_FWRITE fwrite
	#define MZ_FTELL64 ftello64
	#define MZ_FSEEK64 fseeko64
	#define MZ_FILE_STAT_STRUCT stat64
	#define MZ_FILE_STAT stat64
	#define MZ_FFLUSH fflush
	#define MZ_FREOPEN(p, m, s) freopen64(p, m, s)
	#define MZ_DELETE_FILE remove
  #else
	#define MZ_FILE FILE
	#define MZ_FOPEN(f, m) fopen(f, m)
	#define MZ_FCLOSE fclose
	#define MZ_FREAD fread
	#define MZ_FWRITE fwrite
	#define MZ_FTELL64 ftello
	#define MZ_FSEEK64 fseeko
	#define MZ_FILE_STAT_STRUCT stat
	#define MZ_FILE_STAT stat
	#define MZ_FFLUSH fflush
	#define MZ_FREOPEN(f, m, s) freopen(f, m, s)
	#define MZ_DELETE_FILE remove
  #endif // #ifdef _MSC_VER
#endif // #ifdef MINIZ_NO_STDIO

#define MZ_TOLOWER(c) ((((c) >= 'A') && ((c) <= 'Z')) ? ((c) - 'A' + 'a') : (c))

// Various ZIP archive enums. To completely avoid cross platform compiler alignment and platform endian issues, miniz.c doesn't use structs for any of this stuff.
enum
{
  // ZIP archive identifiers and record sizes
  MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG = 0x06054b50, MZ_ZIP_CENTRAL_DIR_HEADER_SIG = 0x02014b50, MZ_ZIP_LOCAL_DIR_HEADER_SIG = 0x04034b50,
  MZ_ZIP_LOCAL_DIR_HEADER_SIZE = 30, MZ_ZIP_CENTRAL_DIR_HEADER_SIZE = 46, MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE = 22,
  // ZIP64 archive identifiers and record sizes
  MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIG = 0x06064b50, MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIG = 0x07064b50,
  MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE = 56, MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE = 20,
  MZ_ZIP64_END_OF_CENTRAL_DIR_V2_HEADER_SIZE = 84,
  MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID = 0x0001,
  // WinZip AES extra field (0x9901): 2 id + 2 size + 2 vendor-version + 2 vendor-id("AE") + 1 strength + 2 real-method = 11 bytes total.
  MZ_ZIP_AES_EXTRA_FIELD_HEADER_ID = 0x9901,
  MZ_ZIP_AES_EXTRA_FIELD_SIZE = 11,
  MZ_ZIP_AES_EXTRA_DATA_SIZE = 7,       // the "size" field value (bytes after id+size)
  MZ_ZIP_AES_VENDOR_ID = 0x4541,        // bytes 'A'(0x41),'E'(0x45) -> MZ_READ_LE16 == 0x4541
  // Central directory header record offsets
  MZ_ZIP_CDH_SIG_OFS = 0, MZ_ZIP_CDH_VERSION_MADE_BY_OFS = 4, MZ_ZIP_CDH_VERSION_NEEDED_OFS = 6, MZ_ZIP_CDH_BIT_FLAG_OFS = 8,
  MZ_ZIP_CDH_METHOD_OFS = 10, MZ_ZIP_CDH_FILE_TIME_OFS = 12, MZ_ZIP_CDH_FILE_DATE_OFS = 14, MZ_ZIP_CDH_CRC32_OFS = 16,
  MZ_ZIP_CDH_COMPRESSED_SIZE_OFS = 20, MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS = 24, MZ_ZIP_CDH_FILENAME_LEN_OFS = 28, MZ_ZIP_CDH_EXTRA_LEN_OFS = 30,
  MZ_ZIP_CDH_COMMENT_LEN_OFS = 32, MZ_ZIP_CDH_DISK_START_OFS = 34, MZ_ZIP_CDH_INTERNAL_ATTR_OFS = 36, MZ_ZIP_CDH_EXTERNAL_ATTR_OFS = 38, MZ_ZIP_CDH_LOCAL_HEADER_OFS = 42,
  // Local directory header offsets
  MZ_ZIP_LDH_SIG_OFS = 0, MZ_ZIP_LDH_VERSION_NEEDED_OFS = 4, MZ_ZIP_LDH_BIT_FLAG_OFS = 6, MZ_ZIP_LDH_METHOD_OFS = 8, MZ_ZIP_LDH_FILE_TIME_OFS = 10,
  MZ_ZIP_LDH_FILE_DATE_OFS = 12, MZ_ZIP_LDH_CRC32_OFS = 14, MZ_ZIP_LDH_COMPRESSED_SIZE_OFS = 18, MZ_ZIP_LDH_DECOMPRESSED_SIZE_OFS = 22,
  MZ_ZIP_LDH_FILENAME_LEN_OFS = 26, MZ_ZIP_LDH_EXTRA_LEN_OFS = 28,
  // End of central directory offsets
  MZ_ZIP_ECDH_SIG_OFS = 0, MZ_ZIP_ECDH_NUM_THIS_DISK_OFS = 4, MZ_ZIP_ECDH_NUM_DISK_CDIR_OFS = 6, MZ_ZIP_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS = 8,
  MZ_ZIP_ECDH_CDIR_TOTAL_ENTRIES_OFS = 10, MZ_ZIP_ECDH_CDIR_SIZE_OFS = 12, MZ_ZIP_ECDH_CDIR_OFS_OFS = 16, MZ_ZIP_ECDH_COMMENT_SIZE_OFS = 20,
  // ZIP64 end of central directory locator offsets
  MZ_ZIP64_ECDL_SIG_OFS = 0, MZ_ZIP64_ECDL_NUM_DISK_CDIR_OFS = 4, MZ_ZIP64_ECDL_REL_OFS_TO_ZIP64_ECDR_OFS = 8,
  MZ_ZIP64_ECDL_TOTAL_NUMBER_OF_DISKS_OFS = 16,
  // ZIP64 end of central directory header offsets
  MZ_ZIP64_ECDH_SIG_OFS = 0, MZ_ZIP64_ECDH_SIZE_OF_RECORD_OFS = 4, MZ_ZIP64_ECDH_VERSION_MADE_BY_OFS = 12,
  MZ_ZIP64_ECDH_VERSION_NEEDED_OFS = 14, MZ_ZIP64_ECDH_NUM_THIS_DISK_OFS = 16, MZ_ZIP64_ECDH_NUM_DISK_CDIR_OFS = 20,
  MZ_ZIP64_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS = 24, MZ_ZIP64_ECDH_CDIR_TOTAL_ENTRIES_OFS = 32,
  MZ_ZIP64_ECDH_CDIR_SIZE_OFS = 40, MZ_ZIP64_ECDH_CDIR_OFS_OFS = 48,
};

typedef struct
{
  void *m_p;
  size_t m_size, m_capacity;
  mz_uint m_element_size;
} mz_zip_array;

struct mz_zip_internal_state_tag
{
  mz_zip_array m_central_dir;
  mz_zip_array m_central_dir_offsets;
  mz_zip_array m_sorted_central_dir_offsets;
  mz_zip_array m_disk_offsets;
  mz_zip_array m_files;
  mz_uint32 m_num_disks;
  mz_uint64 m_split_size;
  char *m_pZip_filename;
  mz_uint16 m_central_dir_method;
  mz_uint16 m_central_dir_level;
  MZ_FILE *m_pFile;
  void *m_pMem;
  size_t m_mem_size;
  size_t m_mem_capacity;
  // WinZip AES: per-archive default password + settings (used when an entry is encrypted but no per-entry password was given).
  char *m_pPassword;               // heap copy of the default password, or NULL
  size_t m_password_len;
  mz_uint8 m_aes_strength;         // 1=AES-128, 2=AES-192, 3=AES-256 (default 3)
  mz_uint8 m_aes_ae_version;       // 1=AE-1 (keeps CRC), 2=AE-2 (CRC stored as 0). Default 2.
};

#define MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(array_ptr, element_size) (array_ptr)->m_element_size = element_size
#define MZ_ZIP_ARRAY_ELEMENT(array_ptr, element_type, index) ((element_type *)((array_ptr)->m_p))[index]

static MZ_FORCEINLINE void mz_zip_array_clear(mz_zip_archive *pZip, mz_zip_array *pArray)
{
  pZip->m_pFree(pZip->m_pAlloc_opaque, pArray->m_p);
  memset(pArray, 0, sizeof(mz_zip_array));
}

static mz_bool mz_zip_array_ensure_capacity(mz_zip_archive *pZip, mz_zip_array *pArray, size_t min_new_capacity, mz_uint growing)
{
  void *pNew_p; size_t new_capacity = min_new_capacity, max_capacity; MZ_ASSERT(pArray->m_element_size); if (pArray->m_capacity >= min_new_capacity) return MZ_TRUE;
  max_capacity = (size_t)-1 / pArray->m_element_size;
  if (min_new_capacity > max_capacity) return MZ_FALSE;
  if (growing)
  {
	new_capacity = MZ_MAX(1, pArray->m_capacity);
	while (new_capacity < min_new_capacity)
	{
	  if (new_capacity > (max_capacity / 2))
	  {
		new_capacity = min_new_capacity;
		break;
	  }
	  new_capacity *= 2;
	}
  }
  if (NULL == (pNew_p = pZip->m_pRealloc(pZip->m_pAlloc_opaque, pArray->m_p, pArray->m_element_size, new_capacity))) return MZ_FALSE;
  pArray->m_p = pNew_p; pArray->m_capacity = new_capacity;
  return MZ_TRUE;
}

static MZ_FORCEINLINE mz_bool mz_zip_array_reserve(mz_zip_archive *pZip, mz_zip_array *pArray, size_t new_capacity, mz_uint growing)
{
  if (new_capacity > pArray->m_capacity) { if (!mz_zip_array_ensure_capacity(pZip, pArray, new_capacity, growing)) return MZ_FALSE; }
  return MZ_TRUE;
}

static MZ_FORCEINLINE mz_bool mz_zip_array_resize(mz_zip_archive *pZip, mz_zip_array *pArray, size_t new_size, mz_uint growing)
{
  if (new_size > pArray->m_capacity) { if (!mz_zip_array_ensure_capacity(pZip, pArray, new_size, growing)) return MZ_FALSE; }
  pArray->m_size = new_size;
  return MZ_TRUE;
}

static MZ_FORCEINLINE mz_bool mz_zip_array_ensure_room(mz_zip_archive *pZip, mz_zip_array *pArray, size_t n)
{
  return mz_zip_array_reserve(pZip, pArray, pArray->m_size + n, MZ_TRUE);
}

static MZ_FORCEINLINE mz_bool mz_zip_array_push_back(mz_zip_archive *pZip, mz_zip_array *pArray, const void *pElements, size_t n)
{
  size_t orig_size = pArray->m_size; if (!mz_zip_array_resize(pZip, pArray, orig_size + n, MZ_TRUE)) return MZ_FALSE;
  memcpy((mz_uint8*)pArray->m_p + orig_size * pArray->m_element_size, pElements, n * pArray->m_element_size);
  return MZ_TRUE;
}

#ifndef MINIZ_NO_TIME
static mz_time mz_zip_dos_to_time_t(int dos_time, int dos_date) {
#if defined(WIKICMS_VERSION)
  WSDATE tm;
  tm.year = ((dos_date >> 9) & 127) + 1980 - 1900;
  tm.month = ((dos_date >> 5) & 15) - 1;
  tm.dayOfMonth = (dos_date & 31);
  tm.hour = (dos_time >> 11) & 31;
  tm.minute = (dos_time >> 5) & 63;
  tm.second = (dos_time << 1) & 62;
  return WSDate_GetTime(&tm);
#else
  struct tm tm;
  memset(&tm, 0, sizeof(tm));
  tm.tm_isdst = -1;
  tm.tm_year = ((dos_date >> 9) & 127) + 1980 - 1900;
  tm.tm_mon = ((dos_date >> 5) & 15) - 1;
  tm.tm_mday = dos_date & 31;
  tm.tm_hour = (dos_time >> 11) & 31;
  tm.tm_min = (dos_time >> 5) & 63;
  tm.tm_sec = (dos_time << 1) & 62;
  return mktime(&tm);
#endif
}

static void mz_zip_time_to_dos_time(mz_time time, mz_uint16 *pDOS_time,
									  mz_uint16 *pDOS_date) {
#if defined(WIKICMS_VERSION)
  WSDATE tm;
  WSTime_GetDate(time, &tm);
  *pDOS_time = (mz_uint16)(((tm.hour) << 11) + ((tm.minute) << 5) + ((tm.second) >> 1));
  *pDOS_date = (mz_uint16)(((tm.year + 1900 - 1980) << 9) + ((tm.month + 1) << 5) + (tm.dayOfMonth));
#else
  #ifdef _MSC_VER
	struct tm tm_struct;
	struct tm *tm = &tm_struct;
	errno_t err = localtime_s(tm, &time);
	if (err) {
	  *pDOS_date = 0;
	  *pDOS_time = 0;
	  return;
	}
  #else
	struct tm *tm = localtime(&time);
  #endif /* #ifdef _MSC_VER */

	*pDOS_time = (mz_uint16)(((tm->tm_hour) << 11) + ((tm->tm_min) << 5) +
							((tm->tm_sec) >> 1));
	*pDOS_date = (mz_uint16)(((tm->tm_year + 1900 - 1980) << 9) +
							((tm->tm_mon + 1) << 5) + tm->tm_mday);
#endif
}
#endif

#ifndef MINIZ_NO_STDIO
static mz_bool mz_zip_get_file_modified_time(const char *pFilename, mz_uint16 *pDOS_time, mz_uint16 *pDOS_date)
{
#ifdef MINIZ_NO_TIME
  (void)pFilename; *pDOS_date = *pDOS_time = 0;
#else
  struct MZ_FILE_STAT_STRUCT file_stat;
  // On Linux with x86 glibc, this call will fail on large files (>= 0x80000000 bytes) unless you compiled with _LARGEFILE64_SOURCE. Argh.
  if (MZ_FILE_STAT(pFilename, &file_stat) != 0)
	return MZ_FALSE;
  mz_zip_time_to_dos_time(file_stat.st_mtime, pDOS_time, pDOS_date);
#endif // #ifdef MINIZ_NO_TIME
  return MZ_TRUE;
}

#ifndef MINIZ_NO_TIME
#include <sys/utime.h>
static mz_bool mz_zip_set_file_times(const char *pFilename, time_t access_time, time_t modified_time)
{
  struct utimbuf t; t.actime = access_time; t.modtime = modified_time;
  return !utime(pFilename, &t);
}
#endif // #ifndef MINIZ_NO_TIME
#endif // #ifndef MINIZ_NO_STDIO

static mz_bool mz_zip_reader_init_internal(mz_zip_archive *pZip, mz_uint32 flags)
{
  (void)flags;
  if ((!pZip) || (pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_INVALID))
	return MZ_FALSE;

  if (!pZip->m_pAlloc) pZip->m_pAlloc = def_alloc_func;
  if (!pZip->m_pFree) pZip->m_pFree = def_free_func;
  if (!pZip->m_pRealloc) pZip->m_pRealloc = def_realloc_func;

  pZip->m_zip_mode = MZ_ZIP_MODE_READING;
  pZip->m_archive_size = 0;
  pZip->m_central_directory_file_ofs = 0;
  pZip->m_total_files = 0;

  if (NULL == (pZip->m_pState = (mz_zip_internal_state *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, sizeof(mz_zip_internal_state))))
	return MZ_FALSE;
  memset(pZip->m_pState, 0, sizeof(mz_zip_internal_state));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir, sizeof(mz_uint8));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir_offsets, sizeof(size_t));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_sorted_central_dir_offsets, sizeof(mz_uint32));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_disk_offsets, sizeof(mz_uint64));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_files, sizeof(MZ_FILE *));
  return MZ_TRUE;
}

#ifdef MINIZ_USE_LZMA_SDK
static mz_bool mz_zip_lzma_compress(mz_zip_archive *pZip, const void *pSrc, size_t src_size, mz_uint level, mz_zip_array *pOutput)
{
  size_t capacity, attempt;
  if (src_size > (size_t)-1 - (src_size / 3) - 65536)
	return MZ_FALSE;
  capacity = src_size + src_size / 3 + 65536;
  for (attempt = 0; attempt < 5; ++attempt)
  {
	size_t dest_size = capacity, props_size = LZMA_PROPS_SIZE;
	mz_uint8 *p;
	int result;
	if ((capacity > (size_t)-1 - 4 - LZMA_PROPS_SIZE) ||
		(!mz_zip_array_resize(pZip, pOutput, 4 + LZMA_PROPS_SIZE + capacity, MZ_FALSE)))
	  return MZ_FALSE;
	p = (mz_uint8 *)pOutput->m_p;
	result = LzmaCompress(p + 4 + LZMA_PROPS_SIZE, &dest_size, (const unsigned char *)pSrc, src_size,
	  p + 4, &props_size, (int)MZ_MIN(level, 9), 0, -1, -1, -1, -1, 1);
	if ((result == SZ_OK) && (props_size == LZMA_PROPS_SIZE))
	{
	  p[0] = (mz_uint8)MY_VER_MAJOR;
	  p[1] = (mz_uint8)MY_VER_MINOR;
	  p[2] = (mz_uint8)LZMA_PROPS_SIZE;
	  p[3] = 0;
	  return mz_zip_array_resize(pZip, pOutput, 4 + LZMA_PROPS_SIZE + dest_size, MZ_FALSE);
	}
	if ((result != SZ_ERROR_OUTPUT_EOF) || (capacity > (size_t)-1 / 2))
	  return MZ_FALSE;
	capacity *= 2;
  }
  return MZ_FALSE;
}

static mz_bool mz_zip_lzma_decompress(void *pDst, size_t dst_size, const void *pSrc, size_t src_size)
{
  const mz_uint8 *p = (const mz_uint8 *)pSrc;
  size_t props_size, dest_size = dst_size, source_size;
  if (src_size < 4)
	return MZ_FALSE;
  props_size = (size_t)p[2] | ((size_t)p[3] << 8);
  if ((props_size > src_size - 4) || (props_size > (size_t)-1 - 4))
	return MZ_FALSE;
  source_size = src_size - 4 - props_size;
  if (LzmaUncompress((unsigned char *)pDst, &dest_size, p + 4 + props_size, &source_size, p + 4, props_size) != SZ_OK)
	return MZ_FALSE;
  return dest_size == dst_size;
}
#endif

static mz_bool mz_zip_reader_set_multidisk_offsets(mz_zip_archive *pZip, mz_uint64 archive_size, mz_uint32 num_disks, const mz_uint64 *pDisk_start_offsets)
{
  mz_uint32 i;
  mz_zip_internal_state *pState = pZip->m_pState;
  if ((!pState) || (!num_disks) || (!pDisk_start_offsets))
	return MZ_FALSE;
  if (pDisk_start_offsets[0] != 0)
	return MZ_FALSE;
  for (i = 0; i < num_disks; ++i)
  {
	if (pDisk_start_offsets[i] >= archive_size)
	  return MZ_FALSE;
	if ((i) && (pDisk_start_offsets[i] <= pDisk_start_offsets[i - 1]))
	  return MZ_FALSE;
  }
  if (!mz_zip_array_resize(pZip, &pState->m_disk_offsets, num_disks, MZ_FALSE))
	return MZ_FALSE;
  memcpy(pState->m_disk_offsets.m_p, pDisk_start_offsets, (size_t)num_disks * sizeof(mz_uint64));
  pState->m_num_disks = num_disks;
  return MZ_TRUE;
}

static mz_bool mz_zip_reader_get_disk_base(mz_zip_archive *pZip, mz_uint32 disk_index, mz_uint64 *pBase_ofs)
{
  mz_zip_internal_state *pState = pZip->m_pState;
  if ((pState) && (pState->m_num_disks))
  {
	if (disk_index >= pState->m_num_disks)
	  return MZ_FALSE;
	*pBase_ofs = MZ_ZIP_ARRAY_ELEMENT(&pState->m_disk_offsets, mz_uint64, disk_index);
	return MZ_TRUE;
  }

  /* Preserve compatibility with old single-disk archives which used 1 instead of 0. */
  if (disk_index <= 1)
  {
	*pBase_ofs = 0;
	return MZ_TRUE;
  }
  return MZ_FALSE;
}

static mz_bool mz_zip_reader_disk_to_archive_file_ofs(mz_zip_archive *pZip, mz_uint32 disk_index, mz_uint64 disk_ofs, mz_uint64 *pArchive_ofs)
{
  mz_uint64 disk_base;
  if (!mz_zip_reader_get_disk_base(pZip, disk_index, &disk_base))
	return MZ_FALSE;
  if ((disk_base > pZip->m_archive_size) || (disk_ofs > (pZip->m_archive_size - disk_base)))
	return MZ_FALSE;
  *pArchive_ofs = disk_base + disk_ofs;
  return MZ_TRUE;
}

static MZ_FORCEINLINE mz_bool mz_zip_reader_filename_less(const mz_zip_array *pCentral_dir_array, const mz_zip_array *pCentral_dir_offsets, mz_uint l_index, mz_uint r_index)
{
  const mz_uint8 *pL = &MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_array, mz_uint8, MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_offsets, size_t, l_index)), *pE;
  const mz_uint8 *pR = &MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_array, mz_uint8, MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_offsets, size_t, r_index));
  mz_uint l_len = MZ_READ_LE16(pL + MZ_ZIP_CDH_FILENAME_LEN_OFS), r_len = MZ_READ_LE16(pR + MZ_ZIP_CDH_FILENAME_LEN_OFS);
  mz_uint8 l = 0, r = 0;
  pL += MZ_ZIP_CENTRAL_DIR_HEADER_SIZE; pR += MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
  pE = pL + MZ_MIN(l_len, r_len);
  while (pL < pE)
  {
	if ((l = MZ_TOLOWER(*pL)) != (r = MZ_TOLOWER(*pR)))
	  break;
	pL++; pR++;
  }
  return (pL == pE) ? (l_len < r_len) : (l < r);
}

#define MZ_SWAP_UINT32(a, b) do { mz_uint32 t = a; a = b; b = t; } MZ_MACRO_END

// Heap sort of lowercased filenames, used to help accelerate plain central directory searches by mz_zip_reader_locate_file(). (Could also use qsort(), but it could allocate memory.)
static void mz_zip_reader_sort_central_dir_offsets_by_filename(mz_zip_archive *pZip)
{
  mz_zip_internal_state *pState = pZip->m_pState;
  const mz_zip_array *pCentral_dir_offsets = &pState->m_central_dir_offsets;
  const mz_zip_array *pCentral_dir = &pState->m_central_dir;
  mz_uint32 *pIndices = &MZ_ZIP_ARRAY_ELEMENT(&pState->m_sorted_central_dir_offsets, mz_uint32, 0);
  const int size = pZip->m_total_files;
  int start = (size - 2) >> 1, end;
  while (start >= 0)
  {
	int child, root = start;
	for ( ; ; )
	{
	  if ((child = (root << 1) + 1) >= size)
		break;
	  child += (((child + 1) < size) && (mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets, pIndices[child], pIndices[child + 1])));
	  if (!mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets, pIndices[root], pIndices[child]))
		break;
	  MZ_SWAP_UINT32(pIndices[root], pIndices[child]); root = child;
	}
	start--;
  }

  end = size - 1;
  while (end > 0)
  {
	int child, root = 0;
	  MZ_SWAP_UINT32(pIndices[end], pIndices[0]);
	for ( ; ; )
	{
	  if ((child = (root << 1) + 1) >= end)
		break;
	  child += (((child + 1) < end) && mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets, pIndices[child], pIndices[child + 1]));
	  if (!mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets, pIndices[root], pIndices[child]))
		break;
	  MZ_SWAP_UINT32(pIndices[root], pIndices[child]); root = child;
	}
	end--;
  }
}

static mz_bool mz_zip_reader_read_zip64_extra(const mz_uint8 *pExtra, mz_uint extra_size, mz_uint64 *pComp_size, mz_uint64 *pUncomp_size, mz_uint64 *pLocal_header_ofs, mz_uint32 *pDisk_index)
{
  mz_uint extra_remaining = extra_size;
  while (extra_remaining)
  {
	mz_uint field_id, field_size;
	const mz_uint8 *pField_data;
	if (extra_remaining < sizeof(mz_uint16) * 2)
	  return MZ_FALSE;
	field_id = MZ_READ_LE16(pExtra);
	field_size = MZ_READ_LE16(pExtra + sizeof(mz_uint16));
	if ((field_size + sizeof(mz_uint16) * 2) > extra_remaining)
	  return MZ_FALSE;
	pField_data = pExtra + sizeof(mz_uint16) * 2;
	if (field_id == MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID)
	{
	  mz_uint field_remaining = field_size;
	  if (pUncomp_size && (*pUncomp_size == MZ_UINT32_MAX))
	  {
		if (field_remaining < sizeof(mz_uint64))
		  return MZ_FALSE;
		*pUncomp_size = MZ_READ_LE64(pField_data);
		pField_data += sizeof(mz_uint64);
		field_remaining -= sizeof(mz_uint64);
	  }
	  if (pComp_size && (*pComp_size == MZ_UINT32_MAX))
	  {
		if (field_remaining < sizeof(mz_uint64))
		  return MZ_FALSE;
		*pComp_size = MZ_READ_LE64(pField_data);
		pField_data += sizeof(mz_uint64);
		field_remaining -= sizeof(mz_uint64);
	  }
	  if (pLocal_header_ofs && (*pLocal_header_ofs == MZ_UINT32_MAX))
	  {
		if (field_remaining < sizeof(mz_uint64))
		  return MZ_FALSE;
		*pLocal_header_ofs = MZ_READ_LE64(pField_data);
		pField_data += sizeof(mz_uint64);
		field_remaining -= sizeof(mz_uint64);
	  }
	  if (pDisk_index && (*pDisk_index == MZ_UINT16_MAX))
	  {
		if (field_remaining < sizeof(mz_uint32))
		  return MZ_FALSE;
		*pDisk_index = MZ_READ_LE32(pField_data);
	  }
	  return MZ_TRUE;
	}
	pExtra += sizeof(mz_uint16) * 2 + field_size;
	extra_remaining -= sizeof(mz_uint16) * 2 + field_size;
  }

  return ((pComp_size == NULL) || (*pComp_size != MZ_UINT32_MAX)) &&
		 ((pUncomp_size == NULL) || (*pUncomp_size != MZ_UINT32_MAX)) &&
		 ((pLocal_header_ofs == NULL) || (*pLocal_header_ofs != MZ_UINT32_MAX)) &&
		 ((pDisk_index == NULL) || (*pDisk_index != MZ_UINT16_MAX));
}

static mz_bool mz_zip_reader_read_central_dir(mz_zip_archive *pZip, mz_uint32 flags)
{
  mz_zip_internal_state *pState = pZip->m_pState;
  size_t cdir_size, cdir_stored_size;
  mz_uint32 num_this_disk, cdir_disk_index, total_disks = 1;
  mz_uint64 cdir_entries_on_this_disk, total_files, cdir_ofs_on_disk, cdir_ofs, eocd_ofs, last_disk_base = 0, cur_file_ofs, max_scan_back;
  const mz_uint8 *p;
  mz_uint32 buf_u32[4096 / sizeof(mz_uint32)]; mz_uint8 *pBuf = (mz_uint8 *)buf_u32;
  mz_bool sort_central_dir = ((flags & MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY) == 0);
  mz_bool compressed_cdir = MZ_FALSE;
  mz_uint16 cdir_compression_method = 0;
  // Basic sanity checks - reject files which are too small, and check the first 4 bytes of the file to make sure a local header is there.
  if (pZip->m_archive_size < MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE)
	return MZ_FALSE;

  if (pState->m_num_disks)
	last_disk_base = MZ_ZIP_ARRAY_ELEMENT(&pState->m_disk_offsets, mz_uint64, pState->m_num_disks - 1);
  if ((last_disk_base >= pZip->m_archive_size) || ((pZip->m_archive_size - last_disk_base) < MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE))
	return MZ_FALSE;

  // Find the end of central directory record by scanning the file from the end towards the beginning.
  max_scan_back = MZ_MIN((mz_uint64)0xFFFF + MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE, pZip->m_archive_size - last_disk_base);
  cur_file_ofs = pZip->m_archive_size - MZ_MIN((mz_uint64)sizeof(buf_u32), pZip->m_archive_size - last_disk_base);
  for ( ; ; )
  {
	int i, n = (int)MZ_MIN(sizeof(buf_u32), pZip->m_archive_size - cur_file_ofs);
	if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pBuf, (size_t)n) != (size_t)n)
	  return MZ_FALSE;
	for (i = n - 4; i >= 0; --i)
	  if (MZ_READ_LE32(pBuf + i) == MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG)
		break;
	if (i >= 0)
	{
	  cur_file_ofs += i;
	  break;
	}
	if ((cur_file_ofs == last_disk_base) || ((pZip->m_archive_size - cur_file_ofs) >= max_scan_back))
	  return MZ_FALSE;
	cur_file_ofs = ((cur_file_ofs - last_disk_base) > (sizeof(buf_u32) - 3)) ? (cur_file_ofs - (sizeof(buf_u32) - 3)) : last_disk_base;
  }
  eocd_ofs = cur_file_ofs;
  // Read and verify the end of central directory record.
  if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pBuf, MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE) != MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE)
	return MZ_FALSE;
  if (MZ_READ_LE32(pBuf + MZ_ZIP_ECDH_SIG_OFS) != MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG)
	return MZ_FALSE;
  if ((eocd_ofs + MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE + MZ_READ_LE16(pBuf + MZ_ZIP_ECDH_COMMENT_SIZE_OFS)) != pZip->m_archive_size)
	return MZ_FALSE;

  num_this_disk = MZ_READ_LE16(pBuf + MZ_ZIP_ECDH_NUM_THIS_DISK_OFS);
  cdir_disk_index = MZ_READ_LE16(pBuf + MZ_ZIP_ECDH_NUM_DISK_CDIR_OFS);
  cdir_entries_on_this_disk = MZ_READ_LE16(pBuf + MZ_ZIP_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS);
  total_files = MZ_READ_LE16(pBuf + MZ_ZIP_ECDH_CDIR_TOTAL_ENTRIES_OFS);
  cdir_size = MZ_READ_LE32(pBuf + MZ_ZIP_ECDH_CDIR_SIZE_OFS);
  cdir_stored_size = cdir_size;
  cdir_ofs_on_disk = MZ_READ_LE32(pBuf + MZ_ZIP_ECDH_CDIR_OFS_OFS);

  if (eocd_ofs >= MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE)
  {
	mz_uint8 zip64_locator[MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE];
	if (pZip->m_pRead(pZip->m_pIO_opaque, eocd_ofs - MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE, zip64_locator, sizeof(zip64_locator)) == sizeof(zip64_locator))
	{
	  if (MZ_READ_LE32(zip64_locator + MZ_ZIP64_ECDL_SIG_OFS) == MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIG)
	  {
		mz_uint8 zip64_eocd[MZ_ZIP64_END_OF_CENTRAL_DIR_V2_HEADER_SIZE];
		mz_uint64 zip64_eocd_ofs_on_disk = MZ_READ_LE64(zip64_locator + MZ_ZIP64_ECDL_REL_OFS_TO_ZIP64_ECDR_OFS);
		mz_uint64 zip64_eocd_ofs;
		mz_uint64 zip64_total_files, zip64_total_files_on_disk, zip64_cdir_size, zip64_record_size, zip64_locator_ofs = eocd_ofs - MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE;
		mz_uint32 zip64_eocd_disk_index = MZ_READ_LE32(zip64_locator + MZ_ZIP64_ECDL_NUM_DISK_CDIR_OFS);
		mz_uint32 zip64_total_disks = MZ_READ_LE32(zip64_locator + MZ_ZIP64_ECDL_TOTAL_NUMBER_OF_DISKS_OFS);

		if ((!zip64_total_disks) || ((pState->m_num_disks) && (zip64_total_disks != pState->m_num_disks)) || ((!pState->m_num_disks) && (zip64_total_disks > 1)))
		  return MZ_FALSE;
		total_disks = zip64_total_disks;
		if ((!mz_zip_reader_disk_to_archive_file_ofs(pZip, zip64_eocd_disk_index, zip64_eocd_ofs_on_disk, &zip64_eocd_ofs)) || (zip64_eocd_ofs > zip64_locator_ofs) ||
			(pZip->m_pRead(pZip->m_pIO_opaque, zip64_eocd_ofs, zip64_eocd, MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE) != MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE) ||
			(MZ_READ_LE32(zip64_eocd + MZ_ZIP64_ECDH_SIG_OFS) != MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIG))
		  return MZ_FALSE;
		zip64_record_size = MZ_READ_LE64(zip64_eocd + MZ_ZIP64_ECDH_SIZE_OF_RECORD_OFS);
		if (((zip64_locator_ofs - zip64_eocd_ofs) < 12) || (zip64_record_size < (MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE - 12)) || (zip64_record_size > (zip64_locator_ofs - zip64_eocd_ofs - 12)))
		  return MZ_FALSE;

		num_this_disk = MZ_READ_LE32(zip64_eocd + MZ_ZIP64_ECDH_NUM_THIS_DISK_OFS);
		cdir_disk_index = MZ_READ_LE32(zip64_eocd + MZ_ZIP64_ECDH_NUM_DISK_CDIR_OFS);
		zip64_total_files_on_disk = MZ_READ_LE64(zip64_eocd + MZ_ZIP64_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS);
		zip64_total_files = MZ_READ_LE64(zip64_eocd + MZ_ZIP64_ECDH_CDIR_TOTAL_ENTRIES_OFS);
		zip64_cdir_size = MZ_READ_LE64(zip64_eocd + MZ_ZIP64_ECDH_CDIR_SIZE_OFS);
		cdir_ofs_on_disk = MZ_READ_LE64(zip64_eocd + MZ_ZIP64_ECDH_CDIR_OFS_OFS);
		if ((zip64_total_files_on_disk > zip64_total_files) || (zip64_total_files > (mz_uint64)((mz_uint)-1)) || (zip64_cdir_size > (mz_uint64)((size_t)-1)))
		  return MZ_FALSE;
		total_files = zip64_total_files;
		cdir_entries_on_this_disk = zip64_total_files_on_disk;
		cdir_size = (size_t)zip64_cdir_size;
		cdir_stored_size = cdir_size;

		if (zip64_record_size >= (MZ_ZIP64_END_OF_CENTRAL_DIR_V2_HEADER_SIZE - 12))
		{
		  mz_uint64 compressed_size, original_size;
		  mz_uint16 alg_id, hash_size;
		  if (pZip->m_pRead(pZip->m_pIO_opaque, zip64_eocd_ofs, zip64_eocd, MZ_ZIP64_END_OF_CENTRAL_DIR_V2_HEADER_SIZE) != MZ_ZIP64_END_OF_CENTRAL_DIR_V2_HEADER_SIZE)
			return MZ_FALSE;
		  cdir_compression_method = MZ_READ_LE16(zip64_eocd + MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE);
		  compressed_size = MZ_READ_LE64(zip64_eocd + MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE + 2);
		  original_size = MZ_READ_LE64(zip64_eocd + MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE + 10);
		  alg_id = MZ_READ_LE16(zip64_eocd + MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE + 18);
		  hash_size = MZ_READ_LE16(zip64_eocd + MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE + 26);
		  if ((alg_id != 0) || ((cdir_compression_method != 0) && (cdir_compression_method != MZ_DEFLATED)
#ifdef MINIZ_USE_LZMA_SDK
			  && (cdir_compression_method != MZ_LZMA)
#endif
			 ) ||
			  (compressed_size > (mz_uint64)((size_t)-1)) || (original_size > (mz_uint64)((size_t)-1)) ||
			  ((!cdir_compression_method) && (compressed_size != original_size)) ||
			  ((mz_uint64)hash_size > zip64_record_size - (MZ_ZIP64_END_OF_CENTRAL_DIR_V2_HEADER_SIZE - 12)))
			return MZ_FALSE;
		  compressed_cdir = (cdir_compression_method != 0);
		  cdir_stored_size = (size_t)compressed_size;
		  cdir_size = (size_t)original_size;
		}
	  }
	}
  }

  if ((pState->m_num_disks) && (total_disks == 1) && (num_this_disk != MZ_UINT16_MAX))
	total_disks = num_this_disk + 1;

  if (pState->m_num_disks)
  {
	if ((!total_disks) || (total_disks != pState->m_num_disks) || (num_this_disk >= total_disks) || (cdir_disk_index >= total_disks) || (num_this_disk != total_disks - 1))
	  return MZ_FALSE;
  }
  else
  {
	if (((num_this_disk | cdir_disk_index) != 0) && ((num_this_disk != 1) || (cdir_disk_index != 1)))
	  return MZ_FALSE;
	num_this_disk = cdir_disk_index = 0;
	total_disks = 1;
  }

  if ((total_files > (mz_uint64)((mz_uint)-1)) || (cdir_entries_on_this_disk > total_files) || ((!compressed_cdir) && (total_disks == 1) && (cdir_entries_on_this_disk != total_files)))
	return MZ_FALSE;
  pZip->m_total_files = (mz_uint)total_files;

  if (pZip->m_total_files > ((mz_uint)-1 / MZ_ZIP_CENTRAL_DIR_HEADER_SIZE))
	return MZ_FALSE;
  if (cdir_size < (size_t)pZip->m_total_files * MZ_ZIP_CENTRAL_DIR_HEADER_SIZE)
	return MZ_FALSE;
  if (!mz_zip_reader_disk_to_archive_file_ofs(pZip, cdir_disk_index, cdir_ofs_on_disk, &cdir_ofs))
	return MZ_FALSE;
  if ((cdir_ofs > pZip->m_archive_size) || ((mz_uint64)cdir_stored_size > (pZip->m_archive_size - cdir_ofs)))
	return MZ_FALSE;

  pZip->m_central_directory_file_ofs = cdir_ofs;

  if (pZip->m_total_files)
  {
	mz_uint i;
	size_t n;

	// Read the entire central directory into a heap block, and allocate another heap block to hold the unsorted central dir file record offsets, and another to hold the sorted indices.
	if ((!mz_zip_array_resize(pZip, &pZip->m_pState->m_central_dir, cdir_size, MZ_FALSE)) ||
		(!mz_zip_array_resize(pZip, &pZip->m_pState->m_central_dir_offsets, pZip->m_total_files, MZ_FALSE)))
	  return MZ_FALSE;

	if (sort_central_dir)
	{
	  if (!mz_zip_array_resize(pZip, &pZip->m_pState->m_sorted_central_dir_offsets, pZip->m_total_files, MZ_FALSE))
		return MZ_FALSE;
	}

	if (compressed_cdir)
	{
	  mz_zip_array compressed_data;
	  MZ_CLEAR_OBJ(compressed_data);
	  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&compressed_data, sizeof(mz_uint8));
	  if ((!mz_zip_array_resize(pZip, &compressed_data, cdir_stored_size, MZ_FALSE)) ||
		  (pZip->m_pRead(pZip->m_pIO_opaque, cdir_ofs, compressed_data.m_p, cdir_stored_size) != cdir_stored_size))
	  {
		mz_zip_array_clear(pZip, &compressed_data);
		return MZ_FALSE;
	  }
	  if (cdir_compression_method == MZ_DEFLATED)
	  {
		if (tinfl_decompress_mem_to_mem(pZip->m_pState->m_central_dir.m_p, cdir_size, compressed_data.m_p, cdir_stored_size, 0) != cdir_size)
		{
		  mz_zip_array_clear(pZip, &compressed_data);
		  return MZ_FALSE;
		}
	  }
#ifdef MINIZ_USE_LZMA_SDK
	  else if (!mz_zip_lzma_decompress(pZip->m_pState->m_central_dir.m_p, cdir_size, compressed_data.m_p, cdir_stored_size))
	  {
		mz_zip_array_clear(pZip, &compressed_data);
		return MZ_FALSE;
	  }
#endif
	  mz_zip_array_clear(pZip, &compressed_data);
	}
	else if (pZip->m_pRead(pZip->m_pIO_opaque, cdir_ofs, pZip->m_pState->m_central_dir.m_p, cdir_size) != cdir_size)
	  return MZ_FALSE;

	// Now create an index into the central directory file records and do some basic sanity checking on each record.
	p = (const mz_uint8 *)pZip->m_pState->m_central_dir.m_p;
	for (n = cdir_size, i = 0; i < pZip->m_total_files; ++i)
	{
	  mz_uint total_header_size, filename_size, extra_size, comment_size, method;
	  mz_uint32 disk_index;
	  mz_uint64 comp_size, decomp_size, local_header_ofs, local_header_abs_ofs;
	  if ((n < MZ_ZIP_CENTRAL_DIR_HEADER_SIZE) || (MZ_READ_LE32(p) != MZ_ZIP_CENTRAL_DIR_HEADER_SIG))
		return MZ_FALSE;
	  MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, size_t, i) = (size_t)(p - (const mz_uint8 *)pZip->m_pState->m_central_dir.m_p);
	  if (sort_central_dir)
		MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_sorted_central_dir_offsets, mz_uint32, i) = i;
	  comp_size = MZ_READ_LE32(p + MZ_ZIP_CDH_COMPRESSED_SIZE_OFS);
	  decomp_size = MZ_READ_LE32(p + MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS);
	  local_header_ofs = MZ_READ_LE32(p + MZ_ZIP_CDH_LOCAL_HEADER_OFS);
	  disk_index = MZ_READ_LE16(p + MZ_ZIP_CDH_DISK_START_OFS);
	  filename_size = MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS);
	  extra_size = MZ_READ_LE16(p + MZ_ZIP_CDH_EXTRA_LEN_OFS);
	  comment_size = MZ_READ_LE16(p + MZ_ZIP_CDH_COMMENT_LEN_OFS);
	  if ((total_header_size = MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_size + extra_size + comment_size) > n)
		return MZ_FALSE;
	  if ((comp_size == MZ_UINT32_MAX) || (decomp_size == MZ_UINT32_MAX) || (local_header_ofs == MZ_UINT32_MAX) || (disk_index == MZ_UINT16_MAX))
	  {
		if (!mz_zip_reader_read_zip64_extra(p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_size, extra_size, &comp_size, &decomp_size, &local_header_ofs, &disk_index))
		  return MZ_FALSE;
	  }
	  method = MZ_READ_LE16(p + MZ_ZIP_CDH_METHOD_OFS);
	  if (((!method) && (decomp_size != comp_size)) || (decomp_size && !comp_size))
		return MZ_FALSE;
	  if (!mz_zip_reader_disk_to_archive_file_ofs(pZip, disk_index, local_header_ofs, &local_header_abs_ofs))
		return MZ_FALSE;
	  if ((local_header_abs_ofs > pZip->m_archive_size) || (pZip->m_archive_size - local_header_abs_ofs < MZ_ZIP_LOCAL_DIR_HEADER_SIZE) ||
		  (comp_size > (pZip->m_archive_size - local_header_abs_ofs - MZ_ZIP_LOCAL_DIR_HEADER_SIZE)))
		return MZ_FALSE;
	  n -= total_header_size; p += total_header_size;
	}
  }

  if (sort_central_dir)
	mz_zip_reader_sort_central_dir_offsets_by_filename(pZip);

  return MZ_TRUE;
}

mz_bool mz_zip_reader_init(mz_zip_archive *pZip, mz_uint64 size, mz_uint32 flags)
{
  if ((!pZip) || (!pZip->m_pRead))
	return MZ_FALSE;
  if (!mz_zip_reader_init_internal(pZip, flags))
	return MZ_FALSE;
  pZip->m_archive_size = size;
  if (!mz_zip_reader_read_central_dir(pZip, flags))
  {
	mz_zip_reader_end(pZip);
	return MZ_FALSE;
  }
  return MZ_TRUE;
}

mz_bool mz_zip_reader_init_multidisk(mz_zip_archive *pZip, mz_uint64 size, mz_uint32 num_disks, const mz_uint64 *pDisk_start_offsets, mz_uint32 flags)
{
  if ((!pZip) || (!pZip->m_pRead))
	return MZ_FALSE;
  if (!mz_zip_reader_init_internal(pZip, flags))
	return MZ_FALSE;
  pZip->m_archive_size = size;
  if ((!mz_zip_reader_set_multidisk_offsets(pZip, size, num_disks, pDisk_start_offsets)) || (!mz_zip_reader_read_central_dir(pZip, flags)))
  {
	mz_zip_reader_end(pZip);
	return MZ_FALSE;
  }
  return MZ_TRUE;
}

static size_t mz_zip_mem_read_func(void *pOpaque, mz_uint64 file_ofs, void *pBuf, size_t n)
{
  mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;
  size_t s = (file_ofs >= pZip->m_archive_size) ? 0 : (size_t)MZ_MIN(pZip->m_archive_size - file_ofs, n);
  memcpy(pBuf, (const mz_uint8 *)pZip->m_pState->m_pMem + file_ofs, s);
  return s;
}

mz_bool mz_zip_reader_init_mem(mz_zip_archive *pZip, const void *pMem, size_t size, mz_uint32 flags)
{
  if (!mz_zip_reader_init_internal(pZip, flags))
	return MZ_FALSE;
  pZip->m_archive_size = size;
  pZip->m_pRead = mz_zip_mem_read_func;
  pZip->m_pIO_opaque = pZip;
#ifdef __cplusplus
  pZip->m_pState->m_pMem = const_cast<void *>(pMem);
#else
  pZip->m_pState->m_pMem = (void *)pMem;
#endif
  pZip->m_pState->m_mem_size = size;
  if (!mz_zip_reader_read_central_dir(pZip, flags))
  {
	mz_zip_reader_end(pZip);
	return MZ_FALSE;
  }
  return MZ_TRUE;
}

mz_bool mz_zip_reader_init_mem_multidisk(mz_zip_archive *pZip, const void *pMem, size_t size, mz_uint32 num_disks, const mz_uint64 *pDisk_start_offsets, mz_uint32 flags)
{
  if (!mz_zip_reader_init_internal(pZip, flags))
	return MZ_FALSE;
  pZip->m_archive_size = size;
  pZip->m_pRead = mz_zip_mem_read_func;
  pZip->m_pIO_opaque = pZip;
#ifdef __cplusplus
  pZip->m_pState->m_pMem = const_cast<void *>(pMem);
#else
  pZip->m_pState->m_pMem = (void *)pMem;
#endif
  pZip->m_pState->m_mem_size = size;
  if ((!mz_zip_reader_set_multidisk_offsets(pZip, size, num_disks, pDisk_start_offsets)) || (!mz_zip_reader_read_central_dir(pZip, flags)))
  {
	mz_zip_reader_end(pZip);
	return MZ_FALSE;
  }
  return MZ_TRUE;
}

#ifndef MINIZ_NO_STDIO
static size_t mz_zip_file_read_func(void *pOpaque, mz_uint64 file_ofs, void *pBuf, size_t n)
{
  mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;
  mz_zip_internal_state *pState = pZip->m_pState;
  if (pState->m_files.m_size)
  {
	size_t total_read = 0, disk_index = 0;
	if ((file_ofs >= pZip->m_archive_size) || (pState->m_files.m_size != pState->m_disk_offsets.m_size))
	  return 0;
	while ((disk_index + 1 < pState->m_disk_offsets.m_size) &&
		   (file_ofs >= MZ_ZIP_ARRAY_ELEMENT(&pState->m_disk_offsets, mz_uint64, disk_index + 1)))
	  ++disk_index;
	while (n && (disk_index < pState->m_files.m_size))
	{
	  MZ_FILE *pFile = MZ_ZIP_ARRAY_ELEMENT(&pState->m_files, MZ_FILE *, disk_index);
	  mz_uint64 disk_base = MZ_ZIP_ARRAY_ELEMENT(&pState->m_disk_offsets, mz_uint64, disk_index);
	  mz_uint64 disk_end = (disk_index + 1 < pState->m_disk_offsets.m_size) ?
		MZ_ZIP_ARRAY_ELEMENT(&pState->m_disk_offsets, mz_uint64, disk_index + 1) : pZip->m_archive_size;
	  mz_uint64 disk_file_ofs = file_ofs - disk_base;
	  size_t to_read = (size_t)MZ_MIN((mz_uint64)n, disk_end - file_ofs);
	  size_t bytes_read;
	  mz_int64 cur_ofs = MZ_FTELL64(pFile);
	  if (((mz_int64)disk_file_ofs < 0) ||
		  ((cur_ofs != (mz_int64)disk_file_ofs) && MZ_FSEEK64(pFile, (mz_int64)disk_file_ofs, SEEK_SET)))
		break;
	  bytes_read = MZ_FREAD((mz_uint8 *)pBuf + total_read, 1, to_read, pFile);
	  total_read += bytes_read;
	  file_ofs += bytes_read;
	  n -= bytes_read;
	  if (bytes_read != to_read)
		break;
	  ++disk_index;
	}
	return total_read;
  }
  else
  {
	mz_int64 cur_ofs = MZ_FTELL64(pState->m_pFile);
	if (((mz_int64)file_ofs < 0) || ((cur_ofs != (mz_int64)file_ofs) && MZ_FSEEK64(pState->m_pFile, (mz_int64)file_ofs, SEEK_SET)))
	  return 0;
	return MZ_FREAD(pBuf, 1, n, pState->m_pFile);
  }
}

static mz_bool mz_zip_file_get_size(MZ_FILE *pFile, mz_uint64 *pSize)
{
  mz_int64 size;
  if (MZ_FSEEK64(pFile, 0, SEEK_END))
	return MZ_FALSE;
  size = MZ_FTELL64(pFile);
  if (size < 0)
	return MZ_FALSE;
  *pSize = (mz_uint64)size;
  return MZ_TRUE;
}

static mz_bool mz_zip_split_make_filename(char *pDst, size_t dst_size, const char *pZip_filename, mz_uint32 disk_index)
{
  size_t filename_size = strlen(pZip_filename), stem_size, num_digits = 0, min_digits, i;
  char digits[10];
  if ((filename_size < 4) ||
	  (pZip_filename[filename_size - 4] != '.') ||
	  (MZ_TOLOWER(pZip_filename[filename_size - 3]) != 'z') ||
	  (MZ_TOLOWER(pZip_filename[filename_size - 2]) != 'i') ||
	  (MZ_TOLOWER(pZip_filename[filename_size - 1]) != 'p'))
	return MZ_FALSE;
  do
  {
	digits[num_digits++] = (char)('0' + (disk_index % 10));
	disk_index /= 10;
  } while (disk_index);
  min_digits = MZ_MAX((size_t)2, num_digits);
  stem_size = filename_size - 4;
  if (stem_size + 2 + min_digits + 1 > dst_size)
	return MZ_FALSE;
  memcpy(pDst, pZip_filename, stem_size);
  pDst[stem_size] = '.';
  pDst[stem_size + 1] = 'z';
  for (i = num_digits; i < min_digits; ++i)
	pDst[stem_size + 2 + min_digits - i - 1] = '0';
  for (i = 0; i < num_digits; ++i)
	pDst[stem_size + 2 + min_digits - i - 1] = digits[i];
  pDst[stem_size + 2 + min_digits] = '\0';
  return MZ_TRUE;
}

mz_bool mz_zip_reader_init_file(mz_zip_archive *pZip, const char *pFilename, mz_uint32 flags)
{
  mz_uint64 file_size, archive_size = 0;
  mz_uint32 disk_index = 1;
  char *pSegment_filename = NULL;
  MZ_FILE *pSegment_file = NULL;
  MZ_FILE *pFile;
  if ((!pZip) || (!pFilename))
	return MZ_FALSE;
  pFile = MZ_FOPEN(pFilename, "rb");
  if (!pFile)
	return MZ_FALSE;
  if (!mz_zip_file_get_size(pFile, &file_size))
  {
	MZ_FCLOSE(pFile);
	return MZ_FALSE;
  }
  if (!mz_zip_reader_init_internal(pZip, flags))
  {
	MZ_FCLOSE(pFile);
	return MZ_FALSE;
  }
  pZip->m_pRead = mz_zip_file_read_func;
  pZip->m_pIO_opaque = pZip;

  if (strlen(pFilename) <= (size_t)-17)
  {
	size_t segment_filename_capacity = strlen(pFilename) + 17;
	pSegment_filename = (char *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, segment_filename_capacity);
	if (pSegment_filename && mz_zip_split_make_filename(pSegment_filename, segment_filename_capacity, pFilename, disk_index))
	  pSegment_file = MZ_FOPEN(pSegment_filename, "rb");
  }

  if (pSegment_file)
  {
	for (;;)
	{
	  mz_uint64 segment_size, disk_base = archive_size;
	  size_t orig_disk_count = pZip->m_pState->m_disk_offsets.m_size;
	  if ((!mz_zip_file_get_size(pSegment_file, &segment_size)) || (!segment_size) ||
		  (segment_size > ((mz_uint64)-1 - archive_size)) ||
		  (!mz_zip_array_push_back(pZip, &pZip->m_pState->m_disk_offsets, &disk_base, 1)) ||
		  (!mz_zip_array_push_back(pZip, &pZip->m_pState->m_files, &pSegment_file, 1)))
	  {
		mz_zip_array_resize(pZip, &pZip->m_pState->m_disk_offsets, orig_disk_count, MZ_FALSE);
		MZ_FCLOSE(pSegment_file);
		MZ_FCLOSE(pFile);
		if (pSegment_filename) pZip->m_pFree(pZip->m_pAlloc_opaque, pSegment_filename);
		mz_zip_reader_end(pZip);
		return MZ_FALSE;
	  }
	  archive_size += segment_size;
	  pSegment_file = NULL;
	  if (disk_index == MZ_UINT32_MAX)
	  {
		MZ_FCLOSE(pFile);
		if (pSegment_filename) pZip->m_pFree(pZip->m_pAlloc_opaque, pSegment_filename);
		mz_zip_reader_end(pZip);
		return MZ_FALSE;
	  }
	  ++disk_index;
	  if (!mz_zip_split_make_filename(pSegment_filename, strlen(pFilename) + 17, pFilename, disk_index))
		break;
	  pSegment_file = MZ_FOPEN(pSegment_filename, "rb");
	  if (!pSegment_file)
		break;
	}

	{
	  mz_uint64 disk_base = archive_size;
	  size_t orig_disk_count = pZip->m_pState->m_disk_offsets.m_size;
	  if ((file_size > ((mz_uint64)-1 - archive_size)) ||
		  (!mz_zip_array_push_back(pZip, &pZip->m_pState->m_disk_offsets, &disk_base, 1)) ||
		  (!mz_zip_array_push_back(pZip, &pZip->m_pState->m_files, &pFile, 1)))
	  {
		mz_zip_array_resize(pZip, &pZip->m_pState->m_disk_offsets, orig_disk_count, MZ_FALSE);
		MZ_FCLOSE(pFile);
		if (pSegment_filename) pZip->m_pFree(pZip->m_pAlloc_opaque, pSegment_filename);
		mz_zip_reader_end(pZip);
		return MZ_FALSE;
	  }
	  archive_size += file_size;
	}
	pZip->m_pState->m_num_disks = (mz_uint32)pZip->m_pState->m_files.m_size;
	pZip->m_archive_size = archive_size;
  }
  else
  {
	pZip->m_pState->m_pFile = pFile;
	pZip->m_archive_size = file_size;
  }

  if (pSegment_filename)
	pZip->m_pFree(pZip->m_pAlloc_opaque, pSegment_filename);
  if (!mz_zip_reader_read_central_dir(pZip, flags))
  {
	mz_zip_reader_end(pZip);
	return MZ_FALSE;
  }
  return MZ_TRUE;
}
#endif // #ifndef MINIZ_NO_STDIO

mz_uint mz_zip_reader_get_num_files(mz_zip_archive *pZip)
{
  return pZip ? pZip->m_total_files : 0;
}

static MZ_FORCEINLINE const mz_uint8 *mz_zip_reader_get_cdh(mz_zip_archive *pZip, mz_uint file_index)
{
  if ((!pZip) || (!pZip->m_pState) || (file_index >= pZip->m_total_files) || (pZip->m_zip_mode != MZ_ZIP_MODE_READING))
	return NULL;
  return &MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir, mz_uint8, MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, size_t, file_index));
}

mz_bool mz_zip_reader_is_file_encrypted(mz_zip_archive *pZip, mz_uint file_index)
{
  mz_uint m_bit_flag;
  const mz_uint8 *p = mz_zip_reader_get_cdh(pZip, file_index);
  if (!p)
	return MZ_FALSE;
  m_bit_flag = MZ_READ_LE16(p + MZ_ZIP_CDH_BIT_FLAG_OFS);
  return (m_bit_flag & 1);
}

mz_bool mz_zip_reader_is_file_a_directory(mz_zip_archive *pZip, mz_uint file_index)
{
  mz_uint filename_len, external_attr;
  const mz_uint8 *p = mz_zip_reader_get_cdh(pZip, file_index);
  if (!p)
	return MZ_FALSE;

  // First see if the filename ends with a '/' character.
  filename_len = MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS);
  if (filename_len)
  {
	if (*(p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_len - 1) == '/')
	  return MZ_TRUE;
  }

  // Bugfix: This code was also checking if the internal attribute was non-zero, which wasn't correct.
  // Most/all zip writers (hopefully) set DOS file/directory attributes in the low 16-bits, so check for the DOS directory flag and ignore the source OS ID in the created by field.
  // FIXME: Remove this check? Is it necessary - we already check the filename.
  external_attr = MZ_READ_LE32(p + MZ_ZIP_CDH_EXTERNAL_ATTR_OFS);
  if ((external_attr & 0x10) != 0)
	return MZ_TRUE;

  return MZ_FALSE;
}

mz_bool mz_zip_reader_file_stat(mz_zip_archive *pZip, mz_uint file_index, mz_zip_archive_file_stat *pStat)
{
  mz_uint n;
  const mz_uint8 *p = mz_zip_reader_get_cdh(pZip, file_index);
  if ((!p) || (!pStat))
	return MZ_FALSE;

  // Unpack the central directory record.
  pStat->m_file_index = file_index;
  pStat->m_central_dir_ofs = (mz_uint64)MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, size_t, file_index);
  pStat->m_version_made_by = MZ_READ_LE16(p + MZ_ZIP_CDH_VERSION_MADE_BY_OFS);
  pStat->m_version_needed = MZ_READ_LE16(p + MZ_ZIP_CDH_VERSION_NEEDED_OFS);
  pStat->m_bit_flag = MZ_READ_LE16(p + MZ_ZIP_CDH_BIT_FLAG_OFS);
  pStat->m_method = MZ_READ_LE16(p + MZ_ZIP_CDH_METHOD_OFS);
#ifndef MINIZ_NO_TIME
  pStat->m_time = mz_zip_dos_to_time_t(MZ_READ_LE16(p + MZ_ZIP_CDH_FILE_TIME_OFS), MZ_READ_LE16(p + MZ_ZIP_CDH_FILE_DATE_OFS));
#endif
  pStat->m_crc32 = MZ_READ_LE32(p + MZ_ZIP_CDH_CRC32_OFS);
  pStat->m_comp_size = MZ_READ_LE32(p + MZ_ZIP_CDH_COMPRESSED_SIZE_OFS);
  pStat->m_uncomp_size = MZ_READ_LE32(p + MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS);
  pStat->m_internal_attr = MZ_READ_LE16(p + MZ_ZIP_CDH_INTERNAL_ATTR_OFS);
  pStat->m_external_attr = MZ_READ_LE32(p + MZ_ZIP_CDH_EXTERNAL_ATTR_OFS);
  pStat->m_local_header_ofs = MZ_READ_LE32(p + MZ_ZIP_CDH_LOCAL_HEADER_OFS);
  pStat->m_disk_index = MZ_READ_LE16(p + MZ_ZIP_CDH_DISK_START_OFS);
  pStat->m_is_encrypted = (mz_uint8)(pStat->m_bit_flag & 1);
  pStat->m_aes_strength = 0;
  pStat->m_aes_version = 0;
  {
	// Scan the central extra for a WinZip AES (0x9901) field. If present, m_method 99 is replaced by the real method.
	mz_uint filename_len = MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS);
	mz_uint extra_len = MZ_READ_LE16(p + MZ_ZIP_CDH_EXTRA_LEN_OFS);
	const mz_uint8 *pExtra = p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_len;
	mz_uint rem = extra_len;
	while (rem >= 4)
	{
	  mz_uint fid = MZ_READ_LE16(pExtra), fsz = MZ_READ_LE16(pExtra + 2);
	  if ((mz_uint)(fsz + 4) > rem)
		break;
	  if ((fid == MZ_ZIP_AES_EXTRA_FIELD_HEADER_ID) && (fsz >= MZ_ZIP_AES_EXTRA_DATA_SIZE))
	  {
		pStat->m_aes_version = (mz_uint8)MZ_READ_LE16(pExtra + 4);
		pStat->m_aes_strength = pExtra[8];
		pStat->m_method = MZ_READ_LE16(pExtra + 9); // real compression method
		break;
	  }
	  pExtra += 4 + fsz; rem -= 4 + fsz;
	}
  }
  if ((pStat->m_comp_size == MZ_UINT32_MAX) || (pStat->m_uncomp_size == MZ_UINT32_MAX) || (pStat->m_local_header_ofs == MZ_UINT32_MAX) || (pStat->m_disk_index == MZ_UINT16_MAX))
  {
	mz_uint filename_len = MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS);
	mz_uint extra_len = MZ_READ_LE16(p + MZ_ZIP_CDH_EXTRA_LEN_OFS);
	if (!mz_zip_reader_read_zip64_extra(p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_len, extra_len, &pStat->m_comp_size, &pStat->m_uncomp_size, &pStat->m_local_header_ofs, &pStat->m_disk_index))
	  return MZ_FALSE;
  }
  if (!mz_zip_reader_disk_to_archive_file_ofs(pZip, pStat->m_disk_index, pStat->m_local_header_ofs, &pStat->m_local_header_ofs))
	return MZ_FALSE;

  // Copy as much of the filename and comment as possible.
  n = MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS); n = MZ_MIN(n, MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE - 1);
  memcpy(pStat->m_filename, p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE, n); pStat->m_filename[n] = '\0';

  n = MZ_READ_LE16(p + MZ_ZIP_CDH_COMMENT_LEN_OFS); n = MZ_MIN(n, MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE - 1);
  pStat->m_comment_size = n;
  memcpy(pStat->m_comment, p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS) + MZ_READ_LE16(p + MZ_ZIP_CDH_EXTRA_LEN_OFS), n); pStat->m_comment[n] = '\0';

  return MZ_TRUE;
}

mz_uint mz_zip_reader_get_filename(mz_zip_archive *pZip, mz_uint file_index, char *pFilename, mz_uint filename_buf_size)
{
  mz_uint n;
  const mz_uint8 *p = mz_zip_reader_get_cdh(pZip, file_index);
  if (!p) { if (filename_buf_size) pFilename[0] = '\0'; return 0; }
  n = MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS);
  if (filename_buf_size)
  {
	n = MZ_MIN(n, filename_buf_size - 1);
	memcpy(pFilename, p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE, n);
	pFilename[n] = '\0';
  }
  return n + 1;
}

static MZ_FORCEINLINE mz_bool mz_zip_reader_string_equal(const char *pA, const char *pB, mz_uint len, mz_uint flags)
{
  mz_uint i;
  if (flags & MZ_ZIP_FLAG_CASE_SENSITIVE)
	return 0 == memcmp(pA, pB, len);
  for (i = 0; i < len; ++i)
	if (MZ_TOLOWER(pA[i]) != MZ_TOLOWER(pB[i]))
	  return MZ_FALSE;
  return MZ_TRUE;
}

static MZ_FORCEINLINE int mz_zip_reader_filename_compare(const mz_zip_array *pCentral_dir_array, const mz_zip_array *pCentral_dir_offsets, mz_uint l_index, const char *pR, mz_uint r_len)
{
  const mz_uint8 *pL = &MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_array, mz_uint8, MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_offsets, size_t, l_index)), *pE;
  mz_uint l_len = MZ_READ_LE16(pL + MZ_ZIP_CDH_FILENAME_LEN_OFS);
  mz_uint8 l = 0, r = 0;
  pL += MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
  pE = pL + MZ_MIN(l_len, r_len);
  while (pL < pE)
  {
	if ((l = MZ_TOLOWER(*pL)) != (r = MZ_TOLOWER(*pR)))
	  break;
	pL++; pR++;
  }
  return (pL == pE) ? (int)(l_len - r_len) : (l - r);
}

static int mz_zip_reader_locate_file_binary_search(mz_zip_archive *pZip, const char *pFilename)
{
  mz_zip_internal_state *pState = pZip->m_pState;
  const mz_zip_array *pCentral_dir_offsets = &pState->m_central_dir_offsets;
  const mz_zip_array *pCentral_dir = &pState->m_central_dir;
  mz_uint32 *pIndices = &MZ_ZIP_ARRAY_ELEMENT(&pState->m_sorted_central_dir_offsets, mz_uint32, 0);
  const int size = pZip->m_total_files;
  const mz_uint filename_len = (mz_uint)strlen(pFilename);
  int l = 0, h = size - 1;
  while (l <= h)
  {
	int m = (l + h) >> 1, file_index = pIndices[m], comp = mz_zip_reader_filename_compare(pCentral_dir, pCentral_dir_offsets, file_index, pFilename, filename_len);
	if (!comp)
	  return file_index;
	else if (comp < 0)
	  l = m + 1;
	else
	  h = m - 1;
  }
  return -1;
}

int mz_zip_reader_locate_file(mz_zip_archive *pZip, const char *pName, const char *pComment, mz_uint flags)
{
  mz_uint file_index; size_t name_len, comment_len;
  if ((!pZip) || (!pZip->m_pState) || (!pName) || (pZip->m_zip_mode != MZ_ZIP_MODE_READING))
	return -1;
  if (((flags & (MZ_ZIP_FLAG_IGNORE_PATH | MZ_ZIP_FLAG_CASE_SENSITIVE)) == 0) && (!pComment) && (pZip->m_pState->m_sorted_central_dir_offsets.m_size))
	return mz_zip_reader_locate_file_binary_search(pZip, pName);
  name_len = strlen(pName); if (name_len > 0xFFFF) return -1;
  comment_len = pComment ? strlen(pComment) : 0; if (comment_len > 0xFFFF) return -1;
  for (file_index = 0; file_index < pZip->m_total_files; file_index++)
  {
	const mz_uint8 *pHeader = &MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir, mz_uint8, MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, size_t, file_index));
	mz_uint filename_len = MZ_READ_LE16(pHeader + MZ_ZIP_CDH_FILENAME_LEN_OFS);
	const char *pFilename = (const char *)pHeader + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
	if (filename_len < name_len)
	  continue;
	if (comment_len)
	{
	  mz_uint file_extra_len = MZ_READ_LE16(pHeader + MZ_ZIP_CDH_EXTRA_LEN_OFS), file_comment_len = MZ_READ_LE16(pHeader + MZ_ZIP_CDH_COMMENT_LEN_OFS);
	  const char *pFile_comment = pFilename + filename_len + file_extra_len;
	  if ((file_comment_len != comment_len) || (!mz_zip_reader_string_equal(pComment, pFile_comment, file_comment_len, flags)))
		continue;
	}
	if ((flags & MZ_ZIP_FLAG_IGNORE_PATH) && (filename_len))
	{
	  int ofs = filename_len - 1;
	  do
	  {
		if ((pFilename[ofs] == '/') || (pFilename[ofs] == '\\') || (pFilename[ofs] == ':'))
		  break;
	  } while (--ofs >= 0);
	  ofs++;
	  pFilename += ofs; filename_len -= ofs;
	}
	if ((filename_len == name_len) && (mz_zip_reader_string_equal(pName, pFilename, filename_len, flags)))
	  return file_index;
  }
  return -1;
}

#ifdef MINIZ_USE_AES
// Runs AES-CTR (encrypt == decrypt) over data in place using the inlined software AES. WinZip's counter is a
// 16-byte little-endian value starting at 1, which mz_aes_ctr_xor_buf reproduces. No alignment constraints.
static mz_bool mz_zip_aes_ctr_xor(mz_zip_archive *pZip, const mz_uint8 *key, size_t key_len, mz_uint8 *data, size_t data_len)
{
  mz_aes_ctx ctx;
  (void)pZip;
  mz_aes_key_expand(&ctx, key, key_len);
  mz_aes_ctr_xor_buf(&ctx, data, data_len);
  memset(&ctx, 0, sizeof(ctx)); // scrub round keys
  return MZ_TRUE;
}

// Reads a WinZip AES entry's payload from data_ofs (length pStat->m_comp_size), verifies the password and HMAC,
// AES-CTR decrypts, then decompresses the plaintext with the real method into pOut (pStat->m_uncomp_size bytes).
static mz_bool mz_zip_reader_extract_aes_to_mem(mz_zip_archive *pZip, const mz_zip_archive_file_stat *pStat, mz_uint64 data_ofs, void *pOut, size_t out_size)
{
  int strength = pStat->m_aes_strength;
  size_t salt_len, key_len, ct_len;
  mz_uint64 comp_size = pStat->m_comp_size;
  const char *pPassword = pZip->m_pState->m_pPassword;
  mz_uint8 *pBlob = NULL, *pSalt, *pVerify, *pCipher, *pAuth;
  mz_uint8 enc_key[32], auth_key[32], verify[2], computed_auth[MZ_SHA1_DIGEST_SIZE];
  mz_hmac_sha1_ctx hm;
  mz_bool ok = MZ_FALSE;

  if ((strength < 1) || (strength > 3) || (!pPassword))
	return MZ_FALSE;
  salt_len = mz_aes_salt_len(strength);
  key_len = mz_aes_key_len(strength);
  if (comp_size < salt_len + MZ_AES_PW_VERIFY_LEN + MZ_AES_AUTH_CODE_LEN)
	return MZ_FALSE;
  if (comp_size > (mz_uint64)((size_t)-1))
	return MZ_FALSE;
  ct_len = (size_t)comp_size - salt_len - MZ_AES_PW_VERIFY_LEN - MZ_AES_AUTH_CODE_LEN;

  pBlob = (mz_uint8 *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)comp_size);
  if (!pBlob)
	return MZ_FALSE;
  if (pZip->m_pRead(pZip->m_pIO_opaque, data_ofs, pBlob, (size_t)comp_size) != comp_size)
	goto done;
  pSalt = pBlob;
  pVerify = pSalt + salt_len;
  pCipher = pVerify + MZ_AES_PW_VERIFY_LEN;
  pAuth = pCipher + ct_len;

  mz_aes_derive_keys((const mz_uint8 *)pPassword, strlen(pPassword), pSalt, strength, enc_key, auth_key, verify);
  if ((verify[0] != pVerify[0]) || (verify[1] != pVerify[1]))
	goto done; // wrong password

  // Authenticate the ciphertext before trusting it (encrypt-then-MAC).
  mz_hmac_sha1_init(&hm, auth_key, key_len);
  mz_hmac_sha1_update(&hm, pCipher, ct_len);
  mz_hmac_sha1_final(&hm, computed_auth);
  if (memcmp(computed_auth, pAuth, MZ_AES_AUTH_CODE_LEN) != 0)
	goto done; // data corrupted or tampered

  if (!mz_zip_aes_ctr_xor(pZip, enc_key, key_len, pCipher, ct_len))
	goto done;

  // Now pCipher holds the compressed plaintext; decompress with the real method.
  if (pStat->m_method == 0)
  {
	if (ct_len != out_size)
	  goto done;
	memcpy(pOut, pCipher, out_size);
	ok = MZ_TRUE;
  }
  else if (pStat->m_method == MZ_DEFLATED)
  {
	size_t got = tinfl_decompress_mem_to_mem(pOut, out_size, pCipher, ct_len, TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
	ok = (got == out_size);
  }
#ifdef MINIZ_USE_LZMA_SDK
  else if (pStat->m_method == MZ_LZMA)
	ok = mz_zip_lzma_decompress(pOut, out_size, pCipher, ct_len);
#endif
  // AE-1 stores the real CRC; verify it. AE-2 stores 0 and relies on the HMAC (already checked).
  if (ok && (pStat->m_aes_version == 1))
	ok = (mz_crc32(MZ_CRC32_INIT, (const mz_uint8 *)pOut, out_size) == pStat->m_crc32);

done:
  if (pBlob)
  {
	memset(pBlob, 0, (size_t)comp_size); // scrub decrypted plaintext from the temp buffer
	pZip->m_pFree(pZip->m_pAlloc_opaque, pBlob);
  }
  return ok;
}

// Reads a legacy PKWARE ZipCrypto entry (read-only): 12-byte encryption header + ciphertext at data_ofs (total
// pStat->m_comp_size), decrypts, checks the verification byte, then decompresses with pStat->m_method into pOut.
static mz_bool mz_zip_reader_extract_zipcrypto_to_mem(mz_zip_archive *pZip, const mz_zip_archive_file_stat *pStat, mz_uint64 data_ofs, mz_uint16 local_bit_flag, void *pOut, size_t out_size)
{
  const char *pPassword = pZip->m_pState->m_pPassword;
  mz_uint64 comp_size = pStat->m_comp_size;
  mz_uint8 *pBlob, *pCipher, check;
  size_t ct_len;
  mz_zipcrypto_keys keys;
  mz_bool ok = MZ_FALSE;

  if ((!pPassword) || (comp_size < 12) || (comp_size > (mz_uint64)((size_t)-1)))
	return MZ_FALSE;
  ct_len = (size_t)comp_size - 12;

  pBlob = (mz_uint8 *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)comp_size);
  if (!pBlob)
	return MZ_FALSE;
  if (pZip->m_pRead(pZip->m_pIO_opaque, data_ofs, pBlob, (size_t)comp_size) != comp_size)
	goto done;

  mz_zipcrypto_init(&keys, (const mz_uint8 *)pPassword, strlen(pPassword));
  mz_zipcrypto_decrypt(&keys, pBlob, 12); // decrypt the 12-byte header
  // Verification byte: if a data descriptor is used (bit 3), the check byte is the DOS-time high byte; otherwise
  // it is the CRC-32 high byte. We accept either to tolerate both writer conventions.
  check = pBlob[11];
  if ((check != (mz_uint8)(pStat->m_crc32 >> 24)) && (local_bit_flag & 8) == 0)
	goto done; // wrong password (best-effort; 1-in-256 false accept is inherent to ZipCrypto)

  pCipher = pBlob + 12;
  mz_zipcrypto_decrypt(&keys, pCipher, ct_len);

  if (pStat->m_method == 0)
  {
	if (ct_len != out_size) goto done;
	memcpy(pOut, pCipher, out_size);
	ok = MZ_TRUE;
  }
  else if (pStat->m_method == MZ_DEFLATED)
	ok = (tinfl_decompress_mem_to_mem(pOut, out_size, pCipher, ct_len, TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF) == out_size);
#ifdef MINIZ_USE_LZMA_SDK
  else if (pStat->m_method == MZ_LZMA)
	ok = mz_zip_lzma_decompress(pOut, out_size, pCipher, ct_len);
#endif
  if (ok)
	ok = (mz_crc32(MZ_CRC32_INIT, (const mz_uint8 *)pOut, out_size) == pStat->m_crc32);

done:
  memset(pBlob, 0, (size_t)comp_size);
  pZip->m_pFree(pZip->m_pAlloc_opaque, pBlob);
  return ok;
}
#endif // MINIZ_USE_AES

mz_bool mz_zip_reader_extract_to_mem_no_alloc(mz_zip_archive *pZip, mz_uint file_index, void *pBuf, size_t buf_size, mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size)
{
  int status = TINFL_STATUS_DONE;
  mz_uint64 needed_size, cur_file_ofs, comp_remaining, out_buf_ofs = 0, read_buf_size, read_buf_ofs = 0, read_buf_avail;
  mz_zip_archive_file_stat file_stat;
  void *pRead_buf;
  mz_uint32 local_header_u32[(MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof(mz_uint32) - 1) / sizeof(mz_uint32)]; mz_uint8 *pLocal_header = (mz_uint8 *)local_header_u32;
  tinfl_decompressor inflator;

  if ((buf_size) && (!pBuf))
	return MZ_FALSE;

  if (!mz_zip_reader_file_stat(pZip, file_index, &file_stat))
	return MZ_FALSE;

  // Empty file, or a directory (but not always a directory - I've seen odd zips with directories that have compressed data which inflates to 0 bytes)
  if (!file_stat.m_comp_size)
	return MZ_TRUE;

  // Entry is a subdirectory (I've seen old zips with dir entries which have compressed deflate data which inflates to 0 bytes, but these entries claim to uncompress to 512 bytes in the headers).
  // I'm torn how to handle this case - should it fail instead?
  if (mz_zip_reader_is_file_a_directory(pZip, file_index))
	return MZ_TRUE;

  // Encryption. WinZip AES (m_aes_strength != 0) and, when MINIZ_USE_AES is defined, legacy ZipCrypto (bit 0 set,
  // no AES extra) are supported below. Strong Encryption (bit 6, indicated by header bit 0x20) is not, and no
  // encrypted entry can be returned as raw compressed data through this API.
  if (file_stat.m_bit_flag & 32)
	return MZ_FALSE;
#ifdef MINIZ_USE_AES
  if ((file_stat.m_bit_flag & 1) && (flags & MZ_ZIP_FLAG_COMPRESSED_DATA))
	return MZ_FALSE; // can't hand back raw ciphertext through this API
#else
  if (file_stat.m_bit_flag & 1)
	return MZ_FALSE; // encryption unsupported without MINIZ_USE_AES
#endif

  // This function supports stored, Deflate, and optionally LZMA (m_method is the *real* method for AES entries).
  if ((!(flags & MZ_ZIP_FLAG_COMPRESSED_DATA)) && (file_stat.m_method != 0) && (file_stat.m_method != MZ_DEFLATED)
#ifdef MINIZ_USE_LZMA_SDK
	  && (file_stat.m_method != MZ_LZMA)
#endif
	 )
	return MZ_FALSE;

  // Ensure supplied output buffer is large enough.
  needed_size = (flags & MZ_ZIP_FLAG_COMPRESSED_DATA) ? file_stat.m_comp_size : file_stat.m_uncomp_size;
  if (buf_size < needed_size)
	return MZ_FALSE;

  // Read and parse the local directory entry.
  cur_file_ofs = file_stat.m_local_header_ofs;
  if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pLocal_header, MZ_ZIP_LOCAL_DIR_HEADER_SIZE) != MZ_ZIP_LOCAL_DIR_HEADER_SIZE)
	return MZ_FALSE;
  if (MZ_READ_LE32(pLocal_header) != MZ_ZIP_LOCAL_DIR_HEADER_SIG)
	return MZ_FALSE;

  cur_file_ofs += MZ_ZIP_LOCAL_DIR_HEADER_SIZE + MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_FILENAME_LEN_OFS) + MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_EXTRA_LEN_OFS);
  if ((cur_file_ofs > pZip->m_archive_size) || (file_stat.m_comp_size > (pZip->m_archive_size - cur_file_ofs)))
	return MZ_FALSE;

#ifdef MINIZ_USE_AES
  if (file_stat.m_aes_strength)
  {
	// Read the whole encrypted payload, verify + decrypt, then decompress the plaintext with the real method.
	return mz_zip_reader_extract_aes_to_mem(pZip, &file_stat, cur_file_ofs, pBuf, (size_t)needed_size);
  }
  if (file_stat.m_bit_flag & 1)
  {
	// Legacy ZipCrypto (read-only). Uses the local header's bit flag for the check-byte convention.
	return mz_zip_reader_extract_zipcrypto_to_mem(pZip, &file_stat, cur_file_ofs, MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_BIT_FLAG_OFS), pBuf, (size_t)needed_size);
  }
#endif

  if ((flags & MZ_ZIP_FLAG_COMPRESSED_DATA) || (!file_stat.m_method))
  {
	// The file is stored or the caller has requested the compressed data.
	if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pBuf, (size_t)needed_size) != needed_size)
	  return MZ_FALSE;
	return ((flags & MZ_ZIP_FLAG_COMPRESSED_DATA) != 0) || (mz_crc32(MZ_CRC32_INIT, (const mz_uint8 *)pBuf, (size_t)file_stat.m_uncomp_size) == file_stat.m_crc32);
  }

#ifdef MINIZ_USE_LZMA_SDK
  if (file_stat.m_method == MZ_LZMA)
  {
	void *pComp_data;
	mz_bool status;
	if (file_stat.m_comp_size > (mz_uint64)((size_t)-1))
	  return MZ_FALSE;
	if (pZip->m_pState->m_pMem)
	  pComp_data = (mz_uint8 *)pZip->m_pState->m_pMem + cur_file_ofs;
	else
	{
	  if (NULL == (pComp_data = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)file_stat.m_comp_size)))
		return MZ_FALSE;
	  if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pComp_data, (size_t)file_stat.m_comp_size) != file_stat.m_comp_size)
	  {
		pZip->m_pFree(pZip->m_pAlloc_opaque, pComp_data);
		return MZ_FALSE;
	  }
	}
	status = mz_zip_lzma_decompress(pBuf, (size_t)file_stat.m_uncomp_size, pComp_data, (size_t)file_stat.m_comp_size) &&
	  (mz_crc32(MZ_CRC32_INIT, (const mz_uint8 *)pBuf, (size_t)file_stat.m_uncomp_size) == file_stat.m_crc32);
	if (!pZip->m_pState->m_pMem)
	  pZip->m_pFree(pZip->m_pAlloc_opaque, pComp_data);
	return status;
  }
#endif

  // Decompress the file either directly from memory or from a file input buffer.
  tinfl_init(&inflator);

  if (pZip->m_pState->m_pMem)
  {
	// Read directly from the archive in memory.
	pRead_buf = (mz_uint8 *)pZip->m_pState->m_pMem + cur_file_ofs;
	read_buf_size = read_buf_avail = file_stat.m_comp_size;
	comp_remaining = 0;
  }
  else if (pUser_read_buf)
  {
	// Use a user provided read buffer.
	if (!user_read_buf_size)
	  return MZ_FALSE;
	pRead_buf = (mz_uint8 *)pUser_read_buf;
	read_buf_size = user_read_buf_size;
	read_buf_avail = 0;
	comp_remaining = file_stat.m_comp_size;
  }
  else
  {
	// Temporarily allocate a read buffer.
	read_buf_size = MZ_MIN(file_stat.m_comp_size, MZ_ZIP_MAX_IO_BUF_SIZE);
#if defined(_MSC_VER) && !defined(__clang__)
	if (((0, sizeof(size_t) == sizeof(mz_uint32))) && (read_buf_size > 0x7FFFFFFF))
#else
	if (((sizeof(size_t) == sizeof(mz_uint32))) && (read_buf_size > 0x7FFFFFFF))
#endif
	  return MZ_FALSE;
	if (NULL == (pRead_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)read_buf_size)))
	  return MZ_FALSE;
	read_buf_avail = 0;
	comp_remaining = file_stat.m_comp_size;
  }

  do
  {
	size_t in_buf_size, out_buf_size = (size_t)(file_stat.m_uncomp_size - out_buf_ofs);
	if ((!read_buf_avail) && (!pZip->m_pState->m_pMem))
	{
	  read_buf_avail = MZ_MIN(read_buf_size, comp_remaining);
	  if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pRead_buf, (size_t)read_buf_avail) != read_buf_avail)
	  {
		status = TINFL_STATUS_FAILED;
		break;
	  }
	  cur_file_ofs += read_buf_avail;
	  comp_remaining -= read_buf_avail;
	  read_buf_ofs = 0;
	}
	in_buf_size = (size_t)read_buf_avail;
	status = tinfl_decompress(&inflator, (mz_uint8 *)pRead_buf + read_buf_ofs, &in_buf_size, (mz_uint8 *)pBuf, (mz_uint8 *)pBuf + out_buf_ofs, &out_buf_size, TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF | (comp_remaining ? TINFL_FLAG_HAS_MORE_INPUT : 0));
	read_buf_avail -= in_buf_size;
	read_buf_ofs += in_buf_size;
	out_buf_ofs += out_buf_size;
  } while (status == TINFL_STATUS_NEEDS_MORE_INPUT);

  if (status == TINFL_STATUS_DONE)
  {
	// Make sure the entire file was decompressed, and check its CRC.
	if ((out_buf_ofs != file_stat.m_uncomp_size) || (mz_crc32(MZ_CRC32_INIT, (const mz_uint8 *)pBuf, (size_t)file_stat.m_uncomp_size) != file_stat.m_crc32))
	  status = TINFL_STATUS_FAILED;
  }

  if ((!pZip->m_pState->m_pMem) && (!pUser_read_buf))
	pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);

  return status == TINFL_STATUS_DONE;
}

mz_bool mz_zip_reader_extract_file_to_mem_no_alloc(mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size)
{
  int file_index = mz_zip_reader_locate_file(pZip, pFilename, NULL, flags);
  if (file_index < 0)
	return MZ_FALSE;
  return mz_zip_reader_extract_to_mem_no_alloc(pZip, file_index, pBuf, buf_size, flags, pUser_read_buf, user_read_buf_size);
}

mz_bool mz_zip_reader_extract_to_mem(mz_zip_archive *pZip, mz_uint file_index, void *pBuf, size_t buf_size, mz_uint flags)
{
  return mz_zip_reader_extract_to_mem_no_alloc(pZip, file_index, pBuf, buf_size, flags, NULL, 0);
}

mz_bool mz_zip_reader_extract_file_to_mem(mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, mz_uint flags)
{
  return mz_zip_reader_extract_file_to_mem_no_alloc(pZip, pFilename, pBuf, buf_size, flags, NULL, 0);
}

void *mz_zip_reader_extract_to_heap(mz_zip_archive *pZip, mz_uint file_index, size_t *pSize, mz_uint flags)
{
  mz_uint64 alloc_size;
  mz_zip_archive_file_stat file_stat;
  void *pBuf;

  if (pSize)
	*pSize = 0;
  if (!mz_zip_reader_file_stat(pZip, file_index, &file_stat))
	return NULL;

  alloc_size = (flags & MZ_ZIP_FLAG_COMPRESSED_DATA) ? file_stat.m_comp_size : file_stat.m_uncomp_size;
#if defined(_MSC_VER) && !defined(__clang__)
  if (((0, sizeof(size_t) == sizeof(mz_uint32))) && (alloc_size > 0x7FFFFFFF))
#else
  if (((sizeof(size_t) == sizeof(mz_uint32))) && (alloc_size > 0x7FFFFFFF))
#endif
	return NULL;
  if (NULL == (pBuf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)alloc_size)))
	return NULL;

  if (!mz_zip_reader_extract_to_mem(pZip, file_index, pBuf, (size_t)alloc_size, flags))
  {
	pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
	return NULL;
  }

  if (pSize) *pSize = (size_t)alloc_size;
  return pBuf;
}

void *mz_zip_reader_extract_file_to_heap(mz_zip_archive *pZip, const char *pFilename, size_t *pSize, mz_uint flags)
{
  int file_index = mz_zip_reader_locate_file(pZip, pFilename, NULL, flags);
  if (file_index < 0)
  {
	if (pSize) *pSize = 0;
	return MZ_FALSE;
  }
  return mz_zip_reader_extract_to_heap(pZip, file_index, pSize, flags);
}

mz_bool mz_zip_reader_extract_to_callback(mz_zip_archive *pZip, mz_uint file_index, mz_file_write_func pCallback, void *pOpaque, mz_uint flags)
{
  int status = TINFL_STATUS_DONE; mz_uint file_crc32 = MZ_CRC32_INIT;
  mz_uint64 read_buf_size, read_buf_ofs = 0, read_buf_avail, comp_remaining, out_buf_ofs = 0, cur_file_ofs;
  mz_zip_archive_file_stat file_stat;
  void *pRead_buf = NULL; void *pWrite_buf = NULL;
  mz_uint32 local_header_u32[(MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof(mz_uint32) - 1) / sizeof(mz_uint32)]; mz_uint8 *pLocal_header = (mz_uint8 *)local_header_u32;

  if (!mz_zip_reader_file_stat(pZip, file_index, &file_stat))
	return MZ_FALSE;

  // Empty file, or a directory (but not always a directory - I've seen odd zips with directories that have compressed data which inflates to 0 bytes)
  if (!file_stat.m_comp_size)
	return MZ_TRUE;

  // Entry is a subdirectory (I've seen old zips with dir entries which have compressed deflate data which inflates to 0 bytes, but these entries claim to uncompress to 512 bytes in the headers).
  // I'm torn how to handle this case - should it fail instead?
  if (mz_zip_reader_is_file_a_directory(pZip, file_index))
	return MZ_TRUE;

#ifdef MINIZ_USE_AES
  // Encrypted entries (WinZip AES or legacy ZipCrypto) are decrypted+decompressed to a heap buffer via the
  // mem path, then streamed to the callback. Raw ciphertext output is not offered for encrypted entries.
  if ((file_stat.m_bit_flag & 1) && !(flags & MZ_ZIP_FLAG_COMPRESSED_DATA))
  {
	size_t sz = 0;
	void *p = mz_zip_reader_extract_to_heap(pZip, file_index, &sz, flags);
	mz_bool ok;
	if (!p)
	  return MZ_FALSE;
	ok = (pCallback(pOpaque, 0, p, sz) == sz);
	pZip->m_pFree(pZip->m_pAlloc_opaque, p);
	return ok;
  }
#endif

  // Encryption is not supported. Patched data can only be returned in its raw compressed form.
  if ((file_stat.m_bit_flag & 1) || ((file_stat.m_bit_flag & 32) && !(flags & MZ_ZIP_FLAG_COMPRESSED_DATA)))
	return MZ_FALSE;

  // This function supports stored, Deflate, and optionally LZMA.
  if ((!(flags & MZ_ZIP_FLAG_COMPRESSED_DATA)) && (file_stat.m_method != 0) && (file_stat.m_method != MZ_DEFLATED)
#ifdef MINIZ_USE_LZMA_SDK
	  && (file_stat.m_method != MZ_LZMA)
#endif
	 )
	return MZ_FALSE;

#ifdef MINIZ_USE_LZMA_SDK
  if ((file_stat.m_method == MZ_LZMA) && !(flags & MZ_ZIP_FLAG_COMPRESSED_DATA))
  {
	void *pOutput;
	mz_bool result;
	if (file_stat.m_uncomp_size > (mz_uint64)((size_t)-1))
	  return MZ_FALSE;
	if (NULL == (pOutput = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)MZ_MAX(file_stat.m_uncomp_size, 1))))
	  return MZ_FALSE;
	result = mz_zip_reader_extract_to_mem(pZip, file_index, pOutput, (size_t)file_stat.m_uncomp_size, flags) &&
	  (pCallback(pOpaque, 0, pOutput, (size_t)file_stat.m_uncomp_size) == file_stat.m_uncomp_size);
	pZip->m_pFree(pZip->m_pAlloc_opaque, pOutput);
	return result;
  }
#endif

  // Read and parse the local directory entry.
  cur_file_ofs = file_stat.m_local_header_ofs;
  if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pLocal_header, MZ_ZIP_LOCAL_DIR_HEADER_SIZE) != MZ_ZIP_LOCAL_DIR_HEADER_SIZE)
	return MZ_FALSE;
  if (MZ_READ_LE32(pLocal_header) != MZ_ZIP_LOCAL_DIR_HEADER_SIG)
	return MZ_FALSE;

  cur_file_ofs += MZ_ZIP_LOCAL_DIR_HEADER_SIZE + MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_FILENAME_LEN_OFS) + MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_EXTRA_LEN_OFS);
  if ((cur_file_ofs > pZip->m_archive_size) || (file_stat.m_comp_size > (pZip->m_archive_size - cur_file_ofs)))
	return MZ_FALSE;

  // Decompress the file either directly from memory or from a file input buffer.
  if (pZip->m_pState->m_pMem)
  {
	pRead_buf = (mz_uint8 *)pZip->m_pState->m_pMem + cur_file_ofs;
	read_buf_size = read_buf_avail = file_stat.m_comp_size;
	comp_remaining = 0;
  }
  else
  {
	read_buf_size = MZ_MIN(file_stat.m_comp_size, MZ_ZIP_MAX_IO_BUF_SIZE);
	if (NULL == (pRead_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)read_buf_size)))
	  return MZ_FALSE;
	read_buf_avail = 0;
	comp_remaining = file_stat.m_comp_size;
  }

  if ((flags & MZ_ZIP_FLAG_COMPRESSED_DATA) || (!file_stat.m_method))
  {
	// The file is stored or the caller has requested the compressed data.
	if (pZip->m_pState->m_pMem)
	{
#if defined(_MSC_VER) && !defined(__clang__)
	  if (((0, sizeof(size_t) == sizeof(mz_uint32))) && (file_stat.m_comp_size > 0xFFFFFFFF))
#else
	  if (((sizeof(size_t) == sizeof(mz_uint32))) && (file_stat.m_comp_size > 0xFFFFFFFF))
#endif
		return MZ_FALSE;
	  if (pCallback(pOpaque, out_buf_ofs, pRead_buf, (size_t)file_stat.m_comp_size) != file_stat.m_comp_size)
		status = TINFL_STATUS_FAILED;
	  else if (!(flags & MZ_ZIP_FLAG_COMPRESSED_DATA))
		file_crc32 = (mz_uint32)mz_crc32(file_crc32, (const mz_uint8 *)pRead_buf, (size_t)file_stat.m_comp_size);
	  cur_file_ofs += file_stat.m_comp_size;
	  out_buf_ofs += file_stat.m_comp_size;
	  comp_remaining = 0;
	}
	else
	{
	  while (comp_remaining)
	  {
		read_buf_avail = MZ_MIN(read_buf_size, comp_remaining);
		if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pRead_buf, (size_t)read_buf_avail) != read_buf_avail)
		{
		  status = TINFL_STATUS_FAILED;
		  break;
		}

		if (!(flags & MZ_ZIP_FLAG_COMPRESSED_DATA))
		  file_crc32 = (mz_uint32)mz_crc32(file_crc32, (const mz_uint8 *)pRead_buf, (size_t)read_buf_avail);

		if (pCallback(pOpaque, out_buf_ofs, pRead_buf, (size_t)read_buf_avail) != read_buf_avail)
		{
		  status = TINFL_STATUS_FAILED;
		  break;
		}
		cur_file_ofs += read_buf_avail;
		out_buf_ofs += read_buf_avail;
		comp_remaining -= read_buf_avail;
	  }
	}
  }
  else
  {
	tinfl_decompressor inflator;
	tinfl_init(&inflator);

	if (NULL == (pWrite_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, TINFL_LZ_DICT_SIZE)))
	  status = TINFL_STATUS_FAILED;
	else
	{
	  do
	  {
		mz_uint8 *pWrite_buf_cur = (mz_uint8 *)pWrite_buf + (out_buf_ofs & (TINFL_LZ_DICT_SIZE - 1));
		size_t in_buf_size, out_buf_size = TINFL_LZ_DICT_SIZE - (out_buf_ofs & (TINFL_LZ_DICT_SIZE - 1));
		if ((!read_buf_avail) && (!pZip->m_pState->m_pMem))
		{
		  read_buf_avail = MZ_MIN(read_buf_size, comp_remaining);
		  if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pRead_buf, (size_t)read_buf_avail) != read_buf_avail)
		  {
			status = TINFL_STATUS_FAILED;
			break;
		  }
		  cur_file_ofs += read_buf_avail;
		  comp_remaining -= read_buf_avail;
		  read_buf_ofs = 0;
		}

		in_buf_size = (size_t)read_buf_avail;
		status = tinfl_decompress(&inflator, (const mz_uint8 *)pRead_buf + read_buf_ofs, &in_buf_size, (mz_uint8 *)pWrite_buf, pWrite_buf_cur, &out_buf_size, comp_remaining ? TINFL_FLAG_HAS_MORE_INPUT : 0);
		read_buf_avail -= in_buf_size;
		read_buf_ofs += in_buf_size;

		if (out_buf_size)
		{
		  if (pCallback(pOpaque, out_buf_ofs, pWrite_buf_cur, out_buf_size) != out_buf_size)
		  {
			status = TINFL_STATUS_FAILED;
			break;
		  }
		  file_crc32 = (mz_uint32)mz_crc32(file_crc32, pWrite_buf_cur, out_buf_size);
		  if ((out_buf_ofs += out_buf_size) > file_stat.m_uncomp_size)
		  {
			status = TINFL_STATUS_FAILED;
			break;
		  }
		}
	  } while ((status == TINFL_STATUS_NEEDS_MORE_INPUT) || (status == TINFL_STATUS_HAS_MORE_OUTPUT));
	}
  }

  if ((status == TINFL_STATUS_DONE) && (!(flags & MZ_ZIP_FLAG_COMPRESSED_DATA)))
  {
	// Make sure the entire file was decompressed, and check its CRC.
	if ((out_buf_ofs != file_stat.m_uncomp_size) || (file_crc32 != file_stat.m_crc32))
	  status = TINFL_STATUS_FAILED;
  }

  if (!pZip->m_pState->m_pMem)
	pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
  if (pWrite_buf)
	pZip->m_pFree(pZip->m_pAlloc_opaque, pWrite_buf);

  return status == TINFL_STATUS_DONE;
}

mz_bool mz_zip_reader_extract_file_to_callback(mz_zip_archive *pZip, const char *pFilename, mz_file_write_func pCallback, void *pOpaque, mz_uint flags)
{
  int file_index = mz_zip_reader_locate_file(pZip, pFilename, NULL, flags);
  if (file_index < 0)
	return MZ_FALSE;
  return mz_zip_reader_extract_to_callback(pZip, file_index, pCallback, pOpaque, flags);
}

#ifndef MINIZ_NO_STDIO
static size_t mz_zip_file_write_callback(void *pOpaque, mz_uint64 ofs, const void *pBuf, size_t n)
{
  (void)ofs; return MZ_FWRITE(pBuf, 1, n, (MZ_FILE*)pOpaque);
}

mz_bool mz_zip_reader_extract_to_file(mz_zip_archive *pZip, mz_uint file_index, const char *pDst_filename, mz_uint flags)
{
  mz_bool status;
  mz_zip_archive_file_stat file_stat;
  MZ_FILE *pFile;
  if (!mz_zip_reader_file_stat(pZip, file_index, &file_stat))
	return MZ_FALSE;
  pFile = MZ_FOPEN(pDst_filename, "wb");
  if (!pFile)
	return MZ_FALSE;
  status = mz_zip_reader_extract_to_callback(pZip, file_index, mz_zip_file_write_callback, pFile, flags);
  if (MZ_FCLOSE(pFile) == EOF)
	return MZ_FALSE;
#ifndef MINIZ_NO_TIME
  if (status)
	mz_zip_set_file_times(pDst_filename, file_stat.m_time, file_stat.m_time);
#endif
  return status;
}
#endif // #ifndef MINIZ_NO_STDIO

mz_bool mz_zip_reader_end(mz_zip_archive *pZip)
{
	if ((!pZip) || (!pZip->m_pState) || (!pZip->m_pAlloc) || (!pZip->m_pFree) || (pZip->m_zip_mode != MZ_ZIP_MODE_READING))
	return MZ_FALSE;

	if (pZip->m_pState)
	{
	  mz_zip_internal_state *pState = pZip->m_pState; pZip->m_pState = NULL;

#ifndef MINIZ_NO_STDIO
	  if (pState->m_files.m_p)
	  {
		 size_t i;
		 for (i = 0; i < pState->m_files.m_size; ++i)
		 {
		   MZ_FILE *pFile = MZ_ZIP_ARRAY_ELEMENT(&pState->m_files, MZ_FILE *, i);
		   if (pFile)
			 MZ_FCLOSE(pFile);
		 }
	  }
#endif // #ifndef MINIZ_NO_STDIO

	  mz_zip_array_clear(pZip, &pState->m_central_dir);
	  mz_zip_array_clear(pZip, &pState->m_central_dir_offsets);
	  mz_zip_array_clear(pZip, &pState->m_sorted_central_dir_offsets);
	  mz_zip_array_clear(pZip, &pState->m_disk_offsets);
	  mz_zip_array_clear(pZip, &pState->m_files);

	  if (pState->m_pPassword)
	  {
		memset(pState->m_pPassword, 0, pState->m_password_len);
		pZip->m_pFree(pZip->m_pAlloc_opaque, pState->m_pPassword);
		pState->m_pPassword = NULL;
		pState->m_password_len = 0;
	  }

#ifndef MINIZ_NO_STDIO
	if (pState->m_pFile)
	{
	  MZ_FCLOSE(pState->m_pFile);
	  pState->m_pFile = NULL;
	}
#endif // #ifndef MINIZ_NO_STDIO

	pZip->m_pFree(pZip->m_pAlloc_opaque, pState);
  }
  pZip->m_zip_mode = MZ_ZIP_MODE_INVALID;

  return MZ_TRUE;
}

mz_bool mz_zip_reader_set_password(mz_zip_archive *pZip, const char *pPassword)
{
  mz_zip_internal_state *pState;
  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_READING))
	return MZ_FALSE;
  pState = pZip->m_pState;
  if (pState->m_pPassword)
  {
	memset(pState->m_pPassword, 0, pState->m_password_len);
	pZip->m_pFree(pZip->m_pAlloc_opaque, pState->m_pPassword);
	pState->m_pPassword = NULL;
	pState->m_password_len = 0;
  }
  if (pPassword)
  {
	size_t n = strlen(pPassword);
	pState->m_pPassword = (char *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, n + 1);
	if (!pState->m_pPassword)
	  return MZ_FALSE;
	memcpy(pState->m_pPassword, pPassword, n + 1);
	pState->m_password_len = n;
  }
  return MZ_TRUE;
}

#ifndef MINIZ_NO_STDIO
mz_bool mz_zip_reader_extract_file_to_file(mz_zip_archive *pZip, const char *pArchive_filename, const char *pDst_filename, mz_uint flags)
{
  int file_index = mz_zip_reader_locate_file(pZip, pArchive_filename, NULL, flags);
  if (file_index < 0)
	return MZ_FALSE;
  return mz_zip_reader_extract_to_file(pZip, file_index, pDst_filename, flags);
}
#endif

// ------------------- .ZIP archive writing

#ifndef MINIZ_NO_ARCHIVE_WRITING_APIS

static void mz_write_le16(mz_uint8 *p, mz_uint16 v) { p[0] = (mz_uint8)v; p[1] = (mz_uint8)(v >> 8); }
static void mz_write_le32(mz_uint8 *p, mz_uint32 v) { p[0] = (mz_uint8)v; p[1] = (mz_uint8)(v >> 8); p[2] = (mz_uint8)(v >> 16); p[3] = (mz_uint8)(v >> 24); }
static void mz_write_le64(mz_uint8 *p, mz_uint64 v) { mz_write_le32(p, (mz_uint32)v); mz_write_le32(p + 4, (mz_uint32)(v >> 32)); }
#define MZ_WRITE_LE16(p, v) mz_write_le16((mz_uint8 *)(p), (mz_uint16)(v))
#define MZ_WRITE_LE32(p, v) mz_write_le32((mz_uint8 *)(p), (mz_uint32)(v))
#define MZ_WRITE_LE64(p, v) mz_write_le64((mz_uint8 *)(p), (mz_uint64)(v))

static mz_uint mz_zip_writer_create_zip64_extra_data(mz_uint8 *pDst, mz_uint64 uncomp_size, mz_uint64 comp_size, mz_uint64 local_header_ofs, mz_bool write_uncomp_size, mz_bool write_comp_size, mz_bool write_local_header_ofs)
{
  mz_uint8 *p = pDst + sizeof(mz_uint16) * 2;
  mz_uint16 payload_size = 0;
  if (write_uncomp_size)
  {
	MZ_WRITE_LE64(p, uncomp_size);
	p += sizeof(mz_uint64);
	payload_size = (mz_uint16)(payload_size + sizeof(mz_uint64));
  }
  if (write_comp_size)
  {
	MZ_WRITE_LE64(p, comp_size);
	p += sizeof(mz_uint64);
	payload_size = (mz_uint16)(payload_size + sizeof(mz_uint64));
  }
  if (write_local_header_ofs)
  {
	MZ_WRITE_LE64(p, local_header_ofs);
	payload_size = (mz_uint16)(payload_size + sizeof(mz_uint64));
  }
  MZ_WRITE_LE16(pDst, MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID);
  MZ_WRITE_LE16(pDst + sizeof(mz_uint16), payload_size);
  return sizeof(mz_uint16) * 2 + payload_size;
}

static mz_bool mz_zip_writer_may_need_zip64_compressed_size(mz_uint64 uncomp_size)
{
  mz_uint64 conservative_threshold = (((mz_uint64)MZ_UINT32_MAX - 128) * 100) / 110;
  return uncomp_size > conservative_threshold;
}

mz_bool mz_zip_writer_init(mz_zip_archive *pZip, mz_uint64 existing_size)
{
  if ((!pZip) || (pZip->m_pState) || (!pZip->m_pWrite) || (pZip->m_zip_mode != MZ_ZIP_MODE_INVALID))
	return MZ_FALSE;

  if (pZip->m_file_offset_alignment)
  {
	// Ensure user specified file offset alignment is a power of 2.
	if (pZip->m_file_offset_alignment & (pZip->m_file_offset_alignment - 1))
	  return MZ_FALSE;
  }

  if (!pZip->m_pAlloc) pZip->m_pAlloc = def_alloc_func;
  if (!pZip->m_pFree) pZip->m_pFree = def_free_func;
  if (!pZip->m_pRealloc) pZip->m_pRealloc = def_realloc_func;

  pZip->m_zip_mode = MZ_ZIP_MODE_WRITING;
  pZip->m_archive_size = existing_size;
  pZip->m_central_directory_file_ofs = 0;
  pZip->m_total_files = 0;

  if (NULL == (pZip->m_pState = (mz_zip_internal_state *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, sizeof(mz_zip_internal_state))))
	return MZ_FALSE;
  memset(pZip->m_pState, 0, sizeof(mz_zip_internal_state));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir, sizeof(mz_uint8));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir_offsets, sizeof(size_t));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_sorted_central_dir_offsets, sizeof(mz_uint32));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_disk_offsets, sizeof(mz_uint64));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_files, sizeof(MZ_FILE *));
  return MZ_TRUE;
}

static size_t mz_zip_heap_write_func(void *pOpaque, mz_uint64 file_ofs, const void *pBuf, size_t n)
{
  mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;
  mz_zip_internal_state *pState = pZip->m_pState;
  mz_uint64 new_size;
  if ((mz_uint64)n > ((mz_uint64)-1 - file_ofs))
	return 0;
  new_size = MZ_MAX(file_ofs + n, pState->m_mem_size);
#if defined(_MSC_VER) && !defined(__clang__)
  if ((!n) || ((0, sizeof(size_t) == sizeof(mz_uint32)) && (new_size > 0x7FFFFFFF)))
#else
  if ((!n) || ((sizeof(size_t) == sizeof(mz_uint32)) && (new_size > 0x7FFFFFFF)))
#endif
	return 0;
  if (new_size > pState->m_mem_capacity)
  {
	void *pNew_block;
	size_t new_capacity = MZ_MAX(64, pState->m_mem_capacity);
	while (new_capacity < new_size)
	{
	  if (new_capacity > ((size_t)-1 / 2))
		return 0;
	  new_capacity *= 2;
	}
	if (NULL == (pNew_block = pZip->m_pRealloc(pZip->m_pAlloc_opaque, pState->m_pMem, 1, new_capacity)))
	  return 0;
	pState->m_pMem = pNew_block; pState->m_mem_capacity = new_capacity;
  }
  memcpy((mz_uint8 *)pState->m_pMem + file_ofs, pBuf, n);
  pState->m_mem_size = (size_t)new_size;
  return n;
}

mz_bool mz_zip_writer_init_heap(mz_zip_archive *pZip, size_t size_to_reserve_at_beginning, size_t initial_allocation_size)
{
  pZip->m_pWrite = mz_zip_heap_write_func;
  pZip->m_pIO_opaque = pZip;
  if (!mz_zip_writer_init(pZip, size_to_reserve_at_beginning))
	return MZ_FALSE;
  if (0 != (initial_allocation_size = MZ_MAX(initial_allocation_size, size_to_reserve_at_beginning)))
  {
	if (NULL == (pZip->m_pState->m_pMem = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, initial_allocation_size)))
	{
	  mz_zip_writer_end(pZip);
	  return MZ_FALSE;
	}
	pZip->m_pState->m_mem_capacity = initial_allocation_size;
  }
  return MZ_TRUE;
}

#ifndef MINIZ_NO_STDIO
static size_t mz_zip_file_write_func(void *pOpaque, mz_uint64 file_ofs, const void *pBuf, size_t n)
{
  mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;
  mz_int64 cur_ofs = MZ_FTELL64(pZip->m_pState->m_pFile);
  if (((mz_int64)file_ofs < 0) || (((cur_ofs != (mz_int64)file_ofs)) && (MZ_FSEEK64(pZip->m_pState->m_pFile, (mz_int64)file_ofs, SEEK_SET))))
	return 0;
  return MZ_FWRITE(pBuf, 1, n, pZip->m_pState->m_pFile);
}

static char *mz_zip_writer_split_filename(mz_zip_archive *pZip, mz_uint32 disk_number)
{
  mz_zip_internal_state *pState = pZip->m_pState;
  size_t capacity = strlen(pState->m_pZip_filename) + 17;
  char *pName = (char *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, capacity);
  if ((!pName) || (!mz_zip_split_make_filename(pName, capacity, pState->m_pZip_filename, disk_number)))
  {
	if (pName) pZip->m_pFree(pZip->m_pAlloc_opaque, pName);
	return NULL;
  }
  return pName;
}

static mz_bool mz_zip_writer_split_open_next_disk(mz_zip_archive *pZip, mz_uint64 disk_base)
{
  mz_zip_internal_state *pState = pZip->m_pState;
  MZ_FILE *pFile;
  char *pName;
  size_t orig_disk_count = pState->m_disk_offsets.m_size;
  if ((orig_disk_count >= MZ_UINT32_MAX) || (NULL == (pName = mz_zip_writer_split_filename(pZip, (mz_uint32)orig_disk_count + 1))))
	return MZ_FALSE;
  if (NULL == (pFile = MZ_FOPEN(pName, "w+b")))
  {
	pZip->m_pFree(pZip->m_pAlloc_opaque, pName);
	return MZ_FALSE;
  }
  pZip->m_pFree(pZip->m_pAlloc_opaque, pName);
  if ((!mz_zip_array_push_back(pZip, &pState->m_disk_offsets, &disk_base, 1)) ||
	  (!mz_zip_array_push_back(pZip, &pState->m_files, &pFile, 1)))
  {
	mz_zip_array_resize(pZip, &pState->m_disk_offsets, orig_disk_count, MZ_FALSE);
	MZ_FCLOSE(pFile);
	return MZ_FALSE;
  }
  pState->m_num_disks = (mz_uint32)pState->m_files.m_size;
  return MZ_TRUE;
}

static mz_bool mz_zip_writer_split_close_current_disk(mz_zip_archive *pZip)
{
  mz_zip_internal_state *pState = pZip->m_pState;
  size_t disk_index;
  MZ_FILE *pFile;
  if (!pState->m_files.m_size)
	return MZ_FALSE;
  disk_index = pState->m_files.m_size - 1;
  pFile = MZ_ZIP_ARRAY_ELEMENT(&pState->m_files, MZ_FILE *, disk_index);
  if (pFile)
  {
	int close_status = MZ_FCLOSE(pFile);
	MZ_ZIP_ARRAY_ELEMENT(&pState->m_files, MZ_FILE *, disk_index) = NULL;
	if (close_status == EOF)
	  return MZ_FALSE;
  }
  return MZ_TRUE;
}

static size_t mz_zip_split_file_write_func(void *pOpaque, mz_uint64 file_ofs, const void *pBuf, size_t n)
{
  mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;
  mz_zip_internal_state *pState = pZip->m_pState;
  size_t total_written = 0;
  while (n)
  {
	size_t disk_index = 0, bytes_written, to_write;
	mz_uint64 disk_base, disk_end, disk_ofs;
	MZ_FILE *pFile;
	mz_bool close_after_write = MZ_FALSE;
	char *pName = NULL;
	while ((disk_index + 1 < pState->m_disk_offsets.m_size) &&
		   (file_ofs >= MZ_ZIP_ARRAY_ELEMENT(&pState->m_disk_offsets, mz_uint64, disk_index + 1)))
	  ++disk_index;
	disk_base = MZ_ZIP_ARRAY_ELEMENT(&pState->m_disk_offsets, mz_uint64, disk_index);
	disk_end = (disk_index + 1 < pState->m_disk_offsets.m_size) ?
	  MZ_ZIP_ARRAY_ELEMENT(&pState->m_disk_offsets, mz_uint64, disk_index + 1) : disk_base + pState->m_split_size;
	if (file_ofs == disk_end)
	{
	  if (disk_index + 1 < pState->m_disk_offsets.m_size)
	  {
		++disk_index;
		disk_base = disk_end;
		disk_end = (disk_index + 1 < pState->m_disk_offsets.m_size) ?
		  MZ_ZIP_ARRAY_ELEMENT(&pState->m_disk_offsets, mz_uint64, disk_index + 1) : disk_base + pState->m_split_size;
	  }
	  else
	  {
		if ((!mz_zip_writer_split_close_current_disk(pZip)) || (!mz_zip_writer_split_open_next_disk(pZip, file_ofs)))
		  break;
		continue;
	  }
	}
	if ((file_ofs < disk_base) || (file_ofs >= disk_end))
	  break;
	disk_ofs = file_ofs - disk_base;
	to_write = (size_t)MZ_MIN((mz_uint64)n, disk_end - file_ofs);
	pFile = MZ_ZIP_ARRAY_ELEMENT(&pState->m_files, MZ_FILE *, disk_index);
	if (!pFile)
	{
	  if (NULL == (pName = mz_zip_writer_split_filename(pZip, (mz_uint32)disk_index + 1)))
		break;
	  pFile = MZ_FOPEN(pName, "r+b");
	  pZip->m_pFree(pZip->m_pAlloc_opaque, pName);
	  if (!pFile)
		break;
	  close_after_write = MZ_TRUE;
	}
	if (MZ_FSEEK64(pFile, (mz_int64)disk_ofs, SEEK_SET))
	{
	  if (close_after_write) MZ_FCLOSE(pFile);
	  break;
	}
	bytes_written = MZ_FWRITE((const mz_uint8 *)pBuf + total_written, 1, to_write, pFile);
	if (close_after_write) MZ_FCLOSE(pFile);
	total_written += bytes_written;
	file_ofs += bytes_written;
	n -= bytes_written;
	if (bytes_written != to_write)
	  break;
  }
  return total_written;
}

mz_bool mz_zip_writer_init_file(mz_zip_archive *pZip, const char *pFilename, mz_uint64 size_to_reserve_at_beginning)
{
  MZ_FILE *pFile;
  pZip->m_pWrite = mz_zip_file_write_func;
  pZip->m_pIO_opaque = pZip;
  if (!mz_zip_writer_init(pZip, size_to_reserve_at_beginning))
	return MZ_FALSE;
  if (NULL == (pFile = MZ_FOPEN(pFilename, "wb")))
  {
	mz_zip_writer_end(pZip);
	return MZ_FALSE;
  }
  pZip->m_pState->m_pFile = pFile;
  if (size_to_reserve_at_beginning)
  {
	mz_uint64 cur_ofs = 0; char buf[4096]; MZ_CLEAR_OBJ(buf);
	do
	{
	  size_t n = (size_t)MZ_MIN(sizeof(buf), size_to_reserve_at_beginning);
	  if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_ofs, buf, n) != n)
	  {
		mz_zip_writer_end(pZip);
		return MZ_FALSE;
	  }
	  cur_ofs += n; size_to_reserve_at_beginning -= n;
	} while (size_to_reserve_at_beginning);
  }
  return MZ_TRUE;
}

mz_bool mz_zip_writer_init_file_split(mz_zip_archive *pZip, const char *pFilename, mz_uint64 split_size)
{
  size_t filename_size;
  mz_uint8 spanning_sig[sizeof(mz_uint32)];
  char *pFirst_name;
  if ((!pZip) || (!pFilename) || (split_size < 65536) || (split_size > MZ_UINT32_MAX))
	return MZ_FALSE;
  filename_size = strlen(pFilename);
  if (filename_size > (size_t)-17)
	return MZ_FALSE;
  pZip->m_pWrite = mz_zip_split_file_write_func;
  pZip->m_pIO_opaque = pZip;
  if (!mz_zip_writer_init(pZip, 0))
	return MZ_FALSE;
  pZip->m_pState->m_split_size = split_size;
  if (NULL == (pZip->m_pState->m_pZip_filename = (char *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, filename_size + 1)))
  {
	mz_zip_writer_end(pZip);
	return MZ_FALSE;
  }
  memcpy(pZip->m_pState->m_pZip_filename, pFilename, filename_size + 1);
  if (NULL == (pFirst_name = mz_zip_writer_split_filename(pZip, 1)))
  {
	mz_zip_writer_end(pZip);
	return MZ_FALSE;
  }
  pZip->m_pFree(pZip->m_pAlloc_opaque, pFirst_name);
  MZ_DELETE_FILE(pFilename);
  if (!mz_zip_writer_split_open_next_disk(pZip, 0))
  {
	mz_zip_writer_end(pZip);
	return MZ_FALSE;
  }
  MZ_WRITE_LE32(spanning_sig, 0x08074b50);
  if (pZip->m_pWrite(pZip->m_pIO_opaque, 0, spanning_sig, sizeof(spanning_sig)) != sizeof(spanning_sig))
  {
	mz_zip_writer_end(pZip);
	return MZ_FALSE;
  }
  pZip->m_archive_size = sizeof(spanning_sig);
  return MZ_TRUE;
}
#endif // #ifndef MINIZ_NO_STDIO

mz_bool mz_zip_writer_set_central_directory_compression(mz_zip_archive *pZip, mz_uint method, mz_uint level)
{
  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING) ||
	  ((method != 0) && (method != MZ_DEFLATED)
#ifdef MINIZ_USE_LZMA_SDK
	   && (method != MZ_LZMA)
#endif
	  ) || (level > MZ_UBER_COMPRESSION))
	return MZ_FALSE;
  pZip->m_pState->m_central_dir_method = (mz_uint16)method;
  pZip->m_pState->m_central_dir_level = (mz_uint16)level;
  return MZ_TRUE;
}

mz_bool mz_zip_writer_init_from_reader(mz_zip_archive *pZip,
	const char *pFilename)
{
  mz_zip_internal_state *pState;
#ifdef MINIZ_NO_STDIO
	(void)(pFilename);
#endif
  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_READING))
	return MZ_FALSE;
  // Leave one file index value reserved for overflow detection.
  if (pZip->m_total_files == (mz_uint)-1)
	return MZ_FALSE;

  pState = pZip->m_pState;

  // Split archives cannot be converted in place because the writer only emits single-disk archives.
  if (pState->m_files.m_size)
	return MZ_FALSE;

  if (pState->m_pFile)
  {
#ifdef MINIZ_NO_STDIO
	return MZ_FALSE;
#else
	// Archive is being read from stdio - try to reopen as writable.
	if (pZip->m_pIO_opaque != pZip)
	  return MZ_FALSE;
	if (!pFilename)
	  return MZ_FALSE;
	pZip->m_pWrite = mz_zip_file_write_func;
	if (NULL == (pState->m_pFile = MZ_FREOPEN(pFilename, "r+b", pState->m_pFile)))
	{
	  // The mz_zip_archive is now in a bogus state because pState->m_pFile is NULL, so just close it.
	  mz_zip_reader_end(pZip);
	  return MZ_FALSE;
	}
#endif // #ifdef MINIZ_NO_STDIO
  }
  else if (pState->m_pMem)
  {
	// Archive lives in a memory block. Assume it's from the heap that we can resize using the realloc callback.
	if (pZip->m_pIO_opaque != pZip)
	  return MZ_FALSE;
	pState->m_mem_capacity = pState->m_mem_size;
	pZip->m_pWrite = mz_zip_heap_write_func;
  }
  // Archive is being read via a user provided read function - make sure the user has specified a write function too.
  else if (!pZip->m_pWrite)
	return MZ_FALSE;

  // Start writing new files at the archive's current central directory location.
  pZip->m_archive_size = pZip->m_central_directory_file_ofs;
  pZip->m_zip_mode = MZ_ZIP_MODE_WRITING;
  pZip->m_central_directory_file_ofs = 0;

  return MZ_TRUE;
}

mz_bool mz_zip_writer_add_mem(mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf, size_t buf_size, mz_uint level_and_flags)
{
  return mz_zip_writer_add_mem_ex(pZip, pArchive_name, pBuf, buf_size, NULL, 0, level_and_flags, 0, 0);
}

typedef struct
{
  mz_zip_archive *m_pZip;
  mz_uint64 m_cur_archive_file_ofs;
  mz_uint64 m_comp_size;
} mz_zip_writer_add_state;

static mz_bool mz_zip_writer_add_put_buf_callback(const void* pBuf, int len, void *pUser)
{
  mz_zip_writer_add_state *pState = (mz_zip_writer_add_state *)pUser;
  if ((int)pState->m_pZip->m_pWrite(pState->m_pZip->m_pIO_opaque, pState->m_cur_archive_file_ofs, pBuf, len) != len)
	return MZ_FALSE;
  pState->m_cur_archive_file_ofs += len;
  pState->m_comp_size += len;
  return MZ_TRUE;
}

static mz_bool mz_zip_writer_create_local_dir_header(mz_zip_archive *pZip, mz_uint8 *pDst, mz_uint16 filename_size, mz_uint16 extra_size, mz_uint64 uncomp_size, mz_uint64 comp_size, mz_uint32 uncomp_crc32, mz_uint16 method, mz_uint16 bit_flags, mz_uint16 dos_time, mz_uint16 dos_date, mz_bool use_zip64)
{
  (void)pZip;
  memset(pDst, 0, MZ_ZIP_LOCAL_DIR_HEADER_SIZE);
  MZ_WRITE_LE32(pDst + MZ_ZIP_LDH_SIG_OFS, MZ_ZIP_LOCAL_DIR_HEADER_SIG);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_VERSION_NEEDED_OFS, (method == MZ_ZIP_AES) ? 51 : (method == MZ_LZMA) ? 63 : (use_zip64 ? 45 : (method ? 20 : 0)));
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_BIT_FLAG_OFS, bit_flags);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_METHOD_OFS, method);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_FILE_TIME_OFS, dos_time);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_FILE_DATE_OFS, dos_date);
  MZ_WRITE_LE32(pDst + MZ_ZIP_LDH_CRC32_OFS, uncomp_crc32);
  MZ_WRITE_LE32(pDst + MZ_ZIP_LDH_COMPRESSED_SIZE_OFS, use_zip64 ? MZ_UINT32_MAX : comp_size);
  MZ_WRITE_LE32(pDst + MZ_ZIP_LDH_DECOMPRESSED_SIZE_OFS, use_zip64 ? MZ_UINT32_MAX : uncomp_size);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_FILENAME_LEN_OFS, filename_size);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_EXTRA_LEN_OFS, extra_size);
  return MZ_TRUE;
}

static mz_bool mz_zip_writer_create_central_dir_header(mz_zip_archive *pZip, mz_uint8 *pDst, mz_uint16 filename_size, mz_uint16 extra_size, mz_uint16 comment_size, mz_uint64 uncomp_size, mz_uint64 comp_size, mz_uint32 uncomp_crc32, mz_uint16 method, mz_uint16 bit_flags, mz_uint16 dos_time, mz_uint16 dos_date, mz_uint64 local_header_ofs, mz_uint32 disk_index, mz_uint32 ext_attributes, mz_bool force_zip64)
{
  (void)pZip;
  memset(pDst, 0, MZ_ZIP_CENTRAL_DIR_HEADER_SIZE);
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_SIG_OFS, MZ_ZIP_CENTRAL_DIR_HEADER_SIG);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_VERSION_NEEDED_OFS, (method == MZ_ZIP_AES) ? 51 : (method == MZ_LZMA) ? 63 : ((force_zip64 || (uncomp_size >= MZ_UINT32_MAX) || (comp_size >= MZ_UINT32_MAX) || (local_header_ofs >= MZ_UINT32_MAX) || (disk_index >= MZ_UINT16_MAX)) ? 45 : (method ? 20 : 0)));
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_BIT_FLAG_OFS, bit_flags);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_METHOD_OFS, method);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_FILE_TIME_OFS, dos_time);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_FILE_DATE_OFS, dos_date);
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_CRC32_OFS, uncomp_crc32);
  // When a ZIP64 extra field is present for a value, the corresponding base field MUST hold the 0xFFFFFFFF/0xFFFF sentinel.
  // The extra is emitted (by the caller) for a value iff force_zip64 or that value overflows 32 bits, so mirror the same condition here.
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_COMPRESSED_SIZE_OFS, (force_zip64 || (comp_size >= MZ_UINT32_MAX)) ? MZ_UINT32_MAX : comp_size);
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS, (force_zip64 || (uncomp_size >= MZ_UINT32_MAX)) ? MZ_UINT32_MAX : uncomp_size);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_FILENAME_LEN_OFS, filename_size);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_EXTRA_LEN_OFS, extra_size);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_COMMENT_LEN_OFS, comment_size);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_DISK_START_OFS, (disk_index >= MZ_UINT16_MAX) ? MZ_UINT16_MAX : disk_index);
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_EXTERNAL_ATTR_OFS, ext_attributes);
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_LOCAL_HEADER_OFS, (force_zip64 || (local_header_ofs >= MZ_UINT32_MAX)) ? MZ_UINT32_MAX : local_header_ofs);
  return MZ_TRUE;
}

static mz_bool mz_zip_writer_get_disk_relative_ofs(mz_zip_archive *pZip, mz_uint64 archive_ofs, mz_uint32 *pDisk_index, mz_uint64 *pDisk_ofs)
{
  mz_zip_internal_state *pState = pZip->m_pState;
  size_t disk_index = 0;
  if (!pState->m_split_size)
  {
	*pDisk_index = 0;
	*pDisk_ofs = archive_ofs;
	return MZ_TRUE;
  }
  while ((disk_index + 1 < pState->m_disk_offsets.m_size) &&
		 (archive_ofs >= MZ_ZIP_ARRAY_ELEMENT(&pState->m_disk_offsets, mz_uint64, disk_index + 1)))
	++disk_index;
  if ((disk_index >= pState->m_disk_offsets.m_size) || (disk_index > MZ_UINT32_MAX))
	return MZ_FALSE;
  *pDisk_index = (mz_uint32)disk_index;
  *pDisk_ofs = archive_ofs - MZ_ZIP_ARRAY_ELEMENT(&pState->m_disk_offsets, mz_uint64, disk_index);
  return MZ_TRUE;
}

static mz_bool mz_zip_writer_add_to_central_dir(mz_zip_archive *pZip, const char *pFilename, mz_uint16 filename_size, const void *pExtra, mz_uint16 extra_size, const void *pComment, mz_uint16 comment_size, mz_uint64 uncomp_size, mz_uint64 comp_size, mz_uint32 uncomp_crc32, mz_uint16 method, mz_uint16 bit_flags, mz_uint16 dos_time, mz_uint16 dos_date, mz_uint64 local_header_ofs, mz_uint32 ext_attributes, mz_bool force_zip64)
{
  mz_zip_internal_state *pState = pZip->m_pState;
  size_t central_dir_ofs = pState->m_central_dir.m_size;
  size_t orig_central_dir_size = pState->m_central_dir.m_size;
  mz_uint8 central_dir_header[MZ_ZIP_CENTRAL_DIR_HEADER_SIZE];
  mz_uint8 zip64_extra[sizeof(mz_uint16) * 2 + sizeof(mz_uint64) * 3 + sizeof(mz_uint32)];
  mz_uint zip64_extra_size = 0;
  mz_uint total_extra_size = extra_size;
  mz_uint32 disk_index;
  mz_uint64 disk_local_header_ofs;

  if (!mz_zip_writer_get_disk_relative_ofs(pZip, local_header_ofs, &disk_index, &disk_local_header_ofs))
	return MZ_FALSE;

  if (force_zip64 || (uncomp_size >= MZ_UINT32_MAX) || (comp_size >= MZ_UINT32_MAX) || (disk_local_header_ofs >= MZ_UINT32_MAX) || (disk_index >= MZ_UINT16_MAX))
  {
	zip64_extra_size = mz_zip_writer_create_zip64_extra_data(zip64_extra, uncomp_size, comp_size, disk_local_header_ofs, force_zip64 || (uncomp_size >= MZ_UINT32_MAX), force_zip64 || (comp_size >= MZ_UINT32_MAX), force_zip64 || (disk_local_header_ofs >= MZ_UINT32_MAX));
	if (disk_index >= MZ_UINT16_MAX)
	{
	  MZ_WRITE_LE32(zip64_extra + zip64_extra_size, disk_index);
	  zip64_extra_size += sizeof(mz_uint32);
	  MZ_WRITE_LE16(zip64_extra + sizeof(mz_uint16), zip64_extra_size - sizeof(mz_uint16) * 2);
	}
	total_extra_size += zip64_extra_size;
	if (total_extra_size > MZ_UINT16_MAX)
	  return MZ_FALSE;
  }

  if (!mz_zip_writer_create_central_dir_header(pZip, central_dir_header, filename_size, (mz_uint16)total_extra_size, comment_size, uncomp_size, comp_size, uncomp_crc32, method, bit_flags, dos_time, dos_date, disk_local_header_ofs, disk_index, ext_attributes, force_zip64))
	return MZ_FALSE;

  if ((!mz_zip_array_push_back(pZip, &pState->m_central_dir, central_dir_header, MZ_ZIP_CENTRAL_DIR_HEADER_SIZE)) ||
	  (!mz_zip_array_push_back(pZip, &pState->m_central_dir, pFilename, filename_size)) ||
	  (!mz_zip_array_push_back(pZip, &pState->m_central_dir, zip64_extra, zip64_extra_size)) ||
	  (!mz_zip_array_push_back(pZip, &pState->m_central_dir, pExtra, extra_size)) ||
	  (!mz_zip_array_push_back(pZip, &pState->m_central_dir, pComment, comment_size)) ||
	  (!mz_zip_array_push_back(pZip, &pState->m_central_dir_offsets, &central_dir_ofs, 1)))
  {
	// Try to push the central directory array back into its original state.
	mz_zip_array_resize(pZip, &pState->m_central_dir, orig_central_dir_size, MZ_FALSE);
	return MZ_FALSE;
  }

  return MZ_TRUE;
}

static mz_bool mz_zip_writer_validate_archive_name(const char *pArchive_name)
{
  // Basic ZIP archive filename validity checks: Valid filenames cannot start with a forward slash, cannot contain a drive letter, and cannot use DOS-style backward slashes.
  if (*pArchive_name == '/')
	return MZ_FALSE;
  while (*pArchive_name)
  {
	if ((*pArchive_name == '\\') || (*pArchive_name == ':'))
	  return MZ_FALSE;
	pArchive_name++;
  }
  return MZ_TRUE;
}

static mz_uint mz_zip_writer_compute_padding_needed_for_file_alignment(mz_zip_archive *pZip)
{
  mz_uint32 n;
  if (!pZip->m_file_offset_alignment)
	return 0;
  n = (mz_uint32)(pZip->m_archive_size & (pZip->m_file_offset_alignment - 1));
  return (pZip->m_file_offset_alignment - n) & (pZip->m_file_offset_alignment - 1);
}

static mz_bool mz_zip_writer_prepare_header(mz_zip_archive *pZip, mz_uint64 header_size, mz_uint *pPadding)
{
  mz_zip_internal_state *pState = pZip->m_pState;
  mz_uint padding = mz_zip_writer_compute_padding_needed_for_file_alignment(pZip);
#ifdef MINIZ_NO_STDIO
  (void)header_size;
#endif
  if (pState->m_split_size)
  {
#ifndef MINIZ_NO_STDIO
	mz_uint64 disk_base = MZ_ZIP_ARRAY_ELEMENT(&pState->m_disk_offsets, mz_uint64, pState->m_disk_offsets.m_size - 1);
	mz_uint64 disk_used = pZip->m_archive_size - disk_base;
	if ((header_size > pState->m_split_size) || (disk_used > pState->m_split_size))
	  return MZ_FALSE;
	if (((mz_uint64)padding > pState->m_split_size - disk_used) ||
		(header_size > pState->m_split_size - disk_used - padding))
	{
	  if ((!mz_zip_writer_split_close_current_disk(pZip)) || (!mz_zip_writer_split_open_next_disk(pZip, pZip->m_archive_size)))
		return MZ_FALSE;
	  padding = mz_zip_writer_compute_padding_needed_for_file_alignment(pZip);
	  if ((mz_uint64)padding + header_size > pState->m_split_size)
		return MZ_FALSE;
	}
#else
	return MZ_FALSE;
#endif
  }
  *pPadding = padding;
  return MZ_TRUE;
}

static mz_bool mz_zip_writer_write_zeros(mz_zip_archive *pZip, mz_uint64 cur_file_ofs, mz_uint32 n)
{
  char buf[4096];
  memset(buf, 0, MZ_MIN(sizeof(buf), n));
  while (n)
  {
	mz_uint32 s = MZ_MIN(sizeof(buf), n);
	if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_file_ofs, buf, s) != s)
	  return MZ_FALSE;
	cur_file_ofs += s; n -= s;
  }
  return MZ_TRUE;
}

#ifdef MINIZ_USE_AES
// Fills buf with cryptographically strong random bytes. Prefers the OS CSPRNG; the mixed-LCG path is only a
// last-resort fallback for platforms exposing neither (a salt still varies per entry there, but is weaker).
// Returns MZ_TRUE if an OS CSPRNG was used.
static mz_bool mz_zip_aes_random(mz_zip_archive *pZip, mz_uint8 *buf, size_t n, const void *pEntropy, size_t entropy_len)
{
  mz_bool strong = MZ_FALSE;
#if defined(_WIN32)
  {
    /* rand_s (RtlGenRandom) is a CSPRNG and needs no extra link libs. Declare it locally so this works
       regardless of whether the includer defined _CRT_RAND_S before <stdlib.h>. */
    extern int __cdecl rand_s(unsigned int *);
    size_t i = 0;
    while (i + sizeof(unsigned) <= n) { unsigned r; if (rand_s(&r) != 0) break; memcpy(buf + i, &r, sizeof(unsigned)); i += sizeof(unsigned); }
    if (i < n) { unsigned r; if (rand_s(&r) == 0) { while (i < n) { buf[i++] = (mz_uint8)r; r >>= 8; } } }
    strong = (i >= n);
  }
#elif defined(__unix__) || defined(__APPLE__)
  {
    FILE *f = fopen("/dev/urandom", "rb");
    if (f) { strong = (fread(buf, 1, n, f) == n); fclose(f); }
  }
#endif
  if (!strong)
  {
    mz_uint32 s0 = (mz_uint32)(uintptr_t)buf ^ (mz_uint32)n ^ mz_crc32(MZ_CRC32_INIT, (const mz_uint8 *)pEntropy, entropy_len);
    mz_uint32 s1 = (mz_uint32)((uintptr_t)buf >> 16) ^ (mz_uint32)pZip->m_archive_size ^ (mz_uint32)pZip->m_total_files ^ 0x9e3779b9u;
    size_t i;
#ifndef MINIZ_NO_TIME
    s1 ^= (mz_uint32)WSTimeGetLocalValue();
#endif
    for (i = 0; i < n; ++i)
    {
      s0 = s0 * 1664525u + 1013904223u;
      s1 = s1 * 22695477u + 1u;
      buf[i] = (mz_uint8)((s0 >> 24) ^ (s1 >> 16) ^ (mz_uint8)i);
    }
  }
  return strong;
}

// Encrypts a compressed payload in place-into a freshly allocated buffer laid out as:
//   salt || 2-byte pw-verify || AES-CTR(payload) || 10-byte HMAC-SHA1(auth_key, ciphertext).
// Returns the heap buffer (caller frees via pZip->m_pFree) and its size, or NULL on failure.
static mz_uint8 *mz_zip_aes_encrypt_payload(mz_zip_archive *pZip, const char *pPassword, size_t password_len,
    int strength, const mz_uint8 *pComp_data, size_t comp_len, size_t *pOut_size)
{
  size_t salt_len = mz_aes_salt_len(strength);
  size_t key_len = mz_aes_key_len(strength);
  size_t total = salt_len + MZ_AES_PW_VERIFY_LEN + comp_len + MZ_AES_AUTH_CODE_LEN;
  mz_uint8 enc_key[32], auth_key[32], verify[2], auth[MZ_SHA1_DIGEST_SIZE];
  mz_uint8 *pOut, *pSalt, *pCipher;
  mz_hmac_sha1_ctx hm;
  mz_uint i;

  pOut = (mz_uint8 *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, total ? total : 1);
  if (!pOut)
    return NULL;
  pSalt = pOut;
  (void)i;
  mz_zip_aes_random(pZip, pSalt, salt_len, pComp_data, comp_len);
  mz_aes_derive_keys((const mz_uint8 *)pPassword, password_len, pSalt, strength, enc_key, auth_key, verify);
  pOut[salt_len] = verify[0];
  pOut[salt_len + 1] = verify[1];
  pCipher = pOut + salt_len + MZ_AES_PW_VERIFY_LEN;
  if (comp_len)
    memcpy(pCipher, pComp_data, comp_len);
  if (!mz_zip_aes_ctr_xor(pZip, enc_key, key_len, pCipher, comp_len))
  {
    pZip->m_pFree(pZip->m_pAlloc_opaque, pOut);
    return NULL;
  }
  mz_hmac_sha1_init(&hm, auth_key, key_len);
  mz_hmac_sha1_update(&hm, pCipher, comp_len);
  mz_hmac_sha1_final(&hm, auth);
  memcpy(pCipher + comp_len, auth, MZ_AES_AUTH_CODE_LEN);
  *pOut_size = total;
  return pOut;
}

// Builds the 11-byte 0x9901 AES extra field. real_method is 0 (store) or 8 (deflate)/14 (lzma).
static void mz_zip_aes_create_extra(mz_uint8 *pDst, int ae_version, int strength, mz_uint16 real_method)
{
  MZ_WRITE_LE16(pDst, MZ_ZIP_AES_EXTRA_FIELD_HEADER_ID);
  MZ_WRITE_LE16(pDst + 2, MZ_ZIP_AES_EXTRA_DATA_SIZE);
  MZ_WRITE_LE16(pDst + 4, (mz_uint16)ae_version);
  pDst[6] = 'A'; pDst[7] = 'E';
  pDst[8] = (mz_uint8)strength;
  MZ_WRITE_LE16(pDst + 9, real_method);
}

// Full encrypted add. Compresses pBuf to a heap buffer, encrypts it, and writes a method-99 entry.
static mz_bool mz_zip_writer_add_mem_encrypted(mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf, size_t buf_size,
    const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags,
    const char *pPassword, mz_uint aes_strength, mz_uint ae_version)
{
  mz_zip_internal_state *pState = pZip->m_pState;
  mz_uint level = level_and_flags & 0xF;
  mz_bool use_lzma = (level_and_flags & MZ_ZIP_FLAG_LZMA) != 0;
  mz_bool force_zip64 = (level_and_flags & MZ_ZIP_FLAG_WRITE_ZIP64) != 0;
  mz_bool store_uncompressed;
  int strength = aes_strength ? (int)aes_strength : 3;
  int aever = ae_version ? (int)ae_version : 2;
  size_t archive_name_size, password_len, comp_len = 0, enc_size = 0;
  mz_uint16 real_method = 0, dos_time = 0, dos_date = 0, method = MZ_ZIP_AES, bit_flag = 1; // bit 0 = encrypted
  mz_uint16 ext_attributes = 0;
  mz_uint num_alignment_padding_bytes, local_zip64_extra_size = 0;
  mz_uint64 local_dir_header_ofs, cur_archive_file_ofs, uncomp_size = buf_size, comp_size;
  mz_uint32 uncomp_crc32;
  mz_uint8 local_dir_header[MZ_ZIP_LOCAL_DIR_HEADER_SIZE];
  mz_uint8 local_zip64_extra[sizeof(mz_uint16) * 2 + sizeof(mz_uint64) * 2];
  mz_uint8 aes_extra[MZ_ZIP_AES_EXTRA_FIELD_SIZE];
  mz_uint8 *pComp_data = NULL, *pEnc = NULL;
  mz_bool use_zip64_local_header;

  if ((strength < 1) || (strength > 3) || (aever < 1) || (aever > 2))
    return MZ_FALSE;
  if (!pPassword) { pPassword = pState->m_pPassword; }
  if (!pPassword) return MZ_FALSE;
  password_len = strlen(pPassword);
  archive_name_size = strlen(pArchive_name);
  if (archive_name_size > 0xFFFF)
    return MZ_FALSE;
  if ((archive_name_size) && (pArchive_name[archive_name_size - 1] == '/') && (buf_size))
    return MZ_FALSE;
  if (!mz_zip_writer_validate_archive_name(pArchive_name))
    return MZ_FALSE;

#ifndef MINIZ_NO_TIME
  { mz_time cur_time = WSTimeGetLocalValue(); mz_zip_time_to_dos_time(cur_time, &dos_time, &dos_date); }
#endif

  uncomp_crc32 = (mz_uint32)mz_crc32(MZ_CRC32_INIT, (const mz_uint8 *)pBuf, buf_size);

  store_uncompressed = (!level && !use_lzma);
  if (buf_size <= 3) store_uncompressed = MZ_TRUE;

  // Produce the compressed (pre-encryption) payload into pComp_data / comp_len.
  if (store_uncompressed || !buf_size)
  {
    real_method = 0;
    comp_len = buf_size;
    if (buf_size)
    {
      pComp_data = (mz_uint8 *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, buf_size);
      if (!pComp_data) return MZ_FALSE;
      memcpy(pComp_data, pBuf, buf_size);
    }
  }
#ifdef MINIZ_USE_LZMA_SDK
  else if (use_lzma)
  {
    mz_zip_array lzma_data; MZ_CLEAR_OBJ(lzma_data); MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&lzma_data, sizeof(mz_uint8));
    if (!mz_zip_lzma_compress(pZip, pBuf, buf_size, level, &lzma_data)) { mz_zip_array_clear(pZip, &lzma_data); return MZ_FALSE; }
    real_method = MZ_LZMA;
    comp_len = lzma_data.m_size;
    pComp_data = (mz_uint8 *)lzma_data.m_p; // hand ownership over; freed below via m_pFree
  }
#endif
  else
  {
    size_t out_len = 0;
    void *p = tdefl_compress_mem_to_heap(pBuf, buf_size, &out_len, tdefl_create_comp_flags_from_zip_params(level, -15, MZ_DEFAULT_STRATEGY));
    if (!p) return MZ_FALSE;
    real_method = MZ_DEFLATED;
    comp_len = out_len;
    pComp_data = (mz_uint8 *)p;
  }

  pEnc = mz_zip_aes_encrypt_payload(pZip, pPassword, password_len, strength, pComp_data ? pComp_data : (const mz_uint8 *)"", comp_len, &enc_size);
  if (pComp_data) pZip->m_pFree(pZip->m_pAlloc_opaque, pComp_data);
  if (!pEnc) return MZ_FALSE;
  comp_size = enc_size;

  // Layout / headers. AE-2 stores CRC 0; AE-1 stores the real CRC.
  mz_zip_aes_create_extra(aes_extra, aever, strength, real_method);

  use_zip64_local_header = force_zip64 || (uncomp_size >= MZ_UINT32_MAX) || (comp_size >= MZ_UINT32_MAX);
  if (use_zip64_local_header)
    local_zip64_extra_size = sizeof(mz_uint16) * 2 + sizeof(mz_uint64) * 2;

  if (!mz_zip_writer_prepare_header(pZip, MZ_ZIP_LOCAL_DIR_HEADER_SIZE + archive_name_size + local_zip64_extra_size + MZ_ZIP_AES_EXTRA_FIELD_SIZE, &num_alignment_padding_bytes))
    { pZip->m_pFree(pZip->m_pAlloc_opaque, pEnc); return MZ_FALSE; }
  local_dir_header_ofs = cur_archive_file_ofs = pZip->m_archive_size;

  if ((!mz_zip_array_ensure_room(pZip, &pState->m_central_dir, MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + archive_name_size + comment_size + local_zip64_extra_size + MZ_ZIP_AES_EXTRA_FIELD_SIZE + sizeof(mz_uint64) * 3)) ||
      (!mz_zip_array_ensure_room(pZip, &pState->m_central_dir_offsets, 1)))
    { pZip->m_pFree(pZip->m_pAlloc_opaque, pEnc); return MZ_FALSE; }

  if (!mz_zip_writer_write_zeros(pZip, cur_archive_file_ofs, num_alignment_padding_bytes + sizeof(local_dir_header)))
    { pZip->m_pFree(pZip->m_pAlloc_opaque, pEnc); return MZ_FALSE; }
  local_dir_header_ofs += num_alignment_padding_bytes;
  cur_archive_file_ofs += num_alignment_padding_bytes + sizeof(local_dir_header);

  // filename
  if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pArchive_name, archive_name_size) != archive_name_size)
    { pZip->m_pFree(pZip->m_pAlloc_opaque, pEnc); return MZ_FALSE; }
  cur_archive_file_ofs += archive_name_size;

  // local zip64 extra (zeros for now, filled below) then AES extra
  if (local_zip64_extra_size)
  {
    mz_zip_writer_create_zip64_extra_data(local_zip64_extra, uncomp_size, comp_size, 0, MZ_TRUE, MZ_TRUE, MZ_FALSE);
    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, local_zip64_extra, local_zip64_extra_size) != local_zip64_extra_size)
      { pZip->m_pFree(pZip->m_pAlloc_opaque, pEnc); return MZ_FALSE; }
    cur_archive_file_ofs += local_zip64_extra_size;
  }
  if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, aes_extra, MZ_ZIP_AES_EXTRA_FIELD_SIZE) != MZ_ZIP_AES_EXTRA_FIELD_SIZE)
    { pZip->m_pFree(pZip->m_pAlloc_opaque, pEnc); return MZ_FALSE; }
  cur_archive_file_ofs += MZ_ZIP_AES_EXTRA_FIELD_SIZE;

  // encrypted payload
  if (comp_size && (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pEnc, (size_t)comp_size) != comp_size))
    { pZip->m_pFree(pZip->m_pAlloc_opaque, pEnc); return MZ_FALSE; }
  cur_archive_file_ofs += comp_size;
  pZip->m_pFree(pZip->m_pAlloc_opaque, pEnc);

  // Write the local dir header. method=99, extra = zip64 extra + AES extra. CRC per AE version.
  if (!mz_zip_writer_create_local_dir_header(pZip, local_dir_header, (mz_uint16)archive_name_size,
        (mz_uint16)(local_zip64_extra_size + MZ_ZIP_AES_EXTRA_FIELD_SIZE), uncomp_size, comp_size,
        (aever == 1) ? uncomp_crc32 : 0, method, bit_flag, dos_time, dos_date, use_zip64_local_header))
    return MZ_FALSE;
  if (pZip->m_pWrite(pZip->m_pIO_opaque, local_dir_header_ofs, local_dir_header, sizeof(local_dir_header)) != sizeof(local_dir_header))
    return MZ_FALSE;

  // Central dir: hand the AES extra to add_to_central_dir as pExtra so it is appended after any zip64 extra.
  if (!mz_zip_writer_add_to_central_dir(pZip, pArchive_name, (mz_uint16)archive_name_size, aes_extra, MZ_ZIP_AES_EXTRA_FIELD_SIZE,
        pComment, comment_size, uncomp_size, comp_size, (aever == 1) ? uncomp_crc32 : 0, method, bit_flag, dos_time, dos_date,
        local_dir_header_ofs, ext_attributes, force_zip64))
    return MZ_FALSE;

  pZip->m_total_files++;
  pZip->m_archive_size = cur_archive_file_ofs;
  return MZ_TRUE;
}
#endif // MINIZ_USE_AES

mz_bool mz_zip_writer_add_mem_ex(mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf, size_t buf_size, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags, mz_uint64 uncomp_size, mz_uint32 uncomp_crc32)
{
  mz_uint16 method = 0, dos_time = 0, dos_date = 0;
  mz_uint level, ext_attributes = 0, num_alignment_padding_bytes;
  mz_uint64 local_dir_header_ofs = pZip->m_archive_size, cur_archive_file_ofs = pZip->m_archive_size, comp_size = 0;
  size_t archive_name_size;
  mz_uint8 local_dir_header[MZ_ZIP_LOCAL_DIR_HEADER_SIZE];
  mz_uint8 local_zip64_extra[sizeof(mz_uint16) * 2 + sizeof(mz_uint64) * 2];
  mz_uint local_zip64_extra_size = 0;
  tdefl_compressor *pComp = NULL;
  mz_bool store_data_uncompressed, use_lzma, use_zip64_local_header = MZ_FALSE, force_zip64;
  mz_zip_internal_state *pState;

  if ((int)level_and_flags < 0)
	level_and_flags = MZ_DEFAULT_LEVEL;
  level = level_and_flags & 0xF;
  use_lzma = (level_and_flags & MZ_ZIP_FLAG_LZMA) != 0;
  force_zip64 = (level_and_flags & MZ_ZIP_FLAG_WRITE_ZIP64) != 0;
  store_data_uncompressed = (((!level) && (!use_lzma)) || (level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA));

  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING) || ((buf_size) && (!pBuf)) || (!pArchive_name) || ((comment_size) && (!pComment)) || (pZip->m_total_files == (mz_uint)-1) || (level > MZ_UBER_COMPRESSION))
	return MZ_FALSE;
#ifdef MINIZ_USE_AES
  if (level_and_flags & MZ_ZIP_FLAG_ENCRYPT)
  {
	// Encryption cannot combine with pre-compressed input (we must compress-then-encrypt ourselves).
	if (level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA)
	  return MZ_FALSE;
	return mz_zip_writer_add_mem_encrypted(pZip, pArchive_name, pBuf, buf_size, pComment, comment_size, level_and_flags, NULL, 0, 0);
  }
#else
  if (level_and_flags & MZ_ZIP_FLAG_ENCRYPT)
	return MZ_FALSE;
#endif
#ifndef MINIZ_USE_LZMA_SDK
  if (use_lzma && !(level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA))
	return MZ_FALSE;
#endif

  pState = pZip->m_pState;

  if ((!(level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA)) && (uncomp_size))
	return MZ_FALSE;
  if (!mz_zip_writer_validate_archive_name(pArchive_name))
	return MZ_FALSE;

#ifndef MINIZ_NO_TIME
  {
	mz_time cur_time = WSTimeGetLocalValue();
	mz_zip_time_to_dos_time(cur_time, &dos_time, &dos_date);
  }
#endif // #ifndef MINIZ_NO_TIME

  archive_name_size = strlen(pArchive_name);
  if (archive_name_size > 0xFFFF)
	return MZ_FALSE;

  if ((archive_name_size) && (pArchive_name[archive_name_size - 1] == '/'))
  {
	// Set DOS Subdirectory attribute bit.
	ext_attributes |= 0x10;
	// Subdirectories cannot contain data.
	if ((buf_size) || (uncomp_size))
	  return MZ_FALSE;
  }

  if (!(level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA))
  {
	uncomp_crc32 = (mz_uint32)mz_crc32(MZ_CRC32_INIT, (const mz_uint8*)pBuf, buf_size);
	uncomp_size = buf_size;
	if (uncomp_size <= 3)
	{
	  level = 0;
	  store_data_uncompressed = MZ_TRUE;
	}
  }

  use_zip64_local_header = force_zip64 || (uncomp_size >= MZ_UINT32_MAX) || (store_data_uncompressed && (buf_size >= MZ_UINT32_MAX)) ||
	((level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA) && (buf_size >= MZ_UINT32_MAX)) ||
	((!store_data_uncompressed) && mz_zip_writer_may_need_zip64_compressed_size(uncomp_size));
  if (use_zip64_local_header)
	local_zip64_extra_size = sizeof(mz_uint16) * 2 + sizeof(mz_uint64) * 2;

  if (!mz_zip_writer_prepare_header(pZip, MZ_ZIP_LOCAL_DIR_HEADER_SIZE + archive_name_size + local_zip64_extra_size, &num_alignment_padding_bytes))
	return MZ_FALSE;
  local_dir_header_ofs = cur_archive_file_ofs = pZip->m_archive_size;

  // Try to do any allocations before writing to the archive, so if an allocation fails the file remains unmodified. (A good idea if we're doing an in-place modification.)
  if ((!mz_zip_array_ensure_room(pZip, &pState->m_central_dir, MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + archive_name_size + comment_size + sizeof(mz_uint16) * 2 + sizeof(mz_uint64) * 3)) || (!mz_zip_array_ensure_room(pZip, &pState->m_central_dir_offsets, 1)))
	return MZ_FALSE;

  if ((!store_data_uncompressed) && (!use_lzma) && (buf_size))
  {
	if (NULL == (pComp = (tdefl_compressor *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, sizeof(tdefl_compressor))))
	  return MZ_FALSE;
  }

  if (!mz_zip_writer_write_zeros(pZip, cur_archive_file_ofs, num_alignment_padding_bytes + sizeof(local_dir_header)))
  {
	pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
	return MZ_FALSE;
  }
  local_dir_header_ofs += num_alignment_padding_bytes;
  if (pZip->m_file_offset_alignment) { MZ_ASSERT((local_dir_header_ofs & (pZip->m_file_offset_alignment - 1)) == 0); }
  cur_archive_file_ofs += num_alignment_padding_bytes + sizeof(local_dir_header);

  MZ_CLEAR_OBJ(local_dir_header);
  if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pArchive_name, archive_name_size) != archive_name_size)
  {
	pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
	return MZ_FALSE;
  }
  cur_archive_file_ofs += archive_name_size;

  if (local_zip64_extra_size)
  {
	if (!mz_zip_writer_write_zeros(pZip, cur_archive_file_ofs, local_zip64_extra_size))
	{
	  pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
	  return MZ_FALSE;
	}
	cur_archive_file_ofs += local_zip64_extra_size;
  }

  if (store_data_uncompressed)
  {
	if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pBuf, buf_size) != buf_size)
	{
	  pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
	  return MZ_FALSE;
	}

	cur_archive_file_ofs += buf_size;
	comp_size = buf_size;

	if (level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA)
	  method = use_lzma ? MZ_LZMA : MZ_DEFLATED;
  }
#ifdef MINIZ_USE_LZMA_SDK
  else if (use_lzma)
  {
	mz_zip_array lzma_data;
	MZ_CLEAR_OBJ(lzma_data);
	MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&lzma_data, sizeof(mz_uint8));
	if ((!mz_zip_lzma_compress(pZip, pBuf, buf_size, level, &lzma_data)) ||
		(pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, lzma_data.m_p, lzma_data.m_size) != lzma_data.m_size))
	{
	  mz_zip_array_clear(pZip, &lzma_data);
	  return MZ_FALSE;
	}
	comp_size = lzma_data.m_size;
	cur_archive_file_ofs += comp_size;
	method = MZ_LZMA;
	mz_zip_array_clear(pZip, &lzma_data);
  }
#endif
  else if (buf_size)
  {
	mz_zip_writer_add_state state;

	state.m_pZip = pZip;
	state.m_cur_archive_file_ofs = cur_archive_file_ofs;
	state.m_comp_size = 0;

	if ((tdefl_init(pComp, mz_zip_writer_add_put_buf_callback, &state, tdefl_create_comp_flags_from_zip_params(level, -15, MZ_DEFAULT_STRATEGY)) != TDEFL_STATUS_OKAY) ||
		(tdefl_compress_buffer(pComp, pBuf, buf_size, TDEFL_FINISH) != TDEFL_STATUS_DONE))
	{
	  pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
	  return MZ_FALSE;
	}

	comp_size = state.m_comp_size;
	cur_archive_file_ofs = state.m_cur_archive_file_ofs;

	method = MZ_DEFLATED;
  }

  pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
  pComp = NULL;

  if ((!use_zip64_local_header) && (comp_size >= MZ_UINT32_MAX))
	return MZ_FALSE;

  if (local_zip64_extra_size)
	mz_zip_writer_create_zip64_extra_data(local_zip64_extra, uncomp_size, comp_size, 0, MZ_TRUE, MZ_TRUE, MZ_FALSE);

  if (!mz_zip_writer_create_local_dir_header(pZip, local_dir_header, (mz_uint16)archive_name_size, (mz_uint16)local_zip64_extra_size, uncomp_size, comp_size, uncomp_crc32, method, 0, dos_time, dos_date, use_zip64_local_header))
	return MZ_FALSE;

  if (pZip->m_pWrite(pZip->m_pIO_opaque, local_dir_header_ofs, local_dir_header, sizeof(local_dir_header)) != sizeof(local_dir_header))
	return MZ_FALSE;
  if (local_zip64_extra_size)
  {
	if (pZip->m_pWrite(pZip->m_pIO_opaque, local_dir_header_ofs + MZ_ZIP_LOCAL_DIR_HEADER_SIZE + archive_name_size, local_zip64_extra, local_zip64_extra_size) != local_zip64_extra_size)
	  return MZ_FALSE;
  }

  if (!mz_zip_writer_add_to_central_dir(pZip, pArchive_name, (mz_uint16)archive_name_size, NULL, 0, pComment, comment_size, uncomp_size, comp_size, uncomp_crc32, method, 0, dos_time, dos_date, local_dir_header_ofs, ext_attributes, force_zip64))
	return MZ_FALSE;

  pZip->m_total_files++;
  pZip->m_archive_size = cur_archive_file_ofs;

  return MZ_TRUE;
}

mz_bool mz_zip_writer_set_password(mz_zip_archive *pZip, const char *pPassword, mz_uint aes_strength, mz_uint ae_version)
{
  mz_zip_internal_state *pState;
  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING))
	return MZ_FALSE;
  if ((aes_strength > 3) || (ae_version > 2))
	return MZ_FALSE;
  pState = pZip->m_pState;
  if (pState->m_pPassword)
  {
	pZip->m_pFree(pZip->m_pAlloc_opaque, pState->m_pPassword);
	pState->m_pPassword = NULL;
	pState->m_password_len = 0;
  }
  if (pPassword)
  {
	size_t n = strlen(pPassword);
	pState->m_pPassword = (char *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, n + 1);
	if (!pState->m_pPassword)
	  return MZ_FALSE;
	memcpy(pState->m_pPassword, pPassword, n + 1);
	pState->m_password_len = n;
  }
  pState->m_aes_strength = (mz_uint8)(aes_strength ? aes_strength : 3);
  pState->m_aes_ae_version = (mz_uint8)(ae_version ? ae_version : 2);
  return MZ_TRUE;
}

mz_bool mz_zip_writer_add_mem_ex_v2(mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf, size_t buf_size, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags, mz_uint64 uncomp_size, mz_uint32 uncomp_crc32, const char *pPassword, mz_uint aes_strength, mz_uint ae_version)
{
#ifdef MINIZ_USE_AES
  if (level_and_flags & MZ_ZIP_FLAG_ENCRYPT)
  {
	mz_zip_internal_state *pState;
	if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING) || ((buf_size) && (!pBuf)) || (!pArchive_name) || ((comment_size) && (!pComment)))
	  return MZ_FALSE;
	if (level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA)
	  return MZ_FALSE;
	pState = pZip->m_pState;
	if (!aes_strength) aes_strength = pState->m_aes_strength ? pState->m_aes_strength : 3;
	if (!ae_version)   ae_version   = pState->m_aes_ae_version ? pState->m_aes_ae_version : 2;
	return mz_zip_writer_add_mem_encrypted(pZip, pArchive_name, pBuf, buf_size, pComment, comment_size, level_and_flags, pPassword, aes_strength, ae_version);
  }
#else
  (void)pPassword; (void)aes_strength; (void)ae_version;
  if (level_and_flags & MZ_ZIP_FLAG_ENCRYPT)
	return MZ_FALSE;
#endif
  return mz_zip_writer_add_mem_ex(pZip, pArchive_name, pBuf, buf_size, pComment, comment_size, level_and_flags, uncomp_size, uncomp_crc32);
}

#ifndef MINIZ_NO_STDIO
mz_bool mz_zip_writer_add_file(mz_zip_archive *pZip, const char *pArchive_name, const char *pSrc_filename, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags)
{
  mz_uint uncomp_crc32 = MZ_CRC32_INIT, level, num_alignment_padding_bytes;
  mz_uint16 method = 0, dos_time = 0, dos_date = 0, ext_attributes = 0;
  mz_uint64 local_dir_header_ofs = pZip->m_archive_size, cur_archive_file_ofs = pZip->m_archive_size, uncomp_size = 0, comp_size = 0;
  size_t archive_name_size;
  mz_uint8 local_dir_header[MZ_ZIP_LOCAL_DIR_HEADER_SIZE];
  mz_uint8 local_zip64_extra[sizeof(mz_uint16) * 2 + sizeof(mz_uint64) * 2];
  mz_uint local_zip64_extra_size = 0;
  mz_bool use_zip64_local_header = MZ_FALSE, force_zip64;
  MZ_FILE *pSrc_file = NULL;

  if ((int)level_and_flags < 0)
	level_and_flags = MZ_DEFAULT_LEVEL;
  level = level_and_flags & 0xF;
  force_zip64 = (level_and_flags & MZ_ZIP_FLAG_WRITE_ZIP64) != 0;

  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING) || (!pArchive_name) || ((comment_size) && (!pComment)) || (pZip->m_total_files == (mz_uint)-1) || (level > MZ_UBER_COMPRESSION))
	return MZ_FALSE;
  if (level_and_flags & (MZ_ZIP_FLAG_COMPRESSED_DATA | MZ_ZIP_FLAG_LZMA))
	return MZ_FALSE;
  if (!mz_zip_writer_validate_archive_name(pArchive_name))
	return MZ_FALSE;

  archive_name_size = strlen(pArchive_name);
  if (archive_name_size > 0xFFFF)
	return MZ_FALSE;

  if (!mz_zip_get_file_modified_time(pSrc_filename, &dos_time, &dos_date))
	return MZ_FALSE;

  pSrc_file = MZ_FOPEN(pSrc_filename, "rb");
  if (!pSrc_file)
	return MZ_FALSE;
  MZ_FSEEK64(pSrc_file, 0, SEEK_END);
  uncomp_size = MZ_FTELL64(pSrc_file);
  MZ_FSEEK64(pSrc_file, 0, SEEK_SET);

  if (uncomp_size <= 3)
	level = 0;
  use_zip64_local_header = force_zip64 || (uncomp_size >= MZ_UINT32_MAX) || ((level) && mz_zip_writer_may_need_zip64_compressed_size(uncomp_size));
  if (use_zip64_local_header)
	local_zip64_extra_size = sizeof(mz_uint16) * 2 + sizeof(mz_uint64) * 2;

  if (!mz_zip_writer_prepare_header(pZip, MZ_ZIP_LOCAL_DIR_HEADER_SIZE + archive_name_size + local_zip64_extra_size, &num_alignment_padding_bytes))
  {
	MZ_FCLOSE(pSrc_file);
	return MZ_FALSE;
  }
  local_dir_header_ofs = cur_archive_file_ofs = pZip->m_archive_size;

  if (!mz_zip_writer_write_zeros(pZip, cur_archive_file_ofs, num_alignment_padding_bytes + sizeof(local_dir_header)))
  {
	MZ_FCLOSE(pSrc_file);
	return MZ_FALSE;
  }
  local_dir_header_ofs += num_alignment_padding_bytes;
  if (pZip->m_file_offset_alignment) { MZ_ASSERT((local_dir_header_ofs & (pZip->m_file_offset_alignment - 1)) == 0); }
  cur_archive_file_ofs += num_alignment_padding_bytes + sizeof(local_dir_header);

  MZ_CLEAR_OBJ(local_dir_header);
  if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pArchive_name, archive_name_size) != archive_name_size)
  {
	MZ_FCLOSE(pSrc_file);
	return MZ_FALSE;
  }
  cur_archive_file_ofs += archive_name_size;

  if (local_zip64_extra_size)
  {
	if (!mz_zip_writer_write_zeros(pZip, cur_archive_file_ofs, local_zip64_extra_size))
	{
	  MZ_FCLOSE(pSrc_file);
	  return MZ_FALSE;
	}
	cur_archive_file_ofs += local_zip64_extra_size;
  }

  if (uncomp_size)
  {
	mz_uint64 uncomp_remaining = uncomp_size;
	void *pRead_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, MZ_ZIP_MAX_IO_BUF_SIZE);
	if (!pRead_buf)
	{
	  MZ_FCLOSE(pSrc_file);
	  return MZ_FALSE;
	}

	if (!level)
	{
	  while (uncomp_remaining)
	  {
		mz_uint n = (mz_uint)MZ_MIN(MZ_ZIP_MAX_IO_BUF_SIZE, uncomp_remaining);
		if ((MZ_FREAD(pRead_buf, 1, n, pSrc_file) != n) || (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pRead_buf, n) != n))
		{
		  pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
		  MZ_FCLOSE(pSrc_file);
		  return MZ_FALSE;
		}
		uncomp_crc32 = (mz_uint32)mz_crc32(uncomp_crc32, (const mz_uint8 *)pRead_buf, n);
		uncomp_remaining -= n;
		cur_archive_file_ofs += n;
	  }
	  comp_size = uncomp_size;
	}
	else
	{
	  mz_bool result = MZ_FALSE;
	  mz_zip_writer_add_state state;
	  tdefl_compressor *pComp = (tdefl_compressor *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, sizeof(tdefl_compressor));
	  if (!pComp)
	  {
		pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
		MZ_FCLOSE(pSrc_file);
		return MZ_FALSE;
	  }

	  state.m_pZip = pZip;
	  state.m_cur_archive_file_ofs = cur_archive_file_ofs;
	  state.m_comp_size = 0;

	  if (tdefl_init(pComp, mz_zip_writer_add_put_buf_callback, &state, tdefl_create_comp_flags_from_zip_params(level, -15, MZ_DEFAULT_STRATEGY)) != TDEFL_STATUS_OKAY)
	  {
		pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
		pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
		MZ_FCLOSE(pSrc_file);
		return MZ_FALSE;
	  }

	  for ( ; ; )
	  {
		size_t in_buf_size = (mz_uint32)MZ_MIN(uncomp_remaining, MZ_ZIP_MAX_IO_BUF_SIZE);
		tdefl_status status;

		if (MZ_FREAD(pRead_buf, 1, in_buf_size, pSrc_file) != in_buf_size)
		  break;

		uncomp_crc32 = (mz_uint32)mz_crc32(uncomp_crc32, (const mz_uint8 *)pRead_buf, in_buf_size);
		uncomp_remaining -= in_buf_size;

		status = tdefl_compress_buffer(pComp, pRead_buf, in_buf_size, uncomp_remaining ? TDEFL_NO_FLUSH : TDEFL_FINISH);
		if (status == TDEFL_STATUS_DONE)
		{
		  result = MZ_TRUE;
		  break;
		}
		else if (status != TDEFL_STATUS_OKAY)
		  break;
	  }

	  pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);

	  if (!result)
	  {
		pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
		MZ_FCLOSE(pSrc_file);
		return MZ_FALSE;
	  }

	  comp_size = state.m_comp_size;
	  cur_archive_file_ofs = state.m_cur_archive_file_ofs;

	  method = MZ_DEFLATED;
	}

	pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
  }

  MZ_FCLOSE(pSrc_file); pSrc_file = NULL;

  if ((!use_zip64_local_header) && (comp_size >= MZ_UINT32_MAX))
	return MZ_FALSE;

  if (local_zip64_extra_size)
	mz_zip_writer_create_zip64_extra_data(local_zip64_extra, uncomp_size, comp_size, 0, MZ_TRUE, MZ_TRUE, MZ_FALSE);

  if (!mz_zip_writer_create_local_dir_header(pZip, local_dir_header, (mz_uint16)archive_name_size, (mz_uint16)local_zip64_extra_size, uncomp_size, comp_size, uncomp_crc32, method, 0, dos_time, dos_date, use_zip64_local_header))
	return MZ_FALSE;

  if (pZip->m_pWrite(pZip->m_pIO_opaque, local_dir_header_ofs, local_dir_header, sizeof(local_dir_header)) != sizeof(local_dir_header))
	return MZ_FALSE;
  if (local_zip64_extra_size)
  {
	if (pZip->m_pWrite(pZip->m_pIO_opaque, local_dir_header_ofs + MZ_ZIP_LOCAL_DIR_HEADER_SIZE + archive_name_size, local_zip64_extra, local_zip64_extra_size) != local_zip64_extra_size)
	  return MZ_FALSE;
  }

  if (!mz_zip_writer_add_to_central_dir(pZip, pArchive_name, (mz_uint16)archive_name_size, NULL, 0, pComment, comment_size, uncomp_size, comp_size, uncomp_crc32, method, 0, dos_time, dos_date, local_dir_header_ofs, ext_attributes, force_zip64))
	return MZ_FALSE;

  pZip->m_total_files++;
  pZip->m_archive_size = cur_archive_file_ofs;

  return MZ_TRUE;
}
#endif // #ifndef MINIZ_NO_STDIO

static mz_bool mz_zip_writer_copy_extra_without_zip64(mz_zip_archive *pZip, mz_zip_array *pDst, const mz_uint8 *pExtra, mz_uint extra_size)
{
  while (extra_size)
  {
	mz_uint field_size;
	if (extra_size < sizeof(mz_uint16) * 2)
	  return MZ_FALSE;
	field_size = sizeof(mz_uint16) * 2 + MZ_READ_LE16(pExtra + sizeof(mz_uint16));
	if (field_size > extra_size)
	  return MZ_FALSE;
	if (MZ_READ_LE16(pExtra) != MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID)
	{
	  if (!mz_zip_array_push_back(pZip, pDst, pExtra, field_size))
		return MZ_FALSE;
	}
	pExtra += field_size;
	extra_size -= field_size;
  }
  return MZ_TRUE;
}

mz_bool mz_zip_writer_add_from_zip_reader(mz_zip_archive *pZip, mz_zip_archive *pSource_zip, mz_uint file_index)
{
  mz_uint n, bit_flags, num_alignment_padding_bytes, descriptor_size = 0;
  mz_uint filename_size, extra_size, comment_size;
  mz_uint64 bytes_remaining, local_dir_header_ofs, local_record_size, data_end_ofs;
  mz_uint64 cur_src_file_ofs, cur_dst_file_ofs;
  mz_uint32 local_header_u32[(MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof(mz_uint32) - 1) / sizeof(mz_uint32)]; mz_uint8 *pLocal_header = (mz_uint8 *)local_header_u32;
  mz_uint8 descriptor[sizeof(mz_uint32) * 2 + sizeof(mz_uint64) * 2];
  size_t orig_central_dir_size, orig_central_dir_offsets_size;
  mz_zip_internal_state *pState;
  void *pBuf; const mz_uint8 *pSrc_central_header;
  const mz_uint8 *pSrc_filename, *pSrc_extra, *pSrc_comment;
  mz_zip_archive_file_stat src_file_stat;
  mz_zip_array filtered_extra;

  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING))
	return MZ_FALSE;
  if (NULL == (pSrc_central_header = mz_zip_reader_get_cdh(pSource_zip, file_index)))
	return MZ_FALSE;
  if (!mz_zip_reader_file_stat(pSource_zip, file_index, &src_file_stat))
	return MZ_FALSE;
  if (pZip->m_total_files == (mz_uint)-1)
	return MZ_FALSE;
  pState = pZip->m_pState;

  cur_src_file_ofs = src_file_stat.m_local_header_ofs;
  cur_dst_file_ofs = pZip->m_archive_size;

  if (pSource_zip->m_pRead(pSource_zip->m_pIO_opaque, cur_src_file_ofs, pLocal_header, MZ_ZIP_LOCAL_DIR_HEADER_SIZE) != MZ_ZIP_LOCAL_DIR_HEADER_SIZE)
	return MZ_FALSE;
  if (MZ_READ_LE32(pLocal_header) != MZ_ZIP_LOCAL_DIR_HEADER_SIG)
	return MZ_FALSE;
  local_record_size = MZ_ZIP_LOCAL_DIR_HEADER_SIZE +
	MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_FILENAME_LEN_OFS) +
	MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_EXTRA_LEN_OFS);
  if (!mz_zip_writer_prepare_header(pZip, local_record_size, &num_alignment_padding_bytes))
	return MZ_FALSE;
  cur_dst_file_ofs = pZip->m_archive_size;
  if ((local_record_size > ((mz_uint64)-1 - src_file_stat.m_comp_size)) ||
	  (src_file_stat.m_local_header_ofs > ((mz_uint64)-1 - local_record_size - src_file_stat.m_comp_size)))
	return MZ_FALSE;
  data_end_ofs = src_file_stat.m_local_header_ofs + local_record_size + src_file_stat.m_comp_size;
  bit_flags = MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_BIT_FLAG_OFS);
  if (bit_flags & 8)
  {
	if ((data_end_ofs > pSource_zip->m_archive_size) ||
		(pSource_zip->m_archive_size - data_end_ofs < sizeof(descriptor)) ||
		(pSource_zip->m_pRead(pSource_zip->m_pIO_opaque, data_end_ofs, descriptor, sizeof(descriptor)) != sizeof(descriptor)))
	  return MZ_FALSE;
	if ((MZ_READ_LE32(descriptor) == 0x08074b50) &&
		(MZ_READ_LE32(descriptor + sizeof(mz_uint32)) == src_file_stat.m_crc32) &&
		(MZ_READ_LE64(descriptor + sizeof(mz_uint32) * 2) == src_file_stat.m_comp_size) &&
		(MZ_READ_LE64(descriptor + sizeof(mz_uint32) * 2 + sizeof(mz_uint64)) == src_file_stat.m_uncomp_size))
	  descriptor_size = 24;
	else if ((MZ_READ_LE32(descriptor) == src_file_stat.m_crc32) &&
			 (MZ_READ_LE64(descriptor + sizeof(mz_uint32)) == src_file_stat.m_comp_size) &&
			 (MZ_READ_LE64(descriptor + sizeof(mz_uint32) + sizeof(mz_uint64)) == src_file_stat.m_uncomp_size))
	  descriptor_size = 20;
	else if ((src_file_stat.m_comp_size < MZ_UINT32_MAX) && (src_file_stat.m_uncomp_size < MZ_UINT32_MAX) &&
			 (MZ_READ_LE32(descriptor) == 0x08074b50) &&
			 (MZ_READ_LE32(descriptor + sizeof(mz_uint32)) == src_file_stat.m_crc32) &&
			 (MZ_READ_LE32(descriptor + sizeof(mz_uint32) * 2) == src_file_stat.m_comp_size) &&
			 (MZ_READ_LE32(descriptor + sizeof(mz_uint32) * 3) == src_file_stat.m_uncomp_size))
	  descriptor_size = 16;
	else if ((src_file_stat.m_comp_size < MZ_UINT32_MAX) && (src_file_stat.m_uncomp_size < MZ_UINT32_MAX) &&
			 (MZ_READ_LE32(descriptor) == src_file_stat.m_crc32) &&
			 (MZ_READ_LE32(descriptor + sizeof(mz_uint32)) == src_file_stat.m_comp_size) &&
			 (MZ_READ_LE32(descriptor + sizeof(mz_uint32) * 2) == src_file_stat.m_uncomp_size))
	  descriptor_size = 12;
	else
	  return MZ_FALSE;
  }
  if ((local_record_size > ((mz_uint64)-1 - src_file_stat.m_comp_size - descriptor_size)) ||
	  (src_file_stat.m_local_header_ofs > pSource_zip->m_archive_size) ||
	  ((local_record_size + src_file_stat.m_comp_size + descriptor_size) > (pSource_zip->m_archive_size - src_file_stat.m_local_header_ofs)))
	return MZ_FALSE;
  bytes_remaining = local_record_size + src_file_stat.m_comp_size + descriptor_size;

  if (!mz_zip_writer_write_zeros(pZip, cur_dst_file_ofs, num_alignment_padding_bytes))
	return MZ_FALSE;
  cur_dst_file_ofs += num_alignment_padding_bytes;
  local_dir_header_ofs = cur_dst_file_ofs;
  if (pZip->m_file_offset_alignment) { MZ_ASSERT((local_dir_header_ofs & (pZip->m_file_offset_alignment - 1)) == 0); }

  if (NULL == (pBuf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)MZ_MIN(MZ_ZIP_MAX_IO_BUF_SIZE, bytes_remaining))))
	return MZ_FALSE;

  while (bytes_remaining)
  {
	n = (mz_uint)MZ_MIN(MZ_ZIP_MAX_IO_BUF_SIZE, bytes_remaining);
	if (pSource_zip->m_pRead(pSource_zip->m_pIO_opaque, cur_src_file_ofs, pBuf, n) != n)
	{
	  pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
	  return MZ_FALSE;
	}
	cur_src_file_ofs += n;

	if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_dst_file_ofs, pBuf, n) != n)
	{
	  pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
	  return MZ_FALSE;
	}
	cur_dst_file_ofs += n;
	bytes_remaining -= n;
  }
  pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);

  orig_central_dir_size = pState->m_central_dir.m_size;
  orig_central_dir_offsets_size = pState->m_central_dir_offsets.m_size;
  filename_size = MZ_READ_LE16(pSrc_central_header + MZ_ZIP_CDH_FILENAME_LEN_OFS);
  extra_size = MZ_READ_LE16(pSrc_central_header + MZ_ZIP_CDH_EXTRA_LEN_OFS);
  comment_size = MZ_READ_LE16(pSrc_central_header + MZ_ZIP_CDH_COMMENT_LEN_OFS);
  pSrc_filename = pSrc_central_header + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
  pSrc_extra = pSrc_filename + filename_size;
  pSrc_comment = pSrc_extra + extra_size;
  MZ_CLEAR_OBJ(filtered_extra);
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&filtered_extra, sizeof(mz_uint8));
  if ((!mz_zip_writer_copy_extra_without_zip64(pZip, &filtered_extra, pSrc_extra, extra_size)) ||
	  (filtered_extra.m_size > MZ_UINT16_MAX) ||
	  (!mz_zip_writer_add_to_central_dir(pZip, (const char *)pSrc_filename, (mz_uint16)filename_size,
		filtered_extra.m_p, (mz_uint16)filtered_extra.m_size, pSrc_comment, (mz_uint16)comment_size,
		src_file_stat.m_uncomp_size, src_file_stat.m_comp_size, src_file_stat.m_crc32,
		MZ_READ_LE16(pSrc_central_header + MZ_ZIP_CDH_METHOD_OFS),
		MZ_READ_LE16(pSrc_central_header + MZ_ZIP_CDH_BIT_FLAG_OFS),
		MZ_READ_LE16(pSrc_central_header + MZ_ZIP_CDH_FILE_TIME_OFS),
		MZ_READ_LE16(pSrc_central_header + MZ_ZIP_CDH_FILE_DATE_OFS), local_dir_header_ofs,
		MZ_READ_LE32(pSrc_central_header + MZ_ZIP_CDH_EXTERNAL_ATTR_OFS), MZ_FALSE)))
  {
	mz_zip_array_resize(pZip, &pState->m_central_dir, orig_central_dir_size, MZ_FALSE);
	mz_zip_array_resize(pZip, &pState->m_central_dir_offsets, orig_central_dir_offsets_size, MZ_FALSE);
	mz_zip_array_clear(pZip, &filtered_extra);
	return MZ_FALSE;
  }
  mz_zip_array_clear(pZip, &filtered_extra);

  {
	mz_uint8 *pDst_central_header = &MZ_ZIP_ARRAY_ELEMENT(&pState->m_central_dir, mz_uint8, orig_central_dir_size);
	mz_uint16 version_needed = MZ_READ_LE16(pDst_central_header + MZ_ZIP_CDH_VERSION_NEEDED_OFS);
	MZ_WRITE_LE16(pDst_central_header + MZ_ZIP_CDH_VERSION_MADE_BY_OFS, MZ_READ_LE16(pSrc_central_header + MZ_ZIP_CDH_VERSION_MADE_BY_OFS));
	MZ_WRITE_LE16(pDst_central_header + MZ_ZIP_CDH_VERSION_NEEDED_OFS,
	  MZ_MAX(version_needed, MZ_READ_LE16(pSrc_central_header + MZ_ZIP_CDH_VERSION_NEEDED_OFS)));
	MZ_WRITE_LE16(pDst_central_header + MZ_ZIP_CDH_INTERNAL_ATTR_OFS, MZ_READ_LE16(pSrc_central_header + MZ_ZIP_CDH_INTERNAL_ATTR_OFS));
  }

  pZip->m_total_files++;
  pZip->m_archive_size = cur_dst_file_ofs;

  return MZ_TRUE;
}

static mz_bool mz_zip_writer_prepare_record(mz_zip_archive *pZip, mz_uint64 record_size)
{
  mz_zip_internal_state *pState = pZip->m_pState;
#ifdef MINIZ_NO_STDIO
  (void)record_size;
#endif
  if (pState->m_split_size)
  {
#ifndef MINIZ_NO_STDIO
	mz_uint64 disk_base = MZ_ZIP_ARRAY_ELEMENT(&pState->m_disk_offsets, mz_uint64, pState->m_disk_offsets.m_size - 1);
	mz_uint64 disk_used = pZip->m_archive_size - disk_base;
	if ((record_size > pState->m_split_size) || (disk_used > pState->m_split_size))
	  return MZ_FALSE;
	if (record_size > pState->m_split_size - disk_used)
	  return mz_zip_writer_split_close_current_disk(pZip) && mz_zip_writer_split_open_next_disk(pZip, pZip->m_archive_size);
#else
	return MZ_FALSE;
#endif
  }
  return MZ_TRUE;
}

typedef struct
{
  mz_zip_archive *m_pZip;
  mz_zip_array *m_pArray;
} mz_zip_writer_cdir_compress_state;

static mz_bool mz_zip_writer_cdir_put_buf(const void *pBuf, int len, void *pUser)
{
  mz_zip_writer_cdir_compress_state *pState = (mz_zip_writer_cdir_compress_state *)pUser;
  return mz_zip_array_push_back(pState->m_pZip, pState->m_pArray, pBuf, (size_t)len);
}

mz_bool mz_zip_writer_finalize_archive(mz_zip_archive *pZip)
{
  mz_zip_internal_state *pState;
  mz_uint64 central_dir_ofs, central_dir_ofs_on_disk = 0, central_dir_size, central_dir_original_size;
  mz_uint64 entries_on_trailer_disk = 0, entries_on_current_cdir_disk = 0;
  mz_uint32 central_dir_disk = 0, current_cdir_entries_disk = 0, trailer_disk, total_disks;
  mz_uint8 hdr[MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE];
  mz_bool use_zip64, compress_cdir;
  mz_zip_array compressed_cdir;

  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING))
	return MZ_FALSE;

  pState = pZip->m_pState;
  compress_cdir = (pState->m_central_dir_method != 0);
  MZ_CLEAR_OBJ(compressed_cdir);
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&compressed_cdir, sizeof(mz_uint8));

  central_dir_ofs = pZip->m_archive_size;
  central_dir_size = 0;
  central_dir_original_size = pState->m_central_dir.m_size;
  if (pZip->m_total_files)
  {
	if (compress_cdir)
	{
	  mz_bool compression_status;
#ifdef MINIZ_USE_LZMA_SDK
	  if (pState->m_central_dir_method == MZ_LZMA)
		compression_status = mz_zip_lzma_compress(pZip, pState->m_central_dir.m_p, pState->m_central_dir.m_size, pState->m_central_dir_level, &compressed_cdir);
	  else
#endif
	  {
		mz_zip_writer_cdir_compress_state compress_state;
		compress_state.m_pZip = pZip;
		compress_state.m_pArray = &compressed_cdir;
		compression_status = tdefl_compress_mem_to_output(pState->m_central_dir.m_p, pState->m_central_dir.m_size,
		  mz_zip_writer_cdir_put_buf, &compress_state,
		  tdefl_create_comp_flags_from_zip_params(pState->m_central_dir_level, -15, MZ_DEFAULT_STRATEGY));
	  }
	  if (!compression_status)
	  {
		mz_zip_array_clear(pZip, &compressed_cdir);
		return MZ_FALSE;
	  }
	  central_dir_size = compressed_cdir.m_size;
	  if (!mz_zip_writer_get_disk_relative_ofs(pZip, pZip->m_archive_size, &central_dir_disk, &central_dir_ofs_on_disk))
	  {
		mz_zip_array_clear(pZip, &compressed_cdir);
		return MZ_FALSE;
	  }
	  central_dir_ofs = pZip->m_archive_size;
	  if ((central_dir_size > ((mz_uint64)-1 - pZip->m_archive_size)) ||
		  (pZip->m_pWrite(pZip->m_pIO_opaque, pZip->m_archive_size, compressed_cdir.m_p, compressed_cdir.m_size) != compressed_cdir.m_size))
	  {
		mz_zip_array_clear(pZip, &compressed_cdir);
		return MZ_FALSE;
	  }
	  pZip->m_archive_size += central_dir_size;
	  mz_zip_array_clear(pZip, &compressed_cdir);
	}
	else
	{
	  mz_uint i;
	  for (i = 0; i < pZip->m_total_files; ++i)
	  {
		size_t record_ofs = MZ_ZIP_ARRAY_ELEMENT(&pState->m_central_dir_offsets, size_t, i);
		size_t next_record_ofs = (i + 1 < pZip->m_total_files) ?
		  MZ_ZIP_ARRAY_ELEMENT(&pState->m_central_dir_offsets, size_t, i + 1) : pState->m_central_dir.m_size;
		size_t record_size = next_record_ofs - record_ofs;
		mz_uint32 record_disk;
		mz_uint64 record_disk_ofs;
		if ((!record_size) || (!mz_zip_writer_prepare_record(pZip, record_size)) ||
			(!mz_zip_writer_get_disk_relative_ofs(pZip, pZip->m_archive_size, &record_disk, &record_disk_ofs)))
		  return MZ_FALSE;
		if (!i)
		{
		  central_dir_ofs = pZip->m_archive_size;
		  central_dir_disk = record_disk;
		  central_dir_ofs_on_disk = record_disk_ofs;
		}
		if (record_disk != current_cdir_entries_disk)
		{
		  current_cdir_entries_disk = record_disk;
		  entries_on_current_cdir_disk = 0;
		}
		++entries_on_current_cdir_disk;
		if (pZip->m_pWrite(pZip->m_pIO_opaque, pZip->m_archive_size,
			(const mz_uint8 *)pState->m_central_dir.m_p + record_ofs, record_size) != record_size)
		  return MZ_FALSE;
		pZip->m_archive_size += record_size;
		central_dir_size += record_size;
	  }
	}
  }
  else if (!mz_zip_writer_get_disk_relative_ofs(pZip, central_dir_ofs, &central_dir_disk, &central_dir_ofs_on_disk))
	return MZ_FALSE;

  pZip->m_central_directory_file_ofs = central_dir_ofs;

	use_zip64 = compress_cdir || pState->m_split_size || (pZip->m_total_files > MZ_UINT16_MAX) || (central_dir_size >= MZ_UINT32_MAX) ||
			  (central_dir_ofs_on_disk >= MZ_UINT32_MAX) ||
			  (pZip->m_archive_size > ((mz_uint64)MZ_UINT32_MAX - MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE));
  if (!mz_zip_writer_prepare_record(pZip, (use_zip64 ?
	(compress_cdir ? MZ_ZIP64_END_OF_CENTRAL_DIR_V2_HEADER_SIZE : MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE) + MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE : 0) +
	MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE))
	return MZ_FALSE;
  trailer_disk = pState->m_split_size ? (mz_uint32)(pState->m_disk_offsets.m_size - 1) : 0;
  total_disks = pState->m_split_size ? (mz_uint32)pState->m_disk_offsets.m_size : 1;
  if ((!compress_cdir) && (current_cdir_entries_disk == trailer_disk))
	entries_on_trailer_disk = entries_on_current_cdir_disk;

  if (use_zip64)
  {
	mz_uint8 zip64_eocd[MZ_ZIP64_END_OF_CENTRAL_DIR_V2_HEADER_SIZE];
	mz_uint8 zip64_locator[MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE];
	mz_uint64 zip64_eocd_ofs = pZip->m_archive_size;
	mz_uint64 zip64_eocd_ofs_on_disk;
	mz_uint32 zip64_eocd_disk;
	size_t zip64_eocd_size = compress_cdir ? MZ_ZIP64_END_OF_CENTRAL_DIR_V2_HEADER_SIZE : MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE;

	MZ_CLEAR_OBJ(zip64_eocd);
	MZ_WRITE_LE32(zip64_eocd + MZ_ZIP64_ECDH_SIG_OFS, MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIG);
	MZ_WRITE_LE64(zip64_eocd + MZ_ZIP64_ECDH_SIZE_OF_RECORD_OFS, zip64_eocd_size - 12);
	MZ_WRITE_LE16(zip64_eocd + MZ_ZIP64_ECDH_VERSION_MADE_BY_OFS, compress_cdir ? 62 : 45);
	MZ_WRITE_LE16(zip64_eocd + MZ_ZIP64_ECDH_VERSION_NEEDED_OFS, compress_cdir ? 62 : 45);
	MZ_WRITE_LE32(zip64_eocd + MZ_ZIP64_ECDH_NUM_THIS_DISK_OFS, trailer_disk);
	MZ_WRITE_LE32(zip64_eocd + MZ_ZIP64_ECDH_NUM_DISK_CDIR_OFS, central_dir_disk);
	MZ_WRITE_LE64(zip64_eocd + MZ_ZIP64_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS, compress_cdir ? 0 : entries_on_trailer_disk);
	MZ_WRITE_LE64(zip64_eocd + MZ_ZIP64_ECDH_CDIR_TOTAL_ENTRIES_OFS, pZip->m_total_files);
	MZ_WRITE_LE64(zip64_eocd + MZ_ZIP64_ECDH_CDIR_SIZE_OFS, central_dir_size);
	MZ_WRITE_LE64(zip64_eocd + MZ_ZIP64_ECDH_CDIR_OFS_OFS, central_dir_ofs_on_disk);
	if (compress_cdir)
	{
	  MZ_WRITE_LE16(zip64_eocd + MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE, pState->m_central_dir_method);
	  MZ_WRITE_LE64(zip64_eocd + MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE + 2, central_dir_size);
	  MZ_WRITE_LE64(zip64_eocd + MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE + 10, central_dir_original_size);
	}
	if ((zip64_eocd_size > ((mz_uint64)-1 - pZip->m_archive_size)) ||
		(!mz_zip_writer_get_disk_relative_ofs(pZip, zip64_eocd_ofs, &zip64_eocd_disk, &zip64_eocd_ofs_on_disk)))
	  return MZ_FALSE;
	if (pZip->m_pWrite(pZip->m_pIO_opaque, pZip->m_archive_size, zip64_eocd, zip64_eocd_size) != zip64_eocd_size)
	  return MZ_FALSE;
	pZip->m_archive_size += zip64_eocd_size;

	MZ_CLEAR_OBJ(zip64_locator);
	MZ_WRITE_LE32(zip64_locator + MZ_ZIP64_ECDL_SIG_OFS, MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIG);
	MZ_WRITE_LE32(zip64_locator + MZ_ZIP64_ECDL_NUM_DISK_CDIR_OFS, zip64_eocd_disk);
	MZ_WRITE_LE64(zip64_locator + MZ_ZIP64_ECDL_REL_OFS_TO_ZIP64_ECDR_OFS, zip64_eocd_ofs_on_disk);
	MZ_WRITE_LE32(zip64_locator + MZ_ZIP64_ECDL_TOTAL_NUMBER_OF_DISKS_OFS, total_disks);
	if (sizeof(zip64_locator) > ((mz_uint64)-1 - pZip->m_archive_size))
	  return MZ_FALSE;
	if (pZip->m_pWrite(pZip->m_pIO_opaque, pZip->m_archive_size, zip64_locator, sizeof(zip64_locator)) != sizeof(zip64_locator))
	  return MZ_FALSE;
	pZip->m_archive_size += sizeof(zip64_locator);
  }

  // Write end of central directory record
  MZ_CLEAR_OBJ(hdr);
  MZ_WRITE_LE32(hdr + MZ_ZIP_ECDH_SIG_OFS, MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG);
  MZ_WRITE_LE16(hdr + MZ_ZIP_ECDH_NUM_THIS_DISK_OFS, (trailer_disk >= MZ_UINT16_MAX) ? MZ_UINT16_MAX : trailer_disk);
  MZ_WRITE_LE16(hdr + MZ_ZIP_ECDH_NUM_DISK_CDIR_OFS, (central_dir_disk >= MZ_UINT16_MAX) ? MZ_UINT16_MAX : central_dir_disk);
  MZ_WRITE_LE16(hdr + MZ_ZIP_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS, compress_cdir ? 0 : ((entries_on_trailer_disk > MZ_UINT16_MAX) ? MZ_UINT16_MAX : (mz_uint16)entries_on_trailer_disk));
  MZ_WRITE_LE16(hdr + MZ_ZIP_ECDH_CDIR_TOTAL_ENTRIES_OFS, (compress_cdir || (pZip->m_total_files > MZ_UINT16_MAX)) ? MZ_UINT16_MAX : pZip->m_total_files);
  MZ_WRITE_LE32(hdr + MZ_ZIP_ECDH_CDIR_SIZE_OFS, (central_dir_size >= MZ_UINT32_MAX) ? MZ_UINT32_MAX : central_dir_size);
  MZ_WRITE_LE32(hdr + MZ_ZIP_ECDH_CDIR_OFS_OFS, (central_dir_ofs_on_disk >= MZ_UINT32_MAX) ? MZ_UINT32_MAX : central_dir_ofs_on_disk);

  if (sizeof(hdr) > ((mz_uint64)-1 - pZip->m_archive_size))
	return MZ_FALSE;
  if (pZip->m_pWrite(pZip->m_pIO_opaque, pZip->m_archive_size, hdr, sizeof(hdr)) != sizeof(hdr))
	return MZ_FALSE;
#ifndef MINIZ_NO_STDIO
  if (pState->m_split_size)
  {
	char *pLast_name;
	mz_uint32 next_disk_number;
	if (pState->m_files.m_size == 1)
	{
	  mz_uint8 temporary_sig[sizeof(mz_uint32)];
	  MZ_WRITE_LE32(temporary_sig, 0x30304b50);
	  if (pZip->m_pWrite(pZip->m_pIO_opaque, 0, temporary_sig, sizeof(temporary_sig)) != sizeof(temporary_sig))
		return MZ_FALSE;
	}
	if (!mz_zip_writer_split_close_current_disk(pZip))
	  return MZ_FALSE;
	if (NULL == (pLast_name = mz_zip_writer_split_filename(pZip, (mz_uint32)pState->m_files.m_size)))
	  return MZ_FALSE;
	MZ_DELETE_FILE(pState->m_pZip_filename);
	if (rename(pLast_name, pState->m_pZip_filename) != 0)
	{
	  pZip->m_pFree(pZip->m_pAlloc_opaque, pLast_name);
	  return MZ_FALSE;
	}
	pZip->m_pFree(pZip->m_pAlloc_opaque, pLast_name);
	for (next_disk_number = (mz_uint32)pState->m_files.m_size + 1; next_disk_number != 0; ++next_disk_number)
	{
	  char *pStale_name = mz_zip_writer_split_filename(pZip, next_disk_number);
	  if (!pStale_name)
		break;
	  if (MZ_DELETE_FILE(pStale_name) != 0)
	  {
		pZip->m_pFree(pZip->m_pAlloc_opaque, pStale_name);
		break;
	  }
	  pZip->m_pFree(pZip->m_pAlloc_opaque, pStale_name);
	}
  }
  else if ((pState->m_pFile) && (MZ_FFLUSH(pState->m_pFile) == EOF))
	return MZ_FALSE;
#endif // #ifndef MINIZ_NO_STDIO

  pZip->m_archive_size += sizeof(hdr);

  pZip->m_zip_mode = MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED;
  return MZ_TRUE;
}

mz_bool mz_zip_writer_finalize_heap_archive(mz_zip_archive *pZip, void **pBuf, size_t *pSize)
{
  if ((!pZip) || (!pZip->m_pState) || (!pBuf) || (!pSize))
	return MZ_FALSE;
  if (pZip->m_pWrite != mz_zip_heap_write_func)
	return MZ_FALSE;
  if (!mz_zip_writer_finalize_archive(pZip))
	return MZ_FALSE;

  *pBuf = pZip->m_pState->m_pMem;
  *pSize = pZip->m_pState->m_mem_size;
  pZip->m_pState->m_pMem = NULL;
  pZip->m_pState->m_mem_size = pZip->m_pState->m_mem_capacity = 0;
  return MZ_TRUE;
}

mz_bool mz_zip_writer_end(mz_zip_archive *pZip)
{
  mz_zip_internal_state *pState;
  mz_bool status = MZ_TRUE;
  if ((!pZip) || (!pZip->m_pState) || (!pZip->m_pAlloc) || (!pZip->m_pFree) || ((pZip->m_zip_mode != MZ_ZIP_MODE_WRITING) && (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED)))
	return MZ_FALSE;

  pState = pZip->m_pState;
  pZip->m_pState = NULL;

#ifndef MINIZ_NO_STDIO
  if (pState->m_files.m_p)
  {
	size_t i;
	for (i = 0; i < pState->m_files.m_size; ++i)
	{
	  MZ_FILE *pFile = MZ_ZIP_ARRAY_ELEMENT(&pState->m_files, MZ_FILE *, i);
	  if (pFile)
		MZ_FCLOSE(pFile);
	}
  }
#endif

  mz_zip_array_clear(pZip, &pState->m_central_dir);
  mz_zip_array_clear(pZip, &pState->m_central_dir_offsets);
  mz_zip_array_clear(pZip, &pState->m_sorted_central_dir_offsets);
  mz_zip_array_clear(pZip, &pState->m_disk_offsets);
  mz_zip_array_clear(pZip, &pState->m_files);

  if (pState->m_pZip_filename)
  {
	pZip->m_pFree(pZip->m_pAlloc_opaque, pState->m_pZip_filename);
	pState->m_pZip_filename = NULL;
  }

  if (pState->m_pPassword)
  {
	memset(pState->m_pPassword, 0, pState->m_password_len); // don't leave the password in freed memory
	pZip->m_pFree(pZip->m_pAlloc_opaque, pState->m_pPassword);
	pState->m_pPassword = NULL;
	pState->m_password_len = 0;
  }

#ifndef MINIZ_NO_STDIO
  if (pState->m_pFile)
  {
	MZ_FCLOSE(pState->m_pFile);
	pState->m_pFile = NULL;
  }
#endif // #ifndef MINIZ_NO_STDIO

  if ((pZip->m_pWrite == mz_zip_heap_write_func) && (pState->m_pMem))
  {
	pZip->m_pFree(pZip->m_pAlloc_opaque, pState->m_pMem);
	pState->m_pMem = NULL;
  }

  pZip->m_pFree(pZip->m_pAlloc_opaque, pState);
  pZip->m_zip_mode = MZ_ZIP_MODE_INVALID;
  return status;
}

#ifndef MINIZ_NO_STDIO
mz_bool mz_zip_add_mem_to_archive_file_in_place(const char *pZip_filename, const char *pArchive_name, const void *pBuf, size_t buf_size, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags)
{
  mz_bool status, created_new_archive = MZ_FALSE;
  mz_zip_archive zip_archive;
  struct MZ_FILE_STAT_STRUCT file_stat;
  MZ_CLEAR_OBJ(zip_archive);
  if ((int)level_and_flags < 0)
	 level_and_flags = MZ_DEFAULT_LEVEL;
  if ((!pZip_filename) || (!pArchive_name) || ((buf_size) && (!pBuf)) || ((comment_size) && (!pComment)) || ((level_and_flags & 0xF) > MZ_UBER_COMPRESSION))
	return MZ_FALSE;
  if (!mz_zip_writer_validate_archive_name(pArchive_name))
	return MZ_FALSE;
  if (MZ_FILE_STAT(pZip_filename, &file_stat) != 0)
  {
	// Create a new archive.
	if (!mz_zip_writer_init_file(&zip_archive, pZip_filename, 0))
	  return MZ_FALSE;
	created_new_archive = MZ_TRUE;
  }
  else
  {
	// Append to an existing archive.
	if (!mz_zip_reader_init_file(&zip_archive, pZip_filename, level_and_flags | MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY))
	  return MZ_FALSE;
	if (!mz_zip_writer_init_from_reader(&zip_archive, pZip_filename))
	{
	  mz_zip_reader_end(&zip_archive);
	  return MZ_FALSE;
	}
  }
  status = mz_zip_writer_add_mem_ex(&zip_archive, pArchive_name, pBuf, buf_size, pComment, comment_size, level_and_flags, 0, 0);
  // Always finalize, even if adding failed for some reason, so we have a valid central directory. (This may not always succeed, but we can try.)
  if (!mz_zip_writer_finalize_archive(&zip_archive))
	status = MZ_FALSE;
  if (!mz_zip_writer_end(&zip_archive))
	status = MZ_FALSE;
  if ((!status) && (created_new_archive))
  {
	// It's a new archive and something went wrong, so just delete it.
	int ignoredStatus = MZ_DELETE_FILE(pZip_filename);
	(void)ignoredStatus;
  }
  return status;
}

void *mz_zip_extract_archive_file_to_heap(const char *pZip_filename, const char *pArchive_name, size_t *pSize, mz_uint flags)
{
  int file_index;
  mz_zip_archive zip_archive;
  void *p = NULL;

  if (pSize)
	*pSize = 0;

  if ((!pZip_filename) || (!pArchive_name))
	return NULL;

  MZ_CLEAR_OBJ(zip_archive);
  if (!mz_zip_reader_init_file(&zip_archive, pZip_filename, flags | MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY))
	return NULL;

  if ((file_index = mz_zip_reader_locate_file(&zip_archive, pArchive_name, NULL, flags)) >= 0)
	p = mz_zip_reader_extract_to_heap(&zip_archive, file_index, pSize, flags);

  mz_zip_reader_end(&zip_archive);
  return p;
}

#endif // #ifndef MINIZ_NO_STDIO

#endif // #ifndef MINIZ_NO_ARCHIVE_WRITING_APIS

#endif // #ifndef MINIZ_NO_ARCHIVE_APIS

#ifdef __cplusplus
}
#endif

#endif // MINIZ_HEADER_FILE_ONLY

/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non-commercial, and by any
  means.

  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.

  For more information, please refer to <http://unlicense.org/>
*/

/*
------------------------------------------------------------------------------
	END miniz.c
------------------------------------------------------------------------------
*/
#if defined(__cplusplus)
class Zip
{
public:
	Zip() { memset(&mMZip, 0, sizeof(mMZip)); }
#ifndef MINIZ_NO_STDIO
	mz_bool Open(const char* path)
	{ return mz_zip_reader_init_file(&mMZip, path, 0); }
#endif
	mz_bool OpenMemory(void* buffer, mz_uint bufferSize)
	{ return mz_zip_reader_init_mem(&mMZip, buffer, bufferSize, 0); }
	void Close()
	{
#ifndef MINIZ_NO_ARCHIVE_WRITING_APIS
		if(mMZip.m_zip_mode == MZ_ZIP_MODE_WRITING)
			mz_zip_writer_end(&mMZip);
		else
#endif
			mz_zip_reader_end(&mMZip);
	}
	mz_uint GetEntryName(mz_uint index, char* buffer, mz_uint bufferSize)
	{ return mz_zip_reader_get_filename(&mMZip, index, buffer, bufferSize); }
	mz_uint GetNumFiles()
	{ return mz_zip_reader_get_num_files(&mMZip); }
	mz_bool IsEntryDirectory(mz_uint index)
	{ return mz_zip_reader_is_file_a_directory(&mMZip, (mz_uint)index); }
	mz_bool IsEntryEncrypted(mz_uint index)
	{ return mz_zip_reader_is_file_encrypted(&mMZip, index); }
	mz_bool Stat(mz_uint index, mz_zip_archive_file_stat* pStat)
	{ return mz_zip_reader_file_stat(&mMZip, index, pStat); }
	mz_bool Read(mz_uint index, void* buffer, mz_uint bufferSize)
	{ return mz_zip_reader_extract_to_mem_no_alloc(&mMZip,
				index, buffer, bufferSize, 0, 0, 0); }
	// Valid flags: MZ_ZIP_FLAG_CASE_SENSITIVE, MZ_ZIP_FLAG_IGNORE_PATH
	// Returns -1 if the file cannot be found.
	int FindEntry(const char* entryName,
		const char* comment = NULL, mz_uint flags = 0)
	{ return mz_zip_reader_locate_file(&mMZip, entryName, comment, flags); }

#ifndef MINIZ_NO_ARCHIVE_WRITING_APIS
	bool ConvertAndCreate(const char* path = NULL)
	{ return mz_zip_writer_init_from_reader(&mMZip, path); }
	bool Create(mz_uint archiveSize = 0)
	{ return mz_zip_writer_init(&mMZip, archiveSize); }
	bool CreateHeap(mz_uint begSizeToReserve, mz_uint initialAllocSize)
	{ return mz_zip_writer_init_heap(&mMZip, begSizeToReserve,
		initialAllocSize); }
#ifndef MINIZ_NO_STDIO
	bool CreateFile(const char* path, mz_uint begSizeToReserve)
	{ return mz_zip_writer_init_file(&mMZip, path, begSizeToReserve); }
	bool CreateSplitFile(const char* path, mz_uint64 splitSize)
	{ return mz_zip_writer_init_file_split(&mMZip, path, splitSize); }
#endif
	bool SetCentralDirectoryCompression(mz_uint method, mz_uint level)
	{ return mz_zip_writer_set_central_directory_compression(&mMZip, method, level); }
	bool AddMem(const char *pArchive_name, const void *pBuf, size_t buf_size,
				mz_uint level_and_flags = MZ_DEFAULT_COMPRESSION,
				const void *pComment = NULL, mz_uint16 comment_size = 0,
				mz_uint64 uncomp_size = 0, mz_uint32 uncomp_crc32 = 0)
	{ return mz_zip_writer_add_mem_ex(&mMZip, pArchive_name, pBuf, buf_size,
				pComment, comment_size, level_and_flags,
		uncomp_size, uncomp_crc32); }
#ifndef MINIZ_NO_STDIO
	bool AddFile(const char * entryName, const char *pSrc_filename,
		const void *pComment, mz_uint16 comment_size,
		mz_uint level_and_flags = MZ_DEFAULT_COMPRESSION)
	{ return mz_zip_writer_add_file(&mMZip, entryName,
		pSrc_filename, pComment, comment_size, level_and_flags); }
#endif
	bool AddFromOther(Zip& mz, mz_uint file_index)
	{ return mz_zip_writer_add_from_zip_reader(&mMZip, &mz.mMZip, file_index); }

	bool Finalize()
	{ return mz_zip_writer_finalize_archive(&mMZip); }
	bool FinalizeHeap(void **pBuf, size_t *pSize)
	{ return mz_zip_writer_finalize_heap_archive(&mMZip, pBuf, pSize); }
#endif
	mz_zip_archive mMZip;
};

extern "C" {
#endif
typedef struct ZIP
{
	mz_zip_archive mMZip;
} ZIP;
void Zip_Construct(ZIP* pThis)
{ memset(&pThis->mMZip, 0, sizeof(pThis->mMZip)); }
#define Zip_Destruct(pThis) /* no-op */
#ifndef MINIZ_NO_STDIO
	mz_bool Zip_Open(ZIP* pThis, const char* path)
	{ return mz_zip_reader_init_file(&pThis->mMZip, path, 0); }
#endif
mz_bool Zip_OpenMemory(ZIP* pThis, void* buffer, mz_uint bufferSize)
{ return mz_zip_reader_init_mem(&pThis->mMZip, buffer, bufferSize, 0); }
void Zip_Close(ZIP* pThis)
{
#ifndef MINIZ_NO_ARCHIVE_WRITING_APIS
	if(pThis->mMZip.m_zip_mode == MZ_ZIP_MODE_WRITING)
		mz_zip_writer_end(&pThis->mMZip);
	else
#endif
		mz_zip_reader_end(&pThis->mMZip);
}
mz_uint Zip_GetEntryName(ZIP* pThis, mz_uint index,
	char* buffer, mz_uint bufferSize)
{ return mz_zip_reader_get_filename(&pThis->mMZip, index,
	buffer, bufferSize); }
mz_uint Zip_GetNumFiles(ZIP* pThis)
{ return mz_zip_reader_get_num_files(&pThis->mMZip); }
mz_bool Zip_IsEntryDirectory(ZIP* pThis, mz_uint index)
{ return mz_zip_reader_is_file_a_directory(&pThis->mMZip, (mz_uint)index); }
mz_bool Zip_IsEntryEncrypted(ZIP* pThis, mz_uint index)
{ return mz_zip_reader_is_file_encrypted(&pThis->mMZip, index); }
mz_bool Zip_Stat(ZIP* pThis, mz_uint index, mz_zip_archive_file_stat* pStat)
{ return mz_zip_reader_file_stat(&pThis->mMZip, index, pStat); }
mz_bool Zip_Read(ZIP* pThis, mz_uint index, void* buffer, mz_uint bufferSize)
{ return mz_zip_reader_extract_to_mem_no_alloc(&pThis->mMZip,
			index, buffer, bufferSize, 0, 0, 0); }
/* Valid flags: MZ_ZIP_FLAG_CASE_SENSITIVE, MZ_ZIP_FLAG_IGNORE_PATH
 Returns -1 if the file cannot be found. */
int Zip_FindEntry(ZIP* pThis, const char* entryName,
	const char* comment /*= NULL*/, mz_uint flags /*= 0*/)
{ return mz_zip_reader_locate_file(&pThis->mMZip, entryName, comment, flags); }

#ifndef MINIZ_NO_ARCHIVE_WRITING_APIS
	mz_bool Zip_ConvertAndCreate(ZIP* pThis, const char* path /*= NULL*/)
	{ return mz_zip_writer_init_from_reader(&pThis->mMZip, path); }
	mz_bool Zip_Create(ZIP* pThis, mz_uint archiveSize /*= 0*/)
	{ return mz_zip_writer_init(&pThis->mMZip, archiveSize); }
	mz_bool Zip_CreateHeap(ZIP* pThis, mz_uint begSizeToReserve,
		mz_uint initialAllocSize)
	{ return mz_zip_writer_init_heap(&pThis->mMZip,
		begSizeToReserve, initialAllocSize); }
#ifndef MINIZ_NO_STDIO
	mz_bool Zip_CreateFile(ZIP* pThis, const char* path, mz_uint begSizeToReserve)
	{ return mz_zip_writer_init_file(&pThis->mMZip, path, begSizeToReserve); }
	mz_bool Zip_CreateSplitFile(ZIP* pThis, const char* path, mz_uint64 splitSize)
	{ return mz_zip_writer_init_file_split(&pThis->mMZip, path, splitSize); }
#endif
	mz_bool Zip_SetCentralDirectoryCompression(ZIP* pThis, mz_uint method, mz_uint level)
	{ return mz_zip_writer_set_central_directory_compression(&pThis->mMZip, method, level); }
	mz_bool Zip_AddMemWithParams(ZIP* pThis, const char *pArchive_name,
		const void *pBuf, size_t buf_size,
		mz_uint level_and_flags /*= MZ_DEFAULT_COMPRESSION */,
		const void *pComment /*= NULL*/, mz_uint16 comment_size /* = 0 */,
		mz_uint64 uncomp_size /*= 0*/, mz_uint32 uncomp_crc32 /* = 0 */)
	{ return mz_zip_writer_add_mem_ex(&pThis->mMZip, pArchive_name, pBuf,
				buf_size, pComment, comment_size, level_and_flags,
				uncomp_size, uncomp_crc32); }

	mz_bool Zip_AddMem(ZIP* pThis, const char *pArchive_name,
		const void *pBuf, size_t buf_size)
	{ return Zip_AddMemWithParams(pThis, pArchive_name, pBuf, buf_size,
				MZ_DEFAULT_COMPRESSION, NULL, 0, 0, 0); }
#ifndef MINIZ_NO_STDIO
	mz_bool Zip_AddFile(ZIP* pThis, const char * entryName,
		const char *pSrc_filename, const void *pComment,
		mz_uint16 comment_size,
		mz_uint level_and_flags /*= MZ_DEFAULT_COMPRESSION*/)
	{ return mz_zip_writer_add_file(&pThis->mMZip, entryName, pSrc_filename,
		pComment, comment_size, level_and_flags); }
#endif
	mz_bool Zip_AddFromOther(ZIP* pThis, ZIP* mz, mz_uint file_index)
	{ return mz_zip_writer_add_from_zip_reader(&pThis->mMZip,
		&mz->mMZip, file_index); }

	mz_bool Zip_Finalize(ZIP* pThis)
	{ return mz_zip_writer_finalize_archive(&pThis->mMZip); }
	mz_bool Zip_FinalizeHeap(ZIP* pThis, void **pBuf, size_t *pSize)
	{ return mz_zip_writer_finalize_heap_archive(&pThis->mMZip, pBuf, pSize); }
#endif
#if defined(__cplusplus)
}
#endif
#if 0
#include <iostream>

int main()
{
	MZ mz;
	mz_bool ok = mz.Open("minilibs-master.zip");
	if(ok)
	{
		std::cout << "Found: " << mz.GetNumFiles() << " Files\n";
		char data[50000];
		mz_zip_archive_file_stat stat;
		mz.Stat(1, &stat);
		if(!mz.Read(1, data, 5000))
			std::cout << "SHOULDNT HAPPEN\n\n\n\n";
		data[stat.m_uncomp_size] = 0;
		std::cout << data;

		MZ mzo;
		mzo.CreateFile("test.zip", stat.m_uncomp_size);
		mzo.AddMem("test.c", data, stat.m_uncomp_size);
		mzo.Finalize();
		mzo.Close();

	}
	else
		std::cout << "NO\n";
}
#endif

