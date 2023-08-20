/*
 * ZIP support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon, with some peeking at "unzip.c"
 *   by Gilles Vollant.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

#if PHYSFS_SUPPORTS_ZIP

#include <errno.h>
#include <time.h>

#if (PHYSFS_BYTEORDER == PHYSFS_LIL_ENDIAN)
#define MINIZ_LITTLE_ENDIAN 1
#else
#define MINIZ_LITTLE_ENDIAN 0
#endif
#include "physfs_miniz.h"

/*
 * A buffer of ZIP_READBUFSIZE is allocated for each compressed file opened,
 *  and is freed when you close the file; compressed data is read into
 *  this buffer, and then is decompressed into the buffer passed to
 *  PHYSFS_read().
 *
 * Uncompressed entries in a zipfile do not allocate this buffer; they just
 *  read data directly into the buffer passed to PHYSFS_read().
 *
 * Depending on your speed and memory requirements, you should tweak this
 *  value.
 */
#define ZIP_READBUFSIZE   (16 * 1024)


/*
 * Entries are "unresolved" until they are first opened. At that time,
 *  local file headers parsed/validated, data offsets will be updated to look
 *  at the actual file data instead of the header, and symlinks will be
 *  followed and optimized. This means that we don't seek and read around the
 *  archive until forced to do so, and after the first time, we had to do
 *  less reading and parsing, which is very CD-ROM friendly.
 */
typedef enum
{
    ZIP_UNRESOLVED_FILE,
    ZIP_UNRESOLVED_SYMLINK,
    ZIP_RESOLVING,
    ZIP_RESOLVED,
    ZIP_DIRECTORY,
    ZIP_BROKEN_FILE,
    ZIP_BROKEN_SYMLINK
} ZipResolveType;


/*
 * One ZIPentry is kept for each file in an open ZIP archive.
 */
typedef struct _ZIPentry
{
    __PHYSFS_DirTreeEntry tree;         /* manages directory tree         */
    struct _ZIPentry *symlink;          /* NULL or file we symlink to     */
    ZipResolveType resolved;            /* Have we resolved file/symlink? */
    PHYSFS_uint64 offset;               /* offset of data in archive      */
    PHYSFS_uint16 version;              /* version made by                */
    PHYSFS_uint16 version_needed;       /* version needed to extract      */
    PHYSFS_uint16 general_bits;         /* general purpose bits           */
    PHYSFS_uint16 compression_method;   /* compression method             */
    PHYSFS_uint32 crc;                  /* crc-32                         */
    PHYSFS_uint64 compressed_size;      /* compressed size                */
    PHYSFS_uint64 uncompressed_size;    /* uncompressed size              */
    PHYSFS_sint64 last_mod_time;        /* last file mod time             */
    PHYSFS_uint32 dos_mod_time;         /* original MS-DOS style mod time */
} ZIPentry;

/*
 * One ZIPinfo is kept for each open ZIP archive.
 */
typedef struct
{
    __PHYSFS_DirTree tree;    /* manages directory tree.                */
    PHYSFS_Io *io;            /* the i/o interface for this archive.    */
    int zip64;                /* non-zero if this is a Zip64 archive.   */
    int has_crypto;           /* non-zero if any entry uses encryption. */
} ZIPinfo;

/*
 * One ZIPfileinfo is kept for each open file in a ZIP archive.
 */
typedef struct
{
    ZIPentry *entry;                      /* Info on file.              */
    PHYSFS_Io *io;                        /* physical file handle.      */
    PHYSFS_uint32 compressed_position;    /* offset in compressed data. */
    PHYSFS_uint32 uncompressed_position;  /* tell() position.           */
    PHYSFS_uint8 *buffer;                 /* decompression buffer.      */
    PHYSFS_uint32 crypto_keys[3];         /* for "traditional" crypto.  */
    PHYSFS_uint32 initial_crypto_keys[3]; /* for "traditional" crypto.  */
    z_stream stream;                      /* zlib stream state.         */
} ZIPfileinfo;


/* Magic numbers... */
#define ZIP_LOCAL_FILE_SIG                          0x04034b50
#define ZIP_CENTRAL_DIR_SIG                         0x02014b50
#define ZIP_END_OF_CENTRAL_DIR_SIG                  0x06054b50
#define ZIP64_END_OF_CENTRAL_DIR_SIG                0x06064b50
#define ZIP64_END_OF_CENTRAL_DIRECTORY_LOCATOR_SIG  0x07064b50
#define ZIP64_EXTENDED_INFO_EXTRA_FIELD_SIG         0x0001

/* compression methods... */
#define COMPMETH_NONE 0
/* ...and others... */


#define UNIX_FILETYPE_MASK    0170000
#define UNIX_FILETYPE_SYMLINK 0120000

#define ZIP_GENERAL_BITS_TRADITIONAL_CRYPTO   (1 << 0)
#define ZIP_GENERAL_BITS_IGNORE_LOCAL_HEADER  (1 << 3)

/* support for "traditional" PKWARE encryption. */
static int zip_entry_is_tradional_crypto(const ZIPentry *entry)
{
    return (entry->general_bits & ZIP_GENERAL_BITS_TRADITIONAL_CRYPTO) != 0;
} /* zip_entry_is_traditional_crypto */

static int zip_entry_ignore_local_header(const ZIPentry *entry)
{
    return (entry->general_bits & ZIP_GENERAL_BITS_IGNORE_LOCAL_HEADER) != 0;
} /* zip_entry_is_traditional_crypto */

static PHYSFS_uint32 zip_crypto_crc32(const PHYSFS_uint32 crc, const PHYSFS_uint8 val)
{
    int i;
    PHYSFS_uint32 xorval = (crc ^ ((PHYSFS_uint32) val)) & 0xFF;
    for (i = 0; i < 8; i++)
        xorval = ((xorval & 1) ? (0xEDB88320 ^ (xorval >> 1)) : (xorval >> 1));
    return xorval ^ (crc >> 8);
} /* zip_crc32 */

static void zip_update_crypto_keys(PHYSFS_uint32 *keys, const PHYSFS_uint8 val)
{
    keys[0] = zip_crypto_crc32(keys[0], val);
    keys[1] = keys[1] + (keys[0] & 0x000000FF);
    keys[1] = (keys[1] * 134775813) + 1;
    keys[2] = zip_crypto_crc32(keys[2], (PHYSFS_uint8) ((keys[1] >> 24) & 0xFF));
} /* zip_update_crypto_keys */

static PHYSFS_uint8 zip_decrypt_byte(const PHYSFS_uint32 *keys)
{
    const PHYSFS_uint16 tmp = keys[2] | 2;
    return (PHYSFS_uint8) ((tmp * (tmp ^ 1)) >> 8);
} /* zip_decrypt_byte */

static PHYSFS_sint64 zip_read_decrypt(ZIPfileinfo *finfo, void *buf, PHYSFS_uint64 len)
{
    PHYSFS_Io *io = finfo->io;
    const PHYSFS_sint64 br = io->read(io, buf, len);

    /* Decompression the new data if necessary. */
    if (zip_entry_is_tradional_crypto(finfo->entry) && (br > 0))
    {
        PHYSFS_uint32 *keys = finfo->crypto_keys;
        PHYSFS_uint8 *ptr = (PHYSFS_uint8 *) buf;
        PHYSFS_sint64 i;
        for (i = 0; i < br; i++, ptr++)
        {
            const PHYSFS_uint8 ch = *ptr ^ zip_decrypt_byte(keys);
            zip_update_crypto_keys(keys, ch);
            *ptr = ch;
        } /* for */
    } /* if  */

    return br;
} /* zip_read_decrypt */

