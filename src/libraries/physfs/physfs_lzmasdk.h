#ifndef _INCLUDE_PHYSFS_LZMASDK_H_
#define _INCLUDE_PHYSFS_LZMASDK_H_

/* This is just a bunch of the LZMA SDK mushed together into one header.
This code is all public domain, and mostly (if not entirely) written by
Igor Pavlov. http://www.7-zip.org/sdk.html
--ryan. */



/* 7zTypes.h -- Basic types
2013-11-12 : Igor Pavlov : Public domain */

#ifndef __7Z_TYPES_H
#define __7Z_TYPES_H

#ifdef _WIN32
/* #include <windows.h> */
#endif

#include <stddef.h>

#ifndef EXTERN_C_BEGIN
#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif
#endif

EXTERN_C_BEGIN

#define SZ_OK 0

#define SZ_ERROR_DATA 1
#define SZ_ERROR_MEM 2
#define SZ_ERROR_CRC 3
#define SZ_ERROR_UNSUPPORTED 4
#define SZ_ERROR_PARAM 5
#define SZ_ERROR_INPUT_EOF 6
#define SZ_ERROR_OUTPUT_EOF 7
#define SZ_ERROR_READ 8
#define SZ_ERROR_WRITE 9
#define SZ_ERROR_PROGRESS 10
#define SZ_ERROR_FAIL 11
#define SZ_ERROR_THREAD 12

#define SZ_ERROR_ARCHIVE 16
#define SZ_ERROR_NO_ARCHIVE 17

typedef int SRes;

#ifdef _WIN32
/* typedef DWORD WRes; */
typedef unsigned WRes;
#else
typedef int WRes;
#endif

#ifndef RINOK
#define RINOK(x) { int __result__ = (x); if (__result__ != 0) return __result__; }
#endif

typedef unsigned char Byte;
typedef short Int16;
typedef unsigned short UInt16;

#ifdef _LZMA_UINT32_IS_ULONG
typedef long Int32;
typedef unsigned long UInt32;
#else
typedef int Int32;
typedef unsigned int UInt32;
#endif

#ifdef _SZ_NO_INT_64

/* define _SZ_NO_INT_64, if your compiler doesn't support 64-bit integers.
   NOTES: Some code will work incorrectly in that case! */

typedef long Int64;
typedef unsigned long UInt64;

#else

#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__WATCOMC__)
typedef __int64 Int64;
typedef unsigned __int64 UInt64;
#define UINT64_CONST(n) n ## ui64
#else
typedef long long int Int64;
typedef unsigned long long int UInt64;
#define UINT64_CONST(n) n ## ULL
#endif

#endif

#ifdef _LZMA_NO_SYSTEM_SIZE_T
typedef UInt32 SizeT;
#else
typedef size_t SizeT;
#endif

typedef int Bool;
#define True 1
#define False 0


#ifdef _WIN32
#define MY_STD_CALL __stdcall
#else
#define MY_STD_CALL
#endif

#ifdef _MSC_VER

#if _MSC_VER >= 1300
#define MY_NO_INLINE __declspec(noinline)
#else
#define MY_NO_INLINE
#endif

#define MY_CDECL __cdecl
#define MY_FAST_CALL __fastcall

#else

#define MY_NO_INLINE
#define MY_CDECL
#define MY_FAST_CALL

#endif


/* The following interfaces use first parameter as pointer to structure */

typedef struct
{
  Byte (*Read)(void *p); /* reads one byte, returns 0 in case of EOF or error */
} IByteIn;

typedef struct
{
  void (*Write)(void *p, Byte b);
} IByteOut;

typedef struct
{
  SRes (*Read)(void *p, void *buf, size_t *size);
    /* if (input(*size) != 0 && output(*size) == 0) means end_of_stream.
       (output(*size) < input(*size)) is allowed */
} ISeqInStream;

typedef struct
{
  size_t (*Write)(void *p, const void *buf, size_t size);
    /* Returns: result - the number of actually written bytes.
       (result < size) means error */
} ISeqOutStream;

typedef enum
{
  SZ_SEEK_SET = 0,
  SZ_SEEK_CUR = 1,
  SZ_SEEK_END = 2
} ESzSeek;

typedef struct
{
  SRes (*Read)(void *p, void *buf, size_t *size);  /* same as ISeqInStream::Read */
  SRes (*Seek)(void *p, Int64 *pos, ESzSeek origin);
} ISeekInStream;

typedef struct
{
  SRes (*Look)(void *p, const void **buf, size_t *size);
    /* if (input(*size) != 0 && output(*size) == 0) means end_of_stream.
       (output(*size) > input(*size)) is not allowed
       (output(*size) < input(*size)) is allowed */
  SRes (*Skip)(void *p, size_t offset);
    /* offset must be <= output(*size) of Look */

  SRes (*Read)(void *p, void *buf, size_t *size);
    /* reads directly (without buffer). It's same as ISeqInStream::Read */
  SRes (*Seek)(void *p, Int64 *pos, ESzSeek origin);
} ILookInStream;

static SRes LookInStream_SeekTo(ILookInStream *stream, UInt64 offset);

/* reads via ILookInStream::Read */
static SRes LookInStream_Read2(ILookInStream *stream, void *buf, size_t size, SRes errorType);
static SRes LookInStream_Read(ILookInStream *stream, void *buf, size_t size);

#define LookToRead_BUF_SIZE (1 << 14)

typedef struct
{
  ILookInStream s;
  ISeekInStream *realStream;
  size_t pos;
  size_t size;
  Byte buf[LookToRead_BUF_SIZE];
} CLookToRead;

static void LookToRead_CreateVTable(CLookToRead *p, int lookahead);
static void LookToRead_Init(CLookToRead *p);

typedef struct
{
  ISeqInStream s;
  ILookInStream *realStream;
} CSecToLook;

typedef struct
{
  ISeqInStream s;
  ILookInStream *realStream;
} CSecToRead;

typedef struct
{
  SRes (*Progress)(void *p, UInt64 inSize, UInt64 outSize);
    /* Returns: result. (result != SZ_OK) means break.
       Value (UInt64)(Int64)-1 for size means unknown value. */
} ICompressProgress;

typedef struct
{
  void *(*Alloc)(void *p, size_t size);
  void (*Free)(void *p, void *address); /* address can be 0 */
} ISzAlloc;

#define IAlloc_Alloc(p, size) (p)->Alloc((p), size)
#define IAlloc_Free(p, a) (p)->Free((p), a)

#ifdef _WIN32

#define CHAR_PATH_SEPARATOR '\\'
#define WCHAR_PATH_SEPARATOR L'\\'
#define STRING_PATH_SEPARATOR "\\"
#define WSTRING_PATH_SEPARATOR L"\\"

#else

#define CHAR_PATH_SEPARATOR '/'
#define WCHAR_PATH_SEPARATOR L'/'
#define STRING_PATH_SEPARATOR "/"
#define WSTRING_PATH_SEPARATOR L"/"

#endif

EXTERN_C_END

#endif

/* 7z.h -- 7z interface
2015-11-18 : Igor Pavlov : Public domain */

#ifndef __7Z_H
#define __7Z_H

/*#include "7zTypes.h"*/

EXTERN_C_BEGIN

#define k7zStartHeaderSize 0x20
#define k7zSignatureSize 6

static const Byte k7zSignature[k7zSignatureSize];

typedef struct
{
  const Byte *Data;
  size_t Size;
} CSzData;

/* CSzCoderInfo & CSzFolder support only default methods */

typedef struct
{
  size_t PropsOffset;
  UInt32 MethodID;
  Byte NumStreams;
  Byte PropsSize;
} CSzCoderInfo;

typedef struct
{
  UInt32 InIndex;
  UInt32 OutIndex;
} CSzBond;

#define SZ_NUM_CODERS_IN_FOLDER_MAX 4
#define SZ_NUM_BONDS_IN_FOLDER_MAX 3
#define SZ_NUM_PACK_STREAMS_IN_FOLDER_MAX 4

typedef struct
{
  UInt32 NumCoders;
  UInt32 NumBonds;
  UInt32 NumPackStreams;
  UInt32 UnpackStream;
  UInt32 PackStreams[SZ_NUM_PACK_STREAMS_IN_FOLDER_MAX];
  CSzBond Bonds[SZ_NUM_BONDS_IN_FOLDER_MAX];
  CSzCoderInfo Coders[SZ_NUM_CODERS_IN_FOLDER_MAX];
} CSzFolder;


static SRes SzGetNextFolderItem(CSzFolder *f, CSzData *sd);

typedef struct
{
  UInt32 Low;
  UInt32 High;
} CNtfsFileTime;

typedef struct
{
  Byte *Defs; /* MSB 0 bit numbering */
  UInt32 *Vals;
} CSzBitUi32s;

typedef struct
{
  Byte *Defs; /* MSB 0 bit numbering */
  /* UInt64 *Vals; */
  CNtfsFileTime *Vals;
} CSzBitUi64s;

#define SzBitArray_Check(p, i) (((p)[(i) >> 3] & (0x80 >> ((i) & 7))) != 0)

#define SzBitWithVals_Check(p, i) ((p)->Defs && ((p)->Defs[(i) >> 3] & (0x80 >> ((i) & 7))) != 0)

typedef struct
{
  UInt32 NumPackStreams;
  UInt32 NumFolders;

  UInt64 *PackPositions;          /* NumPackStreams + 1 */
  CSzBitUi32s FolderCRCs;         /* NumFolders */

  size_t *FoCodersOffsets;        /* NumFolders + 1 */
  UInt32 *FoStartPackStreamIndex; /* NumFolders + 1 */
  UInt32 *FoToCoderUnpackSizes;   /* NumFolders + 1 */
  Byte *FoToMainUnpackSizeIndex;  /* NumFolders */
  UInt64 *CoderUnpackSizes;       /* for all coders in all folders */

  Byte *CodersData;
} CSzAr;

static UInt64 SzAr_GetFolderUnpackSize(const CSzAr *p, UInt32 folderIndex);

static SRes SzAr_DecodeFolder(const CSzAr *p, UInt32 folderIndex,
    ILookInStream *stream, UInt64 startPos,
    Byte *outBuffer, size_t outSize,
    ISzAlloc *allocMain);

typedef struct
{
  CSzAr db;

  UInt64 startPosAfterHeader;
  UInt64 dataPos;
  
  UInt32 NumFiles;

  UInt64 *UnpackPositions;  /* NumFiles + 1 */
  /* Byte *IsEmptyFiles; */
  Byte *IsDirs;
  CSzBitUi32s CRCs;

  CSzBitUi32s Attribs;
  /* CSzBitUi32s Parents; */
  CSzBitUi64s MTime;
  CSzBitUi64s CTime;

  UInt32 *FolderToFile;   /* NumFolders + 1 */
  UInt32 *FileToFolder;   /* NumFiles */

  size_t *FileNameOffsets; /* in 2-byte steps */
  Byte *FileNames;  /* UTF-16-LE */
} CSzArEx;

#define SzArEx_IsDir(p, i) (SzBitArray_Check((p)->IsDirs, i))

#define SzArEx_GetFileSize(p, i) ((p)->UnpackPositions[(i) + 1] - (p)->UnpackPositions[i])

static void SzArEx_Init(CSzArEx *p);
static void SzArEx_Free(CSzArEx *p, ISzAlloc *alloc);

/*
if dest == NULL, the return value specifies the required size of the buffer,
  in 16-bit characters, including the null-terminating character.
if dest != NULL, the return value specifies the number of 16-bit characters that
  are written to the dest, including the null-terminating character. */

static size_t SzArEx_GetFileNameUtf16(const CSzArEx *p, size_t fileIndex, UInt16 *dest);

/*
size_t SzArEx_GetFullNameLen(const CSzArEx *p, size_t fileIndex);
UInt16 *SzArEx_GetFullNameUtf16_Back(const CSzArEx *p, size_t fileIndex, UInt16 *dest);
*/



/*
  SzArEx_Extract extracts file from archive

  *outBuffer must be 0 before first call for each new archive.

  Extracting cache:
    If you need to decompress more than one file, you can send
    these values from previous call:
      *blockIndex,
      *outBuffer,
      *outBufferSize
    You can consider "*outBuffer" as cache of solid block. If your archive is solid,
    it will increase decompression speed.
  
    If you use external function, you can declare these 3 cache variables
    (blockIndex, outBuffer, outBufferSize) as static in that external function.
    
    Free *outBuffer and set *outBuffer to 0, if you want to flush cache.
*/

static SRes SzArEx_Extract(
    const CSzArEx *db,
    ILookInStream *inStream,
    UInt32 fileIndex,         /* index of file */
    UInt32 *blockIndex,       /* index of solid block */
    Byte **outBuffer,         /* pointer to pointer to output buffer (allocated with allocMain) */
    size_t *outBufferSize,    /* buffer size for output buffer */
    size_t *offset,           /* offset of stream for required file in *outBuffer */
    size_t *outSizeProcessed, /* size of file in *outBuffer */
    ISzAlloc *allocMain,
    ISzAlloc *allocTemp);


/*
SzArEx_Open Errors:
SZ_ERROR_NO_ARCHIVE
SZ_ERROR_ARCHIVE
SZ_ERROR_UNSUPPORTED
SZ_ERROR_MEM
SZ_ERROR_CRC
SZ_ERROR_INPUT_EOF
SZ_ERROR_FAIL
*/

static SRes SzArEx_Open(CSzArEx *p, ILookInStream *inStream,
    ISzAlloc *allocMain, ISzAlloc *allocTemp);

EXTERN_C_END

#endif

/* 7zCrc.h -- CRC32 calculation
2013-01-18 : Igor Pavlov : Public domain */

#ifndef __7Z_CRC_H
#define __7Z_CRC_H

/*#include "7zTypes.h" */

EXTERN_C_BEGIN

/* Call CrcGenerateTable one time before other CRC functions */
static void MY_FAST_CALL CrcGenerateTable(void);

#define CRC_INIT_VAL 0xFFFFFFFF
#define CRC_GET_DIGEST(crc) ((crc) ^ CRC_INIT_VAL)
#define CRC_UPDATE_BYTE(crc, b) (g_CrcTable[((crc) ^ (b)) & 0xFF] ^ ((crc) >> 8))

static UInt32 MY_FAST_CALL CrcCalc(const void *data, size_t size);

EXTERN_C_END

#endif

/* CpuArch.h -- CPU specific code
2016-06-09: Igor Pavlov : Public domain */

#ifndef __CPU_ARCH_H
#define __CPU_ARCH_H

/*#include "7zTypes.h"*/

EXTERN_C_BEGIN

/*
MY_CPU_LE means that CPU is LITTLE ENDIAN.
MY_CPU_BE means that CPU is BIG ENDIAN.
If MY_CPU_LE and MY_CPU_BE are not defined, we don't know about ENDIANNESS of platform.

MY_CPU_LE_UNALIGN means that CPU is LITTLE ENDIAN and CPU supports unaligned memory accesses.
*/

#if defined(_M_X64) \
   || defined(_M_AMD64) \
   || defined(__x86_64__) \
   || defined(__AMD64__) \
   || defined(__amd64__)
  #define MY_CPU_AMD64
#endif

#if defined(MY_CPU_AMD64) \
    || defined(_M_ARM64) \
    || defined(_M_IA64) \
    || defined(__AARCH64EL__) \
    || defined(__AARCH64EB__)
  #define MY_CPU_64BIT
#endif

#if defined(_M_IX86) || defined(__i386__)
#define MY_CPU_X86
#endif

#if defined(MY_CPU_X86) || defined(MY_CPU_AMD64)
#define MY_CPU_X86_OR_AMD64
#endif

#if defined(MY_CPU_X86) \
    || defined(_M_ARM) \
    || defined(__ARMEL__) \
    || defined(__THUMBEL__) \
    || defined(__ARMEB__) \
    || defined(__THUMBEB__)
  #define MY_CPU_32BIT
#endif

#if defined(_WIN32) && defined(_M_ARM)
#define MY_CPU_ARM_LE
#elif defined(_WIN64) && defined(_M_ARM64)
#define MY_CPU_ARM_LE
#endif

#if defined(_WIN32) && defined(_M_IA64)
#define MY_CPU_IA64_LE
#endif

#if defined(MY_CPU_X86_OR_AMD64) \
    || defined(MY_CPU_ARM_LE) \
    || defined(MY_CPU_IA64_LE) \
    || defined(__LITTLE_ENDIAN__) \
    || defined(__ARMEL__) \
    || defined(__THUMBEL__) \
    || defined(__AARCH64EL__) \
    || defined(__MIPSEL__) \
    || defined(__MIPSEL) \
    || defined(_MIPSEL) \
    || defined(__BFIN__) \
    || (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__))
  #define MY_CPU_LE
#endif

#if defined(__BIG_ENDIAN__) \
    || defined(__ARMEB__) \
    || defined(__THUMBEB__) \
    || defined(__AARCH64EB__) \
    || defined(__MIPSEB__) \
    || defined(__MIPSEB) \
    || defined(_MIPSEB) \
    || defined(__m68k__) \
    || defined(__s390__) \
    || defined(__s390x__) \
    || defined(__zarch__) \
    || (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__))
  #define MY_CPU_BE
#endif

#if defined(MY_CPU_LE) && defined(MY_CPU_BE)
Stop_Compiling_Bad_Endian
#endif


#ifdef MY_CPU_LE
  #if defined(MY_CPU_X86_OR_AMD64) \
      /* || defined(__AARCH64EL__) */
    /*#define MY_CPU_LE_UNALIGN*/
  #endif
#endif


#ifdef MY_CPU_LE_UNALIGN

#define GetUi16(p) (*(const UInt16 *)(const void *)(p))
#define GetUi32(p) (*(const UInt32 *)(const void *)(p))
#define GetUi64(p) (*(const UInt64 *)(const void *)(p))

#define SetUi16(p, v) { *(UInt16 *)(p) = (v); }
#define SetUi32(p, v) { *(UInt32 *)(p) = (v); }
#define SetUi64(p, v) { *(UInt64 *)(p) = (v); }

#else

#define GetUi16(p) ( (UInt16) ( \
             ((const Byte *)(p))[0] | \
    ((UInt16)((const Byte *)(p))[1] << 8) ))

#define GetUi32(p) ( \
             ((const Byte *)(p))[0]        | \
    ((UInt32)((const Byte *)(p))[1] <<  8) | \
    ((UInt32)((const Byte *)(p))[2] << 16) | \
    ((UInt32)((const Byte *)(p))[3] << 24))

#define GetUi64(p) (GetUi32(p) | ((UInt64)GetUi32(((const Byte *)(p)) + 4) << 32))

#define SetUi16(p, v) { Byte *_ppp_ = (Byte *)(p); UInt32 _vvv_ = (v); \
    _ppp_[0] = (Byte)_vvv_; \
    _ppp_[1] = (Byte)(_vvv_ >> 8); }

#define SetUi32(p, v) { Byte *_ppp_ = (Byte *)(p); UInt32 _vvv_ = (v); \
    _ppp_[0] = (Byte)_vvv_; \
    _ppp_[1] = (Byte)(_vvv_ >> 8); \
    _ppp_[2] = (Byte)(_vvv_ >> 16); \
    _ppp_[3] = (Byte)(_vvv_ >> 24); }

#define SetUi64(p, v) { Byte *_ppp2_ = (Byte *)(p); UInt64 _vvv2_ = (v); \
    SetUi32(_ppp2_    , (UInt32)_vvv2_); \
    SetUi32(_ppp2_ + 4, (UInt32)(_vvv2_ >> 32)); }

#endif


#if defined(MY_CPU_LE_UNALIGN) && /* defined(_WIN64) && */ (_MSC_VER >= 1300)

/* Note: we use bswap instruction, that is unsupported in 386 cpu */

#include <stdlib.h>

#pragma intrinsic(_byteswap_ulong)
#pragma intrinsic(_byteswap_uint64)
#define GetBe32(p) _byteswap_ulong(*(const UInt32 *)(const Byte *)(p))
#define GetBe64(p) _byteswap_uint64(*(const UInt64 *)(const Byte *)(p))

#define SetBe32(p, v) (*(UInt32 *)(void *)(p)) = _byteswap_ulong(v)

#elif defined(MY_CPU_LE_UNALIGN) && defined (__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))

#define GetBe32(p) __builtin_bswap32(*(const UInt32 *)(const Byte *)(p))
#define GetBe64(p) __builtin_bswap64(*(const UInt64 *)(const Byte *)(p))

#define SetBe32(p, v) (*(UInt32 *)(void *)(p)) = __builtin_bswap32(v)

#else

#define GetBe32(p) ( \
    ((UInt32)((const Byte *)(p))[0] << 24) | \
    ((UInt32)((const Byte *)(p))[1] << 16) | \
    ((UInt32)((const Byte *)(p))[2] <<  8) | \
             ((const Byte *)(p))[3] )

#define GetBe64(p) (((UInt64)GetBe32(p) << 32) | GetBe32(((const Byte *)(p)) + 4))

#define SetBe32(p, v) { Byte *_ppp_ = (Byte *)(p); UInt32 _vvv_ = (v); \
    _ppp_[0] = (Byte)(_vvv_ >> 24); \
    _ppp_[1] = (Byte)(_vvv_ >> 16); \
    _ppp_[2] = (Byte)(_vvv_ >> 8); \
    _ppp_[3] = (Byte)_vvv_; }

#endif


#define GetBe16(p) ( (UInt16) ( \
    ((UInt16)((const Byte *)(p))[0] << 8) | \
             ((const Byte *)(p))[1] ))



#ifdef MY_CPU_X86_OR_AMD64

typedef struct
{
  UInt32 maxFunc;
  UInt32 vendor[3];
  UInt32 ver;
  UInt32 b;
  UInt32 c;
  UInt32 d;
} Cx86cpuid;

enum
{
  CPU_FIRM_INTEL,
  CPU_FIRM_AMD,
  CPU_FIRM_VIA
};

static void MyCPUID(UInt32 function, UInt32 *a, UInt32 *b, UInt32 *c, UInt32 *d);

static Bool x86cpuid_CheckAndRead(Cx86cpuid *p);
static int x86cpuid_GetFirm(const Cx86cpuid *p);

#define x86cpuid_GetFamily(ver) (((ver >> 16) & 0xFF0) | ((ver >> 8) & 0xF))
#define x86cpuid_GetModel(ver)  (((ver >> 12) &  0xF0) | ((ver >> 4) & 0xF))
#define x86cpuid_GetStepping(ver) (ver & 0xF)

static Bool CPU_Is_InOrder();

#endif

EXTERN_C_END

#endif

/* 7zBuf.h -- Byte Buffer
2013-01-18 : Igor Pavlov : Public domain */

#ifndef __7Z_BUF_H
#define __7Z_BUF_H

/*#include "7zTypes.h" */

EXTERN_C_BEGIN

typedef struct
{
  Byte *data;
  size_t size;
} CBuf;

static void Buf_Init(CBuf *p);
static int Buf_Create(CBuf *p, size_t size, ISzAlloc *alloc);
static void Buf_Free(CBuf *p, ISzAlloc *alloc);

EXTERN_C_END

#endif


/* Bcj2.h -- BCJ2 Converter for x86 code
2014-11-10 : Igor Pavlov : Public domain */

#ifndef __BCJ2_H
#define __BCJ2_H

/*#include "7zTypes.h" */

EXTERN_C_BEGIN

#define BCJ2_NUM_STREAMS 4

enum
{
  BCJ2_STREAM_MAIN,
  BCJ2_STREAM_CALL,
  BCJ2_STREAM_JUMP,
  BCJ2_STREAM_RC
};

enum
{
  BCJ2_DEC_STATE_ORIG_0 = BCJ2_NUM_STREAMS,
  BCJ2_DEC_STATE_ORIG_1,
  BCJ2_DEC_STATE_ORIG_2,
  BCJ2_DEC_STATE_ORIG_3,
  
  BCJ2_DEC_STATE_ORIG,
  BCJ2_DEC_STATE_OK
};

enum
{
  BCJ2_ENC_STATE_ORIG = BCJ2_NUM_STREAMS,
  BCJ2_ENC_STATE_OK
};


#define BCJ2_IS_32BIT_STREAM(s) ((s) == BCJ2_STREAM_CALL || (s) == BCJ2_STREAM_JUMP)

/*
CBcj2Dec / CBcj2Enc
bufs sizes:
  BUF_SIZE(n) = lims[n] - bufs[n]
bufs sizes for BCJ2_STREAM_CALL and BCJ2_STREAM_JUMP must be mutliply of 4:
    (BUF_SIZE(BCJ2_STREAM_CALL) & 3) == 0
    (BUF_SIZE(BCJ2_STREAM_JUMP) & 3) == 0
*/

