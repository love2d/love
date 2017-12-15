#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

#include "physfs_casefolding.h"


/*
 * From rfc3629, the UTF-8 spec:
 *  https://www.ietf.org/rfc/rfc3629.txt
 *
 *   Char. number range  |        UTF-8 octet sequence
 *      (hexadecimal)    |              (binary)
 *   --------------------+---------------------------------------------
 *   0000 0000-0000 007F | 0xxxxxxx
 *   0000 0080-0000 07FF | 110xxxxx 10xxxxxx
 *   0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
 *   0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 */


/*
 * This may not be the best value, but it's one that isn't represented
 *  in Unicode (0x10FFFF is the largest codepoint value). We return this
 *  value from utf8codepoint() if there's bogus bits in the
 *  stream. utf8codepoint() will turn this value into something
 *  reasonable (like a question mark), for text that wants to try to recover,
 *  whereas utf8valid() will use the value to determine if a string has bad
 *  bits.
 */
#define UNICODE_BOGUS_CHAR_VALUE 0xFFFFFFFF

/*
 * This is the codepoint we currently return when there was bogus bits in a
 *  UTF-8 string. May not fly in Asian locales?
 */
#define UNICODE_BOGUS_CHAR_CODEPOINT '?'

static PHYSFS_uint32 utf8codepoint(const char **_str)
{
    const char *str = *_str;
    PHYSFS_uint32 retval = 0;
    PHYSFS_uint32 octet = (PHYSFS_uint32) ((PHYSFS_uint8) *str);
    PHYSFS_uint32 octet2, octet3, octet4;

    if (octet == 0)  /* null terminator, end of string. */
        return 0;

    else if (octet < 128)  /* one octet char: 0 to 127 */
    {
        (*_str)++;  /* skip to next possible start of codepoint. */
        return octet;
    } /* else if */

    else if ((octet > 127) && (octet < 192))  /* bad (starts with 10xxxxxx). */
    {
        /*
         * Apparently each of these is supposed to be flagged as a bogus
         *  char, instead of just resyncing to the next valid codepoint.
         */
        (*_str)++;  /* skip to next possible start of codepoint. */
        return UNICODE_BOGUS_CHAR_VALUE;
    } /* else if */

    else if (octet < 224)  /* two octets */
    {
        (*_str)++;  /* advance at least one byte in case of an error */
        octet -= (128+64);
        octet2 = (PHYSFS_uint32) ((PHYSFS_uint8) *(++str));
        if ((octet2 & (128+64)) != 128)  /* Format isn't 10xxxxxx? */
            return UNICODE_BOGUS_CHAR_VALUE;

        *_str += 1;  /* skip to next possible start of codepoint. */
        retval = ((octet << 6) | (octet2 - 128));
        if ((retval >= 0x80) && (retval <= 0x7FF))
            return retval;
    } /* else if */

    else if (octet < 240)  /* three octets */
    {
        (*_str)++;  /* advance at least one byte in case of an error */
        octet -= (128+64+32);
        octet2 = (PHYSFS_uint32) ((PHYSFS_uint8) *(++str));
        if ((octet2 & (128+64)) != 128)  /* Format isn't 10xxxxxx? */
            return UNICODE_BOGUS_CHAR_VALUE;

        octet3 = (PHYSFS_uint32) ((PHYSFS_uint8) *(++str));
        if ((octet3 & (128+64)) != 128)  /* Format isn't 10xxxxxx? */
            return UNICODE_BOGUS_CHAR_VALUE;

        *_str += 2;  /* skip to next possible start of codepoint. */
        retval = ( ((octet << 12)) | ((octet2-128) << 6) | ((octet3-128)) );

        /* There are seven "UTF-16 surrogates" that are illegal in UTF-8. */
        switch (retval)
        {
            case 0xD800:
            case 0xDB7F:
            case 0xDB80:
            case 0xDBFF:
            case 0xDC00:
            case 0xDF80:
            case 0xDFFF:
                return UNICODE_BOGUS_CHAR_VALUE;
        } /* switch */

        /* 0xFFFE and 0xFFFF are illegal, too, so we check them at the edge. */
        if ((retval >= 0x800) && (retval <= 0xFFFD))
            return retval;
    } /* else if */

    else if (octet < 248)  /* four octets */
    {
        (*_str)++;  /* advance at least one byte in case of an error */
        octet -= (128+64+32+16);
        octet2 = (PHYSFS_uint32) ((PHYSFS_uint8) *(++str));
        if ((octet2 & (128+64)) != 128)  /* Format isn't 10xxxxxx? */
            return UNICODE_BOGUS_CHAR_VALUE;

        octet3 = (PHYSFS_uint32) ((PHYSFS_uint8) *(++str));
        if ((octet3 & (128+64)) != 128)  /* Format isn't 10xxxxxx? */
            return UNICODE_BOGUS_CHAR_VALUE;

        octet4 = (PHYSFS_uint32) ((PHYSFS_uint8) *(++str));
        if ((octet4 & (128+64)) != 128)  /* Format isn't 10xxxxxx? */
            return UNICODE_BOGUS_CHAR_VALUE;

        *_str += 3;  /* skip to next possible start of codepoint. */
        retval = ( ((octet << 18)) | ((octet2 - 128) << 12) |
                   ((octet3 - 128) << 6) | ((octet4 - 128)) );
        if ((retval >= 0x10000) && (retval <= 0x10FFFF))
            return retval;
    } /* else if */

    /*
     * Five and six octet sequences became illegal in rfc3629.
     *  We throw the codepoint away, but parse them to make sure we move
     *  ahead the right number of bytes and don't overflow the buffer.
     */

    else if (octet < 252)  /* five octets */
    {
        (*_str)++;  /* advance at least one byte in case of an error */
        octet = (PHYSFS_uint32) ((PHYSFS_uint8) *(++str));
        if ((octet & (128+64)) != 128)  /* Format isn't 10xxxxxx? */
            return UNICODE_BOGUS_CHAR_VALUE;

        octet = (PHYSFS_uint32) ((PHYSFS_uint8) *(++str));
        if ((octet & (128+64)) != 128)  /* Format isn't 10xxxxxx? */
            return UNICODE_BOGUS_CHAR_VALUE;

        octet = (PHYSFS_uint32) ((PHYSFS_uint8) *(++str));
        if ((octet & (128+64)) != 128)  /* Format isn't 10xxxxxx? */
            return UNICODE_BOGUS_CHAR_VALUE;

        octet = (PHYSFS_uint32) ((PHYSFS_uint8) *(++str));
        if ((octet & (128+64)) != 128)  /* Format isn't 10xxxxxx? */
            return UNICODE_BOGUS_CHAR_VALUE;

        *_str += 4;  /* skip to next possible start of codepoint. */
        return UNICODE_BOGUS_CHAR_VALUE;
    } /* else if */

    else  /* six octets */
    {
        (*_str)++;  /* advance at least one byte in case of an error */
        octet = (PHYSFS_uint32) ((PHYSFS_uint8) *(++str));
        if ((octet & (128+64)) != 128)  /* Format isn't 10xxxxxx? */
            return UNICODE_BOGUS_CHAR_VALUE;

        octet = (PHYSFS_uint32) ((PHYSFS_uint8) *(++str));
        if ((octet & (128+64)) != 128)  /* Format isn't 10xxxxxx? */
            return UNICODE_BOGUS_CHAR_VALUE;

        octet = (PHYSFS_uint32) ((PHYSFS_uint8) *(++str));
        if ((octet & (128+64)) != 128)  /* Format isn't 10xxxxxx? */
            return UNICODE_BOGUS_CHAR_VALUE;

        octet = (PHYSFS_uint32) ((PHYSFS_uint8) *(++str));
        if ((octet & (128+64)) != 128)  /* Format isn't 10xxxxxx? */
            return UNICODE_BOGUS_CHAR_VALUE;

        octet = (PHYSFS_uint32) ((PHYSFS_uint8) *(++str));
        if ((octet & (128+64)) != 128)  /* Format isn't 10xxxxxx? */
            return UNICODE_BOGUS_CHAR_VALUE;

        *_str += 6;  /* skip to next possible start of codepoint. */
        return UNICODE_BOGUS_CHAR_VALUE;
    } /* else if */

    return UNICODE_BOGUS_CHAR_VALUE;
} /* utf8codepoint */