static int zip_prep_crypto_keys(ZIPfileinfo *finfo, const PHYSFS_uint8 *crypto_header, const PHYSFS_uint8 *password)
{
    /* It doesn't appear to be documented in PKWare's APPNOTE.TXT, but you
       need to use a different byte in the header to verify the password
       if general purpose bit 3 is set. Discovered this from Info-Zip.
       That's what the (verifier) value is doing, below. */

    PHYSFS_uint32 *keys = finfo->crypto_keys;
    const ZIPentry *entry = finfo->entry;
    const int usedate = zip_entry_ignore_local_header(entry);
    const PHYSFS_uint8 verifier = (PHYSFS_uint8) ((usedate ? (entry->dos_mod_time >> 8) : (entry->crc >> 24)) & 0xFF);
    PHYSFS_uint8 finalbyte = 0;
    int i = 0;

    /* initialize vector with defaults, then password, then header. */
    keys[0] = 305419896;
    keys[1] = 591751049;
    keys[2] = 878082192;

    while (*password)
        zip_update_crypto_keys(keys, *(password++));

    for (i = 0; i < 12; i++)
    {
        const PHYSFS_uint8 c = crypto_header[i] ^ zip_decrypt_byte(keys);
        zip_update_crypto_keys(keys, c);
        finalbyte = c;
    } /* for */

    /* you have a 1/256 chance of passing this test incorrectly. :/ */
    if (finalbyte != verifier)
        BAIL(PHYSFS_ERR_BAD_PASSWORD, 0);

    /* save the initial vector for seeking purposes. Not secure!! */
    memcpy(finfo->initial_crypto_keys, finfo->crypto_keys, 12);
    return 1;
} /* zip_prep_crypto_keys */


/*
 * Bridge physfs allocation functions to zlib's format...
 */
static voidpf zlibPhysfsAlloc(voidpf opaque, uInt items, uInt size)
{
    return ((PHYSFS_Allocator *) opaque)->Malloc(items * size);
} /* zlibPhysfsAlloc */

/*
 * Bridge physfs allocation functions to zlib's format...
 */
static void zlibPhysfsFree(voidpf opaque, voidpf address)
{
    ((PHYSFS_Allocator *) opaque)->Free(address);
} /* zlibPhysfsFree */


/*
 * Construct a new z_stream to a sane state.
 */
static void initializeZStream(z_stream *pstr)
{
    memset(pstr, '\0', sizeof (z_stream));
    pstr->zalloc = zlibPhysfsAlloc;
    pstr->zfree = zlibPhysfsFree;
    pstr->opaque = &allocator;
} /* initializeZStream */


static PHYSFS_ErrorCode zlib_error_code(int rc)
{
    switch (rc)
    {
        case Z_OK: return PHYSFS_ERR_OK;  /* not an error. */
        case Z_STREAM_END: return PHYSFS_ERR_OK; /* not an error. */
        case Z_ERRNO: return PHYSFS_ERR_IO;
        case Z_MEM_ERROR: return PHYSFS_ERR_OUT_OF_MEMORY;
        default: return PHYSFS_ERR_CORRUPT;
    } /* switch */
} /* zlib_error_string */


/*
 * Wrap all zlib calls in this, so the physfs error state is set appropriately.
 */
static int zlib_err(const int rc)
{
    PHYSFS_setErrorCode(zlib_error_code(rc));
    return rc;
} /* zlib_err */

/*
 * Read an unsigned 64-bit int and swap to native byte order.
 */
static int readui64(PHYSFS_Io *io, PHYSFS_uint64 *val)
{
    PHYSFS_uint64 v;
    BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &v, sizeof (v)), 0);
    *val = PHYSFS_swapULE64(v);
    return 1;
} /* readui64 */

/*
 * Read an unsigned 32-bit int and swap to native byte order.
 */
static int readui32(PHYSFS_Io *io, PHYSFS_uint32 *val)
{
    PHYSFS_uint32 v;
    BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &v, sizeof (v)), 0);
    *val = PHYSFS_swapULE32(v);
    return 1;
} /* readui32 */


/*
 * Read an unsigned 16-bit int and swap to native byte order.
 */
static int readui16(PHYSFS_Io *io, PHYSFS_uint16 *val)
{
    PHYSFS_uint16 v;
    BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &v, sizeof (v)), 0);
    *val = PHYSFS_swapULE16(v);
    return 1;
} /* readui16 */


static PHYSFS_sint64 ZIP_read(PHYSFS_Io *_io, void *buf, PHYSFS_uint64 len)
{
    ZIPfileinfo *finfo = (ZIPfileinfo *) _io->opaque;
    ZIPentry *entry = finfo->entry;
    PHYSFS_sint64 retval = 0;
    PHYSFS_sint64 maxread = (PHYSFS_sint64) len;
    PHYSFS_sint64 avail = entry->uncompressed_size -
                          finfo->uncompressed_position;

    if (avail < maxread)
        maxread = avail;

    BAIL_IF_ERRPASS(maxread == 0, 0);    /* quick rejection. */

    if (entry->compression_method == COMPMETH_NONE)
        retval = zip_read_decrypt(finfo, buf, maxread);
    else
    {
        finfo->stream.next_out = buf;
        finfo->stream.avail_out = (uInt) maxread;

        while (retval < maxread)
        {
            const PHYSFS_uint32 before = (PHYSFS_uint32) finfo->stream.total_out;
            int rc;

            if (finfo->stream.avail_in == 0)
            {
                PHYSFS_sint64 br;

                br = entry->compressed_size - finfo->compressed_position;
                if (br > 0)
                {
                    if (br > ZIP_READBUFSIZE)
                        br = ZIP_READBUFSIZE;

                    br = zip_read_decrypt(finfo, finfo->buffer, (PHYSFS_uint64) br);
                    if (br <= 0)
                        break;

                    finfo->compressed_position += (PHYSFS_uint32) br;
                    finfo->stream.next_in = finfo->buffer;
                    finfo->stream.avail_in = (unsigned int) br;
                } /* if */
            } /* if */

            rc = zlib_err(inflate(&finfo->stream, Z_SYNC_FLUSH));
            retval += (finfo->stream.total_out - before);

            if (rc != Z_OK)
                break;
        } /* while */
    } /* else */

    if (retval > 0)
        finfo->uncompressed_position += (PHYSFS_uint32) retval;

    return retval;
} /* ZIP_read */


static PHYSFS_sint64 ZIP_write(PHYSFS_Io *io, const void *b, PHYSFS_uint64 len)
{
    BAIL(PHYSFS_ERR_READ_ONLY, -1);
} /* ZIP_write */


static PHYSFS_sint64 ZIP_tell(PHYSFS_Io *io)
{
    return ((ZIPfileinfo *) io->opaque)->uncompressed_position;
} /* ZIP_tell */


static int ZIP_seek(PHYSFS_Io *_io, PHYSFS_uint64 offset)
{
    ZIPfileinfo *finfo = (ZIPfileinfo *) _io->opaque;
    ZIPentry *entry = finfo->entry;
    PHYSFS_Io *io = finfo->io;
    const int encrypted = zip_entry_is_tradional_crypto(entry);

    BAIL_IF(offset > entry->uncompressed_size, PHYSFS_ERR_PAST_EOF, 0);

    if (!encrypted && (entry->compression_method == COMPMETH_NONE))
    {
        PHYSFS_sint64 newpos = offset + entry->offset;
        BAIL_IF_ERRPASS(!io->seek(io, newpos), 0);
        finfo->uncompressed_position = (PHYSFS_uint32) offset;
    } /* if */

    else
    {
        /*
         * If seeking backwards, we need to redecode the file
         *  from the start and throw away the compressed bits until we hit
         *  the offset we need. If seeking forward, we still need to
         *  decode, but we don't rewind first.
         */
        if (offset < finfo->uncompressed_position)
        {
            /* we do a copy so state is sane if inflateInit2() fails. */
            z_stream str;
            initializeZStream(&str);
            if (zlib_err(inflateInit2(&str, -MAX_WBITS)) != Z_OK)
                return 0;

            if (!io->seek(io, entry->offset + (encrypted ? 12 : 0)))
                return 0;

            inflateEnd(&finfo->stream);
            memcpy(&finfo->stream, &str, sizeof (z_stream));
            finfo->uncompressed_position = finfo->compressed_position = 0;

            if (encrypted)
                memcpy(finfo->crypto_keys, finfo->initial_crypto_keys, 12);
        } /* if */

        while (finfo->uncompressed_position != offset)
        {
            PHYSFS_uint8 buf[512];
            PHYSFS_uint32 maxread;

            maxread = (PHYSFS_uint32) (offset - finfo->uncompressed_position);
            if (maxread > sizeof (buf))
                maxread = sizeof (buf);

            if (ZIP_read(_io, buf, maxread) != maxread)
                return 0;
        } /* while */
    } /* else */

    return 1;
} /* ZIP_seek */


