/**
 * PhysicsFS; a portable, flexible file i/o abstraction.
 *
 * Documentation is in physfs.h. It's verbose, honest.  :)
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

#ifndef PHYSFS_Swap16
static inline PHYSFS_uint16 PHYSFS_Swap16(PHYSFS_uint16 D)
{
    return ((D<<8)|(D>>8));
}
#endif
#ifndef PHYSFS_Swap32
static inline PHYSFS_uint32 PHYSFS_Swap32(PHYSFS_uint32 D)
{
    return ((D<<24)|((D<<8)&0x00FF0000)|((D>>8)&0x0000FF00)|(D>>24));
}
#endif
#ifndef PHYSFS_NO_64BIT_SUPPORT
#ifndef PHYSFS_Swap64
static inline PHYSFS_uint64 PHYSFS_Swap64(PHYSFS_uint64 val) {
    PHYSFS_uint32 hi, lo;

    /* Separate into high and low 32-bit values and swap them */
    lo = (PHYSFS_uint32)(val&0xFFFFFFFF);
    val >>= 32;
    hi = (PHYSFS_uint32)(val&0xFFFFFFFF);
    val = PHYSFS_Swap32(lo);
    val <<= 32;
    val |= PHYSFS_Swap32(hi);
    return val;
}
#endif
#else
#ifndef PHYSFS_Swap64
/* This is mainly to keep compilers from complaining in PHYSFS code.
   If there is no real 64-bit datatype, then compilers will complain about
   the fake 64-bit datatype that PHYSFS provides when it compiles user code.
*/
#define PHYSFS_Swap64(X)    (X)
#endif
#endif /* PHYSFS_NO_64BIT_SUPPORT */


/* Byteswap item from the specified endianness to the native endianness */
#if PHYSFS_BYTEORDER == PHYSFS_LIL_ENDIAN
PHYSFS_uint16 PHYSFS_swapULE16(PHYSFS_uint16 x) { return x; }
PHYSFS_sint16 PHYSFS_swapSLE16(PHYSFS_sint16 x) { return x; }
PHYSFS_uint32 PHYSFS_swapULE32(PHYSFS_uint32 x) { return x; }
PHYSFS_sint32 PHYSFS_swapSLE32(PHYSFS_sint32 x) { return x; }
PHYSFS_uint64 PHYSFS_swapULE64(PHYSFS_uint64 x) { return x; }
PHYSFS_sint64 PHYSFS_swapSLE64(PHYSFS_sint64 x) { return x; }

PHYSFS_uint16 PHYSFS_swapUBE16(PHYSFS_uint16 x) { return PHYSFS_Swap16(x); }
PHYSFS_sint16 PHYSFS_swapSBE16(PHYSFS_sint16 x) { return PHYSFS_Swap16(x); }
PHYSFS_uint32 PHYSFS_swapUBE32(PHYSFS_uint32 x) { return PHYSFS_Swap32(x); }
PHYSFS_sint32 PHYSFS_swapSBE32(PHYSFS_sint32 x) { return PHYSFS_Swap32(x); }
PHYSFS_uint64 PHYSFS_swapUBE64(PHYSFS_uint64 x) { return PHYSFS_Swap64(x); }
PHYSFS_sint64 PHYSFS_swapSBE64(PHYSFS_sint64 x) { return PHYSFS_Swap64(x); }
#else
PHYSFS_uint16 PHYSFS_swapULE16(PHYSFS_uint16 x) { return PHYSFS_Swap16(x); }
PHYSFS_sint16 PHYSFS_swapSLE16(PHYSFS_sint16 x) { return PHYSFS_Swap16(x); }
PHYSFS_uint32 PHYSFS_swapULE32(PHYSFS_uint32 x) { return PHYSFS_Swap32(x); }
PHYSFS_sint32 PHYSFS_swapSLE32(PHYSFS_sint32 x) { return PHYSFS_Swap32(x); }
PHYSFS_uint64 PHYSFS_swapULE64(PHYSFS_uint64 x) { return PHYSFS_Swap64(x); }
PHYSFS_sint64 PHYSFS_swapSLE64(PHYSFS_sint64 x) { return PHYSFS_Swap64(x); }

