/* stbi-1.35 - public domain JPEG/PNG reader - http://nothings.org/stb_image.c
   when you control the images you're loading
                                     no warranty implied; use at your own risk

   QUICK NOTES:
      Primarily of interest to game developers and other people who can
          avoid problematic images and only need the trivial interface

      JPEG baseline (no JPEG progressive)
      PNG 8-bit-per-channel only

      TGA (not sure what subset, if a subset)
      BMP non-1bpp, non-RLE
      PSD (composited view only, no extra channels)

      GIF (*comp always reports as 4-channel)
      HDR (radiance rgbE format)
      PIC (Softimage PIC)

      - decode from memory or through FILE (define STBI_NO_STDIO to remove code)
      - decode from arbitrary I/O callbacks
      - overridable dequantizing-IDCT, YCbCr-to-RGB conversion (define STBI_SIMD)

   Latest revisions:
      1.35 (2014-05-27) warnings, bugfixes, TGA optimization, etc
      1.34 (unknown   ) warning fix
      1.33 (2011-07-14) minor fixes suggested by Dave Moore
      1.32 (2011-07-13) info support for all filetypes (SpartanJ)
      1.31 (2011-06-19) a few more leak fixes, bug in PNG handling (SpartanJ)
      1.30 (2011-06-11) added ability to load files via io callbacks (Ben Wenger)
      1.29 (2010-08-16) various warning fixes from Aurelien Pocheville 
      1.28 (2010-08-01) fix bug in GIF palette transparency (SpartanJ)

   See end of file for full revision history.

   TODO:
      stbi_info support for BMP,PSD,HDR,PIC


 ============================    Contributors    =========================
              
 Image formats                                Bug fixes & warning fixes
    Sean Barrett (jpeg, png, bmp)                Marc LeBlanc
    Nicolas Schulz (hdr, psd)                    Christpher Lloyd
    Jonathan Dummer (tga)                        Dave Moore
    Jean-Marc Lienher (gif)                      Won Chun
    Tom Seddon (pic)                             the Horde3D community
    Thatcher Ulrich (psd)                        Janez Zemva
                                                 Jonathan Blow
                                                 Laurent Gomila
 Extensions, features                            Aruelien Pocheville
    Jetro Lauha (stbi_info)                      Ryamond Barbiero
    James "moose2000" Brown (iPhone PNG)         David Woo
    Ben "Disch" Wenger (io callbacks)            Roy Eltham
    Martin "SpartanJ" Golini                     Luke Graham
                                                 Thomas Ruf
                                                 John Bartholomew
 Optimizations & bugfixes                        Ken Hamada
    Fabian "ryg" Giesen                          Cort Stratton
    Arseny Kapoulkine                            Blazej Dariusz Roszkowski
                                                 Thibault Reuille
 If your name should be here but                 Paul Du Bois
 isn't let Sean know.                            Guillaume George

*/

/* Modified by Alex Szpakowski to remove all decoding functionality except for
   TGA and BMP formats.
*/

#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STBI_INCLUDE_STB_IMAGE_H

#define STBI_NO_HDR
#define STBI_NO_STDIO

////   begin header file  ////////////////////////////////////////////////////
//
// Limitations:
//    - no jpeg progressive support
//    - non-HDR formats support 8-bit samples only (jpeg, png)
//    - no delayed line count (jpeg) -- IJG doesn't support either
//    - no 1-bit BMP
//    - GIF always returns *comp=4
//
// Basic usage (see HDR discussion below):
//    int x,y,n;
//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
//    // ... process data if not NULL ... 
//    // ... x = width, y = height, n = # 8-bit components per pixel ...
//    // ... replace '0' with '1'..'4' to force that many components per pixel
//    // ... but 'n' will always be the number that it would have been if you said 0
//    stbi_image_free(data)
//
// Standard parameters:
//    int *x       -- outputs image width in pixels
//    int *y       -- outputs image height in pixels
//    int *comp    -- outputs # of image components in image file
//    int req_comp -- if non-zero, # of image components requested in result
//
// The return value from an image loader is an 'unsigned char *' which points
// to the pixel data. The pixel data consists of *y scanlines of *x pixels,
// with each pixel consisting of N interleaved 8-bit components; the first
// pixel pointed to is top-left-most in the image. There is no padding between
// image scanlines or between pixels, regardless of format. The number of
// components N is 'req_comp' if req_comp is non-zero, or *comp otherwise.
// If req_comp is non-zero, *comp has the number of components that _would_
// have been output otherwise. E.g. if you set req_comp to 4, you will always
// get RGBA output, but you can check *comp to easily see if it's opaque.
//
// An output image with N components has the following components interleaved
// in this order in each pixel:
//
//     N=#comp     components
//       1           grey
//       2           grey, alpha
//       3           red, green, blue
//       4           red, green, blue, alpha
//
// If image loading fails for any reason, the return value will be NULL,
// and *x, *y, *comp will be unchanged. The function stbi_failure_reason()
// can be queried for an extremely brief, end-user unfriendly explanation
// of why the load failed. Define STBI_NO_FAILURE_STRINGS to avoid
// compiling these strings at all, and STBI_FAILURE_USERMSG to get slightly
// more user-friendly ones.
//
// Paletted PNG, BMP, GIF, and PIC images are automatically depalettized.
//
// ===========================================================================
//
// iPhone PNG support:
//
// By default we convert iphone-formatted PNGs back to RGB; nominally they
// would silently load as BGR, except the existing code should have just
// failed on such iPhone PNGs. But you can disable this conversion by
// by calling stbi_convert_iphone_png_to_rgb(0), in which case
// you will always just get the native iphone "format" through.
//
// Call stbi_set_unpremultiply_on_load(1) as well to force a divide per
// pixel to remove any premultiplied alpha *only* if the image file explicitly
// says there's premultiplied data (currently only happens in iPhone images,
// and only if iPhone convert-to-rgb processing is on).
//
// ===========================================================================
//
// HDR image support   (disable by defining STBI_NO_HDR)
//
// stb_image now supports loading HDR images in general, and currently
// the Radiance .HDR file format, although the support is provided
// generically. You can still load any file through the existing interface;
// if you attempt to load an HDR file, it will be automatically remapped to
// LDR, assuming gamma 2.2 and an arbitrary scale factor defaulting to 1;
// both of these constants can be reconfigured through this interface:
//
//     stbi_hdr_to_ldr_gamma(2.2f);
//     stbi_hdr_to_ldr_scale(1.0f);
//
// (note, do not use _inverse_ constants; stbi_image will invert them
// appropriately).
//
// Additionally, there is a new, parallel interface for loading files as
// (linear) floats to preserve the full dynamic range:
//
//    float *data = stbi_loadf(filename, &x, &y, &n, 0);
// 
// If you load LDR images through this interface, those images will
// be promoted to floating point values, run through the inverse of
// constants corresponding to the above:
//
//     stbi_ldr_to_hdr_scale(1.0f);
//     stbi_ldr_to_hdr_gamma(2.2f);
//
// Finally, given a filename (or an open file or memory block--see header
// file for details) containing image data, you can query for the "most
// appropriate" interface to use (that is, whether the image is HDR or
// not), using:
//
//     stbi_is_hdr(char *filename);
//
// ===========================================================================
//
// I/O callbacks
//
// I/O callbacks allow you to read from arbitrary sources, like packaged
// files or some other source. Data read from callbacks are processed
// through a small internal buffer (currently 128 bytes) to try to reduce
// overhead. 
//
// The three functions you must define are "read" (reads some bytes of data),
// "skip" (skips some bytes of data), "eof" (reports if the stream is at the end).