static PHYSFS_sint64 ZIP_length(PHYSFS_Io *io)
{
    const ZIPfileinfo *finfo = (ZIPfileinfo *) io->opaque;
    return (PHYSFS_sint64) finfo->entry->uncompressed_size;
} /* ZIP_length */


static PHYSFS_Io *zip_get_io(PHYSFS_Io *io, ZIPinfo *inf, ZIPentry *entry);

static PHYSFS_Io *ZIP_duplicate(PHYSFS_Io *io)
{
    ZIPfileinfo *origfinfo = (ZIPfileinfo *) io->opaque;
    PHYSFS_Io *retval = (PHYSFS_Io *) allocator.Malloc(sizeof (PHYSFS_Io));
    ZIPfileinfo *finfo = (ZIPfileinfo *) allocator.Malloc(sizeof (ZIPfileinfo));
    GOTO_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, failed);
    GOTO_IF(!finfo, PHYSFS_ERR_OUT_OF_MEMORY, failed);
    memset(finfo, '\0', sizeof (*finfo));

    finfo->entry = origfinfo->entry;
    finfo->io = zip_get_io(origfinfo->io, NULL, finfo->entry);
    GOTO_IF_ERRPASS(!finfo->io, failed);

    initializeZStream(&finfo->stream);
    if (finfo->entry->compression_method != COMPMETH_NONE)
    {
        finfo->buffer = (PHYSFS_uint8 *) allocator.Malloc(ZIP_READBUFSIZE);
        GOTO_IF(!finfo->buffer, PHYSFS_ERR_OUT_OF_MEMORY, failed);
        if (zlib_err(inflateInit2(&finfo->stream, -MAX_WBITS)) != Z_OK)
            goto failed;
    } /* if */

    memcpy(retval, io, sizeof (PHYSFS_Io));
    retval->opaque = finfo;
    return retval;

failed:
    if (finfo != NULL)
    {
        if (finfo->io != NULL)
            finfo->io->destroy(finfo->io);

        if (finfo->buffer != NULL)
        {
            allocator.Free(finfo->buffer);
            inflateEnd(&finfo->stream);
        } /* if */

        allocator.Free(finfo);
    } /* if */

    if (retval != NULL)
        allocator.Free(retval);

    return NULL;
} /* ZIP_duplicate */

static int ZIP_flush(PHYSFS_Io *io) { return 1;  /* no write support. */ }

static void ZIP_destroy(PHYSFS_Io *io)
{
    ZIPfileinfo *finfo = (ZIPfileinfo *) io->opaque;
    finfo->io->destroy(finfo->io);

    if (finfo->entry->compression_method != COMPMETH_NONE)
        inflateEnd(&finfo->stream);

    if (finfo->buffer != NULL)
        allocator.Free(finfo->buffer);

    allocator.Free(finfo);
    allocator.Free(io);
} /* ZIP_destroy */


static const PHYSFS_Io ZIP_Io =
{
    CURRENT_PHYSFS_IO_API_VERSION, NULL,
    ZIP_read,
    ZIP_write,
    ZIP_seek,
    ZIP_tell,
    ZIP_length,
    ZIP_duplicate,
    ZIP_flush,
    ZIP_destroy
};



static PHYSFS_sint64 zip_find_end_of_central_dir(PHYSFS_Io *io, PHYSFS_sint64 *len)
{
    PHYSFS_uint8 buf[256];
    PHYSFS_uint8 extra[4] = { 0, 0, 0, 0 };
    PHYSFS_sint32 i = 0;
    PHYSFS_sint64 filelen;
    PHYSFS_sint64 filepos;
    PHYSFS_sint32 maxread;
    PHYSFS_sint32 totalread = 0;
    int found = 0;

    filelen = io->length(io);
    BAIL_IF_ERRPASS(filelen == -1, -1);

    /*
     * Jump to the end of the file and start reading backwards.
     *  The last thing in the file is the zipfile comment, which is variable
     *  length, and the field that specifies its size is before it in the
     *  file (argh!)...this means that we need to scan backwards until we
     *  hit the end-of-central-dir signature. We can then sanity check that
     *  the comment was as big as it should be to make sure we're in the
     *  right place. The comment length field is 16 bits, so we can stop
     *  searching for that signature after a little more than 64k at most,
     *  and call it a corrupted zipfile.
     */

    if (sizeof (buf) < filelen)
    {
        filepos = filelen - sizeof (buf);
        maxread = sizeof (buf);
    } /* if */
    else
    {
        filepos = 0;
        maxread = (PHYSFS_uint32) filelen;
    } /* else */

    while ((totalread < filelen) && (totalread < 65557))
    {
        BAIL_IF_ERRPASS(!io->seek(io, filepos), -1);

        /* make sure we catch a signature between buffers. */
        if (totalread != 0)
        {
            if (!__PHYSFS_readAll(io, buf, maxread - 4))
                return -1;
            memcpy(&buf[maxread - 4], extra, sizeof (extra));
            totalread += maxread - 4;
        } /* if */
        else
        {
            if (!__PHYSFS_readAll(io, buf, maxread))
                return -1;
            totalread += maxread;
        } /* else */

        memcpy(extra, buf, sizeof (extra));

        for (i = maxread - 4; i > 0; i--)
        {
            if ((buf[i + 0] == 0x50) &&
                (buf[i + 1] == 0x4B) &&
                (buf[i + 2] == 0x05) &&
                (buf[i + 3] == 0x06) )
            {
                found = 1;  /* that's the signature! */
                break;  
            } /* if */
        } /* for */

        if (found)
            break;

        filepos -= (maxread - 4);
        if (filepos < 0)
            filepos = 0;
    } /* while */

    BAIL_IF(!found, PHYSFS_ERR_UNSUPPORTED, -1);

    if (len != NULL)
        *len = filelen;

    return (filepos + i);
} /* zip_find_end_of_central_dir */


static int isZip(PHYSFS_Io *io)
{
    PHYSFS_uint32 sig = 0;
    int retval = 0;

    /*
     * The first thing in a zip file might be the signature of the
     *  first local file record, so it makes for a quick determination.
     */
    if (readui32(io, &sig))
    {
        retval = (sig == ZIP_LOCAL_FILE_SIG);
        if (!retval)
        {
            /*
             * No sig...might be a ZIP with data at the start
             *  (a self-extracting executable, etc), so we'll have to do
             *  it the hard way...
             */
            retval = (zip_find_end_of_central_dir(io, NULL) != -1);
        } /* if */
    } /* if */

    return retval;
} /* isZip */


/* Convert paths from old, buggy DOS zippers... */
static void zip_convert_dos_path(const PHYSFS_uint16 entryversion, char *path)
{
    const PHYSFS_uint8 hosttype = (PHYSFS_uint8) ((entryversion >> 8) & 0xFF);
    if (hosttype == 0)  /* FS_FAT_ */
    {
        while (*path)
        {
            if (*path == '\\')
                *path = '/';
            path++;
        } /* while */
    } /* if */
} /* zip_convert_dos_path */


static void zip_expand_symlink_path(char *path)
{
    char *ptr = path;
    char *prevptr = path;

    while (1)
    {
        ptr = strchr(ptr, '/');
        if (ptr == NULL)
            break;

        if (*(ptr + 1) == '.')
        {
            if (*(ptr + 2) == '/')
            {
                /* current dir in middle of string: ditch it. */
                memmove(ptr, ptr + 2, strlen(ptr + 2) + 1);
            } /* else if */

            else if (*(ptr + 2) == '\0')
            {
                /* current dir at end of string: ditch it. */
                *ptr = '\0';
            } /* else if */

            else if (*(ptr + 2) == '.')
            {
                if (*(ptr + 3) == '/')
                {
                    /* parent dir in middle: move back one, if possible. */
                    memmove(prevptr, ptr + 4, strlen(ptr + 4) + 1);
                    ptr = prevptr;
                    while (prevptr != path)
                    {
                        prevptr--;
                        if (*prevptr == '/')
                        {
                            prevptr++;
                            break;
                        } /* if */
                    } /* while */
                } /* if */

                if (*(ptr + 3) == '\0')
                {
                    /* parent dir at end: move back one, if possible. */
                    *prevptr = '\0';
                } /* if */
            } /* if */
        } /* if */
        else
        {
            prevptr = ptr;
            ptr++;
        } /* else */
    } /* while */
} /* zip_expand_symlink_path */