static PHYSFS_uint32 utf16codepoint(const PHYSFS_uint16 **_str)
{
    const PHYSFS_uint16 *src = *_str;
    PHYSFS_uint32 cp = (PHYSFS_uint32) *(src++);

    if (cp == 0)  /* null terminator, end of string. */
        return 0;
    /* Orphaned second half of surrogate pair? */
    else if ((cp >= 0xDC00) && (cp <= 0xDFFF))
        cp = UNICODE_BOGUS_CHAR_CODEPOINT;
    else if ((cp >= 0xD800) && (cp <= 0xDBFF))  /* start surrogate pair! */
    {
        const PHYSFS_uint32 pair = (PHYSFS_uint32) *src;
        if (pair == 0)
            cp = UNICODE_BOGUS_CHAR_CODEPOINT;
        else if ((pair < 0xDC00) || (pair > 0xDFFF))
            cp = UNICODE_BOGUS_CHAR_CODEPOINT;
        else
        {
            src++;  /* eat the other surrogate. */
            cp = (((cp - 0xD800) << 10) | (pair - 0xDC00));
        } /* else */
    } /* else if */

    *_str = src;
    return cp;
} /* utf16codepoint */

static PHYSFS_uint32 utf32codepoint(const PHYSFS_uint32 **_str)
{
    const PHYSFS_uint32 *src = *_str;
    PHYSFS_uint32 cp = *(src++);

    if (cp == 0)  /* null terminator, end of string. */
        return 0;
    else if (cp > 0x10FFF)
        cp = UNICODE_BOGUS_CHAR_CODEPOINT;

    *_str = src;
    return cp;
} /* utf32codepoint */