PHYSFS_uint16 PHYSFS_swapUBE16(PHYSFS_uint16 x) { return x; }
PHYSFS_sint16 PHYSFS_swapSBE16(PHYSFS_sint16 x) { return x; }
PHYSFS_uint32 PHYSFS_swapUBE32(PHYSFS_uint32 x) { return x; }
PHYSFS_sint32 PHYSFS_swapSBE32(PHYSFS_sint32 x) { return x; }
PHYSFS_uint64 PHYSFS_swapUBE64(PHYSFS_uint64 x) { return x; }
PHYSFS_sint64 PHYSFS_swapSBE64(PHYSFS_sint64 x) { return x; }
#endif

static inline int readAll(PHYSFS_File *file, void *val, const size_t len)
{
    return (PHYSFS_readBytes(file, val, len) == len);
} /* readAll */

#define PHYSFS_BYTEORDER_READ(datatype, swaptype) \
    int PHYSFS_read##swaptype(PHYSFS_File *file, PHYSFS_##datatype *val) { \
        PHYSFS_##datatype in; \
        BAIL_IF(val == NULL, PHYSFS_ERR_INVALID_ARGUMENT, 0); \
        BAIL_IF_ERRPASS(!readAll(file, &in, sizeof (in)), 0); \
        *val = PHYSFS_swap##swaptype(in); \
        return 1; \
    }

PHYSFS_BYTEORDER_READ(sint16, SLE16)
PHYSFS_BYTEORDER_READ(uint16, ULE16)
PHYSFS_BYTEORDER_READ(sint16, SBE16)
PHYSFS_BYTEORDER_READ(uint16, UBE16)
PHYSFS_BYTEORDER_READ(sint32, SLE32)
PHYSFS_BYTEORDER_READ(uint32, ULE32)
PHYSFS_BYTEORDER_READ(sint32, SBE32)
PHYSFS_BYTEORDER_READ(uint32, UBE32)
PHYSFS_BYTEORDER_READ(sint64, SLE64)
PHYSFS_BYTEORDER_READ(uint64, ULE64)
PHYSFS_BYTEORDER_READ(sint64, SBE64)
PHYSFS_BYTEORDER_READ(uint64, UBE64)


static inline int writeAll(PHYSFS_File *f, const void *val, const size_t len)
{
    return (PHYSFS_writeBytes(f, val, len) == len);
} /* writeAll */

#define PHYSFS_BYTEORDER_WRITE(datatype, swaptype) \
    int PHYSFS_write##swaptype(PHYSFS_File *file, PHYSFS_##datatype val) { \
        const PHYSFS_##datatype out = PHYSFS_swap##swaptype(val); \
        BAIL_IF_ERRPASS(!writeAll(file, &out, sizeof (out)), 0); \
        return 1; \
    }

PHYSFS_BYTEORDER_WRITE(sint16, SLE16)
PHYSFS_BYTEORDER_WRITE(uint16, ULE16)
PHYSFS_BYTEORDER_WRITE(sint16, SBE16)
PHYSFS_BYTEORDER_WRITE(uint16, UBE16)
PHYSFS_BYTEORDER_WRITE(sint32, SLE32)
PHYSFS_BYTEORDER_WRITE(uint32, ULE32)
PHYSFS_BYTEORDER_WRITE(sint32, SBE32)
PHYSFS_BYTEORDER_WRITE(uint32, UBE32)
PHYSFS_BYTEORDER_WRITE(sint64, SLE64)
PHYSFS_BYTEORDER_WRITE(uint64, ULE64)
PHYSFS_BYTEORDER_WRITE(sint64, SBE64)
PHYSFS_BYTEORDER_WRITE(uint64, UBE64)

/* end of physfs_byteorder.c ... */