static inline ZIPentry *zip_find_entry(ZIPinfo *info, const char *path)
{
    return (ZIPentry *) __PHYSFS_DirTreeFind(&info->tree, path);
} /* zip_find_entry */

/* (forward reference: zip_follow_symlink and zip_resolve call each other.) */
static int zip_resolve(PHYSFS_Io *io, ZIPinfo *info, ZIPentry *entry);

/*
 * Look for the entry named by (path). If it exists, resolve it, and return
 *  a pointer to that entry. If it's another symlink, keep resolving until you
 *  hit a real file and then return a pointer to the final non-symlink entry.
 *  If there's a problem, return NULL.
 */
static ZIPentry *zip_follow_symlink(PHYSFS_Io *io, ZIPinfo *info, char *path)
{
    ZIPentry *entry;

    zip_expand_symlink_path(path);
    entry = zip_find_entry(info, path);
    if (entry != NULL)
    {
        if (!zip_resolve(io, info, entry))  /* recursive! */
            entry = NULL;
        else
        {
            if (entry->symlink != NULL)
                entry = entry->symlink;
        } /* else */
    } /* if */

    return entry;
} /* zip_follow_symlink */


static int zip_resolve_symlink(PHYSFS_Io *io, ZIPinfo *info, ZIPentry *entry)
{
    const size_t size = (size_t) entry->uncompressed_size;
    char *path = NULL;
    int rc = 0;

    /*
     * We've already parsed the local file header of the symlink at this
     *  point. Now we need to read the actual link from the file data and
     *  follow it.
     */

    BAIL_IF_ERRPASS(!io->seek(io, entry->offset), 0);

    path = (char *) __PHYSFS_smallAlloc(size + 1);
    BAIL_IF(!path, PHYSFS_ERR_OUT_OF_MEMORY, 0);
    
    if (entry->compression_method == COMPMETH_NONE)
        rc = __PHYSFS_readAll(io, path, size);

    else  /* symlink target path is compressed... */
    {
        z_stream stream;
        const size_t complen = (size_t) entry->compressed_size;
        PHYSFS_uint8 *compressed = (PHYSFS_uint8*) __PHYSFS_smallAlloc(complen);
        if (compressed != NULL)
        {
            if (__PHYSFS_readAll(io, compressed, complen))
            {
                initializeZStream(&stream);
                stream.next_in = compressed;
                stream.avail_in = (unsigned int) complen;
                stream.next_out = (unsigned char *) path;
                stream.avail_out = (unsigned int) size;
                if (zlib_err(inflateInit2(&stream, -MAX_WBITS)) == Z_OK)
                {
                    rc = zlib_err(inflate(&stream, Z_FINISH));
                    inflateEnd(&stream);

                    /* both are acceptable outcomes... */
                    rc = ((rc == Z_OK) || (rc == Z_STREAM_END));
                } /* if */
            } /* if */
            __PHYSFS_smallFree(compressed);
        } /* if */
    } /* else */

    if (rc)
    {
        path[entry->uncompressed_size] = '\0';    /* null-terminate it. */
        zip_convert_dos_path(entry->version, path);
        entry->symlink = zip_follow_symlink(io, info, path);
    } /* else */

    __PHYSFS_smallFree(path);

    return (entry->symlink != NULL);
} /* zip_resolve_symlink */


/*
 * Parse the local file header of an entry, and update entry->offset.
 */
static int zip_parse_local(PHYSFS_Io *io, ZIPentry *entry)
{
    PHYSFS_uint32 ui32;
    PHYSFS_uint16 ui16;
    PHYSFS_uint16 fnamelen;
    PHYSFS_uint16 extralen;

    /*
     * crc and (un)compressed_size are always zero if this is a "JAR"
     *  archive created with Sun's Java tools, apparently. We only
     *  consider this archive corrupted if those entries don't match and
     *  aren't zero. That seems to work well.
     * We also ignore a mismatch if the value is 0xFFFFFFFF here, since it's
     *  possible that's a Zip64 thing.
     */

    /* !!! FIXME: apparently these are zero if general purpose bit 3 is set,
       !!! FIXME:  which is probably true for Jar files, fwiw, but we don't
       !!! FIXME:  care about these values anyhow. */

    BAIL_IF_ERRPASS(!io->seek(io, entry->offset), 0);
    BAIL_IF_ERRPASS(!readui32(io, &ui32), 0);
    BAIL_IF(ui32 != ZIP_LOCAL_FILE_SIG, PHYSFS_ERR_CORRUPT, 0);
    BAIL_IF_ERRPASS(!readui16(io, &ui16), 0);
    /* Windows Explorer might rewrite the entire central directory, setting
       this field to 2.0/MS-DOS for all files, so favor the local version,
       which it leaves intact if it didn't alter that specific file. */
    entry->version_needed = ui16;
    BAIL_IF_ERRPASS(!readui16(io, &ui16), 0);  /* general bits. */
    BAIL_IF_ERRPASS(!readui16(io, &ui16), 0);
    BAIL_IF(ui16 != entry->compression_method, PHYSFS_ERR_CORRUPT, 0);
    BAIL_IF_ERRPASS(!readui32(io, &ui32), 0);  /* date/time */
    BAIL_IF_ERRPASS(!readui32(io, &ui32), 0);
    BAIL_IF(ui32 && (ui32 != entry->crc), PHYSFS_ERR_CORRUPT, 0);

    BAIL_IF_ERRPASS(!readui32(io, &ui32), 0);
    BAIL_IF(ui32 && (ui32 != 0xFFFFFFFF) &&
                  (ui32 != entry->compressed_size), PHYSFS_ERR_CORRUPT, 0);

    BAIL_IF_ERRPASS(!readui32(io, &ui32), 0);
    BAIL_IF(ui32 && (ui32 != 0xFFFFFFFF) &&
                 (ui32 != entry->uncompressed_size), PHYSFS_ERR_CORRUPT, 0);

    BAIL_IF_ERRPASS(!readui16(io, &fnamelen), 0);
    BAIL_IF_ERRPASS(!readui16(io, &extralen), 0);

    entry->offset += fnamelen + extralen + 30;
    return 1;
} /* zip_parse_local */


static int zip_resolve(PHYSFS_Io *io, ZIPinfo *info, ZIPentry *entry)
{
    int retval = 1;
    const ZipResolveType resolve_type = entry->resolved;

    if (resolve_type == ZIP_DIRECTORY)
        return 1;   /* we're good. */

    /* Don't bother if we've failed to resolve this entry before. */
    BAIL_IF(resolve_type == ZIP_BROKEN_FILE, PHYSFS_ERR_CORRUPT, 0);
    BAIL_IF(resolve_type == ZIP_BROKEN_SYMLINK, PHYSFS_ERR_CORRUPT, 0);

    /* uhoh...infinite symlink loop! */
    BAIL_IF(resolve_type == ZIP_RESOLVING, PHYSFS_ERR_SYMLINK_LOOP, 0);

    /*
     * We fix up the offset to point to the actual data on the
     *  first open, since we don't want to seek across the whole file on
     *  archive open (can be SLOW on large, CD-stored files), but we
     *  need to check the local file header...not just for corruption,
     *  but since it stores offset info the central directory does not.
     */
    if (resolve_type != ZIP_RESOLVED)
    {
        if (entry->tree.isdir)  /* an ancestor dir that DirTree filled in? */
        {
            entry->resolved = ZIP_DIRECTORY;
            return 1;
        } /* if */

        retval = zip_parse_local(io, entry);
        if (retval)
        {
            /*
             * If it's a symlink, find the original file. This will cause
             *  resolution of other entries (other symlinks and, eventually,
             *  the real file) if all goes well.
             */
            if (resolve_type == ZIP_UNRESOLVED_SYMLINK)
                retval = zip_resolve_symlink(io, info, entry);
        } /* if */

        if (resolve_type == ZIP_UNRESOLVED_SYMLINK)
            entry->resolved = ((retval) ? ZIP_RESOLVED : ZIP_BROKEN_SYMLINK);
        else if (resolve_type == ZIP_UNRESOLVED_FILE)
            entry->resolved = ((retval) ? ZIP_RESOLVED : ZIP_BROKEN_FILE);
    } /* if */

    return retval;
} /* zip_resolve */