void PHYSFS_utf8ToUcs4(const char *src, PHYSFS_uint32 *dst, PHYSFS_uint64 len)
{
    len -= sizeof (PHYSFS_uint32);   /* save room for null char. */
    while (len >= sizeof (PHYSFS_uint32))
    {
        PHYSFS_uint32 cp = utf8codepoint(&src);
        if (cp == 0)
            break;
        else if (cp == UNICODE_BOGUS_CHAR_VALUE)
            cp = UNICODE_BOGUS_CHAR_CODEPOINT;
        *(dst++) = cp;
        len -= sizeof (PHYSFS_uint32);
    } /* while */

    *dst = 0;
} /* PHYSFS_utf8ToUcs4 */


void PHYSFS_utf8ToUcs2(const char *src, PHYSFS_uint16 *dst, PHYSFS_uint64 len)
{
    len -= sizeof (PHYSFS_uint16);   /* save room for null char. */
    while (len >= sizeof (PHYSFS_uint16))
    {
        PHYSFS_uint32 cp = utf8codepoint(&src);
        if (cp == 0)
            break;
        else if (cp == UNICODE_BOGUS_CHAR_VALUE)
            cp = UNICODE_BOGUS_CHAR_CODEPOINT;

        if (cp > 0xFFFF)  /* UTF-16 surrogates (bogus chars in UCS-2) */
            cp = UNICODE_BOGUS_CHAR_CODEPOINT;

        *(dst++) = cp;
        len -= sizeof (PHYSFS_uint16);
    } /* while */

    *dst = 0;
} /* PHYSFS_utf8ToUcs2 */


void PHYSFS_utf8ToUtf16(const char *src, PHYSFS_uint16 *dst, PHYSFS_uint64 len)
{
    len -= sizeof (PHYSFS_uint16);   /* save room for null char. */
    while (len >= sizeof (PHYSFS_uint16))
    {
        PHYSFS_uint32 cp = utf8codepoint(&src);
        if (cp == 0)
            break;
        else if (cp == UNICODE_BOGUS_CHAR_VALUE)
            cp = UNICODE_BOGUS_CHAR_CODEPOINT;

        if (cp > 0xFFFF)  /* encode as surrogate pair */
        {
            if (len < (sizeof (PHYSFS_uint16) * 2))
                break;  /* not enough room for the pair, stop now. */

            cp -= 0x10000;  /* Make this a 20-bit value */

            *(dst++) = 0xD800 + ((cp >> 10) & 0x3FF);
            len -= sizeof (PHYSFS_uint16);

            cp = 0xDC00 + (cp & 0x3FF);
        } /* if */

        *(dst++) = cp;
        len -= sizeof (PHYSFS_uint16);
    } /* while */

    *dst = 0;
} /* PHYSFS_utf8ToUtf16 */