/*
CBcj2Dec:
dest is allowed to overlap with bufs[BCJ2_STREAM_MAIN], with the following conditions:
  bufs[BCJ2_STREAM_MAIN] >= dest &&
  bufs[BCJ2_STREAM_MAIN] - dest >= tempReserv +
        BUF_SIZE(BCJ2_STREAM_CALL) +
        BUF_SIZE(BCJ2_STREAM_JUMP)
     tempReserv = 0 : for first call of Bcj2Dec_Decode
     tempReserv = 4 : for any other calls of Bcj2Dec_Decode
  overlap with offset = 1 is not allowed
*/

typedef struct
{
  const Byte *bufs[BCJ2_NUM_STREAMS];
  const Byte *lims[BCJ2_NUM_STREAMS];
  Byte *dest;
  const Byte *destLim;

  unsigned state; /* BCJ2_STREAM_MAIN has more priority than BCJ2_STATE_ORIG */

  UInt32 ip;
  Byte temp[4];
  UInt32 range;
  UInt32 code;
  UInt16 probs[2 + 256];
} CBcj2Dec;

static void Bcj2Dec_Init(CBcj2Dec *p);

/* Returns: SZ_OK or SZ_ERROR_DATA */
static SRes Bcj2Dec_Decode(CBcj2Dec *p);

#define Bcj2Dec_IsFinished(_p_) ((_p_)->code == 0)

#define BCJ2_RELAT_LIMIT_NUM_BITS 26
#define BCJ2_RELAT_LIMIT ((UInt32)1 << BCJ2_RELAT_LIMIT_NUM_BITS)

/* limit for CBcj2Enc::fileSize variable */
#define BCJ2_FileSize_MAX ((UInt32)1 << 31)

EXTERN_C_END

#endif

/* Bra.h -- Branch converters for executables
2013-01-18 : Igor Pavlov : Public domain */

#ifndef __BRA_H
#define __BRA_H

/*#include "7zTypes.h"*/

EXTERN_C_BEGIN

/*
These functions convert relative addresses to absolute addresses
in CALL instructions to increase the compression ratio.
  
  In:
    data     - data buffer
    size     - size of data
    ip       - current virtual Instruction Pinter (IP) value
    state    - state variable for x86 converter
    encoding - 0 (for decoding), 1 (for encoding)
  
  Out:
    state    - state variable for x86 converter

  Returns:
    The number of processed bytes. If you call these functions with multiple calls,
    you must start next call with first byte after block of processed bytes.
  
  Type   Endian  Alignment  LookAhead
  
  x86    little      1          4
  ARMT   little      2          2
  ARM    little      4          0
  PPC     big        4          0
  SPARC   big        4          0
  IA64   little     16          0

  size must be >= Alignment + LookAhead, if it's not last block.
  If (size < Alignment + LookAhead), converter returns 0.

  Example:

    UInt32 ip = 0;
    for ()
    {
      ; size must be >= Alignment + LookAhead, if it's not last block
      SizeT processed = Convert(data, size, ip, 1);
      data += processed;
      size -= processed;
      ip += processed;
    }
*/

#define x86_Convert_Init(state) { state = 0; }
static SizeT x86_Convert(Byte *data, SizeT size, UInt32 ip, UInt32 *state, int encoding);
static SizeT ARM_Convert(Byte *data, SizeT size, UInt32 ip, int encoding);
static SizeT ARMT_Convert(Byte *data, SizeT size, UInt32 ip, int encoding);
static SizeT PPC_Convert(Byte *data, SizeT size, UInt32 ip, int encoding);
static SizeT SPARC_Convert(Byte *data, SizeT size, UInt32 ip, int encoding);
static SizeT IA64_Convert(Byte *data, SizeT size, UInt32 ip, int encoding);

EXTERN_C_END

#endif

/* Delta.h -- Delta converter
2013-01-18 : Igor Pavlov : Public domain */

#ifndef __DELTA_H
#define __DELTA_H

/*#include "7zTypes.h" */

EXTERN_C_BEGIN

#define DELTA_STATE_SIZE 256

static void Delta_Init(Byte *state);
static void Delta_Decode(Byte *state, unsigned delta, Byte *data, SizeT size);

EXTERN_C_END

#endif

/* LzmaDec.h -- LZMA Decoder
2013-01-18 : Igor Pavlov : Public domain */

#ifndef __LZMA_DEC_H
#define __LZMA_DEC_H

/*#include "7zTypes.h"*/

EXTERN_C_BEGIN

/* #define _LZMA_PROB32 */
/* _LZMA_PROB32 can increase the speed on some CPUs,
   but memory usage for CLzmaDec::probs will be doubled in that case */

#ifdef _LZMA_PROB32
#define CLzmaProb UInt32
#else
#define CLzmaProb UInt16
#endif


/* ---------- LZMA Properties ---------- */

#define LZMA_PROPS_SIZE 5

typedef struct _CLzmaProps
{
  unsigned lc, lp, pb;
  UInt32 dicSize;
} CLzmaProps;

/* LzmaProps_Decode - decodes properties
Returns:
  SZ_OK
  SZ_ERROR_UNSUPPORTED - Unsupported properties
*/

static SRes LzmaProps_Decode(CLzmaProps *p, const Byte *data, unsigned size);


/* ---------- LZMA Decoder state ---------- */

/* LZMA_REQUIRED_INPUT_MAX = number of required input bytes for worst case.
   Num bits = log2((2^11 / 31) ^ 22) + 26 < 134 + 26 = 160; */

#define LZMA_REQUIRED_INPUT_MAX 20

typedef struct
{
  CLzmaProps prop;
  CLzmaProb *probs;
  Byte *dic;
  const Byte *buf;
  UInt32 range, code;
  SizeT dicPos;
  SizeT dicBufSize;
  UInt32 processedPos;
  UInt32 checkDicSize;
  unsigned state;
  UInt32 reps[4];
  unsigned remainLen;
  int needFlush;
  int needInitState;
  UInt32 numProbs;
  unsigned tempBufSize;
  Byte tempBuf[LZMA_REQUIRED_INPUT_MAX];
} CLzmaDec;

#define LzmaDec_Construct(p) { (p)->dic = 0; (p)->probs = 0; }

static void LzmaDec_Init(CLzmaDec *p);

/* There are two types of LZMA streams:
     0) Stream with end mark. That end mark adds about 6 bytes to compressed size.
     1) Stream without end mark. You must know exact uncompressed size to decompress such stream. */

typedef enum
{
  LZMA_FINISH_ANY,   /* finish at any point */
  LZMA_FINISH_END    /* block must be finished at the end */
} ELzmaFinishMode;

/* ELzmaFinishMode has meaning only if the decoding reaches output limit !!!

   You must use LZMA_FINISH_END, when you know that current output buffer
   covers last bytes of block. In other cases you must use LZMA_FINISH_ANY.

   If LZMA decoder sees end marker before reaching output limit, it returns SZ_OK,
   and output value of destLen will be less than output buffer size limit.
   You can check status result also.

   You can use multiple checks to test data integrity after full decompression:
     1) Check Result and "status" variable.
     2) Check that output(destLen) = uncompressedSize, if you know real uncompressedSize.
     3) Check that output(srcLen) = compressedSize, if you know real compressedSize.
        You must use correct finish mode in that case. */

typedef enum
{
  LZMA_STATUS_NOT_SPECIFIED,               /* use main error code instead */
  LZMA_STATUS_FINISHED_WITH_MARK,          /* stream was finished with end mark. */
  LZMA_STATUS_NOT_FINISHED,                /* stream was not finished */
  LZMA_STATUS_NEEDS_MORE_INPUT,            /* you must provide more input bytes */
  LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK  /* there is probability that stream was finished without end mark */
} ELzmaStatus;

/* ELzmaStatus is used only as output value for function call */


/* ---------- Interfaces ---------- */

/* There are 3 levels of interfaces:
     1) Dictionary Interface
     2) Buffer Interface
     3) One Call Interface
   You can select any of these interfaces, but don't mix functions from different
   groups for same object. */


/* There are two variants to allocate state for Dictionary Interface:
     1) LzmaDec_Allocate / LzmaDec_Free
     2) LzmaDec_AllocateProbs / LzmaDec_FreeProbs
   You can use variant 2, if you set dictionary buffer manually.
   For Buffer Interface you must always use variant 1.

LzmaDec_Allocate* can return:
  SZ_OK
  SZ_ERROR_MEM         - Memory allocation error
  SZ_ERROR_UNSUPPORTED - Unsupported properties
*/
   
static SRes LzmaDec_AllocateProbs(CLzmaDec *p, const Byte *props, unsigned propsSize, ISzAlloc *alloc);
static void LzmaDec_FreeProbs(CLzmaDec *p, ISzAlloc *alloc);

/* ---------- Dictionary Interface ---------- */

/* You can use it, if you want to eliminate the overhead for data copying from
   dictionary to some other external buffer.
   You must work with CLzmaDec variables directly in this interface.

   STEPS:
     LzmaDec_Constr()
     LzmaDec_Allocate()
     for (each new stream)
     {
       LzmaDec_Init()
       while (it needs more decompression)
       {
         LzmaDec_DecodeToDic()
         use data from CLzmaDec::dic and update CLzmaDec::dicPos
       }
     }
     LzmaDec_Free()
*/

/* LzmaDec_DecodeToDic
   
   The decoding to internal dictionary buffer (CLzmaDec::dic).
   You must manually update CLzmaDec::dicPos, if it reaches CLzmaDec::dicBufSize !!!

finishMode:
  It has meaning only if the decoding reaches output limit (dicLimit).
  LZMA_FINISH_ANY - Decode just dicLimit bytes.
  LZMA_FINISH_END - Stream must be finished after dicLimit.

Returns:
  SZ_OK
    status:
      LZMA_STATUS_FINISHED_WITH_MARK
      LZMA_STATUS_NOT_FINISHED
      LZMA_STATUS_NEEDS_MORE_INPUT
      LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK
  SZ_ERROR_DATA - Data error
*/

static SRes LzmaDec_DecodeToDic(CLzmaDec *p, SizeT dicLimit,
    const Byte *src, SizeT *srcLen, ELzmaFinishMode finishMode, ELzmaStatus *status);

EXTERN_C_END

#endif

/* Lzma2Dec.h -- LZMA2 Decoder
2015-05-13 : Igor Pavlov : Public domain */

#ifndef __LZMA2_DEC_H
#define __LZMA2_DEC_H

/*#include "LzmaDec.h"*/

EXTERN_C_BEGIN

/* ---------- State Interface ---------- */

typedef struct
{
  CLzmaDec decoder;
  UInt32 packSize;
  UInt32 unpackSize;
  unsigned state;
  Byte control;
  Bool needInitDic;
  Bool needInitState;
  Bool needInitProp;
} CLzma2Dec;

#define Lzma2Dec_Construct(p) LzmaDec_Construct(&(p)->decoder)
#define Lzma2Dec_FreeProbs(p, alloc) LzmaDec_FreeProbs(&(p)->decoder, alloc);
#define Lzma2Dec_Free(p, alloc) LzmaDec_Free(&(p)->decoder, alloc);

static SRes Lzma2Dec_AllocateProbs(CLzma2Dec *p, Byte prop, ISzAlloc *alloc);
static void Lzma2Dec_Init(CLzma2Dec *p);


/*
finishMode:
  It has meaning only if the decoding reaches output limit (*destLen or dicLimit).
  LZMA_FINISH_ANY - use smallest number of input bytes
  LZMA_FINISH_END - read EndOfStream marker after decoding

Returns:
  SZ_OK
    status:
      LZMA_STATUS_FINISHED_WITH_MARK
      LZMA_STATUS_NOT_FINISHED
      LZMA_STATUS_NEEDS_MORE_INPUT
  SZ_ERROR_DATA - Data error
*/

static SRes Lzma2Dec_DecodeToDic(CLzma2Dec *p, SizeT dicLimit,
    const Byte *src, SizeT *srcLen, ELzmaFinishMode finishMode, ELzmaStatus *status);


EXTERN_C_END

#endif


/* END HEADERS */


/* 7zCrc.c -- CRC32 init
2015-03-10 : Igor Pavlov : Public domain */

/*
#include "Precomp.h"

#include "7zCrc.h"
#include "CpuArch.h"
*/
#define UNUSED_VAR(x) (void)x;

#define kCrcPoly 0xEDB88320

#ifdef MY_CPU_LE
  #define CRC_NUM_TABLES 8
#else
  #define CRC_NUM_TABLES 9

  #define CRC_UINT32_SWAP(v) ((v >> 24) | ((v >> 8) & 0xFF00) | ((v << 8) & 0xFF0000) | (v << 24))

  static UInt32 MY_FAST_CALL CrcUpdateT1_BeT4(UInt32 v, const void *data, size_t size, const UInt32 *table);
  static UInt32 MY_FAST_CALL CrcUpdateT1_BeT8(UInt32 v, const void *data, size_t size, const UInt32 *table);
#endif

#ifndef MY_CPU_BE
  static UInt32 MY_FAST_CALL CrcUpdateT4(UInt32 v, const void *data, size_t size, const UInt32 *table);
  static UInt32 MY_FAST_CALL CrcUpdateT8(UInt32 v, const void *data, size_t size, const UInt32 *table);
#endif

typedef UInt32 (MY_FAST_CALL *CRC_FUNC)(UInt32 v, const void *data, size_t size, const UInt32 *table);

static CRC_FUNC g_CrcUpdateT4;
static CRC_FUNC g_CrcUpdateT8;
static CRC_FUNC g_CrcUpdate;

static UInt32 g_CrcTable[256 * CRC_NUM_TABLES];

static UInt32 MY_FAST_CALL CrcCalc(const void *data, size_t size)
{
  return g_CrcUpdate(CRC_INIT_VAL, data, size, g_CrcTable) ^ CRC_INIT_VAL;
}

#define CRC_UPDATE_BYTE_2(crc, b) (table[((crc) ^ (b)) & 0xFF] ^ ((crc) >> 8))

#if CRC_NUM_TABLES < 4
static UInt32 MY_FAST_CALL CrcUpdateT1(UInt32 v, const void *data, size_t size, const UInt32 *table)
{
  const Byte *p = (const Byte *)data;
  const Byte *pEnd = p + size;
  for (; p != pEnd; p++)
    v = CRC_UPDATE_BYTE_2(v, *p);
  return v;
}
#endif

static void MY_FAST_CALL CrcGenerateTable()
{
  UInt32 i;
  for (i = 0; i < 256; i++)
  {
    UInt32 r = i;
    unsigned j;
    for (j = 0; j < 8; j++)
      r = (r >> 1) ^ (kCrcPoly & ~((r & 1) - 1));
    g_CrcTable[i] = r;
  }
  for (; i < 256 * CRC_NUM_TABLES; i++)
  {
    UInt32 r = g_CrcTable[i - 256];
    g_CrcTable[i] = g_CrcTable[r & 0xFF] ^ (r >> 8);
  }

  #if CRC_NUM_TABLES < 4
  
  g_CrcUpdate = CrcUpdateT1;
  
  #else
 
  #ifdef MY_CPU_LE

    g_CrcUpdateT4 = CrcUpdateT4;
    g_CrcUpdate = CrcUpdateT4;

    #if CRC_NUM_TABLES >= 8
      g_CrcUpdateT8 = CrcUpdateT8;
  
      #ifdef MY_CPU_X86_OR_AMD64
      if (!CPU_Is_InOrder())
        g_CrcUpdate = CrcUpdateT8;
      #endif
    #endif

  #else
  {
    #ifndef MY_CPU_BE
    UInt32 k = 0x01020304;
    const Byte *p = (const Byte *)&k;
    if (p[0] == 4 && p[1] == 3)
    {
      g_CrcUpdateT4 = CrcUpdateT4;
      g_CrcUpdate = CrcUpdateT4;
      #if CRC_NUM_TABLES >= 8
      g_CrcUpdateT8 = CrcUpdateT8;
      /* g_CrcUpdate = CrcUpdateT8; */
      #endif
    }
    else if (p[0] != 1 || p[1] != 2)
      g_CrcUpdate = CrcUpdateT1;
    else
    #endif
    {
      for (i = 256 * CRC_NUM_TABLES - 1; i >= 256; i--)
      {
        UInt32 x = g_CrcTable[i - 256];
        g_CrcTable[i] = CRC_UINT32_SWAP(x);
      }
      g_CrcUpdateT4 = CrcUpdateT1_BeT4;
      g_CrcUpdate = CrcUpdateT1_BeT4;
      #if CRC_NUM_TABLES >= 8
      g_CrcUpdateT8 = CrcUpdateT1_BeT8;
      /* g_CrcUpdate = CrcUpdateT1_BeT8; */
      #endif
    }
  }
  #endif

  #endif
}

/* 7zCrcOpt.c -- CRC32 calculation
2015-03-01 : Igor Pavlov : Public domain */

/*
#include "Precomp.h"

#include "CpuArch.h"
*/

#ifndef MY_CPU_BE

#define CRC_UPDATE_BYTE_2(crc, b) (table[((crc) ^ (b)) & 0xFF] ^ ((crc) >> 8))

static UInt32 MY_FAST_CALL CrcUpdateT4(UInt32 v, const void *data, size_t size, const UInt32 *table)
{
  const Byte *p = (const Byte *)data;
  for (; size > 0 && ((unsigned)(ptrdiff_t)p & 3) != 0; size--, p++)
    v = CRC_UPDATE_BYTE_2(v, *p);
  for (; size >= 4; size -= 4, p += 4)
  {
    v ^= *(const UInt32 *)p;
    v =
          table[0x300 + ((v      ) & 0xFF)]
        ^ table[0x200 + ((v >>  8) & 0xFF)]
        ^ table[0x100 + ((v >> 16) & 0xFF)]
        ^ table[0x000 + ((v >> 24))];
  }
  for (; size > 0; size--, p++)
    v = CRC_UPDATE_BYTE_2(v, *p);
  return v;
}

static UInt32 MY_FAST_CALL CrcUpdateT8(UInt32 v, const void *data, size_t size, const UInt32 *table)
{
  const Byte *p = (const Byte *)data;
  for (; size > 0 && ((unsigned)(ptrdiff_t)p & 7) != 0; size--, p++)
    v = CRC_UPDATE_BYTE_2(v, *p);
  for (; size >= 8; size -= 8, p += 8)
  {
    UInt32 d;
    v ^= *(const UInt32 *)p;
    v =
          table[0x700 + ((v      ) & 0xFF)]
        ^ table[0x600 + ((v >>  8) & 0xFF)]
        ^ table[0x500 + ((v >> 16) & 0xFF)]
        ^ table[0x400 + ((v >> 24))];
    d = *((const UInt32 *)p + 1);
    v ^=
          table[0x300 + ((d      ) & 0xFF)]
        ^ table[0x200 + ((d >>  8) & 0xFF)]
        ^ table[0x100 + ((d >> 16) & 0xFF)]
        ^ table[0x000 + ((d >> 24))];
  }
  for (; size > 0; size--, p++)
    v = CRC_UPDATE_BYTE_2(v, *p);
  return v;
}

#endif


#ifndef MY_CPU_LE

#define CRC_UINT32_SWAP(v) ((v >> 24) | ((v >> 8) & 0xFF00) | ((v << 8) & 0xFF0000) | (v << 24))

#define CRC_UPDATE_BYTE_2_BE(crc, b) (table[(((crc) >> 24) ^ (b))] ^ ((crc) << 8))

static UInt32 MY_FAST_CALL CrcUpdateT1_BeT4(UInt32 v, const void *data, size_t size, const UInt32 *table)
{
  const Byte *p = (const Byte *)data;
  table += 0x100;
  v = CRC_UINT32_SWAP(v);
  for (; size > 0 && ((unsigned)(ptrdiff_t)p & 3) != 0; size--, p++)
    v = CRC_UPDATE_BYTE_2_BE(v, *p);
  for (; size >= 4; size -= 4, p += 4)
  {
    v ^= *(const UInt32 *)p;
    v =
          table[0x000 + ((v      ) & 0xFF)]
        ^ table[0x100 + ((v >>  8) & 0xFF)]
        ^ table[0x200 + ((v >> 16) & 0xFF)]
        ^ table[0x300 + ((v >> 24))];
  }
  for (; size > 0; size--, p++)
    v = CRC_UPDATE_BYTE_2_BE(v, *p);
  return CRC_UINT32_SWAP(v);
}

static UInt32 MY_FAST_CALL CrcUpdateT1_BeT8(UInt32 v, const void *data, size_t size, const UInt32 *table)
{
  const Byte *p = (const Byte *)data;
  table += 0x100;
  v = CRC_UINT32_SWAP(v);
  for (; size > 0 && ((unsigned)(ptrdiff_t)p & 7) != 0; size--, p++)
    v = CRC_UPDATE_BYTE_2_BE(v, *p);
  for (; size >= 8; size -= 8, p += 8)
  {
    UInt32 d;
    v ^= *(const UInt32 *)p;
    v =
          table[0x400 + ((v      ) & 0xFF)]
        ^ table[0x500 + ((v >>  8) & 0xFF)]
        ^ table[0x600 + ((v >> 16) & 0xFF)]
        ^ table[0x700 + ((v >> 24))];
    d = *((const UInt32 *)p + 1);
    v ^=
          table[0x000 + ((d      ) & 0xFF)]
        ^ table[0x100 + ((d >>  8) & 0xFF)]
        ^ table[0x200 + ((d >> 16) & 0xFF)]
        ^ table[0x300 + ((d >> 24))];
  }
  for (; size > 0; size--, p++)
    v = CRC_UPDATE_BYTE_2_BE(v, *p);
  return CRC_UINT32_SWAP(v);
}

#endif

/* CpuArch.c -- CPU specific code
2016-02-25: Igor Pavlov : Public domain */

/*
#include "Precomp.h"

#include "CpuArch.h"
*/

#ifdef MY_CPU_X86_OR_AMD64

#if (defined(_MSC_VER) && !defined(MY_CPU_AMD64)) || defined(__GNUC__)
#define USE_ASM
#endif

#if !defined(USE_ASM) && _MSC_VER >= 1500
#include <intrin.h>
#endif

#if defined(USE_ASM) && !defined(MY_CPU_AMD64)
static UInt32 CheckFlag(UInt32 flag)
{
  #ifdef _MSC_VER
  __asm pushfd;
  __asm pop EAX;
  __asm mov EDX, EAX;
  __asm xor EAX, flag;
  __asm push EAX;
  __asm popfd;
  __asm pushfd;
  __asm pop EAX;
  __asm xor EAX, EDX;
  __asm push EDX;
  __asm popfd;
  __asm and flag, EAX;
  #else
  __asm__ __volatile__ (
    "pushf\n\t"
    "pop  %%EAX\n\t"
    "movl %%EAX,%%EDX\n\t"
    "xorl %0,%%EAX\n\t"
    "push %%EAX\n\t"
    "popf\n\t"
    "pushf\n\t"
    "pop  %%EAX\n\t"
    "xorl %%EDX,%%EAX\n\t"
    "push %%EDX\n\t"
    "popf\n\t"
    "andl %%EAX, %0\n\t":
    "=c" (flag) : "c" (flag) :
    "%eax", "%edx");
  #endif
  return flag;
}
#define CHECK_CPUID_IS_SUPPORTED if (CheckFlag(1 << 18) == 0 || CheckFlag(1 << 21) == 0) return False;
#else
#define CHECK_CPUID_IS_SUPPORTED
#endif

#if defined(__WATCOMC__)
static void __cpuid(int *cpuinfo, const UInt32 infotype);
#pragma aux __cpuid =     \
    ".586"                \
    "cpuid"               \
    "mov  [esi+0],eax"    \
    "mov  [esi+4],ebx"    \
    "mov  [esi+8],ecx"    \
    "mov  [esi+12],edx"   \
    parm [esi] [eax] modify [ebx ecx edx];
#endif