static int zip_entry_is_symlink(const ZIPentry *entry)
{
    return ((entry->resolved == ZIP_UNRESOLVED_SYMLINK) ||
            (entry->resolved == ZIP_BROKEN_SYMLINK) ||
            (entry->symlink));
} /* zip_entry_is_symlink */


static int zip_version_does_symlinks(PHYSFS_uint32 version)
{
    int retval = 0;
    PHYSFS_uint8 hosttype = (PHYSFS_uint8) ((version >> 8) & 0xFF);

    switch (hosttype)
    {
            /*
             * These are the platforms that can NOT build an archive with
             *  symlinks, according to the Info-ZIP project.
             */
        case 0:  /* FS_FAT_  */
        case 1:  /* AMIGA_   */
        case 2:  /* VMS_     */
        case 4:  /* VM_CSM_  */
        case 6:  /* FS_HPFS_ */
        case 11: /* FS_NTFS_ */
        case 14: /* FS_VFAT_ */
        case 13: /* ACORN_   */
        case 15: /* MVS_     */
        case 18: /* THEOS_   */
            break;  /* do nothing. */

        default:  /* assume the rest to be unix-like. */
            retval = 1;
            break;
    } /* switch */

    return retval;
} /* zip_version_does_symlinks */


static inline int zip_has_symlink_attr(const ZIPentry *entry,
                                       const PHYSFS_uint32 extern_attr)
{
    PHYSFS_uint16 xattr = ((extern_attr >> 16) & 0xFFFF);
    return ( (zip_version_does_symlinks(entry->version)) &&
             (entry->uncompressed_size > 0) &&
             ((xattr & UNIX_FILETYPE_MASK) == UNIX_FILETYPE_SYMLINK) );
} /* zip_has_symlink_attr */


static PHYSFS_sint64 zip_dos_time_to_physfs_time(PHYSFS_uint32 dostime)
{
    PHYSFS_uint32 dosdate;
    struct tm unixtime;
    memset(&unixtime, '\0', sizeof (unixtime));

    dosdate = (PHYSFS_uint32) ((dostime >> 16) & 0xFFFF);
    dostime &= 0xFFFF;

    /* dissect date */
    unixtime.tm_year = ((dosdate >> 9) & 0x7F) + 80;
    unixtime.tm_mon  = ((dosdate >> 5) & 0x0F) - 1;
    unixtime.tm_mday = ((dosdate     ) & 0x1F);

    /* dissect time */
    unixtime.tm_hour = ((dostime >> 11) & 0x1F);
    unixtime.tm_min  = ((dostime >>  5) & 0x3F);
    unixtime.tm_sec  = ((dostime <<  1) & 0x3E);

    /* let mktime calculate daylight savings time. */
    unixtime.tm_isdst = -1;

    return ((PHYSFS_sint64) mktime(&unixtime));
} /* zip_dos_time_to_physfs_time */


static ZIPentry *zip_load_entry(ZIPinfo *info, const int zip64,
                                const PHYSFS_uint64 ofs_fixup)
{
    PHYSFS_Io *io = info->io;
    ZIPentry entry;
    ZIPentry *retval = NULL;
    PHYSFS_uint16 fnamelen, extralen, commentlen;
    PHYSFS_uint32 external_attr;
    PHYSFS_uint32 starting_disk;
    PHYSFS_uint64 offset;
    PHYSFS_uint16 ui16;
    PHYSFS_uint32 ui32;
    PHYSFS_sint64 si64;
    char *name = NULL;
    int isdir = 0;

    /* sanity check with central directory signature... */
    BAIL_IF_ERRPASS(!readui32(io, &ui32), NULL);
    BAIL_IF(ui32 != ZIP_CENTRAL_DIR_SIG, PHYSFS_ERR_CORRUPT, NULL);

    memset(&entry, '\0', sizeof (entry));

    /* Get the pertinent parts of the record... */
    BAIL_IF_ERRPASS(!readui16(io, &entry.version), NULL);
    BAIL_IF_ERRPASS(!readui16(io, &entry.version_needed), NULL);
    BAIL_IF_ERRPASS(!readui16(io, &entry.general_bits), NULL);  /* general bits */
    BAIL_IF_ERRPASS(!readui16(io, &entry.compression_method), NULL);
    BAIL_IF_ERRPASS(!readui32(io, &entry.dos_mod_time), NULL);
    entry.last_mod_time = zip_dos_time_to_physfs_time(entry.dos_mod_time);
    BAIL_IF_ERRPASS(!readui32(io, &entry.crc), NULL);
    BAIL_IF_ERRPASS(!readui32(io, &ui32), NULL);
    entry.compressed_size = (PHYSFS_uint64) ui32;
    BAIL_IF_ERRPASS(!readui32(io, &ui32), NULL);
    entry.uncompressed_size = (PHYSFS_uint64) ui32;
    BAIL_IF_ERRPASS(!readui16(io, &fnamelen), NULL);
    BAIL_IF_ERRPASS(!readui16(io, &extralen), NULL);
    BAIL_IF_ERRPASS(!readui16(io, &commentlen), NULL);
    BAIL_IF_ERRPASS(!readui16(io, &ui16), NULL);
    starting_disk = (PHYSFS_uint32) ui16;
    BAIL_IF_ERRPASS(!readui16(io, &ui16), NULL);  /* internal file attribs */
    BAIL_IF_ERRPASS(!readui32(io, &external_attr), NULL);
    BAIL_IF_ERRPASS(!readui32(io, &ui32), NULL);
    offset = (PHYSFS_uint64) ui32;

    name = (char *) __PHYSFS_smallAlloc(fnamelen + 1);
    BAIL_IF(!name, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    if (!__PHYSFS_readAll(io, name, fnamelen))
    {
        __PHYSFS_smallFree(name);
        return NULL;
    } /* if */

    if (name[fnamelen - 1] == '/')
    {
        name[fnamelen - 1] = '\0';
        isdir = 1;
    } /* if */
    name[fnamelen] = '\0';  /* null-terminate the filename. */

    zip_convert_dos_path(entry.version, name);

    retval = (ZIPentry *) __PHYSFS_DirTreeAdd(&info->tree, name, isdir);
    __PHYSFS_smallFree(name);

    BAIL_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, NULL);

    /* It's okay to BAIL without freeing retval, because it's stored in the
       __PHYSFS_DirTree and will be freed later anyhow. */
    BAIL_IF(retval->last_mod_time != 0, PHYSFS_ERR_CORRUPT, NULL); /* dupe? */

    /* Move the data we already read into place in the official object. */
    memcpy(((PHYSFS_uint8 *) retval) + sizeof (__PHYSFS_DirTreeEntry),
           ((PHYSFS_uint8 *) &entry) + sizeof (__PHYSFS_DirTreeEntry),
           sizeof (*retval) - sizeof (__PHYSFS_DirTreeEntry));

    retval->symlink = NULL;  /* will be resolved later, if necessary. */

    if (isdir)
        retval->resolved = ZIP_DIRECTORY;
    else
    {
        retval->resolved = (zip_has_symlink_attr(retval, external_attr)) ?
                                ZIP_UNRESOLVED_SYMLINK : ZIP_UNRESOLVED_FILE;
    } /* else */

    si64 = io->tell(io);
    BAIL_IF_ERRPASS(si64 == -1, NULL);

    /* If the actual sizes didn't fit in 32-bits, look for the Zip64
        extended information extra field... */
    if ( (zip64) &&
         ((offset == 0xFFFFFFFF) ||
          (starting_disk == 0xFFFFFFFF) ||
          (retval->compressed_size == 0xFFFFFFFF) ||
          (retval->uncompressed_size == 0xFFFFFFFF)) )
    {
        int found = 0;
        PHYSFS_uint16 sig = 0;
        PHYSFS_uint16 len = 0;
        while (extralen > 4)
        {
            BAIL_IF_ERRPASS(!readui16(io, &sig), NULL);
            BAIL_IF_ERRPASS(!readui16(io, &len), NULL);

            si64 += 4 + len;
            extralen -= 4 + len;
            if (sig != ZIP64_EXTENDED_INFO_EXTRA_FIELD_SIG)
            {
                BAIL_IF_ERRPASS(!io->seek(io, si64), NULL);
                continue;
            } /* if */

            found = 1;
            break;
        } /* while */

        BAIL_IF(!found, PHYSFS_ERR_CORRUPT, NULL);

        if (retval->uncompressed_size == 0xFFFFFFFF)
        {
            BAIL_IF(len < 8, PHYSFS_ERR_CORRUPT, NULL);
            BAIL_IF_ERRPASS(!readui64(io, &retval->uncompressed_size), NULL);
            len -= 8;
        } /* if */

        if (retval->compressed_size == 0xFFFFFFFF)
        {
            BAIL_IF(len < 8, PHYSFS_ERR_CORRUPT, NULL);
            BAIL_IF_ERRPASS(!readui64(io, &retval->compressed_size), NULL);
            len -= 8;
        } /* if */

        if (offset == 0xFFFFFFFF)
        {
            BAIL_IF(len < 8, PHYSFS_ERR_CORRUPT, NULL);
            BAIL_IF_ERRPASS(!readui64(io, &offset), NULL);
            len -= 8;
        } /* if */

        if (starting_disk == 0xFFFFFFFF)
        {
            BAIL_IF(len < 8, PHYSFS_ERR_CORRUPT, NULL);
            BAIL_IF_ERRPASS(!readui32(io, &starting_disk), NULL);
            len -= 4;
        } /* if */

        BAIL_IF(len != 0, PHYSFS_ERR_CORRUPT, NULL);
    } /* if */

    BAIL_IF(starting_disk != 0, PHYSFS_ERR_CORRUPT, NULL);

    retval->offset = offset + ofs_fixup;

    /* seek to the start of the next entry in the central directory... */
    BAIL_IF_ERRPASS(!io->seek(io, si64 + extralen + commentlen), NULL);

    return retval;  /* success. */
} /* zip_load_entry */