#ifndef STBI_NO_STDIO

#if defined(_MSC_VER) && _MSC_VER >= 1400
#define _CRT_SECURE_NO_WARNINGS // suppress warnings about fopen()
#pragma warning(push)
#pragma warning(disable:4996)   // suppress even more warnings about fopen()
#endif
#include <stdio.h>
#endif // STBI_NO_STDIO

#define STBI_VERSION 1

enum
{
   STBI_default = 0, // only used for req_comp

   STBI_grey       = 1,
   STBI_grey_alpha = 2,
   STBI_rgb        = 3,
   STBI_rgb_alpha  = 4
};

typedef unsigned char stbi_uc;

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////
//
// PRIMARY API - works on images of any type
//

//
// load image by filename, open file, or memory buffer
//

extern stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);

#ifndef STBI_NO_STDIO
extern stbi_uc *stbi_load            (char const *filename,     int *x, int *y, int *comp, int req_comp);
extern stbi_uc *stbi_load_from_file  (FILE *f,                  int *x, int *y, int *comp, int req_comp);
// for stbi_load_from_file, file pointer is left pointing immediately after image
#endif

typedef struct
{
   int      (*read)  (void *user,char *data,int size);   // fill 'data' with 'size' bytes.  return number of bytes actually read 
   void     (*skip)  (void *user,int n);                 // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
   int      (*eof)   (void *user);                       // returns nonzero if we are at end of file/data
} stbi_io_callbacks;

extern stbi_uc *stbi_load_from_callbacks  (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp);

#ifndef STBI_NO_HDR
   extern float *stbi_loadf_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);

   #ifndef STBI_NO_STDIO
   extern float *stbi_loadf            (char const *filename,   int *x, int *y, int *comp, int req_comp);
   extern float *stbi_loadf_from_file  (FILE *f,                int *x, int *y, int *comp, int req_comp);
   #endif
   
   extern float *stbi_loadf_from_callbacks  (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp);

   extern void   stbi_hdr_to_ldr_gamma(float gamma);
   extern void   stbi_hdr_to_ldr_scale(float scale);

   extern void   stbi_ldr_to_hdr_gamma(float gamma);
   extern void   stbi_ldr_to_hdr_scale(float scale);
#endif // STBI_NO_HDR

// stbi_is_hdr is always defined
extern int    stbi_is_hdr_from_callbacks(stbi_io_callbacks const *clbk, void *user);
extern int    stbi_is_hdr_from_memory(stbi_uc const *buffer, int len);
#ifndef STBI_NO_STDIO
extern int      stbi_is_hdr          (char const *filename);
extern int      stbi_is_hdr_from_file(FILE *f);
#endif // STBI_NO_STDIO


// get a VERY brief reason for failure
// NOT THREADSAFE
extern const char *stbi_failure_reason  (void); 

// free the loaded image -- this is just free()
extern void     stbi_image_free      (void *retval_from_stbi_load);

// get image dimensions & components without fully decoding
extern int      stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp);
extern int      stbi_info_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp);

#ifndef STBI_NO_STDIO
extern int      stbi_info            (char const *filename,     int *x, int *y, int *comp);
extern int      stbi_info_from_file  (FILE *f,                  int *x, int *y, int *comp);

#endif


#ifdef __cplusplus
}
#endif

#if !defined(STBI_NO_STDIO) && defined(_MSC_VER) && _MSC_VER >= 1400
#pragma warning(pop)
#endif

//
//
////   end header file   /////////////////////////////////////////////////////
#endif // STBI_INCLUDE_STB_IMAGE_H