static void MyCPUID(UInt32 function, UInt32 *a, UInt32 *b, UInt32 *c, UInt32 *d)
{
  #ifdef USE_ASM

  #ifdef _MSC_VER

  UInt32 a2, b2, c2, d2;
  __asm xor EBX, EBX;
  __asm xor ECX, ECX;
  __asm xor EDX, EDX;
  __asm mov EAX, function;
  __asm cpuid;
  __asm mov a2, EAX;
  __asm mov b2, EBX;
  __asm mov c2, ECX;
  __asm mov d2, EDX;

  *a = a2;
  *b = b2;
  *c = c2;
  *d = d2;

  #else

  __asm__ __volatile__ (
  #if defined(MY_CPU_AMD64) && defined(__PIC__)
    "mov %%rbx, %%rdi;"
    "cpuid;"
    "xchg %%rbx, %%rdi;"
    : "=a" (*a) ,
      "=D" (*b) ,
  #elif defined(MY_CPU_X86) && defined(__PIC__)
    "mov %%ebx, %%edi;"
    "cpuid;"
    "xchgl %%ebx, %%edi;"
    : "=a" (*a) ,
      "=D" (*b) ,
  #else
    "cpuid"
    : "=a" (*a) ,
      "=b" (*b) ,
  #endif
      "=c" (*c) ,
      "=d" (*d)
    : "0" (function)) ;

  #endif

  #else

  int CPUInfo[4];
  __cpuid(CPUInfo, function);
  *a = CPUInfo[0];
  *b = CPUInfo[1];
  *c = CPUInfo[2];
  *d = CPUInfo[3];

  #endif
}

static Bool x86cpuid_CheckAndRead(Cx86cpuid *p)
{
  CHECK_CPUID_IS_SUPPORTED
  MyCPUID(0, &p->maxFunc, &p->vendor[0], &p->vendor[2], &p->vendor[1]);
  MyCPUID(1, &p->ver, &p->b, &p->c, &p->d);
  return True;
}

static const UInt32 kVendors[][3] =
{
  { 0x756E6547, 0x49656E69, 0x6C65746E},
  { 0x68747541, 0x69746E65, 0x444D4163},
  { 0x746E6543, 0x48727561, 0x736C7561}
};

static int x86cpuid_GetFirm(const Cx86cpuid *p)
{
  unsigned i;
  for (i = 0; i < sizeof(kVendors) / sizeof(kVendors[i]); i++)
  {
    const UInt32 *v = kVendors[i];
    if (v[0] == p->vendor[0] &&
        v[1] == p->vendor[1] &&
        v[2] == p->vendor[2])
      return (int)i;
  }
  return -1;
}

static Bool CPU_Is_InOrder()
{
  Cx86cpuid p;
  int firm;
  UInt32 family, model;
  if (!x86cpuid_CheckAndRead(&p))
    return True;

  family = x86cpuid_GetFamily(p.ver);
  model = x86cpuid_GetModel(p.ver);
  
  firm = x86cpuid_GetFirm(&p);

  switch (firm)
  {
    case CPU_FIRM_INTEL: return (family < 6 || (family == 6 && (
        /* In-Order Atom CPU */
           model == 0x1C  /* 45 nm, N4xx, D4xx, N5xx, D5xx, 230, 330 */
        || model == 0x26  /* 45 nm, Z6xx */
        || model == 0x27  /* 32 nm, Z2460 */
        || model == 0x35  /* 32 nm, Z2760 */
        || model == 0x36  /* 32 nm, N2xxx, D2xxx */
        )));
    case CPU_FIRM_AMD: return (family < 5 || (family == 5 && (model < 6 || model == 0xA)));
    case CPU_FIRM_VIA: return (family < 6 || (family == 6 && model < 0xF));
  }
  return True;
}

#endif

/* 7zStream.c -- 7z Stream functions
2013-11-12 : Igor Pavlov : Public domain */

/*#include "Precomp.h"*/

#include <string.h>

/*#include "7zTypes.h"*/

static SRes LookInStream_SeekTo(ILookInStream *stream, UInt64 offset)
{
  Int64 t = offset;
  return stream->Seek(stream, &t, SZ_SEEK_SET);
}

static SRes LookInStream_Read2(ILookInStream *stream, void *buf, size_t size, SRes errorType)
{
  while (size != 0)
  {
    size_t processed = size;
    RINOK(stream->Read(stream, buf, &processed));
    if (processed == 0)
      return errorType;
    buf = (void *)((Byte *)buf + processed);
    size -= processed;
  }
  return SZ_OK;
}

static SRes LookInStream_Read(ILookInStream *stream, void *buf, size_t size)
{
  return LookInStream_Read2(stream, buf, size, SZ_ERROR_INPUT_EOF);
}

static SRes LookToRead_Look_Lookahead(void *pp, const void **buf, size_t *size)
{
  SRes res = SZ_OK;
  CLookToRead *p = (CLookToRead *)pp;
  size_t size2 = p->size - p->pos;
  if (size2 == 0 && *size > 0)
  {
    p->pos = 0;
    size2 = LookToRead_BUF_SIZE;
    res = p->realStream->Read(p->realStream, p->buf, &size2);
    p->size = size2;
  }
  if (size2 < *size)
    *size = size2;
  *buf = p->buf + p->pos;
  return res;
}

static SRes LookToRead_Look_Exact(void *pp, const void **buf, size_t *size)
{
  SRes res = SZ_OK;
  CLookToRead *p = (CLookToRead *)pp;
  size_t size2 = p->size - p->pos;
  if (size2 == 0 && *size > 0)
  {
    p->pos = 0;
    if (*size > LookToRead_BUF_SIZE)
      *size = LookToRead_BUF_SIZE;
    res = p->realStream->Read(p->realStream, p->buf, size);
    size2 = p->size = *size;
  }
  if (size2 < *size)
    *size = size2;
  *buf = p->buf + p->pos;
  return res;
}

static SRes LookToRead_Skip(void *pp, size_t offset)
{
  CLookToRead *p = (CLookToRead *)pp;
  p->pos += offset;
  return SZ_OK;
}

static SRes LookToRead_Read(void *pp, void *buf, size_t *size)
{
  CLookToRead *p = (CLookToRead *)pp;
  size_t rem = p->size - p->pos;
  if (rem == 0)
    return p->realStream->Read(p->realStream, buf, size);
  if (rem > *size)
    rem = *size;
  memcpy(buf, p->buf + p->pos, rem);
  p->pos += rem;
  *size = rem;
  return SZ_OK;
}

static SRes LookToRead_Seek(void *pp, Int64 *pos, ESzSeek origin)
{
  CLookToRead *p = (CLookToRead *)pp;
  p->pos = p->size = 0;
  return p->realStream->Seek(p->realStream, pos, origin);
}

static void LookToRead_CreateVTable(CLookToRead *p, int lookahead)
{
  p->s.Look = lookahead ?
      LookToRead_Look_Lookahead :
      LookToRead_Look_Exact;
  p->s.Skip = LookToRead_Skip;
  p->s.Read = LookToRead_Read;
  p->s.Seek = LookToRead_Seek;
}

static void LookToRead_Init(CLookToRead *p)
{
  p->pos = p->size = 0;
}


/* 7zArcIn.c -- 7z Input functions
2016-05-16 : Igor Pavlov : Public domain */

/*
#include "Precomp.h"

#include <string.h>

#include "7z.h"
#include "7zBuf.h"
#include "7zCrc.h"
#include "CpuArch.h"
*/

#define MY_ALLOC(T, p, size, alloc) { \
  if ((p = (T *)IAlloc_Alloc(alloc, (size) * sizeof(T))) == NULL) return SZ_ERROR_MEM; }

#define MY_ALLOC_ZE(T, p, size, alloc) { if ((size) == 0) p = NULL; else MY_ALLOC(T, p, size, alloc) }

#define MY_ALLOC_AND_CPY(to, size, from, alloc) \
  { MY_ALLOC(Byte, to, size, alloc); memcpy(to, from, size); }

#define MY_ALLOC_ZE_AND_CPY(to, size, from, alloc) \
  { if ((size) == 0) p = NULL; else { MY_ALLOC_AND_CPY(to, size, from, alloc) } }

#define k7zMajorVersion 0

enum EIdEnum
{
  k7zIdEnd,
  k7zIdHeader,
  k7zIdArchiveProperties,
  k7zIdAdditionalStreamsInfo,
  k7zIdMainStreamsInfo,
  k7zIdFilesInfo,
  k7zIdPackInfo,
  k7zIdUnpackInfo,
  k7zIdSubStreamsInfo,
  k7zIdSize,
  k7zIdCRC,
  k7zIdFolder,
  k7zIdCodersUnpackSize,
  k7zIdNumUnpackStream,
  k7zIdEmptyStream,
  k7zIdEmptyFile,
  k7zIdAnti,
  k7zIdName,
  k7zIdCTime,
  k7zIdATime,
  k7zIdMTime,
  k7zIdWinAttrib,
  k7zIdComment,
  k7zIdEncodedHeader,
  k7zIdStartPos,
  k7zIdDummy
  /* k7zNtSecure, */
  /* k7zParent, */
  /* k7zIsReal */
};

static const Byte k7zSignature[k7zSignatureSize] = {'7', 'z', 0xBC, 0xAF, 0x27, 0x1C};

#define SzBitUi32s_Init(p) { (p)->Defs = NULL; (p)->Vals = NULL; }

static SRes SzBitUi32s_Alloc(CSzBitUi32s *p, size_t num, ISzAlloc *alloc)
{
  if (num == 0)
  {
    p->Defs = NULL;
    p->Vals = NULL;
  }
  else
  {
    MY_ALLOC(Byte, p->Defs, (num + 7) >> 3, alloc);
    MY_ALLOC(UInt32, p->Vals, num, alloc);
  }
  return SZ_OK;
}

static void SzBitUi32s_Free(CSzBitUi32s *p, ISzAlloc *alloc)
{
  IAlloc_Free(alloc, p->Defs); p->Defs = NULL;
  IAlloc_Free(alloc, p->Vals); p->Vals = NULL;
}

#define SzBitUi64s_Init(p) { (p)->Defs = NULL; (p)->Vals = NULL; }

static void SzBitUi64s_Free(CSzBitUi64s *p, ISzAlloc *alloc)
{
  IAlloc_Free(alloc, p->Defs); p->Defs = NULL;
  IAlloc_Free(alloc, p->Vals); p->Vals = NULL;
}


static void SzAr_Init(CSzAr *p)
{
  p->NumPackStreams = 0;
  p->NumFolders = 0;
  
  p->PackPositions = NULL;
  SzBitUi32s_Init(&p->FolderCRCs);

  p->FoCodersOffsets = NULL;
  p->FoStartPackStreamIndex = NULL;
  p->FoToCoderUnpackSizes = NULL;
  p->FoToMainUnpackSizeIndex = NULL;
  p->CoderUnpackSizes = NULL;

  p->CodersData = NULL;
}

static void SzAr_Free(CSzAr *p, ISzAlloc *alloc)
{
  IAlloc_Free(alloc, p->PackPositions);
  SzBitUi32s_Free(&p->FolderCRCs, alloc);
 
  IAlloc_Free(alloc, p->FoCodersOffsets);
  IAlloc_Free(alloc, p->FoStartPackStreamIndex);
  IAlloc_Free(alloc, p->FoToCoderUnpackSizes);
  IAlloc_Free(alloc, p->FoToMainUnpackSizeIndex);
  IAlloc_Free(alloc, p->CoderUnpackSizes);
  
  IAlloc_Free(alloc, p->CodersData);

  SzAr_Init(p);
}


static void SzArEx_Init(CSzArEx *p)
{
  SzAr_Init(&p->db);
  
  p->NumFiles = 0;
  p->dataPos = 0;
  
  p->UnpackPositions = NULL;
  p->IsDirs = NULL;
  
  p->FolderToFile = NULL;
  p->FileToFolder = NULL;
  
  p->FileNameOffsets = NULL;
  p->FileNames = NULL;
  
  SzBitUi32s_Init(&p->CRCs);
  SzBitUi32s_Init(&p->Attribs);
  /* SzBitUi32s_Init(&p->Parents); */
  SzBitUi64s_Init(&p->MTime);
  SzBitUi64s_Init(&p->CTime);
}

static void SzArEx_Free(CSzArEx *p, ISzAlloc *alloc)
{
  IAlloc_Free(alloc, p->UnpackPositions);
  IAlloc_Free(alloc, p->IsDirs);

  IAlloc_Free(alloc, p->FolderToFile);
  IAlloc_Free(alloc, p->FileToFolder);

  IAlloc_Free(alloc, p->FileNameOffsets);
  IAlloc_Free(alloc, p->FileNames);

  SzBitUi32s_Free(&p->CRCs, alloc);
  SzBitUi32s_Free(&p->Attribs, alloc);
  /* SzBitUi32s_Free(&p->Parents, alloc); */
  SzBitUi64s_Free(&p->MTime, alloc);
  SzBitUi64s_Free(&p->CTime, alloc);
  
  SzAr_Free(&p->db, alloc);
  SzArEx_Init(p);
}


static int TestSignatureCandidate(const Byte *testBytes)
{
  unsigned i;
  for (i = 0; i < k7zSignatureSize; i++)
    if (testBytes[i] != k7zSignature[i])
      return 0;
  return 1;
}

#define SzData_Clear(p) { (p)->Data = NULL; (p)->Size = 0; }

#define SZ_READ_BYTE_SD(_sd_, dest) if ((_sd_)->Size == 0) return SZ_ERROR_ARCHIVE; (_sd_)->Size--; dest = *(_sd_)->Data++;
#define SZ_READ_BYTE(dest) SZ_READ_BYTE_SD(sd, dest)
#define SZ_READ_BYTE_2(dest) if (sd.Size == 0) return SZ_ERROR_ARCHIVE; sd.Size--; dest = *sd.Data++;

#define SKIP_DATA(sd, size) { sd->Size -= (size_t)(size); sd->Data += (size_t)(size); }
#define SKIP_DATA2(sd, size) { sd.Size -= (size_t)(size); sd.Data += (size_t)(size); }

#define SZ_READ_32(dest) if (sd.Size < 4) return SZ_ERROR_ARCHIVE; \
   dest = GetUi32(sd.Data); SKIP_DATA2(sd, 4);

static MY_NO_INLINE SRes ReadNumber(CSzData *sd, UInt64 *value)
{
  Byte firstByte, mask;
  unsigned i;
  UInt32 v;

  SZ_READ_BYTE(firstByte);
  if ((firstByte & 0x80) == 0)
  {
    *value = firstByte;
    return SZ_OK;
  }
  SZ_READ_BYTE(v);
  if ((firstByte & 0x40) == 0)
  {
    *value = (((UInt32)firstByte & 0x3F) << 8) | v;
    return SZ_OK;
  }
  SZ_READ_BYTE(mask);
  *value = v | ((UInt32)mask << 8);
  mask = 0x20;
  for (i = 2; i < 8; i++)
  {
    Byte b;
    if ((firstByte & mask) == 0)
    {
      UInt64 highPart = (unsigned)firstByte & (unsigned)(mask - 1);
      *value |= (highPart << (8 * i));
      return SZ_OK;
    }
    SZ_READ_BYTE(b);
    *value |= ((UInt64)b << (8 * i));
    mask >>= 1;
  }
  return SZ_OK;
}


static MY_NO_INLINE SRes SzReadNumber32(CSzData *sd, UInt32 *value)
{
  Byte firstByte;
  UInt64 value64;
  if (sd->Size == 0)
    return SZ_ERROR_ARCHIVE;
  firstByte = *sd->Data;
  if ((firstByte & 0x80) == 0)
  {
    *value = firstByte;
    sd->Data++;
    sd->Size--;
    return SZ_OK;
  }
  RINOK(ReadNumber(sd, &value64));
  if (value64 >= (UInt32)0x80000000 - 1)
    return SZ_ERROR_UNSUPPORTED;
  if (value64 >= ((UInt64)(1) << ((sizeof(size_t) - 1) * 8 + 4)))
    return SZ_ERROR_UNSUPPORTED;
  *value = (UInt32)value64;
  return SZ_OK;
}

#define ReadID(sd, value) ReadNumber(sd, value)

static SRes SkipData(CSzData *sd)
{
  UInt64 size;
  RINOK(ReadNumber(sd, &size));
  if (size > sd->Size)
    return SZ_ERROR_ARCHIVE;
  SKIP_DATA(sd, size);
  return SZ_OK;
}

static SRes WaitId(CSzData *sd, UInt32 id)
{
  for (;;)
  {
    UInt64 type;
    RINOK(ReadID(sd, &type));
    if (type == id)
      return SZ_OK;
    if (type == k7zIdEnd)
      return SZ_ERROR_ARCHIVE;
    RINOK(SkipData(sd));
  }
}

static SRes RememberBitVector(CSzData *sd, UInt32 numItems, const Byte **v)
{
  UInt32 numBytes = (numItems + 7) >> 3;
  if (numBytes > sd->Size)
    return SZ_ERROR_ARCHIVE;
  *v = sd->Data;
  SKIP_DATA(sd, numBytes);
  return SZ_OK;
}

static UInt32 CountDefinedBits(const Byte *bits, UInt32 numItems)
{
  Byte b = 0;
  unsigned m = 0;
  UInt32 sum = 0;
  for (; numItems != 0; numItems--)
  {
    if (m == 0)
    {
      b = *bits++;
      m = 8;
    }
    m--;
    sum += ((b >> m) & 1);
  }
  return sum;
}

static MY_NO_INLINE SRes ReadBitVector(CSzData *sd, UInt32 numItems, Byte **v, ISzAlloc *alloc)
{
  Byte allAreDefined;
  Byte *v2;
  UInt32 numBytes = (numItems + 7) >> 3;
  *v = NULL;
  SZ_READ_BYTE(allAreDefined);
  if (numBytes == 0)
    return SZ_OK;
  if (allAreDefined == 0)
  {
    if (numBytes > sd->Size)
      return SZ_ERROR_ARCHIVE;
    MY_ALLOC_AND_CPY(*v, numBytes, sd->Data, alloc);
    SKIP_DATA(sd, numBytes);
    return SZ_OK;
  }
  MY_ALLOC(Byte, *v, numBytes, alloc);
  v2 = *v;
  memset(v2, 0xFF, (size_t)numBytes);
  {
    unsigned numBits = (unsigned)numItems & 7;
    if (numBits != 0)
      v2[numBytes - 1] = (Byte)((((UInt32)1 << numBits) - 1) << (8 - numBits));
  }
  return SZ_OK;
}

static MY_NO_INLINE SRes ReadUi32s(CSzData *sd2, UInt32 numItems, CSzBitUi32s *crcs, ISzAlloc *alloc)
{
  UInt32 i;
  CSzData sd;
  UInt32 *vals;
  const Byte *defs;
  MY_ALLOC_ZE(UInt32, crcs->Vals, numItems, alloc);
  sd = *sd2;
  defs = crcs->Defs;
  vals = crcs->Vals;
  for (i = 0; i < numItems; i++)
    if (SzBitArray_Check(defs, i))
    {
      SZ_READ_32(vals[i]);
    }
    else
      vals[i] = 0;
  *sd2 = sd;
  return SZ_OK;
}

static SRes ReadBitUi32s(CSzData *sd, UInt32 numItems, CSzBitUi32s *crcs, ISzAlloc *alloc)
{
  SzBitUi32s_Free(crcs, alloc);
  RINOK(ReadBitVector(sd, numItems, &crcs->Defs, alloc));
  return ReadUi32s(sd, numItems, crcs, alloc);
}

static SRes SkipBitUi32s(CSzData *sd, UInt32 numItems)
{
  Byte allAreDefined;
  UInt32 numDefined = numItems;
  SZ_READ_BYTE(allAreDefined);
  if (!allAreDefined)
  {
    size_t numBytes = (numItems + 7) >> 3;
    if (numBytes > sd->Size)
      return SZ_ERROR_ARCHIVE;
    numDefined = CountDefinedBits(sd->Data, numItems);
    SKIP_DATA(sd, numBytes);
  }
  if (numDefined > (sd->Size >> 2))
    return SZ_ERROR_ARCHIVE;
  SKIP_DATA(sd, (size_t)numDefined * 4);
  return SZ_OK;
}

static SRes ReadPackInfo(CSzAr *p, CSzData *sd, ISzAlloc *alloc)
{
  RINOK(SzReadNumber32(sd, &p->NumPackStreams));

  RINOK(WaitId(sd, k7zIdSize));
  MY_ALLOC(UInt64, p->PackPositions, (size_t)p->NumPackStreams + 1, alloc);
  {
    UInt64 sum = 0;
    UInt32 i;
    UInt32 numPackStreams = p->NumPackStreams;
    for (i = 0; i < numPackStreams; i++)
    {
      UInt64 packSize;
      p->PackPositions[i] = sum;
      RINOK(ReadNumber(sd, &packSize));
      sum += packSize;
      if (sum < packSize)
        return SZ_ERROR_ARCHIVE;
    }
    p->PackPositions[i] = sum;
  }

  for (;;)
  {
    UInt64 type;
    RINOK(ReadID(sd, &type));
    if (type == k7zIdEnd)
      return SZ_OK;
    if (type == k7zIdCRC)
    {
      /* CRC of packed streams is unused now */
      RINOK(SkipBitUi32s(sd, p->NumPackStreams));
      continue;
    }
    RINOK(SkipData(sd));
  }
}

/*
static SRes SzReadSwitch(CSzData *sd)
{
  Byte external;
  RINOK(SzReadByte(sd, &external));
  return (external == 0) ? SZ_OK: SZ_ERROR_UNSUPPORTED;
}
*/

#define k_NumCodersStreams_in_Folder_MAX (SZ_NUM_BONDS_IN_FOLDER_MAX + SZ_NUM_PACK_STREAMS_IN_FOLDER_MAX)

static SRes SzGetNextFolderItem(CSzFolder *f, CSzData *sd)
{
  UInt32 numCoders, i;
  UInt32 numInStreams = 0;
  const Byte *dataStart = sd->Data;

  f->NumCoders = 0;
  f->NumBonds = 0;
  f->NumPackStreams = 0;
  f->UnpackStream = 0;
  
  RINOK(SzReadNumber32(sd, &numCoders));
  if (numCoders == 0 || numCoders > SZ_NUM_CODERS_IN_FOLDER_MAX)
    return SZ_ERROR_UNSUPPORTED;
  
  for (i = 0; i < numCoders; i++)
  {
    Byte mainByte;
    CSzCoderInfo *coder = f->Coders + i;
    unsigned idSize, j;
    UInt64 id;
    
    SZ_READ_BYTE(mainByte);
    if ((mainByte & 0xC0) != 0)
      return SZ_ERROR_UNSUPPORTED;
    
    idSize = (unsigned)(mainByte & 0xF);
    if (idSize > sizeof(id))
      return SZ_ERROR_UNSUPPORTED;
    if (idSize > sd->Size)
      return SZ_ERROR_ARCHIVE;
    id = 0;
    for (j = 0; j < idSize; j++)
    {
      id = ((id << 8) | *sd->Data);
      sd->Data++;
      sd->Size--;
    }
    if (id > UINT64_CONST(0xFFFFFFFF))
      return SZ_ERROR_UNSUPPORTED;
    coder->MethodID = (UInt32)id;
    
    coder->NumStreams = 1;
    coder->PropsOffset = 0;
    coder->PropsSize = 0;
    
    if ((mainByte & 0x10) != 0)
    {
      UInt32 numStreams;
      
      RINOK(SzReadNumber32(sd, &numStreams));
      if (numStreams > k_NumCodersStreams_in_Folder_MAX)
        return SZ_ERROR_UNSUPPORTED;
      coder->NumStreams = (Byte)numStreams;

      RINOK(SzReadNumber32(sd, &numStreams));
      if (numStreams != 1)
        return SZ_ERROR_UNSUPPORTED;
    }

    numInStreams += coder->NumStreams;

    if (numInStreams > k_NumCodersStreams_in_Folder_MAX)
      return SZ_ERROR_UNSUPPORTED;

    if ((mainByte & 0x20) != 0)
    {
      UInt32 propsSize = 0;
      RINOK(SzReadNumber32(sd, &propsSize));
      if (propsSize > sd->Size)
        return SZ_ERROR_ARCHIVE;
      if (propsSize >= 0x80)
        return SZ_ERROR_UNSUPPORTED;
      coder->PropsOffset = sd->Data - dataStart;
      coder->PropsSize = (Byte)propsSize;
      sd->Data += (size_t)propsSize;
      sd->Size -= (size_t)propsSize;
    }
  }

  /*
  if (numInStreams == 1 && numCoders == 1)
  {
    f->NumPackStreams = 1;
    f->PackStreams[0] = 0;
  }
  else
  */
  {
    Byte streamUsed[k_NumCodersStreams_in_Folder_MAX];
    UInt32 numBonds, numPackStreams;
    
    numBonds = numCoders - 1;
    if (numInStreams < numBonds)
      return SZ_ERROR_ARCHIVE;
    if (numBonds > SZ_NUM_BONDS_IN_FOLDER_MAX)
      return SZ_ERROR_UNSUPPORTED;
    f->NumBonds = numBonds;
    
    numPackStreams = numInStreams - numBonds;
    if (numPackStreams > SZ_NUM_PACK_STREAMS_IN_FOLDER_MAX)
      return SZ_ERROR_UNSUPPORTED;
    f->NumPackStreams = numPackStreams;
  
    for (i = 0; i < numInStreams; i++)
      streamUsed[i] = False;
    
    if (numBonds != 0)
    {
      Byte coderUsed[SZ_NUM_CODERS_IN_FOLDER_MAX];

      for (i = 0; i < numCoders; i++)
        coderUsed[i] = False;
      
      for (i = 0; i < numBonds; i++)
      {
        CSzBond *bp = f->Bonds + i;
        
        RINOK(SzReadNumber32(sd, &bp->InIndex));
        if (bp->InIndex >= numInStreams || streamUsed[bp->InIndex])
          return SZ_ERROR_ARCHIVE;
        streamUsed[bp->InIndex] = True;
        
        RINOK(SzReadNumber32(sd, &bp->OutIndex));
        if (bp->OutIndex >= numCoders || coderUsed[bp->OutIndex])
          return SZ_ERROR_ARCHIVE;
        coderUsed[bp->OutIndex] = True;
      }
      
      for (i = 0; i < numCoders; i++)
        if (!coderUsed[i])
        {
          f->UnpackStream = i;
          break;
        }
      
      if (i == numCoders)
        return SZ_ERROR_ARCHIVE;
    }
    
    if (numPackStreams == 1)
    {
      for (i = 0; i < numInStreams; i++)
        if (!streamUsed[i])
          break;
      if (i == numInStreams)
        return SZ_ERROR_ARCHIVE;
      f->PackStreams[0] = i;
    }
    else
      for (i = 0; i < numPackStreams; i++)
      {
        UInt32 index;
        RINOK(SzReadNumber32(sd, &index));
        if (index >= numInStreams || streamUsed[index])
          return SZ_ERROR_ARCHIVE;
        streamUsed[index] = True;
        f->PackStreams[i] = index;
      }
  }

  f->NumCoders = numCoders;

  return SZ_OK;
}