/* This leaves things allocated on error; the caller will clean up the mess. */
static int zip_load_entries(ZIPinfo *info,
                            const PHYSFS_uint64 data_ofs,
                            const PHYSFS_uint64 central_ofs,
                            const PHYSFS_uint64 entry_count)
{
    PHYSFS_Io *io = info->io;
    const int zip64 = info->zip64;
    PHYSFS_uint64 i;

    BAIL_IF_ERRPASS(!io->seek(io, central_ofs), 0);

    for (i = 0; i < entry_count; i++)
    {
        ZIPentry *entry = zip_load_entry(info, zip64, data_ofs);
        BAIL_IF_ERRPASS(!entry, 0);
        if (zip_entry_is_tradional_crypto(entry))
            info->has_crypto = 1;
    } /* for */

    return 1;
} /* zip_load_entries */


static PHYSFS_sint64 zip64_find_end_of_central_dir(PHYSFS_Io *io,
                                                   PHYSFS_sint64 _pos,
                                                   PHYSFS_uint64 offset)
{
    /*
     * Naturally, the offset is useless to us; it is the offset from the
     *  start of file, which is meaningless if we've appended this .zip to
     *  a self-extracting .exe. We need to find this on our own. It should
     *  be directly before the locator record, but the record in question,
     *  like the original end-of-central-directory record, ends with a
     *  variable-length field. Unlike the original, which has to store the
     *  size of that variable-length field in a 16-bit int and thus has to be
     *  within 64k, the new one gets 64-bits.
     *
     * Fortunately, the only currently-specified record for that variable
     *  length block is some weird proprietary thing that deals with EBCDIC
     *  and tape backups or something. So we don't seek far.
     */

    PHYSFS_uint32 ui32;
    const PHYSFS_uint64 pos = (PHYSFS_uint64) _pos;

    assert(_pos > 0);

    /* Try offset specified in the Zip64 end of central directory locator. */
    /* This works if the entire PHYSFS_Io is the zip file. */
    BAIL_IF_ERRPASS(!io->seek(io, offset), -1);
    BAIL_IF_ERRPASS(!readui32(io, &ui32), -1);
    if (ui32 == ZIP64_END_OF_CENTRAL_DIR_SIG)
        return offset;

    /* Try 56 bytes before the Zip64 end of central directory locator. */
    /* This works if the record isn't variable length and is version 1. */
    if (pos > 56)
    {
        BAIL_IF_ERRPASS(!io->seek(io, pos-56), -1);
        BAIL_IF_ERRPASS(!readui32(io, &ui32), -1);
        if (ui32 == ZIP64_END_OF_CENTRAL_DIR_SIG)
            return pos-56;
    } /* if */

    /* Try 84 bytes before the Zip64 end of central directory locator. */
    /* This works if the record isn't variable length and is version 2. */
    if (pos > 84)
    {
        BAIL_IF_ERRPASS(!io->seek(io, pos-84), -1);
        BAIL_IF_ERRPASS(!readui32(io, &ui32), -1);
        if (ui32 == ZIP64_END_OF_CENTRAL_DIR_SIG)
            return pos-84;
    } /* if */

    /* Ok, brute force: we know it's between (offset) and (pos) somewhere. */
    /*  Just try moving back at most 256k. Oh well. */
    if ((offset < pos) && (pos > 4))
    {
        const size_t maxbuflen = 256 * 1024;
        size_t len = (size_t) (pos - offset);
        PHYSFS_uint8 *buf = NULL;
        PHYSFS_sint32 i;

        if (len > maxbuflen)
            len = maxbuflen;

        buf = (PHYSFS_uint8 *) __PHYSFS_smallAlloc(len);
        BAIL_IF(!buf, PHYSFS_ERR_OUT_OF_MEMORY, -1);

        if (!io->seek(io, pos - len) || !__PHYSFS_readAll(io, buf, len))
        {
            __PHYSFS_smallFree(buf);
            return -1;  /* error was set elsewhere. */
        } /* if */

        for (i = (PHYSFS_sint32) (len - 4); i >= 0; i--)
        {
            if ( (buf[i] == 0x50) && (buf[i+1] == 0x4b) &&
                 (buf[i+2] == 0x06) && (buf[i+3] == 0x06) )
            {
                __PHYSFS_smallFree(buf);
                return pos - ((PHYSFS_sint64) (len - i));
            } /* if */
        } /* for */

        __PHYSFS_smallFree(buf);
    } /* if */

    BAIL(PHYSFS_ERR_CORRUPT, -1);  /* didn't find it. */
} /* zip64_find_end_of_central_dir */