static void utf8fromcodepoint(PHYSFS_uint32 cp, char **_dst, PHYSFS_uint64 *_len)
{
    char *dst = *_dst;
    PHYSFS_uint64 len = *_len;

    if (len == 0)
        return;

    if (cp > 0x10FFFF)
        cp = UNICODE_BOGUS_CHAR_CODEPOINT;
    else if ((cp == 0xFFFE) || (cp == 0xFFFF))  /* illegal values. */
        cp = UNICODE_BOGUS_CHAR_CODEPOINT;
    else
    {
        /* There are seven "UTF-16 surrogates" that are illegal in UTF-8. */
        switch (cp)
        {
            case 0xD800:
            case 0xDB7F:
            case 0xDB80:
            case 0xDBFF:
            case 0xDC00:
            case 0xDF80:
            case 0xDFFF:
                cp = UNICODE_BOGUS_CHAR_CODEPOINT;
        } /* switch */
    } /* else */

    /* Do the encoding... */
    if (cp < 0x80)
    {
        *(dst++) = (char) cp;
        len--;
    } /* if */

    else if (cp < 0x800)
    {
        if (len < 2)
            len = 0;
        else
        {
            *(dst++) = (char) ((cp >> 6) | 128 | 64);
            *(dst++) = (char) (cp & 0x3F) | 128;
            len -= 2;
        } /* else */
    } /* else if */

    else if (cp < 0x10000)
    {
        if (len < 3)
            len = 0;
        else
        {
            *(dst++) = (char) ((cp >> 12) | 128 | 64 | 32);
            *(dst++) = (char) ((cp >> 6) & 0x3F) | 128;
            *(dst++) = (char) (cp & 0x3F) | 128;
            len -= 3;
        } /* else */
    } /* else if */

    else
    {
        if (len < 4)
            len = 0;
        else
        {
            *(dst++) = (char) ((cp >> 18) | 128 | 64 | 32 | 16);
            *(dst++) = (char) ((cp >> 12) & 0x3F) | 128;
            *(dst++) = (char) ((cp >> 6) & 0x3F) | 128;
            *(dst++) = (char) (cp & 0x3F) | 128;
            len -= 4;
        } /* else if */
    } /* else */

    *_dst = dst;
    *_len = len;
} /* utf8fromcodepoint */

#define UTF8FROMTYPE(typ, src, dst, len) \
    if (len == 0) return; \
    len--;  \
    while (len) \
    { \
        const PHYSFS_uint32 cp = (PHYSFS_uint32) ((typ) (*(src++))); \
        if (cp == 0) break; \
        utf8fromcodepoint(cp, &dst, &len); \
    } \
    *dst = '\0'; \

void PHYSFS_utf8FromUcs4(const PHYSFS_uint32 *src, char *dst, PHYSFS_uint64 len)
{
    UTF8FROMTYPE(PHYSFS_uint32, src, dst, len);
} /* PHYSFS_utf8FromUcs4 */

void PHYSFS_utf8FromUcs2(const PHYSFS_uint16 *src, char *dst, PHYSFS_uint64 len)
{
    UTF8FROMTYPE(PHYSFS_uint64, src, dst, len);
} /* PHYSFS_utf8FromUcs2 */

/* latin1 maps to unicode codepoints directly, we just utf-8 encode it. */
void PHYSFS_utf8FromLatin1(const char *src, char *dst, PHYSFS_uint64 len)
{
    UTF8FROMTYPE(PHYSFS_uint8, src, dst, len);
} /* PHYSFS_utf8FromLatin1 */

#undef UTF8FROMTYPE


void PHYSFS_utf8FromUtf16(const PHYSFS_uint16 *src, char *dst, PHYSFS_uint64 len)
{
    if (len == 0)
        return;

    len--;
    while (len)
    {
        const PHYSFS_uint32 cp = utf16codepoint(&src);
        if (!cp)
            break;
        utf8fromcodepoint(cp, &dst, &len);
    } /* while */

    *dst = '\0';
} /* PHYSFS_utf8FromUtf16 */