static MY_NO_INLINE SRes SkipNumbers(CSzData *sd2, UInt32 num)
{
  CSzData sd;
  sd = *sd2;
  for (; num != 0; num--)
  {
    Byte firstByte, mask;
    unsigned i;
    SZ_READ_BYTE_2(firstByte);
    if ((firstByte & 0x80) == 0)
      continue;
    if ((firstByte & 0x40) == 0)
    {
      if (sd.Size == 0)
        return SZ_ERROR_ARCHIVE;
      sd.Size--;
      sd.Data++;
      continue;
    }
    mask = 0x20;
    for (i = 2; i < 8 && (firstByte & mask) != 0; i++)
      mask >>= 1;
    if (i > sd.Size)
      return SZ_ERROR_ARCHIVE;
    SKIP_DATA2(sd, i);
  }
  *sd2 = sd;
  return SZ_OK;
}


#define k_Scan_NumCoders_MAX 64
#define k_Scan_NumCodersStreams_in_Folder_MAX 64


static SRes ReadUnpackInfo(CSzAr *p,
    CSzData *sd2,
    UInt32 numFoldersMax,
    const CBuf *tempBufs, UInt32 numTempBufs,
    ISzAlloc *alloc)
{
  CSzData sd;
  
  UInt32 fo, numFolders, numCodersOutStreams, packStreamIndex;
  const Byte *startBufPtr;
  Byte external;
  
  RINOK(WaitId(sd2, k7zIdFolder));
  
  RINOK(SzReadNumber32(sd2, &numFolders));
  if (numFolders > numFoldersMax)
    return SZ_ERROR_UNSUPPORTED;
  p->NumFolders = numFolders;

  SZ_READ_BYTE_SD(sd2, external);
  if (external == 0)
    sd = *sd2;
  else
  {
    UInt32 index;
    RINOK(SzReadNumber32(sd2, &index));
    if (index >= numTempBufs)
      return SZ_ERROR_ARCHIVE;
    sd.Data = tempBufs[index].data;
    sd.Size = tempBufs[index].size;
  }
  
  MY_ALLOC(size_t, p->FoCodersOffsets, (size_t)numFolders + 1, alloc);
  MY_ALLOC(UInt32, p->FoStartPackStreamIndex, (size_t)numFolders + 1, alloc);
  MY_ALLOC(UInt32, p->FoToCoderUnpackSizes, (size_t)numFolders + 1, alloc);
  MY_ALLOC(Byte, p->FoToMainUnpackSizeIndex, (size_t)numFolders, alloc);
  
  startBufPtr = sd.Data;
  
  packStreamIndex = 0;
  numCodersOutStreams = 0;

  for (fo = 0; fo < numFolders; fo++)
  {
    UInt32 numCoders, ci, numInStreams = 0;
    
    p->FoCodersOffsets[fo] = sd.Data - startBufPtr;
    
    RINOK(SzReadNumber32(&sd, &numCoders));
    if (numCoders == 0 || numCoders > k_Scan_NumCoders_MAX)
      return SZ_ERROR_UNSUPPORTED;
    
    for (ci = 0; ci < numCoders; ci++)
    {
      Byte mainByte;
      unsigned idSize;
      UInt32 coderInStreams;
      
      SZ_READ_BYTE_2(mainByte);
      if ((mainByte & 0xC0) != 0)
        return SZ_ERROR_UNSUPPORTED;
      idSize = (mainByte & 0xF);
      if (idSize > 8)
        return SZ_ERROR_UNSUPPORTED;
      if (idSize > sd.Size)
        return SZ_ERROR_ARCHIVE;
      SKIP_DATA2(sd, idSize);
      
      coderInStreams = 1;
      
      if ((mainByte & 0x10) != 0)
      {
        UInt32 coderOutStreams;
        RINOK(SzReadNumber32(&sd, &coderInStreams));
        RINOK(SzReadNumber32(&sd, &coderOutStreams));
        if (coderInStreams > k_Scan_NumCodersStreams_in_Folder_MAX || coderOutStreams != 1)
          return SZ_ERROR_UNSUPPORTED;
      }
      
      numInStreams += coderInStreams;

      if ((mainByte & 0x20) != 0)
      {
        UInt32 propsSize;
        RINOK(SzReadNumber32(&sd, &propsSize));
        if (propsSize > sd.Size)
          return SZ_ERROR_ARCHIVE;
        SKIP_DATA2(sd, propsSize);
      }
    }
    
    {
      UInt32 indexOfMainStream = 0;
      UInt32 numPackStreams = 1;
      
      if (numCoders != 1 || numInStreams != 1)
      {
        Byte streamUsed[k_Scan_NumCodersStreams_in_Folder_MAX];
        Byte coderUsed[k_Scan_NumCoders_MAX];
    
        UInt32 i;
        UInt32 numBonds = numCoders - 1;
        if (numInStreams < numBonds)
          return SZ_ERROR_ARCHIVE;
        
        if (numInStreams > k_Scan_NumCodersStreams_in_Folder_MAX)
          return SZ_ERROR_UNSUPPORTED;
        
        for (i = 0; i < numInStreams; i++)
          streamUsed[i] = False;
        for (i = 0; i < numCoders; i++)
          coderUsed[i] = False;
        
        for (i = 0; i < numBonds; i++)
        {
          UInt32 index;
          
          RINOK(SzReadNumber32(&sd, &index));
          if (index >= numInStreams || streamUsed[index])
            return SZ_ERROR_ARCHIVE;
          streamUsed[index] = True;
          
          RINOK(SzReadNumber32(&sd, &index));
          if (index >= numCoders || coderUsed[index])
            return SZ_ERROR_ARCHIVE;
          coderUsed[index] = True;
        }
        
        numPackStreams = numInStreams - numBonds;
        
        if (numPackStreams != 1)
          for (i = 0; i < numPackStreams; i++)
          {
            UInt32 index;
            RINOK(SzReadNumber32(&sd, &index));
            if (index >= numInStreams || streamUsed[index])
              return SZ_ERROR_ARCHIVE;
            streamUsed[index] = True;
          }
          
        for (i = 0; i < numCoders; i++)
          if (!coderUsed[i])
          {
            indexOfMainStream = i;
            break;
          }
 
        if (i == numCoders)
          return SZ_ERROR_ARCHIVE;
      }
      
      p->FoStartPackStreamIndex[fo] = packStreamIndex;
      p->FoToCoderUnpackSizes[fo] = numCodersOutStreams;
      p->FoToMainUnpackSizeIndex[fo] = (Byte)indexOfMainStream;
      numCodersOutStreams += numCoders;
      if (numCodersOutStreams < numCoders)
        return SZ_ERROR_UNSUPPORTED;
      if (numPackStreams > p->NumPackStreams - packStreamIndex)
        return SZ_ERROR_ARCHIVE;
      packStreamIndex += numPackStreams;
    }
  }

  p->FoToCoderUnpackSizes[fo] = numCodersOutStreams;
  
  {
    size_t dataSize = sd.Data - startBufPtr;
    p->FoStartPackStreamIndex[fo] = packStreamIndex;
    p->FoCodersOffsets[fo] = dataSize;
    MY_ALLOC_ZE_AND_CPY(p->CodersData, dataSize, startBufPtr, alloc);
  }
  
  if (external != 0)
  {
    if (sd.Size != 0)
      return SZ_ERROR_ARCHIVE;
    sd = *sd2;
  }
  
  RINOK(WaitId(&sd, k7zIdCodersUnpackSize));
  
  MY_ALLOC_ZE(UInt64, p->CoderUnpackSizes, (size_t)numCodersOutStreams, alloc);
  {
    UInt32 i;
    for (i = 0; i < numCodersOutStreams; i++)
    {
      RINOK(ReadNumber(&sd, p->CoderUnpackSizes + i));
    }
  }

  for (;;)
  {
    UInt64 type;
    RINOK(ReadID(&sd, &type));
    if (type == k7zIdEnd)
    {
      *sd2 = sd;
      return SZ_OK;
    }
    if (type == k7zIdCRC)
    {
      RINOK(ReadBitUi32s(&sd, numFolders, &p->FolderCRCs, alloc));
      continue;
    }
    RINOK(SkipData(&sd));
  }
}


static UInt64 SzAr_GetFolderUnpackSize(const CSzAr *p, UInt32 folderIndex)
{
  return p->CoderUnpackSizes[p->FoToCoderUnpackSizes[folderIndex] + p->FoToMainUnpackSizeIndex[folderIndex]];
}


typedef struct
{
  UInt32 NumTotalSubStreams;
  UInt32 NumSubDigests;
  CSzData sdNumSubStreams;
  CSzData sdSizes;
  CSzData sdCRCs;
} CSubStreamInfo;


static SRes ReadSubStreamsInfo(CSzAr *p, CSzData *sd, CSubStreamInfo *ssi)
{
  UInt64 type = 0;
  UInt32 numSubDigests = 0;
  UInt32 numFolders = p->NumFolders;
  UInt32 numUnpackStreams = numFolders;
  UInt32 numUnpackSizesInData = 0;

  for (;;)
  {
    RINOK(ReadID(sd, &type));
    if (type == k7zIdNumUnpackStream)
    {
      UInt32 i;
      ssi->sdNumSubStreams.Data = sd->Data;
      numUnpackStreams = 0;
      numSubDigests = 0;
      for (i = 0; i < numFolders; i++)
      {
        UInt32 numStreams;
        RINOK(SzReadNumber32(sd, &numStreams));
        if (numUnpackStreams > numUnpackStreams + numStreams)
          return SZ_ERROR_UNSUPPORTED;
        numUnpackStreams += numStreams;
        if (numStreams != 0)
          numUnpackSizesInData += (numStreams - 1);
        if (numStreams != 1 || !SzBitWithVals_Check(&p->FolderCRCs, i))
          numSubDigests += numStreams;
      }
      ssi->sdNumSubStreams.Size = sd->Data - ssi->sdNumSubStreams.Data;
      continue;
    }
    if (type == k7zIdCRC || type == k7zIdSize || type == k7zIdEnd)
      break;
    RINOK(SkipData(sd));
  }

  if (!ssi->sdNumSubStreams.Data)
  {
    numSubDigests = numFolders;
    if (p->FolderCRCs.Defs)
      numSubDigests = numFolders - CountDefinedBits(p->FolderCRCs.Defs, numFolders);
  }
  
  ssi->NumTotalSubStreams = numUnpackStreams;
  ssi->NumSubDigests = numSubDigests;

  if (type == k7zIdSize)
  {
    ssi->sdSizes.Data = sd->Data;
    RINOK(SkipNumbers(sd, numUnpackSizesInData));
    ssi->sdSizes.Size = sd->Data - ssi->sdSizes.Data;
    RINOK(ReadID(sd, &type));
  }

  for (;;)
  {
    if (type == k7zIdEnd)
      return SZ_OK;
    if (type == k7zIdCRC)
    {
      ssi->sdCRCs.Data = sd->Data;
      RINOK(SkipBitUi32s(sd, numSubDigests));
      ssi->sdCRCs.Size = sd->Data - ssi->sdCRCs.Data;
    }
    else
    {
      RINOK(SkipData(sd));
    }
    RINOK(ReadID(sd, &type));
  }
}

static SRes SzReadStreamsInfo(CSzAr *p,
    CSzData *sd,
    UInt32 numFoldersMax, const CBuf *tempBufs, UInt32 numTempBufs,
    UInt64 *dataOffset,
    CSubStreamInfo *ssi,
    ISzAlloc *alloc)
{
  UInt64 type;

  SzData_Clear(&ssi->sdSizes);
  SzData_Clear(&ssi->sdCRCs);
  SzData_Clear(&ssi->sdNumSubStreams);

  *dataOffset = 0;
  RINOK(ReadID(sd, &type));
  if (type == k7zIdPackInfo)
  {
    RINOK(ReadNumber(sd, dataOffset));
    RINOK(ReadPackInfo(p, sd, alloc));
    RINOK(ReadID(sd, &type));
  }
  if (type == k7zIdUnpackInfo)
  {
    RINOK(ReadUnpackInfo(p, sd, numFoldersMax, tempBufs, numTempBufs, alloc));
    RINOK(ReadID(sd, &type));
  }
  if (type == k7zIdSubStreamsInfo)
  {
    RINOK(ReadSubStreamsInfo(p, sd, ssi));
    RINOK(ReadID(sd, &type));
  }
  else
  {
    ssi->NumTotalSubStreams = p->NumFolders;
    /* ssi->NumSubDigests = 0; */
  }

  return (type == k7zIdEnd ? SZ_OK : SZ_ERROR_UNSUPPORTED);
}

static SRes SzReadAndDecodePackedStreams(
    ILookInStream *inStream,
    CSzData *sd,
    CBuf *tempBufs,
    UInt32 numFoldersMax,
    UInt64 baseOffset,
    CSzAr *p,
    ISzAlloc *allocTemp)
{
  UInt64 dataStartPos = 0;
  UInt32 fo;
  CSubStreamInfo ssi;
  UInt32 numFolders;

  RINOK(SzReadStreamsInfo(p, sd, numFoldersMax, NULL, 0, &dataStartPos, &ssi, allocTemp));

  numFolders = p->NumFolders;
  if (numFolders == 0)
    return SZ_ERROR_ARCHIVE;
  else if (numFolders > numFoldersMax)
    return SZ_ERROR_UNSUPPORTED;

  dataStartPos += baseOffset;

  for (fo = 0; fo < numFolders; fo++)
    Buf_Init(tempBufs + fo);
  
  for (fo = 0; fo < numFolders; fo++)
  {
    CBuf *tempBuf = tempBufs + fo;
    UInt64 unpackSize = SzAr_GetFolderUnpackSize(p, fo);
    if ((size_t)unpackSize != unpackSize)
      return SZ_ERROR_MEM;
    if (!Buf_Create(tempBuf, (size_t)unpackSize, allocTemp))
      return SZ_ERROR_MEM;
  }
  
  for (fo = 0; fo < numFolders; fo++)
  {
    const CBuf *tempBuf = tempBufs + fo;
    RINOK(LookInStream_SeekTo(inStream, dataStartPos));
    RINOK(SzAr_DecodeFolder(p, fo, inStream, dataStartPos, tempBuf->data, tempBuf->size, allocTemp));
  }
  
  return SZ_OK;
}

static SRes SzReadFileNames(const Byte *data, size_t size, UInt32 numFiles, size_t *offsets)
{
  size_t pos = 0;
  *offsets++ = 0;
  if (numFiles == 0)
    return (size == 0) ? SZ_OK : SZ_ERROR_ARCHIVE;
  if (size < 2)
    return SZ_ERROR_ARCHIVE;
  if (data[size - 2] != 0 || data[size - 1] != 0)
    return SZ_ERROR_ARCHIVE;
  do
  {
    const Byte *p;
    if (pos == size)
      return SZ_ERROR_ARCHIVE;
    for (p = data + pos;
      #ifdef _WIN32
      *(const UInt16 *)p != 0
      #else
      p[0] != 0 || p[1] != 0
      #endif
      ; p += 2);
    pos = p - data + 2;
    *offsets++ = (pos >> 1);
  }
  while (--numFiles);
  return (pos == size) ? SZ_OK : SZ_ERROR_ARCHIVE;
}

static MY_NO_INLINE SRes ReadTime(CSzBitUi64s *p, UInt32 num,
    CSzData *sd2,
    const CBuf *tempBufs, UInt32 numTempBufs,
    ISzAlloc *alloc)
{
  CSzData sd;
  UInt32 i;
  CNtfsFileTime *vals;
  Byte *defs;
  Byte external;
  
  RINOK(ReadBitVector(sd2, num, &p->Defs, alloc));
  
  SZ_READ_BYTE_SD(sd2, external);
  if (external == 0)
    sd = *sd2;
  else
  {
    UInt32 index;
    RINOK(SzReadNumber32(sd2, &index));
    if (index >= numTempBufs)
      return SZ_ERROR_ARCHIVE;
    sd.Data = tempBufs[index].data;
    sd.Size = tempBufs[index].size;
  }
  
  MY_ALLOC_ZE(CNtfsFileTime, p->Vals, num, alloc);
  vals = p->Vals;
  defs = p->Defs;
  for (i = 0; i < num; i++)
    if (SzBitArray_Check(defs, i))
    {
      if (sd.Size < 8)
        return SZ_ERROR_ARCHIVE;
      vals[i].Low = GetUi32(sd.Data);
      vals[i].High = GetUi32(sd.Data + 4);
      SKIP_DATA2(sd, 8);
    }
    else
      vals[i].High = vals[i].Low = 0;
  
  if (external == 0)
    *sd2 = sd;
  
  return SZ_OK;
}


#define NUM_ADDITIONAL_STREAMS_MAX 8