static int zip64_parse_end_of_central_dir(ZIPinfo *info,
                                          PHYSFS_uint64 *data_start,
                                          PHYSFS_uint64 *dir_ofs,
                                          PHYSFS_uint64 *entry_count,
                                          PHYSFS_sint64 pos)
{
    PHYSFS_Io *io = info->io;
    PHYSFS_uint64 ui64;
    PHYSFS_uint32 ui32;
    PHYSFS_uint16 ui16;

    /* We should be positioned right past the locator signature. */

    if ((pos < 0) || (!io->seek(io, pos)))
        return 0;

    BAIL_IF_ERRPASS(!readui32(io, &ui32), 0);
    if (ui32 != ZIP64_END_OF_CENTRAL_DIRECTORY_LOCATOR_SIG)
        return -1;  /* it's not a Zip64 archive. Not an error, though! */

    info->zip64 = 1;

    /* number of the disk with the start of the central directory. */
    BAIL_IF_ERRPASS(!readui32(io, &ui32), 0);
    BAIL_IF(ui32 != 0, PHYSFS_ERR_CORRUPT, 0);

    /* offset of Zip64 end of central directory record. */
    BAIL_IF_ERRPASS(!readui64(io, &ui64), 0);

    /* total number of disks */
    BAIL_IF_ERRPASS(!readui32(io, &ui32), 0);
    BAIL_IF(ui32 != 1, PHYSFS_ERR_CORRUPT, 0);

    pos = zip64_find_end_of_central_dir(io, pos, ui64);
    if (pos < 0)
        return 0;  /* oh well. */

    /*
     * For self-extracting archives, etc, there's crapola in the file
     *  before the zipfile records; we calculate how much data there is
     *  prepended by determining how far the zip64-end-of-central-directory
     *  offset is from where it is supposed to be...the difference in bytes
     *  is how much arbitrary data is at the start of the physical file.
     */
    assert(((PHYSFS_uint64) pos) >= ui64);
    *data_start = ((PHYSFS_uint64) pos) - ui64;

    BAIL_IF_ERRPASS(!io->seek(io, pos), 0);

    /* check signature again, just in case. */
    BAIL_IF_ERRPASS(!readui32(io, &ui32), 0);
    BAIL_IF(ui32 != ZIP64_END_OF_CENTRAL_DIR_SIG, PHYSFS_ERR_CORRUPT, 0);

    /* size of Zip64 end of central directory record. */
    BAIL_IF_ERRPASS(!readui64(io, &ui64), 0);

    /* version made by. */
    BAIL_IF_ERRPASS(!readui16(io, &ui16), 0);

    /* version needed to extract. */
    BAIL_IF_ERRPASS(!readui16(io, &ui16), 0);

    /* number of this disk. */
    BAIL_IF_ERRPASS(!readui32(io, &ui32), 0);
    BAIL_IF(ui32 != 0, PHYSFS_ERR_CORRUPT, 0);

    /* number of disk with start of central directory record. */
    BAIL_IF_ERRPASS(!readui32(io, &ui32), 0);
    BAIL_IF(ui32 != 0, PHYSFS_ERR_CORRUPT, 0);

    /* total number of entries in the central dir on this disk */
    BAIL_IF_ERRPASS(!readui64(io, &ui64), 0);

    /* total number of entries in the central dir */
    BAIL_IF_ERRPASS(!readui64(io, entry_count), 0);
    BAIL_IF(ui64 != *entry_count, PHYSFS_ERR_CORRUPT, 0);

    /* size of the central directory */
    BAIL_IF_ERRPASS(!readui64(io, &ui64), 0);

    /* offset of central directory */
    BAIL_IF_ERRPASS(!readui64(io, dir_ofs), 0);

    /* Since we know the difference, fix up the central dir offset... */
    *dir_ofs += *data_start;

    /*
     * There are more fields here, for encryption and feature-specific things,
     *  but we don't care about any of them at the moment.
     */

    return 1;  /* made it. */
} /* zip64_parse_end_of_central_dir */


static int zip_parse_end_of_central_dir(ZIPinfo *info,
                                        PHYSFS_uint64 *data_start,
                                        PHYSFS_uint64 *dir_ofs,
                                        PHYSFS_uint64 *entry_count)
{
    PHYSFS_Io *io = info->io;
    PHYSFS_uint16 entryCount16;
    PHYSFS_uint32 offset32;
    PHYSFS_uint32 ui32;
    PHYSFS_uint16 ui16;
    PHYSFS_sint64 len;
    PHYSFS_sint64 pos;
    int rc;

    /* find the end-of-central-dir record, and seek to it. */
    pos = zip_find_end_of_central_dir(io, &len);
    BAIL_IF_ERRPASS(pos == -1, 0);
    BAIL_IF_ERRPASS(!io->seek(io, pos), 0);

    /* check signature again, just in case. */
    BAIL_IF_ERRPASS(!readui32(io, &ui32), 0);
    BAIL_IF(ui32 != ZIP_END_OF_CENTRAL_DIR_SIG, PHYSFS_ERR_CORRUPT, 0);

    /* Seek back to see if "Zip64 end of central directory locator" exists. */
    /* this record is 20 bytes before end-of-central-dir */
    rc = zip64_parse_end_of_central_dir(info, data_start, dir_ofs,
                                        entry_count, pos - 20);

    /* Error or success? Bounce out of here. Keep going if not zip64. */
    if ((rc == 0) || (rc == 1))
        return rc;

    assert(rc == -1);  /* no error, just not a Zip64 archive. */

    /* Not Zip64? Seek back to where we were and keep processing. */
    BAIL_IF_ERRPASS(!io->seek(io, pos + 4), 0);

    /* number of this disk */
    BAIL_IF_ERRPASS(!readui16(io, &ui16), 0);
    BAIL_IF(ui16 != 0, PHYSFS_ERR_CORRUPT, 0);

    /* number of the disk with the start of the central directory */
    BAIL_IF_ERRPASS(!readui16(io, &ui16), 0);
    BAIL_IF(ui16 != 0, PHYSFS_ERR_CORRUPT, 0);

    /* total number of entries in the central dir on this disk */
    BAIL_IF_ERRPASS(!readui16(io, &ui16), 0);

    /* total number of entries in the central dir */
    BAIL_IF_ERRPASS(!readui16(io, &entryCount16), 0);
    BAIL_IF(ui16 != entryCount16, PHYSFS_ERR_CORRUPT, 0);

    *entry_count = entryCount16;

    /* size of the central directory */
    BAIL_IF_ERRPASS(!readui32(io, &ui32), 0);

    /* offset of central directory */
    BAIL_IF_ERRPASS(!readui32(io, &offset32), 0);
    *dir_ofs = (PHYSFS_uint64) offset32;
    BAIL_IF(((PHYSFS_uint64) pos) < (*dir_ofs + ui32), PHYSFS_ERR_CORRUPT, 0);

    /*
     * For self-extracting archives, etc, there's crapola in the file
     *  before the zipfile records; we calculate how much data there is
     *  prepended by determining how far the central directory offset is
     *  from where it is supposed to be (start of end-of-central-dir minus
     *  sizeof central dir)...the difference in bytes is how much arbitrary
     *  data is at the start of the physical file.
     */
    *data_start = (PHYSFS_uint64) (pos - (*dir_ofs + ui32));

    /* Now that we know the difference, fix up the central dir offset... */
    *dir_ofs += *data_start;

    /* zipfile comment length */
    BAIL_IF_ERRPASS(!readui16(io, &ui16), 0);

    /*
     * Make sure that the comment length matches to the end of file...
     *  If it doesn't, we're either in the wrong part of the file, or the
     *  file is corrupted, but we give up either way.
     */
    BAIL_IF((pos + 22 + ui16) != len, PHYSFS_ERR_CORRUPT, 0);

    return 1;  /* made it. */
} /* zip_parse_end_of_central_dir */


static void ZIP_closeArchive(void *opaque)
{
    ZIPinfo *info = (ZIPinfo *) (opaque);

    if (!info)
        return;

    if (info->io)
        info->io->destroy(info->io);

    __PHYSFS_DirTreeDeinit(&info->tree);

    allocator.Free(info);
} /* ZIP_closeArchive */