int PHYSFS_caseFold(const PHYSFS_uint32 from, PHYSFS_uint32 *to)
{
    int i;

    if (from < 128)  /* low-ASCII, easy! */
    {
        if ((from >= 'A') && (from <= 'Z'))
            *to = from - ('A' - 'a');
        else
            *to = from;
        return 1;
    } /* if */

    else if (from <= 0xFFFF)
    {
        const PHYSFS_uint8 hash = ((from ^ (from >> 8)) & 0xFF);
        const PHYSFS_uint16 from16 = (PHYSFS_uint16) from;

        {
            const CaseFoldHashBucket1_16 *bucket = &case_fold_hash1_16[hash];
            const int count = (int) bucket->count;
            for (i = 0; i < count; i++)
            {
                const CaseFoldMapping1_16 *mapping = &bucket->list[i];
                if (mapping->from == from16)
                {
                    *to = mapping->to0;
                    return 1;
                } /* if */
            } /* for */
        }

        {
            const CaseFoldHashBucket2_16 *bucket = &case_fold_hash2_16[hash & 15];
            const int count = (int) bucket->count;
            for (i = 0; i < count; i++)
            {
                const CaseFoldMapping2_16 *mapping = &bucket->list[i];
                if (mapping->from == from16)
                {
                    to[0] = mapping->to0;
                    to[1] = mapping->to1;
                    return 2;
                } /* if */
            } /* for */
        }

        {
            const CaseFoldHashBucket3_16 *bucket = &case_fold_hash3_16[hash & 3];
            const int count = (int) bucket->count;
            for (i = 0; i < count; i++)
            {
                const CaseFoldMapping3_16 *mapping = &bucket->list[i];
                if (mapping->from == from16)
                {
                    to[0] = mapping->to0;
                    to[1] = mapping->to1;
                    to[2] = mapping->to2;
                    return 3;
                } /* if */
            } /* for */
        }
    } /* else if */

    else  /* codepoint that doesn't fit in 16 bits. */
    {
        const PHYSFS_uint8 hash = ((from ^ (from >> 8)) & 0xFF);
        const CaseFoldHashBucket1_32 *bucket = &case_fold_hash1_32[hash & 15];
        const int count = (int) bucket->count;
        for (i = 0; i < count; i++)
        {
            const CaseFoldMapping1_32 *mapping = &bucket->list[i];
            if (mapping->from == from)
            {
                *to = mapping->to0;
                return 1;
            } /* if */
        } /* for */
    } /* else */


    /* Not found...there's no remapping for this codepoint. */
    *to = from;
    return 1;
} /* PHYSFS_caseFold */


#define UTFSTRICMP(bits) \
    PHYSFS_uint32 folded1[3], folded2[3]; \
    int head1 = 0, tail1 = 0, head2 = 0, tail2 = 0; \
    while (1) { \
        PHYSFS_uint32 cp1, cp2; \
        if (head1 != tail1) { \
            cp1 = folded1[tail1++]; \
        } else { \
            head1 = PHYSFS_caseFold(utf##bits##codepoint(&str1), folded1); \
            cp1 = folded1[0]; \
            tail1 = 1; \
        } \
        if (head2 != tail2) { \
            cp2 = folded2[tail2++]; \
        } else { \
            head2 = PHYSFS_caseFold(utf##bits##codepoint(&str2), folded2); \
            cp2 = folded2[0]; \
            tail2 = 1; \
        } \
        if (cp1 < cp2) { \
            return -1; \
        } else if (cp1 > cp2) { \
            return 1; \
        } else if (cp1 == 0) { \
            break;  /* complete match. */ \
        } \
    } \
    return 0

int PHYSFS_utf8stricmp(const char *str1, const char *str2)
{
    UTFSTRICMP(8);
} /* PHYSFS_utf8stricmp */

int PHYSFS_utf16stricmp(const PHYSFS_uint16 *str1, const PHYSFS_uint16 *str2)
{
    UTFSTRICMP(16);
} /* PHYSFS_utf16stricmp */

int PHYSFS_ucs4stricmp(const PHYSFS_uint32 *str1, const PHYSFS_uint32 *str2)
{
    UTFSTRICMP(32);
} /* PHYSFS_ucs4stricmp */

#undef UTFSTRICMP

/* end of physfs_unicode.c ... */