static SRes SzReadHeader2(
    CSzArEx *p,   /* allocMain */
    CSzData *sd,
    ILookInStream *inStream,
    CBuf *tempBufs, UInt32 *numTempBufs,
    ISzAlloc *allocMain,
    ISzAlloc *allocTemp
    )
{
  CSubStreamInfo ssi;

{
  UInt64 type;
  
  SzData_Clear(&ssi.sdSizes);
  SzData_Clear(&ssi.sdCRCs);
  SzData_Clear(&ssi.sdNumSubStreams);

  ssi.NumSubDigests = 0;
  ssi.NumTotalSubStreams = 0;

  RINOK(ReadID(sd, &type));

  if (type == k7zIdArchiveProperties)
  {
    for (;;)
    {
      UInt64 type2;
      RINOK(ReadID(sd, &type2));
      if (type2 == k7zIdEnd)
        break;
      RINOK(SkipData(sd));
    }
    RINOK(ReadID(sd, &type));
  }

  if (type == k7zIdAdditionalStreamsInfo)
  {
    CSzAr tempAr;
    SRes res;
    
    SzAr_Init(&tempAr);
    res = SzReadAndDecodePackedStreams(inStream, sd, tempBufs, NUM_ADDITIONAL_STREAMS_MAX,
        p->startPosAfterHeader, &tempAr, allocTemp);
    *numTempBufs = tempAr.NumFolders;
    SzAr_Free(&tempAr, allocTemp);
    
    if (res != SZ_OK)
      return res;
    RINOK(ReadID(sd, &type));
  }

  if (type == k7zIdMainStreamsInfo)
  {
    RINOK(SzReadStreamsInfo(&p->db, sd, (UInt32)1 << 30, tempBufs, *numTempBufs,
        &p->dataPos, &ssi, allocMain));
    p->dataPos += p->startPosAfterHeader;
    RINOK(ReadID(sd, &type));
  }

  if (type == k7zIdEnd)
  {
    return SZ_OK;
  }

  if (type != k7zIdFilesInfo)
    return SZ_ERROR_ARCHIVE;
}

{
  UInt32 numFiles = 0;
  UInt32 numEmptyStreams = 0;
  const Byte *emptyStreams = NULL;
  const Byte *emptyFiles = NULL;
  
  RINOK(SzReadNumber32(sd, &numFiles));
  p->NumFiles = numFiles;

  for (;;)
  {
    UInt64 type;
    UInt64 size;
    RINOK(ReadID(sd, &type));
    if (type == k7zIdEnd)
      break;
    RINOK(ReadNumber(sd, &size));
    if (size > sd->Size)
      return SZ_ERROR_ARCHIVE;
    
    if (type >= ((UInt32)1 << 8))
    {
      SKIP_DATA(sd, size);
    }
    else switch ((unsigned)type)
    {
      case k7zIdName:
      {
        size_t namesSize;
        const Byte *namesData;
        Byte external;

        SZ_READ_BYTE(external);
        if (external == 0)
        {
          namesSize = (size_t)size - 1;
          namesData = sd->Data;
        }
        else
        {
          UInt32 index;
          RINOK(SzReadNumber32(sd, &index));
          if (index >= *numTempBufs)
            return SZ_ERROR_ARCHIVE;
          namesData = (tempBufs)[index].data;
          namesSize = (tempBufs)[index].size;
        }

        if ((namesSize & 1) != 0)
          return SZ_ERROR_ARCHIVE;
        MY_ALLOC(size_t, p->FileNameOffsets, numFiles + 1, allocMain);
        MY_ALLOC_ZE_AND_CPY(p->FileNames, namesSize, namesData, allocMain);
        RINOK(SzReadFileNames(p->FileNames, namesSize, numFiles, p->FileNameOffsets))
        if (external == 0)
        {
          SKIP_DATA(sd, namesSize);
        }
        break;
      }
      case k7zIdEmptyStream:
      {
        RINOK(RememberBitVector(sd, numFiles, &emptyStreams));
        numEmptyStreams = CountDefinedBits(emptyStreams, numFiles);
        emptyFiles = NULL;
        break;
      }
      case k7zIdEmptyFile:
      {
        RINOK(RememberBitVector(sd, numEmptyStreams, &emptyFiles));
        break;
      }
      case k7zIdWinAttrib:
      {
        Byte external;
        CSzData sdSwitch;
        CSzData *sdPtr;
        SzBitUi32s_Free(&p->Attribs, allocMain);
        RINOK(ReadBitVector(sd, numFiles, &p->Attribs.Defs, allocMain));

        SZ_READ_BYTE(external);
        if (external == 0)
          sdPtr = sd;
        else
        {
          UInt32 index;
          RINOK(SzReadNumber32(sd, &index));
          if (index >= *numTempBufs)
            return SZ_ERROR_ARCHIVE;
          sdSwitch.Data = (tempBufs)[index].data;
          sdSwitch.Size = (tempBufs)[index].size;
          sdPtr = &sdSwitch;
        }
        RINOK(ReadUi32s(sdPtr, numFiles, &p->Attribs, allocMain));
        break;
      }
      /*
      case k7zParent:
      {
        SzBitUi32s_Free(&p->Parents, allocMain);
        RINOK(ReadBitVector(sd, numFiles, &p->Parents.Defs, allocMain));
        RINOK(SzReadSwitch(sd));
        RINOK(ReadUi32s(sd, numFiles, &p->Parents, allocMain));
        break;
      }
      */
      case k7zIdMTime: RINOK(ReadTime(&p->MTime, numFiles, sd, tempBufs, *numTempBufs, allocMain)); break;
      case k7zIdCTime: RINOK(ReadTime(&p->CTime, numFiles, sd, tempBufs, *numTempBufs, allocMain)); break;
      default:
      {
        SKIP_DATA(sd, size);
      }
    }
  }

  if (numFiles - numEmptyStreams != ssi.NumTotalSubStreams)
    return SZ_ERROR_ARCHIVE;

  for (;;)
  {
    UInt64 type;
    RINOK(ReadID(sd, &type));
    if (type == k7zIdEnd)
      break;
    RINOK(SkipData(sd));
  }

  {
    UInt32 i;
    UInt32 emptyFileIndex = 0;
    UInt32 folderIndex = 0;
    UInt32 remSubStreams = 0;
    UInt32 numSubStreams = 0;
    UInt64 unpackPos = 0;
    const Byte *digestsDefs = NULL;
    const Byte *digestsVals = NULL;
    UInt32 digestsValsIndex = 0;
    UInt32 digestIndex;
    Byte allDigestsDefined = 0;
    Byte isDirMask = 0;
    Byte crcMask = 0;
    Byte mask = 0x80;
    
    MY_ALLOC(UInt32, p->FolderToFile, p->db.NumFolders + 1, allocMain);
    MY_ALLOC_ZE(UInt32, p->FileToFolder, p->NumFiles, allocMain);
    MY_ALLOC(UInt64, p->UnpackPositions, p->NumFiles + 1, allocMain);
    MY_ALLOC_ZE(Byte, p->IsDirs, (p->NumFiles + 7) >> 3, allocMain);

    RINOK(SzBitUi32s_Alloc(&p->CRCs, p->NumFiles, allocMain));

    if (ssi.sdCRCs.Size != 0)
    {
      SZ_READ_BYTE_SD(&ssi.sdCRCs, allDigestsDefined);
      if (allDigestsDefined)
        digestsVals = ssi.sdCRCs.Data;
      else
      {
        size_t numBytes = (ssi.NumSubDigests + 7) >> 3;
        digestsDefs = ssi.sdCRCs.Data;
        digestsVals = digestsDefs + numBytes;
      }
    }

    digestIndex = 0;
    
    for (i = 0; i < numFiles; i++, mask >>= 1)
    {
      if (mask == 0)
      {
        UInt32 byteIndex = (i - 1) >> 3;
        p->IsDirs[byteIndex] = isDirMask;
        p->CRCs.Defs[byteIndex] = crcMask;
        isDirMask = 0;
        crcMask = 0;
        mask = 0x80;
      }

      p->UnpackPositions[i] = unpackPos;
      p->CRCs.Vals[i] = 0;
      
      if (emptyStreams && SzBitArray_Check(emptyStreams, i))
      {
        if (emptyFiles)
        {
          if (!SzBitArray_Check(emptyFiles, emptyFileIndex))
            isDirMask |= mask;
          emptyFileIndex++;
        }
        else
          isDirMask |= mask;
        if (remSubStreams == 0)
        {
          p->FileToFolder[i] = (UInt32)-1;
          continue;
        }
      }
      
      if (remSubStreams == 0)
      {
        for (;;)
        {
          if (folderIndex >= p->db.NumFolders)
            return SZ_ERROR_ARCHIVE;
          p->FolderToFile[folderIndex] = i;
          numSubStreams = 1;
          if (ssi.sdNumSubStreams.Data)
          {
            RINOK(SzReadNumber32(&ssi.sdNumSubStreams, &numSubStreams));
          }
          remSubStreams = numSubStreams;
          if (numSubStreams != 0)
            break;
          {
            UInt64 folderUnpackSize = SzAr_GetFolderUnpackSize(&p->db, folderIndex);
            unpackPos += folderUnpackSize;
            if (unpackPos < folderUnpackSize)
              return SZ_ERROR_ARCHIVE;
          }

          folderIndex++;
        }
      }
      
      p->FileToFolder[i] = folderIndex;
      
      if (emptyStreams && SzBitArray_Check(emptyStreams, i))
        continue;
      
      if (--remSubStreams == 0)
      {
        UInt64 folderUnpackSize = SzAr_GetFolderUnpackSize(&p->db, folderIndex);
        UInt64 startFolderUnpackPos = p->UnpackPositions[p->FolderToFile[folderIndex]];
        if (folderUnpackSize < unpackPos - startFolderUnpackPos)
          return SZ_ERROR_ARCHIVE;
        unpackPos = startFolderUnpackPos + folderUnpackSize;
        if (unpackPos < folderUnpackSize)
          return SZ_ERROR_ARCHIVE;

        if (numSubStreams == 1 && SzBitWithVals_Check(&p->db.FolderCRCs, i))
        {
          p->CRCs.Vals[i] = p->db.FolderCRCs.Vals[folderIndex];
          crcMask |= mask;
        }
        else if (allDigestsDefined || (digestsDefs && SzBitArray_Check(digestsDefs, digestIndex)))
        {
          p->CRCs.Vals[i] = GetUi32(digestsVals + (size_t)digestsValsIndex * 4);
          digestsValsIndex++;
          crcMask |= mask;
        }
        
        folderIndex++;
      }
      else
      {
        UInt64 v;
        RINOK(ReadNumber(&ssi.sdSizes, &v));
        unpackPos += v;
        if (unpackPos < v)
          return SZ_ERROR_ARCHIVE;
        if (allDigestsDefined || (digestsDefs && SzBitArray_Check(digestsDefs, digestIndex)))
        {
          p->CRCs.Vals[i] = GetUi32(digestsVals + (size_t)digestsValsIndex * 4);
          digestsValsIndex++;
          crcMask |= mask;
        }
      }
    }

    if (mask != 0x80)
    {
      UInt32 byteIndex = (i - 1) >> 3;
      p->IsDirs[byteIndex] = isDirMask;
      p->CRCs.Defs[byteIndex] = crcMask;
    }
    
    p->UnpackPositions[i] = unpackPos;

    if (remSubStreams != 0)
      return SZ_ERROR_ARCHIVE;

    for (;;)
    {
      p->FolderToFile[folderIndex] = i;
      if (folderIndex >= p->db.NumFolders)
        break;
      if (!ssi.sdNumSubStreams.Data)
        return SZ_ERROR_ARCHIVE;
      RINOK(SzReadNumber32(&ssi.sdNumSubStreams, &numSubStreams));
      if (numSubStreams != 0)
        return SZ_ERROR_ARCHIVE;
      /*
      {
        UInt64 folderUnpackSize = SzAr_GetFolderUnpackSize(&p->db, folderIndex);
        unpackPos += folderUnpackSize;
        if (unpackPos < folderUnpackSize)
          return SZ_ERROR_ARCHIVE;
      }
      */
      folderIndex++;
    }

    if (ssi.sdNumSubStreams.Data && ssi.sdNumSubStreams.Size != 0)
      return SZ_ERROR_ARCHIVE;
  }
}
  return SZ_OK;
}


static SRes SzReadHeader(
    CSzArEx *p,
    CSzData *sd,
    ILookInStream *inStream,
    ISzAlloc *allocMain,
    ISzAlloc *allocTemp)
{
  UInt32 i;
  UInt32 numTempBufs = 0;
  SRes res;
  CBuf tempBufs[NUM_ADDITIONAL_STREAMS_MAX];

  for (i = 0; i < NUM_ADDITIONAL_STREAMS_MAX; i++)
    Buf_Init(tempBufs + i);
  
  res = SzReadHeader2(p, sd, inStream,
      tempBufs, &numTempBufs,
      allocMain, allocTemp);
  
  for (i = 0; i < NUM_ADDITIONAL_STREAMS_MAX; i++)
    Buf_Free(tempBufs + i, allocTemp);

  RINOK(res);

  if (sd->Size != 0)
    return SZ_ERROR_FAIL;

  return res;
}

static SRes SzArEx_Open2(
    CSzArEx *p,
    ILookInStream *inStream,
    ISzAlloc *allocMain,
    ISzAlloc *allocTemp)
{
  Byte header[k7zStartHeaderSize];
  Int64 startArcPos;
  UInt64 nextHeaderOffset, nextHeaderSize;
  size_t nextHeaderSizeT;
  UInt32 nextHeaderCRC;
  CBuf buf;
  SRes res;

  startArcPos = 0;
  RINOK(inStream->Seek(inStream, &startArcPos, SZ_SEEK_CUR));

  RINOK(LookInStream_Read2(inStream, header, k7zStartHeaderSize, SZ_ERROR_NO_ARCHIVE));

  if (!TestSignatureCandidate(header))
    return SZ_ERROR_NO_ARCHIVE;
  if (header[6] != k7zMajorVersion)
    return SZ_ERROR_UNSUPPORTED;

  nextHeaderOffset = GetUi64(header + 12);
  nextHeaderSize = GetUi64(header + 20);
  nextHeaderCRC = GetUi32(header + 28);

  p->startPosAfterHeader = startArcPos + k7zStartHeaderSize;
  
  if (CrcCalc(header + 12, 20) != GetUi32(header + 8))
    return SZ_ERROR_CRC;

  nextHeaderSizeT = (size_t)nextHeaderSize;
  if (nextHeaderSizeT != nextHeaderSize)
    return SZ_ERROR_MEM;
  if (nextHeaderSizeT == 0)
    return SZ_OK;
  if (nextHeaderOffset > nextHeaderOffset + nextHeaderSize ||
      nextHeaderOffset > nextHeaderOffset + nextHeaderSize + k7zStartHeaderSize)
    return SZ_ERROR_NO_ARCHIVE;

  {
    Int64 pos = 0;
    RINOK(inStream->Seek(inStream, &pos, SZ_SEEK_END));
    if ((UInt64)pos < startArcPos + nextHeaderOffset ||
        (UInt64)pos < startArcPos + k7zStartHeaderSize + nextHeaderOffset ||
        (UInt64)pos < startArcPos + k7zStartHeaderSize + nextHeaderOffset + nextHeaderSize)
      return SZ_ERROR_INPUT_EOF;
  }

  RINOK(LookInStream_SeekTo(inStream, startArcPos + k7zStartHeaderSize + nextHeaderOffset));

  if (!Buf_Create(&buf, nextHeaderSizeT, allocTemp))
    return SZ_ERROR_MEM;

  res = LookInStream_Read(inStream, buf.data, nextHeaderSizeT);
  
  if (res == SZ_OK)
  {
    res = SZ_ERROR_ARCHIVE;
    if (CrcCalc(buf.data, nextHeaderSizeT) == nextHeaderCRC)
    {
      CSzData sd;
      UInt64 type;
      sd.Data = buf.data;
      sd.Size = buf.size;
      
      res = ReadID(&sd, &type);
      
      if (res == SZ_OK && type == k7zIdEncodedHeader)
      {
        CSzAr tempAr;
        CBuf tempBuf;
        Buf_Init(&tempBuf);
        
        SzAr_Init(&tempAr);
        res = SzReadAndDecodePackedStreams(inStream, &sd, &tempBuf, 1, p->startPosAfterHeader, &tempAr, allocTemp);
        SzAr_Free(&tempAr, allocTemp);
       
        if (res != SZ_OK)
        {
          Buf_Free(&tempBuf, allocTemp);
        }
        else
        {
          Buf_Free(&buf, allocTemp);
          buf.data = tempBuf.data;
          buf.size = tempBuf.size;
          sd.Data = buf.data;
          sd.Size = buf.size;
          res = ReadID(&sd, &type);
        }
      }
  
      if (res == SZ_OK)
      {
        if (type == k7zIdHeader)
        {
          /*
          CSzData sd2;
          unsigned ttt;
          for (ttt = 0; ttt < 40000; ttt++)
          {
            SzArEx_Free(p, allocMain);
            sd2 = sd;
            res = SzReadHeader(p, &sd2, inStream, allocMain, allocTemp);
            if (res != SZ_OK)
              break;
          }
          */
          res = SzReadHeader(p, &sd, inStream, allocMain, allocTemp);
        }
        else
          res = SZ_ERROR_UNSUPPORTED;
      }
    }
  }
 
  Buf_Free(&buf, allocTemp);
  return res;
}


static SRes SzArEx_Open(CSzArEx *p, ILookInStream *inStream,
    ISzAlloc *allocMain, ISzAlloc *allocTemp)
{
  SRes res = SzArEx_Open2(p, inStream, allocMain, allocTemp);
  if (res != SZ_OK)
    SzArEx_Free(p, allocMain);
  return res;
}


static SRes SzArEx_Extract(
    const CSzArEx *p,
    ILookInStream *inStream,
    UInt32 fileIndex,
    UInt32 *blockIndex,
    Byte **tempBuf,
    size_t *outBufferSize,
    size_t *offset,
    size_t *outSizeProcessed,
    ISzAlloc *allocMain,
    ISzAlloc *allocTemp)
{
  UInt32 folderIndex = p->FileToFolder[fileIndex];
  SRes res = SZ_OK;
  
  *offset = 0;
  *outSizeProcessed = 0;
  
  if (folderIndex == (UInt32)-1)
  {
    IAlloc_Free(allocMain, *tempBuf);
    *blockIndex = folderIndex;
    *tempBuf = NULL;
    *outBufferSize = 0;
    return SZ_OK;
  }

  if (*tempBuf == NULL || *blockIndex != folderIndex)
  {
    UInt64 unpackSizeSpec = SzAr_GetFolderUnpackSize(&p->db, folderIndex);
    /*
    UInt64 unpackSizeSpec =
        p->UnpackPositions[p->FolderToFile[folderIndex + 1]] -
        p->UnpackPositions[p->FolderToFile[folderIndex]];
    */
    size_t unpackSize = (size_t)unpackSizeSpec;

    if (unpackSize != unpackSizeSpec)
      return SZ_ERROR_MEM;
    *blockIndex = folderIndex;
    IAlloc_Free(allocMain, *tempBuf);
    *tempBuf = NULL;
    
    if (res == SZ_OK)
    {
      *outBufferSize = unpackSize;
      if (unpackSize != 0)
      {
        *tempBuf = (Byte *)IAlloc_Alloc(allocMain, unpackSize);
        if (*tempBuf == NULL)
          res = SZ_ERROR_MEM;
      }
  
      if (res == SZ_OK)
      {
        res = SzAr_DecodeFolder(&p->db, folderIndex,
            inStream, p->dataPos, *tempBuf, unpackSize, allocTemp);
      }
    }
  }

  if (res == SZ_OK)
  {
    UInt64 unpackPos = p->UnpackPositions[fileIndex];
    *offset = (size_t)(unpackPos - p->UnpackPositions[p->FolderToFile[folderIndex]]);
    *outSizeProcessed = (size_t)(p->UnpackPositions[fileIndex + 1] - unpackPos);
    if (*offset + *outSizeProcessed > *outBufferSize)
      return SZ_ERROR_FAIL;
    if (SzBitWithVals_Check(&p->CRCs, fileIndex))
      if (CrcCalc(*tempBuf + *offset, *outSizeProcessed) != p->CRCs.Vals[fileIndex])
        res = SZ_ERROR_CRC;
  }

  return res;
}


static size_t SzArEx_GetFileNameUtf16(const CSzArEx *p, size_t fileIndex, UInt16 *dest)
{
  size_t offs = p->FileNameOffsets[fileIndex];
  size_t len = p->FileNameOffsets[fileIndex + 1] - offs;
  if (dest != 0)
  {
    size_t i;
    const Byte *src = p->FileNames + offs * 2;
    for (i = 0; i < len; i++)
      dest[i] = GetUi16(src + i * 2);
  }
  return len;
}

/*
static size_t SzArEx_GetFullNameLen(const CSzArEx *p, size_t fileIndex)
{
  size_t len;
  if (!p->FileNameOffsets)
    return 1;
  len = 0;
  for (;;)
  {
    UInt32 parent = (UInt32)(Int32)-1;
    len += p->FileNameOffsets[fileIndex + 1] - p->FileNameOffsets[fileIndex];
    if SzBitWithVals_Check(&p->Parents, fileIndex)
      parent = p->Parents.Vals[fileIndex];
    if (parent == (UInt32)(Int32)-1)
      return len;
    fileIndex = parent;
  }
}

static UInt16 *SzArEx_GetFullNameUtf16_Back(const CSzArEx *p, size_t fileIndex, UInt16 *dest)
{
  Bool needSlash;
  if (!p->FileNameOffsets)
  {
    *(--dest) = 0;
    return dest;
  }
  needSlash = False;
  for (;;)
  {
    UInt32 parent = (UInt32)(Int32)-1;
    size_t curLen = p->FileNameOffsets[fileIndex + 1] - p->FileNameOffsets[fileIndex];
    SzArEx_GetFileNameUtf16(p, fileIndex, dest - curLen);
    if (needSlash)
      *(dest - 1) = '/';
    needSlash = True;
    dest -= curLen;

    if SzBitWithVals_Check(&p->Parents, fileIndex)
      parent = p->Parents.Vals[fileIndex];
    if (parent == (UInt32)(Int32)-1)
      return dest;
    fileIndex = parent;
  }
}
*/

/* 7zBuf.c -- Byte Buffer
2013-01-21 : Igor Pavlov : Public domain */

/*
#include "Precomp.h"

#include "7zBuf.h"
*/

static void Buf_Init(CBuf *p)
{
  p->data = 0;
  p->size = 0;
}

static int Buf_Create(CBuf *p, size_t size, ISzAlloc *alloc)
{
  p->size = 0;
  if (size == 0)
  {
    p->data = 0;
    return 1;
  }
  p->data = (Byte *)alloc->Alloc(alloc, size);
  if (p->data != 0)
  {
    p->size = size;
    return 1;
  }
  return 0;
}

static void Buf_Free(CBuf *p, ISzAlloc *alloc)
{
  alloc->Free(alloc, p->data);
  p->data = 0;
  p->size = 0;
}

/* 7zDec.c -- Decoding from 7z folder
2015-11-18 : Igor Pavlov : Public domain */

/* #define _7ZIP_PPMD_SUPPPORT */

/*
#include "Precomp.h"

#include <string.h>

#include "7z.h"
#include "7zCrc.h"

#include "Bcj2.h"
#include "Bra.h"
#include "CpuArch.h"
#include "Delta.h"
#include "LzmaDec.h"
#include "Lzma2Dec.h"
#ifdef _7ZIP_PPMD_SUPPPORT
#include "Ppmd7.h"
#endif
*/

#define k_Copy 0
#define k_Delta 3
#define k_LZMA2 0x21
#define k_LZMA  0x30101
#define k_BCJ   0x3030103
#define k_BCJ2  0x303011B
#define k_PPC   0x3030205
#define k_IA64  0x3030401
#define k_ARM   0x3030501
#define k_ARMT  0x3030701
#define k_SPARC 0x3030805


#ifdef _7ZIP_PPMD_SUPPPORT

#define k_PPMD 0x30401

typedef struct
{
  IByteIn p;
  const Byte *cur;
  const Byte *end;
  const Byte *begin;
  UInt64 processed;
  Bool extra;
  SRes res;
  ILookInStream *inStream;
} CByteInToLook;

static Byte ReadByte(void *pp)
{
  CByteInToLook *p = (CByteInToLook *)pp;
  if (p->cur != p->end)
    return *p->cur++;
  if (p->res == SZ_OK)
  {
    size_t size = p->cur - p->begin;
    p->processed += size;
    p->res = p->inStream->Skip(p->inStream, size);
    size = (1 << 25);
    p->res = p->inStream->Look(p->inStream, (const void **)&p->begin, &size);
    p->cur = p->begin;
    p->end = p->begin + size;
    if (size != 0)
      return *p->cur++;;
  }
  p->extra = True;
  return 0;
}

static SRes SzDecodePpmd(const Byte *props, unsigned propsSize, UInt64 inSize, ILookInStream *inStream,
    Byte *outBuffer, SizeT outSize, ISzAlloc *allocMain)
{
  CPpmd7 ppmd;
  CByteInToLook s;
  SRes res = SZ_OK;

  s.p.Read = ReadByte;
  s.inStream = inStream;
  s.begin = s.end = s.cur = NULL;
  s.extra = False;
  s.res = SZ_OK;
  s.processed = 0;

  if (propsSize != 5)
    return SZ_ERROR_UNSUPPORTED;

  {
    unsigned order = props[0];
    UInt32 memSize = GetUi32(props + 1);
    if (order < PPMD7_MIN_ORDER ||
        order > PPMD7_MAX_ORDER ||
        memSize < PPMD7_MIN_MEM_SIZE ||
        memSize > PPMD7_MAX_MEM_SIZE)
      return SZ_ERROR_UNSUPPORTED;
    Ppmd7_Construct(&ppmd);
    if (!Ppmd7_Alloc(&ppmd, memSize, allocMain))
      return SZ_ERROR_MEM;
    Ppmd7_Init(&ppmd, order);
  }
  {
    CPpmd7z_RangeDec rc;
    Ppmd7z_RangeDec_CreateVTable(&rc);
    rc.Stream = &s.p;
    if (!Ppmd7z_RangeDec_Init(&rc))
      res = SZ_ERROR_DATA;
    else if (s.extra)
      res = (s.res != SZ_OK ? s.res : SZ_ERROR_DATA);
    else
    {
      SizeT i;
      for (i = 0; i < outSize; i++)
      {
        int sym = Ppmd7_DecodeSymbol(&ppmd, &rc.p);
        if (s.extra || sym < 0)
          break;
        outBuffer[i] = (Byte)sym;
      }
      if (i != outSize)
        res = (s.res != SZ_OK ? s.res : SZ_ERROR_DATA);
      else if (s.processed + (s.cur - s.begin) != inSize || !Ppmd7z_RangeDec_IsFinishedOK(&rc))
        res = SZ_ERROR_DATA;
    }
  }
  Ppmd7_Free(&ppmd, allocMain);
  return res;
}

#endif


static SRes SzDecodeLzma(const Byte *props, unsigned propsSize, UInt64 inSize, ILookInStream *inStream,
    Byte *outBuffer, SizeT outSize, ISzAlloc *allocMain)
{
  CLzmaDec state;
  SRes res = SZ_OK;

  LzmaDec_Construct(&state);
  RINOK(LzmaDec_AllocateProbs(&state, props, propsSize, allocMain));
  state.dic = outBuffer;
  state.dicBufSize = outSize;
  LzmaDec_Init(&state);

  for (;;)
  {
    const void *inBuf = NULL;
    size_t lookahead = (1 << 18);
    if (lookahead > inSize)
      lookahead = (size_t)inSize;
    res = inStream->Look(inStream, &inBuf, &lookahead);
    if (res != SZ_OK)
      break;

    {
      SizeT inProcessed = (SizeT)lookahead, dicPos = state.dicPos;
      ELzmaStatus status;
      res = LzmaDec_DecodeToDic(&state, outSize, inBuf, &inProcessed, LZMA_FINISH_END, &status);
      lookahead -= inProcessed;
      inSize -= inProcessed;
      if (res != SZ_OK)
        break;

      if (status == LZMA_STATUS_FINISHED_WITH_MARK)
      {
        if (outSize != state.dicPos || inSize != 0)
          res = SZ_ERROR_DATA;
        break;
      }

      if (outSize == state.dicPos && inSize == 0 && status == LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK)
        break;

      if (inProcessed == 0 && dicPos == state.dicPos)
      {
        res = SZ_ERROR_DATA;
        break;
      }

      res = inStream->Skip((void *)inStream, inProcessed);
      if (res != SZ_OK)
        break;
    }
  }

  LzmaDec_FreeProbs(&state, allocMain);
  return res;
}


#ifndef _7Z_NO_METHOD_LZMA2

static SRes SzDecodeLzma2(const Byte *props, unsigned propsSize, UInt64 inSize, ILookInStream *inStream,
    Byte *outBuffer, SizeT outSize, ISzAlloc *allocMain)
{
  CLzma2Dec state;
  SRes res = SZ_OK;

  Lzma2Dec_Construct(&state);
  if (propsSize != 1)
    return SZ_ERROR_DATA;
  RINOK(Lzma2Dec_AllocateProbs(&state, props[0], allocMain));
  state.decoder.dic = outBuffer;
  state.decoder.dicBufSize = outSize;
  Lzma2Dec_Init(&state);

  for (;;)
  {
    const void *inBuf = NULL;
    size_t lookahead = (1 << 18);
    if (lookahead > inSize)
      lookahead = (size_t)inSize;
    res = inStream->Look(inStream, &inBuf, &lookahead);
    if (res != SZ_OK)
      break;

    {
      SizeT inProcessed = (SizeT)lookahead, dicPos = state.decoder.dicPos;
      ELzmaStatus status;
      res = Lzma2Dec_DecodeToDic(&state, outSize, inBuf, &inProcessed, LZMA_FINISH_END, &status);
      lookahead -= inProcessed;
      inSize -= inProcessed;
      if (res != SZ_OK)
        break;

      if (status == LZMA_STATUS_FINISHED_WITH_MARK)
      {
        if (outSize != state.decoder.dicPos || inSize != 0)
          res = SZ_ERROR_DATA;
        break;
      }

      if (inProcessed == 0 && dicPos == state.decoder.dicPos)
      {
        res = SZ_ERROR_DATA;
        break;
      }

      res = inStream->Skip((void *)inStream, inProcessed);
      if (res != SZ_OK)
        break;
    }
  }

  Lzma2Dec_FreeProbs(&state, allocMain);
  return res;
}