static void *ZIP_openArchive(PHYSFS_Io *io, const char *name,
                             int forWriting, int *claimed)
{
    ZIPinfo *info = NULL;
    ZIPentry *root = NULL;
    PHYSFS_uint64 dstart = 0;  /* data start */
    PHYSFS_uint64 cdir_ofs;  /* central dir offset */
    PHYSFS_uint64 count;

    assert(io != NULL);  /* shouldn't ever happen. */

    BAIL_IF(forWriting, PHYSFS_ERR_READ_ONLY, NULL);
    BAIL_IF_ERRPASS(!isZip(io), NULL);

    *claimed = 1;

    info = (ZIPinfo *) allocator.Malloc(sizeof (ZIPinfo));
    BAIL_IF(!info, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    memset(info, '\0', sizeof (ZIPinfo));

    info->io = io;

    if (!zip_parse_end_of_central_dir(info, &dstart, &cdir_ofs, &count))
        goto ZIP_openarchive_failed;
    else if (!__PHYSFS_DirTreeInit(&info->tree, sizeof (ZIPentry), 1, 0))
        goto ZIP_openarchive_failed;

    root = (ZIPentry *) info->tree.root;
    root->resolved = ZIP_DIRECTORY;

    if (!zip_load_entries(info, dstart, cdir_ofs, count))
        goto ZIP_openarchive_failed;

    assert(info->tree.root->sibling == NULL);
    return info;

ZIP_openarchive_failed:
    info->io = NULL;  /* don't let ZIP_closeArchive destroy (io). */
    ZIP_closeArchive(info);
    return NULL;
} /* ZIP_openArchive */


static PHYSFS_Io *zip_get_io(PHYSFS_Io *io, ZIPinfo *inf, ZIPentry *entry)
{
    int success;
    PHYSFS_Io *retval = io->duplicate(io);
    BAIL_IF_ERRPASS(!retval, NULL);

    assert(!entry->tree.isdir); /* should have been checked before calling. */

    /* (inf) can be NULL if we already resolved. */
    success = (inf == NULL) || zip_resolve(retval, inf, entry);
    if (success)
    {
        PHYSFS_sint64 offset;
        offset = ((entry->symlink) ? entry->symlink->offset : entry->offset);
        success = retval->seek(retval, offset);
    } /* if */

    if (!success)
    {
        retval->destroy(retval);
        retval = NULL;
    } /* if */

    return retval;
} /* zip_get_io */


static PHYSFS_Io *ZIP_openRead(void *opaque, const char *filename)
{
    PHYSFS_Io *retval = NULL;
    ZIPinfo *info = (ZIPinfo *) opaque;
    ZIPentry *entry = zip_find_entry(info, filename);
    ZIPfileinfo *finfo = NULL;
    PHYSFS_Io *io = NULL;
    PHYSFS_uint8 *password = NULL;

    /* if not found, see if maybe "$PASSWORD" is appended. */
    if ((!entry) && (info->has_crypto))
    {
        const char *ptr = strrchr(filename, '$');
        if (ptr != NULL)
        {
            const size_t len = (size_t) (ptr - filename);
            char *str = (char *) __PHYSFS_smallAlloc(len + 1);
            BAIL_IF(!str, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
            memcpy(str, filename, len);
            str[len] = '\0';
            entry = zip_find_entry(info, str);
            __PHYSFS_smallFree(str);
            password = (PHYSFS_uint8 *) (ptr + 1);
        } /* if */
    } /* if */

    BAIL_IF_ERRPASS(!entry, NULL);

    BAIL_IF_ERRPASS(!zip_resolve(info->io, info, entry), NULL);

    BAIL_IF(entry->tree.isdir, PHYSFS_ERR_NOT_A_FILE, NULL);

    retval = (PHYSFS_Io *) allocator.Malloc(sizeof (PHYSFS_Io));
    GOTO_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, ZIP_openRead_failed);

    finfo = (ZIPfileinfo *) allocator.Malloc(sizeof (ZIPfileinfo));
    GOTO_IF(!finfo, PHYSFS_ERR_OUT_OF_MEMORY, ZIP_openRead_failed);
    memset(finfo, '\0', sizeof (ZIPfileinfo));

    io = zip_get_io(info->io, info, entry);
    GOTO_IF_ERRPASS(!io, ZIP_openRead_failed);
    finfo->io = io;
    finfo->entry = ((entry->symlink != NULL) ? entry->symlink : entry);
    initializeZStream(&finfo->stream);

    if (finfo->entry->compression_method != COMPMETH_NONE)
    {
        finfo->buffer = (PHYSFS_uint8 *) allocator.Malloc(ZIP_READBUFSIZE);
        if (!finfo->buffer)
            GOTO(PHYSFS_ERR_OUT_OF_MEMORY, ZIP_openRead_failed);
        else if (zlib_err(inflateInit2(&finfo->stream, -MAX_WBITS)) != Z_OK)
            goto ZIP_openRead_failed;
    } /* if */

    if (!zip_entry_is_tradional_crypto(entry))
        GOTO_IF(password != NULL, PHYSFS_ERR_BAD_PASSWORD, ZIP_openRead_failed);
    else
    {
        PHYSFS_uint8 crypto_header[12];
        GOTO_IF(password == NULL, PHYSFS_ERR_BAD_PASSWORD, ZIP_openRead_failed);
        if (io->read(io, crypto_header, 12) != 12)
            goto ZIP_openRead_failed;
        else if (!zip_prep_crypto_keys(finfo, crypto_header, password))
            goto ZIP_openRead_failed;
    } /* if */

    memcpy(retval, &ZIP_Io, sizeof (PHYSFS_Io));
    retval->opaque = finfo;

    return retval;

ZIP_openRead_failed:
    if (finfo != NULL)
    {
        if (finfo->io != NULL)
            finfo->io->destroy(finfo->io);

        if (finfo->buffer != NULL)
        {
            allocator.Free(finfo->buffer);
            inflateEnd(&finfo->stream);
        } /* if */

        allocator.Free(finfo);
    } /* if */

    if (retval != NULL)
        allocator.Free(retval);

    return NULL;
} /* ZIP_openRead */


static PHYSFS_Io *ZIP_openWrite(void *opaque, const char *filename)
{
    BAIL(PHYSFS_ERR_READ_ONLY, NULL);
} /* ZIP_openWrite */


static PHYSFS_Io *ZIP_openAppend(void *opaque, const char *filename)
{
    BAIL(PHYSFS_ERR_READ_ONLY, NULL);
} /* ZIP_openAppend */


static int ZIP_remove(void *opaque, const char *name)
{
    BAIL(PHYSFS_ERR_READ_ONLY, 0);
} /* ZIP_remove */


static int ZIP_mkdir(void *opaque, const char *name)
{
    BAIL(PHYSFS_ERR_READ_ONLY, 0);
} /* ZIP_mkdir */


static int ZIP_stat(void *opaque, const char *filename, PHYSFS_Stat *stat)
{
    ZIPinfo *info = (ZIPinfo *) opaque;
    ZIPentry *entry = zip_find_entry(info, filename);

    if (entry == NULL)
        return 0;

    else if (!zip_resolve(info->io, info, entry))
        return 0;

    else if (entry->resolved == ZIP_DIRECTORY)
    {
        stat->filesize = 0;
        stat->filetype = PHYSFS_FILETYPE_DIRECTORY;
    } /* if */

    else if (zip_entry_is_symlink(entry))
    {
        stat->filesize = 0;
        stat->filetype = PHYSFS_FILETYPE_SYMLINK;
    } /* else if */

    else
    {
        stat->filesize = (PHYSFS_sint64) entry->uncompressed_size;
        stat->filetype = PHYSFS_FILETYPE_REGULAR;
    } /* else */

    stat->modtime = ((entry) ? entry->last_mod_time : 0);
    stat->createtime = stat->modtime;
    stat->accesstime = -1;
    stat->readonly = 1; /* .zip files are always read only */

    return 1;
} /* ZIP_stat */


const PHYSFS_Archiver __PHYSFS_Archiver_ZIP =
{
    CURRENT_PHYSFS_ARCHIVER_API_VERSION,
    {
        "ZIP",
        "PkZip/WinZip/Info-Zip compatible",
        "Ryan C. Gordon <icculus@icculus.org>",
        "https://icculus.org/physfs/",
        1,  /* supportsSymlinks */
    },
    ZIP_openArchive,
    __PHYSFS_DirTreeEnumerate,
    ZIP_openRead,
    ZIP_openWrite,
    ZIP_openAppend,
    ZIP_remove,
    ZIP_mkdir,
    ZIP_stat,
    ZIP_closeArchive
};

#endif  /* defined PHYSFS_SUPPORTS_ZIP */

/* end of physfs_archiver_zip.c ... */