#endif


static SRes SzDecodeCopy(UInt64 inSize, ILookInStream *inStream, Byte *outBuffer)
{
  while (inSize > 0)
  {
    const void *inBuf;
    size_t curSize = (1 << 18);
    if (curSize > inSize)
      curSize = (size_t)inSize;
    RINOK(inStream->Look(inStream, &inBuf, &curSize));
    if (curSize == 0)
      return SZ_ERROR_INPUT_EOF;
    memcpy(outBuffer, inBuf, curSize);
    outBuffer += curSize;
    inSize -= curSize;
    RINOK(inStream->Skip((void *)inStream, curSize));
  }
  return SZ_OK;
}

static Bool IS_MAIN_METHOD(UInt32 m)
{
  switch (m)
  {
    case k_Copy:
    case k_LZMA:
    #ifndef _7Z_NO_METHOD_LZMA2
    case k_LZMA2:
    #endif
    #ifdef _7ZIP_PPMD_SUPPPORT
    case k_PPMD:
    #endif
      return True;
  }
  return False;
}

static Bool IS_SUPPORTED_CODER(const CSzCoderInfo *c)
{
  return
      c->NumStreams == 1
      /* && c->MethodID <= (UInt32)0xFFFFFFFF */
      && IS_MAIN_METHOD((UInt32)c->MethodID);
}

#define IS_BCJ2(c) ((c)->MethodID == k_BCJ2 && (c)->NumStreams == 4)

static SRes CheckSupportedFolder(const CSzFolder *f)
{
  if (f->NumCoders < 1 || f->NumCoders > 4)
    return SZ_ERROR_UNSUPPORTED;
  if (!IS_SUPPORTED_CODER(&f->Coders[0]))
    return SZ_ERROR_UNSUPPORTED;
  if (f->NumCoders == 1)
  {
    if (f->NumPackStreams != 1 || f->PackStreams[0] != 0 || f->NumBonds != 0)
      return SZ_ERROR_UNSUPPORTED;
    return SZ_OK;
  }
  
  
  #ifndef _7Z_NO_METHODS_FILTERS

  if (f->NumCoders == 2)
  {
    const CSzCoderInfo *c = &f->Coders[1];
    if (
        /* c->MethodID > (UInt32)0xFFFFFFFF || */
        c->NumStreams != 1
        || f->NumPackStreams != 1
        || f->PackStreams[0] != 0
        || f->NumBonds != 1
        || f->Bonds[0].InIndex != 1
        || f->Bonds[0].OutIndex != 0)
      return SZ_ERROR_UNSUPPORTED;
    switch ((UInt32)c->MethodID)
    {
      case k_Delta:
      case k_BCJ:
      case k_PPC:
      case k_IA64:
      case k_SPARC:
      case k_ARM:
      case k_ARMT:
        break;
      default:
        return SZ_ERROR_UNSUPPORTED;
    }
    return SZ_OK;
  }

  #endif

  
  if (f->NumCoders == 4)
  {
    if (!IS_SUPPORTED_CODER(&f->Coders[1])
        || !IS_SUPPORTED_CODER(&f->Coders[2])
        || !IS_BCJ2(&f->Coders[3]))
      return SZ_ERROR_UNSUPPORTED;
    if (f->NumPackStreams != 4
        || f->PackStreams[0] != 2
        || f->PackStreams[1] != 6
        || f->PackStreams[2] != 1
        || f->PackStreams[3] != 0
        || f->NumBonds != 3
        || f->Bonds[0].InIndex != 5 || f->Bonds[0].OutIndex != 0
        || f->Bonds[1].InIndex != 4 || f->Bonds[1].OutIndex != 1
        || f->Bonds[2].InIndex != 3 || f->Bonds[2].OutIndex != 2)
      return SZ_ERROR_UNSUPPORTED;
    return SZ_OK;
  }
  
  return SZ_ERROR_UNSUPPORTED;
}

#define CASE_BRA_CONV(isa) case k_ ## isa: isa ## _Convert(outBuffer, outSize, 0, 0); break;

static SRes SzFolder_Decode2(const CSzFolder *folder,
    const Byte *propsData,
    const UInt64 *unpackSizes,
    const UInt64 *packPositions,
    ILookInStream *inStream, UInt64 startPos,
    Byte *outBuffer, SizeT outSize, ISzAlloc *allocMain,
    Byte *tempBuf[])
{
  UInt32 ci;
  SizeT tempSizes[3] = { 0, 0, 0};
  SizeT tempSize3 = 0;
  Byte *tempBuf3 = 0;

  RINOK(CheckSupportedFolder(folder));

  for (ci = 0; ci < folder->NumCoders; ci++)
  {
    const CSzCoderInfo *coder = &folder->Coders[ci];

    if (IS_MAIN_METHOD((UInt32)coder->MethodID))
    {
      UInt32 si = 0;
      UInt64 offset;
      UInt64 inSize;
      Byte *outBufCur = outBuffer;
      SizeT outSizeCur = outSize;
      if (folder->NumCoders == 4)
      {
        UInt32 indices[] = { 3, 2, 0 };
        UInt64 unpackSize = unpackSizes[ci];
        si = indices[ci];
        if (ci < 2)
        {
          Byte *temp;
          outSizeCur = (SizeT)unpackSize;
          if (outSizeCur != unpackSize)
            return SZ_ERROR_MEM;
          temp = (Byte *)IAlloc_Alloc(allocMain, outSizeCur);
          if (!temp && outSizeCur != 0)
            return SZ_ERROR_MEM;
          outBufCur = tempBuf[1 - ci] = temp;
          tempSizes[1 - ci] = outSizeCur;
        }
        else if (ci == 2)
        {
          if (unpackSize > outSize) /* check it */
            return SZ_ERROR_PARAM;
          tempBuf3 = outBufCur = outBuffer + (outSize - (size_t)unpackSize);
          tempSize3 = outSizeCur = (SizeT)unpackSize;
        }
        else
          return SZ_ERROR_UNSUPPORTED;
      }
      offset = packPositions[si];
      inSize = packPositions[si + 1] - offset;
      RINOK(LookInStream_SeekTo(inStream, startPos + offset));

      if (coder->MethodID == k_Copy)
      {
        if (inSize != outSizeCur) /* check it */
          return SZ_ERROR_DATA;
        RINOK(SzDecodeCopy(inSize, inStream, outBufCur));
      }
      else if (coder->MethodID == k_LZMA)
      {
        RINOK(SzDecodeLzma(propsData + coder->PropsOffset, coder->PropsSize, inSize, inStream, outBufCur, outSizeCur, allocMain));
      }
      #ifndef _7Z_NO_METHOD_LZMA2
      else if (coder->MethodID == k_LZMA2)
      {
        RINOK(SzDecodeLzma2(propsData + coder->PropsOffset, coder->PropsSize, inSize, inStream, outBufCur, outSizeCur, allocMain));
      }
      #endif
      #ifdef _7ZIP_PPMD_SUPPPORT
      else if (coder->MethodID == k_PPMD)
      {
        RINOK(SzDecodePpmd(propsData + coder->PropsOffset, coder->PropsSize, inSize, inStream, outBufCur, outSizeCur, allocMain));
      }
      #endif
      else
        return SZ_ERROR_UNSUPPORTED;
    }
    else if (coder->MethodID == k_BCJ2)
    {
      UInt64 offset = packPositions[1];
      UInt64 s3Size = packPositions[2] - offset;
      
      if (ci != 3)
        return SZ_ERROR_UNSUPPORTED;
      
      tempSizes[2] = (SizeT)s3Size;
      if (tempSizes[2] != s3Size)
        return SZ_ERROR_MEM;
      tempBuf[2] = (Byte *)IAlloc_Alloc(allocMain, tempSizes[2]);
      if (!tempBuf[2] && tempSizes[2] != 0)
        return SZ_ERROR_MEM;
      
      RINOK(LookInStream_SeekTo(inStream, startPos + offset));
      RINOK(SzDecodeCopy(s3Size, inStream, tempBuf[2]));

      if ((tempSizes[0] & 3) != 0 ||
          (tempSizes[1] & 3) != 0 ||
          tempSize3 + tempSizes[0] + tempSizes[1] != outSize)
        return SZ_ERROR_DATA;

      {
        CBcj2Dec p;
        
        p.bufs[0] = tempBuf3;   p.lims[0] = tempBuf3 + tempSize3;
        p.bufs[1] = tempBuf[0]; p.lims[1] = tempBuf[0] + tempSizes[0];
        p.bufs[2] = tempBuf[1]; p.lims[2] = tempBuf[1] + tempSizes[1];
        p.bufs[3] = tempBuf[2]; p.lims[3] = tempBuf[2] + tempSizes[2];
        
        p.dest = outBuffer;
        p.destLim = outBuffer + outSize;
        
        Bcj2Dec_Init(&p);
        RINOK(Bcj2Dec_Decode(&p));

        {
          unsigned i;
          for (i = 0; i < 4; i++)
            if (p.bufs[i] != p.lims[i])
              return SZ_ERROR_DATA;
          
          if (!Bcj2Dec_IsFinished(&p))
            return SZ_ERROR_DATA;

          if (p.dest != p.destLim
             || p.state != BCJ2_STREAM_MAIN)
            return SZ_ERROR_DATA;
        }
      }
    }
    #ifndef _7Z_NO_METHODS_FILTERS
    else if (ci == 1)
    {
      if (coder->MethodID == k_Delta)
      {
        if (coder->PropsSize != 1)
          return SZ_ERROR_UNSUPPORTED;
        {
          Byte state[DELTA_STATE_SIZE];
          Delta_Init(state);
          Delta_Decode(state, (unsigned)(propsData[coder->PropsOffset]) + 1, outBuffer, outSize);
        }
      }
      else
      {
        if (coder->PropsSize != 0)
          return SZ_ERROR_UNSUPPORTED;
        switch (coder->MethodID)
        {
          case k_BCJ:
          {
            UInt32 state;
            x86_Convert_Init(state);
            x86_Convert(outBuffer, outSize, 0, &state, 0);
            break;
          }
          CASE_BRA_CONV(PPC)
          CASE_BRA_CONV(IA64)
          CASE_BRA_CONV(SPARC)
          CASE_BRA_CONV(ARM)
          CASE_BRA_CONV(ARMT)
          default:
            return SZ_ERROR_UNSUPPORTED;
        }
      }
    }
    #endif
    else
      return SZ_ERROR_UNSUPPORTED;
  }

  return SZ_OK;
}


static SRes SzAr_DecodeFolder(const CSzAr *p, UInt32 folderIndex,
    ILookInStream *inStream, UInt64 startPos,
    Byte *outBuffer, size_t outSize,
    ISzAlloc *allocMain)
{
  SRes res;
  CSzFolder folder;
  CSzData sd;
  
  const Byte *data = p->CodersData + p->FoCodersOffsets[folderIndex];
  sd.Data = data;
  sd.Size = p->FoCodersOffsets[folderIndex + 1] - p->FoCodersOffsets[folderIndex];
  
  res = SzGetNextFolderItem(&folder, &sd);
  
  if (res != SZ_OK)
    return res;

  if (sd.Size != 0
      || folder.UnpackStream != p->FoToMainUnpackSizeIndex[folderIndex]
      || outSize != SzAr_GetFolderUnpackSize(p, folderIndex))
    return SZ_ERROR_FAIL;
  {
    unsigned i;
    Byte *tempBuf[3] = { 0, 0, 0};

    res = SzFolder_Decode2(&folder, data,
        &p->CoderUnpackSizes[p->FoToCoderUnpackSizes[folderIndex]],
        p->PackPositions + p->FoStartPackStreamIndex[folderIndex],
        inStream, startPos,
        outBuffer, (SizeT)outSize, allocMain, tempBuf);
    
    for (i = 0; i < 3; i++)
      IAlloc_Free(allocMain, tempBuf[i]);

    if (res == SZ_OK)
      if (SzBitWithVals_Check(&p->FolderCRCs, folderIndex))
        if (CrcCalc(outBuffer, outSize) != p->FolderCRCs.Vals[folderIndex])
          res = SZ_ERROR_CRC;

    return res;
  }
}

/* Bcj2.c -- BCJ2 Decoder (Converter for x86 code)
2015-08-01 : Igor Pavlov : Public domain */

/*
#include "Precomp.h"

#include "Bcj2.h"
#include "CpuArch.h"
*/

#define CProb UInt16

#define kTopValue ((UInt32)1 << 24)
#define kNumModelBits 11
#define kBitModelTotal (1 << kNumModelBits)
#define kNumMoveBits 5

#define _IF_BIT_0 ttt = *prob; bound = (p->range >> kNumModelBits) * ttt; if (p->code < bound)
#define _UPDATE_0 p->range = bound; *prob = (CProb)(ttt + ((kBitModelTotal - ttt) >> kNumMoveBits));
#define _UPDATE_1 p->range -= bound; p->code -= bound; *prob = (CProb)(ttt - (ttt >> kNumMoveBits));

static void Bcj2Dec_Init(CBcj2Dec *p)
{
  unsigned i;

  p->state = BCJ2_DEC_STATE_OK;
  p->ip = 0;
  p->temp[3] = 0;
  p->range = 0;
  p->code = 0;
  for (i = 0; i < sizeof(p->probs) / sizeof(p->probs[0]); i++)
    p->probs[i] = kBitModelTotal >> 1;
}

static SRes Bcj2Dec_Decode(CBcj2Dec *p)
{
  if (p->range <= 5)
  {
    p->state = BCJ2_DEC_STATE_OK;
    for (; p->range != 5; p->range++)
    {
      if (p->range == 1 && p->code != 0)
        return SZ_ERROR_DATA;
      
      if (p->bufs[BCJ2_STREAM_RC] == p->lims[BCJ2_STREAM_RC])
      {
        p->state = BCJ2_STREAM_RC;
        return SZ_OK;
      }

      p->code = (p->code << 8) | *(p->bufs[BCJ2_STREAM_RC])++;
    }
    
    if (p->code == 0xFFFFFFFF)
      return SZ_ERROR_DATA;
    
    p->range = 0xFFFFFFFF;
  }
  else if (p->state >= BCJ2_DEC_STATE_ORIG_0)
  {
    while (p->state <= BCJ2_DEC_STATE_ORIG_3)
    {
      Byte *dest = p->dest;
      if (dest == p->destLim)
        return SZ_OK;
      *dest = p->temp[p->state++ - BCJ2_DEC_STATE_ORIG_0];
      p->dest = dest + 1;
    }
  }

  /*
  if (BCJ2_IS_32BIT_STREAM(p->state))
  {
    const Byte *cur = p->bufs[p->state];
    if (cur == p->lims[p->state])
      return SZ_OK;
    p->bufs[p->state] = cur + 4;
    
    {
      UInt32 val;
      Byte *dest;
      SizeT rem;
      
      p->ip += 4;
      val = GetBe32(cur) - p->ip;
      dest = p->dest;
      rem = p->destLim - dest;
      if (rem < 4)
      {
        SizeT i;
        SetUi32(p->temp, val);
        for (i = 0; i < rem; i++)
          dest[i] = p->temp[i];
        p->dest = dest + rem;
        p->state = BCJ2_DEC_STATE_ORIG_0 + (unsigned)rem;
        return SZ_OK;
      }
      SetUi32(dest, val);
      p->temp[3] = (Byte)(val >> 24);
      p->dest = dest + 4;
      p->state = BCJ2_DEC_STATE_OK;
    }
  }
  */

  for (;;)
  {
    if (BCJ2_IS_32BIT_STREAM(p->state))
      p->state = BCJ2_DEC_STATE_OK;
    else
    {
      if (p->range < kTopValue)
      {
        if (p->bufs[BCJ2_STREAM_RC] == p->lims[BCJ2_STREAM_RC])
        {
          p->state = BCJ2_STREAM_RC;
          return SZ_OK;
        }
        p->range <<= 8;
        p->code = (p->code << 8) | *(p->bufs[BCJ2_STREAM_RC])++;
      }

      {
        const Byte *src = p->bufs[BCJ2_STREAM_MAIN];
        const Byte *srcLim;
        Byte *dest;
        SizeT num = p->lims[BCJ2_STREAM_MAIN] - src;
        
        if (num == 0)
        {
          p->state = BCJ2_STREAM_MAIN;
          return SZ_OK;
        }
        
        dest = p->dest;
        if (num > (SizeT)(p->destLim - dest))
        {
          num = p->destLim - dest;
          if (num == 0)
          {
            p->state = BCJ2_DEC_STATE_ORIG;
            return SZ_OK;
          }
        }
       
        srcLim = src + num;

        if (p->temp[3] == 0x0F && (src[0] & 0xF0) == 0x80)
          *dest = src[0];
        else for (;;)
        {
          Byte b = *src;
          *dest = b;
          if (b != 0x0F)
          {
            if ((b & 0xFE) == 0xE8)
              break;
            dest++;
            if (++src != srcLim)
              continue;
            break;
          }
          dest++;
          if (++src == srcLim)
            break;
          if ((*src & 0xF0) != 0x80)
            continue;
          *dest = *src;
          break;
        }
        
        num = src - p->bufs[BCJ2_STREAM_MAIN];
        
        if (src == srcLim)
        {
          p->temp[3] = src[-1];
          p->bufs[BCJ2_STREAM_MAIN] = src;
          p->ip += (UInt32)num;
          p->dest += num;
          p->state =
            p->bufs[BCJ2_STREAM_MAIN] ==
            p->lims[BCJ2_STREAM_MAIN] ?
              (unsigned)BCJ2_STREAM_MAIN :
              (unsigned)BCJ2_DEC_STATE_ORIG;
          return SZ_OK;
        }
        
        {
          UInt32 bound, ttt;
          CProb *prob;
          Byte b = src[0];
          Byte prev = (Byte)(num == 0 ? p->temp[3] : src[-1]);
          
          p->temp[3] = b;
          p->bufs[BCJ2_STREAM_MAIN] = src + 1;
          num++;
          p->ip += (UInt32)num;
          p->dest += num;
          
          prob = p->probs + (unsigned)(b == 0xE8 ? 2 + (unsigned)prev : (b == 0xE9 ? 1 : 0));
          
          _IF_BIT_0
          {
            _UPDATE_0
            continue;
          }
          _UPDATE_1
            
        }
      }
    }

    {
      UInt32 val;
      unsigned cj = (p->temp[3] == 0xE8) ? BCJ2_STREAM_CALL : BCJ2_STREAM_JUMP;
      const Byte *cur = p->bufs[cj];
      Byte *dest;
      SizeT rem;
      
      if (cur == p->lims[cj])
      {
        p->state = cj;
        break;
      }
      
      val = GetBe32(cur);
      p->bufs[cj] = cur + 4;

      p->ip += 4;
      val -= p->ip;
      dest = p->dest;
      rem = p->destLim - dest;
      
      if (rem < 4)
      {
        SizeT i;
        SetUi32(p->temp, val);
        for (i = 0; i < rem; i++)
          dest[i] = p->temp[i];
        p->dest = dest + rem;
        p->state = BCJ2_DEC_STATE_ORIG_0 + (unsigned)rem;
        break;
      }
      
      SetUi32(dest, val);
      p->temp[3] = (Byte)(val >> 24);
      p->dest = dest + 4;
    }
  }

  if (p->range < kTopValue && p->bufs[BCJ2_STREAM_RC] != p->lims[BCJ2_STREAM_RC])
  {
    p->range <<= 8;
    p->code = (p->code << 8) | *(p->bufs[BCJ2_STREAM_RC])++;
  }

  return SZ_OK;
}

#undef kTopValue  /* reused later. --ryan. */
#undef kBitModelTotal  /* reused later. --ryan. */


/* Bra.c -- Converters for RISC code
2010-04-16 : Igor Pavlov : Public domain */

/*
#include "Precomp.h"

#include "Bra.h"
*/

static SizeT ARM_Convert(Byte *data, SizeT size, UInt32 ip, int encoding)
{
  SizeT i;
  if (size < 4)
    return 0;
  size -= 4;
  ip += 8;
  for (i = 0; i <= size; i += 4)
  {
    if (data[i + 3] == 0xEB)
    {
      UInt32 dest;
      UInt32 src = ((UInt32)data[i + 2] << 16) | ((UInt32)data[i + 1] << 8) | (data[i + 0]);
      src <<= 2;
      if (encoding)
        dest = ip + (UInt32)i + src;
      else
        dest = src - (ip + (UInt32)i);
      dest >>= 2;
      data[i + 2] = (Byte)(dest >> 16);
      data[i + 1] = (Byte)(dest >> 8);
      data[i + 0] = (Byte)dest;
    }
  }
  return i;
}

static SizeT ARMT_Convert(Byte *data, SizeT size, UInt32 ip, int encoding)
{
  SizeT i;
  if (size < 4)
    return 0;
  size -= 4;
  ip += 4;
  for (i = 0; i <= size; i += 2)
  {
    if ((data[i + 1] & 0xF8) == 0xF0 &&
        (data[i + 3] & 0xF8) == 0xF8)
    {
      UInt32 dest;
      UInt32 src =
        (((UInt32)data[i + 1] & 0x7) << 19) |
        ((UInt32)data[i + 0] << 11) |
        (((UInt32)data[i + 3] & 0x7) << 8) |
        (data[i + 2]);
      
      src <<= 1;
      if (encoding)
        dest = ip + (UInt32)i + src;
      else
        dest = src - (ip + (UInt32)i);
      dest >>= 1;
      
      data[i + 1] = (Byte)(0xF0 | ((dest >> 19) & 0x7));
      data[i + 0] = (Byte)(dest >> 11);
      data[i + 3] = (Byte)(0xF8 | ((dest >> 8) & 0x7));
      data[i + 2] = (Byte)dest;
      i += 2;
    }
  }
  return i;
}

static SizeT PPC_Convert(Byte *data, SizeT size, UInt32 ip, int encoding)
{
  SizeT i;
  if (size < 4)
    return 0;
  size -= 4;
  for (i = 0; i <= size; i += 4)
  {
    if ((data[i] >> 2) == 0x12 && (data[i + 3] & 3) == 1)
    {
      UInt32 src = ((UInt32)(data[i + 0] & 3) << 24) |
        ((UInt32)data[i + 1] << 16) |
        ((UInt32)data[i + 2] << 8) |
        ((UInt32)data[i + 3] & (~3));
      
      UInt32 dest;
      if (encoding)
        dest = ip + (UInt32)i + src;
      else
        dest = src - (ip + (UInt32)i);
      data[i + 0] = (Byte)(0x48 | ((dest >> 24) &  0x3));
      data[i + 1] = (Byte)(dest >> 16);
      data[i + 2] = (Byte)(dest >> 8);
      data[i + 3] &= 0x3;
      data[i + 3] |= dest;
    }
  }
  return i;
}

static SizeT SPARC_Convert(Byte *data, SizeT size, UInt32 ip, int encoding)
{
  UInt32 i;
  if (size < 4)
    return 0;
  size -= 4;
  for (i = 0; i <= size; i += 4)
  {
    if ((data[i] == 0x40 && (data[i + 1] & 0xC0) == 0x00) ||
        (data[i] == 0x7F && (data[i + 1] & 0xC0) == 0xC0))
    {
      UInt32 src =
        ((UInt32)data[i + 0] << 24) |
        ((UInt32)data[i + 1] << 16) |
        ((UInt32)data[i + 2] << 8) |
        ((UInt32)data[i + 3]);
      UInt32 dest;
      
      src <<= 2;
      if (encoding)
        dest = ip + i + src;
      else
        dest = src - (ip + i);
      dest >>= 2;
      
      dest = (((0 - ((dest >> 22) & 1)) << 22) & 0x3FFFFFFF) | (dest & 0x3FFFFF) | 0x40000000;

      data[i + 0] = (Byte)(dest >> 24);
      data[i + 1] = (Byte)(dest >> 16);
      data[i + 2] = (Byte)(dest >> 8);
      data[i + 3] = (Byte)dest;
    }
  }
  return i;
}

/* Bra86.c -- Converter for x86 code (BCJ)
2013-11-12 : Igor Pavlov : Public domain */

/*
#include "Precomp.h"

#include "Bra.h"
*/

#define Test86MSByte(b) ((((b) + 1) & 0xFE) == 0)

static SizeT x86_Convert(Byte *data, SizeT size, UInt32 ip, UInt32 *state, int encoding)
{
  SizeT pos = 0;
  UInt32 mask = *state & 7;
  if (size < 5)
    return 0;
  size -= 4;
  ip += 5;

  for (;;)
  {
    Byte *p = data + pos;
    const Byte *limit = data + size;
    for (; p < limit; p++)
      if ((*p & 0xFE) == 0xE8)
        break;

    {
      SizeT d = (SizeT)(p - data - pos);
      pos = (SizeT)(p - data);
      if (p >= limit)
      {
        *state = (d > 2 ? 0 : mask >> (unsigned)d);
        return pos;
      }
      if (d > 2)
        mask = 0;
      else
      {
        mask >>= (unsigned)d;
        if (mask != 0 && (mask > 4 || mask == 3 || Test86MSByte(p[(mask >> 1) + 1])))
        {
          mask = (mask >> 1) | 4;
          pos++;
          continue;
        }
      }
    }

    if (Test86MSByte(p[4]))
    {
      UInt32 v = ((UInt32)p[4] << 24) | ((UInt32)p[3] << 16) | ((UInt32)p[2] << 8) | ((UInt32)p[1]);
      UInt32 cur = ip + (UInt32)pos;
      pos += 5;
      if (encoding)
        v += cur;
      else
        v -= cur;
      if (mask != 0)
      {
        unsigned sh = (mask & 6) << 2;
        if (Test86MSByte((Byte)(v >> sh)))
        {
          v ^= (((UInt32)0x100 << sh) - 1);
          if (encoding)
            v += cur;
          else
            v -= cur;
        }
        mask = 0;
      }
      p[1] = (Byte)v;
      p[2] = (Byte)(v >> 8);
      p[3] = (Byte)(v >> 16);
      p[4] = (Byte)(0 - ((v >> 24) & 1));
    }
    else
    {
      mask = (mask >> 1) | 4;
      pos++;
    }
  }
}


/* BraIA64.c -- Converter for IA-64 code
2013-11-12 : Igor Pavlov : Public domain */

/*
#include "Precomp.h"

#include "Bra.h"
*/
static const Byte kBranchTable[32] =
{
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  4, 4, 6, 6, 0, 0, 7, 7,
  4, 4, 0, 0, 4, 4, 0, 0
};

static SizeT IA64_Convert(Byte *data, SizeT size, UInt32 ip, int encoding)
{
  SizeT i;
  if (size < 16)
    return 0;
  size -= 16;
  for (i = 0; i <= size; i += 16)
  {
    UInt32 instrTemplate = data[i] & 0x1F;
    UInt32 mask = kBranchTable[instrTemplate];
    UInt32 bitPos = 5;
    int slot;
    for (slot = 0; slot < 3; slot++, bitPos += 41)
    {
      UInt32 bytePos, bitRes;
      UInt64 instruction, instNorm;
      int j;
      if (((mask >> slot) & 1) == 0)
        continue;
      bytePos = (bitPos >> 3);
      bitRes = bitPos & 0x7;
      instruction = 0;
      for (j = 0; j < 6; j++)
        instruction += (UInt64)data[i + j + bytePos] << (8 * j);

      instNorm = instruction >> bitRes;
      if (((instNorm >> 37) & 0xF) == 0x5 && ((instNorm >> 9) & 0x7) == 0)
      {
        UInt32 src = (UInt32)((instNorm >> 13) & 0xFFFFF);
        UInt32 dest;
        src |= ((UInt32)(instNorm >> 36) & 1) << 20;
        
        src <<= 4;
        
        if (encoding)
          dest = ip + (UInt32)i + src;
        else
          dest = src - (ip + (UInt32)i);
        
        dest >>= 4;
        
        instNorm &= ~((UInt64)(0x8FFFFF) << 13);
        instNorm |= ((UInt64)(dest & 0xFFFFF) << 13);
        instNorm |= ((UInt64)(dest & 0x100000) << (36 - 20));
        
        instruction &= (1 << bitRes) - 1;
        instruction |= (instNorm << bitRes);
        for (j = 0; j < 6; j++)
          data[i + j + bytePos] = (Byte)(instruction >> (8 * j));
      }
    }
  }
  return i;
}


/* Delta.c -- Delta converter
2009-05-26 : Igor Pavlov : Public domain */

/*
#include "Precomp.h"

#include "Delta.h"
*/

static void Delta_Init(Byte *state)
{
  unsigned i;
  for (i = 0; i < DELTA_STATE_SIZE; i++)
    state[i] = 0;
}

static void MyMemCpy(Byte *dest, const Byte *src, unsigned size)
{
  unsigned i;
  for (i = 0; i < size; i++)
    dest[i] = src[i];
}

static void Delta_Decode(Byte *state, unsigned delta, Byte *data, SizeT size)
{
  Byte buf[DELTA_STATE_SIZE];
  unsigned j = 0;
  MyMemCpy(buf, state, delta);
  {
    SizeT i;
    for (i = 0; i < size;)
    {
      for (j = 0; j < delta && i < size; i++, j++)
      {
        buf[j] = data[i] = (Byte)(buf[j] + data[i]);
      }
    }
  }
  if (j == delta)
    j = 0;
  MyMemCpy(state, buf + j, delta - j);
  MyMemCpy(state + delta - j, buf, j);
}

/* LzmaDec.c -- LZMA Decoder
2016-05-16 : Igor Pavlov : Public domain */

/*
#include "Precomp.h"

#include "LzmaDec.h"

#include <string.h>
*/

#define kNumTopBits 24
#define kTopValue ((UInt32)1 << kNumTopBits)

#define kNumBitModelTotalBits 11
#define kBitModelTotal (1 << kNumBitModelTotalBits)
#define kNumMoveBits 5

#define RC_INIT_SIZE 5

#define NORMALIZE if (range < kTopValue) { range <<= 8; code = (code << 8) | (*buf++); }

#define IF_BIT_0(p) ttt = *(p); NORMALIZE; bound = (range >> kNumBitModelTotalBits) * ttt; if (code < bound)
#define UPDATE_0(p) range = bound; *(p) = (CLzmaProb)(ttt + ((kBitModelTotal - ttt) >> kNumMoveBits));
#define UPDATE_1(p) range -= bound; code -= bound; *(p) = (CLzmaProb)(ttt - (ttt >> kNumMoveBits));
#define GET_BIT2(p, i, A0, A1) IF_BIT_0(p) \
  { UPDATE_0(p); i = (i + i); A0; } else \
  { UPDATE_1(p); i = (i + i) + 1; A1; }
#define GET_BIT(p, i) GET_BIT2(p, i, ; , ;)

#define TREE_GET_BIT(probs, i) { GET_BIT((probs + i), i); }
#define TREE_DECODE(probs, limit, i) \
  { i = 1; do { TREE_GET_BIT(probs, i); } while (i < limit); i -= limit; }

/* #define _LZMA_SIZE_OPT */

#ifdef _LZMA_SIZE_OPT
#define TREE_6_DECODE(probs, i) TREE_DECODE(probs, (1 << 6), i)
#else
#define TREE_6_DECODE(probs, i) \
  { i = 1; \
  TREE_GET_BIT(probs, i); \
  TREE_GET_BIT(probs, i); \
  TREE_GET_BIT(probs, i); \
  TREE_GET_BIT(probs, i); \
  TREE_GET_BIT(probs, i); \
  TREE_GET_BIT(probs, i); \
  i -= 0x40; }
#endif

#define NORMAL_LITER_DEC GET_BIT(prob + symbol, symbol)
#define MATCHED_LITER_DEC \
  matchByte <<= 1; \
  bit = (matchByte & offs); \
  probLit = prob + offs + bit + symbol; \
  GET_BIT2(probLit, symbol, offs &= ~bit, offs &= bit)

#define NORMALIZE_CHECK if (range < kTopValue) { if (buf >= bufLimit) return DUMMY_ERROR; range <<= 8; code = (code << 8) | (*buf++); }

#define IF_BIT_0_CHECK(p) ttt = *(p); NORMALIZE_CHECK; bound = (range >> kNumBitModelTotalBits) * ttt; if (code < bound)
#define UPDATE_0_CHECK range = bound;
#define UPDATE_1_CHECK range -= bound; code -= bound;
#define GET_BIT2_CHECK(p, i, A0, A1) IF_BIT_0_CHECK(p) \
  { UPDATE_0_CHECK; i = (i + i); A0; } else \
  { UPDATE_1_CHECK; i = (i + i) + 1; A1; }
#define GET_BIT_CHECK(p, i) GET_BIT2_CHECK(p, i, ; , ;)
#define TREE_DECODE_CHECK(probs, limit, i) \
  { i = 1; do { GET_BIT_CHECK(probs + i, i) } while (i < limit); i -= limit; }


#define kNumPosBitsMax 4
#define kNumPosStatesMax (1 << kNumPosBitsMax)

#define kLenNumLowBits 3
#define kLenNumLowSymbols (1 << kLenNumLowBits)
#define kLenNumMidBits 3
#define kLenNumMidSymbols (1 << kLenNumMidBits)
#define kLenNumHighBits 8
#define kLenNumHighSymbols (1 << kLenNumHighBits)

#define LenChoice 0
#define LenChoice2 (LenChoice + 1)
#define LenLow (LenChoice2 + 1)
#define LenMid (LenLow + (kNumPosStatesMax << kLenNumLowBits))
#define LenHigh (LenMid + (kNumPosStatesMax << kLenNumMidBits))
#define kNumLenProbs (LenHigh + kLenNumHighSymbols)


#define kNumStates 12
#define kNumLitStates 7

#define kStartPosModelIndex 4
#define kEndPosModelIndex 14
#define kNumFullDistances (1 << (kEndPosModelIndex >> 1))

#define kNumPosSlotBits 6
#define kNumLenToPosStates 4

#define kNumAlignBits 4
#define kAlignTableSize (1 << kNumAlignBits)

#define kMatchMinLen 2
#define kMatchSpecLenStart (kMatchMinLen + kLenNumLowSymbols + kLenNumMidSymbols + kLenNumHighSymbols)

#define IsMatch 0
#define IsRep (IsMatch + (kNumStates << kNumPosBitsMax))
#define IsRepG0 (IsRep + kNumStates)
#define IsRepG1 (IsRepG0 + kNumStates)
#define IsRepG2 (IsRepG1 + kNumStates)
#define IsRep0Long (IsRepG2 + kNumStates)
#define PosSlot (IsRep0Long + (kNumStates << kNumPosBitsMax))
#define SpecPos (PosSlot + (kNumLenToPosStates << kNumPosSlotBits))
#define Align (SpecPos + kNumFullDistances - kEndPosModelIndex)
#define LenCoder (Align + kAlignTableSize)
#define RepLenCoder (LenCoder + kNumLenProbs)
#define Literal (RepLenCoder + kNumLenProbs)

#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE 0x300

#if Literal != LZMA_BASE_SIZE
StopCompilingDueBUG
#endif

#define LzmaProps_GetNumProbs(p) (Literal + ((UInt32)LZMA_LIT_SIZE << ((p)->lc + (p)->lp)))

#define LZMA_DIC_MIN (1 << 12)

/* First LZMA-symbol is always decoded.
And it decodes new LZMA-symbols while (buf < bufLimit), but "buf" is without last normalization
Out:
  Result:
    SZ_OK - OK
    SZ_ERROR_DATA - Error
  p->remainLen:
    < kMatchSpecLenStart : normal remain
    = kMatchSpecLenStart : finished
    = kMatchSpecLenStart + 1 : Flush marker (unused now)
    = kMatchSpecLenStart + 2 : State Init Marker (unused now)
*/

static int MY_FAST_CALL LzmaDec_DecodeReal(CLzmaDec *p, SizeT limit, const Byte *bufLimit)
{
  CLzmaProb *probs = p->probs;

  unsigned state = p->state;
  UInt32 rep0 = p->reps[0], rep1 = p->reps[1], rep2 = p->reps[2], rep3 = p->reps[3];
  unsigned pbMask = ((unsigned)1 << (p->prop.pb)) - 1;
  unsigned lpMask = ((unsigned)1 << (p->prop.lp)) - 1;
  unsigned lc = p->prop.lc;

  Byte *dic = p->dic;
  SizeT dicBufSize = p->dicBufSize;
  SizeT dicPos = p->dicPos;
  
  UInt32 processedPos = p->processedPos;
  UInt32 checkDicSize = p->checkDicSize;
  unsigned len = 0;

  const Byte *buf = p->buf;
  UInt32 range = p->range;
  UInt32 code = p->code;

  do
  {
    CLzmaProb *prob;
    UInt32 bound;
    unsigned ttt;
    unsigned posState = processedPos & pbMask;

    prob = probs + IsMatch + (state << kNumPosBitsMax) + posState;
    IF_BIT_0(prob)
    {
      unsigned symbol;
      UPDATE_0(prob);
      prob = probs + Literal;
      if (processedPos != 0 || checkDicSize != 0)
        prob += ((UInt32)LZMA_LIT_SIZE * (((processedPos & lpMask) << lc) +
            (dic[(dicPos == 0 ? dicBufSize : dicPos) - 1] >> (8 - lc))));
      processedPos++;

      if (state < kNumLitStates)
      {
        state -= (state < 4) ? state : 3;
        symbol = 1;
        #ifdef _LZMA_SIZE_OPT
        do { NORMAL_LITER_DEC } while (symbol < 0x100);
        #else
        NORMAL_LITER_DEC
        NORMAL_LITER_DEC
        NORMAL_LITER_DEC
        NORMAL_LITER_DEC
        NORMAL_LITER_DEC
        NORMAL_LITER_DEC
        NORMAL_LITER_DEC
        NORMAL_LITER_DEC
        #endif
      }
      else
      {
        unsigned matchByte = dic[dicPos - rep0 + (dicPos < rep0 ? dicBufSize : 0)];
        unsigned offs = 0x100;
        state -= (state < 10) ? 3 : 6;
        symbol = 1;
        #ifdef _LZMA_SIZE_OPT
        do
        {
          unsigned bit;
          CLzmaProb *probLit;
          MATCHED_LITER_DEC
        }
        while (symbol < 0x100);
        #else
        {
          unsigned bit;
          CLzmaProb *probLit;
          MATCHED_LITER_DEC
          MATCHED_LITER_DEC
          MATCHED_LITER_DEC
          MATCHED_LITER_DEC
          MATCHED_LITER_DEC
          MATCHED_LITER_DEC
          MATCHED_LITER_DEC
          MATCHED_LITER_DEC
        }
        #endif
      }

      dic[dicPos++] = (Byte)symbol;
      continue;
    }
    
    {
      UPDATE_1(prob);
      prob = probs + IsRep + state;
      IF_BIT_0(prob)
      {
        UPDATE_0(prob);
        state += kNumStates;
        prob = probs + LenCoder;
      }
      else
      {
        UPDATE_1(prob);
        if (checkDicSize == 0 && processedPos == 0)
          return SZ_ERROR_DATA;
        prob = probs + IsRepG0 + state;
        IF_BIT_0(prob)
        {
          UPDATE_0(prob);
          prob = probs + IsRep0Long + (state << kNumPosBitsMax) + posState;
          IF_BIT_0(prob)
          {
            UPDATE_0(prob);
            dic[dicPos] = dic[dicPos - rep0 + (dicPos < rep0 ? dicBufSize : 0)];
            dicPos++;
            processedPos++;
            state = state < kNumLitStates ? 9 : 11;
            continue;
          }
          UPDATE_1(prob);
        }
        else
        {
          UInt32 distance;
          UPDATE_1(prob);
          prob = probs + IsRepG1 + state;
          IF_BIT_0(prob)
          {
            UPDATE_0(prob);
            distance = rep1;
          }
          else
          {
            UPDATE_1(prob);
            prob = probs + IsRepG2 + state;
            IF_BIT_0(prob)
            {
              UPDATE_0(prob);
              distance = rep2;
            }
            else
            {
              UPDATE_1(prob);
              distance = rep3;
              rep3 = rep2;
            }
            rep2 = rep1;
          }
          rep1 = rep0;
          rep0 = distance;
        }
        state = state < kNumLitStates ? 8 : 11;
        prob = probs + RepLenCoder;
      }
      
      #ifdef _LZMA_SIZE_OPT
      {
        unsigned lim, offset;
        CLzmaProb *probLen = prob + LenChoice;
        IF_BIT_0(probLen)
        {
          UPDATE_0(probLen);
          probLen = prob + LenLow + (posState << kLenNumLowBits);
          offset = 0;
          lim = (1 << kLenNumLowBits);
        }
        else
        {
          UPDATE_1(probLen);
          probLen = prob + LenChoice2;
          IF_BIT_0(probLen)
          {
            UPDATE_0(probLen);
            probLen = prob + LenMid + (posState << kLenNumMidBits);
            offset = kLenNumLowSymbols;
            lim = (1 << kLenNumMidBits);
          }
          else
          {
            UPDATE_1(probLen);
            probLen = prob + LenHigh;
            offset = kLenNumLowSymbols + kLenNumMidSymbols;
            lim = (1 << kLenNumHighBits);
          }
        }
        TREE_DECODE(probLen, lim, len);
        len += offset;
      }
      #else
      {
        CLzmaProb *probLen = prob + LenChoice;
        IF_BIT_0(probLen)
        {
          UPDATE_0(probLen);
          probLen = prob + LenLow + (posState << kLenNumLowBits);
          len = 1;
          TREE_GET_BIT(probLen, len);
          TREE_GET_BIT(probLen, len);
          TREE_GET_BIT(probLen, len);
          len -= 8;
        }
        else
        {
          UPDATE_1(probLen);
          probLen = prob + LenChoice2;
          IF_BIT_0(probLen)
          {
            UPDATE_0(probLen);
            probLen = prob + LenMid + (posState << kLenNumMidBits);
            len = 1;
            TREE_GET_BIT(probLen, len);
            TREE_GET_BIT(probLen, len);
            TREE_GET_BIT(probLen, len);
          }
          else
          {
            UPDATE_1(probLen);
            probLen = prob + LenHigh;
            TREE_DECODE(probLen, (1 << kLenNumHighBits), len);
            len += kLenNumLowSymbols + kLenNumMidSymbols;
          }
        }
      }
      #endif

      if (state >= kNumStates)
      {
        UInt32 distance;
        prob = probs + PosSlot +
            ((len < kNumLenToPosStates ? len : kNumLenToPosStates - 1) << kNumPosSlotBits);
        TREE_6_DECODE(prob, distance);
        if (distance >= kStartPosModelIndex)
        {
          unsigned posSlot = (unsigned)distance;
          unsigned numDirectBits = (unsigned)(((distance >> 1) - 1));
          distance = (2 | (distance & 1));
          if (posSlot < kEndPosModelIndex)
          {
            distance <<= numDirectBits;
            prob = probs + SpecPos + distance - posSlot - 1;
            {
              UInt32 mask = 1;
              unsigned i = 1;
              do
              {
                GET_BIT2(prob + i, i, ; , distance |= mask);
                mask <<= 1;
              }
              while (--numDirectBits != 0);
            }
          }
          else
          {
            numDirectBits -= kNumAlignBits;
            do
            {
              NORMALIZE
              range >>= 1;
              
              {
                UInt32 t;
                code -= range;
                t = (0 - ((UInt32)code >> 31)); /* (UInt32)((Int32)code >> 31) */
                distance = (distance << 1) + (t + 1);
                code += range & t;
              }
              /*
              distance <<= 1;
              if (code >= range)
              {
                code -= range;
                distance |= 1;
              }
              */
            }
            while (--numDirectBits != 0);
            prob = probs + Align;
            distance <<= kNumAlignBits;
            {
              unsigned i = 1;
              GET_BIT2(prob + i, i, ; , distance |= 1);
              GET_BIT2(prob + i, i, ; , distance |= 2);
              GET_BIT2(prob + i, i, ; , distance |= 4);
              GET_BIT2(prob + i, i, ; , distance |= 8);
            }
            if (distance == (UInt32)0xFFFFFFFF)
            {
              len += kMatchSpecLenStart;
              state -= kNumStates;
              break;
            }
          }
        }
        
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        rep0 = distance + 1;
        if (checkDicSize == 0)
        {
          if (distance >= processedPos)
          {
            p->dicPos = dicPos;
            return SZ_ERROR_DATA;
          }
        }
        else if (distance >= checkDicSize)
        {
          p->dicPos = dicPos;
          return SZ_ERROR_DATA;
        }
        state = (state < kNumStates + kNumLitStates) ? kNumLitStates : kNumLitStates + 3;
      }

      len += kMatchMinLen;

      {
        SizeT rem;
        unsigned curLen;
        SizeT pos;
        
        if ((rem = limit - dicPos) == 0)
        {
          p->dicPos = dicPos;
          return SZ_ERROR_DATA;
        }
        
        curLen = ((rem < len) ? (unsigned)rem : len);
        pos = dicPos - rep0 + (dicPos < rep0 ? dicBufSize : 0);

        processedPos += curLen;

        len -= curLen;
        if (curLen <= dicBufSize - pos)
        {
          Byte *dest = dic + dicPos;
          ptrdiff_t src = (ptrdiff_t)pos - (ptrdiff_t)dicPos;
          const Byte *lim = dest + curLen;
          dicPos += curLen;
          do
            *(dest) = (Byte)*(dest + src);
          while (++dest != lim);
        }
        else
        {
          do
          {
            dic[dicPos++] = dic[pos];
            if (++pos == dicBufSize)
              pos = 0;
          }
          while (--curLen != 0);
        }
      }
    }
  }
  while (dicPos < limit && buf < bufLimit);

  NORMALIZE;
  
  p->buf = buf;
  p->range = range;
  p->code = code;
  p->remainLen = len;
  p->dicPos = dicPos;
  p->processedPos = processedPos;
  p->reps[0] = rep0;
  p->reps[1] = rep1;
  p->reps[2] = rep2;
  p->reps[3] = rep3;
  p->state = state;

  return SZ_OK;
}

static void MY_FAST_CALL LzmaDec_WriteRem(CLzmaDec *p, SizeT limit)
{
  if (p->remainLen != 0 && p->remainLen < kMatchSpecLenStart)
  {
    Byte *dic = p->dic;
    SizeT dicPos = p->dicPos;
    SizeT dicBufSize = p->dicBufSize;
    unsigned len = p->remainLen;
    SizeT rep0 = p->reps[0]; /* we use SizeT to avoid the BUG of VC14 for AMD64 */
    SizeT rem = limit - dicPos;
    if (rem < len)
      len = (unsigned)(rem);

    if (p->checkDicSize == 0 && p->prop.dicSize - p->processedPos <= len)
      p->checkDicSize = p->prop.dicSize;

    p->processedPos += len;
    p->remainLen -= len;
    while (len != 0)
    {
      len--;
      dic[dicPos] = dic[dicPos - rep0 + (dicPos < rep0 ? dicBufSize : 0)];
      dicPos++;
    }
    p->dicPos = dicPos;
  }
}

static int MY_FAST_CALL LzmaDec_DecodeReal2(CLzmaDec *p, SizeT limit, const Byte *bufLimit)
{
  do
  {
    SizeT limit2 = limit;
    if (p->checkDicSize == 0)
    {
      UInt32 rem = p->prop.dicSize - p->processedPos;
      if (limit - p->dicPos > rem)
        limit2 = p->dicPos + rem;
    }
    
    RINOK(LzmaDec_DecodeReal(p, limit2, bufLimit));
    
    if (p->checkDicSize == 0 && p->processedPos >= p->prop.dicSize)
      p->checkDicSize = p->prop.dicSize;
    
    LzmaDec_WriteRem(p, limit);
  }
  while (p->dicPos < limit && p->buf < bufLimit && p->remainLen < kMatchSpecLenStart);

  if (p->remainLen > kMatchSpecLenStart)
    p->remainLen = kMatchSpecLenStart;

  return 0;
}

typedef enum
{
  DUMMY_ERROR, /* unexpected end of input stream */
  DUMMY_LIT,
  DUMMY_MATCH,
  DUMMY_REP
} ELzmaDummy;

static ELzmaDummy LzmaDec_TryDummy(const CLzmaDec *p, const Byte *buf, SizeT inSize)
{
  UInt32 range = p->range;
  UInt32 code = p->code;
  const Byte *bufLimit = buf + inSize;
  const CLzmaProb *probs = p->probs;
  unsigned state = p->state;
  ELzmaDummy res;

  {
    const CLzmaProb *prob;
    UInt32 bound;
    unsigned ttt;
    unsigned posState = (p->processedPos) & ((1 << p->prop.pb) - 1);

    prob = probs + IsMatch + (state << kNumPosBitsMax) + posState;
    IF_BIT_0_CHECK(prob)
    {
      UPDATE_0_CHECK

      /* if (bufLimit - buf >= 7) return DUMMY_LIT; */

      prob = probs + Literal;
      if (p->checkDicSize != 0 || p->processedPos != 0)
        prob += ((UInt32)LZMA_LIT_SIZE *
            ((((p->processedPos) & ((1 << (p->prop.lp)) - 1)) << p->prop.lc) +
            (p->dic[(p->dicPos == 0 ? p->dicBufSize : p->dicPos) - 1] >> (8 - p->prop.lc))));

      if (state < kNumLitStates)
      {
        unsigned symbol = 1;
        do { GET_BIT_CHECK(prob + symbol, symbol) } while (symbol < 0x100);
      }
      else
      {
        unsigned matchByte = p->dic[p->dicPos - p->reps[0] +
            (p->dicPos < p->reps[0] ? p->dicBufSize : 0)];
        unsigned offs = 0x100;
        unsigned symbol = 1;
        do
        {
          unsigned bit;
          const CLzmaProb *probLit;
          matchByte <<= 1;
          bit = (matchByte & offs);
          probLit = prob + offs + bit + symbol;
          GET_BIT2_CHECK(probLit, symbol, offs &= ~bit, offs &= bit)
        }
        while (symbol < 0x100);
      }
      res = DUMMY_LIT;
    }
    else
    {
      unsigned len;
      UPDATE_1_CHECK;

      prob = probs + IsRep + state;
      IF_BIT_0_CHECK(prob)
      {
        UPDATE_0_CHECK;
        state = 0;
        prob = probs + LenCoder;
        res = DUMMY_MATCH;
      }
      else
      {
        UPDATE_1_CHECK;
        res = DUMMY_REP;
        prob = probs + IsRepG0 + state;
        IF_BIT_0_CHECK(prob)
        {
          UPDATE_0_CHECK;
          prob = probs + IsRep0Long + (state << kNumPosBitsMax) + posState;
          IF_BIT_0_CHECK(prob)
          {
            UPDATE_0_CHECK;
            NORMALIZE_CHECK;
            return DUMMY_REP;
          }
          else
          {
            UPDATE_1_CHECK;
          }
        }
        else
        {
          UPDATE_1_CHECK;
          prob = probs + IsRepG1 + state;
          IF_BIT_0_CHECK(prob)
          {
            UPDATE_0_CHECK;
          }
          else
          {
            UPDATE_1_CHECK;
            prob = probs + IsRepG2 + state;
            IF_BIT_0_CHECK(prob)
            {
              UPDATE_0_CHECK;
            }
            else
            {
              UPDATE_1_CHECK;
            }
          }
        }
        state = kNumStates;
        prob = probs + RepLenCoder;
      }
      {
        unsigned limit, offset;
        const CLzmaProb *probLen = prob + LenChoice;
        IF_BIT_0_CHECK(probLen)
        {
          UPDATE_0_CHECK;
          probLen = prob + LenLow + (posState << kLenNumLowBits);
          offset = 0;
          limit = 1 << kLenNumLowBits;
        }
        else
        {
          UPDATE_1_CHECK;
          probLen = prob + LenChoice2;
          IF_BIT_0_CHECK(probLen)
          {
            UPDATE_0_CHECK;
            probLen = prob + LenMid + (posState << kLenNumMidBits);
            offset = kLenNumLowSymbols;
            limit = 1 << kLenNumMidBits;
          }
          else
          {
            UPDATE_1_CHECK;
            probLen = prob + LenHigh;
            offset = kLenNumLowSymbols + kLenNumMidSymbols;
            limit = 1 << kLenNumHighBits;
          }
        }
        TREE_DECODE_CHECK(probLen, limit, len);
        len += offset;
      }

      if (state < 4)
      {
        unsigned posSlot;
        prob = probs + PosSlot +
            ((len < kNumLenToPosStates ? len : kNumLenToPosStates - 1) <<
            kNumPosSlotBits);
        TREE_DECODE_CHECK(prob, 1 << kNumPosSlotBits, posSlot);
        if (posSlot >= kStartPosModelIndex)
        {
          unsigned numDirectBits = ((posSlot >> 1) - 1);

          /* if (bufLimit - buf >= 8) return DUMMY_MATCH; */

          if (posSlot < kEndPosModelIndex)
          {
            prob = probs + SpecPos + ((2 | (posSlot & 1)) << numDirectBits) - posSlot - 1;
          }
          else
          {
            numDirectBits -= kNumAlignBits;
            do
            {
              NORMALIZE_CHECK
              range >>= 1;
              code -= range & (((code - range) >> 31) - 1);
              /* if (code >= range) code -= range; */
            }
            while (--numDirectBits != 0);
            prob = probs + Align;
            numDirectBits = kNumAlignBits;
          }
          {
            unsigned i = 1;
            do
            {
              GET_BIT_CHECK(prob + i, i);
            }
            while (--numDirectBits != 0);
          }
        }
      }
    }
  }
  NORMALIZE_CHECK;
  return res;
}


static void LzmaDec_InitDicAndState(CLzmaDec *p, Bool initDic, Bool initState)
{
  p->needFlush = 1;
  p->remainLen = 0;
  p->tempBufSize = 0;

  if (initDic)
  {
    p->processedPos = 0;
    p->checkDicSize = 0;
    p->needInitState = 1;
  }
  if (initState)
    p->needInitState = 1;
}

static void LzmaDec_Init(CLzmaDec *p)
{
  p->dicPos = 0;
  LzmaDec_InitDicAndState(p, True, True);
}

static void LzmaDec_InitStateReal(CLzmaDec *p)
{
  SizeT numProbs = LzmaProps_GetNumProbs(&p->prop);
  SizeT i;
  CLzmaProb *probs = p->probs;
  for (i = 0; i < numProbs; i++)
    probs[i] = kBitModelTotal >> 1;
  p->reps[0] = p->reps[1] = p->reps[2] = p->reps[3] = 1;
  p->state = 0;
  p->needInitState = 0;
}

static SRes LzmaDec_DecodeToDic(CLzmaDec *p, SizeT dicLimit, const Byte *src, SizeT *srcLen,
    ELzmaFinishMode finishMode, ELzmaStatus *status)
{
  SizeT inSize = *srcLen;
  (*srcLen) = 0;
  LzmaDec_WriteRem(p, dicLimit);
  
  *status = LZMA_STATUS_NOT_SPECIFIED;

  while (p->remainLen != kMatchSpecLenStart)
  {
      int checkEndMarkNow;

      if (p->needFlush)
      {
        for (; inSize > 0 && p->tempBufSize < RC_INIT_SIZE; (*srcLen)++, inSize--)
          p->tempBuf[p->tempBufSize++] = *src++;
        if (p->tempBufSize < RC_INIT_SIZE)
        {
          *status = LZMA_STATUS_NEEDS_MORE_INPUT;
          return SZ_OK;
        }
        if (p->tempBuf[0] != 0)
          return SZ_ERROR_DATA;
        p->code =
              ((UInt32)p->tempBuf[1] << 24)
            | ((UInt32)p->tempBuf[2] << 16)
            | ((UInt32)p->tempBuf[3] << 8)
            | ((UInt32)p->tempBuf[4]);
        p->range = 0xFFFFFFFF;
        p->needFlush = 0;
        p->tempBufSize = 0;
      }

      checkEndMarkNow = 0;
      if (p->dicPos >= dicLimit)
      {
        if (p->remainLen == 0 && p->code == 0)
        {
          *status = LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK;
          return SZ_OK;
        }
        if (finishMode == LZMA_FINISH_ANY)
        {
          *status = LZMA_STATUS_NOT_FINISHED;
          return SZ_OK;
        }
        if (p->remainLen != 0)
        {
          *status = LZMA_STATUS_NOT_FINISHED;
          return SZ_ERROR_DATA;
        }
        checkEndMarkNow = 1;
      }

      if (p->needInitState)
        LzmaDec_InitStateReal(p);
  
      if (p->tempBufSize == 0)
      {
        SizeT processed;
        const Byte *bufLimit;
        if (inSize < LZMA_REQUIRED_INPUT_MAX || checkEndMarkNow)
        {
          int dummyRes = LzmaDec_TryDummy(p, src, inSize);
          if (dummyRes == DUMMY_ERROR)
          {
            memcpy(p->tempBuf, src, inSize);
            p->tempBufSize = (unsigned)inSize;
            (*srcLen) += inSize;
            *status = LZMA_STATUS_NEEDS_MORE_INPUT;
            return SZ_OK;
          }
          if (checkEndMarkNow && dummyRes != DUMMY_MATCH)
          {
            *status = LZMA_STATUS_NOT_FINISHED;
            return SZ_ERROR_DATA;
          }
          bufLimit = src;
        }
        else
          bufLimit = src + inSize - LZMA_REQUIRED_INPUT_MAX;
        p->buf = src;
        if (LzmaDec_DecodeReal2(p, dicLimit, bufLimit) != 0)
          return SZ_ERROR_DATA;
        processed = (SizeT)(p->buf - src);
        (*srcLen) += processed;
        src += processed;
        inSize -= processed;
      }
      else
      {
        unsigned rem = p->tempBufSize, lookAhead = 0;
        while (rem < LZMA_REQUIRED_INPUT_MAX && lookAhead < inSize)
          p->tempBuf[rem++] = src[lookAhead++];
        p->tempBufSize = rem;
        if (rem < LZMA_REQUIRED_INPUT_MAX || checkEndMarkNow)
        {
          int dummyRes = LzmaDec_TryDummy(p, p->tempBuf, rem);
          if (dummyRes == DUMMY_ERROR)
          {
            (*srcLen) += lookAhead;
            *status = LZMA_STATUS_NEEDS_MORE_INPUT;
            return SZ_OK;
          }
          if (checkEndMarkNow && dummyRes != DUMMY_MATCH)
          {
            *status = LZMA_STATUS_NOT_FINISHED;
            return SZ_ERROR_DATA;
          }
        }
        p->buf = p->tempBuf;
        if (LzmaDec_DecodeReal2(p, dicLimit, p->buf) != 0)
          return SZ_ERROR_DATA;
        
        {
          unsigned kkk = (unsigned)(p->buf - p->tempBuf);
          if (rem < kkk)
            return SZ_ERROR_FAIL; /* some internal error */
          rem -= kkk;
          if (lookAhead < rem)
            return SZ_ERROR_FAIL; /* some internal error */
          lookAhead -= rem;
        }
        (*srcLen) += lookAhead;
        src += lookAhead;
        inSize -= lookAhead;
        p->tempBufSize = 0;
      }
  }
  if (p->code == 0)
    *status = LZMA_STATUS_FINISHED_WITH_MARK;
  return (p->code == 0) ? SZ_OK : SZ_ERROR_DATA;
}

static void LzmaDec_FreeProbs(CLzmaDec *p, ISzAlloc *alloc)
{
  alloc->Free(alloc, p->probs);
  p->probs = NULL;
}

static SRes LzmaProps_Decode(CLzmaProps *p, const Byte *data, unsigned size)
{
  UInt32 dicSize;
  Byte d;
  
  if (size < LZMA_PROPS_SIZE)
    return SZ_ERROR_UNSUPPORTED;
  else
    dicSize = data[1] | ((UInt32)data[2] << 8) | ((UInt32)data[3] << 16) | ((UInt32)data[4] << 24);
 
  if (dicSize < LZMA_DIC_MIN)
    dicSize = LZMA_DIC_MIN;
  p->dicSize = dicSize;

  d = data[0];
  if (d >= (9 * 5 * 5))
    return SZ_ERROR_UNSUPPORTED;

  p->lc = d % 9;
  d /= 9;
  p->pb = d / 5;
  p->lp = d % 5;

  return SZ_OK;
}

static SRes LzmaDec_AllocateProbs2(CLzmaDec *p, const CLzmaProps *propNew, ISzAlloc *alloc)
{
  UInt32 numProbs = LzmaProps_GetNumProbs(propNew);
  if (!p->probs || numProbs != p->numProbs)
  {
    LzmaDec_FreeProbs(p, alloc);
    p->probs = (CLzmaProb *)alloc->Alloc(alloc, numProbs * sizeof(CLzmaProb));
    p->numProbs = numProbs;
    if (!p->probs)
      return SZ_ERROR_MEM;
  }
  return SZ_OK;
}

static SRes LzmaDec_AllocateProbs(CLzmaDec *p, const Byte *props, unsigned propsSize, ISzAlloc *alloc)
{
  CLzmaProps propNew;
  RINOK(LzmaProps_Decode(&propNew, props, propsSize));
  RINOK(LzmaDec_AllocateProbs2(p, &propNew, alloc));
  p->prop = propNew;
  return SZ_OK;
}

/* Lzma2Dec.c -- LZMA2 Decoder
2015-11-09 : Igor Pavlov : Public domain */

/* #define SHOW_DEBUG_INFO */

/*
#include "Precomp.h"

#ifdef SHOW_DEBUG_INFO
#include <stdio.h>
#endif

#include <string.h>

#include "Lzma2Dec.h"
*/

/*
00000000  -  EOS
00000001 U U  -  Uncompressed Reset Dic
00000010 U U  -  Uncompressed No Reset
100uuuuu U U P P  -  LZMA no reset
101uuuuu U U P P  -  LZMA reset state
110uuuuu U U P P S  -  LZMA reset state + new prop
111uuuuu U U P P S  -  LZMA reset state + new prop + reset dic

  u, U - Unpack Size
  P - Pack Size
  S - Props
*/

#define LZMA2_CONTROL_LZMA (1 << 7)
#define LZMA2_CONTROL_COPY_NO_RESET 2
#define LZMA2_CONTROL_COPY_RESET_DIC 1
#define LZMA2_CONTROL_EOF 0

#define LZMA2_IS_UNCOMPRESSED_STATE(p) (((p)->control & LZMA2_CONTROL_LZMA) == 0)

#define LZMA2_GET_LZMA_MODE(p) (((p)->control >> 5) & 3)
#define LZMA2_IS_THERE_PROP(mode) ((mode) >= 2)

#define LZMA2_LCLP_MAX 4
#define LZMA2_DIC_SIZE_FROM_PROP(p) (((UInt32)2 | ((p) & 1)) << ((p) / 2 + 11))

#ifdef SHOW_DEBUG_INFO
#define PRF(x) x
#else
#define PRF(x)
#endif

typedef enum
{
  LZMA2_STATE_CONTROL,
  LZMA2_STATE_UNPACK0,
  LZMA2_STATE_UNPACK1,
  LZMA2_STATE_PACK0,
  LZMA2_STATE_PACK1,
  LZMA2_STATE_PROP,
  LZMA2_STATE_DATA,
  LZMA2_STATE_DATA_CONT,
  LZMA2_STATE_FINISHED,
  LZMA2_STATE_ERROR
} ELzma2State;

static SRes Lzma2Dec_GetOldProps(Byte prop, Byte *props)
{
  UInt32 dicSize;
  if (prop > 40)
    return SZ_ERROR_UNSUPPORTED;
  dicSize = (prop == 40) ? 0xFFFFFFFF : LZMA2_DIC_SIZE_FROM_PROP(prop);
  props[0] = (Byte)LZMA2_LCLP_MAX;
  props[1] = (Byte)(dicSize);
  props[2] = (Byte)(dicSize >> 8);
  props[3] = (Byte)(dicSize >> 16);
  props[4] = (Byte)(dicSize >> 24);
  return SZ_OK;
}

static SRes Lzma2Dec_AllocateProbs(CLzma2Dec *p, Byte prop, ISzAlloc *alloc)
{
  Byte props[LZMA_PROPS_SIZE];
  RINOK(Lzma2Dec_GetOldProps(prop, props));
  return LzmaDec_AllocateProbs(&p->decoder, props, LZMA_PROPS_SIZE, alloc);
}

static void Lzma2Dec_Init(CLzma2Dec *p)
{
  p->state = LZMA2_STATE_CONTROL;
  p->needInitDic = True;
  p->needInitState = True;
  p->needInitProp = True;
  LzmaDec_Init(&p->decoder);
}

static ELzma2State Lzma2Dec_UpdateState(CLzma2Dec *p, Byte b)
{
  switch (p->state)
  {
    case LZMA2_STATE_CONTROL:
      p->control = b;
      PRF(printf("\n %4X ", (unsigned)p->decoder.dicPos));
      PRF(printf(" %2X", (unsigned)b));
      if (p->control == 0)
        return LZMA2_STATE_FINISHED;
      if (LZMA2_IS_UNCOMPRESSED_STATE(p))
      {
        if ((p->control & 0x7F) > 2)
          return LZMA2_STATE_ERROR;
        p->unpackSize = 0;
      }
      else
        p->unpackSize = (UInt32)(p->control & 0x1F) << 16;
      return LZMA2_STATE_UNPACK0;
    
    case LZMA2_STATE_UNPACK0:
      p->unpackSize |= (UInt32)b << 8;
      return LZMA2_STATE_UNPACK1;
    
    case LZMA2_STATE_UNPACK1:
      p->unpackSize |= (UInt32)b;
      p->unpackSize++;
      PRF(printf(" %8u", (unsigned)p->unpackSize));
      return (LZMA2_IS_UNCOMPRESSED_STATE(p)) ? LZMA2_STATE_DATA : LZMA2_STATE_PACK0;
    
    case LZMA2_STATE_PACK0:
      p->packSize = (UInt32)b << 8;
      return LZMA2_STATE_PACK1;

    case LZMA2_STATE_PACK1:
      p->packSize |= (UInt32)b;
      p->packSize++;
      PRF(printf(" %8u", (unsigned)p->packSize));
      return LZMA2_IS_THERE_PROP(LZMA2_GET_LZMA_MODE(p)) ? LZMA2_STATE_PROP:
        (p->needInitProp ? LZMA2_STATE_ERROR : LZMA2_STATE_DATA);

    case LZMA2_STATE_PROP:
    {
      unsigned lc, lp;
      if (b >= (9 * 5 * 5))
        return LZMA2_STATE_ERROR;
      lc = b % 9;
      b /= 9;
      p->decoder.prop.pb = b / 5;
      lp = b % 5;
      if (lc + lp > LZMA2_LCLP_MAX)
        return LZMA2_STATE_ERROR;
      p->decoder.prop.lc = lc;
      p->decoder.prop.lp = lp;
      p->needInitProp = False;
      return LZMA2_STATE_DATA;
    }
  }
  return LZMA2_STATE_ERROR;
}

static void LzmaDec_UpdateWithUncompressed(CLzmaDec *p, const Byte *src, SizeT size)
{
  memcpy(p->dic + p->dicPos, src, size);
  p->dicPos += size;
  if (p->checkDicSize == 0 && p->prop.dicSize - p->processedPos <= size)
    p->checkDicSize = p->prop.dicSize;
  p->processedPos += (UInt32)size;
}

static void LzmaDec_InitDicAndState(CLzmaDec *p, Bool initDic, Bool initState);

static SRes Lzma2Dec_DecodeToDic(CLzma2Dec *p, SizeT dicLimit,
    const Byte *src, SizeT *srcLen, ELzmaFinishMode finishMode, ELzmaStatus *status)
{
  SizeT inSize = *srcLen;
  *srcLen = 0;
  *status = LZMA_STATUS_NOT_SPECIFIED;

  while (p->state != LZMA2_STATE_FINISHED)
  {
    SizeT dicPos = p->decoder.dicPos;
    
    if (p->state == LZMA2_STATE_ERROR)
      return SZ_ERROR_DATA;
    
    if (dicPos == dicLimit && finishMode == LZMA_FINISH_ANY)
    {
      *status = LZMA_STATUS_NOT_FINISHED;
      return SZ_OK;
    }

    if (p->state != LZMA2_STATE_DATA && p->state != LZMA2_STATE_DATA_CONT)
    {
      if (*srcLen == inSize)
      {
        *status = LZMA_STATUS_NEEDS_MORE_INPUT;
        return SZ_OK;
      }
      (*srcLen)++;
      p->state = Lzma2Dec_UpdateState(p, *src++);

      if (dicPos == dicLimit && p->state != LZMA2_STATE_FINISHED)
      {
        p->state = LZMA2_STATE_ERROR;
        return SZ_ERROR_DATA;
      }
      continue;
    }
    
    {
      SizeT destSizeCur = dicLimit - dicPos;
      SizeT srcSizeCur = inSize - *srcLen;
      ELzmaFinishMode curFinishMode = LZMA_FINISH_ANY;
      
      if (p->unpackSize <= destSizeCur)
      {
        destSizeCur = (SizeT)p->unpackSize;
        curFinishMode = LZMA_FINISH_END;
      }

      if (LZMA2_IS_UNCOMPRESSED_STATE(p))
      {
        if (*srcLen == inSize)
        {
          *status = LZMA_STATUS_NEEDS_MORE_INPUT;
          return SZ_OK;
        }

        if (p->state == LZMA2_STATE_DATA)
        {
          Bool initDic = (p->control == LZMA2_CONTROL_COPY_RESET_DIC);
          if (initDic)
            p->needInitProp = p->needInitState = True;
          else if (p->needInitDic)
          {
            p->state = LZMA2_STATE_ERROR;
            return SZ_ERROR_DATA;
          }
          p->needInitDic = False;
          LzmaDec_InitDicAndState(&p->decoder, initDic, False);
        }

        if (srcSizeCur > destSizeCur)
          srcSizeCur = destSizeCur;

        if (srcSizeCur == 0)
        {
          p->state = LZMA2_STATE_ERROR;
          return SZ_ERROR_DATA;
        }

        LzmaDec_UpdateWithUncompressed(&p->decoder, src, srcSizeCur);

        src += srcSizeCur;
        *srcLen += srcSizeCur;
        p->unpackSize -= (UInt32)srcSizeCur;
        p->state = (p->unpackSize == 0) ? LZMA2_STATE_CONTROL : LZMA2_STATE_DATA_CONT;
      }
      else
      {
        SizeT outSizeProcessed;
        SRes res;

        if (p->state == LZMA2_STATE_DATA)
        {
          unsigned mode = LZMA2_GET_LZMA_MODE(p);
          Bool initDic = (mode == 3);
          Bool initState = (mode != 0);
          if ((!initDic && p->needInitDic) || (!initState && p->needInitState))
          {
            p->state = LZMA2_STATE_ERROR;
            return SZ_ERROR_DATA;
          }
          
          LzmaDec_InitDicAndState(&p->decoder, initDic, initState);
          p->needInitDic = False;
          p->needInitState = False;
          p->state = LZMA2_STATE_DATA_CONT;
        }
  
        if (srcSizeCur > p->packSize)
          srcSizeCur = (SizeT)p->packSize;
          
        res = LzmaDec_DecodeToDic(&p->decoder, dicPos + destSizeCur, src, &srcSizeCur, curFinishMode, status);
        
        src += srcSizeCur;
        *srcLen += srcSizeCur;
        p->packSize -= (UInt32)srcSizeCur;

        outSizeProcessed = p->decoder.dicPos - dicPos;
        p->unpackSize -= (UInt32)outSizeProcessed;

        RINOK(res);
        if (*status == LZMA_STATUS_NEEDS_MORE_INPUT)
          return res;

        if (srcSizeCur == 0 && outSizeProcessed == 0)
        {
          if (*status != LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK
              || p->unpackSize != 0
              || p->packSize != 0)
          {
            p->state = LZMA2_STATE_ERROR;
            return SZ_ERROR_DATA;
          }
          p->state = LZMA2_STATE_CONTROL;
        }
        
        if (*status == LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK)
          *status = LZMA_STATUS_NOT_FINISHED;
      }
    }
  }
  
  *status = LZMA_STATUS_FINISHED_WITH_MARK;
  return SZ_OK;
}

#endif  /* _INCLUDE_PHYSFS_LZMASDK_H_ */

/* end of physfs_lzmasdk.h ... */

