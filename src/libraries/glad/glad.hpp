
/**
 * The MIT License (MIT)
 * 
 * Copyright (c) 2013 David Herberth, modified by Sasha Szpakowski
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **/
 

#ifndef __glad_hpp_

#if defined(__gl_h_) || defined(__glext_h_) || defined(__glcorearb_h_) \
    || defined(__gl3_h) || defined(__gl3_ext_h)
#error OpenGL header already included, remove this include, glad already provides it
#endif

#define __glad_hpp_
#define __gl_h_

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

#include <windows.h>

#ifdef near
#undef near
#endif

#ifdef far
#undef far
#endif

#endif

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
#ifndef GLAPI
#define GLAPI extern
#endif

#include <stddef.h>

#ifndef GLEXT_64_TYPES_DEFINED
/* This code block is duplicated in glxext.h, so must be protected */
#define GLEXT_64_TYPES_DEFINED
/* Define int32_t, int64_t, and uint64_t types for UST/MSC */
/* (as used in the GL_EXT_timer_query extension). */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <inttypes.h>
#elif defined(__sun__) || defined(__digital__)
#include <inttypes.h>
#if defined(__STDC__)
#if defined(__arch64__) || defined(_LP64)
typedef long int int64_t;
typedef unsigned long int uint64_t;
#else
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
#endif /* __arch64__ */
#endif /* __STDC__ */
#elif defined( __VMS ) || defined(__sgi)
#include <inttypes.h>
#elif defined(__SCO__) || defined(__USLC__)
#include <stdint.h>
#elif defined(__UNIXOS2__) || defined(__SOL64__)
typedef long int int32_t;
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
#elif defined(_WIN32) && defined(__GNUC__)
#include <stdint.h>
#elif defined(_WIN32)
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
/* Fallback if nothing above works */
#include <inttypes.h>
#endif
#endif

namespace glad {

bool gladLoadGL(void);

typedef void* (* LOADER)(const char *name);
bool gladLoadGLLoader(LOADER);

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLclampx;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef int GLsizei;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void *GLeglImageOES;
typedef char GLchar;
typedef char GLcharARB;
#ifdef __APPLE__
typedef void *GLhandleARB;
#else
typedef unsigned int GLhandleARB;
#endif
typedef unsigned short GLhalfARB;
typedef unsigned short GLhalf;
typedef GLint GLfixed;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;
typedef int64_t GLint64;
typedef uint64_t GLuint64;
typedef ptrdiff_t GLintptrARB;
typedef ptrdiff_t GLsizeiptrARB;
typedef int64_t GLint64EXT;
typedef uint64_t GLuint64EXT;
typedef struct __GLsync *GLsync;
struct _cl_context;
struct _cl_event;
typedef void (APIENTRY *GLDEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
typedef void (APIENTRY *GLDEBUGPROCARB)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
typedef void (APIENTRY *GLDEBUGPROCKHR)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
typedef void (APIENTRY *GLDEBUGPROCAMD)(GLuint id,GLenum category,GLenum severity,GLsizei length,const GLchar *message,void *userParam);
typedef unsigned short GLhalfNV;
typedef GLintptr GLvdpauSurfaceNV;

 /* GL_VERSION_1_0 */
extern GLboolean GLAD_VERSION_1_0;
typedef void (APIENTRYP pfn_glCullFace) (GLenum);
extern pfn_glCullFace fp_glCullFace;
typedef void (APIENTRYP pfn_glFrontFace) (GLenum);
extern pfn_glFrontFace fp_glFrontFace;
typedef void (APIENTRYP pfn_glHint) (GLenum, GLenum);
extern pfn_glHint fp_glHint;
typedef void (APIENTRYP pfn_glLineWidth) (GLfloat);
extern pfn_glLineWidth fp_glLineWidth;
typedef void (APIENTRYP pfn_glPointSize) (GLfloat);
extern pfn_glPointSize fp_glPointSize;
typedef void (APIENTRYP pfn_glPolygonMode) (GLenum, GLenum);
extern pfn_glPolygonMode fp_glPolygonMode;
typedef void (APIENTRYP pfn_glScissor) (GLint, GLint, GLsizei, GLsizei);
extern pfn_glScissor fp_glScissor;
typedef void (APIENTRYP pfn_glTexParameterf) (GLenum, GLenum, GLfloat);
extern pfn_glTexParameterf fp_glTexParameterf;
typedef void (APIENTRYP pfn_glTexParameterfv) (GLenum, GLenum, const GLfloat*);
extern pfn_glTexParameterfv fp_glTexParameterfv;
typedef void (APIENTRYP pfn_glTexParameteri) (GLenum, GLenum, GLint);
extern pfn_glTexParameteri fp_glTexParameteri;
typedef void (APIENTRYP pfn_glTexParameteriv) (GLenum, GLenum, const GLint*);
extern pfn_glTexParameteriv fp_glTexParameteriv;
typedef void (APIENTRYP pfn_glTexImage1D) (GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const void*);
extern pfn_glTexImage1D fp_glTexImage1D;
typedef void (APIENTRYP pfn_glTexImage2D) (GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*);
extern pfn_glTexImage2D fp_glTexImage2D;
typedef void (APIENTRYP pfn_glDrawBuffer) (GLenum);
extern pfn_glDrawBuffer fp_glDrawBuffer;
typedef void (APIENTRYP pfn_glClear) (GLbitfield);
extern pfn_glClear fp_glClear;
typedef void (APIENTRYP pfn_glClearColor) (GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glClearColor fp_glClearColor;
typedef void (APIENTRYP pfn_glClearStencil) (GLint);
extern pfn_glClearStencil fp_glClearStencil;
typedef void (APIENTRYP pfn_glClearDepth) (GLdouble);
extern pfn_glClearDepth fp_glClearDepth;
typedef void (APIENTRYP pfn_glStencilMask) (GLuint);
extern pfn_glStencilMask fp_glStencilMask;
typedef void (APIENTRYP pfn_glColorMask) (GLboolean, GLboolean, GLboolean, GLboolean);
extern pfn_glColorMask fp_glColorMask;
typedef void (APIENTRYP pfn_glDepthMask) (GLboolean);
extern pfn_glDepthMask fp_glDepthMask;
typedef void (APIENTRYP pfn_glDisable) (GLenum);
extern pfn_glDisable fp_glDisable;
typedef void (APIENTRYP pfn_glEnable) (GLenum);
extern pfn_glEnable fp_glEnable;
typedef void (APIENTRYP pfn_glFinish) ();
extern pfn_glFinish fp_glFinish;
typedef void (APIENTRYP pfn_glFlush) ();
extern pfn_glFlush fp_glFlush;
typedef void (APIENTRYP pfn_glBlendFunc) (GLenum, GLenum);
extern pfn_glBlendFunc fp_glBlendFunc;
typedef void (APIENTRYP pfn_glLogicOp) (GLenum);
extern pfn_glLogicOp fp_glLogicOp;
typedef void (APIENTRYP pfn_glStencilFunc) (GLenum, GLint, GLuint);
extern pfn_glStencilFunc fp_glStencilFunc;
typedef void (APIENTRYP pfn_glStencilOp) (GLenum, GLenum, GLenum);
extern pfn_glStencilOp fp_glStencilOp;
typedef void (APIENTRYP pfn_glDepthFunc) (GLenum);
extern pfn_glDepthFunc fp_glDepthFunc;
typedef void (APIENTRYP pfn_glPixelStoref) (GLenum, GLfloat);
extern pfn_glPixelStoref fp_glPixelStoref;
typedef void (APIENTRYP pfn_glPixelStorei) (GLenum, GLint);
extern pfn_glPixelStorei fp_glPixelStorei;
typedef void (APIENTRYP pfn_glReadBuffer) (GLenum);
extern pfn_glReadBuffer fp_glReadBuffer;
typedef void (APIENTRYP pfn_glReadPixels) (GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void*);
extern pfn_glReadPixels fp_glReadPixels;
typedef void (APIENTRYP pfn_glGetBooleanv) (GLenum, GLboolean*);
extern pfn_glGetBooleanv fp_glGetBooleanv;
typedef void (APIENTRYP pfn_glGetDoublev) (GLenum, GLdouble*);
extern pfn_glGetDoublev fp_glGetDoublev;
typedef GLenum (APIENTRYP pfn_glGetError) ();
extern pfn_glGetError fp_glGetError;
typedef void (APIENTRYP pfn_glGetFloatv) (GLenum, GLfloat*);
extern pfn_glGetFloatv fp_glGetFloatv;
typedef void (APIENTRYP pfn_glGetIntegerv) (GLenum, GLint*);
extern pfn_glGetIntegerv fp_glGetIntegerv;
typedef const GLubyte* (APIENTRYP pfn_glGetString) (GLenum);
extern pfn_glGetString fp_glGetString;
typedef void (APIENTRYP pfn_glGetTexImage) (GLenum, GLint, GLenum, GLenum, void*);
extern pfn_glGetTexImage fp_glGetTexImage;
typedef void (APIENTRYP pfn_glGetTexParameterfv) (GLenum, GLenum, GLfloat*);
extern pfn_glGetTexParameterfv fp_glGetTexParameterfv;
typedef void (APIENTRYP pfn_glGetTexParameteriv) (GLenum, GLenum, GLint*);
extern pfn_glGetTexParameteriv fp_glGetTexParameteriv;
typedef void (APIENTRYP pfn_glGetTexLevelParameterfv) (GLenum, GLint, GLenum, GLfloat*);
extern pfn_glGetTexLevelParameterfv fp_glGetTexLevelParameterfv;
typedef void (APIENTRYP pfn_glGetTexLevelParameteriv) (GLenum, GLint, GLenum, GLint*);
extern pfn_glGetTexLevelParameteriv fp_glGetTexLevelParameteriv;
typedef GLboolean (APIENTRYP pfn_glIsEnabled) (GLenum);
extern pfn_glIsEnabled fp_glIsEnabled;
typedef void (APIENTRYP pfn_glDepthRange) (GLdouble, GLdouble);
extern pfn_glDepthRange fp_glDepthRange;
typedef void (APIENTRYP pfn_glViewport) (GLint, GLint, GLsizei, GLsizei);
extern pfn_glViewport fp_glViewport;
typedef void (APIENTRYP pfn_glNewList) (GLuint, GLenum);
extern pfn_glNewList fp_glNewList;
typedef void (APIENTRYP pfn_glEndList) ();
extern pfn_glEndList fp_glEndList;
typedef void (APIENTRYP pfn_glCallList) (GLuint);
extern pfn_glCallList fp_glCallList;
typedef void (APIENTRYP pfn_glCallLists) (GLsizei, GLenum, const void*);
extern pfn_glCallLists fp_glCallLists;
typedef void (APIENTRYP pfn_glDeleteLists) (GLuint, GLsizei);
extern pfn_glDeleteLists fp_glDeleteLists;
typedef GLuint (APIENTRYP pfn_glGenLists) (GLsizei);
extern pfn_glGenLists fp_glGenLists;
typedef void (APIENTRYP pfn_glListBase) (GLuint);
extern pfn_glListBase fp_glListBase;
typedef void (APIENTRYP pfn_glBegin) (GLenum);
extern pfn_glBegin fp_glBegin;
typedef void (APIENTRYP pfn_glBitmap) (GLsizei, GLsizei, GLfloat, GLfloat, GLfloat, GLfloat, const GLubyte*);
extern pfn_glBitmap fp_glBitmap;
typedef void (APIENTRYP pfn_glColor3b) (GLbyte, GLbyte, GLbyte);
extern pfn_glColor3b fp_glColor3b;
typedef void (APIENTRYP pfn_glColor3bv) (const GLbyte*);
extern pfn_glColor3bv fp_glColor3bv;
typedef void (APIENTRYP pfn_glColor3d) (GLdouble, GLdouble, GLdouble);
extern pfn_glColor3d fp_glColor3d;
typedef void (APIENTRYP pfn_glColor3dv) (const GLdouble*);
extern pfn_glColor3dv fp_glColor3dv;
typedef void (APIENTRYP pfn_glColor3f) (GLfloat, GLfloat, GLfloat);
extern pfn_glColor3f fp_glColor3f;
typedef void (APIENTRYP pfn_glColor3fv) (const GLfloat*);
extern pfn_glColor3fv fp_glColor3fv;
typedef void (APIENTRYP pfn_glColor3i) (GLint, GLint, GLint);
extern pfn_glColor3i fp_glColor3i;
typedef void (APIENTRYP pfn_glColor3iv) (const GLint*);
extern pfn_glColor3iv fp_glColor3iv;
typedef void (APIENTRYP pfn_glColor3s) (GLshort, GLshort, GLshort);
extern pfn_glColor3s fp_glColor3s;
typedef void (APIENTRYP pfn_glColor3sv) (const GLshort*);
extern pfn_glColor3sv fp_glColor3sv;
typedef void (APIENTRYP pfn_glColor3ub) (GLubyte, GLubyte, GLubyte);
extern pfn_glColor3ub fp_glColor3ub;
typedef void (APIENTRYP pfn_glColor3ubv) (const GLubyte*);
extern pfn_glColor3ubv fp_glColor3ubv;
typedef void (APIENTRYP pfn_glColor3ui) (GLuint, GLuint, GLuint);
extern pfn_glColor3ui fp_glColor3ui;
typedef void (APIENTRYP pfn_glColor3uiv) (const GLuint*);
extern pfn_glColor3uiv fp_glColor3uiv;
typedef void (APIENTRYP pfn_glColor3us) (GLushort, GLushort, GLushort);
extern pfn_glColor3us fp_glColor3us;
typedef void (APIENTRYP pfn_glColor3usv) (const GLushort*);
extern pfn_glColor3usv fp_glColor3usv;
typedef void (APIENTRYP pfn_glColor4b) (GLbyte, GLbyte, GLbyte, GLbyte);
extern pfn_glColor4b fp_glColor4b;
typedef void (APIENTRYP pfn_glColor4bv) (const GLbyte*);
extern pfn_glColor4bv fp_glColor4bv;
typedef void (APIENTRYP pfn_glColor4d) (GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glColor4d fp_glColor4d;
typedef void (APIENTRYP pfn_glColor4dv) (const GLdouble*);
extern pfn_glColor4dv fp_glColor4dv;
typedef void (APIENTRYP pfn_glColor4f) (GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glColor4f fp_glColor4f;
typedef void (APIENTRYP pfn_glColor4fv) (const GLfloat*);
extern pfn_glColor4fv fp_glColor4fv;
typedef void (APIENTRYP pfn_glColor4i) (GLint, GLint, GLint, GLint);
extern pfn_glColor4i fp_glColor4i;
typedef void (APIENTRYP pfn_glColor4iv) (const GLint*);
extern pfn_glColor4iv fp_glColor4iv;
typedef void (APIENTRYP pfn_glColor4s) (GLshort, GLshort, GLshort, GLshort);
extern pfn_glColor4s fp_glColor4s;
typedef void (APIENTRYP pfn_glColor4sv) (const GLshort*);
extern pfn_glColor4sv fp_glColor4sv;
typedef void (APIENTRYP pfn_glColor4ub) (GLubyte, GLubyte, GLubyte, GLubyte);
extern pfn_glColor4ub fp_glColor4ub;
typedef void (APIENTRYP pfn_glColor4ubv) (const GLubyte*);
extern pfn_glColor4ubv fp_glColor4ubv;
typedef void (APIENTRYP pfn_glColor4ui) (GLuint, GLuint, GLuint, GLuint);
extern pfn_glColor4ui fp_glColor4ui;
typedef void (APIENTRYP pfn_glColor4uiv) (const GLuint*);
extern pfn_glColor4uiv fp_glColor4uiv;
typedef void (APIENTRYP pfn_glColor4us) (GLushort, GLushort, GLushort, GLushort);
extern pfn_glColor4us fp_glColor4us;
typedef void (APIENTRYP pfn_glColor4usv) (const GLushort*);
extern pfn_glColor4usv fp_glColor4usv;
typedef void (APIENTRYP pfn_glEdgeFlag) (GLboolean);
extern pfn_glEdgeFlag fp_glEdgeFlag;
typedef void (APIENTRYP pfn_glEdgeFlagv) (const GLboolean*);
extern pfn_glEdgeFlagv fp_glEdgeFlagv;
typedef void (APIENTRYP pfn_glEnd) ();
extern pfn_glEnd fp_glEnd;
typedef void (APIENTRYP pfn_glIndexd) (GLdouble);
extern pfn_glIndexd fp_glIndexd;
typedef void (APIENTRYP pfn_glIndexdv) (const GLdouble*);
extern pfn_glIndexdv fp_glIndexdv;
typedef void (APIENTRYP pfn_glIndexf) (GLfloat);
extern pfn_glIndexf fp_glIndexf;
typedef void (APIENTRYP pfn_glIndexfv) (const GLfloat*);
extern pfn_glIndexfv fp_glIndexfv;
typedef void (APIENTRYP pfn_glIndexi) (GLint);
extern pfn_glIndexi fp_glIndexi;
typedef void (APIENTRYP pfn_glIndexiv) (const GLint*);
extern pfn_glIndexiv fp_glIndexiv;
typedef void (APIENTRYP pfn_glIndexs) (GLshort);
extern pfn_glIndexs fp_glIndexs;
typedef void (APIENTRYP pfn_glIndexsv) (const GLshort*);
extern pfn_glIndexsv fp_glIndexsv;
typedef void (APIENTRYP pfn_glNormal3b) (GLbyte, GLbyte, GLbyte);
extern pfn_glNormal3b fp_glNormal3b;
typedef void (APIENTRYP pfn_glNormal3bv) (const GLbyte*);
extern pfn_glNormal3bv fp_glNormal3bv;
typedef void (APIENTRYP pfn_glNormal3d) (GLdouble, GLdouble, GLdouble);
extern pfn_glNormal3d fp_glNormal3d;
typedef void (APIENTRYP pfn_glNormal3dv) (const GLdouble*);
extern pfn_glNormal3dv fp_glNormal3dv;
typedef void (APIENTRYP pfn_glNormal3f) (GLfloat, GLfloat, GLfloat);
extern pfn_glNormal3f fp_glNormal3f;
typedef void (APIENTRYP pfn_glNormal3fv) (const GLfloat*);
extern pfn_glNormal3fv fp_glNormal3fv;
typedef void (APIENTRYP pfn_glNormal3i) (GLint, GLint, GLint);
extern pfn_glNormal3i fp_glNormal3i;
typedef void (APIENTRYP pfn_glNormal3iv) (const GLint*);
extern pfn_glNormal3iv fp_glNormal3iv;
typedef void (APIENTRYP pfn_glNormal3s) (GLshort, GLshort, GLshort);
extern pfn_glNormal3s fp_glNormal3s;
typedef void (APIENTRYP pfn_glNormal3sv) (const GLshort*);
extern pfn_glNormal3sv fp_glNormal3sv;
typedef void (APIENTRYP pfn_glRasterPos2d) (GLdouble, GLdouble);
extern pfn_glRasterPos2d fp_glRasterPos2d;
typedef void (APIENTRYP pfn_glRasterPos2dv) (const GLdouble*);
extern pfn_glRasterPos2dv fp_glRasterPos2dv;
typedef void (APIENTRYP pfn_glRasterPos2f) (GLfloat, GLfloat);
extern pfn_glRasterPos2f fp_glRasterPos2f;
typedef void (APIENTRYP pfn_glRasterPos2fv) (const GLfloat*);
extern pfn_glRasterPos2fv fp_glRasterPos2fv;
typedef void (APIENTRYP pfn_glRasterPos2i) (GLint, GLint);
extern pfn_glRasterPos2i fp_glRasterPos2i;
typedef void (APIENTRYP pfn_glRasterPos2iv) (const GLint*);
extern pfn_glRasterPos2iv fp_glRasterPos2iv;
typedef void (APIENTRYP pfn_glRasterPos2s) (GLshort, GLshort);
extern pfn_glRasterPos2s fp_glRasterPos2s;
typedef void (APIENTRYP pfn_glRasterPos2sv) (const GLshort*);
extern pfn_glRasterPos2sv fp_glRasterPos2sv;
typedef void (APIENTRYP pfn_glRasterPos3d) (GLdouble, GLdouble, GLdouble);
extern pfn_glRasterPos3d fp_glRasterPos3d;
typedef void (APIENTRYP pfn_glRasterPos3dv) (const GLdouble*);
extern pfn_glRasterPos3dv fp_glRasterPos3dv;
typedef void (APIENTRYP pfn_glRasterPos3f) (GLfloat, GLfloat, GLfloat);
extern pfn_glRasterPos3f fp_glRasterPos3f;
typedef void (APIENTRYP pfn_glRasterPos3fv) (const GLfloat*);
extern pfn_glRasterPos3fv fp_glRasterPos3fv;
typedef void (APIENTRYP pfn_glRasterPos3i) (GLint, GLint, GLint);
extern pfn_glRasterPos3i fp_glRasterPos3i;
typedef void (APIENTRYP pfn_glRasterPos3iv) (const GLint*);
extern pfn_glRasterPos3iv fp_glRasterPos3iv;
typedef void (APIENTRYP pfn_glRasterPos3s) (GLshort, GLshort, GLshort);
extern pfn_glRasterPos3s fp_glRasterPos3s;
typedef void (APIENTRYP pfn_glRasterPos3sv) (const GLshort*);
extern pfn_glRasterPos3sv fp_glRasterPos3sv;
typedef void (APIENTRYP pfn_glRasterPos4d) (GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glRasterPos4d fp_glRasterPos4d;
typedef void (APIENTRYP pfn_glRasterPos4dv) (const GLdouble*);
extern pfn_glRasterPos4dv fp_glRasterPos4dv;
typedef void (APIENTRYP pfn_glRasterPos4f) (GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glRasterPos4f fp_glRasterPos4f;
typedef void (APIENTRYP pfn_glRasterPos4fv) (const GLfloat*);
extern pfn_glRasterPos4fv fp_glRasterPos4fv;
typedef void (APIENTRYP pfn_glRasterPos4i) (GLint, GLint, GLint, GLint);
extern pfn_glRasterPos4i fp_glRasterPos4i;
typedef void (APIENTRYP pfn_glRasterPos4iv) (const GLint*);
extern pfn_glRasterPos4iv fp_glRasterPos4iv;
typedef void (APIENTRYP pfn_glRasterPos4s) (GLshort, GLshort, GLshort, GLshort);
extern pfn_glRasterPos4s fp_glRasterPos4s;
typedef void (APIENTRYP pfn_glRasterPos4sv) (const GLshort*);
extern pfn_glRasterPos4sv fp_glRasterPos4sv;
typedef void (APIENTRYP pfn_glRectd) (GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glRectd fp_glRectd;
typedef void (APIENTRYP pfn_glRectdv) (const GLdouble*, const GLdouble*);
extern pfn_glRectdv fp_glRectdv;
typedef void (APIENTRYP pfn_glRectf) (GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glRectf fp_glRectf;
typedef void (APIENTRYP pfn_glRectfv) (const GLfloat*, const GLfloat*);
extern pfn_glRectfv fp_glRectfv;
typedef void (APIENTRYP pfn_glRecti) (GLint, GLint, GLint, GLint);
extern pfn_glRecti fp_glRecti;
typedef void (APIENTRYP pfn_glRectiv) (const GLint*, const GLint*);
extern pfn_glRectiv fp_glRectiv;
typedef void (APIENTRYP pfn_glRects) (GLshort, GLshort, GLshort, GLshort);
extern pfn_glRects fp_glRects;
typedef void (APIENTRYP pfn_glRectsv) (const GLshort*, const GLshort*);
extern pfn_glRectsv fp_glRectsv;
typedef void (APIENTRYP pfn_glTexCoord1d) (GLdouble);
extern pfn_glTexCoord1d fp_glTexCoord1d;
typedef void (APIENTRYP pfn_glTexCoord1dv) (const GLdouble*);
extern pfn_glTexCoord1dv fp_glTexCoord1dv;
typedef void (APIENTRYP pfn_glTexCoord1f) (GLfloat);
extern pfn_glTexCoord1f fp_glTexCoord1f;
typedef void (APIENTRYP pfn_glTexCoord1fv) (const GLfloat*);
extern pfn_glTexCoord1fv fp_glTexCoord1fv;
typedef void (APIENTRYP pfn_glTexCoord1i) (GLint);
extern pfn_glTexCoord1i fp_glTexCoord1i;
typedef void (APIENTRYP pfn_glTexCoord1iv) (const GLint*);
extern pfn_glTexCoord1iv fp_glTexCoord1iv;
typedef void (APIENTRYP pfn_glTexCoord1s) (GLshort);
extern pfn_glTexCoord1s fp_glTexCoord1s;
typedef void (APIENTRYP pfn_glTexCoord1sv) (const GLshort*);
extern pfn_glTexCoord1sv fp_glTexCoord1sv;
typedef void (APIENTRYP pfn_glTexCoord2d) (GLdouble, GLdouble);
extern pfn_glTexCoord2d fp_glTexCoord2d;
typedef void (APIENTRYP pfn_glTexCoord2dv) (const GLdouble*);
extern pfn_glTexCoord2dv fp_glTexCoord2dv;
typedef void (APIENTRYP pfn_glTexCoord2f) (GLfloat, GLfloat);
extern pfn_glTexCoord2f fp_glTexCoord2f;
typedef void (APIENTRYP pfn_glTexCoord2fv) (const GLfloat*);
extern pfn_glTexCoord2fv fp_glTexCoord2fv;
typedef void (APIENTRYP pfn_glTexCoord2i) (GLint, GLint);
extern pfn_glTexCoord2i fp_glTexCoord2i;
typedef void (APIENTRYP pfn_glTexCoord2iv) (const GLint*);
extern pfn_glTexCoord2iv fp_glTexCoord2iv;
typedef void (APIENTRYP pfn_glTexCoord2s) (GLshort, GLshort);
extern pfn_glTexCoord2s fp_glTexCoord2s;
typedef void (APIENTRYP pfn_glTexCoord2sv) (const GLshort*);
extern pfn_glTexCoord2sv fp_glTexCoord2sv;
typedef void (APIENTRYP pfn_glTexCoord3d) (GLdouble, GLdouble, GLdouble);
extern pfn_glTexCoord3d fp_glTexCoord3d;
typedef void (APIENTRYP pfn_glTexCoord3dv) (const GLdouble*);
extern pfn_glTexCoord3dv fp_glTexCoord3dv;
typedef void (APIENTRYP pfn_glTexCoord3f) (GLfloat, GLfloat, GLfloat);
extern pfn_glTexCoord3f fp_glTexCoord3f;
typedef void (APIENTRYP pfn_glTexCoord3fv) (const GLfloat*);
extern pfn_glTexCoord3fv fp_glTexCoord3fv;
typedef void (APIENTRYP pfn_glTexCoord3i) (GLint, GLint, GLint);
extern pfn_glTexCoord3i fp_glTexCoord3i;
typedef void (APIENTRYP pfn_glTexCoord3iv) (const GLint*);
extern pfn_glTexCoord3iv fp_glTexCoord3iv;
typedef void (APIENTRYP pfn_glTexCoord3s) (GLshort, GLshort, GLshort);
extern pfn_glTexCoord3s fp_glTexCoord3s;
typedef void (APIENTRYP pfn_glTexCoord3sv) (const GLshort*);
extern pfn_glTexCoord3sv fp_glTexCoord3sv;
typedef void (APIENTRYP pfn_glTexCoord4d) (GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glTexCoord4d fp_glTexCoord4d;
typedef void (APIENTRYP pfn_glTexCoord4dv) (const GLdouble*);
extern pfn_glTexCoord4dv fp_glTexCoord4dv;
typedef void (APIENTRYP pfn_glTexCoord4f) (GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glTexCoord4f fp_glTexCoord4f;
typedef void (APIENTRYP pfn_glTexCoord4fv) (const GLfloat*);
extern pfn_glTexCoord4fv fp_glTexCoord4fv;
typedef void (APIENTRYP pfn_glTexCoord4i) (GLint, GLint, GLint, GLint);
extern pfn_glTexCoord4i fp_glTexCoord4i;
typedef void (APIENTRYP pfn_glTexCoord4iv) (const GLint*);
extern pfn_glTexCoord4iv fp_glTexCoord4iv;
typedef void (APIENTRYP pfn_glTexCoord4s) (GLshort, GLshort, GLshort, GLshort);
extern pfn_glTexCoord4s fp_glTexCoord4s;
typedef void (APIENTRYP pfn_glTexCoord4sv) (const GLshort*);
extern pfn_glTexCoord4sv fp_glTexCoord4sv;
typedef void (APIENTRYP pfn_glVertex2d) (GLdouble, GLdouble);
extern pfn_glVertex2d fp_glVertex2d;
typedef void (APIENTRYP pfn_glVertex2dv) (const GLdouble*);
extern pfn_glVertex2dv fp_glVertex2dv;
typedef void (APIENTRYP pfn_glVertex2f) (GLfloat, GLfloat);
extern pfn_glVertex2f fp_glVertex2f;
typedef void (APIENTRYP pfn_glVertex2fv) (const GLfloat*);
extern pfn_glVertex2fv fp_glVertex2fv;
typedef void (APIENTRYP pfn_glVertex2i) (GLint, GLint);
extern pfn_glVertex2i fp_glVertex2i;
typedef void (APIENTRYP pfn_glVertex2iv) (const GLint*);
extern pfn_glVertex2iv fp_glVertex2iv;
typedef void (APIENTRYP pfn_glVertex2s) (GLshort, GLshort);
extern pfn_glVertex2s fp_glVertex2s;
typedef void (APIENTRYP pfn_glVertex2sv) (const GLshort*);
extern pfn_glVertex2sv fp_glVertex2sv;
typedef void (APIENTRYP pfn_glVertex3d) (GLdouble, GLdouble, GLdouble);
extern pfn_glVertex3d fp_glVertex3d;
typedef void (APIENTRYP pfn_glVertex3dv) (const GLdouble*);
extern pfn_glVertex3dv fp_glVertex3dv;
typedef void (APIENTRYP pfn_glVertex3f) (GLfloat, GLfloat, GLfloat);
extern pfn_glVertex3f fp_glVertex3f;
typedef void (APIENTRYP pfn_glVertex3fv) (const GLfloat*);
extern pfn_glVertex3fv fp_glVertex3fv;
typedef void (APIENTRYP pfn_glVertex3i) (GLint, GLint, GLint);
extern pfn_glVertex3i fp_glVertex3i;
typedef void (APIENTRYP pfn_glVertex3iv) (const GLint*);
extern pfn_glVertex3iv fp_glVertex3iv;
typedef void (APIENTRYP pfn_glVertex3s) (GLshort, GLshort, GLshort);
extern pfn_glVertex3s fp_glVertex3s;
typedef void (APIENTRYP pfn_glVertex3sv) (const GLshort*);
extern pfn_glVertex3sv fp_glVertex3sv;
typedef void (APIENTRYP pfn_glVertex4d) (GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glVertex4d fp_glVertex4d;
typedef void (APIENTRYP pfn_glVertex4dv) (const GLdouble*);
extern pfn_glVertex4dv fp_glVertex4dv;
typedef void (APIENTRYP pfn_glVertex4f) (GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glVertex4f fp_glVertex4f;
typedef void (APIENTRYP pfn_glVertex4fv) (const GLfloat*);
extern pfn_glVertex4fv fp_glVertex4fv;
typedef void (APIENTRYP pfn_glVertex4i) (GLint, GLint, GLint, GLint);
extern pfn_glVertex4i fp_glVertex4i;
typedef void (APIENTRYP pfn_glVertex4iv) (const GLint*);
extern pfn_glVertex4iv fp_glVertex4iv;
typedef void (APIENTRYP pfn_glVertex4s) (GLshort, GLshort, GLshort, GLshort);
extern pfn_glVertex4s fp_glVertex4s;
typedef void (APIENTRYP pfn_glVertex4sv) (const GLshort*);
extern pfn_glVertex4sv fp_glVertex4sv;
typedef void (APIENTRYP pfn_glClipPlane) (GLenum, const GLdouble*);
extern pfn_glClipPlane fp_glClipPlane;
typedef void (APIENTRYP pfn_glColorMaterial) (GLenum, GLenum);
extern pfn_glColorMaterial fp_glColorMaterial;
typedef void (APIENTRYP pfn_glFogf) (GLenum, GLfloat);
extern pfn_glFogf fp_glFogf;
typedef void (APIENTRYP pfn_glFogfv) (GLenum, const GLfloat*);
extern pfn_glFogfv fp_glFogfv;
typedef void (APIENTRYP pfn_glFogi) (GLenum, GLint);
extern pfn_glFogi fp_glFogi;
typedef void (APIENTRYP pfn_glFogiv) (GLenum, const GLint*);
extern pfn_glFogiv fp_glFogiv;
typedef void (APIENTRYP pfn_glLightf) (GLenum, GLenum, GLfloat);
extern pfn_glLightf fp_glLightf;
typedef void (APIENTRYP pfn_glLightfv) (GLenum, GLenum, const GLfloat*);
extern pfn_glLightfv fp_glLightfv;
typedef void (APIENTRYP pfn_glLighti) (GLenum, GLenum, GLint);
extern pfn_glLighti fp_glLighti;
typedef void (APIENTRYP pfn_glLightiv) (GLenum, GLenum, const GLint*);
extern pfn_glLightiv fp_glLightiv;
typedef void (APIENTRYP pfn_glLightModelf) (GLenum, GLfloat);
extern pfn_glLightModelf fp_glLightModelf;
typedef void (APIENTRYP pfn_glLightModelfv) (GLenum, const GLfloat*);
extern pfn_glLightModelfv fp_glLightModelfv;
typedef void (APIENTRYP pfn_glLightModeli) (GLenum, GLint);
extern pfn_glLightModeli fp_glLightModeli;
typedef void (APIENTRYP pfn_glLightModeliv) (GLenum, const GLint*);
extern pfn_glLightModeliv fp_glLightModeliv;
typedef void (APIENTRYP pfn_glLineStipple) (GLint, GLushort);
extern pfn_glLineStipple fp_glLineStipple;
typedef void (APIENTRYP pfn_glMaterialf) (GLenum, GLenum, GLfloat);
extern pfn_glMaterialf fp_glMaterialf;
typedef void (APIENTRYP pfn_glMaterialfv) (GLenum, GLenum, const GLfloat*);
extern pfn_glMaterialfv fp_glMaterialfv;
typedef void (APIENTRYP pfn_glMateriali) (GLenum, GLenum, GLint);
extern pfn_glMateriali fp_glMateriali;
typedef void (APIENTRYP pfn_glMaterialiv) (GLenum, GLenum, const GLint*);
extern pfn_glMaterialiv fp_glMaterialiv;
typedef void (APIENTRYP pfn_glPolygonStipple) (const GLubyte*);
extern pfn_glPolygonStipple fp_glPolygonStipple;
typedef void (APIENTRYP pfn_glShadeModel) (GLenum);
extern pfn_glShadeModel fp_glShadeModel;
typedef void (APIENTRYP pfn_glTexEnvf) (GLenum, GLenum, GLfloat);
extern pfn_glTexEnvf fp_glTexEnvf;
typedef void (APIENTRYP pfn_glTexEnvfv) (GLenum, GLenum, const GLfloat*);
extern pfn_glTexEnvfv fp_glTexEnvfv;
typedef void (APIENTRYP pfn_glTexEnvi) (GLenum, GLenum, GLint);
extern pfn_glTexEnvi fp_glTexEnvi;
typedef void (APIENTRYP pfn_glTexEnviv) (GLenum, GLenum, const GLint*);
extern pfn_glTexEnviv fp_glTexEnviv;
typedef void (APIENTRYP pfn_glTexGend) (GLenum, GLenum, GLdouble);
extern pfn_glTexGend fp_glTexGend;
typedef void (APIENTRYP pfn_glTexGendv) (GLenum, GLenum, const GLdouble*);
extern pfn_glTexGendv fp_glTexGendv;
typedef void (APIENTRYP pfn_glTexGenf) (GLenum, GLenum, GLfloat);
extern pfn_glTexGenf fp_glTexGenf;
typedef void (APIENTRYP pfn_glTexGenfv) (GLenum, GLenum, const GLfloat*);
extern pfn_glTexGenfv fp_glTexGenfv;
typedef void (APIENTRYP pfn_glTexGeni) (GLenum, GLenum, GLint);
extern pfn_glTexGeni fp_glTexGeni;
typedef void (APIENTRYP pfn_glTexGeniv) (GLenum, GLenum, const GLint*);
extern pfn_glTexGeniv fp_glTexGeniv;
typedef void (APIENTRYP pfn_glFeedbackBuffer) (GLsizei, GLenum, GLfloat*);
extern pfn_glFeedbackBuffer fp_glFeedbackBuffer;
typedef void (APIENTRYP pfn_glSelectBuffer) (GLsizei, GLuint*);
extern pfn_glSelectBuffer fp_glSelectBuffer;
typedef GLint (APIENTRYP pfn_glRenderMode) (GLenum);
extern pfn_glRenderMode fp_glRenderMode;
typedef void (APIENTRYP pfn_glInitNames) ();
extern pfn_glInitNames fp_glInitNames;
typedef void (APIENTRYP pfn_glLoadName) (GLuint);
extern pfn_glLoadName fp_glLoadName;
typedef void (APIENTRYP pfn_glPassThrough) (GLfloat);
extern pfn_glPassThrough fp_glPassThrough;
typedef void (APIENTRYP pfn_glPopName) ();
extern pfn_glPopName fp_glPopName;
typedef void (APIENTRYP pfn_glPushName) (GLuint);
extern pfn_glPushName fp_glPushName;
typedef void (APIENTRYP pfn_glClearAccum) (GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glClearAccum fp_glClearAccum;
typedef void (APIENTRYP pfn_glClearIndex) (GLfloat);
extern pfn_glClearIndex fp_glClearIndex;
typedef void (APIENTRYP pfn_glIndexMask) (GLuint);
extern pfn_glIndexMask fp_glIndexMask;
typedef void (APIENTRYP pfn_glAccum) (GLenum, GLfloat);
extern pfn_glAccum fp_glAccum;
typedef void (APIENTRYP pfn_glPopAttrib) ();
extern pfn_glPopAttrib fp_glPopAttrib;
typedef void (APIENTRYP pfn_glPushAttrib) (GLbitfield);
extern pfn_glPushAttrib fp_glPushAttrib;
typedef void (APIENTRYP pfn_glMap1d) (GLenum, GLdouble, GLdouble, GLint, GLint, const GLdouble*);
extern pfn_glMap1d fp_glMap1d;
typedef void (APIENTRYP pfn_glMap1f) (GLenum, GLfloat, GLfloat, GLint, GLint, const GLfloat*);
extern pfn_glMap1f fp_glMap1f;
typedef void (APIENTRYP pfn_glMap2d) (GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, const GLdouble*);
extern pfn_glMap2d fp_glMap2d;
typedef void (APIENTRYP pfn_glMap2f) (GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, const GLfloat*);
extern pfn_glMap2f fp_glMap2f;
typedef void (APIENTRYP pfn_glMapGrid1d) (GLint, GLdouble, GLdouble);
extern pfn_glMapGrid1d fp_glMapGrid1d;
typedef void (APIENTRYP pfn_glMapGrid1f) (GLint, GLfloat, GLfloat);
extern pfn_glMapGrid1f fp_glMapGrid1f;
typedef void (APIENTRYP pfn_glMapGrid2d) (GLint, GLdouble, GLdouble, GLint, GLdouble, GLdouble);
extern pfn_glMapGrid2d fp_glMapGrid2d;
typedef void (APIENTRYP pfn_glMapGrid2f) (GLint, GLfloat, GLfloat, GLint, GLfloat, GLfloat);
extern pfn_glMapGrid2f fp_glMapGrid2f;
typedef void (APIENTRYP pfn_glEvalCoord1d) (GLdouble);
extern pfn_glEvalCoord1d fp_glEvalCoord1d;
typedef void (APIENTRYP pfn_glEvalCoord1dv) (const GLdouble*);
extern pfn_glEvalCoord1dv fp_glEvalCoord1dv;
typedef void (APIENTRYP pfn_glEvalCoord1f) (GLfloat);
extern pfn_glEvalCoord1f fp_glEvalCoord1f;
typedef void (APIENTRYP pfn_glEvalCoord1fv) (const GLfloat*);
extern pfn_glEvalCoord1fv fp_glEvalCoord1fv;
typedef void (APIENTRYP pfn_glEvalCoord2d) (GLdouble, GLdouble);
extern pfn_glEvalCoord2d fp_glEvalCoord2d;
typedef void (APIENTRYP pfn_glEvalCoord2dv) (const GLdouble*);
extern pfn_glEvalCoord2dv fp_glEvalCoord2dv;
typedef void (APIENTRYP pfn_glEvalCoord2f) (GLfloat, GLfloat);
extern pfn_glEvalCoord2f fp_glEvalCoord2f;
typedef void (APIENTRYP pfn_glEvalCoord2fv) (const GLfloat*);
extern pfn_glEvalCoord2fv fp_glEvalCoord2fv;
typedef void (APIENTRYP pfn_glEvalMesh1) (GLenum, GLint, GLint);
extern pfn_glEvalMesh1 fp_glEvalMesh1;
typedef void (APIENTRYP pfn_glEvalPoint1) (GLint);
extern pfn_glEvalPoint1 fp_glEvalPoint1;
typedef void (APIENTRYP pfn_glEvalMesh2) (GLenum, GLint, GLint, GLint, GLint);
extern pfn_glEvalMesh2 fp_glEvalMesh2;
typedef void (APIENTRYP pfn_glEvalPoint2) (GLint, GLint);
extern pfn_glEvalPoint2 fp_glEvalPoint2;
typedef void (APIENTRYP pfn_glAlphaFunc) (GLenum, GLfloat);
extern pfn_glAlphaFunc fp_glAlphaFunc;
typedef void (APIENTRYP pfn_glPixelZoom) (GLfloat, GLfloat);
extern pfn_glPixelZoom fp_glPixelZoom;
typedef void (APIENTRYP pfn_glPixelTransferf) (GLenum, GLfloat);
extern pfn_glPixelTransferf fp_glPixelTransferf;
typedef void (APIENTRYP pfn_glPixelTransferi) (GLenum, GLint);
extern pfn_glPixelTransferi fp_glPixelTransferi;
typedef void (APIENTRYP pfn_glPixelMapfv) (GLenum, GLsizei, const GLfloat*);
extern pfn_glPixelMapfv fp_glPixelMapfv;
typedef void (APIENTRYP pfn_glPixelMapuiv) (GLenum, GLsizei, const GLuint*);
extern pfn_glPixelMapuiv fp_glPixelMapuiv;
typedef void (APIENTRYP pfn_glPixelMapusv) (GLenum, GLsizei, const GLushort*);
extern pfn_glPixelMapusv fp_glPixelMapusv;
typedef void (APIENTRYP pfn_glCopyPixels) (GLint, GLint, GLsizei, GLsizei, GLenum);
extern pfn_glCopyPixels fp_glCopyPixels;
typedef void (APIENTRYP pfn_glDrawPixels) (GLsizei, GLsizei, GLenum, GLenum, const void*);
extern pfn_glDrawPixels fp_glDrawPixels;
typedef void (APIENTRYP pfn_glGetClipPlane) (GLenum, GLdouble*);
extern pfn_glGetClipPlane fp_glGetClipPlane;
typedef void (APIENTRYP pfn_glGetLightfv) (GLenum, GLenum, GLfloat*);
extern pfn_glGetLightfv fp_glGetLightfv;
typedef void (APIENTRYP pfn_glGetLightiv) (GLenum, GLenum, GLint*);
extern pfn_glGetLightiv fp_glGetLightiv;
typedef void (APIENTRYP pfn_glGetMapdv) (GLenum, GLenum, GLdouble*);
extern pfn_glGetMapdv fp_glGetMapdv;
typedef void (APIENTRYP pfn_glGetMapfv) (GLenum, GLenum, GLfloat*);
extern pfn_glGetMapfv fp_glGetMapfv;
typedef void (APIENTRYP pfn_glGetMapiv) (GLenum, GLenum, GLint*);
extern pfn_glGetMapiv fp_glGetMapiv;
typedef void (APIENTRYP pfn_glGetMaterialfv) (GLenum, GLenum, GLfloat*);
extern pfn_glGetMaterialfv fp_glGetMaterialfv;
typedef void (APIENTRYP pfn_glGetMaterialiv) (GLenum, GLenum, GLint*);
extern pfn_glGetMaterialiv fp_glGetMaterialiv;
typedef void (APIENTRYP pfn_glGetPixelMapfv) (GLenum, GLfloat*);
extern pfn_glGetPixelMapfv fp_glGetPixelMapfv;
typedef void (APIENTRYP pfn_glGetPixelMapuiv) (GLenum, GLuint*);
extern pfn_glGetPixelMapuiv fp_glGetPixelMapuiv;
typedef void (APIENTRYP pfn_glGetPixelMapusv) (GLenum, GLushort*);
extern pfn_glGetPixelMapusv fp_glGetPixelMapusv;
typedef void (APIENTRYP pfn_glGetPolygonStipple) (GLubyte*);
extern pfn_glGetPolygonStipple fp_glGetPolygonStipple;
typedef void (APIENTRYP pfn_glGetTexEnvfv) (GLenum, GLenum, GLfloat*);
extern pfn_glGetTexEnvfv fp_glGetTexEnvfv;
typedef void (APIENTRYP pfn_glGetTexEnviv) (GLenum, GLenum, GLint*);
extern pfn_glGetTexEnviv fp_glGetTexEnviv;
typedef void (APIENTRYP pfn_glGetTexGendv) (GLenum, GLenum, GLdouble*);
extern pfn_glGetTexGendv fp_glGetTexGendv;
typedef void (APIENTRYP pfn_glGetTexGenfv) (GLenum, GLenum, GLfloat*);
extern pfn_glGetTexGenfv fp_glGetTexGenfv;
typedef void (APIENTRYP pfn_glGetTexGeniv) (GLenum, GLenum, GLint*);
extern pfn_glGetTexGeniv fp_glGetTexGeniv;
typedef GLboolean (APIENTRYP pfn_glIsList) (GLuint);
extern pfn_glIsList fp_glIsList;
typedef void (APIENTRYP pfn_glFrustum) (GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glFrustum fp_glFrustum;
typedef void (APIENTRYP pfn_glLoadIdentity) ();
extern pfn_glLoadIdentity fp_glLoadIdentity;
typedef void (APIENTRYP pfn_glLoadMatrixf) (const GLfloat*);
extern pfn_glLoadMatrixf fp_glLoadMatrixf;
typedef void (APIENTRYP pfn_glLoadMatrixd) (const GLdouble*);
extern pfn_glLoadMatrixd fp_glLoadMatrixd;
typedef void (APIENTRYP pfn_glMatrixMode) (GLenum);
extern pfn_glMatrixMode fp_glMatrixMode;
typedef void (APIENTRYP pfn_glMultMatrixf) (const GLfloat*);
extern pfn_glMultMatrixf fp_glMultMatrixf;
typedef void (APIENTRYP pfn_glMultMatrixd) (const GLdouble*);
extern pfn_glMultMatrixd fp_glMultMatrixd;
typedef void (APIENTRYP pfn_glOrtho) (GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glOrtho fp_glOrtho;
typedef void (APIENTRYP pfn_glPopMatrix) ();
extern pfn_glPopMatrix fp_glPopMatrix;
typedef void (APIENTRYP pfn_glPushMatrix) ();
extern pfn_glPushMatrix fp_glPushMatrix;
typedef void (APIENTRYP pfn_glRotated) (GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glRotated fp_glRotated;
typedef void (APIENTRYP pfn_glRotatef) (GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glRotatef fp_glRotatef;
typedef void (APIENTRYP pfn_glScaled) (GLdouble, GLdouble, GLdouble);
extern pfn_glScaled fp_glScaled;
typedef void (APIENTRYP pfn_glScalef) (GLfloat, GLfloat, GLfloat);
extern pfn_glScalef fp_glScalef;
typedef void (APIENTRYP pfn_glTranslated) (GLdouble, GLdouble, GLdouble);
extern pfn_glTranslated fp_glTranslated;
typedef void (APIENTRYP pfn_glTranslatef) (GLfloat, GLfloat, GLfloat);
extern pfn_glTranslatef fp_glTranslatef;

 /* GL_VERSION_1_1 */
extern GLboolean GLAD_VERSION_1_1;
#define GL_DEPTH_BUFFER_BIT                    0x00000100
#define GL_STENCIL_BUFFER_BIT                  0x00000400
#define GL_COLOR_BUFFER_BIT                    0x00004000
#define GL_FALSE                               0
#define GL_TRUE                                1
#define GL_POINTS                              0x0000
#define GL_LINES                               0x0001
#define GL_LINE_LOOP                           0x0002
#define GL_LINE_STRIP                          0x0003
#define GL_TRIANGLES                           0x0004
#define GL_TRIANGLE_STRIP                      0x0005
#define GL_TRIANGLE_FAN                        0x0006
#define GL_QUADS                               0x0007
#define GL_NEVER                               0x0200
#define GL_LESS                                0x0201
#define GL_EQUAL                               0x0202
#define GL_LEQUAL                              0x0203
#define GL_GREATER                             0x0204
#define GL_NOTEQUAL                            0x0205
#define GL_GEQUAL                              0x0206
#define GL_ALWAYS                              0x0207
#define GL_ZERO                                0
#define GL_ONE                                 1
#define GL_SRC_COLOR                           0x0300
#define GL_ONE_MINUS_SRC_COLOR                 0x0301
#define GL_SRC_ALPHA                           0x0302
#define GL_ONE_MINUS_SRC_ALPHA                 0x0303
#define GL_DST_ALPHA                           0x0304
#define GL_ONE_MINUS_DST_ALPHA                 0x0305
#define GL_DST_COLOR                           0x0306
#define GL_ONE_MINUS_DST_COLOR                 0x0307
#define GL_SRC_ALPHA_SATURATE                  0x0308
#define GL_NONE                                0
#define GL_FRONT_LEFT                          0x0400
#define GL_FRONT_RIGHT                         0x0401
#define GL_BACK_LEFT                           0x0402
#define GL_BACK_RIGHT                          0x0403
#define GL_FRONT                               0x0404
#define GL_BACK                                0x0405
#define GL_LEFT                                0x0406
#define GL_RIGHT                               0x0407
#define GL_FRONT_AND_BACK                      0x0408
#define GL_NO_ERROR                            0
#define GL_INVALID_ENUM                        0x0500
#define GL_INVALID_VALUE                       0x0501
#define GL_INVALID_OPERATION                   0x0502
#define GL_OUT_OF_MEMORY                       0x0505
#define GL_CW                                  0x0900
#define GL_CCW                                 0x0901
#define GL_POINT_SIZE                          0x0B11
#define GL_POINT_SIZE_RANGE                    0x0B12
#define GL_POINT_SIZE_GRANULARITY              0x0B13
#define GL_LINE_SMOOTH                         0x0B20
#define GL_LINE_WIDTH                          0x0B21
#define GL_LINE_WIDTH_RANGE                    0x0B22
#define GL_LINE_WIDTH_GRANULARITY              0x0B23
#define GL_POLYGON_MODE                        0x0B40
#define GL_POLYGON_SMOOTH                      0x0B41
#define GL_CULL_FACE                           0x0B44
#define GL_CULL_FACE_MODE                      0x0B45
#define GL_FRONT_FACE                          0x0B46
#define GL_DEPTH_RANGE                         0x0B70
#define GL_DEPTH_TEST                          0x0B71
#define GL_DEPTH_WRITEMASK                     0x0B72
#define GL_DEPTH_CLEAR_VALUE                   0x0B73
#define GL_DEPTH_FUNC                          0x0B74
#define GL_STENCIL_TEST                        0x0B90
#define GL_STENCIL_CLEAR_VALUE                 0x0B91
#define GL_STENCIL_FUNC                        0x0B92
#define GL_STENCIL_VALUE_MASK                  0x0B93
#define GL_STENCIL_FAIL                        0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL             0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS             0x0B96
#define GL_STENCIL_REF                         0x0B97
#define GL_STENCIL_WRITEMASK                   0x0B98
#define GL_VIEWPORT                            0x0BA2
#define GL_DITHER                              0x0BD0
#define GL_BLEND_DST                           0x0BE0
#define GL_BLEND_SRC                           0x0BE1
#define GL_BLEND                               0x0BE2
#define GL_LOGIC_OP_MODE                       0x0BF0
#define GL_COLOR_LOGIC_OP                      0x0BF2
#define GL_DRAW_BUFFER                         0x0C01
#define GL_READ_BUFFER                         0x0C02
#define GL_SCISSOR_BOX                         0x0C10
#define GL_SCISSOR_TEST                        0x0C11
#define GL_COLOR_CLEAR_VALUE                   0x0C22
#define GL_COLOR_WRITEMASK                     0x0C23
#define GL_DOUBLEBUFFER                        0x0C32
#define GL_STEREO                              0x0C33
#define GL_LINE_SMOOTH_HINT                    0x0C52
#define GL_POLYGON_SMOOTH_HINT                 0x0C53
#define GL_UNPACK_SWAP_BYTES                   0x0CF0
#define GL_UNPACK_LSB_FIRST                    0x0CF1
#define GL_UNPACK_ROW_LENGTH                   0x0CF2
#define GL_UNPACK_SKIP_ROWS                    0x0CF3
#define GL_UNPACK_SKIP_PIXELS                  0x0CF4
#define GL_UNPACK_ALIGNMENT                    0x0CF5
#define GL_PACK_SWAP_BYTES                     0x0D00
#define GL_PACK_LSB_FIRST                      0x0D01
#define GL_PACK_ROW_LENGTH                     0x0D02
#define GL_PACK_SKIP_ROWS                      0x0D03
#define GL_PACK_SKIP_PIXELS                    0x0D04
#define GL_PACK_ALIGNMENT                      0x0D05
#define GL_MAX_TEXTURE_SIZE                    0x0D33
#define GL_MAX_VIEWPORT_DIMS                   0x0D3A
#define GL_SUBPIXEL_BITS                       0x0D50
#define GL_TEXTURE_1D                          0x0DE0
#define GL_TEXTURE_2D                          0x0DE1
#define GL_POLYGON_OFFSET_UNITS                0x2A00
#define GL_POLYGON_OFFSET_POINT                0x2A01
#define GL_POLYGON_OFFSET_LINE                 0x2A02
#define GL_POLYGON_OFFSET_FILL                 0x8037
#define GL_POLYGON_OFFSET_FACTOR               0x8038
#define GL_TEXTURE_BINDING_1D                  0x8068
#define GL_TEXTURE_BINDING_2D                  0x8069
#define GL_TEXTURE_WIDTH                       0x1000
#define GL_TEXTURE_HEIGHT                      0x1001
#define GL_TEXTURE_INTERNAL_FORMAT             0x1003
#define GL_TEXTURE_BORDER_COLOR                0x1004
#define GL_TEXTURE_RED_SIZE                    0x805C
#define GL_TEXTURE_GREEN_SIZE                  0x805D
#define GL_TEXTURE_BLUE_SIZE                   0x805E
#define GL_TEXTURE_ALPHA_SIZE                  0x805F
#define GL_DONT_CARE                           0x1100
#define GL_FASTEST                             0x1101
#define GL_NICEST                              0x1102
#define GL_BYTE                                0x1400
#define GL_UNSIGNED_BYTE                       0x1401
#define GL_SHORT                               0x1402
#define GL_UNSIGNED_SHORT                      0x1403
#define GL_INT                                 0x1404
#define GL_UNSIGNED_INT                        0x1405
#define GL_FLOAT                               0x1406
#define GL_DOUBLE                              0x140A
#define GL_STACK_OVERFLOW                      0x0503
#define GL_STACK_UNDERFLOW                     0x0504
#define GL_CLEAR                               0x1500
#define GL_AND                                 0x1501
#define GL_AND_REVERSE                         0x1502
#define GL_COPY                                0x1503
#define GL_AND_INVERTED                        0x1504
#define GL_NOOP                                0x1505
#define GL_XOR                                 0x1506
#define GL_OR                                  0x1507
#define GL_NOR                                 0x1508
#define GL_EQUIV                               0x1509
#define GL_INVERT                              0x150A
#define GL_OR_REVERSE                          0x150B
#define GL_COPY_INVERTED                       0x150C
#define GL_OR_INVERTED                         0x150D
#define GL_NAND                                0x150E
#define GL_SET                                 0x150F
#define GL_TEXTURE                             0x1702
#define GL_COLOR                               0x1800
#define GL_DEPTH                               0x1801
#define GL_STENCIL                             0x1802
#define GL_STENCIL_INDEX                       0x1901
#define GL_DEPTH_COMPONENT                     0x1902
#define GL_RED                                 0x1903
#define GL_GREEN                               0x1904
#define GL_BLUE                                0x1905
#define GL_ALPHA                               0x1906
#define GL_RGB                                 0x1907
#define GL_RGBA                                0x1908
#define GL_POINT                               0x1B00
#define GL_LINE                                0x1B01
#define GL_FILL                                0x1B02
#define GL_KEEP                                0x1E00
#define GL_REPLACE                             0x1E01
#define GL_INCR                                0x1E02
#define GL_DECR                                0x1E03
#define GL_VENDOR                              0x1F00
#define GL_RENDERER                            0x1F01
#define GL_VERSION                             0x1F02
#define GL_EXTENSIONS                          0x1F03
#define GL_NEAREST                             0x2600
#define GL_LINEAR                              0x2601
#define GL_NEAREST_MIPMAP_NEAREST              0x2700
#define GL_LINEAR_MIPMAP_NEAREST               0x2701
#define GL_NEAREST_MIPMAP_LINEAR               0x2702
#define GL_LINEAR_MIPMAP_LINEAR                0x2703
#define GL_TEXTURE_MAG_FILTER                  0x2800
#define GL_TEXTURE_MIN_FILTER                  0x2801
#define GL_TEXTURE_WRAP_S                      0x2802
#define GL_TEXTURE_WRAP_T                      0x2803
#define GL_PROXY_TEXTURE_1D                    0x8063
#define GL_PROXY_TEXTURE_2D                    0x8064
#define GL_REPEAT                              0x2901
#define GL_R3_G3_B2                            0x2A10
#define GL_RGB4                                0x804F
#define GL_RGB5                                0x8050
#define GL_RGB8                                0x8051
#define GL_RGB10                               0x8052
#define GL_RGB12                               0x8053
#define GL_RGB16                               0x8054
#define GL_RGBA2                               0x8055
#define GL_RGBA4                               0x8056
#define GL_RGB5_A1                             0x8057
#define GL_RGBA8                               0x8058
#define GL_RGB10_A2                            0x8059
#define GL_RGBA12                              0x805A
#define GL_RGBA16                              0x805B
#define GL_CURRENT_BIT                         0x00000001
#define GL_POINT_BIT                           0x00000002
#define GL_LINE_BIT                            0x00000004
#define GL_POLYGON_BIT                         0x00000008
#define GL_POLYGON_STIPPLE_BIT                 0x00000010
#define GL_PIXEL_MODE_BIT                      0x00000020
#define GL_LIGHTING_BIT                        0x00000040
#define GL_FOG_BIT                             0x00000080
#define GL_ACCUM_BUFFER_BIT                    0x00000200
#define GL_VIEWPORT_BIT                        0x00000800
#define GL_TRANSFORM_BIT                       0x00001000
#define GL_ENABLE_BIT                          0x00002000
#define GL_HINT_BIT                            0x00008000
#define GL_EVAL_BIT                            0x00010000
#define GL_LIST_BIT                            0x00020000
#define GL_TEXTURE_BIT                         0x00040000
#define GL_SCISSOR_BIT                         0x00080000
#define GL_ALL_ATTRIB_BITS                     0xFFFFFFFF
#define GL_CLIENT_PIXEL_STORE_BIT              0x00000001
#define GL_CLIENT_VERTEX_ARRAY_BIT             0x00000002
#define GL_CLIENT_ALL_ATTRIB_BITS              0xFFFFFFFF
#define GL_QUAD_STRIP                          0x0008
#define GL_POLYGON                             0x0009
#define GL_ACCUM                               0x0100
#define GL_LOAD                                0x0101
#define GL_RETURN                              0x0102
#define GL_MULT                                0x0103
#define GL_ADD                                 0x0104
#define GL_AUX0                                0x0409
#define GL_AUX1                                0x040A
#define GL_AUX2                                0x040B
#define GL_AUX3                                0x040C
#define GL_2D                                  0x0600
#define GL_3D                                  0x0601
#define GL_3D_COLOR                            0x0602
#define GL_3D_COLOR_TEXTURE                    0x0603
#define GL_4D_COLOR_TEXTURE                    0x0604
#define GL_PASS_THROUGH_TOKEN                  0x0700
#define GL_POINT_TOKEN                         0x0701
#define GL_LINE_TOKEN                          0x0702
#define GL_POLYGON_TOKEN                       0x0703
#define GL_BITMAP_TOKEN                        0x0704
#define GL_DRAW_PIXEL_TOKEN                    0x0705
#define GL_COPY_PIXEL_TOKEN                    0x0706
#define GL_LINE_RESET_TOKEN                    0x0707
#define GL_EXP                                 0x0800
#define GL_EXP2                                0x0801
#define GL_COEFF                               0x0A00
#define GL_ORDER                               0x0A01
#define GL_DOMAIN                              0x0A02
#define GL_PIXEL_MAP_I_TO_I                    0x0C70
#define GL_PIXEL_MAP_S_TO_S                    0x0C71
#define GL_PIXEL_MAP_I_TO_R                    0x0C72
#define GL_PIXEL_MAP_I_TO_G                    0x0C73
#define GL_PIXEL_MAP_I_TO_B                    0x0C74
#define GL_PIXEL_MAP_I_TO_A                    0x0C75
#define GL_PIXEL_MAP_R_TO_R                    0x0C76
#define GL_PIXEL_MAP_G_TO_G                    0x0C77
#define GL_PIXEL_MAP_B_TO_B                    0x0C78
#define GL_PIXEL_MAP_A_TO_A                    0x0C79
#define GL_VERTEX_ARRAY_POINTER                0x808E
#define GL_NORMAL_ARRAY_POINTER                0x808F
#define GL_COLOR_ARRAY_POINTER                 0x8090
#define GL_INDEX_ARRAY_POINTER                 0x8091
#define GL_TEXTURE_COORD_ARRAY_POINTER         0x8092
#define GL_EDGE_FLAG_ARRAY_POINTER             0x8093
#define GL_FEEDBACK_BUFFER_POINTER             0x0DF0
#define GL_SELECTION_BUFFER_POINTER            0x0DF3
#define GL_CURRENT_COLOR                       0x0B00
#define GL_CURRENT_INDEX                       0x0B01
#define GL_CURRENT_NORMAL                      0x0B02
#define GL_CURRENT_TEXTURE_COORDS              0x0B03
#define GL_CURRENT_RASTER_COLOR                0x0B04
#define GL_CURRENT_RASTER_INDEX                0x0B05
#define GL_CURRENT_RASTER_TEXTURE_COORDS       0x0B06
#define GL_CURRENT_RASTER_POSITION             0x0B07
#define GL_CURRENT_RASTER_POSITION_VALID       0x0B08
#define GL_CURRENT_RASTER_DISTANCE             0x0B09
#define GL_POINT_SMOOTH                        0x0B10
#define GL_LINE_STIPPLE                        0x0B24
#define GL_LINE_STIPPLE_PATTERN                0x0B25
#define GL_LINE_STIPPLE_REPEAT                 0x0B26
#define GL_LIST_MODE                           0x0B30
#define GL_MAX_LIST_NESTING                    0x0B31
#define GL_LIST_BASE                           0x0B32
#define GL_LIST_INDEX                          0x0B33
#define GL_POLYGON_STIPPLE                     0x0B42
#define GL_EDGE_FLAG                           0x0B43
#define GL_LIGHTING                            0x0B50
#define GL_LIGHT_MODEL_LOCAL_VIEWER            0x0B51
#define GL_LIGHT_MODEL_TWO_SIDE                0x0B52
#define GL_LIGHT_MODEL_AMBIENT                 0x0B53
#define GL_SHADE_MODEL                         0x0B54
#define GL_COLOR_MATERIAL_FACE                 0x0B55
#define GL_COLOR_MATERIAL_PARAMETER            0x0B56
#define GL_COLOR_MATERIAL                      0x0B57
#define GL_FOG                                 0x0B60
#define GL_FOG_INDEX                           0x0B61
#define GL_FOG_DENSITY                         0x0B62
#define GL_FOG_START                           0x0B63
#define GL_FOG_END                             0x0B64
#define GL_FOG_MODE                            0x0B65
#define GL_FOG_COLOR                           0x0B66
#define GL_ACCUM_CLEAR_VALUE                   0x0B80
#define GL_MATRIX_MODE                         0x0BA0
#define GL_NORMALIZE                           0x0BA1
#define GL_MODELVIEW_STACK_DEPTH               0x0BA3
#define GL_PROJECTION_STACK_DEPTH              0x0BA4
#define GL_TEXTURE_STACK_DEPTH                 0x0BA5
#define GL_MODELVIEW_MATRIX                    0x0BA6
#define GL_PROJECTION_MATRIX                   0x0BA7
#define GL_TEXTURE_MATRIX                      0x0BA8
#define GL_ATTRIB_STACK_DEPTH                  0x0BB0
#define GL_CLIENT_ATTRIB_STACK_DEPTH           0x0BB1
#define GL_ALPHA_TEST                          0x0BC0
#define GL_ALPHA_TEST_FUNC                     0x0BC1
#define GL_ALPHA_TEST_REF                      0x0BC2
#define GL_INDEX_LOGIC_OP                      0x0BF1
#define GL_LOGIC_OP                            0x0BF1
#define GL_AUX_BUFFERS                         0x0C00
#define GL_INDEX_CLEAR_VALUE                   0x0C20
#define GL_INDEX_WRITEMASK                     0x0C21
#define GL_INDEX_MODE                          0x0C30
#define GL_RGBA_MODE                           0x0C31
#define GL_RENDER_MODE                         0x0C40
#define GL_PERSPECTIVE_CORRECTION_HINT         0x0C50
#define GL_POINT_SMOOTH_HINT                   0x0C51
#define GL_FOG_HINT                            0x0C54
#define GL_TEXTURE_GEN_S                       0x0C60
#define GL_TEXTURE_GEN_T                       0x0C61
#define GL_TEXTURE_GEN_R                       0x0C62
#define GL_TEXTURE_GEN_Q                       0x0C63
#define GL_PIXEL_MAP_I_TO_I_SIZE               0x0CB0
#define GL_PIXEL_MAP_S_TO_S_SIZE               0x0CB1
#define GL_PIXEL_MAP_I_TO_R_SIZE               0x0CB2
#define GL_PIXEL_MAP_I_TO_G_SIZE               0x0CB3
#define GL_PIXEL_MAP_I_TO_B_SIZE               0x0CB4
#define GL_PIXEL_MAP_I_TO_A_SIZE               0x0CB5
#define GL_PIXEL_MAP_R_TO_R_SIZE               0x0CB6
#define GL_PIXEL_MAP_G_TO_G_SIZE               0x0CB7
#define GL_PIXEL_MAP_B_TO_B_SIZE               0x0CB8
#define GL_PIXEL_MAP_A_TO_A_SIZE               0x0CB9
#define GL_MAP_COLOR                           0x0D10
#define GL_MAP_STENCIL                         0x0D11
#define GL_INDEX_SHIFT                         0x0D12
#define GL_INDEX_OFFSET                        0x0D13
#define GL_RED_SCALE                           0x0D14
#define GL_RED_BIAS                            0x0D15
#define GL_ZOOM_X                              0x0D16
#define GL_ZOOM_Y                              0x0D17
#define GL_GREEN_SCALE                         0x0D18
#define GL_GREEN_BIAS                          0x0D19
#define GL_BLUE_SCALE                          0x0D1A
#define GL_BLUE_BIAS                           0x0D1B
#define GL_ALPHA_SCALE                         0x0D1C
#define GL_ALPHA_BIAS                          0x0D1D
#define GL_DEPTH_SCALE                         0x0D1E
#define GL_DEPTH_BIAS                          0x0D1F
#define GL_MAX_EVAL_ORDER                      0x0D30
#define GL_MAX_LIGHTS                          0x0D31
#define GL_MAX_CLIP_PLANES                     0x0D32
#define GL_MAX_PIXEL_MAP_TABLE                 0x0D34
#define GL_MAX_ATTRIB_STACK_DEPTH              0x0D35
#define GL_MAX_MODELVIEW_STACK_DEPTH           0x0D36
#define GL_MAX_NAME_STACK_DEPTH                0x0D37
#define GL_MAX_PROJECTION_STACK_DEPTH          0x0D38
#define GL_MAX_TEXTURE_STACK_DEPTH             0x0D39
#define GL_MAX_CLIENT_ATTRIB_STACK_DEPTH       0x0D3B
#define GL_INDEX_BITS                          0x0D51
#define GL_RED_BITS                            0x0D52
#define GL_GREEN_BITS                          0x0D53
#define GL_BLUE_BITS                           0x0D54
#define GL_ALPHA_BITS                          0x0D55
#define GL_DEPTH_BITS                          0x0D56
#define GL_STENCIL_BITS                        0x0D57
#define GL_ACCUM_RED_BITS                      0x0D58
#define GL_ACCUM_GREEN_BITS                    0x0D59
#define GL_ACCUM_BLUE_BITS                     0x0D5A
#define GL_ACCUM_ALPHA_BITS                    0x0D5B
#define GL_NAME_STACK_DEPTH                    0x0D70
#define GL_AUTO_NORMAL                         0x0D80
#define GL_MAP1_COLOR_4                        0x0D90
#define GL_MAP1_INDEX                          0x0D91
#define GL_MAP1_NORMAL                         0x0D92
#define GL_MAP1_TEXTURE_COORD_1                0x0D93
#define GL_MAP1_TEXTURE_COORD_2                0x0D94
#define GL_MAP1_TEXTURE_COORD_3                0x0D95
#define GL_MAP1_TEXTURE_COORD_4                0x0D96
#define GL_MAP1_VERTEX_3                       0x0D97
#define GL_MAP1_VERTEX_4                       0x0D98
#define GL_MAP2_COLOR_4                        0x0DB0
#define GL_MAP2_INDEX                          0x0DB1
#define GL_MAP2_NORMAL                         0x0DB2
#define GL_MAP2_TEXTURE_COORD_1                0x0DB3
#define GL_MAP2_TEXTURE_COORD_2                0x0DB4
#define GL_MAP2_TEXTURE_COORD_3                0x0DB5
#define GL_MAP2_TEXTURE_COORD_4                0x0DB6
#define GL_MAP2_VERTEX_3                       0x0DB7
#define GL_MAP2_VERTEX_4                       0x0DB8
#define GL_MAP1_GRID_DOMAIN                    0x0DD0
#define GL_MAP1_GRID_SEGMENTS                  0x0DD1
#define GL_MAP2_GRID_DOMAIN                    0x0DD2
#define GL_MAP2_GRID_SEGMENTS                  0x0DD3
#define GL_FEEDBACK_BUFFER_SIZE                0x0DF1
#define GL_FEEDBACK_BUFFER_TYPE                0x0DF2
#define GL_SELECTION_BUFFER_SIZE               0x0DF4
#define GL_VERTEX_ARRAY                        0x8074
#define GL_NORMAL_ARRAY                        0x8075
#define GL_COLOR_ARRAY                         0x8076
#define GL_INDEX_ARRAY                         0x8077
#define GL_TEXTURE_COORD_ARRAY                 0x8078
#define GL_EDGE_FLAG_ARRAY                     0x8079
#define GL_VERTEX_ARRAY_SIZE                   0x807A
#define GL_VERTEX_ARRAY_TYPE                   0x807B
#define GL_VERTEX_ARRAY_STRIDE                 0x807C
#define GL_NORMAL_ARRAY_TYPE                   0x807E
#define GL_NORMAL_ARRAY_STRIDE                 0x807F
#define GL_COLOR_ARRAY_SIZE                    0x8081
#define GL_COLOR_ARRAY_TYPE                    0x8082
#define GL_COLOR_ARRAY_STRIDE                  0x8083
#define GL_INDEX_ARRAY_TYPE                    0x8085
#define GL_INDEX_ARRAY_STRIDE                  0x8086
#define GL_TEXTURE_COORD_ARRAY_SIZE            0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE            0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE          0x808A
#define GL_EDGE_FLAG_ARRAY_STRIDE              0x808C
#define GL_TEXTURE_COMPONENTS                  0x1003
#define GL_TEXTURE_BORDER                      0x1005
#define GL_TEXTURE_LUMINANCE_SIZE              0x8060
#define GL_TEXTURE_INTENSITY_SIZE              0x8061
#define GL_TEXTURE_PRIORITY                    0x8066
#define GL_TEXTURE_RESIDENT                    0x8067
#define GL_AMBIENT                             0x1200
#define GL_DIFFUSE                             0x1201
#define GL_SPECULAR                            0x1202
#define GL_POSITION                            0x1203
#define GL_SPOT_DIRECTION                      0x1204
#define GL_SPOT_EXPONENT                       0x1205
#define GL_SPOT_CUTOFF                         0x1206
#define GL_CONSTANT_ATTENUATION                0x1207
#define GL_LINEAR_ATTENUATION                  0x1208
#define GL_QUADRATIC_ATTENUATION               0x1209
#define GL_COMPILE                             0x1300
#define GL_COMPILE_AND_EXECUTE                 0x1301
#define GL_2_BYTES                             0x1407
#define GL_3_BYTES                             0x1408
#define GL_4_BYTES                             0x1409
#define GL_EMISSION                            0x1600
#define GL_SHININESS                           0x1601
#define GL_AMBIENT_AND_DIFFUSE                 0x1602
#define GL_COLOR_INDEXES                       0x1603
#define GL_MODELVIEW                           0x1700
#define GL_PROJECTION                          0x1701
#define GL_COLOR_INDEX                         0x1900
#define GL_LUMINANCE                           0x1909
#define GL_LUMINANCE_ALPHA                     0x190A
#define GL_BITMAP                              0x1A00
#define GL_RENDER                              0x1C00
#define GL_FEEDBACK                            0x1C01
#define GL_SELECT                              0x1C02
#define GL_FLAT                                0x1D00
#define GL_SMOOTH                              0x1D01
#define GL_S                                   0x2000
#define GL_T                                   0x2001
#define GL_R                                   0x2002
#define GL_Q                                   0x2003
#define GL_MODULATE                            0x2100
#define GL_DECAL                               0x2101
#define GL_TEXTURE_ENV_MODE                    0x2200
#define GL_TEXTURE_ENV_COLOR                   0x2201
#define GL_TEXTURE_ENV                         0x2300
#define GL_EYE_LINEAR                          0x2400
#define GL_OBJECT_LINEAR                       0x2401
#define GL_SPHERE_MAP                          0x2402
#define GL_TEXTURE_GEN_MODE                    0x2500
#define GL_OBJECT_PLANE                        0x2501
#define GL_EYE_PLANE                           0x2502
#define GL_CLAMP                               0x2900
#define GL_ALPHA4                              0x803B
#define GL_ALPHA8                              0x803C
#define GL_ALPHA12                             0x803D
#define GL_ALPHA16                             0x803E
#define GL_LUMINANCE4                          0x803F
#define GL_LUMINANCE8                          0x8040
#define GL_LUMINANCE12                         0x8041
#define GL_LUMINANCE16                         0x8042
#define GL_LUMINANCE4_ALPHA4                   0x8043
#define GL_LUMINANCE6_ALPHA2                   0x8044
#define GL_LUMINANCE8_ALPHA8                   0x8045
#define GL_LUMINANCE12_ALPHA4                  0x8046
#define GL_LUMINANCE12_ALPHA12                 0x8047
#define GL_LUMINANCE16_ALPHA16                 0x8048
#define GL_INTENSITY                           0x8049
#define GL_INTENSITY4                          0x804A
#define GL_INTENSITY8                          0x804B
#define GL_INTENSITY12                         0x804C
#define GL_INTENSITY16                         0x804D
#define GL_V2F                                 0x2A20
#define GL_V3F                                 0x2A21
#define GL_C4UB_V2F                            0x2A22
#define GL_C4UB_V3F                            0x2A23
#define GL_C3F_V3F                             0x2A24
#define GL_N3F_V3F                             0x2A25
#define GL_C4F_N3F_V3F                         0x2A26
#define GL_T2F_V3F                             0x2A27
#define GL_T4F_V4F                             0x2A28
#define GL_T2F_C4UB_V3F                        0x2A29
#define GL_T2F_C3F_V3F                         0x2A2A
#define GL_T2F_N3F_V3F                         0x2A2B
#define GL_T2F_C4F_N3F_V3F                     0x2A2C
#define GL_T4F_C4F_N3F_V4F                     0x2A2D
#define GL_CLIP_PLANE0                         0x3000
#define GL_CLIP_PLANE1                         0x3001
#define GL_CLIP_PLANE2                         0x3002
#define GL_CLIP_PLANE3                         0x3003
#define GL_CLIP_PLANE4                         0x3004
#define GL_CLIP_PLANE5                         0x3005
#define GL_LIGHT0                              0x4000
#define GL_LIGHT1                              0x4001
#define GL_LIGHT2                              0x4002
#define GL_LIGHT3                              0x4003
#define GL_LIGHT4                              0x4004
#define GL_LIGHT5                              0x4005
#define GL_LIGHT6                              0x4006
#define GL_LIGHT7                              0x4007
typedef void (APIENTRYP pfn_glDrawArrays) (GLenum, GLint, GLsizei);
extern pfn_glDrawArrays fp_glDrawArrays;
typedef void (APIENTRYP pfn_glDrawElements) (GLenum, GLsizei, GLenum, const void*);
extern pfn_glDrawElements fp_glDrawElements;
typedef void (APIENTRYP pfn_glGetPointerv) (GLenum, void**);
extern pfn_glGetPointerv fp_glGetPointerv;
typedef void (APIENTRYP pfn_glPolygonOffset) (GLfloat, GLfloat);
extern pfn_glPolygonOffset fp_glPolygonOffset;
typedef void (APIENTRYP pfn_glCopyTexImage1D) (GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint);
extern pfn_glCopyTexImage1D fp_glCopyTexImage1D;
typedef void (APIENTRYP pfn_glCopyTexImage2D) (GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
extern pfn_glCopyTexImage2D fp_glCopyTexImage2D;
typedef void (APIENTRYP pfn_glCopyTexSubImage1D) (GLenum, GLint, GLint, GLint, GLint, GLsizei);
extern pfn_glCopyTexSubImage1D fp_glCopyTexSubImage1D;
typedef void (APIENTRYP pfn_glCopyTexSubImage2D) (GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
extern pfn_glCopyTexSubImage2D fp_glCopyTexSubImage2D;
typedef void (APIENTRYP pfn_glTexSubImage1D) (GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const void*);
extern pfn_glTexSubImage1D fp_glTexSubImage1D;
typedef void (APIENTRYP pfn_glTexSubImage2D) (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*);
extern pfn_glTexSubImage2D fp_glTexSubImage2D;
typedef void (APIENTRYP pfn_glBindTexture) (GLenum, GLuint);
extern pfn_glBindTexture fp_glBindTexture;
typedef void (APIENTRYP pfn_glDeleteTextures) (GLsizei, const GLuint*);
extern pfn_glDeleteTextures fp_glDeleteTextures;
typedef void (APIENTRYP pfn_glGenTextures) (GLsizei, GLuint*);
extern pfn_glGenTextures fp_glGenTextures;
typedef GLboolean (APIENTRYP pfn_glIsTexture) (GLuint);
extern pfn_glIsTexture fp_glIsTexture;
typedef void (APIENTRYP pfn_glArrayElement) (GLint);
extern pfn_glArrayElement fp_glArrayElement;
typedef void (APIENTRYP pfn_glColorPointer) (GLint, GLenum, GLsizei, const void*);
extern pfn_glColorPointer fp_glColorPointer;
typedef void (APIENTRYP pfn_glDisableClientState) (GLenum);
extern pfn_glDisableClientState fp_glDisableClientState;
typedef void (APIENTRYP pfn_glEdgeFlagPointer) (GLsizei, const void*);
extern pfn_glEdgeFlagPointer fp_glEdgeFlagPointer;
typedef void (APIENTRYP pfn_glEnableClientState) (GLenum);
extern pfn_glEnableClientState fp_glEnableClientState;
typedef void (APIENTRYP pfn_glIndexPointer) (GLenum, GLsizei, const void*);
extern pfn_glIndexPointer fp_glIndexPointer;
typedef void (APIENTRYP pfn_glInterleavedArrays) (GLenum, GLsizei, const void*);
extern pfn_glInterleavedArrays fp_glInterleavedArrays;
typedef void (APIENTRYP pfn_glNormalPointer) (GLenum, GLsizei, const void*);
extern pfn_glNormalPointer fp_glNormalPointer;
typedef void (APIENTRYP pfn_glTexCoordPointer) (GLint, GLenum, GLsizei, const void*);
extern pfn_glTexCoordPointer fp_glTexCoordPointer;
typedef void (APIENTRYP pfn_glVertexPointer) (GLint, GLenum, GLsizei, const void*);
extern pfn_glVertexPointer fp_glVertexPointer;
typedef GLboolean (APIENTRYP pfn_glAreTexturesResident) (GLsizei, const GLuint*, GLboolean*);
extern pfn_glAreTexturesResident fp_glAreTexturesResident;
typedef void (APIENTRYP pfn_glPrioritizeTextures) (GLsizei, const GLuint*, const GLfloat*);
extern pfn_glPrioritizeTextures fp_glPrioritizeTextures;
typedef void (APIENTRYP pfn_glIndexub) (GLubyte);
extern pfn_glIndexub fp_glIndexub;
typedef void (APIENTRYP pfn_glIndexubv) (const GLubyte*);
extern pfn_glIndexubv fp_glIndexubv;
typedef void (APIENTRYP pfn_glPopClientAttrib) ();
extern pfn_glPopClientAttrib fp_glPopClientAttrib;
typedef void (APIENTRYP pfn_glPushClientAttrib) (GLbitfield);
extern pfn_glPushClientAttrib fp_glPushClientAttrib;

 /* GL_VERSION_1_2 */
extern GLboolean GLAD_VERSION_1_2;
#define GL_UNSIGNED_BYTE_3_3_2                 0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4              0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1              0x8034
#define GL_UNSIGNED_INT_8_8_8_8                0x8035
#define GL_UNSIGNED_INT_10_10_10_2             0x8036
#define GL_TEXTURE_BINDING_3D                  0x806A
#define GL_PACK_SKIP_IMAGES                    0x806B
#define GL_PACK_IMAGE_HEIGHT                   0x806C
#define GL_UNPACK_SKIP_IMAGES                  0x806D
#define GL_UNPACK_IMAGE_HEIGHT                 0x806E
#define GL_TEXTURE_3D                          0x806F
#define GL_PROXY_TEXTURE_3D                    0x8070
#define GL_TEXTURE_DEPTH                       0x8071
#define GL_TEXTURE_WRAP_R                      0x8072
#define GL_MAX_3D_TEXTURE_SIZE                 0x8073
#define GL_UNSIGNED_BYTE_2_3_3_REV             0x8362
#define GL_UNSIGNED_SHORT_5_6_5                0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV            0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4_REV          0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV          0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV            0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV         0x8368
#define GL_BGR                                 0x80E0
#define GL_BGRA                                0x80E1
#define GL_MAX_ELEMENTS_VERTICES               0x80E8
#define GL_MAX_ELEMENTS_INDICES                0x80E9
#define GL_CLAMP_TO_EDGE                       0x812F
#define GL_TEXTURE_MIN_LOD                     0x813A
#define GL_TEXTURE_MAX_LOD                     0x813B
#define GL_TEXTURE_BASE_LEVEL                  0x813C
#define GL_TEXTURE_MAX_LEVEL                   0x813D
#define GL_SMOOTH_POINT_SIZE_RANGE             0x0B12
#define GL_SMOOTH_POINT_SIZE_GRANULARITY       0x0B13
#define GL_SMOOTH_LINE_WIDTH_RANGE             0x0B22
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY       0x0B23
#define GL_ALIASED_LINE_WIDTH_RANGE            0x846E
#define GL_RESCALE_NORMAL                      0x803A
#define GL_LIGHT_MODEL_COLOR_CONTROL           0x81F8
#define GL_SINGLE_COLOR                        0x81F9
#define GL_SEPARATE_SPECULAR_COLOR             0x81FA
#define GL_ALIASED_POINT_SIZE_RANGE            0x846D
typedef void (APIENTRYP pfn_glDrawRangeElements) (GLenum, GLuint, GLuint, GLsizei, GLenum, const void*);
extern pfn_glDrawRangeElements fp_glDrawRangeElements;
typedef void (APIENTRYP pfn_glTexImage3D) (GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*);
extern pfn_glTexImage3D fp_glTexImage3D;
typedef void (APIENTRYP pfn_glTexSubImage3D) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*);
extern pfn_glTexSubImage3D fp_glTexSubImage3D;
typedef void (APIENTRYP pfn_glCopyTexSubImage3D) (GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
extern pfn_glCopyTexSubImage3D fp_glCopyTexSubImage3D;

 /* GL_VERSION_1_3 */
extern GLboolean GLAD_VERSION_1_3;
#define GL_TEXTURE0                            0x84C0
#define GL_TEXTURE1                            0x84C1
#define GL_TEXTURE2                            0x84C2
#define GL_TEXTURE3                            0x84C3
#define GL_TEXTURE4                            0x84C4
#define GL_TEXTURE5                            0x84C5
#define GL_TEXTURE6                            0x84C6
#define GL_TEXTURE7                            0x84C7
#define GL_TEXTURE8                            0x84C8
#define GL_TEXTURE9                            0x84C9
#define GL_TEXTURE10                           0x84CA
#define GL_TEXTURE11                           0x84CB
#define GL_TEXTURE12                           0x84CC
#define GL_TEXTURE13                           0x84CD
#define GL_TEXTURE14                           0x84CE
#define GL_TEXTURE15                           0x84CF
#define GL_TEXTURE16                           0x84D0
#define GL_TEXTURE17                           0x84D1
#define GL_TEXTURE18                           0x84D2
#define GL_TEXTURE19                           0x84D3
#define GL_TEXTURE20                           0x84D4
#define GL_TEXTURE21                           0x84D5
#define GL_TEXTURE22                           0x84D6
#define GL_TEXTURE23                           0x84D7
#define GL_TEXTURE24                           0x84D8
#define GL_TEXTURE25                           0x84D9
#define GL_TEXTURE26                           0x84DA
#define GL_TEXTURE27                           0x84DB
#define GL_TEXTURE28                           0x84DC
#define GL_TEXTURE29                           0x84DD
#define GL_TEXTURE30                           0x84DE
#define GL_TEXTURE31                           0x84DF
#define GL_ACTIVE_TEXTURE                      0x84E0
#define GL_MULTISAMPLE                         0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE            0x809E
#define GL_SAMPLE_ALPHA_TO_ONE                 0x809F
#define GL_SAMPLE_COVERAGE                     0x80A0
#define GL_SAMPLE_BUFFERS                      0x80A8
#define GL_SAMPLES                             0x80A9
#define GL_SAMPLE_COVERAGE_VALUE               0x80AA
#define GL_SAMPLE_COVERAGE_INVERT              0x80AB
#define GL_TEXTURE_CUBE_MAP                    0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP            0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X         0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X         0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y         0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y         0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z         0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z         0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP              0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE           0x851C
#define GL_COMPRESSED_RGB                      0x84ED
#define GL_COMPRESSED_RGBA                     0x84EE
#define GL_TEXTURE_COMPRESSION_HINT            0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE       0x86A0
#define GL_TEXTURE_COMPRESSED                  0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS      0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS          0x86A3
#define GL_CLAMP_TO_BORDER                     0x812D
#define GL_CLIENT_ACTIVE_TEXTURE               0x84E1
#define GL_MAX_TEXTURE_UNITS                   0x84E2
#define GL_TRANSPOSE_MODELVIEW_MATRIX          0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX         0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX            0x84E5
#define GL_TRANSPOSE_COLOR_MATRIX              0x84E6
#define GL_MULTISAMPLE_BIT                     0x20000000
#define GL_NORMAL_MAP                          0x8511
#define GL_REFLECTION_MAP                      0x8512
#define GL_COMPRESSED_ALPHA                    0x84E9
#define GL_COMPRESSED_LUMINANCE                0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA          0x84EB
#define GL_COMPRESSED_INTENSITY                0x84EC
#define GL_COMBINE                             0x8570
#define GL_COMBINE_RGB                         0x8571
#define GL_COMBINE_ALPHA                       0x8572
#define GL_SOURCE0_RGB                         0x8580
#define GL_SOURCE1_RGB                         0x8581
#define GL_SOURCE2_RGB                         0x8582
#define GL_SOURCE0_ALPHA                       0x8588
#define GL_SOURCE1_ALPHA                       0x8589
#define GL_SOURCE2_ALPHA                       0x858A
#define GL_OPERAND0_RGB                        0x8590
#define GL_OPERAND1_RGB                        0x8591
#define GL_OPERAND2_RGB                        0x8592
#define GL_OPERAND0_ALPHA                      0x8598
#define GL_OPERAND1_ALPHA                      0x8599
#define GL_OPERAND2_ALPHA                      0x859A
#define GL_RGB_SCALE                           0x8573
#define GL_ADD_SIGNED                          0x8574
#define GL_INTERPOLATE                         0x8575
#define GL_SUBTRACT                            0x84E7
#define GL_CONSTANT                            0x8576
#define GL_PRIMARY_COLOR                       0x8577
#define GL_PREVIOUS                            0x8578
#define GL_DOT3_RGB                            0x86AE
#define GL_DOT3_RGBA                           0x86AF
typedef void (APIENTRYP pfn_glActiveTexture) (GLenum);
extern pfn_glActiveTexture fp_glActiveTexture;
typedef void (APIENTRYP pfn_glSampleCoverage) (GLfloat, GLboolean);
extern pfn_glSampleCoverage fp_glSampleCoverage;
typedef void (APIENTRYP pfn_glCompressedTexImage3D) (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const void*);
extern pfn_glCompressedTexImage3D fp_glCompressedTexImage3D;
typedef void (APIENTRYP pfn_glCompressedTexImage2D) (GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void*);
extern pfn_glCompressedTexImage2D fp_glCompressedTexImage2D;
typedef void (APIENTRYP pfn_glCompressedTexImage1D) (GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const void*);
extern pfn_glCompressedTexImage1D fp_glCompressedTexImage1D;
typedef void (APIENTRYP pfn_glCompressedTexSubImage3D) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void*);
extern pfn_glCompressedTexSubImage3D fp_glCompressedTexSubImage3D;
typedef void (APIENTRYP pfn_glCompressedTexSubImage2D) (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void*);
extern pfn_glCompressedTexSubImage2D fp_glCompressedTexSubImage2D;
typedef void (APIENTRYP pfn_glCompressedTexSubImage1D) (GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const void*);
extern pfn_glCompressedTexSubImage1D fp_glCompressedTexSubImage1D;
typedef void (APIENTRYP pfn_glGetCompressedTexImage) (GLenum, GLint, void*);
extern pfn_glGetCompressedTexImage fp_glGetCompressedTexImage;
typedef void (APIENTRYP pfn_glClientActiveTexture) (GLenum);
extern pfn_glClientActiveTexture fp_glClientActiveTexture;
typedef void (APIENTRYP pfn_glMultiTexCoord1d) (GLenum, GLdouble);
extern pfn_glMultiTexCoord1d fp_glMultiTexCoord1d;
typedef void (APIENTRYP pfn_glMultiTexCoord1dv) (GLenum, const GLdouble*);
extern pfn_glMultiTexCoord1dv fp_glMultiTexCoord1dv;
typedef void (APIENTRYP pfn_glMultiTexCoord1f) (GLenum, GLfloat);
extern pfn_glMultiTexCoord1f fp_glMultiTexCoord1f;
typedef void (APIENTRYP pfn_glMultiTexCoord1fv) (GLenum, const GLfloat*);
extern pfn_glMultiTexCoord1fv fp_glMultiTexCoord1fv;
typedef void (APIENTRYP pfn_glMultiTexCoord1i) (GLenum, GLint);
extern pfn_glMultiTexCoord1i fp_glMultiTexCoord1i;
typedef void (APIENTRYP pfn_glMultiTexCoord1iv) (GLenum, const GLint*);
extern pfn_glMultiTexCoord1iv fp_glMultiTexCoord1iv;
typedef void (APIENTRYP pfn_glMultiTexCoord1s) (GLenum, GLshort);
extern pfn_glMultiTexCoord1s fp_glMultiTexCoord1s;
typedef void (APIENTRYP pfn_glMultiTexCoord1sv) (GLenum, const GLshort*);
extern pfn_glMultiTexCoord1sv fp_glMultiTexCoord1sv;
typedef void (APIENTRYP pfn_glMultiTexCoord2d) (GLenum, GLdouble, GLdouble);
extern pfn_glMultiTexCoord2d fp_glMultiTexCoord2d;
typedef void (APIENTRYP pfn_glMultiTexCoord2dv) (GLenum, const GLdouble*);
extern pfn_glMultiTexCoord2dv fp_glMultiTexCoord2dv;
typedef void (APIENTRYP pfn_glMultiTexCoord2f) (GLenum, GLfloat, GLfloat);
extern pfn_glMultiTexCoord2f fp_glMultiTexCoord2f;
typedef void (APIENTRYP pfn_glMultiTexCoord2fv) (GLenum, const GLfloat*);
extern pfn_glMultiTexCoord2fv fp_glMultiTexCoord2fv;
typedef void (APIENTRYP pfn_glMultiTexCoord2i) (GLenum, GLint, GLint);
extern pfn_glMultiTexCoord2i fp_glMultiTexCoord2i;
typedef void (APIENTRYP pfn_glMultiTexCoord2iv) (GLenum, const GLint*);
extern pfn_glMultiTexCoord2iv fp_glMultiTexCoord2iv;
typedef void (APIENTRYP pfn_glMultiTexCoord2s) (GLenum, GLshort, GLshort);
extern pfn_glMultiTexCoord2s fp_glMultiTexCoord2s;
typedef void (APIENTRYP pfn_glMultiTexCoord2sv) (GLenum, const GLshort*);
extern pfn_glMultiTexCoord2sv fp_glMultiTexCoord2sv;
typedef void (APIENTRYP pfn_glMultiTexCoord3d) (GLenum, GLdouble, GLdouble, GLdouble);
extern pfn_glMultiTexCoord3d fp_glMultiTexCoord3d;
typedef void (APIENTRYP pfn_glMultiTexCoord3dv) (GLenum, const GLdouble*);
extern pfn_glMultiTexCoord3dv fp_glMultiTexCoord3dv;
typedef void (APIENTRYP pfn_glMultiTexCoord3f) (GLenum, GLfloat, GLfloat, GLfloat);
extern pfn_glMultiTexCoord3f fp_glMultiTexCoord3f;
typedef void (APIENTRYP pfn_glMultiTexCoord3fv) (GLenum, const GLfloat*);
extern pfn_glMultiTexCoord3fv fp_glMultiTexCoord3fv;
typedef void (APIENTRYP pfn_glMultiTexCoord3i) (GLenum, GLint, GLint, GLint);
extern pfn_glMultiTexCoord3i fp_glMultiTexCoord3i;
typedef void (APIENTRYP pfn_glMultiTexCoord3iv) (GLenum, const GLint*);
extern pfn_glMultiTexCoord3iv fp_glMultiTexCoord3iv;
typedef void (APIENTRYP pfn_glMultiTexCoord3s) (GLenum, GLshort, GLshort, GLshort);
extern pfn_glMultiTexCoord3s fp_glMultiTexCoord3s;
typedef void (APIENTRYP pfn_glMultiTexCoord3sv) (GLenum, const GLshort*);
extern pfn_glMultiTexCoord3sv fp_glMultiTexCoord3sv;
typedef void (APIENTRYP pfn_glMultiTexCoord4d) (GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glMultiTexCoord4d fp_glMultiTexCoord4d;
typedef void (APIENTRYP pfn_glMultiTexCoord4dv) (GLenum, const GLdouble*);
extern pfn_glMultiTexCoord4dv fp_glMultiTexCoord4dv;
typedef void (APIENTRYP pfn_glMultiTexCoord4f) (GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glMultiTexCoord4f fp_glMultiTexCoord4f;
typedef void (APIENTRYP pfn_glMultiTexCoord4fv) (GLenum, const GLfloat*);
extern pfn_glMultiTexCoord4fv fp_glMultiTexCoord4fv;
typedef void (APIENTRYP pfn_glMultiTexCoord4i) (GLenum, GLint, GLint, GLint, GLint);
extern pfn_glMultiTexCoord4i fp_glMultiTexCoord4i;
typedef void (APIENTRYP pfn_glMultiTexCoord4iv) (GLenum, const GLint*);
extern pfn_glMultiTexCoord4iv fp_glMultiTexCoord4iv;
typedef void (APIENTRYP pfn_glMultiTexCoord4s) (GLenum, GLshort, GLshort, GLshort, GLshort);
extern pfn_glMultiTexCoord4s fp_glMultiTexCoord4s;
typedef void (APIENTRYP pfn_glMultiTexCoord4sv) (GLenum, const GLshort*);
extern pfn_glMultiTexCoord4sv fp_glMultiTexCoord4sv;
typedef void (APIENTRYP pfn_glLoadTransposeMatrixf) (const GLfloat*);
extern pfn_glLoadTransposeMatrixf fp_glLoadTransposeMatrixf;
typedef void (APIENTRYP pfn_glLoadTransposeMatrixd) (const GLdouble*);
extern pfn_glLoadTransposeMatrixd fp_glLoadTransposeMatrixd;
typedef void (APIENTRYP pfn_glMultTransposeMatrixf) (const GLfloat*);
extern pfn_glMultTransposeMatrixf fp_glMultTransposeMatrixf;
typedef void (APIENTRYP pfn_glMultTransposeMatrixd) (const GLdouble*);
extern pfn_glMultTransposeMatrixd fp_glMultTransposeMatrixd;

 /* GL_VERSION_1_4 */
extern GLboolean GLAD_VERSION_1_4;
#define GL_BLEND_DST_RGB                       0x80C8
#define GL_BLEND_SRC_RGB                       0x80C9
#define GL_BLEND_DST_ALPHA                     0x80CA
#define GL_BLEND_SRC_ALPHA                     0x80CB
#define GL_POINT_FADE_THRESHOLD_SIZE           0x8128
#define GL_DEPTH_COMPONENT16                   0x81A5
#define GL_DEPTH_COMPONENT24                   0x81A6
#define GL_DEPTH_COMPONENT32                   0x81A7
#define GL_MIRRORED_REPEAT                     0x8370
#define GL_MAX_TEXTURE_LOD_BIAS                0x84FD
#define GL_TEXTURE_LOD_BIAS                    0x8501
#define GL_INCR_WRAP                           0x8507
#define GL_DECR_WRAP                           0x8508
#define GL_TEXTURE_DEPTH_SIZE                  0x884A
#define GL_TEXTURE_COMPARE_MODE                0x884C
#define GL_TEXTURE_COMPARE_FUNC                0x884D
#define GL_POINT_SIZE_MIN                      0x8126
#define GL_POINT_SIZE_MAX                      0x8127
#define GL_POINT_DISTANCE_ATTENUATION          0x8129
#define GL_GENERATE_MIPMAP                     0x8191
#define GL_GENERATE_MIPMAP_HINT                0x8192
#define GL_FOG_COORDINATE_SOURCE               0x8450
#define GL_FOG_COORDINATE                      0x8451
#define GL_FRAGMENT_DEPTH                      0x8452
#define GL_CURRENT_FOG_COORDINATE              0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE           0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE         0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER        0x8456
#define GL_FOG_COORDINATE_ARRAY                0x8457
#define GL_COLOR_SUM                           0x8458
#define GL_CURRENT_SECONDARY_COLOR             0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE          0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE          0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE        0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER       0x845D
#define GL_SECONDARY_COLOR_ARRAY               0x845E
#define GL_TEXTURE_FILTER_CONTROL              0x8500
#define GL_DEPTH_TEXTURE_MODE                  0x884B
#define GL_COMPARE_R_TO_TEXTURE                0x884E
#define GL_FUNC_ADD                            0x8006
#define GL_FUNC_SUBTRACT                       0x800A
#define GL_FUNC_REVERSE_SUBTRACT               0x800B
#define GL_MIN                                 0x8007
#define GL_MAX                                 0x8008
#define GL_CONSTANT_COLOR                      0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR            0x8002
#define GL_CONSTANT_ALPHA                      0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA            0x8004
typedef void (APIENTRYP pfn_glBlendFuncSeparate) (GLenum, GLenum, GLenum, GLenum);
extern pfn_glBlendFuncSeparate fp_glBlendFuncSeparate;
typedef void (APIENTRYP pfn_glMultiDrawArrays) (GLenum, const GLint*, const GLsizei*, GLsizei);
extern pfn_glMultiDrawArrays fp_glMultiDrawArrays;
typedef void (APIENTRYP pfn_glMultiDrawElements) (GLenum, const GLsizei*, GLenum, const void**, GLsizei);
extern pfn_glMultiDrawElements fp_glMultiDrawElements;
typedef void (APIENTRYP pfn_glPointParameterf) (GLenum, GLfloat);
extern pfn_glPointParameterf fp_glPointParameterf;
typedef void (APIENTRYP pfn_glPointParameterfv) (GLenum, const GLfloat*);
extern pfn_glPointParameterfv fp_glPointParameterfv;
typedef void (APIENTRYP pfn_glPointParameteri) (GLenum, GLint);
extern pfn_glPointParameteri fp_glPointParameteri;
typedef void (APIENTRYP pfn_glPointParameteriv) (GLenum, const GLint*);
extern pfn_glPointParameteriv fp_glPointParameteriv;
typedef void (APIENTRYP pfn_glFogCoordf) (GLfloat);
extern pfn_glFogCoordf fp_glFogCoordf;
typedef void (APIENTRYP pfn_glFogCoordfv) (const GLfloat*);
extern pfn_glFogCoordfv fp_glFogCoordfv;
typedef void (APIENTRYP pfn_glFogCoordd) (GLdouble);
extern pfn_glFogCoordd fp_glFogCoordd;
typedef void (APIENTRYP pfn_glFogCoorddv) (const GLdouble*);
extern pfn_glFogCoorddv fp_glFogCoorddv;
typedef void (APIENTRYP pfn_glFogCoordPointer) (GLenum, GLsizei, const void*);
extern pfn_glFogCoordPointer fp_glFogCoordPointer;
typedef void (APIENTRYP pfn_glSecondaryColor3b) (GLbyte, GLbyte, GLbyte);
extern pfn_glSecondaryColor3b fp_glSecondaryColor3b;
typedef void (APIENTRYP pfn_glSecondaryColor3bv) (const GLbyte*);
extern pfn_glSecondaryColor3bv fp_glSecondaryColor3bv;
typedef void (APIENTRYP pfn_glSecondaryColor3d) (GLdouble, GLdouble, GLdouble);
extern pfn_glSecondaryColor3d fp_glSecondaryColor3d;
typedef void (APIENTRYP pfn_glSecondaryColor3dv) (const GLdouble*);
extern pfn_glSecondaryColor3dv fp_glSecondaryColor3dv;
typedef void (APIENTRYP pfn_glSecondaryColor3f) (GLfloat, GLfloat, GLfloat);
extern pfn_glSecondaryColor3f fp_glSecondaryColor3f;
typedef void (APIENTRYP pfn_glSecondaryColor3fv) (const GLfloat*);
extern pfn_glSecondaryColor3fv fp_glSecondaryColor3fv;
typedef void (APIENTRYP pfn_glSecondaryColor3i) (GLint, GLint, GLint);
extern pfn_glSecondaryColor3i fp_glSecondaryColor3i;
typedef void (APIENTRYP pfn_glSecondaryColor3iv) (const GLint*);
extern pfn_glSecondaryColor3iv fp_glSecondaryColor3iv;
typedef void (APIENTRYP pfn_glSecondaryColor3s) (GLshort, GLshort, GLshort);
extern pfn_glSecondaryColor3s fp_glSecondaryColor3s;
typedef void (APIENTRYP pfn_glSecondaryColor3sv) (const GLshort*);
extern pfn_glSecondaryColor3sv fp_glSecondaryColor3sv;
typedef void (APIENTRYP pfn_glSecondaryColor3ub) (GLubyte, GLubyte, GLubyte);
extern pfn_glSecondaryColor3ub fp_glSecondaryColor3ub;
typedef void (APIENTRYP pfn_glSecondaryColor3ubv) (const GLubyte*);
extern pfn_glSecondaryColor3ubv fp_glSecondaryColor3ubv;
typedef void (APIENTRYP pfn_glSecondaryColor3ui) (GLuint, GLuint, GLuint);
extern pfn_glSecondaryColor3ui fp_glSecondaryColor3ui;
typedef void (APIENTRYP pfn_glSecondaryColor3uiv) (const GLuint*);
extern pfn_glSecondaryColor3uiv fp_glSecondaryColor3uiv;
typedef void (APIENTRYP pfn_glSecondaryColor3us) (GLushort, GLushort, GLushort);
extern pfn_glSecondaryColor3us fp_glSecondaryColor3us;
typedef void (APIENTRYP pfn_glSecondaryColor3usv) (const GLushort*);
extern pfn_glSecondaryColor3usv fp_glSecondaryColor3usv;
typedef void (APIENTRYP pfn_glSecondaryColorPointer) (GLint, GLenum, GLsizei, const void*);
extern pfn_glSecondaryColorPointer fp_glSecondaryColorPointer;
typedef void (APIENTRYP pfn_glWindowPos2d) (GLdouble, GLdouble);
extern pfn_glWindowPos2d fp_glWindowPos2d;
typedef void (APIENTRYP pfn_glWindowPos2dv) (const GLdouble*);
extern pfn_glWindowPos2dv fp_glWindowPos2dv;
typedef void (APIENTRYP pfn_glWindowPos2f) (GLfloat, GLfloat);
extern pfn_glWindowPos2f fp_glWindowPos2f;
typedef void (APIENTRYP pfn_glWindowPos2fv) (const GLfloat*);
extern pfn_glWindowPos2fv fp_glWindowPos2fv;
typedef void (APIENTRYP pfn_glWindowPos2i) (GLint, GLint);
extern pfn_glWindowPos2i fp_glWindowPos2i;
typedef void (APIENTRYP pfn_glWindowPos2iv) (const GLint*);
extern pfn_glWindowPos2iv fp_glWindowPos2iv;
typedef void (APIENTRYP pfn_glWindowPos2s) (GLshort, GLshort);
extern pfn_glWindowPos2s fp_glWindowPos2s;
typedef void (APIENTRYP pfn_glWindowPos2sv) (const GLshort*);
extern pfn_glWindowPos2sv fp_glWindowPos2sv;
typedef void (APIENTRYP pfn_glWindowPos3d) (GLdouble, GLdouble, GLdouble);
extern pfn_glWindowPos3d fp_glWindowPos3d;
typedef void (APIENTRYP pfn_glWindowPos3dv) (const GLdouble*);
extern pfn_glWindowPos3dv fp_glWindowPos3dv;
typedef void (APIENTRYP pfn_glWindowPos3f) (GLfloat, GLfloat, GLfloat);
extern pfn_glWindowPos3f fp_glWindowPos3f;
typedef void (APIENTRYP pfn_glWindowPos3fv) (const GLfloat*);
extern pfn_glWindowPos3fv fp_glWindowPos3fv;
typedef void (APIENTRYP pfn_glWindowPos3i) (GLint, GLint, GLint);
extern pfn_glWindowPos3i fp_glWindowPos3i;
typedef void (APIENTRYP pfn_glWindowPos3iv) (const GLint*);
extern pfn_glWindowPos3iv fp_glWindowPos3iv;
typedef void (APIENTRYP pfn_glWindowPos3s) (GLshort, GLshort, GLshort);
extern pfn_glWindowPos3s fp_glWindowPos3s;
typedef void (APIENTRYP pfn_glWindowPos3sv) (const GLshort*);
extern pfn_glWindowPos3sv fp_glWindowPos3sv;
typedef void (APIENTRYP pfn_glBlendColor) (GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glBlendColor fp_glBlendColor;
typedef void (APIENTRYP pfn_glBlendEquation) (GLenum);
extern pfn_glBlendEquation fp_glBlendEquation;

 /* GL_VERSION_1_5 */
extern GLboolean GLAD_VERSION_1_5;
#define GL_BUFFER_SIZE                         0x8764
#define GL_BUFFER_USAGE                        0x8765
#define GL_QUERY_COUNTER_BITS                  0x8864
#define GL_CURRENT_QUERY                       0x8865
#define GL_QUERY_RESULT                        0x8866
#define GL_QUERY_RESULT_AVAILABLE              0x8867
#define GL_ARRAY_BUFFER                        0x8892
#define GL_ELEMENT_ARRAY_BUFFER                0x8893
#define GL_ARRAY_BUFFER_BINDING                0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING        0x8895
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING  0x889F
#define GL_READ_ONLY                           0x88B8
#define GL_WRITE_ONLY                          0x88B9
#define GL_READ_WRITE                          0x88BA
#define GL_BUFFER_ACCESS                       0x88BB
#define GL_BUFFER_MAPPED                       0x88BC
#define GL_BUFFER_MAP_POINTER                  0x88BD
#define GL_STREAM_DRAW                         0x88E0
#define GL_STREAM_READ                         0x88E1
#define GL_STREAM_COPY                         0x88E2
#define GL_STATIC_DRAW                         0x88E4
#define GL_STATIC_READ                         0x88E5
#define GL_STATIC_COPY                         0x88E6
#define GL_DYNAMIC_DRAW                        0x88E8
#define GL_DYNAMIC_READ                        0x88E9
#define GL_DYNAMIC_COPY                        0x88EA
#define GL_SAMPLES_PASSED                      0x8914
#define GL_SRC1_ALPHA                          0x8589
#define GL_VERTEX_ARRAY_BUFFER_BINDING         0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING         0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING          0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING          0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING  0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING      0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING  0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING  0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING         0x889E
#define GL_FOG_COORD_SRC                       0x8450
#define GL_FOG_COORD                           0x8451
#define GL_CURRENT_FOG_COORD                   0x8453
#define GL_FOG_COORD_ARRAY_TYPE                0x8454
#define GL_FOG_COORD_ARRAY_STRIDE              0x8455
#define GL_FOG_COORD_ARRAY_POINTER             0x8456
#define GL_FOG_COORD_ARRAY                     0x8457
#define GL_FOG_COORD_ARRAY_BUFFER_BINDING      0x889D
#define GL_SRC0_RGB                            0x8580
#define GL_SRC1_RGB                            0x8581
#define GL_SRC2_RGB                            0x8582
#define GL_SRC0_ALPHA                          0x8588
#define GL_SRC2_ALPHA                          0x858A
typedef void (APIENTRYP pfn_glGenQueries) (GLsizei, GLuint*);
extern pfn_glGenQueries fp_glGenQueries;
typedef void (APIENTRYP pfn_glDeleteQueries) (GLsizei, const GLuint*);
extern pfn_glDeleteQueries fp_glDeleteQueries;
typedef GLboolean (APIENTRYP pfn_glIsQuery) (GLuint);
extern pfn_glIsQuery fp_glIsQuery;
typedef void (APIENTRYP pfn_glBeginQuery) (GLenum, GLuint);
extern pfn_glBeginQuery fp_glBeginQuery;
typedef void (APIENTRYP pfn_glEndQuery) (GLenum);
extern pfn_glEndQuery fp_glEndQuery;
typedef void (APIENTRYP pfn_glGetQueryiv) (GLenum, GLenum, GLint*);
extern pfn_glGetQueryiv fp_glGetQueryiv;
typedef void (APIENTRYP pfn_glGetQueryObjectiv) (GLuint, GLenum, GLint*);
extern pfn_glGetQueryObjectiv fp_glGetQueryObjectiv;
typedef void (APIENTRYP pfn_glGetQueryObjectuiv) (GLuint, GLenum, GLuint*);
extern pfn_glGetQueryObjectuiv fp_glGetQueryObjectuiv;
typedef void (APIENTRYP pfn_glBindBuffer) (GLenum, GLuint);
extern pfn_glBindBuffer fp_glBindBuffer;
typedef void (APIENTRYP pfn_glDeleteBuffers) (GLsizei, const GLuint*);
extern pfn_glDeleteBuffers fp_glDeleteBuffers;
typedef void (APIENTRYP pfn_glGenBuffers) (GLsizei, GLuint*);
extern pfn_glGenBuffers fp_glGenBuffers;
typedef GLboolean (APIENTRYP pfn_glIsBuffer) (GLuint);
extern pfn_glIsBuffer fp_glIsBuffer;
typedef void (APIENTRYP pfn_glBufferData) (GLenum, GLsizeiptr, const void*, GLenum);
extern pfn_glBufferData fp_glBufferData;
typedef void (APIENTRYP pfn_glBufferSubData) (GLenum, GLintptr, GLsizeiptr, const void*);
extern pfn_glBufferSubData fp_glBufferSubData;
typedef void (APIENTRYP pfn_glGetBufferSubData) (GLenum, GLintptr, GLsizeiptr, void*);
extern pfn_glGetBufferSubData fp_glGetBufferSubData;
typedef void* (APIENTRYP pfn_glMapBuffer) (GLenum, GLenum);
extern pfn_glMapBuffer fp_glMapBuffer;
typedef GLboolean (APIENTRYP pfn_glUnmapBuffer) (GLenum);
extern pfn_glUnmapBuffer fp_glUnmapBuffer;
typedef void (APIENTRYP pfn_glGetBufferParameteriv) (GLenum, GLenum, GLint*);
extern pfn_glGetBufferParameteriv fp_glGetBufferParameteriv;
typedef void (APIENTRYP pfn_glGetBufferPointerv) (GLenum, GLenum, void**);
extern pfn_glGetBufferPointerv fp_glGetBufferPointerv;

 /* GL_VERSION_2_0 */
extern GLboolean GLAD_VERSION_2_0;
#define GL_BLEND_EQUATION_RGB                  0x8009
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED         0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE            0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE          0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE            0x8625
#define GL_CURRENT_VERTEX_ATTRIB               0x8626
#define GL_VERTEX_PROGRAM_POINT_SIZE           0x8642
#define GL_VERTEX_ATTRIB_ARRAY_POINTER         0x8645
#define GL_STENCIL_BACK_FUNC                   0x8800
#define GL_STENCIL_BACK_FAIL                   0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL        0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS        0x8803
#define GL_MAX_DRAW_BUFFERS                    0x8824
#define GL_DRAW_BUFFER0                        0x8825
#define GL_DRAW_BUFFER1                        0x8826
#define GL_DRAW_BUFFER2                        0x8827
#define GL_DRAW_BUFFER3                        0x8828
#define GL_DRAW_BUFFER4                        0x8829
#define GL_DRAW_BUFFER5                        0x882A
#define GL_DRAW_BUFFER6                        0x882B
#define GL_DRAW_BUFFER7                        0x882C
#define GL_DRAW_BUFFER8                        0x882D
#define GL_DRAW_BUFFER9                        0x882E
#define GL_DRAW_BUFFER10                       0x882F
#define GL_DRAW_BUFFER11                       0x8830
#define GL_DRAW_BUFFER12                       0x8831
#define GL_DRAW_BUFFER13                       0x8832
#define GL_DRAW_BUFFER14                       0x8833
#define GL_DRAW_BUFFER15                       0x8834
#define GL_BLEND_EQUATION_ALPHA                0x883D
#define GL_MAX_VERTEX_ATTRIBS                  0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED      0x886A
#define GL_MAX_TEXTURE_IMAGE_UNITS             0x8872
#define GL_FRAGMENT_SHADER                     0x8B30
#define GL_VERTEX_SHADER                       0x8B31
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS     0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS       0x8B4A
#define GL_MAX_VARYING_FLOATS                  0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS      0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS    0x8B4D
#define GL_SHADER_TYPE                         0x8B4F
#define GL_FLOAT_VEC2                          0x8B50
#define GL_FLOAT_VEC3                          0x8B51
#define GL_FLOAT_VEC4                          0x8B52
#define GL_INT_VEC2                            0x8B53
#define GL_INT_VEC3                            0x8B54
#define GL_INT_VEC4                            0x8B55
#define GL_BOOL                                0x8B56
#define GL_BOOL_VEC2                           0x8B57
#define GL_BOOL_VEC3                           0x8B58
#define GL_BOOL_VEC4                           0x8B59
#define GL_FLOAT_MAT2                          0x8B5A
#define GL_FLOAT_MAT3                          0x8B5B
#define GL_FLOAT_MAT4                          0x8B5C
#define GL_SAMPLER_1D                          0x8B5D
#define GL_SAMPLER_2D                          0x8B5E
#define GL_SAMPLER_3D                          0x8B5F
#define GL_SAMPLER_CUBE                        0x8B60
#define GL_SAMPLER_1D_SHADOW                   0x8B61
#define GL_SAMPLER_2D_SHADOW                   0x8B62
#define GL_DELETE_STATUS                       0x8B80
#define GL_COMPILE_STATUS                      0x8B81
#define GL_LINK_STATUS                         0x8B82
#define GL_VALIDATE_STATUS                     0x8B83
#define GL_INFO_LOG_LENGTH                     0x8B84
#define GL_ATTACHED_SHADERS                    0x8B85
#define GL_ACTIVE_UNIFORMS                     0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH           0x8B87
#define GL_SHADER_SOURCE_LENGTH                0x8B88
#define GL_ACTIVE_ATTRIBUTES                   0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH         0x8B8A
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT     0x8B8B
#define GL_SHADING_LANGUAGE_VERSION            0x8B8C
#define GL_CURRENT_PROGRAM                     0x8B8D
#define GL_POINT_SPRITE_COORD_ORIGIN           0x8CA0
#define GL_LOWER_LEFT                          0x8CA1
#define GL_UPPER_LEFT                          0x8CA2
#define GL_STENCIL_BACK_REF                    0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK             0x8CA4
#define GL_STENCIL_BACK_WRITEMASK              0x8CA5
#define GL_VERTEX_PROGRAM_TWO_SIDE             0x8643
#define GL_POINT_SPRITE                        0x8861
#define GL_COORD_REPLACE                       0x8862
#define GL_MAX_TEXTURE_COORDS                  0x8871
typedef void (APIENTRYP pfn_glBlendEquationSeparate) (GLenum, GLenum);
extern pfn_glBlendEquationSeparate fp_glBlendEquationSeparate;
typedef void (APIENTRYP pfn_glDrawBuffers) (GLsizei, const GLenum*);
extern pfn_glDrawBuffers fp_glDrawBuffers;
typedef void (APIENTRYP pfn_glStencilOpSeparate) (GLenum, GLenum, GLenum, GLenum);
extern pfn_glStencilOpSeparate fp_glStencilOpSeparate;
typedef void (APIENTRYP pfn_glStencilFuncSeparate) (GLenum, GLenum, GLint, GLuint);
extern pfn_glStencilFuncSeparate fp_glStencilFuncSeparate;
typedef void (APIENTRYP pfn_glStencilMaskSeparate) (GLenum, GLuint);
extern pfn_glStencilMaskSeparate fp_glStencilMaskSeparate;
typedef void (APIENTRYP pfn_glAttachShader) (GLuint, GLuint);
extern pfn_glAttachShader fp_glAttachShader;
typedef void (APIENTRYP pfn_glBindAttribLocation) (GLuint, GLuint, const GLchar*);
extern pfn_glBindAttribLocation fp_glBindAttribLocation;
typedef void (APIENTRYP pfn_glCompileShader) (GLuint);
extern pfn_glCompileShader fp_glCompileShader;
typedef GLuint (APIENTRYP pfn_glCreateProgram) ();
extern pfn_glCreateProgram fp_glCreateProgram;
typedef GLuint (APIENTRYP pfn_glCreateShader) (GLenum);
extern pfn_glCreateShader fp_glCreateShader;
typedef void (APIENTRYP pfn_glDeleteProgram) (GLuint);
extern pfn_glDeleteProgram fp_glDeleteProgram;
typedef void (APIENTRYP pfn_glDeleteShader) (GLuint);
extern pfn_glDeleteShader fp_glDeleteShader;
typedef void (APIENTRYP pfn_glDetachShader) (GLuint, GLuint);
extern pfn_glDetachShader fp_glDetachShader;
typedef void (APIENTRYP pfn_glDisableVertexAttribArray) (GLuint);
extern pfn_glDisableVertexAttribArray fp_glDisableVertexAttribArray;
typedef void (APIENTRYP pfn_glEnableVertexAttribArray) (GLuint);
extern pfn_glEnableVertexAttribArray fp_glEnableVertexAttribArray;
typedef void (APIENTRYP pfn_glGetActiveAttrib) (GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*);
extern pfn_glGetActiveAttrib fp_glGetActiveAttrib;
typedef void (APIENTRYP pfn_glGetActiveUniform) (GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*);
extern pfn_glGetActiveUniform fp_glGetActiveUniform;
typedef void (APIENTRYP pfn_glGetAttachedShaders) (GLuint, GLsizei, GLsizei*, GLuint*);
extern pfn_glGetAttachedShaders fp_glGetAttachedShaders;
typedef GLint (APIENTRYP pfn_glGetAttribLocation) (GLuint, const GLchar*);
extern pfn_glGetAttribLocation fp_glGetAttribLocation;
typedef void (APIENTRYP pfn_glGetProgramiv) (GLuint, GLenum, GLint*);
extern pfn_glGetProgramiv fp_glGetProgramiv;
typedef void (APIENTRYP pfn_glGetProgramInfoLog) (GLuint, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetProgramInfoLog fp_glGetProgramInfoLog;
typedef void (APIENTRYP pfn_glGetShaderiv) (GLuint, GLenum, GLint*);
extern pfn_glGetShaderiv fp_glGetShaderiv;
typedef void (APIENTRYP pfn_glGetShaderInfoLog) (GLuint, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetShaderInfoLog fp_glGetShaderInfoLog;
typedef void (APIENTRYP pfn_glGetShaderSource) (GLuint, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetShaderSource fp_glGetShaderSource;
typedef GLint (APIENTRYP pfn_glGetUniformLocation) (GLuint, const GLchar*);
extern pfn_glGetUniformLocation fp_glGetUniformLocation;
typedef void (APIENTRYP pfn_glGetUniformfv) (GLuint, GLint, GLfloat*);
extern pfn_glGetUniformfv fp_glGetUniformfv;
typedef void (APIENTRYP pfn_glGetUniformiv) (GLuint, GLint, GLint*);
extern pfn_glGetUniformiv fp_glGetUniformiv;
typedef void (APIENTRYP pfn_glGetVertexAttribdv) (GLuint, GLenum, GLdouble*);
extern pfn_glGetVertexAttribdv fp_glGetVertexAttribdv;
typedef void (APIENTRYP pfn_glGetVertexAttribfv) (GLuint, GLenum, GLfloat*);
extern pfn_glGetVertexAttribfv fp_glGetVertexAttribfv;
typedef void (APIENTRYP pfn_glGetVertexAttribiv) (GLuint, GLenum, GLint*);
extern pfn_glGetVertexAttribiv fp_glGetVertexAttribiv;
typedef void (APIENTRYP pfn_glGetVertexAttribPointerv) (GLuint, GLenum, void**);
extern pfn_glGetVertexAttribPointerv fp_glGetVertexAttribPointerv;
typedef GLboolean (APIENTRYP pfn_glIsProgram) (GLuint);
extern pfn_glIsProgram fp_glIsProgram;
typedef GLboolean (APIENTRYP pfn_glIsShader) (GLuint);
extern pfn_glIsShader fp_glIsShader;
typedef void (APIENTRYP pfn_glLinkProgram) (GLuint);
extern pfn_glLinkProgram fp_glLinkProgram;
typedef void (APIENTRYP pfn_glShaderSource) (GLuint, GLsizei, const GLchar**, const GLint*);
extern pfn_glShaderSource fp_glShaderSource;
typedef void (APIENTRYP pfn_glUseProgram) (GLuint);
extern pfn_glUseProgram fp_glUseProgram;
typedef void (APIENTRYP pfn_glUniform1f) (GLint, GLfloat);
extern pfn_glUniform1f fp_glUniform1f;
typedef void (APIENTRYP pfn_glUniform2f) (GLint, GLfloat, GLfloat);
extern pfn_glUniform2f fp_glUniform2f;
typedef void (APIENTRYP pfn_glUniform3f) (GLint, GLfloat, GLfloat, GLfloat);
extern pfn_glUniform3f fp_glUniform3f;
typedef void (APIENTRYP pfn_glUniform4f) (GLint, GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glUniform4f fp_glUniform4f;
typedef void (APIENTRYP pfn_glUniform1i) (GLint, GLint);
extern pfn_glUniform1i fp_glUniform1i;
typedef void (APIENTRYP pfn_glUniform2i) (GLint, GLint, GLint);
extern pfn_glUniform2i fp_glUniform2i;
typedef void (APIENTRYP pfn_glUniform3i) (GLint, GLint, GLint, GLint);
extern pfn_glUniform3i fp_glUniform3i;
typedef void (APIENTRYP pfn_glUniform4i) (GLint, GLint, GLint, GLint, GLint);
extern pfn_glUniform4i fp_glUniform4i;
typedef void (APIENTRYP pfn_glUniform1fv) (GLint, GLsizei, const GLfloat*);
extern pfn_glUniform1fv fp_glUniform1fv;
typedef void (APIENTRYP pfn_glUniform2fv) (GLint, GLsizei, const GLfloat*);
extern pfn_glUniform2fv fp_glUniform2fv;
typedef void (APIENTRYP pfn_glUniform3fv) (GLint, GLsizei, const GLfloat*);
extern pfn_glUniform3fv fp_glUniform3fv;
typedef void (APIENTRYP pfn_glUniform4fv) (GLint, GLsizei, const GLfloat*);
extern pfn_glUniform4fv fp_glUniform4fv;
typedef void (APIENTRYP pfn_glUniform1iv) (GLint, GLsizei, const GLint*);
extern pfn_glUniform1iv fp_glUniform1iv;
typedef void (APIENTRYP pfn_glUniform2iv) (GLint, GLsizei, const GLint*);
extern pfn_glUniform2iv fp_glUniform2iv;
typedef void (APIENTRYP pfn_glUniform3iv) (GLint, GLsizei, const GLint*);
extern pfn_glUniform3iv fp_glUniform3iv;
typedef void (APIENTRYP pfn_glUniform4iv) (GLint, GLsizei, const GLint*);
extern pfn_glUniform4iv fp_glUniform4iv;
typedef void (APIENTRYP pfn_glUniformMatrix2fv) (GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glUniformMatrix2fv fp_glUniformMatrix2fv;
typedef void (APIENTRYP pfn_glUniformMatrix3fv) (GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glUniformMatrix3fv fp_glUniformMatrix3fv;
typedef void (APIENTRYP pfn_glUniformMatrix4fv) (GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glUniformMatrix4fv fp_glUniformMatrix4fv;
typedef void (APIENTRYP pfn_glValidateProgram) (GLuint);
extern pfn_glValidateProgram fp_glValidateProgram;
typedef void (APIENTRYP pfn_glVertexAttrib1d) (GLuint, GLdouble);
extern pfn_glVertexAttrib1d fp_glVertexAttrib1d;
typedef void (APIENTRYP pfn_glVertexAttrib1dv) (GLuint, const GLdouble*);
extern pfn_glVertexAttrib1dv fp_glVertexAttrib1dv;
typedef void (APIENTRYP pfn_glVertexAttrib1f) (GLuint, GLfloat);
extern pfn_glVertexAttrib1f fp_glVertexAttrib1f;
typedef void (APIENTRYP pfn_glVertexAttrib1fv) (GLuint, const GLfloat*);
extern pfn_glVertexAttrib1fv fp_glVertexAttrib1fv;
typedef void (APIENTRYP pfn_glVertexAttrib1s) (GLuint, GLshort);
extern pfn_glVertexAttrib1s fp_glVertexAttrib1s;
typedef void (APIENTRYP pfn_glVertexAttrib1sv) (GLuint, const GLshort*);
extern pfn_glVertexAttrib1sv fp_glVertexAttrib1sv;
typedef void (APIENTRYP pfn_glVertexAttrib2d) (GLuint, GLdouble, GLdouble);
extern pfn_glVertexAttrib2d fp_glVertexAttrib2d;
typedef void (APIENTRYP pfn_glVertexAttrib2dv) (GLuint, const GLdouble*);
extern pfn_glVertexAttrib2dv fp_glVertexAttrib2dv;
typedef void (APIENTRYP pfn_glVertexAttrib2f) (GLuint, GLfloat, GLfloat);
extern pfn_glVertexAttrib2f fp_glVertexAttrib2f;
typedef void (APIENTRYP pfn_glVertexAttrib2fv) (GLuint, const GLfloat*);
extern pfn_glVertexAttrib2fv fp_glVertexAttrib2fv;
typedef void (APIENTRYP pfn_glVertexAttrib2s) (GLuint, GLshort, GLshort);
extern pfn_glVertexAttrib2s fp_glVertexAttrib2s;
typedef void (APIENTRYP pfn_glVertexAttrib2sv) (GLuint, const GLshort*);
extern pfn_glVertexAttrib2sv fp_glVertexAttrib2sv;
typedef void (APIENTRYP pfn_glVertexAttrib3d) (GLuint, GLdouble, GLdouble, GLdouble);
extern pfn_glVertexAttrib3d fp_glVertexAttrib3d;
typedef void (APIENTRYP pfn_glVertexAttrib3dv) (GLuint, const GLdouble*);
extern pfn_glVertexAttrib3dv fp_glVertexAttrib3dv;
typedef void (APIENTRYP pfn_glVertexAttrib3f) (GLuint, GLfloat, GLfloat, GLfloat);
extern pfn_glVertexAttrib3f fp_glVertexAttrib3f;
typedef void (APIENTRYP pfn_glVertexAttrib3fv) (GLuint, const GLfloat*);
extern pfn_glVertexAttrib3fv fp_glVertexAttrib3fv;
typedef void (APIENTRYP pfn_glVertexAttrib3s) (GLuint, GLshort, GLshort, GLshort);
extern pfn_glVertexAttrib3s fp_glVertexAttrib3s;
typedef void (APIENTRYP pfn_glVertexAttrib3sv) (GLuint, const GLshort*);
extern pfn_glVertexAttrib3sv fp_glVertexAttrib3sv;
typedef void (APIENTRYP pfn_glVertexAttrib4Nbv) (GLuint, const GLbyte*);
extern pfn_glVertexAttrib4Nbv fp_glVertexAttrib4Nbv;
typedef void (APIENTRYP pfn_glVertexAttrib4Niv) (GLuint, const GLint*);
extern pfn_glVertexAttrib4Niv fp_glVertexAttrib4Niv;
typedef void (APIENTRYP pfn_glVertexAttrib4Nsv) (GLuint, const GLshort*);
extern pfn_glVertexAttrib4Nsv fp_glVertexAttrib4Nsv;
typedef void (APIENTRYP pfn_glVertexAttrib4Nub) (GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
extern pfn_glVertexAttrib4Nub fp_glVertexAttrib4Nub;
typedef void (APIENTRYP pfn_glVertexAttrib4Nubv) (GLuint, const GLubyte*);
extern pfn_glVertexAttrib4Nubv fp_glVertexAttrib4Nubv;
typedef void (APIENTRYP pfn_glVertexAttrib4Nuiv) (GLuint, const GLuint*);
extern pfn_glVertexAttrib4Nuiv fp_glVertexAttrib4Nuiv;
typedef void (APIENTRYP pfn_glVertexAttrib4Nusv) (GLuint, const GLushort*);
extern pfn_glVertexAttrib4Nusv fp_glVertexAttrib4Nusv;
typedef void (APIENTRYP pfn_glVertexAttrib4bv) (GLuint, const GLbyte*);
extern pfn_glVertexAttrib4bv fp_glVertexAttrib4bv;
typedef void (APIENTRYP pfn_glVertexAttrib4d) (GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glVertexAttrib4d fp_glVertexAttrib4d;
typedef void (APIENTRYP pfn_glVertexAttrib4dv) (GLuint, const GLdouble*);
extern pfn_glVertexAttrib4dv fp_glVertexAttrib4dv;
typedef void (APIENTRYP pfn_glVertexAttrib4f) (GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glVertexAttrib4f fp_glVertexAttrib4f;
typedef void (APIENTRYP pfn_glVertexAttrib4fv) (GLuint, const GLfloat*);
extern pfn_glVertexAttrib4fv fp_glVertexAttrib4fv;
typedef void (APIENTRYP pfn_glVertexAttrib4iv) (GLuint, const GLint*);
extern pfn_glVertexAttrib4iv fp_glVertexAttrib4iv;
typedef void (APIENTRYP pfn_glVertexAttrib4s) (GLuint, GLshort, GLshort, GLshort, GLshort);
extern pfn_glVertexAttrib4s fp_glVertexAttrib4s;
typedef void (APIENTRYP pfn_glVertexAttrib4sv) (GLuint, const GLshort*);
extern pfn_glVertexAttrib4sv fp_glVertexAttrib4sv;
typedef void (APIENTRYP pfn_glVertexAttrib4ubv) (GLuint, const GLubyte*);
extern pfn_glVertexAttrib4ubv fp_glVertexAttrib4ubv;
typedef void (APIENTRYP pfn_glVertexAttrib4uiv) (GLuint, const GLuint*);
extern pfn_glVertexAttrib4uiv fp_glVertexAttrib4uiv;
typedef void (APIENTRYP pfn_glVertexAttrib4usv) (GLuint, const GLushort*);
extern pfn_glVertexAttrib4usv fp_glVertexAttrib4usv;
typedef void (APIENTRYP pfn_glVertexAttribPointer) (GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
extern pfn_glVertexAttribPointer fp_glVertexAttribPointer;

 /* GL_VERSION_2_1 */
extern GLboolean GLAD_VERSION_2_1;
#define GL_PIXEL_PACK_BUFFER                   0x88EB
#define GL_PIXEL_UNPACK_BUFFER                 0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING           0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING         0x88EF
#define GL_FLOAT_MAT2x3                        0x8B65
#define GL_FLOAT_MAT2x4                        0x8B66
#define GL_FLOAT_MAT3x2                        0x8B67
#define GL_FLOAT_MAT3x4                        0x8B68
#define GL_FLOAT_MAT4x2                        0x8B69
#define GL_FLOAT_MAT4x3                        0x8B6A
#define GL_SRGB                                0x8C40
#define GL_SRGB8                               0x8C41
#define GL_SRGB_ALPHA                          0x8C42
#define GL_SRGB8_ALPHA8                        0x8C43
#define GL_COMPRESSED_SRGB                     0x8C48
#define GL_COMPRESSED_SRGB_ALPHA               0x8C49
#define GL_CURRENT_RASTER_SECONDARY_COLOR      0x845F
#define GL_SLUMINANCE_ALPHA                    0x8C44
#define GL_SLUMINANCE8_ALPHA8                  0x8C45
#define GL_SLUMINANCE                          0x8C46
#define GL_SLUMINANCE8                         0x8C47
#define GL_COMPRESSED_SLUMINANCE               0x8C4A
#define GL_COMPRESSED_SLUMINANCE_ALPHA         0x8C4B
typedef void (APIENTRYP pfn_glUniformMatrix2x3fv) (GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glUniformMatrix2x3fv fp_glUniformMatrix2x3fv;
typedef void (APIENTRYP pfn_glUniformMatrix3x2fv) (GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glUniformMatrix3x2fv fp_glUniformMatrix3x2fv;
typedef void (APIENTRYP pfn_glUniformMatrix2x4fv) (GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glUniformMatrix2x4fv fp_glUniformMatrix2x4fv;
typedef void (APIENTRYP pfn_glUniformMatrix4x2fv) (GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glUniformMatrix4x2fv fp_glUniformMatrix4x2fv;
typedef void (APIENTRYP pfn_glUniformMatrix3x4fv) (GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glUniformMatrix3x4fv fp_glUniformMatrix3x4fv;
typedef void (APIENTRYP pfn_glUniformMatrix4x3fv) (GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glUniformMatrix4x3fv fp_glUniformMatrix4x3fv;

 /* GL_VERSION_3_0 */
extern GLboolean GLAD_VERSION_3_0;
#define GL_COMPARE_REF_TO_TEXTURE              0x884E
#define GL_CLIP_DISTANCE0                      0x3000
#define GL_CLIP_DISTANCE1                      0x3001
#define GL_CLIP_DISTANCE2                      0x3002
#define GL_CLIP_DISTANCE3                      0x3003
#define GL_CLIP_DISTANCE4                      0x3004
#define GL_CLIP_DISTANCE5                      0x3005
#define GL_CLIP_DISTANCE6                      0x3006
#define GL_CLIP_DISTANCE7                      0x3007
#define GL_MAX_CLIP_DISTANCES                  0x0D32
#define GL_MAJOR_VERSION                       0x821B
#define GL_MINOR_VERSION                       0x821C
#define GL_NUM_EXTENSIONS                      0x821D
#define GL_CONTEXT_FLAGS                       0x821E
#define GL_COMPRESSED_RED                      0x8225
#define GL_COMPRESSED_RG                       0x8226
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT  0x00000001
#define GL_RGBA32F                             0x8814
#define GL_RGB32F                              0x8815
#define GL_RGBA16F                             0x881A
#define GL_RGB16F                              0x881B
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER         0x88FD
#define GL_MAX_ARRAY_TEXTURE_LAYERS            0x88FF
#define GL_MIN_PROGRAM_TEXEL_OFFSET            0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET            0x8905
#define GL_CLAMP_READ_COLOR                    0x891C
#define GL_FIXED_ONLY                          0x891D
#define GL_MAX_VARYING_COMPONENTS              0x8B4B
#define GL_TEXTURE_1D_ARRAY                    0x8C18
#define GL_PROXY_TEXTURE_1D_ARRAY              0x8C19
#define GL_TEXTURE_2D_ARRAY                    0x8C1A
#define GL_PROXY_TEXTURE_2D_ARRAY              0x8C1B
#define GL_TEXTURE_BINDING_1D_ARRAY            0x8C1C
#define GL_TEXTURE_BINDING_2D_ARRAY            0x8C1D
#define GL_R11F_G11F_B10F                      0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV        0x8C3B
#define GL_RGB9_E5                             0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV            0x8C3E
#define GL_TEXTURE_SHARED_SIZE                 0x8C3F
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH  0x8C76
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE      0x8C7F
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS  0x8C80
#define GL_TRANSFORM_FEEDBACK_VARYINGS         0x8C83
#define GL_TRANSFORM_FEEDBACK_BUFFER_START     0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE      0x8C85
#define GL_PRIMITIVES_GENERATED                0x8C87
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN  0x8C88
#define GL_RASTERIZER_DISCARD                  0x8C89
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS  0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS  0x8C8B
#define GL_INTERLEAVED_ATTRIBS                 0x8C8C
#define GL_SEPARATE_ATTRIBS                    0x8C8D
#define GL_TRANSFORM_FEEDBACK_BUFFER           0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING   0x8C8F
#define GL_RGBA32UI                            0x8D70
#define GL_RGB32UI                             0x8D71
#define GL_RGBA16UI                            0x8D76
#define GL_RGB16UI                             0x8D77
#define GL_RGBA8UI                             0x8D7C
#define GL_RGB8UI                              0x8D7D
#define GL_RGBA32I                             0x8D82
#define GL_RGB32I                              0x8D83
#define GL_RGBA16I                             0x8D88
#define GL_RGB16I                              0x8D89
#define GL_RGBA8I                              0x8D8E
#define GL_RGB8I                               0x8D8F
#define GL_RED_INTEGER                         0x8D94
#define GL_GREEN_INTEGER                       0x8D95
#define GL_BLUE_INTEGER                        0x8D96
#define GL_RGB_INTEGER                         0x8D98
#define GL_RGBA_INTEGER                        0x8D99
#define GL_BGR_INTEGER                         0x8D9A
#define GL_BGRA_INTEGER                        0x8D9B
#define GL_SAMPLER_1D_ARRAY                    0x8DC0
#define GL_SAMPLER_2D_ARRAY                    0x8DC1
#define GL_SAMPLER_1D_ARRAY_SHADOW             0x8DC3
#define GL_SAMPLER_2D_ARRAY_SHADOW             0x8DC4
#define GL_SAMPLER_CUBE_SHADOW                 0x8DC5
#define GL_UNSIGNED_INT_VEC2                   0x8DC6
#define GL_UNSIGNED_INT_VEC3                   0x8DC7
#define GL_UNSIGNED_INT_VEC4                   0x8DC8
#define GL_INT_SAMPLER_1D                      0x8DC9
#define GL_INT_SAMPLER_2D                      0x8DCA
#define GL_INT_SAMPLER_3D                      0x8DCB
#define GL_INT_SAMPLER_CUBE                    0x8DCC
#define GL_INT_SAMPLER_1D_ARRAY                0x8DCE
#define GL_INT_SAMPLER_2D_ARRAY                0x8DCF
#define GL_UNSIGNED_INT_SAMPLER_1D             0x8DD1
#define GL_UNSIGNED_INT_SAMPLER_2D             0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_3D             0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_CUBE           0x8DD4
#define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY       0x8DD6
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY       0x8DD7
#define GL_QUERY_WAIT                          0x8E13
#define GL_QUERY_NO_WAIT                       0x8E14
#define GL_QUERY_BY_REGION_WAIT                0x8E15
#define GL_QUERY_BY_REGION_NO_WAIT             0x8E16
#define GL_BUFFER_ACCESS_FLAGS                 0x911F
#define GL_BUFFER_MAP_LENGTH                   0x9120
#define GL_BUFFER_MAP_OFFSET                   0x9121
#define GL_DEPTH_COMPONENT32F                  0x8CAC
#define GL_DEPTH32F_STENCIL8                   0x8CAD
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV      0x8DAD
#define GL_INVALID_FRAMEBUFFER_OPERATION       0x0506
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING  0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE  0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE     0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE   0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE    0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE   0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE   0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE  0x8217
#define GL_FRAMEBUFFER_DEFAULT                 0x8218
#define GL_FRAMEBUFFER_UNDEFINED               0x8219
#define GL_DEPTH_STENCIL_ATTACHMENT            0x821A
#define GL_MAX_RENDERBUFFER_SIZE               0x84E8
#define GL_DEPTH_STENCIL                       0x84F9
#define GL_UNSIGNED_INT_24_8                   0x84FA
#define GL_DEPTH24_STENCIL8                    0x88F0
#define GL_TEXTURE_STENCIL_SIZE                0x88F1
#define GL_TEXTURE_RED_TYPE                    0x8C10
#define GL_TEXTURE_GREEN_TYPE                  0x8C11
#define GL_TEXTURE_BLUE_TYPE                   0x8C12
#define GL_TEXTURE_ALPHA_TYPE                  0x8C13
#define GL_TEXTURE_DEPTH_TYPE                  0x8C16
#define GL_UNSIGNED_NORMALIZED                 0x8C17
#define GL_FRAMEBUFFER_BINDING                 0x8CA6
#define GL_DRAW_FRAMEBUFFER_BINDING            0x8CA6
#define GL_RENDERBUFFER_BINDING                0x8CA7
#define GL_READ_FRAMEBUFFER                    0x8CA8
#define GL_DRAW_FRAMEBUFFER                    0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING            0x8CAA
#define GL_RENDERBUFFER_SAMPLES                0x8CAB
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE  0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME  0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL  0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE  0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER  0x8CD4
#define GL_FRAMEBUFFER_COMPLETE                0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT   0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT  0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER  0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER  0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED             0x8CDD
#define GL_MAX_COLOR_ATTACHMENTS               0x8CDF
#define GL_COLOR_ATTACHMENT0                   0x8CE0
#define GL_COLOR_ATTACHMENT1                   0x8CE1
#define GL_COLOR_ATTACHMENT2                   0x8CE2
#define GL_COLOR_ATTACHMENT3                   0x8CE3
#define GL_COLOR_ATTACHMENT4                   0x8CE4
#define GL_COLOR_ATTACHMENT5                   0x8CE5
#define GL_COLOR_ATTACHMENT6                   0x8CE6
#define GL_COLOR_ATTACHMENT7                   0x8CE7
#define GL_COLOR_ATTACHMENT8                   0x8CE8
#define GL_COLOR_ATTACHMENT9                   0x8CE9
#define GL_COLOR_ATTACHMENT10                  0x8CEA
#define GL_COLOR_ATTACHMENT11                  0x8CEB
#define GL_COLOR_ATTACHMENT12                  0x8CEC
#define GL_COLOR_ATTACHMENT13                  0x8CED
#define GL_COLOR_ATTACHMENT14                  0x8CEE
#define GL_COLOR_ATTACHMENT15                  0x8CEF
#define GL_COLOR_ATTACHMENT16                  0x8CF0
#define GL_COLOR_ATTACHMENT17                  0x8CF1
#define GL_COLOR_ATTACHMENT18                  0x8CF2
#define GL_COLOR_ATTACHMENT19                  0x8CF3
#define GL_COLOR_ATTACHMENT20                  0x8CF4
#define GL_COLOR_ATTACHMENT21                  0x8CF5
#define GL_COLOR_ATTACHMENT22                  0x8CF6
#define GL_COLOR_ATTACHMENT23                  0x8CF7
#define GL_COLOR_ATTACHMENT24                  0x8CF8
#define GL_COLOR_ATTACHMENT25                  0x8CF9
#define GL_COLOR_ATTACHMENT26                  0x8CFA
#define GL_COLOR_ATTACHMENT27                  0x8CFB
#define GL_COLOR_ATTACHMENT28                  0x8CFC
#define GL_COLOR_ATTACHMENT29                  0x8CFD
#define GL_COLOR_ATTACHMENT30                  0x8CFE
#define GL_COLOR_ATTACHMENT31                  0x8CFF
#define GL_DEPTH_ATTACHMENT                    0x8D00
#define GL_STENCIL_ATTACHMENT                  0x8D20
#define GL_FRAMEBUFFER                         0x8D40
#define GL_RENDERBUFFER                        0x8D41
#define GL_RENDERBUFFER_WIDTH                  0x8D42
#define GL_RENDERBUFFER_HEIGHT                 0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT        0x8D44
#define GL_STENCIL_INDEX1                      0x8D46
#define GL_STENCIL_INDEX4                      0x8D47
#define GL_STENCIL_INDEX8                      0x8D48
#define GL_STENCIL_INDEX16                     0x8D49
#define GL_RENDERBUFFER_RED_SIZE               0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE             0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE              0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE             0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE             0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE           0x8D55
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE  0x8D56
#define GL_MAX_SAMPLES                         0x8D57
#define GL_INDEX                               0x8222
#define GL_TEXTURE_LUMINANCE_TYPE              0x8C14
#define GL_TEXTURE_INTENSITY_TYPE              0x8C15
#define GL_FRAMEBUFFER_SRGB                    0x8DB9
#define GL_HALF_FLOAT                          0x140B
#define GL_MAP_READ_BIT                        0x0001
#define GL_MAP_WRITE_BIT                       0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT            0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT           0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT              0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT              0x0020
#define GL_COMPRESSED_RED_RGTC1                0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1         0x8DBC
#define GL_COMPRESSED_RG_RGTC2                 0x8DBD
#define GL_COMPRESSED_SIGNED_RG_RGTC2          0x8DBE
#define GL_RG                                  0x8227
#define GL_RG_INTEGER                          0x8228
#define GL_R8                                  0x8229
#define GL_R16                                 0x822A
#define GL_RG8                                 0x822B
#define GL_RG16                                0x822C
#define GL_R16F                                0x822D
#define GL_R32F                                0x822E
#define GL_RG16F                               0x822F
#define GL_RG32F                               0x8230
#define GL_R8I                                 0x8231
#define GL_R8UI                                0x8232
#define GL_R16I                                0x8233
#define GL_R16UI                               0x8234
#define GL_R32I                                0x8235
#define GL_R32UI                               0x8236
#define GL_RG8I                                0x8237
#define GL_RG8UI                               0x8238
#define GL_RG16I                               0x8239
#define GL_RG16UI                              0x823A
#define GL_RG32I                               0x823B
#define GL_RG32UI                              0x823C
#define GL_VERTEX_ARRAY_BINDING                0x85B5
#define GL_CLAMP_VERTEX_COLOR                  0x891A
#define GL_CLAMP_FRAGMENT_COLOR                0x891B
#define GL_ALPHA_INTEGER                       0x8D97
typedef void (APIENTRYP pfn_glColorMaski) (GLuint, GLboolean, GLboolean, GLboolean, GLboolean);
extern pfn_glColorMaski fp_glColorMaski;
typedef void (APIENTRYP pfn_glGetBooleani_v) (GLenum, GLuint, GLboolean*);
extern pfn_glGetBooleani_v fp_glGetBooleani_v;
typedef void (APIENTRYP pfn_glGetIntegeri_v) (GLenum, GLuint, GLint*);
extern pfn_glGetIntegeri_v fp_glGetIntegeri_v;
typedef void (APIENTRYP pfn_glEnablei) (GLenum, GLuint);
extern pfn_glEnablei fp_glEnablei;
typedef void (APIENTRYP pfn_glDisablei) (GLenum, GLuint);
extern pfn_glDisablei fp_glDisablei;
typedef GLboolean (APIENTRYP pfn_glIsEnabledi) (GLenum, GLuint);
extern pfn_glIsEnabledi fp_glIsEnabledi;
typedef void (APIENTRYP pfn_glBeginTransformFeedback) (GLenum);
extern pfn_glBeginTransformFeedback fp_glBeginTransformFeedback;
typedef void (APIENTRYP pfn_glEndTransformFeedback) ();
extern pfn_glEndTransformFeedback fp_glEndTransformFeedback;
typedef void (APIENTRYP pfn_glBindBufferRange) (GLenum, GLuint, GLuint, GLintptr, GLsizeiptr);
extern pfn_glBindBufferRange fp_glBindBufferRange;
typedef void (APIENTRYP pfn_glBindBufferBase) (GLenum, GLuint, GLuint);
extern pfn_glBindBufferBase fp_glBindBufferBase;
typedef void (APIENTRYP pfn_glTransformFeedbackVaryings) (GLuint, GLsizei, const GLchar**, GLenum);
extern pfn_glTransformFeedbackVaryings fp_glTransformFeedbackVaryings;
typedef void (APIENTRYP pfn_glGetTransformFeedbackVarying) (GLuint, GLuint, GLsizei, GLsizei*, GLsizei*, GLenum*, GLchar*);
extern pfn_glGetTransformFeedbackVarying fp_glGetTransformFeedbackVarying;
typedef void (APIENTRYP pfn_glClampColor) (GLenum, GLenum);
extern pfn_glClampColor fp_glClampColor;
typedef void (APIENTRYP pfn_glBeginConditionalRender) (GLuint, GLenum);
extern pfn_glBeginConditionalRender fp_glBeginConditionalRender;
typedef void (APIENTRYP pfn_glEndConditionalRender) ();
extern pfn_glEndConditionalRender fp_glEndConditionalRender;
typedef void (APIENTRYP pfn_glVertexAttribIPointer) (GLuint, GLint, GLenum, GLsizei, const void*);
extern pfn_glVertexAttribIPointer fp_glVertexAttribIPointer;
typedef void (APIENTRYP pfn_glGetVertexAttribIiv) (GLuint, GLenum, GLint*);
extern pfn_glGetVertexAttribIiv fp_glGetVertexAttribIiv;
typedef void (APIENTRYP pfn_glGetVertexAttribIuiv) (GLuint, GLenum, GLuint*);
extern pfn_glGetVertexAttribIuiv fp_glGetVertexAttribIuiv;
typedef void (APIENTRYP pfn_glVertexAttribI1i) (GLuint, GLint);
extern pfn_glVertexAttribI1i fp_glVertexAttribI1i;
typedef void (APIENTRYP pfn_glVertexAttribI2i) (GLuint, GLint, GLint);
extern pfn_glVertexAttribI2i fp_glVertexAttribI2i;
typedef void (APIENTRYP pfn_glVertexAttribI3i) (GLuint, GLint, GLint, GLint);
extern pfn_glVertexAttribI3i fp_glVertexAttribI3i;
typedef void (APIENTRYP pfn_glVertexAttribI4i) (GLuint, GLint, GLint, GLint, GLint);
extern pfn_glVertexAttribI4i fp_glVertexAttribI4i;
typedef void (APIENTRYP pfn_glVertexAttribI1ui) (GLuint, GLuint);
extern pfn_glVertexAttribI1ui fp_glVertexAttribI1ui;
typedef void (APIENTRYP pfn_glVertexAttribI2ui) (GLuint, GLuint, GLuint);
extern pfn_glVertexAttribI2ui fp_glVertexAttribI2ui;
typedef void (APIENTRYP pfn_glVertexAttribI3ui) (GLuint, GLuint, GLuint, GLuint);
extern pfn_glVertexAttribI3ui fp_glVertexAttribI3ui;
typedef void (APIENTRYP pfn_glVertexAttribI4ui) (GLuint, GLuint, GLuint, GLuint, GLuint);
extern pfn_glVertexAttribI4ui fp_glVertexAttribI4ui;
typedef void (APIENTRYP pfn_glVertexAttribI1iv) (GLuint, const GLint*);
extern pfn_glVertexAttribI1iv fp_glVertexAttribI1iv;
typedef void (APIENTRYP pfn_glVertexAttribI2iv) (GLuint, const GLint*);
extern pfn_glVertexAttribI2iv fp_glVertexAttribI2iv;
typedef void (APIENTRYP pfn_glVertexAttribI3iv) (GLuint, const GLint*);
extern pfn_glVertexAttribI3iv fp_glVertexAttribI3iv;
typedef void (APIENTRYP pfn_glVertexAttribI4iv) (GLuint, const GLint*);
extern pfn_glVertexAttribI4iv fp_glVertexAttribI4iv;
typedef void (APIENTRYP pfn_glVertexAttribI1uiv) (GLuint, const GLuint*);
extern pfn_glVertexAttribI1uiv fp_glVertexAttribI1uiv;
typedef void (APIENTRYP pfn_glVertexAttribI2uiv) (GLuint, const GLuint*);
extern pfn_glVertexAttribI2uiv fp_glVertexAttribI2uiv;
typedef void (APIENTRYP pfn_glVertexAttribI3uiv) (GLuint, const GLuint*);
extern pfn_glVertexAttribI3uiv fp_glVertexAttribI3uiv;
typedef void (APIENTRYP pfn_glVertexAttribI4uiv) (GLuint, const GLuint*);
extern pfn_glVertexAttribI4uiv fp_glVertexAttribI4uiv;
typedef void (APIENTRYP pfn_glVertexAttribI4bv) (GLuint, const GLbyte*);
extern pfn_glVertexAttribI4bv fp_glVertexAttribI4bv;
typedef void (APIENTRYP pfn_glVertexAttribI4sv) (GLuint, const GLshort*);
extern pfn_glVertexAttribI4sv fp_glVertexAttribI4sv;
typedef void (APIENTRYP pfn_glVertexAttribI4ubv) (GLuint, const GLubyte*);
extern pfn_glVertexAttribI4ubv fp_glVertexAttribI4ubv;
typedef void (APIENTRYP pfn_glVertexAttribI4usv) (GLuint, const GLushort*);
extern pfn_glVertexAttribI4usv fp_glVertexAttribI4usv;
typedef void (APIENTRYP pfn_glGetUniformuiv) (GLuint, GLint, GLuint*);
extern pfn_glGetUniformuiv fp_glGetUniformuiv;
typedef void (APIENTRYP pfn_glBindFragDataLocation) (GLuint, GLuint, const GLchar*);
extern pfn_glBindFragDataLocation fp_glBindFragDataLocation;
typedef GLint (APIENTRYP pfn_glGetFragDataLocation) (GLuint, const GLchar*);
extern pfn_glGetFragDataLocation fp_glGetFragDataLocation;
typedef void (APIENTRYP pfn_glUniform1ui) (GLint, GLuint);
extern pfn_glUniform1ui fp_glUniform1ui;
typedef void (APIENTRYP pfn_glUniform2ui) (GLint, GLuint, GLuint);
extern pfn_glUniform2ui fp_glUniform2ui;
typedef void (APIENTRYP pfn_glUniform3ui) (GLint, GLuint, GLuint, GLuint);
extern pfn_glUniform3ui fp_glUniform3ui;
typedef void (APIENTRYP pfn_glUniform4ui) (GLint, GLuint, GLuint, GLuint, GLuint);
extern pfn_glUniform4ui fp_glUniform4ui;
typedef void (APIENTRYP pfn_glUniform1uiv) (GLint, GLsizei, const GLuint*);
extern pfn_glUniform1uiv fp_glUniform1uiv;
typedef void (APIENTRYP pfn_glUniform2uiv) (GLint, GLsizei, const GLuint*);
extern pfn_glUniform2uiv fp_glUniform2uiv;
typedef void (APIENTRYP pfn_glUniform3uiv) (GLint, GLsizei, const GLuint*);
extern pfn_glUniform3uiv fp_glUniform3uiv;
typedef void (APIENTRYP pfn_glUniform4uiv) (GLint, GLsizei, const GLuint*);
extern pfn_glUniform4uiv fp_glUniform4uiv;
typedef void (APIENTRYP pfn_glTexParameterIiv) (GLenum, GLenum, const GLint*);
extern pfn_glTexParameterIiv fp_glTexParameterIiv;
typedef void (APIENTRYP pfn_glTexParameterIuiv) (GLenum, GLenum, const GLuint*);
extern pfn_glTexParameterIuiv fp_glTexParameterIuiv;
typedef void (APIENTRYP pfn_glGetTexParameterIiv) (GLenum, GLenum, GLint*);
extern pfn_glGetTexParameterIiv fp_glGetTexParameterIiv;
typedef void (APIENTRYP pfn_glGetTexParameterIuiv) (GLenum, GLenum, GLuint*);
extern pfn_glGetTexParameterIuiv fp_glGetTexParameterIuiv;
typedef void (APIENTRYP pfn_glClearBufferiv) (GLenum, GLint, const GLint*);
extern pfn_glClearBufferiv fp_glClearBufferiv;
typedef void (APIENTRYP pfn_glClearBufferuiv) (GLenum, GLint, const GLuint*);
extern pfn_glClearBufferuiv fp_glClearBufferuiv;
typedef void (APIENTRYP pfn_glClearBufferfv) (GLenum, GLint, const GLfloat*);
extern pfn_glClearBufferfv fp_glClearBufferfv;
typedef void (APIENTRYP pfn_glClearBufferfi) (GLenum, GLint, GLfloat, GLint);
extern pfn_glClearBufferfi fp_glClearBufferfi;
typedef const GLubyte* (APIENTRYP pfn_glGetStringi) (GLenum, GLuint);
extern pfn_glGetStringi fp_glGetStringi;
typedef GLboolean (APIENTRYP pfn_glIsRenderbuffer) (GLuint);
extern pfn_glIsRenderbuffer fp_glIsRenderbuffer;
typedef void (APIENTRYP pfn_glBindRenderbuffer) (GLenum, GLuint);
extern pfn_glBindRenderbuffer fp_glBindRenderbuffer;
typedef void (APIENTRYP pfn_glDeleteRenderbuffers) (GLsizei, const GLuint*);
extern pfn_glDeleteRenderbuffers fp_glDeleteRenderbuffers;
typedef void (APIENTRYP pfn_glGenRenderbuffers) (GLsizei, GLuint*);
extern pfn_glGenRenderbuffers fp_glGenRenderbuffers;
typedef void (APIENTRYP pfn_glRenderbufferStorage) (GLenum, GLenum, GLsizei, GLsizei);
extern pfn_glRenderbufferStorage fp_glRenderbufferStorage;
typedef void (APIENTRYP pfn_glGetRenderbufferParameteriv) (GLenum, GLenum, GLint*);
extern pfn_glGetRenderbufferParameteriv fp_glGetRenderbufferParameteriv;
typedef GLboolean (APIENTRYP pfn_glIsFramebuffer) (GLuint);
extern pfn_glIsFramebuffer fp_glIsFramebuffer;
typedef void (APIENTRYP pfn_glBindFramebuffer) (GLenum, GLuint);
extern pfn_glBindFramebuffer fp_glBindFramebuffer;
typedef void (APIENTRYP pfn_glDeleteFramebuffers) (GLsizei, const GLuint*);
extern pfn_glDeleteFramebuffers fp_glDeleteFramebuffers;
typedef void (APIENTRYP pfn_glGenFramebuffers) (GLsizei, GLuint*);
extern pfn_glGenFramebuffers fp_glGenFramebuffers;
typedef GLenum (APIENTRYP pfn_glCheckFramebufferStatus) (GLenum);
extern pfn_glCheckFramebufferStatus fp_glCheckFramebufferStatus;
typedef void (APIENTRYP pfn_glFramebufferTexture1D) (GLenum, GLenum, GLenum, GLuint, GLint);
extern pfn_glFramebufferTexture1D fp_glFramebufferTexture1D;
typedef void (APIENTRYP pfn_glFramebufferTexture2D) (GLenum, GLenum, GLenum, GLuint, GLint);
extern pfn_glFramebufferTexture2D fp_glFramebufferTexture2D;
typedef void (APIENTRYP pfn_glFramebufferTexture3D) (GLenum, GLenum, GLenum, GLuint, GLint, GLint);
extern pfn_glFramebufferTexture3D fp_glFramebufferTexture3D;
typedef void (APIENTRYP pfn_glFramebufferRenderbuffer) (GLenum, GLenum, GLenum, GLuint);
extern pfn_glFramebufferRenderbuffer fp_glFramebufferRenderbuffer;
typedef void (APIENTRYP pfn_glGetFramebufferAttachmentParameteriv) (GLenum, GLenum, GLenum, GLint*);
extern pfn_glGetFramebufferAttachmentParameteriv fp_glGetFramebufferAttachmentParameteriv;
typedef void (APIENTRYP pfn_glGenerateMipmap) (GLenum);
extern pfn_glGenerateMipmap fp_glGenerateMipmap;
typedef void (APIENTRYP pfn_glBlitFramebuffer) (GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
extern pfn_glBlitFramebuffer fp_glBlitFramebuffer;
typedef void (APIENTRYP pfn_glRenderbufferStorageMultisample) (GLenum, GLsizei, GLenum, GLsizei, GLsizei);
extern pfn_glRenderbufferStorageMultisample fp_glRenderbufferStorageMultisample;
typedef void (APIENTRYP pfn_glFramebufferTextureLayer) (GLenum, GLenum, GLuint, GLint, GLint);
extern pfn_glFramebufferTextureLayer fp_glFramebufferTextureLayer;
typedef void* (APIENTRYP pfn_glMapBufferRange) (GLenum, GLintptr, GLsizeiptr, GLbitfield);
extern pfn_glMapBufferRange fp_glMapBufferRange;
typedef void (APIENTRYP pfn_glFlushMappedBufferRange) (GLenum, GLintptr, GLsizeiptr);
extern pfn_glFlushMappedBufferRange fp_glFlushMappedBufferRange;
typedef void (APIENTRYP pfn_glBindVertexArray) (GLuint);
extern pfn_glBindVertexArray fp_glBindVertexArray;
typedef void (APIENTRYP pfn_glDeleteVertexArrays) (GLsizei, const GLuint*);
extern pfn_glDeleteVertexArrays fp_glDeleteVertexArrays;
typedef void (APIENTRYP pfn_glGenVertexArrays) (GLsizei, GLuint*);
extern pfn_glGenVertexArrays fp_glGenVertexArrays;
typedef GLboolean (APIENTRYP pfn_glIsVertexArray) (GLuint);
extern pfn_glIsVertexArray fp_glIsVertexArray;

 /* GL_VERSION_3_1 */
extern GLboolean GLAD_VERSION_3_1;
#define GL_SAMPLER_2D_RECT                     0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW              0x8B64
#define GL_SAMPLER_BUFFER                      0x8DC2
#define GL_INT_SAMPLER_2D_RECT                 0x8DCD
#define GL_INT_SAMPLER_BUFFER                  0x8DD0
#define GL_UNSIGNED_INT_SAMPLER_2D_RECT        0x8DD5
#define GL_UNSIGNED_INT_SAMPLER_BUFFER         0x8DD8
#define GL_TEXTURE_BUFFER                      0x8C2A
#define GL_MAX_TEXTURE_BUFFER_SIZE             0x8C2B
#define GL_TEXTURE_BINDING_BUFFER              0x8C2C
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING   0x8C2D
#define GL_TEXTURE_RECTANGLE                   0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE           0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE             0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE          0x84F8
#define GL_R8_SNORM                            0x8F94
#define GL_RG8_SNORM                           0x8F95
#define GL_RGB8_SNORM                          0x8F96
#define GL_RGBA8_SNORM                         0x8F97
#define GL_R16_SNORM                           0x8F98
#define GL_RG16_SNORM                          0x8F99
#define GL_RGB16_SNORM                         0x8F9A
#define GL_RGBA16_SNORM                        0x8F9B
#define GL_SIGNED_NORMALIZED                   0x8F9C
#define GL_PRIMITIVE_RESTART                   0x8F9D
#define GL_PRIMITIVE_RESTART_INDEX             0x8F9E
#define GL_COPY_READ_BUFFER                    0x8F36
#define GL_COPY_WRITE_BUFFER                   0x8F37
#define GL_UNIFORM_BUFFER                      0x8A11
#define GL_UNIFORM_BUFFER_BINDING              0x8A28
#define GL_UNIFORM_BUFFER_START                0x8A29
#define GL_UNIFORM_BUFFER_SIZE                 0x8A2A
#define GL_MAX_VERTEX_UNIFORM_BLOCKS           0x8A2B
#define GL_MAX_GEOMETRY_UNIFORM_BLOCKS         0x8A2C
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS         0x8A2D
#define GL_MAX_COMBINED_UNIFORM_BLOCKS         0x8A2E
#define GL_MAX_UNIFORM_BUFFER_BINDINGS         0x8A2F
#define GL_MAX_UNIFORM_BLOCK_SIZE              0x8A30
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS  0x8A31
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS  0x8A32
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS  0x8A33
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT     0x8A34
#define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH  0x8A35
#define GL_ACTIVE_UNIFORM_BLOCKS               0x8A36
#define GL_UNIFORM_TYPE                        0x8A37
#define GL_UNIFORM_SIZE                        0x8A38
#define GL_UNIFORM_NAME_LENGTH                 0x8A39
#define GL_UNIFORM_BLOCK_INDEX                 0x8A3A
#define GL_UNIFORM_OFFSET                      0x8A3B
#define GL_UNIFORM_ARRAY_STRIDE                0x8A3C
#define GL_UNIFORM_MATRIX_STRIDE               0x8A3D
#define GL_UNIFORM_IS_ROW_MAJOR                0x8A3E
#define GL_UNIFORM_BLOCK_BINDING               0x8A3F
#define GL_UNIFORM_BLOCK_DATA_SIZE             0x8A40
#define GL_UNIFORM_BLOCK_NAME_LENGTH           0x8A41
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS       0x8A42
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES  0x8A43
#define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER  0x8A44
#define GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER  0x8A45
#define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER  0x8A46
#define GL_INVALID_INDEX                       0xFFFFFFFF
typedef void (APIENTRYP pfn_glDrawArraysInstanced) (GLenum, GLint, GLsizei, GLsizei);
extern pfn_glDrawArraysInstanced fp_glDrawArraysInstanced;
typedef void (APIENTRYP pfn_glDrawElementsInstanced) (GLenum, GLsizei, GLenum, const void*, GLsizei);
extern pfn_glDrawElementsInstanced fp_glDrawElementsInstanced;
typedef void (APIENTRYP pfn_glTexBuffer) (GLenum, GLenum, GLuint);
extern pfn_glTexBuffer fp_glTexBuffer;
typedef void (APIENTRYP pfn_glPrimitiveRestartIndex) (GLuint);
extern pfn_glPrimitiveRestartIndex fp_glPrimitiveRestartIndex;
typedef void (APIENTRYP pfn_glCopyBufferSubData) (GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr);
extern pfn_glCopyBufferSubData fp_glCopyBufferSubData;
typedef void (APIENTRYP pfn_glGetUniformIndices) (GLuint, GLsizei, const GLchar**, GLuint*);
extern pfn_glGetUniformIndices fp_glGetUniformIndices;
typedef void (APIENTRYP pfn_glGetActiveUniformsiv) (GLuint, GLsizei, const GLuint*, GLenum, GLint*);
extern pfn_glGetActiveUniformsiv fp_glGetActiveUniformsiv;
typedef void (APIENTRYP pfn_glGetActiveUniformName) (GLuint, GLuint, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetActiveUniformName fp_glGetActiveUniformName;
typedef GLuint (APIENTRYP pfn_glGetUniformBlockIndex) (GLuint, const GLchar*);
extern pfn_glGetUniformBlockIndex fp_glGetUniformBlockIndex;
typedef void (APIENTRYP pfn_glGetActiveUniformBlockiv) (GLuint, GLuint, GLenum, GLint*);
extern pfn_glGetActiveUniformBlockiv fp_glGetActiveUniformBlockiv;
typedef void (APIENTRYP pfn_glGetActiveUniformBlockName) (GLuint, GLuint, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetActiveUniformBlockName fp_glGetActiveUniformBlockName;
typedef void (APIENTRYP pfn_glUniformBlockBinding) (GLuint, GLuint, GLuint);
extern pfn_glUniformBlockBinding fp_glUniformBlockBinding;

 /* GL_VERSION_3_2 */
extern GLboolean GLAD_VERSION_3_2;
#define GL_CONTEXT_CORE_PROFILE_BIT            0x00000001
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT   0x00000002
#define GL_LINES_ADJACENCY                     0x000A
#define GL_LINE_STRIP_ADJACENCY                0x000B
#define GL_TRIANGLES_ADJACENCY                 0x000C
#define GL_TRIANGLE_STRIP_ADJACENCY            0x000D
#define GL_PROGRAM_POINT_SIZE                  0x8642
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS    0x8C29
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED      0x8DA7
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS  0x8DA8
#define GL_GEOMETRY_SHADER                     0x8DD9
#define GL_GEOMETRY_VERTICES_OUT               0x8916
#define GL_GEOMETRY_INPUT_TYPE                 0x8917
#define GL_GEOMETRY_OUTPUT_TYPE                0x8918
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS     0x8DDF
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES        0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS  0x8DE1
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS        0x9122
#define GL_MAX_GEOMETRY_INPUT_COMPONENTS       0x9123
#define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS      0x9124
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS       0x9125
#define GL_CONTEXT_PROFILE_MASK                0x9126
#define GL_DEPTH_CLAMP                         0x864F
#define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION  0x8E4C
#define GL_FIRST_VERTEX_CONVENTION             0x8E4D
#define GL_LAST_VERTEX_CONVENTION              0x8E4E
#define GL_PROVOKING_VERTEX                    0x8E4F
#define GL_TEXTURE_CUBE_MAP_SEAMLESS           0x884F
#define GL_MAX_SERVER_WAIT_TIMEOUT             0x9111
#define GL_OBJECT_TYPE                         0x9112
#define GL_SYNC_CONDITION                      0x9113
#define GL_SYNC_STATUS                         0x9114
#define GL_SYNC_FLAGS                          0x9115
#define GL_SYNC_FENCE                          0x9116
#define GL_SYNC_GPU_COMMANDS_COMPLETE          0x9117
#define GL_UNSIGNALED                          0x9118
#define GL_SIGNALED                            0x9119
#define GL_ALREADY_SIGNALED                    0x911A
#define GL_TIMEOUT_EXPIRED                     0x911B
#define GL_CONDITION_SATISFIED                 0x911C
#define GL_WAIT_FAILED                         0x911D
#define GL_TIMEOUT_IGNORED                     0xFFFFFFFFFFFFFFFF
#define GL_SYNC_FLUSH_COMMANDS_BIT             0x00000001
#define GL_SAMPLE_POSITION                     0x8E50
#define GL_SAMPLE_MASK                         0x8E51
#define GL_SAMPLE_MASK_VALUE                   0x8E52
#define GL_MAX_SAMPLE_MASK_WORDS               0x8E59
#define GL_TEXTURE_2D_MULTISAMPLE              0x9100
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE        0x9101
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY        0x9102
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY  0x9103
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE      0x9104
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY  0x9105
#define GL_TEXTURE_SAMPLES                     0x9106
#define GL_TEXTURE_FIXED_SAMPLE_LOCATIONS      0x9107
#define GL_SAMPLER_2D_MULTISAMPLE              0x9108
#define GL_INT_SAMPLER_2D_MULTISAMPLE          0x9109
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE  0x910A
#define GL_SAMPLER_2D_MULTISAMPLE_ARRAY        0x910B
#define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY    0x910C
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY  0x910D
#define GL_MAX_COLOR_TEXTURE_SAMPLES           0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES           0x910F
#define GL_MAX_INTEGER_SAMPLES                 0x9110
typedef void (APIENTRYP pfn_glDrawElementsBaseVertex) (GLenum, GLsizei, GLenum, const void*, GLint);
extern pfn_glDrawElementsBaseVertex fp_glDrawElementsBaseVertex;
typedef void (APIENTRYP pfn_glDrawRangeElementsBaseVertex) (GLenum, GLuint, GLuint, GLsizei, GLenum, const void*, GLint);
extern pfn_glDrawRangeElementsBaseVertex fp_glDrawRangeElementsBaseVertex;
typedef void (APIENTRYP pfn_glDrawElementsInstancedBaseVertex) (GLenum, GLsizei, GLenum, const void*, GLsizei, GLint);
extern pfn_glDrawElementsInstancedBaseVertex fp_glDrawElementsInstancedBaseVertex;
typedef void (APIENTRYP pfn_glMultiDrawElementsBaseVertex) (GLenum, const GLsizei*, GLenum, const void**, GLsizei, const GLint*);
extern pfn_glMultiDrawElementsBaseVertex fp_glMultiDrawElementsBaseVertex;
typedef void (APIENTRYP pfn_glProvokingVertex) (GLenum);
extern pfn_glProvokingVertex fp_glProvokingVertex;
typedef GLsync (APIENTRYP pfn_glFenceSync) (GLenum, GLbitfield);
extern pfn_glFenceSync fp_glFenceSync;
typedef GLboolean (APIENTRYP pfn_glIsSync) (GLsync);
extern pfn_glIsSync fp_glIsSync;
typedef void (APIENTRYP pfn_glDeleteSync) (GLsync);
extern pfn_glDeleteSync fp_glDeleteSync;
typedef GLenum (APIENTRYP pfn_glClientWaitSync) (GLsync, GLbitfield, GLuint64);
extern pfn_glClientWaitSync fp_glClientWaitSync;
typedef void (APIENTRYP pfn_glWaitSync) (GLsync, GLbitfield, GLuint64);
extern pfn_glWaitSync fp_glWaitSync;
typedef void (APIENTRYP pfn_glGetInteger64v) (GLenum, GLint64*);
extern pfn_glGetInteger64v fp_glGetInteger64v;
typedef void (APIENTRYP pfn_glGetSynciv) (GLsync, GLenum, GLsizei, GLsizei*, GLint*);
extern pfn_glGetSynciv fp_glGetSynciv;
typedef void (APIENTRYP pfn_glGetInteger64i_v) (GLenum, GLuint, GLint64*);
extern pfn_glGetInteger64i_v fp_glGetInteger64i_v;
typedef void (APIENTRYP pfn_glGetBufferParameteri64v) (GLenum, GLenum, GLint64*);
extern pfn_glGetBufferParameteri64v fp_glGetBufferParameteri64v;
typedef void (APIENTRYP pfn_glFramebufferTexture) (GLenum, GLenum, GLuint, GLint);
extern pfn_glFramebufferTexture fp_glFramebufferTexture;
typedef void (APIENTRYP pfn_glTexImage2DMultisample) (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean);
extern pfn_glTexImage2DMultisample fp_glTexImage2DMultisample;
typedef void (APIENTRYP pfn_glTexImage3DMultisample) (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean);
extern pfn_glTexImage3DMultisample fp_glTexImage3DMultisample;
typedef void (APIENTRYP pfn_glGetMultisamplefv) (GLenum, GLuint, GLfloat*);
extern pfn_glGetMultisamplefv fp_glGetMultisamplefv;
typedef void (APIENTRYP pfn_glSampleMaski) (GLuint, GLbitfield);
extern pfn_glSampleMaski fp_glSampleMaski;

 /* GL_VERSION_3_3 */
extern GLboolean GLAD_VERSION_3_3;
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR         0x88FE
#define GL_SRC1_COLOR                          0x88F9
#define GL_ONE_MINUS_SRC1_COLOR                0x88FA
#define GL_ONE_MINUS_SRC1_ALPHA                0x88FB
#define GL_MAX_DUAL_SOURCE_DRAW_BUFFERS        0x88FC
#define GL_ANY_SAMPLES_PASSED                  0x8C2F
#define GL_SAMPLER_BINDING                     0x8919
#define GL_RGB10_A2UI                          0x906F
#define GL_TEXTURE_SWIZZLE_R                   0x8E42
#define GL_TEXTURE_SWIZZLE_G                   0x8E43
#define GL_TEXTURE_SWIZZLE_B                   0x8E44
#define GL_TEXTURE_SWIZZLE_A                   0x8E45
#define GL_TEXTURE_SWIZZLE_RGBA                0x8E46
#define GL_TIME_ELAPSED                        0x88BF
#define GL_TIMESTAMP                           0x8E28
#define GL_INT_2_10_10_10_REV                  0x8D9F
typedef void (APIENTRYP pfn_glBindFragDataLocationIndexed) (GLuint, GLuint, GLuint, const GLchar*);
extern pfn_glBindFragDataLocationIndexed fp_glBindFragDataLocationIndexed;
typedef GLint (APIENTRYP pfn_glGetFragDataIndex) (GLuint, const GLchar*);
extern pfn_glGetFragDataIndex fp_glGetFragDataIndex;
typedef void (APIENTRYP pfn_glGenSamplers) (GLsizei, GLuint*);
extern pfn_glGenSamplers fp_glGenSamplers;
typedef void (APIENTRYP pfn_glDeleteSamplers) (GLsizei, const GLuint*);
extern pfn_glDeleteSamplers fp_glDeleteSamplers;
typedef GLboolean (APIENTRYP pfn_glIsSampler) (GLuint);
extern pfn_glIsSampler fp_glIsSampler;
typedef void (APIENTRYP pfn_glBindSampler) (GLuint, GLuint);
extern pfn_glBindSampler fp_glBindSampler;
typedef void (APIENTRYP pfn_glSamplerParameteri) (GLuint, GLenum, GLint);
extern pfn_glSamplerParameteri fp_glSamplerParameteri;
typedef void (APIENTRYP pfn_glSamplerParameteriv) (GLuint, GLenum, const GLint*);
extern pfn_glSamplerParameteriv fp_glSamplerParameteriv;
typedef void (APIENTRYP pfn_glSamplerParameterf) (GLuint, GLenum, GLfloat);
extern pfn_glSamplerParameterf fp_glSamplerParameterf;
typedef void (APIENTRYP pfn_glSamplerParameterfv) (GLuint, GLenum, const GLfloat*);
extern pfn_glSamplerParameterfv fp_glSamplerParameterfv;
typedef void (APIENTRYP pfn_glSamplerParameterIiv) (GLuint, GLenum, const GLint*);
extern pfn_glSamplerParameterIiv fp_glSamplerParameterIiv;
typedef void (APIENTRYP pfn_glSamplerParameterIuiv) (GLuint, GLenum, const GLuint*);
extern pfn_glSamplerParameterIuiv fp_glSamplerParameterIuiv;
typedef void (APIENTRYP pfn_glGetSamplerParameteriv) (GLuint, GLenum, GLint*);
extern pfn_glGetSamplerParameteriv fp_glGetSamplerParameteriv;
typedef void (APIENTRYP pfn_glGetSamplerParameterIiv) (GLuint, GLenum, GLint*);
extern pfn_glGetSamplerParameterIiv fp_glGetSamplerParameterIiv;
typedef void (APIENTRYP pfn_glGetSamplerParameterfv) (GLuint, GLenum, GLfloat*);
extern pfn_glGetSamplerParameterfv fp_glGetSamplerParameterfv;
typedef void (APIENTRYP pfn_glGetSamplerParameterIuiv) (GLuint, GLenum, GLuint*);
extern pfn_glGetSamplerParameterIuiv fp_glGetSamplerParameterIuiv;
typedef void (APIENTRYP pfn_glQueryCounter) (GLuint, GLenum);
extern pfn_glQueryCounter fp_glQueryCounter;
typedef void (APIENTRYP pfn_glGetQueryObjecti64v) (GLuint, GLenum, GLint64*);
extern pfn_glGetQueryObjecti64v fp_glGetQueryObjecti64v;
typedef void (APIENTRYP pfn_glGetQueryObjectui64v) (GLuint, GLenum, GLuint64*);
extern pfn_glGetQueryObjectui64v fp_glGetQueryObjectui64v;
typedef void (APIENTRYP pfn_glVertexAttribDivisor) (GLuint, GLuint);
extern pfn_glVertexAttribDivisor fp_glVertexAttribDivisor;
typedef void (APIENTRYP pfn_glVertexAttribP1ui) (GLuint, GLenum, GLboolean, GLuint);
extern pfn_glVertexAttribP1ui fp_glVertexAttribP1ui;
typedef void (APIENTRYP pfn_glVertexAttribP1uiv) (GLuint, GLenum, GLboolean, const GLuint*);
extern pfn_glVertexAttribP1uiv fp_glVertexAttribP1uiv;
typedef void (APIENTRYP pfn_glVertexAttribP2ui) (GLuint, GLenum, GLboolean, GLuint);
extern pfn_glVertexAttribP2ui fp_glVertexAttribP2ui;
typedef void (APIENTRYP pfn_glVertexAttribP2uiv) (GLuint, GLenum, GLboolean, const GLuint*);
extern pfn_glVertexAttribP2uiv fp_glVertexAttribP2uiv;
typedef void (APIENTRYP pfn_glVertexAttribP3ui) (GLuint, GLenum, GLboolean, GLuint);
extern pfn_glVertexAttribP3ui fp_glVertexAttribP3ui;
typedef void (APIENTRYP pfn_glVertexAttribP3uiv) (GLuint, GLenum, GLboolean, const GLuint*);
extern pfn_glVertexAttribP3uiv fp_glVertexAttribP3uiv;
typedef void (APIENTRYP pfn_glVertexAttribP4ui) (GLuint, GLenum, GLboolean, GLuint);
extern pfn_glVertexAttribP4ui fp_glVertexAttribP4ui;
typedef void (APIENTRYP pfn_glVertexAttribP4uiv) (GLuint, GLenum, GLboolean, const GLuint*);
extern pfn_glVertexAttribP4uiv fp_glVertexAttribP4uiv;
typedef void (APIENTRYP pfn_glVertexP2ui) (GLenum, GLuint);
extern pfn_glVertexP2ui fp_glVertexP2ui;
typedef void (APIENTRYP pfn_glVertexP2uiv) (GLenum, const GLuint*);
extern pfn_glVertexP2uiv fp_glVertexP2uiv;
typedef void (APIENTRYP pfn_glVertexP3ui) (GLenum, GLuint);
extern pfn_glVertexP3ui fp_glVertexP3ui;
typedef void (APIENTRYP pfn_glVertexP3uiv) (GLenum, const GLuint*);
extern pfn_glVertexP3uiv fp_glVertexP3uiv;
typedef void (APIENTRYP pfn_glVertexP4ui) (GLenum, GLuint);
extern pfn_glVertexP4ui fp_glVertexP4ui;
typedef void (APIENTRYP pfn_glVertexP4uiv) (GLenum, const GLuint*);
extern pfn_glVertexP4uiv fp_glVertexP4uiv;
typedef void (APIENTRYP pfn_glTexCoordP1ui) (GLenum, GLuint);
extern pfn_glTexCoordP1ui fp_glTexCoordP1ui;
typedef void (APIENTRYP pfn_glTexCoordP1uiv) (GLenum, const GLuint*);
extern pfn_glTexCoordP1uiv fp_glTexCoordP1uiv;
typedef void (APIENTRYP pfn_glTexCoordP2ui) (GLenum, GLuint);
extern pfn_glTexCoordP2ui fp_glTexCoordP2ui;
typedef void (APIENTRYP pfn_glTexCoordP2uiv) (GLenum, const GLuint*);
extern pfn_glTexCoordP2uiv fp_glTexCoordP2uiv;
typedef void (APIENTRYP pfn_glTexCoordP3ui) (GLenum, GLuint);
extern pfn_glTexCoordP3ui fp_glTexCoordP3ui;
typedef void (APIENTRYP pfn_glTexCoordP3uiv) (GLenum, const GLuint*);
extern pfn_glTexCoordP3uiv fp_glTexCoordP3uiv;
typedef void (APIENTRYP pfn_glTexCoordP4ui) (GLenum, GLuint);
extern pfn_glTexCoordP4ui fp_glTexCoordP4ui;
typedef void (APIENTRYP pfn_glTexCoordP4uiv) (GLenum, const GLuint*);
extern pfn_glTexCoordP4uiv fp_glTexCoordP4uiv;
typedef void (APIENTRYP pfn_glMultiTexCoordP1ui) (GLenum, GLenum, GLuint);
extern pfn_glMultiTexCoordP1ui fp_glMultiTexCoordP1ui;
typedef void (APIENTRYP pfn_glMultiTexCoordP1uiv) (GLenum, GLenum, const GLuint*);
extern pfn_glMultiTexCoordP1uiv fp_glMultiTexCoordP1uiv;
typedef void (APIENTRYP pfn_glMultiTexCoordP2ui) (GLenum, GLenum, GLuint);
extern pfn_glMultiTexCoordP2ui fp_glMultiTexCoordP2ui;
typedef void (APIENTRYP pfn_glMultiTexCoordP2uiv) (GLenum, GLenum, const GLuint*);
extern pfn_glMultiTexCoordP2uiv fp_glMultiTexCoordP2uiv;
typedef void (APIENTRYP pfn_glMultiTexCoordP3ui) (GLenum, GLenum, GLuint);
extern pfn_glMultiTexCoordP3ui fp_glMultiTexCoordP3ui;
typedef void (APIENTRYP pfn_glMultiTexCoordP3uiv) (GLenum, GLenum, const GLuint*);
extern pfn_glMultiTexCoordP3uiv fp_glMultiTexCoordP3uiv;
typedef void (APIENTRYP pfn_glMultiTexCoordP4ui) (GLenum, GLenum, GLuint);
extern pfn_glMultiTexCoordP4ui fp_glMultiTexCoordP4ui;
typedef void (APIENTRYP pfn_glMultiTexCoordP4uiv) (GLenum, GLenum, const GLuint*);
extern pfn_glMultiTexCoordP4uiv fp_glMultiTexCoordP4uiv;
typedef void (APIENTRYP pfn_glNormalP3ui) (GLenum, GLuint);
extern pfn_glNormalP3ui fp_glNormalP3ui;
typedef void (APIENTRYP pfn_glNormalP3uiv) (GLenum, const GLuint*);
extern pfn_glNormalP3uiv fp_glNormalP3uiv;
typedef void (APIENTRYP pfn_glColorP3ui) (GLenum, GLuint);
extern pfn_glColorP3ui fp_glColorP3ui;
typedef void (APIENTRYP pfn_glColorP3uiv) (GLenum, const GLuint*);
extern pfn_glColorP3uiv fp_glColorP3uiv;
typedef void (APIENTRYP pfn_glColorP4ui) (GLenum, GLuint);
extern pfn_glColorP4ui fp_glColorP4ui;
typedef void (APIENTRYP pfn_glColorP4uiv) (GLenum, const GLuint*);
extern pfn_glColorP4uiv fp_glColorP4uiv;
typedef void (APIENTRYP pfn_glSecondaryColorP3ui) (GLenum, GLuint);
extern pfn_glSecondaryColorP3ui fp_glSecondaryColorP3ui;
typedef void (APIENTRYP pfn_glSecondaryColorP3uiv) (GLenum, const GLuint*);
extern pfn_glSecondaryColorP3uiv fp_glSecondaryColorP3uiv;

 /* GL_VERSION_4_0 */
extern GLboolean GLAD_VERSION_4_0;
#define GL_SAMPLE_SHADING                      0x8C36
#define GL_MIN_SAMPLE_SHADING_VALUE            0x8C37
#define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET   0x8E5E
#define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET   0x8E5F
#define GL_TEXTURE_CUBE_MAP_ARRAY              0x9009
#define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY      0x900A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARRAY        0x900B
#define GL_SAMPLER_CUBE_MAP_ARRAY              0x900C
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW       0x900D
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY          0x900E
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY  0x900F
#define GL_DRAW_INDIRECT_BUFFER                0x8F3F
#define GL_DRAW_INDIRECT_BUFFER_BINDING        0x8F43
#define GL_GEOMETRY_SHADER_INVOCATIONS         0x887F
#define GL_MAX_GEOMETRY_SHADER_INVOCATIONS     0x8E5A
#define GL_MIN_FRAGMENT_INTERPOLATION_OFFSET   0x8E5B
#define GL_MAX_FRAGMENT_INTERPOLATION_OFFSET   0x8E5C
#define GL_FRAGMENT_INTERPOLATION_OFFSET_BITS  0x8E5D
#define GL_MAX_VERTEX_STREAMS                  0x8E71
#define GL_DOUBLE_VEC2                         0x8FFC
#define GL_DOUBLE_VEC3                         0x8FFD
#define GL_DOUBLE_VEC4                         0x8FFE
#define GL_DOUBLE_MAT2                         0x8F46
#define GL_DOUBLE_MAT3                         0x8F47
#define GL_DOUBLE_MAT4                         0x8F48
#define GL_DOUBLE_MAT2x3                       0x8F49
#define GL_DOUBLE_MAT2x4                       0x8F4A
#define GL_DOUBLE_MAT3x2                       0x8F4B
#define GL_DOUBLE_MAT3x4                       0x8F4C
#define GL_DOUBLE_MAT4x2                       0x8F4D
#define GL_DOUBLE_MAT4x3                       0x8F4E
#define GL_ACTIVE_SUBROUTINES                  0x8DE5
#define GL_ACTIVE_SUBROUTINE_UNIFORMS          0x8DE6
#define GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS  0x8E47
#define GL_ACTIVE_SUBROUTINE_MAX_LENGTH        0x8E48
#define GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH  0x8E49
#define GL_MAX_SUBROUTINES                     0x8DE7
#define GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS    0x8DE8
#define GL_NUM_COMPATIBLE_SUBROUTINES          0x8E4A
#define GL_COMPATIBLE_SUBROUTINES              0x8E4B
#define GL_PATCHES                             0x000E
#define GL_PATCH_VERTICES                      0x8E72
#define GL_PATCH_DEFAULT_INNER_LEVEL           0x8E73
#define GL_PATCH_DEFAULT_OUTER_LEVEL           0x8E74
#define GL_TESS_CONTROL_OUTPUT_VERTICES        0x8E75
#define GL_TESS_GEN_MODE                       0x8E76
#define GL_TESS_GEN_SPACING                    0x8E77
#define GL_TESS_GEN_VERTEX_ORDER               0x8E78
#define GL_TESS_GEN_POINT_MODE                 0x8E79
#define GL_ISOLINES                            0x8E7A
#define GL_FRACTIONAL_ODD                      0x8E7B
#define GL_FRACTIONAL_EVEN                     0x8E7C
#define GL_MAX_PATCH_VERTICES                  0x8E7D
#define GL_MAX_TESS_GEN_LEVEL                  0x8E7E
#define GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS  0x8E7F
#define GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS  0x8E80
#define GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS  0x8E81
#define GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS  0x8E82
#define GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS  0x8E83
#define GL_MAX_TESS_PATCH_COMPONENTS           0x8E84
#define GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS  0x8E85
#define GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS  0x8E86
#define GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS     0x8E89
#define GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS  0x8E8A
#define GL_MAX_TESS_CONTROL_INPUT_COMPONENTS   0x886C
#define GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS  0x886D
#define GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS  0x8E1E
#define GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS  0x8E1F
#define GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER  0x84F0
#define GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER  0x84F1
#define GL_TESS_EVALUATION_SHADER              0x8E87
#define GL_TESS_CONTROL_SHADER                 0x8E88
#define GL_TRANSFORM_FEEDBACK                  0x8E22
#define GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED    0x8E23
#define GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE    0x8E24
#define GL_TRANSFORM_FEEDBACK_BINDING          0x8E25
#define GL_MAX_TRANSFORM_FEEDBACK_BUFFERS      0x8E70
typedef void (APIENTRYP pfn_glMinSampleShading) (GLfloat);
extern pfn_glMinSampleShading fp_glMinSampleShading;
typedef void (APIENTRYP pfn_glBlendEquationi) (GLuint, GLenum);
extern pfn_glBlendEquationi fp_glBlendEquationi;
typedef void (APIENTRYP pfn_glBlendEquationSeparatei) (GLuint, GLenum, GLenum);
extern pfn_glBlendEquationSeparatei fp_glBlendEquationSeparatei;
typedef void (APIENTRYP pfn_glBlendFunci) (GLuint, GLenum, GLenum);
extern pfn_glBlendFunci fp_glBlendFunci;
typedef void (APIENTRYP pfn_glBlendFuncSeparatei) (GLuint, GLenum, GLenum, GLenum, GLenum);
extern pfn_glBlendFuncSeparatei fp_glBlendFuncSeparatei;
typedef void (APIENTRYP pfn_glDrawArraysIndirect) (GLenum, const void*);
extern pfn_glDrawArraysIndirect fp_glDrawArraysIndirect;
typedef void (APIENTRYP pfn_glDrawElementsIndirect) (GLenum, GLenum, const void*);
extern pfn_glDrawElementsIndirect fp_glDrawElementsIndirect;
typedef void (APIENTRYP pfn_glUniform1d) (GLint, GLdouble);
extern pfn_glUniform1d fp_glUniform1d;
typedef void (APIENTRYP pfn_glUniform2d) (GLint, GLdouble, GLdouble);
extern pfn_glUniform2d fp_glUniform2d;
typedef void (APIENTRYP pfn_glUniform3d) (GLint, GLdouble, GLdouble, GLdouble);
extern pfn_glUniform3d fp_glUniform3d;
typedef void (APIENTRYP pfn_glUniform4d) (GLint, GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glUniform4d fp_glUniform4d;
typedef void (APIENTRYP pfn_glUniform1dv) (GLint, GLsizei, const GLdouble*);
extern pfn_glUniform1dv fp_glUniform1dv;
typedef void (APIENTRYP pfn_glUniform2dv) (GLint, GLsizei, const GLdouble*);
extern pfn_glUniform2dv fp_glUniform2dv;
typedef void (APIENTRYP pfn_glUniform3dv) (GLint, GLsizei, const GLdouble*);
extern pfn_glUniform3dv fp_glUniform3dv;
typedef void (APIENTRYP pfn_glUniform4dv) (GLint, GLsizei, const GLdouble*);
extern pfn_glUniform4dv fp_glUniform4dv;
typedef void (APIENTRYP pfn_glUniformMatrix2dv) (GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glUniformMatrix2dv fp_glUniformMatrix2dv;
typedef void (APIENTRYP pfn_glUniformMatrix3dv) (GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glUniformMatrix3dv fp_glUniformMatrix3dv;
typedef void (APIENTRYP pfn_glUniformMatrix4dv) (GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glUniformMatrix4dv fp_glUniformMatrix4dv;
typedef void (APIENTRYP pfn_glUniformMatrix2x3dv) (GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glUniformMatrix2x3dv fp_glUniformMatrix2x3dv;
typedef void (APIENTRYP pfn_glUniformMatrix2x4dv) (GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glUniformMatrix2x4dv fp_glUniformMatrix2x4dv;
typedef void (APIENTRYP pfn_glUniformMatrix3x2dv) (GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glUniformMatrix3x2dv fp_glUniformMatrix3x2dv;
typedef void (APIENTRYP pfn_glUniformMatrix3x4dv) (GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glUniformMatrix3x4dv fp_glUniformMatrix3x4dv;
typedef void (APIENTRYP pfn_glUniformMatrix4x2dv) (GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glUniformMatrix4x2dv fp_glUniformMatrix4x2dv;
typedef void (APIENTRYP pfn_glUniformMatrix4x3dv) (GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glUniformMatrix4x3dv fp_glUniformMatrix4x3dv;
typedef void (APIENTRYP pfn_glGetUniformdv) (GLuint, GLint, GLdouble*);
extern pfn_glGetUniformdv fp_glGetUniformdv;
typedef GLint (APIENTRYP pfn_glGetSubroutineUniformLocation) (GLuint, GLenum, const GLchar*);
extern pfn_glGetSubroutineUniformLocation fp_glGetSubroutineUniformLocation;
typedef GLuint (APIENTRYP pfn_glGetSubroutineIndex) (GLuint, GLenum, const GLchar*);
extern pfn_glGetSubroutineIndex fp_glGetSubroutineIndex;
typedef void (APIENTRYP pfn_glGetActiveSubroutineUniformiv) (GLuint, GLenum, GLuint, GLenum, GLint*);
extern pfn_glGetActiveSubroutineUniformiv fp_glGetActiveSubroutineUniformiv;
typedef void (APIENTRYP pfn_glGetActiveSubroutineUniformName) (GLuint, GLenum, GLuint, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetActiveSubroutineUniformName fp_glGetActiveSubroutineUniformName;
typedef void (APIENTRYP pfn_glGetActiveSubroutineName) (GLuint, GLenum, GLuint, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetActiveSubroutineName fp_glGetActiveSubroutineName;
typedef void (APIENTRYP pfn_glUniformSubroutinesuiv) (GLenum, GLsizei, const GLuint*);
extern pfn_glUniformSubroutinesuiv fp_glUniformSubroutinesuiv;
typedef void (APIENTRYP pfn_glGetUniformSubroutineuiv) (GLenum, GLint, GLuint*);
extern pfn_glGetUniformSubroutineuiv fp_glGetUniformSubroutineuiv;
typedef void (APIENTRYP pfn_glGetProgramStageiv) (GLuint, GLenum, GLenum, GLint*);
extern pfn_glGetProgramStageiv fp_glGetProgramStageiv;
typedef void (APIENTRYP pfn_glPatchParameteri) (GLenum, GLint);
extern pfn_glPatchParameteri fp_glPatchParameteri;
typedef void (APIENTRYP pfn_glPatchParameterfv) (GLenum, const GLfloat*);
extern pfn_glPatchParameterfv fp_glPatchParameterfv;
typedef void (APIENTRYP pfn_glBindTransformFeedback) (GLenum, GLuint);
extern pfn_glBindTransformFeedback fp_glBindTransformFeedback;
typedef void (APIENTRYP pfn_glDeleteTransformFeedbacks) (GLsizei, const GLuint*);
extern pfn_glDeleteTransformFeedbacks fp_glDeleteTransformFeedbacks;
typedef void (APIENTRYP pfn_glGenTransformFeedbacks) (GLsizei, GLuint*);
extern pfn_glGenTransformFeedbacks fp_glGenTransformFeedbacks;
typedef GLboolean (APIENTRYP pfn_glIsTransformFeedback) (GLuint);
extern pfn_glIsTransformFeedback fp_glIsTransformFeedback;
typedef void (APIENTRYP pfn_glPauseTransformFeedback) ();
extern pfn_glPauseTransformFeedback fp_glPauseTransformFeedback;
typedef void (APIENTRYP pfn_glResumeTransformFeedback) ();
extern pfn_glResumeTransformFeedback fp_glResumeTransformFeedback;
typedef void (APIENTRYP pfn_glDrawTransformFeedback) (GLenum, GLuint);
extern pfn_glDrawTransformFeedback fp_glDrawTransformFeedback;
typedef void (APIENTRYP pfn_glDrawTransformFeedbackStream) (GLenum, GLuint, GLuint);
extern pfn_glDrawTransformFeedbackStream fp_glDrawTransformFeedbackStream;
typedef void (APIENTRYP pfn_glBeginQueryIndexed) (GLenum, GLuint, GLuint);
extern pfn_glBeginQueryIndexed fp_glBeginQueryIndexed;
typedef void (APIENTRYP pfn_glEndQueryIndexed) (GLenum, GLuint);
extern pfn_glEndQueryIndexed fp_glEndQueryIndexed;
typedef void (APIENTRYP pfn_glGetQueryIndexediv) (GLenum, GLuint, GLenum, GLint*);
extern pfn_glGetQueryIndexediv fp_glGetQueryIndexediv;

 /* GL_VERSION_4_1 */
extern GLboolean GLAD_VERSION_4_1;
#define GL_FIXED                               0x140C
#define GL_IMPLEMENTATION_COLOR_READ_TYPE      0x8B9A
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT    0x8B9B
#define GL_LOW_FLOAT                           0x8DF0
#define GL_MEDIUM_FLOAT                        0x8DF1
#define GL_HIGH_FLOAT                          0x8DF2
#define GL_LOW_INT                             0x8DF3
#define GL_MEDIUM_INT                          0x8DF4
#define GL_HIGH_INT                            0x8DF5
#define GL_SHADER_COMPILER                     0x8DFA
#define GL_SHADER_BINARY_FORMATS               0x8DF8
#define GL_NUM_SHADER_BINARY_FORMATS           0x8DF9
#define GL_MAX_VERTEX_UNIFORM_VECTORS          0x8DFB
#define GL_MAX_VARYING_VECTORS                 0x8DFC
#define GL_MAX_FRAGMENT_UNIFORM_VECTORS        0x8DFD
#define GL_RGB565                              0x8D62
#define GL_PROGRAM_BINARY_RETRIEVABLE_HINT     0x8257
#define GL_PROGRAM_BINARY_LENGTH               0x8741
#define GL_NUM_PROGRAM_BINARY_FORMATS          0x87FE
#define GL_PROGRAM_BINARY_FORMATS              0x87FF
#define GL_VERTEX_SHADER_BIT                   0x00000001
#define GL_FRAGMENT_SHADER_BIT                 0x00000002
#define GL_GEOMETRY_SHADER_BIT                 0x00000004
#define GL_TESS_CONTROL_SHADER_BIT             0x00000008
#define GL_TESS_EVALUATION_SHADER_BIT          0x00000010
#define GL_ALL_SHADER_BITS                     0xFFFFFFFF
#define GL_PROGRAM_SEPARABLE                   0x8258
#define GL_ACTIVE_PROGRAM                      0x8259
#define GL_PROGRAM_PIPELINE_BINDING            0x825A
#define GL_MAX_VIEWPORTS                       0x825B
#define GL_VIEWPORT_SUBPIXEL_BITS              0x825C
#define GL_VIEWPORT_BOUNDS_RANGE               0x825D
#define GL_LAYER_PROVOKING_VERTEX              0x825E
#define GL_VIEWPORT_INDEX_PROVOKING_VERTEX     0x825F
#define GL_UNDEFINED_VERTEX                    0x8260
typedef void (APIENTRYP pfn_glReleaseShaderCompiler) ();
extern pfn_glReleaseShaderCompiler fp_glReleaseShaderCompiler;
typedef void (APIENTRYP pfn_glShaderBinary) (GLsizei, const GLuint*, GLenum, const void*, GLsizei);
extern pfn_glShaderBinary fp_glShaderBinary;
typedef void (APIENTRYP pfn_glGetShaderPrecisionFormat) (GLenum, GLenum, GLint*, GLint*);
extern pfn_glGetShaderPrecisionFormat fp_glGetShaderPrecisionFormat;
typedef void (APIENTRYP pfn_glDepthRangef) (GLfloat, GLfloat);
extern pfn_glDepthRangef fp_glDepthRangef;
typedef void (APIENTRYP pfn_glClearDepthf) (GLfloat);
extern pfn_glClearDepthf fp_glClearDepthf;
typedef void (APIENTRYP pfn_glGetProgramBinary) (GLuint, GLsizei, GLsizei*, GLenum*, void*);
extern pfn_glGetProgramBinary fp_glGetProgramBinary;
typedef void (APIENTRYP pfn_glProgramBinary) (GLuint, GLenum, const void*, GLsizei);
extern pfn_glProgramBinary fp_glProgramBinary;
typedef void (APIENTRYP pfn_glProgramParameteri) (GLuint, GLenum, GLint);
extern pfn_glProgramParameteri fp_glProgramParameteri;
typedef void (APIENTRYP pfn_glUseProgramStages) (GLuint, GLbitfield, GLuint);
extern pfn_glUseProgramStages fp_glUseProgramStages;
typedef void (APIENTRYP pfn_glActiveShaderProgram) (GLuint, GLuint);
extern pfn_glActiveShaderProgram fp_glActiveShaderProgram;
typedef GLuint (APIENTRYP pfn_glCreateShaderProgramv) (GLenum, GLsizei, const GLchar**);
extern pfn_glCreateShaderProgramv fp_glCreateShaderProgramv;
typedef void (APIENTRYP pfn_glBindProgramPipeline) (GLuint);
extern pfn_glBindProgramPipeline fp_glBindProgramPipeline;
typedef void (APIENTRYP pfn_glDeleteProgramPipelines) (GLsizei, const GLuint*);
extern pfn_glDeleteProgramPipelines fp_glDeleteProgramPipelines;
typedef void (APIENTRYP pfn_glGenProgramPipelines) (GLsizei, GLuint*);
extern pfn_glGenProgramPipelines fp_glGenProgramPipelines;
typedef GLboolean (APIENTRYP pfn_glIsProgramPipeline) (GLuint);
extern pfn_glIsProgramPipeline fp_glIsProgramPipeline;
typedef void (APIENTRYP pfn_glGetProgramPipelineiv) (GLuint, GLenum, GLint*);
extern pfn_glGetProgramPipelineiv fp_glGetProgramPipelineiv;
typedef void (APIENTRYP pfn_glProgramUniform1i) (GLuint, GLint, GLint);
extern pfn_glProgramUniform1i fp_glProgramUniform1i;
typedef void (APIENTRYP pfn_glProgramUniform1iv) (GLuint, GLint, GLsizei, const GLint*);
extern pfn_glProgramUniform1iv fp_glProgramUniform1iv;
typedef void (APIENTRYP pfn_glProgramUniform1f) (GLuint, GLint, GLfloat);
extern pfn_glProgramUniform1f fp_glProgramUniform1f;
typedef void (APIENTRYP pfn_glProgramUniform1fv) (GLuint, GLint, GLsizei, const GLfloat*);
extern pfn_glProgramUniform1fv fp_glProgramUniform1fv;
typedef void (APIENTRYP pfn_glProgramUniform1d) (GLuint, GLint, GLdouble);
extern pfn_glProgramUniform1d fp_glProgramUniform1d;
typedef void (APIENTRYP pfn_glProgramUniform1dv) (GLuint, GLint, GLsizei, const GLdouble*);
extern pfn_glProgramUniform1dv fp_glProgramUniform1dv;
typedef void (APIENTRYP pfn_glProgramUniform1ui) (GLuint, GLint, GLuint);
extern pfn_glProgramUniform1ui fp_glProgramUniform1ui;
typedef void (APIENTRYP pfn_glProgramUniform1uiv) (GLuint, GLint, GLsizei, const GLuint*);
extern pfn_glProgramUniform1uiv fp_glProgramUniform1uiv;
typedef void (APIENTRYP pfn_glProgramUniform2i) (GLuint, GLint, GLint, GLint);
extern pfn_glProgramUniform2i fp_glProgramUniform2i;
typedef void (APIENTRYP pfn_glProgramUniform2iv) (GLuint, GLint, GLsizei, const GLint*);
extern pfn_glProgramUniform2iv fp_glProgramUniform2iv;
typedef void (APIENTRYP pfn_glProgramUniform2f) (GLuint, GLint, GLfloat, GLfloat);
extern pfn_glProgramUniform2f fp_glProgramUniform2f;
typedef void (APIENTRYP pfn_glProgramUniform2fv) (GLuint, GLint, GLsizei, const GLfloat*);
extern pfn_glProgramUniform2fv fp_glProgramUniform2fv;
typedef void (APIENTRYP pfn_glProgramUniform2d) (GLuint, GLint, GLdouble, GLdouble);
extern pfn_glProgramUniform2d fp_glProgramUniform2d;
typedef void (APIENTRYP pfn_glProgramUniform2dv) (GLuint, GLint, GLsizei, const GLdouble*);
extern pfn_glProgramUniform2dv fp_glProgramUniform2dv;
typedef void (APIENTRYP pfn_glProgramUniform2ui) (GLuint, GLint, GLuint, GLuint);
extern pfn_glProgramUniform2ui fp_glProgramUniform2ui;
typedef void (APIENTRYP pfn_glProgramUniform2uiv) (GLuint, GLint, GLsizei, const GLuint*);
extern pfn_glProgramUniform2uiv fp_glProgramUniform2uiv;
typedef void (APIENTRYP pfn_glProgramUniform3i) (GLuint, GLint, GLint, GLint, GLint);
extern pfn_glProgramUniform3i fp_glProgramUniform3i;
typedef void (APIENTRYP pfn_glProgramUniform3iv) (GLuint, GLint, GLsizei, const GLint*);
extern pfn_glProgramUniform3iv fp_glProgramUniform3iv;
typedef void (APIENTRYP pfn_glProgramUniform3f) (GLuint, GLint, GLfloat, GLfloat, GLfloat);
extern pfn_glProgramUniform3f fp_glProgramUniform3f;
typedef void (APIENTRYP pfn_glProgramUniform3fv) (GLuint, GLint, GLsizei, const GLfloat*);
extern pfn_glProgramUniform3fv fp_glProgramUniform3fv;
typedef void (APIENTRYP pfn_glProgramUniform3d) (GLuint, GLint, GLdouble, GLdouble, GLdouble);
extern pfn_glProgramUniform3d fp_glProgramUniform3d;
typedef void (APIENTRYP pfn_glProgramUniform3dv) (GLuint, GLint, GLsizei, const GLdouble*);
extern pfn_glProgramUniform3dv fp_glProgramUniform3dv;
typedef void (APIENTRYP pfn_glProgramUniform3ui) (GLuint, GLint, GLuint, GLuint, GLuint);
extern pfn_glProgramUniform3ui fp_glProgramUniform3ui;
typedef void (APIENTRYP pfn_glProgramUniform3uiv) (GLuint, GLint, GLsizei, const GLuint*);
extern pfn_glProgramUniform3uiv fp_glProgramUniform3uiv;
typedef void (APIENTRYP pfn_glProgramUniform4i) (GLuint, GLint, GLint, GLint, GLint, GLint);
extern pfn_glProgramUniform4i fp_glProgramUniform4i;
typedef void (APIENTRYP pfn_glProgramUniform4iv) (GLuint, GLint, GLsizei, const GLint*);
extern pfn_glProgramUniform4iv fp_glProgramUniform4iv;
typedef void (APIENTRYP pfn_glProgramUniform4f) (GLuint, GLint, GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glProgramUniform4f fp_glProgramUniform4f;
typedef void (APIENTRYP pfn_glProgramUniform4fv) (GLuint, GLint, GLsizei, const GLfloat*);
extern pfn_glProgramUniform4fv fp_glProgramUniform4fv;
typedef void (APIENTRYP pfn_glProgramUniform4d) (GLuint, GLint, GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glProgramUniform4d fp_glProgramUniform4d;
typedef void (APIENTRYP pfn_glProgramUniform4dv) (GLuint, GLint, GLsizei, const GLdouble*);
extern pfn_glProgramUniform4dv fp_glProgramUniform4dv;
typedef void (APIENTRYP pfn_glProgramUniform4ui) (GLuint, GLint, GLuint, GLuint, GLuint, GLuint);
extern pfn_glProgramUniform4ui fp_glProgramUniform4ui;
typedef void (APIENTRYP pfn_glProgramUniform4uiv) (GLuint, GLint, GLsizei, const GLuint*);
extern pfn_glProgramUniform4uiv fp_glProgramUniform4uiv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix2fv) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix2fv fp_glProgramUniformMatrix2fv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix3fv) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix3fv fp_glProgramUniformMatrix3fv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix4fv) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix4fv fp_glProgramUniformMatrix4fv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix2dv) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix2dv fp_glProgramUniformMatrix2dv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix3dv) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix3dv fp_glProgramUniformMatrix3dv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix4dv) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix4dv fp_glProgramUniformMatrix4dv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix2x3fv) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix2x3fv fp_glProgramUniformMatrix2x3fv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix3x2fv) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix3x2fv fp_glProgramUniformMatrix3x2fv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix2x4fv) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix2x4fv fp_glProgramUniformMatrix2x4fv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix4x2fv) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix4x2fv fp_glProgramUniformMatrix4x2fv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix3x4fv) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix3x4fv fp_glProgramUniformMatrix3x4fv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix4x3fv) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix4x3fv fp_glProgramUniformMatrix4x3fv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix2x3dv) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix2x3dv fp_glProgramUniformMatrix2x3dv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix3x2dv) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix3x2dv fp_glProgramUniformMatrix3x2dv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix2x4dv) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix2x4dv fp_glProgramUniformMatrix2x4dv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix4x2dv) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix4x2dv fp_glProgramUniformMatrix4x2dv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix3x4dv) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix3x4dv fp_glProgramUniformMatrix3x4dv;
typedef void (APIENTRYP pfn_glProgramUniformMatrix4x3dv) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix4x3dv fp_glProgramUniformMatrix4x3dv;
typedef void (APIENTRYP pfn_glValidateProgramPipeline) (GLuint);
extern pfn_glValidateProgramPipeline fp_glValidateProgramPipeline;
typedef void (APIENTRYP pfn_glGetProgramPipelineInfoLog) (GLuint, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetProgramPipelineInfoLog fp_glGetProgramPipelineInfoLog;
typedef void (APIENTRYP pfn_glVertexAttribL1d) (GLuint, GLdouble);
extern pfn_glVertexAttribL1d fp_glVertexAttribL1d;
typedef void (APIENTRYP pfn_glVertexAttribL2d) (GLuint, GLdouble, GLdouble);
extern pfn_glVertexAttribL2d fp_glVertexAttribL2d;
typedef void (APIENTRYP pfn_glVertexAttribL3d) (GLuint, GLdouble, GLdouble, GLdouble);
extern pfn_glVertexAttribL3d fp_glVertexAttribL3d;
typedef void (APIENTRYP pfn_glVertexAttribL4d) (GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glVertexAttribL4d fp_glVertexAttribL4d;
typedef void (APIENTRYP pfn_glVertexAttribL1dv) (GLuint, const GLdouble*);
extern pfn_glVertexAttribL1dv fp_glVertexAttribL1dv;
typedef void (APIENTRYP pfn_glVertexAttribL2dv) (GLuint, const GLdouble*);
extern pfn_glVertexAttribL2dv fp_glVertexAttribL2dv;
typedef void (APIENTRYP pfn_glVertexAttribL3dv) (GLuint, const GLdouble*);
extern pfn_glVertexAttribL3dv fp_glVertexAttribL3dv;
typedef void (APIENTRYP pfn_glVertexAttribL4dv) (GLuint, const GLdouble*);
extern pfn_glVertexAttribL4dv fp_glVertexAttribL4dv;
typedef void (APIENTRYP pfn_glVertexAttribLPointer) (GLuint, GLint, GLenum, GLsizei, const void*);
extern pfn_glVertexAttribLPointer fp_glVertexAttribLPointer;
typedef void (APIENTRYP pfn_glGetVertexAttribLdv) (GLuint, GLenum, GLdouble*);
extern pfn_glGetVertexAttribLdv fp_glGetVertexAttribLdv;
typedef void (APIENTRYP pfn_glViewportArrayv) (GLuint, GLsizei, const GLfloat*);
extern pfn_glViewportArrayv fp_glViewportArrayv;
typedef void (APIENTRYP pfn_glViewportIndexedf) (GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glViewportIndexedf fp_glViewportIndexedf;
typedef void (APIENTRYP pfn_glViewportIndexedfv) (GLuint, const GLfloat*);
extern pfn_glViewportIndexedfv fp_glViewportIndexedfv;
typedef void (APIENTRYP pfn_glScissorArrayv) (GLuint, GLsizei, const GLint*);
extern pfn_glScissorArrayv fp_glScissorArrayv;
typedef void (APIENTRYP pfn_glScissorIndexed) (GLuint, GLint, GLint, GLsizei, GLsizei);
extern pfn_glScissorIndexed fp_glScissorIndexed;
typedef void (APIENTRYP pfn_glScissorIndexedv) (GLuint, const GLint*);
extern pfn_glScissorIndexedv fp_glScissorIndexedv;
typedef void (APIENTRYP pfn_glDepthRangeArrayv) (GLuint, GLsizei, const GLdouble*);
extern pfn_glDepthRangeArrayv fp_glDepthRangeArrayv;
typedef void (APIENTRYP pfn_glDepthRangeIndexed) (GLuint, GLdouble, GLdouble);
extern pfn_glDepthRangeIndexed fp_glDepthRangeIndexed;
typedef void (APIENTRYP pfn_glGetFloati_v) (GLenum, GLuint, GLfloat*);
extern pfn_glGetFloati_v fp_glGetFloati_v;
typedef void (APIENTRYP pfn_glGetDoublei_v) (GLenum, GLuint, GLdouble*);
extern pfn_glGetDoublei_v fp_glGetDoublei_v;

 /* GL_VERSION_4_2 */
extern GLboolean GLAD_VERSION_4_2;
#define GL_COPY_READ_BUFFER_BINDING            0x8F36
#define GL_COPY_WRITE_BUFFER_BINDING           0x8F37
#define GL_TRANSFORM_FEEDBACK_ACTIVE           0x8E24
#define GL_TRANSFORM_FEEDBACK_PAUSED           0x8E23
#define GL_UNPACK_COMPRESSED_BLOCK_WIDTH       0x9127
#define GL_UNPACK_COMPRESSED_BLOCK_HEIGHT      0x9128
#define GL_UNPACK_COMPRESSED_BLOCK_DEPTH       0x9129
#define GL_UNPACK_COMPRESSED_BLOCK_SIZE        0x912A
#define GL_PACK_COMPRESSED_BLOCK_WIDTH         0x912B
#define GL_PACK_COMPRESSED_BLOCK_HEIGHT        0x912C
#define GL_PACK_COMPRESSED_BLOCK_DEPTH         0x912D
#define GL_PACK_COMPRESSED_BLOCK_SIZE          0x912E
#define GL_NUM_SAMPLE_COUNTS                   0x9380
#define GL_MIN_MAP_BUFFER_ALIGNMENT            0x90BC
#define GL_ATOMIC_COUNTER_BUFFER               0x92C0
#define GL_ATOMIC_COUNTER_BUFFER_BINDING       0x92C1
#define GL_ATOMIC_COUNTER_BUFFER_START         0x92C2
#define GL_ATOMIC_COUNTER_BUFFER_SIZE          0x92C3
#define GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE     0x92C4
#define GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS  0x92C5
#define GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES  0x92C6
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER  0x92C7
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER  0x92C8
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER  0x92C9
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER  0x92CA
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER  0x92CB
#define GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS   0x92CC
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS  0x92CD
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS  0x92CE
#define GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS  0x92CF
#define GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS  0x92D0
#define GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS  0x92D1
#define GL_MAX_VERTEX_ATOMIC_COUNTERS          0x92D2
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS    0x92D3
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS  0x92D4
#define GL_MAX_GEOMETRY_ATOMIC_COUNTERS        0x92D5
#define GL_MAX_FRAGMENT_ATOMIC_COUNTERS        0x92D6
#define GL_MAX_COMBINED_ATOMIC_COUNTERS        0x92D7
#define GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE      0x92D8
#define GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS  0x92DC
#define GL_ACTIVE_ATOMIC_COUNTER_BUFFERS       0x92D9
#define GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX  0x92DA
#define GL_UNSIGNED_INT_ATOMIC_COUNTER         0x92DB
#define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT     0x00000001
#define GL_ELEMENT_ARRAY_BARRIER_BIT           0x00000002
#define GL_UNIFORM_BARRIER_BIT                 0x00000004
#define GL_TEXTURE_FETCH_BARRIER_BIT           0x00000008
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT     0x00000020
#define GL_COMMAND_BARRIER_BIT                 0x00000040
#define GL_PIXEL_BUFFER_BARRIER_BIT            0x00000080
#define GL_TEXTURE_UPDATE_BARRIER_BIT          0x00000100
#define GL_BUFFER_UPDATE_BARRIER_BIT           0x00000200
#define GL_FRAMEBUFFER_BARRIER_BIT             0x00000400
#define GL_TRANSFORM_FEEDBACK_BARRIER_BIT      0x00000800
#define GL_ATOMIC_COUNTER_BARRIER_BIT          0x00001000
#define GL_ALL_BARRIER_BITS                    0xFFFFFFFF
#define GL_MAX_IMAGE_UNITS                     0x8F38
#define GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS  0x8F39
#define GL_IMAGE_BINDING_NAME                  0x8F3A
#define GL_IMAGE_BINDING_LEVEL                 0x8F3B
#define GL_IMAGE_BINDING_LAYERED               0x8F3C
#define GL_IMAGE_BINDING_LAYER                 0x8F3D
#define GL_IMAGE_BINDING_ACCESS                0x8F3E
#define GL_IMAGE_1D                            0x904C
#define GL_IMAGE_2D                            0x904D
#define GL_IMAGE_3D                            0x904E
#define GL_IMAGE_2D_RECT                       0x904F
#define GL_IMAGE_CUBE                          0x9050
#define GL_IMAGE_BUFFER                        0x9051
#define GL_IMAGE_1D_ARRAY                      0x9052
#define GL_IMAGE_2D_ARRAY                      0x9053
#define GL_IMAGE_CUBE_MAP_ARRAY                0x9054
#define GL_IMAGE_2D_MULTISAMPLE                0x9055
#define GL_IMAGE_2D_MULTISAMPLE_ARRAY          0x9056
#define GL_INT_IMAGE_1D                        0x9057
#define GL_INT_IMAGE_2D                        0x9058
#define GL_INT_IMAGE_3D                        0x9059
#define GL_INT_IMAGE_2D_RECT                   0x905A
#define GL_INT_IMAGE_CUBE                      0x905B
#define GL_INT_IMAGE_BUFFER                    0x905C
#define GL_INT_IMAGE_1D_ARRAY                  0x905D
#define GL_INT_IMAGE_2D_ARRAY                  0x905E
#define GL_INT_IMAGE_CUBE_MAP_ARRAY            0x905F
#define GL_INT_IMAGE_2D_MULTISAMPLE            0x9060
#define GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY      0x9061
#define GL_UNSIGNED_INT_IMAGE_1D               0x9062
#define GL_UNSIGNED_INT_IMAGE_2D               0x9063
#define GL_UNSIGNED_INT_IMAGE_3D               0x9064
#define GL_UNSIGNED_INT_IMAGE_2D_RECT          0x9065
#define GL_UNSIGNED_INT_IMAGE_CUBE             0x9066
#define GL_UNSIGNED_INT_IMAGE_BUFFER           0x9067
#define GL_UNSIGNED_INT_IMAGE_1D_ARRAY         0x9068
#define GL_UNSIGNED_INT_IMAGE_2D_ARRAY         0x9069
#define GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY   0x906A
#define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE   0x906B
#define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY  0x906C
#define GL_MAX_IMAGE_SAMPLES                   0x906D
#define GL_IMAGE_BINDING_FORMAT                0x906E
#define GL_IMAGE_FORMAT_COMPATIBILITY_TYPE     0x90C7
#define GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE  0x90C8
#define GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS  0x90C9
#define GL_MAX_VERTEX_IMAGE_UNIFORMS           0x90CA
#define GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS     0x90CB
#define GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS  0x90CC
#define GL_MAX_GEOMETRY_IMAGE_UNIFORMS         0x90CD
#define GL_MAX_FRAGMENT_IMAGE_UNIFORMS         0x90CE
#define GL_MAX_COMBINED_IMAGE_UNIFORMS         0x90CF
#define GL_COMPRESSED_RGBA_BPTC_UNORM          0x8E8C
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM    0x8E8D
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT    0x8E8E
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT  0x8E8F
#define GL_TEXTURE_IMMUTABLE_FORMAT            0x912F
typedef void (APIENTRYP pfn_glDrawArraysInstancedBaseInstance) (GLenum, GLint, GLsizei, GLsizei, GLuint);
extern pfn_glDrawArraysInstancedBaseInstance fp_glDrawArraysInstancedBaseInstance;
typedef void (APIENTRYP pfn_glDrawElementsInstancedBaseInstance) (GLenum, GLsizei, GLenum, const void*, GLsizei, GLuint);
extern pfn_glDrawElementsInstancedBaseInstance fp_glDrawElementsInstancedBaseInstance;
typedef void (APIENTRYP pfn_glDrawElementsInstancedBaseVertexBaseInstance) (GLenum, GLsizei, GLenum, const void*, GLsizei, GLint, GLuint);
extern pfn_glDrawElementsInstancedBaseVertexBaseInstance fp_glDrawElementsInstancedBaseVertexBaseInstance;
typedef void (APIENTRYP pfn_glGetInternalformativ) (GLenum, GLenum, GLenum, GLsizei, GLint*);
extern pfn_glGetInternalformativ fp_glGetInternalformativ;
typedef void (APIENTRYP pfn_glGetActiveAtomicCounterBufferiv) (GLuint, GLuint, GLenum, GLint*);
extern pfn_glGetActiveAtomicCounterBufferiv fp_glGetActiveAtomicCounterBufferiv;
typedef void (APIENTRYP pfn_glBindImageTexture) (GLuint, GLuint, GLint, GLboolean, GLint, GLenum, GLenum);
extern pfn_glBindImageTexture fp_glBindImageTexture;
typedef void (APIENTRYP pfn_glMemoryBarrier) (GLbitfield);
extern pfn_glMemoryBarrier fp_glMemoryBarrier;
typedef void (APIENTRYP pfn_glTexStorage1D) (GLenum, GLsizei, GLenum, GLsizei);
extern pfn_glTexStorage1D fp_glTexStorage1D;
typedef void (APIENTRYP pfn_glTexStorage2D) (GLenum, GLsizei, GLenum, GLsizei, GLsizei);
extern pfn_glTexStorage2D fp_glTexStorage2D;
typedef void (APIENTRYP pfn_glTexStorage3D) (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei);
extern pfn_glTexStorage3D fp_glTexStorage3D;
typedef void (APIENTRYP pfn_glDrawTransformFeedbackInstanced) (GLenum, GLuint, GLsizei);
extern pfn_glDrawTransformFeedbackInstanced fp_glDrawTransformFeedbackInstanced;
typedef void (APIENTRYP pfn_glDrawTransformFeedbackStreamInstanced) (GLenum, GLuint, GLuint, GLsizei);
extern pfn_glDrawTransformFeedbackStreamInstanced fp_glDrawTransformFeedbackStreamInstanced;

 /* GL_VERSION_4_3 */
extern GLboolean GLAD_VERSION_4_3;
#define GL_NUM_SHADING_LANGUAGE_VERSIONS       0x82E9
#define GL_VERTEX_ATTRIB_ARRAY_LONG            0x874E
#define GL_COMPRESSED_RGB8_ETC2                0x9274
#define GL_COMPRESSED_SRGB8_ETC2               0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2  0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2  0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC           0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC    0x9279
#define GL_COMPRESSED_R11_EAC                  0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC           0x9271
#define GL_COMPRESSED_RG11_EAC                 0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC          0x9273
#define GL_PRIMITIVE_RESTART_FIXED_INDEX       0x8D69
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE     0x8D6A
#define GL_MAX_ELEMENT_INDEX                   0x8D6B
#define GL_COMPUTE_SHADER                      0x91B9
#define GL_MAX_COMPUTE_UNIFORM_BLOCKS          0x91BB
#define GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS     0x91BC
#define GL_MAX_COMPUTE_IMAGE_UNIFORMS          0x91BD
#define GL_MAX_COMPUTE_SHARED_MEMORY_SIZE      0x8262
#define GL_MAX_COMPUTE_UNIFORM_COMPONENTS      0x8263
#define GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS  0x8264
#define GL_MAX_COMPUTE_ATOMIC_COUNTERS         0x8265
#define GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS  0x8266
#define GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS  0x90EB
#define GL_MAX_COMPUTE_WORK_GROUP_COUNT        0x91BE
#define GL_MAX_COMPUTE_WORK_GROUP_SIZE         0x91BF
#define GL_COMPUTE_WORK_GROUP_SIZE             0x8267
#define GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER  0x90EC
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER  0x90ED
#define GL_DISPATCH_INDIRECT_BUFFER            0x90EE
#define GL_DISPATCH_INDIRECT_BUFFER_BINDING    0x90EF
#define GL_COMPUTE_SHADER_BIT                  0x00000020
#define GL_DEBUG_OUTPUT_SYNCHRONOUS            0x8242
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH    0x8243
#define GL_DEBUG_CALLBACK_FUNCTION             0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM           0x8245
#define GL_DEBUG_SOURCE_API                    0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM          0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER        0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY            0x8249
#define GL_DEBUG_SOURCE_APPLICATION            0x824A
#define GL_DEBUG_SOURCE_OTHER                  0x824B
#define GL_DEBUG_TYPE_ERROR                    0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR      0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR       0x824E
#define GL_DEBUG_TYPE_PORTABILITY              0x824F
#define GL_DEBUG_TYPE_PERFORMANCE              0x8250
#define GL_DEBUG_TYPE_OTHER                    0x8251
#define GL_MAX_DEBUG_MESSAGE_LENGTH            0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES           0x9144
#define GL_DEBUG_LOGGED_MESSAGES               0x9145
#define GL_DEBUG_SEVERITY_HIGH                 0x9146
#define GL_DEBUG_SEVERITY_MEDIUM               0x9147
#define GL_DEBUG_SEVERITY_LOW                  0x9148
#define GL_DEBUG_TYPE_MARKER                   0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP               0x8269
#define GL_DEBUG_TYPE_POP_GROUP                0x826A
#define GL_DEBUG_SEVERITY_NOTIFICATION         0x826B
#define GL_MAX_DEBUG_GROUP_STACK_DEPTH         0x826C
#define GL_DEBUG_GROUP_STACK_DEPTH             0x826D
#define GL_BUFFER                              0x82E0
#define GL_SHADER                              0x82E1
#define GL_PROGRAM                             0x82E2
#define GL_QUERY                               0x82E3
#define GL_PROGRAM_PIPELINE                    0x82E4
#define GL_SAMPLER                             0x82E6
#define GL_MAX_LABEL_LENGTH                    0x82E8
#define GL_DEBUG_OUTPUT                        0x92E0
#define GL_CONTEXT_FLAG_DEBUG_BIT              0x00000002
#define GL_MAX_UNIFORM_LOCATIONS               0x826E
#define GL_FRAMEBUFFER_DEFAULT_WIDTH           0x9310
#define GL_FRAMEBUFFER_DEFAULT_HEIGHT          0x9311
#define GL_FRAMEBUFFER_DEFAULT_LAYERS          0x9312
#define GL_FRAMEBUFFER_DEFAULT_SAMPLES         0x9313
#define GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS  0x9314
#define GL_MAX_FRAMEBUFFER_WIDTH               0x9315
#define GL_MAX_FRAMEBUFFER_HEIGHT              0x9316
#define GL_MAX_FRAMEBUFFER_LAYERS              0x9317
#define GL_MAX_FRAMEBUFFER_SAMPLES             0x9318
#define GL_INTERNALFORMAT_SUPPORTED            0x826F
#define GL_INTERNALFORMAT_PREFERRED            0x8270
#define GL_INTERNALFORMAT_RED_SIZE             0x8271
#define GL_INTERNALFORMAT_GREEN_SIZE           0x8272
#define GL_INTERNALFORMAT_BLUE_SIZE            0x8273
#define GL_INTERNALFORMAT_ALPHA_SIZE           0x8274
#define GL_INTERNALFORMAT_DEPTH_SIZE           0x8275
#define GL_INTERNALFORMAT_STENCIL_SIZE         0x8276
#define GL_INTERNALFORMAT_SHARED_SIZE          0x8277
#define GL_INTERNALFORMAT_RED_TYPE             0x8278
#define GL_INTERNALFORMAT_GREEN_TYPE           0x8279
#define GL_INTERNALFORMAT_BLUE_TYPE            0x827A
#define GL_INTERNALFORMAT_ALPHA_TYPE           0x827B
#define GL_INTERNALFORMAT_DEPTH_TYPE           0x827C
#define GL_INTERNALFORMAT_STENCIL_TYPE         0x827D
#define GL_MAX_WIDTH                           0x827E
#define GL_MAX_HEIGHT                          0x827F
#define GL_MAX_DEPTH                           0x8280
#define GL_MAX_LAYERS                          0x8281
#define GL_MAX_COMBINED_DIMENSIONS             0x8282
#define GL_COLOR_COMPONENTS                    0x8283
#define GL_DEPTH_COMPONENTS                    0x8284
#define GL_STENCIL_COMPONENTS                  0x8285
#define GL_COLOR_RENDERABLE                    0x8286
#define GL_DEPTH_RENDERABLE                    0x8287
#define GL_STENCIL_RENDERABLE                  0x8288
#define GL_FRAMEBUFFER_RENDERABLE              0x8289
#define GL_FRAMEBUFFER_RENDERABLE_LAYERED      0x828A
#define GL_FRAMEBUFFER_BLEND                   0x828B
#define GL_READ_PIXELS                         0x828C
#define GL_READ_PIXELS_FORMAT                  0x828D
#define GL_READ_PIXELS_TYPE                    0x828E
#define GL_TEXTURE_IMAGE_FORMAT                0x828F
#define GL_TEXTURE_IMAGE_TYPE                  0x8290
#define GL_GET_TEXTURE_IMAGE_FORMAT            0x8291
#define GL_GET_TEXTURE_IMAGE_TYPE              0x8292
#define GL_MIPMAP                              0x8293
#define GL_MANUAL_GENERATE_MIPMAP              0x8294
#define GL_AUTO_GENERATE_MIPMAP                0x8295
#define GL_COLOR_ENCODING                      0x8296
#define GL_SRGB_READ                           0x8297
#define GL_SRGB_WRITE                          0x8298
#define GL_FILTER                              0x829A
#define GL_VERTEX_TEXTURE                      0x829B
#define GL_TESS_CONTROL_TEXTURE                0x829C
#define GL_TESS_EVALUATION_TEXTURE             0x829D
#define GL_GEOMETRY_TEXTURE                    0x829E
#define GL_FRAGMENT_TEXTURE                    0x829F
#define GL_COMPUTE_TEXTURE                     0x82A0
#define GL_TEXTURE_SHADOW                      0x82A1
#define GL_TEXTURE_GATHER                      0x82A2
#define GL_TEXTURE_GATHER_SHADOW               0x82A3
#define GL_SHADER_IMAGE_LOAD                   0x82A4
#define GL_SHADER_IMAGE_STORE                  0x82A5
#define GL_SHADER_IMAGE_ATOMIC                 0x82A6
#define GL_IMAGE_TEXEL_SIZE                    0x82A7
#define GL_IMAGE_COMPATIBILITY_CLASS           0x82A8
#define GL_IMAGE_PIXEL_FORMAT                  0x82A9
#define GL_IMAGE_PIXEL_TYPE                    0x82AA
#define GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST  0x82AC
#define GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST  0x82AD
#define GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE  0x82AE
#define GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE  0x82AF
#define GL_TEXTURE_COMPRESSED_BLOCK_WIDTH      0x82B1
#define GL_TEXTURE_COMPRESSED_BLOCK_HEIGHT     0x82B2
#define GL_TEXTURE_COMPRESSED_BLOCK_SIZE       0x82B3
#define GL_CLEAR_BUFFER                        0x82B4
#define GL_TEXTURE_VIEW                        0x82B5
#define GL_VIEW_COMPATIBILITY_CLASS            0x82B6
#define GL_FULL_SUPPORT                        0x82B7
#define GL_CAVEAT_SUPPORT                      0x82B8
#define GL_IMAGE_CLASS_4_X_32                  0x82B9
#define GL_IMAGE_CLASS_2_X_32                  0x82BA
#define GL_IMAGE_CLASS_1_X_32                  0x82BB
#define GL_IMAGE_CLASS_4_X_16                  0x82BC
#define GL_IMAGE_CLASS_2_X_16                  0x82BD
#define GL_IMAGE_CLASS_1_X_16                  0x82BE
#define GL_IMAGE_CLASS_4_X_8                   0x82BF
#define GL_IMAGE_CLASS_2_X_8                   0x82C0
#define GL_IMAGE_CLASS_1_X_8                   0x82C1
#define GL_IMAGE_CLASS_11_11_10                0x82C2
#define GL_IMAGE_CLASS_10_10_10_2              0x82C3
#define GL_VIEW_CLASS_128_BITS                 0x82C4
#define GL_VIEW_CLASS_96_BITS                  0x82C5
#define GL_VIEW_CLASS_64_BITS                  0x82C6
#define GL_VIEW_CLASS_48_BITS                  0x82C7
#define GL_VIEW_CLASS_32_BITS                  0x82C8
#define GL_VIEW_CLASS_24_BITS                  0x82C9
#define GL_VIEW_CLASS_16_BITS                  0x82CA
#define GL_VIEW_CLASS_8_BITS                   0x82CB
#define GL_VIEW_CLASS_S3TC_DXT1_RGB            0x82CC
#define GL_VIEW_CLASS_S3TC_DXT1_RGBA           0x82CD
#define GL_VIEW_CLASS_S3TC_DXT3_RGBA           0x82CE
#define GL_VIEW_CLASS_S3TC_DXT5_RGBA           0x82CF
#define GL_VIEW_CLASS_RGTC1_RED                0x82D0
#define GL_VIEW_CLASS_RGTC2_RG                 0x82D1
#define GL_VIEW_CLASS_BPTC_UNORM               0x82D2
#define GL_VIEW_CLASS_BPTC_FLOAT               0x82D3
#define GL_UNIFORM                             0x92E1
#define GL_UNIFORM_BLOCK                       0x92E2
#define GL_PROGRAM_INPUT                       0x92E3
#define GL_PROGRAM_OUTPUT                      0x92E4
#define GL_BUFFER_VARIABLE                     0x92E5
#define GL_SHADER_STORAGE_BLOCK                0x92E6
#define GL_VERTEX_SUBROUTINE                   0x92E8
#define GL_TESS_CONTROL_SUBROUTINE             0x92E9
#define GL_TESS_EVALUATION_SUBROUTINE          0x92EA
#define GL_GEOMETRY_SUBROUTINE                 0x92EB
#define GL_FRAGMENT_SUBROUTINE                 0x92EC
#define GL_COMPUTE_SUBROUTINE                  0x92ED
#define GL_VERTEX_SUBROUTINE_UNIFORM           0x92EE
#define GL_TESS_CONTROL_SUBROUTINE_UNIFORM     0x92EF
#define GL_TESS_EVALUATION_SUBROUTINE_UNIFORM  0x92F0
#define GL_GEOMETRY_SUBROUTINE_UNIFORM         0x92F1
#define GL_FRAGMENT_SUBROUTINE_UNIFORM         0x92F2
#define GL_COMPUTE_SUBROUTINE_UNIFORM          0x92F3
#define GL_TRANSFORM_FEEDBACK_VARYING          0x92F4
#define GL_ACTIVE_RESOURCES                    0x92F5
#define GL_MAX_NAME_LENGTH                     0x92F6
#define GL_MAX_NUM_ACTIVE_VARIABLES            0x92F7
#define GL_MAX_NUM_COMPATIBLE_SUBROUTINES      0x92F8
#define GL_NAME_LENGTH                         0x92F9
#define GL_TYPE                                0x92FA
#define GL_ARRAY_SIZE                          0x92FB
#define GL_OFFSET                              0x92FC
#define GL_BLOCK_INDEX                         0x92FD
#define GL_ARRAY_STRIDE                        0x92FE
#define GL_MATRIX_STRIDE                       0x92FF
#define GL_IS_ROW_MAJOR                        0x9300
#define GL_ATOMIC_COUNTER_BUFFER_INDEX         0x9301
#define GL_BUFFER_BINDING                      0x9302
#define GL_BUFFER_DATA_SIZE                    0x9303
#define GL_NUM_ACTIVE_VARIABLES                0x9304
#define GL_ACTIVE_VARIABLES                    0x9305
#define GL_REFERENCED_BY_VERTEX_SHADER         0x9306
#define GL_REFERENCED_BY_TESS_CONTROL_SHADER   0x9307
#define GL_REFERENCED_BY_TESS_EVALUATION_SHADER  0x9308
#define GL_REFERENCED_BY_GEOMETRY_SHADER       0x9309
#define GL_REFERENCED_BY_FRAGMENT_SHADER       0x930A
#define GL_REFERENCED_BY_COMPUTE_SHADER        0x930B
#define GL_TOP_LEVEL_ARRAY_SIZE                0x930C
#define GL_TOP_LEVEL_ARRAY_STRIDE              0x930D
#define GL_LOCATION                            0x930E
#define GL_LOCATION_INDEX                      0x930F
#define GL_IS_PER_PATCH                        0x92E7
#define GL_SHADER_STORAGE_BUFFER               0x90D2
#define GL_SHADER_STORAGE_BUFFER_BINDING       0x90D3
#define GL_SHADER_STORAGE_BUFFER_START         0x90D4
#define GL_SHADER_STORAGE_BUFFER_SIZE          0x90D5
#define GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS    0x90D6
#define GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS  0x90D7
#define GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS  0x90D8
#define GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS  0x90D9
#define GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS  0x90DA
#define GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS   0x90DB
#define GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS  0x90DC
#define GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS  0x90DD
#define GL_MAX_SHADER_STORAGE_BLOCK_SIZE       0x90DE
#define GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT  0x90DF
#define GL_SHADER_STORAGE_BARRIER_BIT          0x00002000
#define GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES  0x8F39
#define GL_DEPTH_STENCIL_TEXTURE_MODE          0x90EA
#define GL_TEXTURE_BUFFER_OFFSET               0x919D
#define GL_TEXTURE_BUFFER_SIZE                 0x919E
#define GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT     0x919F
#define GL_TEXTURE_VIEW_MIN_LEVEL              0x82DB
#define GL_TEXTURE_VIEW_NUM_LEVELS             0x82DC
#define GL_TEXTURE_VIEW_MIN_LAYER              0x82DD
#define GL_TEXTURE_VIEW_NUM_LAYERS             0x82DE
#define GL_TEXTURE_IMMUTABLE_LEVELS            0x82DF
#define GL_VERTEX_ATTRIB_BINDING               0x82D4
#define GL_VERTEX_ATTRIB_RELATIVE_OFFSET       0x82D5
#define GL_VERTEX_BINDING_DIVISOR              0x82D6
#define GL_VERTEX_BINDING_OFFSET               0x82D7
#define GL_VERTEX_BINDING_STRIDE               0x82D8
#define GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET   0x82D9
#define GL_MAX_VERTEX_ATTRIB_BINDINGS          0x82DA
#define GL_VERTEX_BINDING_BUFFER               0x8F4F
#define GL_DISPLAY_LIST                        0x82E7
typedef void (APIENTRYP pfn_glClearBufferData) (GLenum, GLenum, GLenum, GLenum, const void*);
extern pfn_glClearBufferData fp_glClearBufferData;
typedef void (APIENTRYP pfn_glClearBufferSubData) (GLenum, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void*);
extern pfn_glClearBufferSubData fp_glClearBufferSubData;
typedef void (APIENTRYP pfn_glDispatchCompute) (GLuint, GLuint, GLuint);
extern pfn_glDispatchCompute fp_glDispatchCompute;
typedef void (APIENTRYP pfn_glDispatchComputeIndirect) (GLintptr);
extern pfn_glDispatchComputeIndirect fp_glDispatchComputeIndirect;
typedef void (APIENTRYP pfn_glCopyImageSubData) (GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei);
extern pfn_glCopyImageSubData fp_glCopyImageSubData;
typedef void (APIENTRYP pfn_glFramebufferParameteri) (GLenum, GLenum, GLint);
extern pfn_glFramebufferParameteri fp_glFramebufferParameteri;
typedef void (APIENTRYP pfn_glGetFramebufferParameteriv) (GLenum, GLenum, GLint*);
extern pfn_glGetFramebufferParameteriv fp_glGetFramebufferParameteriv;
typedef void (APIENTRYP pfn_glGetInternalformati64v) (GLenum, GLenum, GLenum, GLsizei, GLint64*);
extern pfn_glGetInternalformati64v fp_glGetInternalformati64v;
typedef void (APIENTRYP pfn_glInvalidateTexSubImage) (GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei);
extern pfn_glInvalidateTexSubImage fp_glInvalidateTexSubImage;
typedef void (APIENTRYP pfn_glInvalidateTexImage) (GLuint, GLint);
extern pfn_glInvalidateTexImage fp_glInvalidateTexImage;
typedef void (APIENTRYP pfn_glInvalidateBufferSubData) (GLuint, GLintptr, GLsizeiptr);
extern pfn_glInvalidateBufferSubData fp_glInvalidateBufferSubData;
typedef void (APIENTRYP pfn_glInvalidateBufferData) (GLuint);
extern pfn_glInvalidateBufferData fp_glInvalidateBufferData;
typedef void (APIENTRYP pfn_glInvalidateFramebuffer) (GLenum, GLsizei, const GLenum*);
extern pfn_glInvalidateFramebuffer fp_glInvalidateFramebuffer;
typedef void (APIENTRYP pfn_glInvalidateSubFramebuffer) (GLenum, GLsizei, const GLenum*, GLint, GLint, GLsizei, GLsizei);
extern pfn_glInvalidateSubFramebuffer fp_glInvalidateSubFramebuffer;
typedef void (APIENTRYP pfn_glMultiDrawArraysIndirect) (GLenum, const void*, GLsizei, GLsizei);
extern pfn_glMultiDrawArraysIndirect fp_glMultiDrawArraysIndirect;
typedef void (APIENTRYP pfn_glMultiDrawElementsIndirect) (GLenum, GLenum, const void*, GLsizei, GLsizei);
extern pfn_glMultiDrawElementsIndirect fp_glMultiDrawElementsIndirect;
typedef void (APIENTRYP pfn_glGetProgramInterfaceiv) (GLuint, GLenum, GLenum, GLint*);
extern pfn_glGetProgramInterfaceiv fp_glGetProgramInterfaceiv;
typedef GLuint (APIENTRYP pfn_glGetProgramResourceIndex) (GLuint, GLenum, const GLchar*);
extern pfn_glGetProgramResourceIndex fp_glGetProgramResourceIndex;
typedef void (APIENTRYP pfn_glGetProgramResourceName) (GLuint, GLenum, GLuint, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetProgramResourceName fp_glGetProgramResourceName;
typedef void (APIENTRYP pfn_glGetProgramResourceiv) (GLuint, GLenum, GLuint, GLsizei, const GLenum*, GLsizei, GLsizei*, GLint*);
extern pfn_glGetProgramResourceiv fp_glGetProgramResourceiv;
typedef GLint (APIENTRYP pfn_glGetProgramResourceLocation) (GLuint, GLenum, const GLchar*);
extern pfn_glGetProgramResourceLocation fp_glGetProgramResourceLocation;
typedef GLint (APIENTRYP pfn_glGetProgramResourceLocationIndex) (GLuint, GLenum, const GLchar*);
extern pfn_glGetProgramResourceLocationIndex fp_glGetProgramResourceLocationIndex;
typedef void (APIENTRYP pfn_glShaderStorageBlockBinding) (GLuint, GLuint, GLuint);
extern pfn_glShaderStorageBlockBinding fp_glShaderStorageBlockBinding;
typedef void (APIENTRYP pfn_glTexBufferRange) (GLenum, GLenum, GLuint, GLintptr, GLsizeiptr);
extern pfn_glTexBufferRange fp_glTexBufferRange;
typedef void (APIENTRYP pfn_glTexStorage2DMultisample) (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean);
extern pfn_glTexStorage2DMultisample fp_glTexStorage2DMultisample;
typedef void (APIENTRYP pfn_glTexStorage3DMultisample) (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean);
extern pfn_glTexStorage3DMultisample fp_glTexStorage3DMultisample;
typedef void (APIENTRYP pfn_glTextureView) (GLuint, GLenum, GLuint, GLenum, GLuint, GLuint, GLuint, GLuint);
extern pfn_glTextureView fp_glTextureView;
typedef void (APIENTRYP pfn_glBindVertexBuffer) (GLuint, GLuint, GLintptr, GLsizei);
extern pfn_glBindVertexBuffer fp_glBindVertexBuffer;
typedef void (APIENTRYP pfn_glVertexAttribFormat) (GLuint, GLint, GLenum, GLboolean, GLuint);
extern pfn_glVertexAttribFormat fp_glVertexAttribFormat;
typedef void (APIENTRYP pfn_glVertexAttribIFormat) (GLuint, GLint, GLenum, GLuint);
extern pfn_glVertexAttribIFormat fp_glVertexAttribIFormat;
typedef void (APIENTRYP pfn_glVertexAttribLFormat) (GLuint, GLint, GLenum, GLuint);
extern pfn_glVertexAttribLFormat fp_glVertexAttribLFormat;
typedef void (APIENTRYP pfn_glVertexAttribBinding) (GLuint, GLuint);
extern pfn_glVertexAttribBinding fp_glVertexAttribBinding;
typedef void (APIENTRYP pfn_glVertexBindingDivisor) (GLuint, GLuint);
extern pfn_glVertexBindingDivisor fp_glVertexBindingDivisor;
typedef void (APIENTRYP pfn_glDebugMessageControl) (GLenum, GLenum, GLenum, GLsizei, const GLuint*, GLboolean);
extern pfn_glDebugMessageControl fp_glDebugMessageControl;
typedef void (APIENTRYP pfn_glDebugMessageInsert) (GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*);
extern pfn_glDebugMessageInsert fp_glDebugMessageInsert;
typedef void (APIENTRYP pfn_glDebugMessageCallback) (GLDEBUGPROC, const void*);
extern pfn_glDebugMessageCallback fp_glDebugMessageCallback;
typedef GLuint (APIENTRYP pfn_glGetDebugMessageLog) (GLuint, GLsizei, GLenum*, GLenum*, GLuint*, GLenum*, GLsizei*, GLchar*);
extern pfn_glGetDebugMessageLog fp_glGetDebugMessageLog;
typedef void (APIENTRYP pfn_glPushDebugGroup) (GLenum, GLuint, GLsizei, const GLchar*);
extern pfn_glPushDebugGroup fp_glPushDebugGroup;
typedef void (APIENTRYP pfn_glPopDebugGroup) ();
extern pfn_glPopDebugGroup fp_glPopDebugGroup;
typedef void (APIENTRYP pfn_glObjectLabel) (GLenum, GLuint, GLsizei, const GLchar*);
extern pfn_glObjectLabel fp_glObjectLabel;
typedef void (APIENTRYP pfn_glGetObjectLabel) (GLenum, GLuint, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetObjectLabel fp_glGetObjectLabel;
typedef void (APIENTRYP pfn_glObjectPtrLabel) (const void*, GLsizei, const GLchar*);
extern pfn_glObjectPtrLabel fp_glObjectPtrLabel;
typedef void (APIENTRYP pfn_glGetObjectPtrLabel) (const void*, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetObjectPtrLabel fp_glGetObjectPtrLabel;

 /* GL_VERSION_4_4 */
extern GLboolean GLAD_VERSION_4_4;
#define GL_MAX_VERTEX_ATTRIB_STRIDE            0x82E5
#define GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED  0x8221
#define GL_TEXTURE_BUFFER_BINDING              0x8C2A
#define GL_MAP_PERSISTENT_BIT                  0x0040
#define GL_MAP_COHERENT_BIT                    0x0080
#define GL_DYNAMIC_STORAGE_BIT                 0x0100
#define GL_CLIENT_STORAGE_BIT                  0x0200
#define GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT    0x00004000
#define GL_BUFFER_IMMUTABLE_STORAGE            0x821F
#define GL_BUFFER_STORAGE_FLAGS                0x8220
#define GL_CLEAR_TEXTURE                       0x9365
#define GL_LOCATION_COMPONENT                  0x934A
#define GL_TRANSFORM_FEEDBACK_BUFFER_INDEX     0x934B
#define GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE    0x934C
#define GL_QUERY_BUFFER                        0x9192
#define GL_QUERY_BUFFER_BARRIER_BIT            0x00008000
#define GL_QUERY_BUFFER_BINDING                0x9193
#define GL_QUERY_RESULT_NO_WAIT                0x9194
#define GL_MIRROR_CLAMP_TO_EDGE                0x8743
typedef void (APIENTRYP pfn_glBufferStorage) (GLenum, GLsizeiptr, const void*, GLbitfield);
extern pfn_glBufferStorage fp_glBufferStorage;
typedef void (APIENTRYP pfn_glClearTexImage) (GLuint, GLint, GLenum, GLenum, const void*);
extern pfn_glClearTexImage fp_glClearTexImage;
typedef void (APIENTRYP pfn_glClearTexSubImage) (GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*);
extern pfn_glClearTexSubImage fp_glClearTexSubImage;
typedef void (APIENTRYP pfn_glBindBuffersBase) (GLenum, GLuint, GLsizei, const GLuint*);
extern pfn_glBindBuffersBase fp_glBindBuffersBase;
typedef void (APIENTRYP pfn_glBindBuffersRange) (GLenum, GLuint, GLsizei, const GLuint*, const GLintptr*, const GLsizeiptr*);
extern pfn_glBindBuffersRange fp_glBindBuffersRange;
typedef void (APIENTRYP pfn_glBindTextures) (GLuint, GLsizei, const GLuint*);
extern pfn_glBindTextures fp_glBindTextures;
typedef void (APIENTRYP pfn_glBindSamplers) (GLuint, GLsizei, const GLuint*);
extern pfn_glBindSamplers fp_glBindSamplers;
typedef void (APIENTRYP pfn_glBindImageTextures) (GLuint, GLsizei, const GLuint*);
extern pfn_glBindImageTextures fp_glBindImageTextures;
typedef void (APIENTRYP pfn_glBindVertexBuffers) (GLuint, GLsizei, const GLuint*, const GLintptr*, const GLsizei*);
extern pfn_glBindVertexBuffers fp_glBindVertexBuffers;

 /* GL_VERSION_4_5 */
extern GLboolean GLAD_VERSION_4_5;
#define GL_CONTEXT_LOST                        0x0507
#define GL_NEGATIVE_ONE_TO_ONE                 0x935E
#define GL_ZERO_TO_ONE                         0x935F
#define GL_CLIP_ORIGIN                         0x935C
#define GL_CLIP_DEPTH_MODE                     0x935D
#define GL_QUERY_WAIT_INVERTED                 0x8E17
#define GL_QUERY_NO_WAIT_INVERTED              0x8E18
#define GL_QUERY_BY_REGION_WAIT_INVERTED       0x8E19
#define GL_QUERY_BY_REGION_NO_WAIT_INVERTED    0x8E1A
#define GL_MAX_CULL_DISTANCES                  0x82F9
#define GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES  0x82FA
#define GL_TEXTURE_TARGET                      0x1006
#define GL_QUERY_TARGET                        0x82EA
#define GL_GUILTY_CONTEXT_RESET                0x8253
#define GL_INNOCENT_CONTEXT_RESET              0x8254
#define GL_UNKNOWN_CONTEXT_RESET               0x8255
#define GL_RESET_NOTIFICATION_STRATEGY         0x8256
#define GL_LOSE_CONTEXT_ON_RESET               0x8252
#define GL_NO_RESET_NOTIFICATION               0x8261
#define GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT      0x00000004
#define GL_CONTEXT_RELEASE_BEHAVIOR            0x82FB
#define GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH      0x82FC
typedef void (APIENTRYP pfn_glClipControl) (GLenum, GLenum);
extern pfn_glClipControl fp_glClipControl;
typedef void (APIENTRYP pfn_glCreateTransformFeedbacks) (GLsizei, GLuint*);
extern pfn_glCreateTransformFeedbacks fp_glCreateTransformFeedbacks;
typedef void (APIENTRYP pfn_glTransformFeedbackBufferBase) (GLuint, GLuint, GLuint);
extern pfn_glTransformFeedbackBufferBase fp_glTransformFeedbackBufferBase;
typedef void (APIENTRYP pfn_glTransformFeedbackBufferRange) (GLuint, GLuint, GLuint, GLintptr, GLsizeiptr);
extern pfn_glTransformFeedbackBufferRange fp_glTransformFeedbackBufferRange;
typedef void (APIENTRYP pfn_glGetTransformFeedbackiv) (GLuint, GLenum, GLint*);
extern pfn_glGetTransformFeedbackiv fp_glGetTransformFeedbackiv;
typedef void (APIENTRYP pfn_glGetTransformFeedbacki_v) (GLuint, GLenum, GLuint, GLint*);
extern pfn_glGetTransformFeedbacki_v fp_glGetTransformFeedbacki_v;
typedef void (APIENTRYP pfn_glGetTransformFeedbacki64_v) (GLuint, GLenum, GLuint, GLint64*);
extern pfn_glGetTransformFeedbacki64_v fp_glGetTransformFeedbacki64_v;
typedef void (APIENTRYP pfn_glCreateBuffers) (GLsizei, GLuint*);
extern pfn_glCreateBuffers fp_glCreateBuffers;
typedef void (APIENTRYP pfn_glNamedBufferStorage) (GLuint, GLsizeiptr, const void*, GLbitfield);
extern pfn_glNamedBufferStorage fp_glNamedBufferStorage;
typedef void (APIENTRYP pfn_glNamedBufferData) (GLuint, GLsizeiptr, const void*, GLenum);
extern pfn_glNamedBufferData fp_glNamedBufferData;
typedef void (APIENTRYP pfn_glNamedBufferSubData) (GLuint, GLintptr, GLsizeiptr, const void*);
extern pfn_glNamedBufferSubData fp_glNamedBufferSubData;
typedef void (APIENTRYP pfn_glCopyNamedBufferSubData) (GLuint, GLuint, GLintptr, GLintptr, GLsizeiptr);
extern pfn_glCopyNamedBufferSubData fp_glCopyNamedBufferSubData;
typedef void (APIENTRYP pfn_glClearNamedBufferData) (GLuint, GLenum, GLenum, GLenum, const void*);
extern pfn_glClearNamedBufferData fp_glClearNamedBufferData;
typedef void (APIENTRYP pfn_glClearNamedBufferSubData) (GLuint, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void*);
extern pfn_glClearNamedBufferSubData fp_glClearNamedBufferSubData;
typedef void* (APIENTRYP pfn_glMapNamedBuffer) (GLuint, GLenum);
extern pfn_glMapNamedBuffer fp_glMapNamedBuffer;
typedef void* (APIENTRYP pfn_glMapNamedBufferRange) (GLuint, GLintptr, GLsizeiptr, GLbitfield);
extern pfn_glMapNamedBufferRange fp_glMapNamedBufferRange;
typedef GLboolean (APIENTRYP pfn_glUnmapNamedBuffer) (GLuint);
extern pfn_glUnmapNamedBuffer fp_glUnmapNamedBuffer;
typedef void (APIENTRYP pfn_glFlushMappedNamedBufferRange) (GLuint, GLintptr, GLsizeiptr);
extern pfn_glFlushMappedNamedBufferRange fp_glFlushMappedNamedBufferRange;
typedef void (APIENTRYP pfn_glGetNamedBufferParameteriv) (GLuint, GLenum, GLint*);
extern pfn_glGetNamedBufferParameteriv fp_glGetNamedBufferParameteriv;
typedef void (APIENTRYP pfn_glGetNamedBufferParameteri64v) (GLuint, GLenum, GLint64*);
extern pfn_glGetNamedBufferParameteri64v fp_glGetNamedBufferParameteri64v;
typedef void (APIENTRYP pfn_glGetNamedBufferPointerv) (GLuint, GLenum, void**);
extern pfn_glGetNamedBufferPointerv fp_glGetNamedBufferPointerv;
typedef void (APIENTRYP pfn_glGetNamedBufferSubData) (GLuint, GLintptr, GLsizeiptr, void*);
extern pfn_glGetNamedBufferSubData fp_glGetNamedBufferSubData;
typedef void (APIENTRYP pfn_glCreateFramebuffers) (GLsizei, GLuint*);
extern pfn_glCreateFramebuffers fp_glCreateFramebuffers;
typedef void (APIENTRYP pfn_glNamedFramebufferRenderbuffer) (GLuint, GLenum, GLenum, GLuint);
extern pfn_glNamedFramebufferRenderbuffer fp_glNamedFramebufferRenderbuffer;
typedef void (APIENTRYP pfn_glNamedFramebufferParameteri) (GLuint, GLenum, GLint);
extern pfn_glNamedFramebufferParameteri fp_glNamedFramebufferParameteri;
typedef void (APIENTRYP pfn_glNamedFramebufferTexture) (GLuint, GLenum, GLuint, GLint);
extern pfn_glNamedFramebufferTexture fp_glNamedFramebufferTexture;
typedef void (APIENTRYP pfn_glNamedFramebufferTextureLayer) (GLuint, GLenum, GLuint, GLint, GLint);
extern pfn_glNamedFramebufferTextureLayer fp_glNamedFramebufferTextureLayer;
typedef void (APIENTRYP pfn_glNamedFramebufferDrawBuffer) (GLuint, GLenum);
extern pfn_glNamedFramebufferDrawBuffer fp_glNamedFramebufferDrawBuffer;
typedef void (APIENTRYP pfn_glNamedFramebufferDrawBuffers) (GLuint, GLsizei, const GLenum*);
extern pfn_glNamedFramebufferDrawBuffers fp_glNamedFramebufferDrawBuffers;
typedef void (APIENTRYP pfn_glNamedFramebufferReadBuffer) (GLuint, GLenum);
extern pfn_glNamedFramebufferReadBuffer fp_glNamedFramebufferReadBuffer;
typedef void (APIENTRYP pfn_glInvalidateNamedFramebufferData) (GLuint, GLsizei, const GLenum*);
extern pfn_glInvalidateNamedFramebufferData fp_glInvalidateNamedFramebufferData;
typedef void (APIENTRYP pfn_glInvalidateNamedFramebufferSubData) (GLuint, GLsizei, const GLenum*, GLint, GLint, GLsizei, GLsizei);
extern pfn_glInvalidateNamedFramebufferSubData fp_glInvalidateNamedFramebufferSubData;
typedef void (APIENTRYP pfn_glClearNamedFramebufferiv) (GLuint, GLenum, GLint, const GLint*);
extern pfn_glClearNamedFramebufferiv fp_glClearNamedFramebufferiv;
typedef void (APIENTRYP pfn_glClearNamedFramebufferuiv) (GLuint, GLenum, GLint, const GLuint*);
extern pfn_glClearNamedFramebufferuiv fp_glClearNamedFramebufferuiv;
typedef void (APIENTRYP pfn_glClearNamedFramebufferfv) (GLuint, GLenum, GLint, const GLfloat*);
extern pfn_glClearNamedFramebufferfv fp_glClearNamedFramebufferfv;
typedef void (APIENTRYP pfn_glClearNamedFramebufferfi) (GLuint, GLenum, GLint, GLfloat, GLint);
extern pfn_glClearNamedFramebufferfi fp_glClearNamedFramebufferfi;
typedef void (APIENTRYP pfn_glBlitNamedFramebuffer) (GLuint, GLuint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
extern pfn_glBlitNamedFramebuffer fp_glBlitNamedFramebuffer;
typedef GLenum (APIENTRYP pfn_glCheckNamedFramebufferStatus) (GLuint, GLenum);
extern pfn_glCheckNamedFramebufferStatus fp_glCheckNamedFramebufferStatus;
typedef void (APIENTRYP pfn_glGetNamedFramebufferParameteriv) (GLuint, GLenum, GLint*);
extern pfn_glGetNamedFramebufferParameteriv fp_glGetNamedFramebufferParameteriv;
typedef void (APIENTRYP pfn_glGetNamedFramebufferAttachmentParameteriv) (GLuint, GLenum, GLenum, GLint*);
extern pfn_glGetNamedFramebufferAttachmentParameteriv fp_glGetNamedFramebufferAttachmentParameteriv;
typedef void (APIENTRYP pfn_glCreateRenderbuffers) (GLsizei, GLuint*);
extern pfn_glCreateRenderbuffers fp_glCreateRenderbuffers;
typedef void (APIENTRYP pfn_glNamedRenderbufferStorage) (GLuint, GLenum, GLsizei, GLsizei);
extern pfn_glNamedRenderbufferStorage fp_glNamedRenderbufferStorage;
typedef void (APIENTRYP pfn_glNamedRenderbufferStorageMultisample) (GLuint, GLsizei, GLenum, GLsizei, GLsizei);
extern pfn_glNamedRenderbufferStorageMultisample fp_glNamedRenderbufferStorageMultisample;
typedef void (APIENTRYP pfn_glGetNamedRenderbufferParameteriv) (GLuint, GLenum, GLint*);
extern pfn_glGetNamedRenderbufferParameteriv fp_glGetNamedRenderbufferParameteriv;
typedef void (APIENTRYP pfn_glCreateTextures) (GLenum, GLsizei, GLuint*);
extern pfn_glCreateTextures fp_glCreateTextures;
typedef void (APIENTRYP pfn_glTextureBuffer) (GLuint, GLenum, GLuint);
extern pfn_glTextureBuffer fp_glTextureBuffer;
typedef void (APIENTRYP pfn_glTextureBufferRange) (GLuint, GLenum, GLuint, GLintptr, GLsizeiptr);
extern pfn_glTextureBufferRange fp_glTextureBufferRange;
typedef void (APIENTRYP pfn_glTextureStorage1D) (GLuint, GLsizei, GLenum, GLsizei);
extern pfn_glTextureStorage1D fp_glTextureStorage1D;
typedef void (APIENTRYP pfn_glTextureStorage2D) (GLuint, GLsizei, GLenum, GLsizei, GLsizei);
extern pfn_glTextureStorage2D fp_glTextureStorage2D;
typedef void (APIENTRYP pfn_glTextureStorage3D) (GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLsizei);
extern pfn_glTextureStorage3D fp_glTextureStorage3D;
typedef void (APIENTRYP pfn_glTextureStorage2DMultisample) (GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLboolean);
extern pfn_glTextureStorage2DMultisample fp_glTextureStorage2DMultisample;
typedef void (APIENTRYP pfn_glTextureStorage3DMultisample) (GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean);
extern pfn_glTextureStorage3DMultisample fp_glTextureStorage3DMultisample;
typedef void (APIENTRYP pfn_glTextureSubImage1D) (GLuint, GLint, GLint, GLsizei, GLenum, GLenum, const void*);
extern pfn_glTextureSubImage1D fp_glTextureSubImage1D;
typedef void (APIENTRYP pfn_glTextureSubImage2D) (GLuint, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*);
extern pfn_glTextureSubImage2D fp_glTextureSubImage2D;
typedef void (APIENTRYP pfn_glTextureSubImage3D) (GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*);
extern pfn_glTextureSubImage3D fp_glTextureSubImage3D;
typedef void (APIENTRYP pfn_glCompressedTextureSubImage1D) (GLuint, GLint, GLint, GLsizei, GLenum, GLsizei, const void*);
extern pfn_glCompressedTextureSubImage1D fp_glCompressedTextureSubImage1D;
typedef void (APIENTRYP pfn_glCompressedTextureSubImage2D) (GLuint, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void*);
extern pfn_glCompressedTextureSubImage2D fp_glCompressedTextureSubImage2D;
typedef void (APIENTRYP pfn_glCompressedTextureSubImage3D) (GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void*);
extern pfn_glCompressedTextureSubImage3D fp_glCompressedTextureSubImage3D;
typedef void (APIENTRYP pfn_glCopyTextureSubImage1D) (GLuint, GLint, GLint, GLint, GLint, GLsizei);
extern pfn_glCopyTextureSubImage1D fp_glCopyTextureSubImage1D;
typedef void (APIENTRYP pfn_glCopyTextureSubImage2D) (GLuint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
extern pfn_glCopyTextureSubImage2D fp_glCopyTextureSubImage2D;
typedef void (APIENTRYP pfn_glCopyTextureSubImage3D) (GLuint, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
extern pfn_glCopyTextureSubImage3D fp_glCopyTextureSubImage3D;
typedef void (APIENTRYP pfn_glTextureParameterf) (GLuint, GLenum, GLfloat);
extern pfn_glTextureParameterf fp_glTextureParameterf;
typedef void (APIENTRYP pfn_glTextureParameterfv) (GLuint, GLenum, const GLfloat*);
extern pfn_glTextureParameterfv fp_glTextureParameterfv;
typedef void (APIENTRYP pfn_glTextureParameteri) (GLuint, GLenum, GLint);
extern pfn_glTextureParameteri fp_glTextureParameteri;
typedef void (APIENTRYP pfn_glTextureParameterIiv) (GLuint, GLenum, const GLint*);
extern pfn_glTextureParameterIiv fp_glTextureParameterIiv;
typedef void (APIENTRYP pfn_glTextureParameterIuiv) (GLuint, GLenum, const GLuint*);
extern pfn_glTextureParameterIuiv fp_glTextureParameterIuiv;
typedef void (APIENTRYP pfn_glTextureParameteriv) (GLuint, GLenum, const GLint*);
extern pfn_glTextureParameteriv fp_glTextureParameteriv;
typedef void (APIENTRYP pfn_glGenerateTextureMipmap) (GLuint);
extern pfn_glGenerateTextureMipmap fp_glGenerateTextureMipmap;
typedef void (APIENTRYP pfn_glBindTextureUnit) (GLuint, GLuint);
extern pfn_glBindTextureUnit fp_glBindTextureUnit;
typedef void (APIENTRYP pfn_glGetTextureImage) (GLuint, GLint, GLenum, GLenum, GLsizei, void*);
extern pfn_glGetTextureImage fp_glGetTextureImage;
typedef void (APIENTRYP pfn_glGetCompressedTextureImage) (GLuint, GLint, GLsizei, void*);
extern pfn_glGetCompressedTextureImage fp_glGetCompressedTextureImage;
typedef void (APIENTRYP pfn_glGetTextureLevelParameterfv) (GLuint, GLint, GLenum, GLfloat*);
extern pfn_glGetTextureLevelParameterfv fp_glGetTextureLevelParameterfv;
typedef void (APIENTRYP pfn_glGetTextureLevelParameteriv) (GLuint, GLint, GLenum, GLint*);
extern pfn_glGetTextureLevelParameteriv fp_glGetTextureLevelParameteriv;
typedef void (APIENTRYP pfn_glGetTextureParameterfv) (GLuint, GLenum, GLfloat*);
extern pfn_glGetTextureParameterfv fp_glGetTextureParameterfv;
typedef void (APIENTRYP pfn_glGetTextureParameterIiv) (GLuint, GLenum, GLint*);
extern pfn_glGetTextureParameterIiv fp_glGetTextureParameterIiv;
typedef void (APIENTRYP pfn_glGetTextureParameterIuiv) (GLuint, GLenum, GLuint*);
extern pfn_glGetTextureParameterIuiv fp_glGetTextureParameterIuiv;
typedef void (APIENTRYP pfn_glGetTextureParameteriv) (GLuint, GLenum, GLint*);
extern pfn_glGetTextureParameteriv fp_glGetTextureParameteriv;
typedef void (APIENTRYP pfn_glCreateVertexArrays) (GLsizei, GLuint*);
extern pfn_glCreateVertexArrays fp_glCreateVertexArrays;
typedef void (APIENTRYP pfn_glDisableVertexArrayAttrib) (GLuint, GLuint);
extern pfn_glDisableVertexArrayAttrib fp_glDisableVertexArrayAttrib;
typedef void (APIENTRYP pfn_glEnableVertexArrayAttrib) (GLuint, GLuint);
extern pfn_glEnableVertexArrayAttrib fp_glEnableVertexArrayAttrib;
typedef void (APIENTRYP pfn_glVertexArrayElementBuffer) (GLuint, GLuint);
extern pfn_glVertexArrayElementBuffer fp_glVertexArrayElementBuffer;
typedef void (APIENTRYP pfn_glVertexArrayVertexBuffer) (GLuint, GLuint, GLuint, GLintptr, GLsizei);
extern pfn_glVertexArrayVertexBuffer fp_glVertexArrayVertexBuffer;
typedef void (APIENTRYP pfn_glVertexArrayVertexBuffers) (GLuint, GLuint, GLsizei, const GLuint*, const GLintptr*, const GLsizei*);
extern pfn_glVertexArrayVertexBuffers fp_glVertexArrayVertexBuffers;
typedef void (APIENTRYP pfn_glVertexArrayAttribBinding) (GLuint, GLuint, GLuint);
extern pfn_glVertexArrayAttribBinding fp_glVertexArrayAttribBinding;
typedef void (APIENTRYP pfn_glVertexArrayAttribFormat) (GLuint, GLuint, GLint, GLenum, GLboolean, GLuint);
extern pfn_glVertexArrayAttribFormat fp_glVertexArrayAttribFormat;
typedef void (APIENTRYP pfn_glVertexArrayAttribIFormat) (GLuint, GLuint, GLint, GLenum, GLuint);
extern pfn_glVertexArrayAttribIFormat fp_glVertexArrayAttribIFormat;
typedef void (APIENTRYP pfn_glVertexArrayAttribLFormat) (GLuint, GLuint, GLint, GLenum, GLuint);
extern pfn_glVertexArrayAttribLFormat fp_glVertexArrayAttribLFormat;
typedef void (APIENTRYP pfn_glVertexArrayBindingDivisor) (GLuint, GLuint, GLuint);
extern pfn_glVertexArrayBindingDivisor fp_glVertexArrayBindingDivisor;
typedef void (APIENTRYP pfn_glGetVertexArrayiv) (GLuint, GLenum, GLint*);
extern pfn_glGetVertexArrayiv fp_glGetVertexArrayiv;
typedef void (APIENTRYP pfn_glGetVertexArrayIndexediv) (GLuint, GLuint, GLenum, GLint*);
extern pfn_glGetVertexArrayIndexediv fp_glGetVertexArrayIndexediv;
typedef void (APIENTRYP pfn_glGetVertexArrayIndexed64iv) (GLuint, GLuint, GLenum, GLint64*);
extern pfn_glGetVertexArrayIndexed64iv fp_glGetVertexArrayIndexed64iv;
typedef void (APIENTRYP pfn_glCreateSamplers) (GLsizei, GLuint*);
extern pfn_glCreateSamplers fp_glCreateSamplers;
typedef void (APIENTRYP pfn_glCreateProgramPipelines) (GLsizei, GLuint*);
extern pfn_glCreateProgramPipelines fp_glCreateProgramPipelines;
typedef void (APIENTRYP pfn_glCreateQueries) (GLenum, GLsizei, GLuint*);
extern pfn_glCreateQueries fp_glCreateQueries;
typedef void (APIENTRYP pfn_glGetQueryBufferObjecti64v) (GLuint, GLuint, GLenum, GLintptr);
extern pfn_glGetQueryBufferObjecti64v fp_glGetQueryBufferObjecti64v;
typedef void (APIENTRYP pfn_glGetQueryBufferObjectiv) (GLuint, GLuint, GLenum, GLintptr);
extern pfn_glGetQueryBufferObjectiv fp_glGetQueryBufferObjectiv;
typedef void (APIENTRYP pfn_glGetQueryBufferObjectui64v) (GLuint, GLuint, GLenum, GLintptr);
extern pfn_glGetQueryBufferObjectui64v fp_glGetQueryBufferObjectui64v;
typedef void (APIENTRYP pfn_glGetQueryBufferObjectuiv) (GLuint, GLuint, GLenum, GLintptr);
extern pfn_glGetQueryBufferObjectuiv fp_glGetQueryBufferObjectuiv;
typedef void (APIENTRYP pfn_glMemoryBarrierByRegion) (GLbitfield);
extern pfn_glMemoryBarrierByRegion fp_glMemoryBarrierByRegion;
typedef void (APIENTRYP pfn_glGetTextureSubImage) (GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, GLsizei, void*);
extern pfn_glGetTextureSubImage fp_glGetTextureSubImage;
typedef void (APIENTRYP pfn_glGetCompressedTextureSubImage) (GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLsizei, void*);
extern pfn_glGetCompressedTextureSubImage fp_glGetCompressedTextureSubImage;
typedef GLenum (APIENTRYP pfn_glGetGraphicsResetStatus) ();
extern pfn_glGetGraphicsResetStatus fp_glGetGraphicsResetStatus;
typedef void (APIENTRYP pfn_glGetnCompressedTexImage) (GLenum, GLint, GLsizei, void*);
extern pfn_glGetnCompressedTexImage fp_glGetnCompressedTexImage;
typedef void (APIENTRYP pfn_glGetnTexImage) (GLenum, GLint, GLenum, GLenum, GLsizei, void*);
extern pfn_glGetnTexImage fp_glGetnTexImage;
typedef void (APIENTRYP pfn_glGetnUniformdv) (GLuint, GLint, GLsizei, GLdouble*);
extern pfn_glGetnUniformdv fp_glGetnUniformdv;
typedef void (APIENTRYP pfn_glGetnUniformfv) (GLuint, GLint, GLsizei, GLfloat*);
extern pfn_glGetnUniformfv fp_glGetnUniformfv;
typedef void (APIENTRYP pfn_glGetnUniformiv) (GLuint, GLint, GLsizei, GLint*);
extern pfn_glGetnUniformiv fp_glGetnUniformiv;
typedef void (APIENTRYP pfn_glGetnUniformuiv) (GLuint, GLint, GLsizei, GLuint*);
extern pfn_glGetnUniformuiv fp_glGetnUniformuiv;
typedef void (APIENTRYP pfn_glReadnPixels) (GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLsizei, void*);
extern pfn_glReadnPixels fp_glReadnPixels;
typedef void (APIENTRYP pfn_glGetnMapdv) (GLenum, GLenum, GLsizei, GLdouble*);
extern pfn_glGetnMapdv fp_glGetnMapdv;
typedef void (APIENTRYP pfn_glGetnMapfv) (GLenum, GLenum, GLsizei, GLfloat*);
extern pfn_glGetnMapfv fp_glGetnMapfv;
typedef void (APIENTRYP pfn_glGetnMapiv) (GLenum, GLenum, GLsizei, GLint*);
extern pfn_glGetnMapiv fp_glGetnMapiv;
typedef void (APIENTRYP pfn_glGetnPixelMapfv) (GLenum, GLsizei, GLfloat*);
extern pfn_glGetnPixelMapfv fp_glGetnPixelMapfv;
typedef void (APIENTRYP pfn_glGetnPixelMapuiv) (GLenum, GLsizei, GLuint*);
extern pfn_glGetnPixelMapuiv fp_glGetnPixelMapuiv;
typedef void (APIENTRYP pfn_glGetnPixelMapusv) (GLenum, GLsizei, GLushort*);
extern pfn_glGetnPixelMapusv fp_glGetnPixelMapusv;
typedef void (APIENTRYP pfn_glGetnPolygonStipple) (GLsizei, GLubyte*);
extern pfn_glGetnPolygonStipple fp_glGetnPolygonStipple;
typedef void (APIENTRYP pfn_glGetnColorTable) (GLenum, GLenum, GLenum, GLsizei, void*);
extern pfn_glGetnColorTable fp_glGetnColorTable;
typedef void (APIENTRYP pfn_glGetnConvolutionFilter) (GLenum, GLenum, GLenum, GLsizei, void*);
extern pfn_glGetnConvolutionFilter fp_glGetnConvolutionFilter;
typedef void (APIENTRYP pfn_glGetnSeparableFilter) (GLenum, GLenum, GLenum, GLsizei, void*, GLsizei, void*, void*);
extern pfn_glGetnSeparableFilter fp_glGetnSeparableFilter;
typedef void (APIENTRYP pfn_glGetnHistogram) (GLenum, GLboolean, GLenum, GLenum, GLsizei, void*);
extern pfn_glGetnHistogram fp_glGetnHistogram;
typedef void (APIENTRYP pfn_glGetnMinmax) (GLenum, GLboolean, GLenum, GLenum, GLsizei, void*);
extern pfn_glGetnMinmax fp_glGetnMinmax;
typedef void (APIENTRYP pfn_glTextureBarrier) ();
extern pfn_glTextureBarrier fp_glTextureBarrier;

 /* GL_ES_VERSION_2_0 */
extern GLboolean GLAD_ES_VERSION_2_0;
#define GL_BLEND_EQUATION                      0x8009
#define GL_BLEND_COLOR                         0x8005
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS   0x8CD9

 /* GL_ES_VERSION_3_0 */
extern GLboolean GLAD_ES_VERSION_3_0;

 /* GL_ES_VERSION_3_1 */
extern GLboolean GLAD_ES_VERSION_3_1;

 /* GL_ES_VERSION_3_2 */
extern GLboolean GLAD_ES_VERSION_3_2;
#define GL_MULTISAMPLE_LINE_WIDTH_RANGE        0x9381
#define GL_MULTISAMPLE_LINE_WIDTH_GRANULARITY  0x9382
#define GL_MULTIPLY                            0x9294
#define GL_SCREEN                              0x9295
#define GL_OVERLAY                             0x9296
#define GL_DARKEN                              0x9297
#define GL_LIGHTEN                             0x9298
#define GL_COLORDODGE                          0x9299
#define GL_COLORBURN                           0x929A
#define GL_HARDLIGHT                           0x929B
#define GL_SOFTLIGHT                           0x929C
#define GL_DIFFERENCE                          0x929E
#define GL_EXCLUSION                           0x92A0
#define GL_HSL_HUE                             0x92AD
#define GL_HSL_SATURATION                      0x92AE
#define GL_HSL_COLOR                           0x92AF
#define GL_HSL_LUMINOSITY                      0x92B0
#define GL_PRIMITIVE_BOUNDING_BOX              0x92BE
#define GL_COMPRESSED_RGBA_ASTC_4x4            0x93B0
#define GL_COMPRESSED_RGBA_ASTC_5x4            0x93B1
#define GL_COMPRESSED_RGBA_ASTC_5x5            0x93B2
#define GL_COMPRESSED_RGBA_ASTC_6x5            0x93B3
#define GL_COMPRESSED_RGBA_ASTC_6x6            0x93B4
#define GL_COMPRESSED_RGBA_ASTC_8x5            0x93B5
#define GL_COMPRESSED_RGBA_ASTC_8x6            0x93B6
#define GL_COMPRESSED_RGBA_ASTC_8x8            0x93B7
#define GL_COMPRESSED_RGBA_ASTC_10x5           0x93B8
#define GL_COMPRESSED_RGBA_ASTC_10x6           0x93B9
#define GL_COMPRESSED_RGBA_ASTC_10x8           0x93BA
#define GL_COMPRESSED_RGBA_ASTC_10x10          0x93BB
#define GL_COMPRESSED_RGBA_ASTC_12x10          0x93BC
#define GL_COMPRESSED_RGBA_ASTC_12x12          0x93BD
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4    0x93D0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4    0x93D1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5    0x93D2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5    0x93D3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6    0x93D4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5    0x93D5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6    0x93D6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8    0x93D7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5   0x93D8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6   0x93D9
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8   0x93DA
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10  0x93DB
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10  0x93DC
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12  0x93DD
typedef void (APIENTRYP pfn_glBlendBarrier) ();
extern pfn_glBlendBarrier fp_glBlendBarrier;
typedef void (APIENTRYP pfn_glPrimitiveBoundingBox) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glPrimitiveBoundingBox fp_glPrimitiveBoundingBox;

 /* GL_KHR_blend_equation_advanced */
extern GLboolean GLAD_KHR_blend_equation_advanced;
#define GL_MULTIPLY_KHR                        0x9294
#define GL_SCREEN_KHR                          0x9295
#define GL_OVERLAY_KHR                         0x9296
#define GL_DARKEN_KHR                          0x9297
#define GL_LIGHTEN_KHR                         0x9298
#define GL_COLORDODGE_KHR                      0x9299
#define GL_COLORBURN_KHR                       0x929A
#define GL_HARDLIGHT_KHR                       0x929B
#define GL_SOFTLIGHT_KHR                       0x929C
#define GL_DIFFERENCE_KHR                      0x929E
#define GL_EXCLUSION_KHR                       0x92A0
#define GL_HSL_HUE_KHR                         0x92AD
#define GL_HSL_SATURATION_KHR                  0x92AE
#define GL_HSL_COLOR_KHR                       0x92AF
#define GL_HSL_LUMINOSITY_KHR                  0x92B0
typedef void (APIENTRYP pfn_glBlendBarrierKHR) ();
extern pfn_glBlendBarrierKHR fp_glBlendBarrierKHR;

 /* GL_KHR_blend_equation_advanced_coherent */
extern GLboolean GLAD_KHR_blend_equation_advanced_coherent;
#define GL_BLEND_ADVANCED_COHERENT_KHR         0x9285

 /* GL_KHR_context_flush_control */
extern GLboolean GLAD_KHR_context_flush_control;
#define GL_CONTEXT_RELEASE_BEHAVIOR_KHR        0x82FB
#define GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_KHR  0x82FC

 /* GL_KHR_debug */
extern GLboolean GLAD_KHR_debug;
#define GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR        0x8242
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_KHR  0x8243
#define GL_DEBUG_CALLBACK_FUNCTION_KHR         0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM_KHR       0x8245
#define GL_DEBUG_SOURCE_API_KHR                0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM_KHR      0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER_KHR    0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY_KHR        0x8249
#define GL_DEBUG_SOURCE_APPLICATION_KHR        0x824A
#define GL_DEBUG_SOURCE_OTHER_KHR              0x824B
#define GL_DEBUG_TYPE_ERROR_KHR                0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_KHR  0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_KHR   0x824E
#define GL_DEBUG_TYPE_PORTABILITY_KHR          0x824F
#define GL_DEBUG_TYPE_PERFORMANCE_KHR          0x8250
#define GL_DEBUG_TYPE_OTHER_KHR                0x8251
#define GL_DEBUG_TYPE_MARKER_KHR               0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP_KHR           0x8269
#define GL_DEBUG_TYPE_POP_GROUP_KHR            0x826A
#define GL_DEBUG_SEVERITY_NOTIFICATION_KHR     0x826B
#define GL_MAX_DEBUG_GROUP_STACK_DEPTH_KHR     0x826C
#define GL_DEBUG_GROUP_STACK_DEPTH_KHR         0x826D
#define GL_BUFFER_KHR                          0x82E0
#define GL_SHADER_KHR                          0x82E1
#define GL_PROGRAM_KHR                         0x82E2
#define GL_VERTEX_ARRAY_KHR                    0x8074
#define GL_QUERY_KHR                           0x82E3
#define GL_PROGRAM_PIPELINE_KHR                0x82E4
#define GL_SAMPLER_KHR                         0x82E6
#define GL_MAX_LABEL_LENGTH_KHR                0x82E8
#define GL_MAX_DEBUG_MESSAGE_LENGTH_KHR        0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES_KHR       0x9144
#define GL_DEBUG_LOGGED_MESSAGES_KHR           0x9145
#define GL_DEBUG_SEVERITY_HIGH_KHR             0x9146
#define GL_DEBUG_SEVERITY_MEDIUM_KHR           0x9147
#define GL_DEBUG_SEVERITY_LOW_KHR              0x9148
#define GL_DEBUG_OUTPUT_KHR                    0x92E0
#define GL_CONTEXT_FLAG_DEBUG_BIT_KHR          0x00000002
#define GL_STACK_OVERFLOW_KHR                  0x0503
#define GL_STACK_UNDERFLOW_KHR                 0x0504
typedef void (APIENTRYP pfn_glDebugMessageControlKHR) (GLenum, GLenum, GLenum, GLsizei, const GLuint*, GLboolean);
extern pfn_glDebugMessageControlKHR fp_glDebugMessageControlKHR;
typedef void (APIENTRYP pfn_glDebugMessageInsertKHR) (GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*);
extern pfn_glDebugMessageInsertKHR fp_glDebugMessageInsertKHR;
typedef void (APIENTRYP pfn_glDebugMessageCallbackKHR) (GLDEBUGPROCKHR, const void*);
extern pfn_glDebugMessageCallbackKHR fp_glDebugMessageCallbackKHR;
typedef GLuint (APIENTRYP pfn_glGetDebugMessageLogKHR) (GLuint, GLsizei, GLenum*, GLenum*, GLuint*, GLenum*, GLsizei*, GLchar*);
extern pfn_glGetDebugMessageLogKHR fp_glGetDebugMessageLogKHR;
typedef void (APIENTRYP pfn_glPushDebugGroupKHR) (GLenum, GLuint, GLsizei, const GLchar*);
extern pfn_glPushDebugGroupKHR fp_glPushDebugGroupKHR;
typedef void (APIENTRYP pfn_glPopDebugGroupKHR) ();
extern pfn_glPopDebugGroupKHR fp_glPopDebugGroupKHR;
typedef void (APIENTRYP pfn_glObjectLabelKHR) (GLenum, GLuint, GLsizei, const GLchar*);
extern pfn_glObjectLabelKHR fp_glObjectLabelKHR;
typedef void (APIENTRYP pfn_glGetObjectLabelKHR) (GLenum, GLuint, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetObjectLabelKHR fp_glGetObjectLabelKHR;
typedef void (APIENTRYP pfn_glObjectPtrLabelKHR) (const void*, GLsizei, const GLchar*);
extern pfn_glObjectPtrLabelKHR fp_glObjectPtrLabelKHR;
typedef void (APIENTRYP pfn_glGetObjectPtrLabelKHR) (const void*, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetObjectPtrLabelKHR fp_glGetObjectPtrLabelKHR;
typedef void (APIENTRYP pfn_glGetPointervKHR) (GLenum, void**);
extern pfn_glGetPointervKHR fp_glGetPointervKHR;

 /* GL_KHR_no_error */
extern GLboolean GLAD_KHR_no_error;
#define GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR       0x00000008

 /* GL_KHR_robust_buffer_access_behavior */
extern GLboolean GLAD_KHR_robust_buffer_access_behavior;

 /* GL_KHR_robustness */
extern GLboolean GLAD_KHR_robustness;
#define GL_CONTEXT_ROBUST_ACCESS               0x90F3
#define GL_CONTEXT_ROBUST_ACCESS_KHR           0x90F3
#define GL_LOSE_CONTEXT_ON_RESET_KHR           0x8252
#define GL_GUILTY_CONTEXT_RESET_KHR            0x8253
#define GL_INNOCENT_CONTEXT_RESET_KHR          0x8254
#define GL_UNKNOWN_CONTEXT_RESET_KHR           0x8255
#define GL_RESET_NOTIFICATION_STRATEGY_KHR     0x8256
#define GL_NO_RESET_NOTIFICATION_KHR           0x8261
#define GL_CONTEXT_LOST_KHR                    0x0507
typedef GLenum (APIENTRYP pfn_glGetGraphicsResetStatusKHR) ();
extern pfn_glGetGraphicsResetStatusKHR fp_glGetGraphicsResetStatusKHR;
typedef void (APIENTRYP pfn_glReadnPixelsKHR) (GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLsizei, void*);
extern pfn_glReadnPixelsKHR fp_glReadnPixelsKHR;
typedef void (APIENTRYP pfn_glGetnUniformfvKHR) (GLuint, GLint, GLsizei, GLfloat*);
extern pfn_glGetnUniformfvKHR fp_glGetnUniformfvKHR;
typedef void (APIENTRYP pfn_glGetnUniformivKHR) (GLuint, GLint, GLsizei, GLint*);
extern pfn_glGetnUniformivKHR fp_glGetnUniformivKHR;
typedef void (APIENTRYP pfn_glGetnUniformuivKHR) (GLuint, GLint, GLsizei, GLuint*);
extern pfn_glGetnUniformuivKHR fp_glGetnUniformuivKHR;

 /* GL_KHR_texture_compression_astc_hdr */
extern GLboolean GLAD_KHR_texture_compression_astc_hdr;
#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR        0x93B0
#define GL_COMPRESSED_RGBA_ASTC_5x4_KHR        0x93B1
#define GL_COMPRESSED_RGBA_ASTC_5x5_KHR        0x93B2
#define GL_COMPRESSED_RGBA_ASTC_6x5_KHR        0x93B3
#define GL_COMPRESSED_RGBA_ASTC_6x6_KHR        0x93B4
#define GL_COMPRESSED_RGBA_ASTC_8x5_KHR        0x93B5
#define GL_COMPRESSED_RGBA_ASTC_8x6_KHR        0x93B6
#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR        0x93B7
#define GL_COMPRESSED_RGBA_ASTC_10x5_KHR       0x93B8
#define GL_COMPRESSED_RGBA_ASTC_10x6_KHR       0x93B9
#define GL_COMPRESSED_RGBA_ASTC_10x8_KHR       0x93BA
#define GL_COMPRESSED_RGBA_ASTC_10x10_KHR      0x93BB
#define GL_COMPRESSED_RGBA_ASTC_12x10_KHR      0x93BC
#define GL_COMPRESSED_RGBA_ASTC_12x12_KHR      0x93BD
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR  0x93D0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR  0x93D1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR  0x93D2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR  0x93D3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR  0x93D4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR  0x93D5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR  0x93D6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR  0x93D7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR  0x93D8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR  0x93D9
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR  0x93DA
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR  0x93DB
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR  0x93DC
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR  0x93DD

 /* GL_KHR_texture_compression_astc_ldr */
extern GLboolean GLAD_KHR_texture_compression_astc_ldr;

 /* GL_KHR_texture_compression_astc_sliced_3d */
extern GLboolean GLAD_KHR_texture_compression_astc_sliced_3d;

 /* GL_ARB_arrays_of_arrays */
extern GLboolean GLAD_ARB_arrays_of_arrays;

 /* GL_ARB_base_instance */
extern GLboolean GLAD_ARB_base_instance;

 /* GL_ARB_bindless_texture */
extern GLboolean GLAD_ARB_bindless_texture;
#define GL_UNSIGNED_INT64_ARB                  0x140F
typedef GLuint64 (APIENTRYP pfn_glGetTextureHandleARB) (GLuint);
extern pfn_glGetTextureHandleARB fp_glGetTextureHandleARB;
typedef GLuint64 (APIENTRYP pfn_glGetTextureSamplerHandleARB) (GLuint, GLuint);
extern pfn_glGetTextureSamplerHandleARB fp_glGetTextureSamplerHandleARB;
typedef void (APIENTRYP pfn_glMakeTextureHandleResidentARB) (GLuint64);
extern pfn_glMakeTextureHandleResidentARB fp_glMakeTextureHandleResidentARB;
typedef void (APIENTRYP pfn_glMakeTextureHandleNonResidentARB) (GLuint64);
extern pfn_glMakeTextureHandleNonResidentARB fp_glMakeTextureHandleNonResidentARB;
typedef GLuint64 (APIENTRYP pfn_glGetImageHandleARB) (GLuint, GLint, GLboolean, GLint, GLenum);
extern pfn_glGetImageHandleARB fp_glGetImageHandleARB;
typedef void (APIENTRYP pfn_glMakeImageHandleResidentARB) (GLuint64, GLenum);
extern pfn_glMakeImageHandleResidentARB fp_glMakeImageHandleResidentARB;
typedef void (APIENTRYP pfn_glMakeImageHandleNonResidentARB) (GLuint64);
extern pfn_glMakeImageHandleNonResidentARB fp_glMakeImageHandleNonResidentARB;
typedef void (APIENTRYP pfn_glUniformHandleui64ARB) (GLint, GLuint64);
extern pfn_glUniformHandleui64ARB fp_glUniformHandleui64ARB;
typedef void (APIENTRYP pfn_glUniformHandleui64vARB) (GLint, GLsizei, const GLuint64*);
extern pfn_glUniformHandleui64vARB fp_glUniformHandleui64vARB;
typedef void (APIENTRYP pfn_glProgramUniformHandleui64ARB) (GLuint, GLint, GLuint64);
extern pfn_glProgramUniformHandleui64ARB fp_glProgramUniformHandleui64ARB;
typedef void (APIENTRYP pfn_glProgramUniformHandleui64vARB) (GLuint, GLint, GLsizei, const GLuint64*);
extern pfn_glProgramUniformHandleui64vARB fp_glProgramUniformHandleui64vARB;
typedef GLboolean (APIENTRYP pfn_glIsTextureHandleResidentARB) (GLuint64);
extern pfn_glIsTextureHandleResidentARB fp_glIsTextureHandleResidentARB;
typedef GLboolean (APIENTRYP pfn_glIsImageHandleResidentARB) (GLuint64);
extern pfn_glIsImageHandleResidentARB fp_glIsImageHandleResidentARB;
typedef void (APIENTRYP pfn_glVertexAttribL1ui64ARB) (GLuint, GLuint64EXT);
extern pfn_glVertexAttribL1ui64ARB fp_glVertexAttribL1ui64ARB;
typedef void (APIENTRYP pfn_glVertexAttribL1ui64vARB) (GLuint, const GLuint64EXT*);
extern pfn_glVertexAttribL1ui64vARB fp_glVertexAttribL1ui64vARB;
typedef void (APIENTRYP pfn_glGetVertexAttribLui64vARB) (GLuint, GLenum, GLuint64EXT*);
extern pfn_glGetVertexAttribLui64vARB fp_glGetVertexAttribLui64vARB;

 /* GL_ARB_blend_func_extended */
extern GLboolean GLAD_ARB_blend_func_extended;

 /* GL_ARB_buffer_storage */
extern GLboolean GLAD_ARB_buffer_storage;

 /* GL_ARB_cl_event */
extern GLboolean GLAD_ARB_cl_event;
#define GL_SYNC_CL_EVENT_ARB                   0x8240
#define GL_SYNC_CL_EVENT_COMPLETE_ARB          0x8241
typedef GLsync (APIENTRYP pfn_glCreateSyncFromCLeventARB) (struct _cl_context*, struct _cl_event*, GLbitfield);
extern pfn_glCreateSyncFromCLeventARB fp_glCreateSyncFromCLeventARB;

 /* GL_ARB_clear_buffer_object */
extern GLboolean GLAD_ARB_clear_buffer_object;

 /* GL_ARB_clear_texture */
extern GLboolean GLAD_ARB_clear_texture;

 /* GL_ARB_clip_control */
extern GLboolean GLAD_ARB_clip_control;

 /* GL_ARB_color_buffer_float */
extern GLboolean GLAD_ARB_color_buffer_float;
#define GL_RGBA_FLOAT_MODE_ARB                 0x8820
#define GL_CLAMP_VERTEX_COLOR_ARB              0x891A
#define GL_CLAMP_FRAGMENT_COLOR_ARB            0x891B
#define GL_CLAMP_READ_COLOR_ARB                0x891C
#define GL_FIXED_ONLY_ARB                      0x891D
typedef void (APIENTRYP pfn_glClampColorARB) (GLenum, GLenum);
extern pfn_glClampColorARB fp_glClampColorARB;

 /* GL_ARB_compatibility */
extern GLboolean GLAD_ARB_compatibility;

 /* GL_ARB_compressed_texture_pixel_storage */
extern GLboolean GLAD_ARB_compressed_texture_pixel_storage;

 /* GL_ARB_compute_shader */
extern GLboolean GLAD_ARB_compute_shader;

 /* GL_ARB_compute_variable_group_size */
extern GLboolean GLAD_ARB_compute_variable_group_size;
#define GL_MAX_COMPUTE_VARIABLE_GROUP_INVOCATIONS_ARB  0x9344
#define GL_MAX_COMPUTE_FIXED_GROUP_INVOCATIONS_ARB  0x90EB
#define GL_MAX_COMPUTE_VARIABLE_GROUP_SIZE_ARB  0x9345
#define GL_MAX_COMPUTE_FIXED_GROUP_SIZE_ARB    0x91BF
typedef void (APIENTRYP pfn_glDispatchComputeGroupSizeARB) (GLuint, GLuint, GLuint, GLuint, GLuint, GLuint);
extern pfn_glDispatchComputeGroupSizeARB fp_glDispatchComputeGroupSizeARB;

 /* GL_ARB_conditional_render_inverted */
extern GLboolean GLAD_ARB_conditional_render_inverted;

 /* GL_ARB_conservative_depth */
extern GLboolean GLAD_ARB_conservative_depth;

 /* GL_ARB_copy_buffer */
extern GLboolean GLAD_ARB_copy_buffer;

 /* GL_ARB_copy_image */
extern GLboolean GLAD_ARB_copy_image;

 /* GL_ARB_cull_distance */
extern GLboolean GLAD_ARB_cull_distance;

 /* GL_ARB_debug_output */
extern GLboolean GLAD_ARB_debug_output;
#define GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB        0x8242
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB  0x8243
#define GL_DEBUG_CALLBACK_FUNCTION_ARB         0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM_ARB       0x8245
#define GL_DEBUG_SOURCE_API_ARB                0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB      0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER_ARB    0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY_ARB        0x8249
#define GL_DEBUG_SOURCE_APPLICATION_ARB        0x824A
#define GL_DEBUG_SOURCE_OTHER_ARB              0x824B
#define GL_DEBUG_TYPE_ERROR_ARB                0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB  0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB   0x824E
#define GL_DEBUG_TYPE_PORTABILITY_ARB          0x824F
#define GL_DEBUG_TYPE_PERFORMANCE_ARB          0x8250
#define GL_DEBUG_TYPE_OTHER_ARB                0x8251
#define GL_MAX_DEBUG_MESSAGE_LENGTH_ARB        0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES_ARB       0x9144
#define GL_DEBUG_LOGGED_MESSAGES_ARB           0x9145
#define GL_DEBUG_SEVERITY_HIGH_ARB             0x9146
#define GL_DEBUG_SEVERITY_MEDIUM_ARB           0x9147
#define GL_DEBUG_SEVERITY_LOW_ARB              0x9148
typedef void (APIENTRYP pfn_glDebugMessageControlARB) (GLenum, GLenum, GLenum, GLsizei, const GLuint*, GLboolean);
extern pfn_glDebugMessageControlARB fp_glDebugMessageControlARB;
typedef void (APIENTRYP pfn_glDebugMessageInsertARB) (GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*);
extern pfn_glDebugMessageInsertARB fp_glDebugMessageInsertARB;
typedef void (APIENTRYP pfn_glDebugMessageCallbackARB) (GLDEBUGPROCARB, const void*);
extern pfn_glDebugMessageCallbackARB fp_glDebugMessageCallbackARB;
typedef GLuint (APIENTRYP pfn_glGetDebugMessageLogARB) (GLuint, GLsizei, GLenum*, GLenum*, GLuint*, GLenum*, GLsizei*, GLchar*);
extern pfn_glGetDebugMessageLogARB fp_glGetDebugMessageLogARB;

 /* GL_ARB_depth_buffer_float */
extern GLboolean GLAD_ARB_depth_buffer_float;

 /* GL_ARB_depth_clamp */
extern GLboolean GLAD_ARB_depth_clamp;

 /* GL_ARB_derivative_control */
extern GLboolean GLAD_ARB_derivative_control;

 /* GL_ARB_direct_state_access */
extern GLboolean GLAD_ARB_direct_state_access;

 /* GL_ARB_draw_buffers_blend */
extern GLboolean GLAD_ARB_draw_buffers_blend;
typedef void (APIENTRYP pfn_glBlendEquationiARB) (GLuint, GLenum);
extern pfn_glBlendEquationiARB fp_glBlendEquationiARB;
typedef void (APIENTRYP pfn_glBlendEquationSeparateiARB) (GLuint, GLenum, GLenum);
extern pfn_glBlendEquationSeparateiARB fp_glBlendEquationSeparateiARB;
typedef void (APIENTRYP pfn_glBlendFunciARB) (GLuint, GLenum, GLenum);
extern pfn_glBlendFunciARB fp_glBlendFunciARB;
typedef void (APIENTRYP pfn_glBlendFuncSeparateiARB) (GLuint, GLenum, GLenum, GLenum, GLenum);
extern pfn_glBlendFuncSeparateiARB fp_glBlendFuncSeparateiARB;

 /* GL_ARB_draw_elements_base_vertex */
extern GLboolean GLAD_ARB_draw_elements_base_vertex;

 /* GL_ARB_draw_indirect */
extern GLboolean GLAD_ARB_draw_indirect;

 /* GL_ARB_draw_instanced */
extern GLboolean GLAD_ARB_draw_instanced;
typedef void (APIENTRYP pfn_glDrawArraysInstancedARB) (GLenum, GLint, GLsizei, GLsizei);
extern pfn_glDrawArraysInstancedARB fp_glDrawArraysInstancedARB;
typedef void (APIENTRYP pfn_glDrawElementsInstancedARB) (GLenum, GLsizei, GLenum, const void*, GLsizei);
extern pfn_glDrawElementsInstancedARB fp_glDrawElementsInstancedARB;

 /* GL_ARB_enhanced_layouts */
extern GLboolean GLAD_ARB_enhanced_layouts;

 /* GL_ARB_ES2_compatibility */
extern GLboolean GLAD_ARB_ES2_compatibility;

 /* GL_ARB_ES3_1_compatibility */
extern GLboolean GLAD_ARB_ES3_1_compatibility;

 /* GL_ARB_ES3_2_compatibility */
extern GLboolean GLAD_ARB_ES3_2_compatibility;
#define GL_PRIMITIVE_BOUNDING_BOX_ARB          0x92BE
#define GL_MULTISAMPLE_LINE_WIDTH_RANGE_ARB    0x9381
#define GL_MULTISAMPLE_LINE_WIDTH_GRANULARITY_ARB  0x9382
typedef void (APIENTRYP pfn_glPrimitiveBoundingBoxARB) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glPrimitiveBoundingBoxARB fp_glPrimitiveBoundingBoxARB;

 /* GL_ARB_ES3_compatibility */
extern GLboolean GLAD_ARB_ES3_compatibility;

 /* GL_ARB_explicit_attrib_location */
extern GLboolean GLAD_ARB_explicit_attrib_location;

 /* GL_ARB_explicit_uniform_location */
extern GLboolean GLAD_ARB_explicit_uniform_location;

 /* GL_ARB_fragment_coord_conventions */
extern GLboolean GLAD_ARB_fragment_coord_conventions;

 /* GL_ARB_fragment_layer_viewport */
extern GLboolean GLAD_ARB_fragment_layer_viewport;

 /* GL_ARB_fragment_shader_interlock */
extern GLboolean GLAD_ARB_fragment_shader_interlock;

 /* GL_ARB_framebuffer_no_attachments */
extern GLboolean GLAD_ARB_framebuffer_no_attachments;

 /* GL_ARB_framebuffer_object */
extern GLboolean GLAD_ARB_framebuffer_object;

 /* GL_ARB_framebuffer_sRGB */
extern GLboolean GLAD_ARB_framebuffer_sRGB;

 /* GL_ARB_geometry_shader4 */
extern GLboolean GLAD_ARB_geometry_shader4;
#define GL_LINES_ADJACENCY_ARB                 0x000A
#define GL_LINE_STRIP_ADJACENCY_ARB            0x000B
#define GL_TRIANGLES_ADJACENCY_ARB             0x000C
#define GL_TRIANGLE_STRIP_ADJACENCY_ARB        0x000D
#define GL_PROGRAM_POINT_SIZE_ARB              0x8642
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB  0x8C29
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED_ARB  0x8DA7
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_ARB  0x8DA8
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_ARB  0x8DA9
#define GL_GEOMETRY_SHADER_ARB                 0x8DD9
#define GL_GEOMETRY_VERTICES_OUT_ARB           0x8DDA
#define GL_GEOMETRY_INPUT_TYPE_ARB             0x8DDB
#define GL_GEOMETRY_OUTPUT_TYPE_ARB            0x8DDC
#define GL_MAX_GEOMETRY_VARYING_COMPONENTS_ARB  0x8DDD
#define GL_MAX_VERTEX_VARYING_COMPONENTS_ARB   0x8DDE
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB  0x8DDF
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB    0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB  0x8DE1
typedef void (APIENTRYP pfn_glProgramParameteriARB) (GLuint, GLenum, GLint);
extern pfn_glProgramParameteriARB fp_glProgramParameteriARB;
typedef void (APIENTRYP pfn_glFramebufferTextureARB) (GLenum, GLenum, GLuint, GLint);
extern pfn_glFramebufferTextureARB fp_glFramebufferTextureARB;
typedef void (APIENTRYP pfn_glFramebufferTextureLayerARB) (GLenum, GLenum, GLuint, GLint, GLint);
extern pfn_glFramebufferTextureLayerARB fp_glFramebufferTextureLayerARB;
typedef void (APIENTRYP pfn_glFramebufferTextureFaceARB) (GLenum, GLenum, GLuint, GLint, GLenum);
extern pfn_glFramebufferTextureFaceARB fp_glFramebufferTextureFaceARB;

 /* GL_ARB_get_program_binary */
extern GLboolean GLAD_ARB_get_program_binary;

 /* GL_ARB_get_texture_sub_image */
extern GLboolean GLAD_ARB_get_texture_sub_image;

 /* GL_ARB_gpu_shader5 */
extern GLboolean GLAD_ARB_gpu_shader5;

 /* GL_ARB_gpu_shader_fp64 */
extern GLboolean GLAD_ARB_gpu_shader_fp64;

 /* GL_ARB_gpu_shader_int64 */
extern GLboolean GLAD_ARB_gpu_shader_int64;
#define GL_INT64_ARB                           0x140E
#define GL_INT64_VEC2_ARB                      0x8FE9
#define GL_INT64_VEC3_ARB                      0x8FEA
#define GL_INT64_VEC4_ARB                      0x8FEB
#define GL_UNSIGNED_INT64_VEC2_ARB             0x8FF5
#define GL_UNSIGNED_INT64_VEC3_ARB             0x8FF6
#define GL_UNSIGNED_INT64_VEC4_ARB             0x8FF7
typedef void (APIENTRYP pfn_glUniform1i64ARB) (GLint, GLint64);
extern pfn_glUniform1i64ARB fp_glUniform1i64ARB;
typedef void (APIENTRYP pfn_glUniform2i64ARB) (GLint, GLint64, GLint64);
extern pfn_glUniform2i64ARB fp_glUniform2i64ARB;
typedef void (APIENTRYP pfn_glUniform3i64ARB) (GLint, GLint64, GLint64, GLint64);
extern pfn_glUniform3i64ARB fp_glUniform3i64ARB;
typedef void (APIENTRYP pfn_glUniform4i64ARB) (GLint, GLint64, GLint64, GLint64, GLint64);
extern pfn_glUniform4i64ARB fp_glUniform4i64ARB;
typedef void (APIENTRYP pfn_glUniform1i64vARB) (GLint, GLsizei, const GLint64*);
extern pfn_glUniform1i64vARB fp_glUniform1i64vARB;
typedef void (APIENTRYP pfn_glUniform2i64vARB) (GLint, GLsizei, const GLint64*);
extern pfn_glUniform2i64vARB fp_glUniform2i64vARB;
typedef void (APIENTRYP pfn_glUniform3i64vARB) (GLint, GLsizei, const GLint64*);
extern pfn_glUniform3i64vARB fp_glUniform3i64vARB;
typedef void (APIENTRYP pfn_glUniform4i64vARB) (GLint, GLsizei, const GLint64*);
extern pfn_glUniform4i64vARB fp_glUniform4i64vARB;
typedef void (APIENTRYP pfn_glUniform1ui64ARB) (GLint, GLuint64);
extern pfn_glUniform1ui64ARB fp_glUniform1ui64ARB;
typedef void (APIENTRYP pfn_glUniform2ui64ARB) (GLint, GLuint64, GLuint64);
extern pfn_glUniform2ui64ARB fp_glUniform2ui64ARB;
typedef void (APIENTRYP pfn_glUniform3ui64ARB) (GLint, GLuint64, GLuint64, GLuint64);
extern pfn_glUniform3ui64ARB fp_glUniform3ui64ARB;
typedef void (APIENTRYP pfn_glUniform4ui64ARB) (GLint, GLuint64, GLuint64, GLuint64, GLuint64);
extern pfn_glUniform4ui64ARB fp_glUniform4ui64ARB;
typedef void (APIENTRYP pfn_glUniform1ui64vARB) (GLint, GLsizei, const GLuint64*);
extern pfn_glUniform1ui64vARB fp_glUniform1ui64vARB;
typedef void (APIENTRYP pfn_glUniform2ui64vARB) (GLint, GLsizei, const GLuint64*);
extern pfn_glUniform2ui64vARB fp_glUniform2ui64vARB;
typedef void (APIENTRYP pfn_glUniform3ui64vARB) (GLint, GLsizei, const GLuint64*);
extern pfn_glUniform3ui64vARB fp_glUniform3ui64vARB;
typedef void (APIENTRYP pfn_glUniform4ui64vARB) (GLint, GLsizei, const GLuint64*);
extern pfn_glUniform4ui64vARB fp_glUniform4ui64vARB;
typedef void (APIENTRYP pfn_glGetUniformi64vARB) (GLuint, GLint, GLint64*);
extern pfn_glGetUniformi64vARB fp_glGetUniformi64vARB;
typedef void (APIENTRYP pfn_glGetUniformui64vARB) (GLuint, GLint, GLuint64*);
extern pfn_glGetUniformui64vARB fp_glGetUniformui64vARB;
typedef void (APIENTRYP pfn_glGetnUniformi64vARB) (GLuint, GLint, GLsizei, GLint64*);
extern pfn_glGetnUniformi64vARB fp_glGetnUniformi64vARB;
typedef void (APIENTRYP pfn_glGetnUniformui64vARB) (GLuint, GLint, GLsizei, GLuint64*);
extern pfn_glGetnUniformui64vARB fp_glGetnUniformui64vARB;
typedef void (APIENTRYP pfn_glProgramUniform1i64ARB) (GLuint, GLint, GLint64);
extern pfn_glProgramUniform1i64ARB fp_glProgramUniform1i64ARB;
typedef void (APIENTRYP pfn_glProgramUniform2i64ARB) (GLuint, GLint, GLint64, GLint64);
extern pfn_glProgramUniform2i64ARB fp_glProgramUniform2i64ARB;
typedef void (APIENTRYP pfn_glProgramUniform3i64ARB) (GLuint, GLint, GLint64, GLint64, GLint64);
extern pfn_glProgramUniform3i64ARB fp_glProgramUniform3i64ARB;
typedef void (APIENTRYP pfn_glProgramUniform4i64ARB) (GLuint, GLint, GLint64, GLint64, GLint64, GLint64);
extern pfn_glProgramUniform4i64ARB fp_glProgramUniform4i64ARB;
typedef void (APIENTRYP pfn_glProgramUniform1i64vARB) (GLuint, GLint, GLsizei, const GLint64*);
extern pfn_glProgramUniform1i64vARB fp_glProgramUniform1i64vARB;
typedef void (APIENTRYP pfn_glProgramUniform2i64vARB) (GLuint, GLint, GLsizei, const GLint64*);
extern pfn_glProgramUniform2i64vARB fp_glProgramUniform2i64vARB;
typedef void (APIENTRYP pfn_glProgramUniform3i64vARB) (GLuint, GLint, GLsizei, const GLint64*);
extern pfn_glProgramUniform3i64vARB fp_glProgramUniform3i64vARB;
typedef void (APIENTRYP pfn_glProgramUniform4i64vARB) (GLuint, GLint, GLsizei, const GLint64*);
extern pfn_glProgramUniform4i64vARB fp_glProgramUniform4i64vARB;
typedef void (APIENTRYP pfn_glProgramUniform1ui64ARB) (GLuint, GLint, GLuint64);
extern pfn_glProgramUniform1ui64ARB fp_glProgramUniform1ui64ARB;
typedef void (APIENTRYP pfn_glProgramUniform2ui64ARB) (GLuint, GLint, GLuint64, GLuint64);
extern pfn_glProgramUniform2ui64ARB fp_glProgramUniform2ui64ARB;
typedef void (APIENTRYP pfn_glProgramUniform3ui64ARB) (GLuint, GLint, GLuint64, GLuint64, GLuint64);
extern pfn_glProgramUniform3ui64ARB fp_glProgramUniform3ui64ARB;
typedef void (APIENTRYP pfn_glProgramUniform4ui64ARB) (GLuint, GLint, GLuint64, GLuint64, GLuint64, GLuint64);
extern pfn_glProgramUniform4ui64ARB fp_glProgramUniform4ui64ARB;
typedef void (APIENTRYP pfn_glProgramUniform1ui64vARB) (GLuint, GLint, GLsizei, const GLuint64*);
extern pfn_glProgramUniform1ui64vARB fp_glProgramUniform1ui64vARB;
typedef void (APIENTRYP pfn_glProgramUniform2ui64vARB) (GLuint, GLint, GLsizei, const GLuint64*);
extern pfn_glProgramUniform2ui64vARB fp_glProgramUniform2ui64vARB;
typedef void (APIENTRYP pfn_glProgramUniform3ui64vARB) (GLuint, GLint, GLsizei, const GLuint64*);
extern pfn_glProgramUniform3ui64vARB fp_glProgramUniform3ui64vARB;
typedef void (APIENTRYP pfn_glProgramUniform4ui64vARB) (GLuint, GLint, GLsizei, const GLuint64*);
extern pfn_glProgramUniform4ui64vARB fp_glProgramUniform4ui64vARB;

 /* GL_ARB_half_float_pixel */
extern GLboolean GLAD_ARB_half_float_pixel;
#define GL_HALF_FLOAT_ARB                      0x140B

 /* GL_ARB_half_float_vertex */
extern GLboolean GLAD_ARB_half_float_vertex;

 /* GL_ARB_indirect_parameters */
extern GLboolean GLAD_ARB_indirect_parameters;
#define GL_PARAMETER_BUFFER_ARB                0x80EE
#define GL_PARAMETER_BUFFER_BINDING_ARB        0x80EF
typedef void (APIENTRYP pfn_glMultiDrawArraysIndirectCountARB) (GLenum, GLintptr, GLintptr, GLsizei, GLsizei);
extern pfn_glMultiDrawArraysIndirectCountARB fp_glMultiDrawArraysIndirectCountARB;
typedef void (APIENTRYP pfn_glMultiDrawElementsIndirectCountARB) (GLenum, GLenum, GLintptr, GLintptr, GLsizei, GLsizei);
extern pfn_glMultiDrawElementsIndirectCountARB fp_glMultiDrawElementsIndirectCountARB;

 /* GL_ARB_instanced_arrays */
extern GLboolean GLAD_ARB_instanced_arrays;
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ARB     0x88FE
typedef void (APIENTRYP pfn_glVertexAttribDivisorARB) (GLuint, GLuint);
extern pfn_glVertexAttribDivisorARB fp_glVertexAttribDivisorARB;

 /* GL_ARB_internalformat_query */
extern GLboolean GLAD_ARB_internalformat_query;

 /* GL_ARB_internalformat_query2 */
extern GLboolean GLAD_ARB_internalformat_query2;
#define GL_SRGB_DECODE_ARB                     0x8299

 /* GL_ARB_invalidate_subdata */
extern GLboolean GLAD_ARB_invalidate_subdata;

 /* GL_ARB_map_buffer_alignment */
extern GLboolean GLAD_ARB_map_buffer_alignment;

 /* GL_ARB_map_buffer_range */
extern GLboolean GLAD_ARB_map_buffer_range;

 /* GL_ARB_multi_bind */
extern GLboolean GLAD_ARB_multi_bind;

 /* GL_ARB_multi_draw_indirect */
extern GLboolean GLAD_ARB_multi_draw_indirect;

 /* GL_ARB_occlusion_query2 */
extern GLboolean GLAD_ARB_occlusion_query2;

 /* GL_ARB_parallel_shader_compile */
extern GLboolean GLAD_ARB_parallel_shader_compile;
#define GL_MAX_SHADER_COMPILER_THREADS_ARB     0x91B0
#define GL_COMPLETION_STATUS_ARB               0x91B1
typedef void (APIENTRYP pfn_glMaxShaderCompilerThreadsARB) (GLuint);
extern pfn_glMaxShaderCompilerThreadsARB fp_glMaxShaderCompilerThreadsARB;

 /* GL_ARB_pipeline_statistics_query */
extern GLboolean GLAD_ARB_pipeline_statistics_query;
#define GL_VERTICES_SUBMITTED_ARB              0x82EE
#define GL_PRIMITIVES_SUBMITTED_ARB            0x82EF
#define GL_VERTEX_SHADER_INVOCATIONS_ARB       0x82F0
#define GL_TESS_CONTROL_SHADER_PATCHES_ARB     0x82F1
#define GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB  0x82F2
#define GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB  0x82F3
#define GL_FRAGMENT_SHADER_INVOCATIONS_ARB     0x82F4
#define GL_COMPUTE_SHADER_INVOCATIONS_ARB      0x82F5
#define GL_CLIPPING_INPUT_PRIMITIVES_ARB       0x82F6
#define GL_CLIPPING_OUTPUT_PRIMITIVES_ARB      0x82F7

 /* GL_ARB_pixel_buffer_object */
extern GLboolean GLAD_ARB_pixel_buffer_object;
#define GL_PIXEL_PACK_BUFFER_ARB               0x88EB
#define GL_PIXEL_UNPACK_BUFFER_ARB             0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING_ARB       0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING_ARB     0x88EF

 /* GL_ARB_post_depth_coverage */
extern GLboolean GLAD_ARB_post_depth_coverage;

 /* GL_ARB_program_interface_query */
extern GLboolean GLAD_ARB_program_interface_query;

 /* GL_ARB_provoking_vertex */
extern GLboolean GLAD_ARB_provoking_vertex;

 /* GL_ARB_query_buffer_object */
extern GLboolean GLAD_ARB_query_buffer_object;

 /* GL_ARB_robust_buffer_access_behavior */
extern GLboolean GLAD_ARB_robust_buffer_access_behavior;

 /* GL_ARB_robustness */
extern GLboolean GLAD_ARB_robustness;
#define GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB  0x00000004
#define GL_LOSE_CONTEXT_ON_RESET_ARB           0x8252
#define GL_GUILTY_CONTEXT_RESET_ARB            0x8253
#define GL_INNOCENT_CONTEXT_RESET_ARB          0x8254
#define GL_UNKNOWN_CONTEXT_RESET_ARB           0x8255
#define GL_RESET_NOTIFICATION_STRATEGY_ARB     0x8256
#define GL_NO_RESET_NOTIFICATION_ARB           0x8261
typedef GLenum (APIENTRYP pfn_glGetGraphicsResetStatusARB) ();
extern pfn_glGetGraphicsResetStatusARB fp_glGetGraphicsResetStatusARB;
typedef void (APIENTRYP pfn_glGetnTexImageARB) (GLenum, GLint, GLenum, GLenum, GLsizei, void*);
extern pfn_glGetnTexImageARB fp_glGetnTexImageARB;
typedef void (APIENTRYP pfn_glReadnPixelsARB) (GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLsizei, void*);
extern pfn_glReadnPixelsARB fp_glReadnPixelsARB;
typedef void (APIENTRYP pfn_glGetnCompressedTexImageARB) (GLenum, GLint, GLsizei, void*);
extern pfn_glGetnCompressedTexImageARB fp_glGetnCompressedTexImageARB;
typedef void (APIENTRYP pfn_glGetnUniformfvARB) (GLuint, GLint, GLsizei, GLfloat*);
extern pfn_glGetnUniformfvARB fp_glGetnUniformfvARB;
typedef void (APIENTRYP pfn_glGetnUniformivARB) (GLuint, GLint, GLsizei, GLint*);
extern pfn_glGetnUniformivARB fp_glGetnUniformivARB;
typedef void (APIENTRYP pfn_glGetnUniformuivARB) (GLuint, GLint, GLsizei, GLuint*);
extern pfn_glGetnUniformuivARB fp_glGetnUniformuivARB;
typedef void (APIENTRYP pfn_glGetnUniformdvARB) (GLuint, GLint, GLsizei, GLdouble*);
extern pfn_glGetnUniformdvARB fp_glGetnUniformdvARB;
typedef void (APIENTRYP pfn_glGetnMapdvARB) (GLenum, GLenum, GLsizei, GLdouble*);
extern pfn_glGetnMapdvARB fp_glGetnMapdvARB;
typedef void (APIENTRYP pfn_glGetnMapfvARB) (GLenum, GLenum, GLsizei, GLfloat*);
extern pfn_glGetnMapfvARB fp_glGetnMapfvARB;
typedef void (APIENTRYP pfn_glGetnMapivARB) (GLenum, GLenum, GLsizei, GLint*);
extern pfn_glGetnMapivARB fp_glGetnMapivARB;
typedef void (APIENTRYP pfn_glGetnPixelMapfvARB) (GLenum, GLsizei, GLfloat*);
extern pfn_glGetnPixelMapfvARB fp_glGetnPixelMapfvARB;
typedef void (APIENTRYP pfn_glGetnPixelMapuivARB) (GLenum, GLsizei, GLuint*);
extern pfn_glGetnPixelMapuivARB fp_glGetnPixelMapuivARB;
typedef void (APIENTRYP pfn_glGetnPixelMapusvARB) (GLenum, GLsizei, GLushort*);
extern pfn_glGetnPixelMapusvARB fp_glGetnPixelMapusvARB;
typedef void (APIENTRYP pfn_glGetnPolygonStippleARB) (GLsizei, GLubyte*);
extern pfn_glGetnPolygonStippleARB fp_glGetnPolygonStippleARB;
typedef void (APIENTRYP pfn_glGetnColorTableARB) (GLenum, GLenum, GLenum, GLsizei, void*);
extern pfn_glGetnColorTableARB fp_glGetnColorTableARB;
typedef void (APIENTRYP pfn_glGetnConvolutionFilterARB) (GLenum, GLenum, GLenum, GLsizei, void*);
extern pfn_glGetnConvolutionFilterARB fp_glGetnConvolutionFilterARB;
typedef void (APIENTRYP pfn_glGetnSeparableFilterARB) (GLenum, GLenum, GLenum, GLsizei, void*, GLsizei, void*, void*);
extern pfn_glGetnSeparableFilterARB fp_glGetnSeparableFilterARB;
typedef void (APIENTRYP pfn_glGetnHistogramARB) (GLenum, GLboolean, GLenum, GLenum, GLsizei, void*);
extern pfn_glGetnHistogramARB fp_glGetnHistogramARB;
typedef void (APIENTRYP pfn_glGetnMinmaxARB) (GLenum, GLboolean, GLenum, GLenum, GLsizei, void*);
extern pfn_glGetnMinmaxARB fp_glGetnMinmaxARB;

 /* GL_ARB_robustness_isolation */
extern GLboolean GLAD_ARB_robustness_isolation;

 /* GL_ARB_sample_locations */
extern GLboolean GLAD_ARB_sample_locations;
#define GL_SAMPLE_LOCATION_SUBPIXEL_BITS_ARB   0x933D
#define GL_SAMPLE_LOCATION_PIXEL_GRID_WIDTH_ARB  0x933E
#define GL_SAMPLE_LOCATION_PIXEL_GRID_HEIGHT_ARB  0x933F
#define GL_PROGRAMMABLE_SAMPLE_LOCATION_TABLE_SIZE_ARB  0x9340
#define GL_SAMPLE_LOCATION_ARB                 0x8E50
#define GL_PROGRAMMABLE_SAMPLE_LOCATION_ARB    0x9341
#define GL_FRAMEBUFFER_PROGRAMMABLE_SAMPLE_LOCATIONS_ARB  0x9342
#define GL_FRAMEBUFFER_SAMPLE_LOCATION_PIXEL_GRID_ARB  0x9343
typedef void (APIENTRYP pfn_glFramebufferSampleLocationsfvARB) (GLenum, GLuint, GLsizei, const GLfloat*);
extern pfn_glFramebufferSampleLocationsfvARB fp_glFramebufferSampleLocationsfvARB;
typedef void (APIENTRYP pfn_glNamedFramebufferSampleLocationsfvARB) (GLuint, GLuint, GLsizei, const GLfloat*);
extern pfn_glNamedFramebufferSampleLocationsfvARB fp_glNamedFramebufferSampleLocationsfvARB;
typedef void (APIENTRYP pfn_glEvaluateDepthValuesARB) ();
extern pfn_glEvaluateDepthValuesARB fp_glEvaluateDepthValuesARB;

 /* GL_ARB_sample_shading */
extern GLboolean GLAD_ARB_sample_shading;
#define GL_SAMPLE_SHADING_ARB                  0x8C36
#define GL_MIN_SAMPLE_SHADING_VALUE_ARB        0x8C37
typedef void (APIENTRYP pfn_glMinSampleShadingARB) (GLfloat);
extern pfn_glMinSampleShadingARB fp_glMinSampleShadingARB;

 /* GL_ARB_sampler_objects */
extern GLboolean GLAD_ARB_sampler_objects;

 /* GL_ARB_seamless_cube_map */
extern GLboolean GLAD_ARB_seamless_cube_map;

 /* GL_ARB_seamless_cubemap_per_texture */
extern GLboolean GLAD_ARB_seamless_cubemap_per_texture;

 /* GL_ARB_separate_shader_objects */
extern GLboolean GLAD_ARB_separate_shader_objects;

 /* GL_ARB_shader_atomic_counter_ops */
extern GLboolean GLAD_ARB_shader_atomic_counter_ops;

 /* GL_ARB_shader_atomic_counters */
extern GLboolean GLAD_ARB_shader_atomic_counters;

 /* GL_ARB_shader_ballot */
extern GLboolean GLAD_ARB_shader_ballot;

 /* GL_ARB_shader_bit_encoding */
extern GLboolean GLAD_ARB_shader_bit_encoding;

 /* GL_ARB_shader_clock */
extern GLboolean GLAD_ARB_shader_clock;

 /* GL_ARB_shader_draw_parameters */
extern GLboolean GLAD_ARB_shader_draw_parameters;

 /* GL_ARB_shader_group_vote */
extern GLboolean GLAD_ARB_shader_group_vote;

 /* GL_ARB_shader_image_load_store */
extern GLboolean GLAD_ARB_shader_image_load_store;

 /* GL_ARB_shader_image_size */
extern GLboolean GLAD_ARB_shader_image_size;

 /* GL_ARB_shader_precision */
extern GLboolean GLAD_ARB_shader_precision;

 /* GL_ARB_shader_stencil_export */
extern GLboolean GLAD_ARB_shader_stencil_export;

 /* GL_ARB_shader_storage_buffer_object */
extern GLboolean GLAD_ARB_shader_storage_buffer_object;

 /* GL_ARB_shader_subroutine */
extern GLboolean GLAD_ARB_shader_subroutine;

 /* GL_ARB_shader_texture_image_samples */
extern GLboolean GLAD_ARB_shader_texture_image_samples;

 /* GL_ARB_shader_texture_lod */
extern GLboolean GLAD_ARB_shader_texture_lod;

 /* GL_ARB_shader_viewport_layer_array */
extern GLboolean GLAD_ARB_shader_viewport_layer_array;

 /* GL_ARB_shading_language_420pack */
extern GLboolean GLAD_ARB_shading_language_420pack;

 /* GL_ARB_shading_language_include */
extern GLboolean GLAD_ARB_shading_language_include;
#define GL_SHADER_INCLUDE_ARB                  0x8DAE
#define GL_NAMED_STRING_LENGTH_ARB             0x8DE9
#define GL_NAMED_STRING_TYPE_ARB               0x8DEA
typedef void (APIENTRYP pfn_glNamedStringARB) (GLenum, GLint, const GLchar*, GLint, const GLchar*);
extern pfn_glNamedStringARB fp_glNamedStringARB;
typedef void (APIENTRYP pfn_glDeleteNamedStringARB) (GLint, const GLchar*);
extern pfn_glDeleteNamedStringARB fp_glDeleteNamedStringARB;
typedef void (APIENTRYP pfn_glCompileShaderIncludeARB) (GLuint, GLsizei, const GLchar**, const GLint*);
extern pfn_glCompileShaderIncludeARB fp_glCompileShaderIncludeARB;
typedef GLboolean (APIENTRYP pfn_glIsNamedStringARB) (GLint, const GLchar*);
extern pfn_glIsNamedStringARB fp_glIsNamedStringARB;
typedef void (APIENTRYP pfn_glGetNamedStringARB) (GLint, const GLchar*, GLsizei, GLint*, GLchar*);
extern pfn_glGetNamedStringARB fp_glGetNamedStringARB;
typedef void (APIENTRYP pfn_glGetNamedStringivARB) (GLint, const GLchar*, GLenum, GLint*);
extern pfn_glGetNamedStringivARB fp_glGetNamedStringivARB;

 /* GL_ARB_shading_language_packing */
extern GLboolean GLAD_ARB_shading_language_packing;

 /* GL_ARB_sparse_buffer */
extern GLboolean GLAD_ARB_sparse_buffer;
#define GL_SPARSE_STORAGE_BIT_ARB              0x0400
#define GL_SPARSE_BUFFER_PAGE_SIZE_ARB         0x82F8
typedef void (APIENTRYP pfn_glBufferPageCommitmentARB) (GLenum, GLintptr, GLsizeiptr, GLboolean);
extern pfn_glBufferPageCommitmentARB fp_glBufferPageCommitmentARB;
typedef void (APIENTRYP pfn_glNamedBufferPageCommitmentEXT) (GLuint, GLintptr, GLsizeiptr, GLboolean);
extern pfn_glNamedBufferPageCommitmentEXT fp_glNamedBufferPageCommitmentEXT;
typedef void (APIENTRYP pfn_glNamedBufferPageCommitmentARB) (GLuint, GLintptr, GLsizeiptr, GLboolean);
extern pfn_glNamedBufferPageCommitmentARB fp_glNamedBufferPageCommitmentARB;

 /* GL_ARB_sparse_texture */
extern GLboolean GLAD_ARB_sparse_texture;
#define GL_TEXTURE_SPARSE_ARB                  0x91A6
#define GL_VIRTUAL_PAGE_SIZE_INDEX_ARB         0x91A7
#define GL_NUM_SPARSE_LEVELS_ARB               0x91AA
#define GL_NUM_VIRTUAL_PAGE_SIZES_ARB          0x91A8
#define GL_VIRTUAL_PAGE_SIZE_X_ARB             0x9195
#define GL_VIRTUAL_PAGE_SIZE_Y_ARB             0x9196
#define GL_VIRTUAL_PAGE_SIZE_Z_ARB             0x9197
#define GL_MAX_SPARSE_TEXTURE_SIZE_ARB         0x9198
#define GL_MAX_SPARSE_3D_TEXTURE_SIZE_ARB      0x9199
#define GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS_ARB  0x919A
#define GL_SPARSE_TEXTURE_FULL_ARRAY_CUBE_MIPMAPS_ARB  0x91A9
typedef void (APIENTRYP pfn_glTexPageCommitmentARB) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLboolean);
extern pfn_glTexPageCommitmentARB fp_glTexPageCommitmentARB;

 /* GL_ARB_sparse_texture2 */
extern GLboolean GLAD_ARB_sparse_texture2;

 /* GL_ARB_sparse_texture_clamp */
extern GLboolean GLAD_ARB_sparse_texture_clamp;

 /* GL_ARB_stencil_texturing */
extern GLboolean GLAD_ARB_stencil_texturing;

 /* GL_ARB_sync */
extern GLboolean GLAD_ARB_sync;

 /* GL_ARB_tessellation_shader */
extern GLboolean GLAD_ARB_tessellation_shader;

 /* GL_ARB_texture_barrier */
extern GLboolean GLAD_ARB_texture_barrier;

 /* GL_ARB_texture_buffer_object */
extern GLboolean GLAD_ARB_texture_buffer_object;
#define GL_TEXTURE_BUFFER_ARB                  0x8C2A
#define GL_MAX_TEXTURE_BUFFER_SIZE_ARB         0x8C2B
#define GL_TEXTURE_BINDING_BUFFER_ARB          0x8C2C
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING_ARB  0x8C2D
#define GL_TEXTURE_BUFFER_FORMAT_ARB           0x8C2E
typedef void (APIENTRYP pfn_glTexBufferARB) (GLenum, GLenum, GLuint);
extern pfn_glTexBufferARB fp_glTexBufferARB;

 /* GL_ARB_texture_buffer_object_rgb32 */
extern GLboolean GLAD_ARB_texture_buffer_object_rgb32;

 /* GL_ARB_texture_buffer_range */
extern GLboolean GLAD_ARB_texture_buffer_range;

 /* GL_ARB_texture_compression_bptc */
extern GLboolean GLAD_ARB_texture_compression_bptc;
#define GL_COMPRESSED_RGBA_BPTC_UNORM_ARB      0x8E8C
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB  0x8E8D
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB  0x8E8E
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB  0x8E8F

 /* GL_ARB_texture_compression_rgtc */
extern GLboolean GLAD_ARB_texture_compression_rgtc;

 /* GL_ARB_texture_cube_map_array */
extern GLboolean GLAD_ARB_texture_cube_map_array;
#define GL_TEXTURE_CUBE_MAP_ARRAY_ARB          0x9009
#define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY_ARB  0x900A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARRAY_ARB    0x900B
#define GL_SAMPLER_CUBE_MAP_ARRAY_ARB          0x900C
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_ARB   0x900D
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY_ARB      0x900E
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_ARB  0x900F

 /* GL_ARB_texture_filter_minmax */
extern GLboolean GLAD_ARB_texture_filter_minmax;
#define GL_TEXTURE_REDUCTION_MODE_ARB          0x9366
#define GL_WEIGHTED_AVERAGE_ARB                0x9367

 /* GL_ARB_texture_float */
extern GLboolean GLAD_ARB_texture_float;
#define GL_TEXTURE_RED_TYPE_ARB                0x8C10
#define GL_TEXTURE_GREEN_TYPE_ARB              0x8C11
#define GL_TEXTURE_BLUE_TYPE_ARB               0x8C12
#define GL_TEXTURE_ALPHA_TYPE_ARB              0x8C13
#define GL_TEXTURE_LUMINANCE_TYPE_ARB          0x8C14
#define GL_TEXTURE_INTENSITY_TYPE_ARB          0x8C15
#define GL_TEXTURE_DEPTH_TYPE_ARB              0x8C16
#define GL_UNSIGNED_NORMALIZED_ARB             0x8C17
#define GL_RGBA32F_ARB                         0x8814
#define GL_RGB32F_ARB                          0x8815
#define GL_ALPHA32F_ARB                        0x8816
#define GL_INTENSITY32F_ARB                    0x8817
#define GL_LUMINANCE32F_ARB                    0x8818
#define GL_LUMINANCE_ALPHA32F_ARB              0x8819
#define GL_RGBA16F_ARB                         0x881A
#define GL_RGB16F_ARB                          0x881B
#define GL_ALPHA16F_ARB                        0x881C
#define GL_INTENSITY16F_ARB                    0x881D
#define GL_LUMINANCE16F_ARB                    0x881E
#define GL_LUMINANCE_ALPHA16F_ARB              0x881F

 /* GL_ARB_texture_gather */
extern GLboolean GLAD_ARB_texture_gather;
#define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_ARB  0x8E5E
#define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_ARB  0x8E5F
#define GL_MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS_ARB  0x8F9F

 /* GL_ARB_texture_mirror_clamp_to_edge */
extern GLboolean GLAD_ARB_texture_mirror_clamp_to_edge;

 /* GL_ARB_texture_multisample */
extern GLboolean GLAD_ARB_texture_multisample;

 /* GL_ARB_texture_query_levels */
extern GLboolean GLAD_ARB_texture_query_levels;

 /* GL_ARB_texture_query_lod */
extern GLboolean GLAD_ARB_texture_query_lod;

 /* GL_ARB_texture_rectangle */
extern GLboolean GLAD_ARB_texture_rectangle;
#define GL_TEXTURE_RECTANGLE_ARB               0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_ARB       0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_ARB         0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB      0x84F8

 /* GL_ARB_texture_rg */
extern GLboolean GLAD_ARB_texture_rg;

 /* GL_ARB_texture_rgb10_a2ui */
extern GLboolean GLAD_ARB_texture_rgb10_a2ui;

 /* GL_ARB_texture_stencil8 */
extern GLboolean GLAD_ARB_texture_stencil8;

 /* GL_ARB_texture_storage */
extern GLboolean GLAD_ARB_texture_storage;

 /* GL_ARB_texture_storage_multisample */
extern GLboolean GLAD_ARB_texture_storage_multisample;

 /* GL_ARB_texture_swizzle */
extern GLboolean GLAD_ARB_texture_swizzle;

 /* GL_ARB_texture_view */
extern GLboolean GLAD_ARB_texture_view;

 /* GL_ARB_timer_query */
extern GLboolean GLAD_ARB_timer_query;

 /* GL_ARB_transform_feedback2 */
extern GLboolean GLAD_ARB_transform_feedback2;

 /* GL_ARB_transform_feedback3 */
extern GLboolean GLAD_ARB_transform_feedback3;

 /* GL_ARB_transform_feedback_instanced */
extern GLboolean GLAD_ARB_transform_feedback_instanced;

 /* GL_ARB_transform_feedback_overflow_query */
extern GLboolean GLAD_ARB_transform_feedback_overflow_query;
#define GL_TRANSFORM_FEEDBACK_OVERFLOW_ARB     0x82EC
#define GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB  0x82ED

 /* GL_ARB_uniform_buffer_object */
extern GLboolean GLAD_ARB_uniform_buffer_object;

 /* GL_ARB_vertex_array_bgra */
extern GLboolean GLAD_ARB_vertex_array_bgra;

 /* GL_ARB_vertex_array_object */
extern GLboolean GLAD_ARB_vertex_array_object;

 /* GL_ARB_vertex_attrib_64bit */
extern GLboolean GLAD_ARB_vertex_attrib_64bit;

 /* GL_ARB_vertex_attrib_binding */
extern GLboolean GLAD_ARB_vertex_attrib_binding;

 /* GL_ARB_vertex_type_10f_11f_11f_rev */
extern GLboolean GLAD_ARB_vertex_type_10f_11f_11f_rev;

 /* GL_ARB_vertex_type_2_10_10_10_rev */
extern GLboolean GLAD_ARB_vertex_type_2_10_10_10_rev;

 /* GL_ARB_viewport_array */
extern GLboolean GLAD_ARB_viewport_array;

 /* GL_EXT_base_instance */
extern GLboolean GLAD_EXT_base_instance;
typedef void (APIENTRYP pfn_glDrawArraysInstancedBaseInstanceEXT) (GLenum, GLint, GLsizei, GLsizei, GLuint);
extern pfn_glDrawArraysInstancedBaseInstanceEXT fp_glDrawArraysInstancedBaseInstanceEXT;
typedef void (APIENTRYP pfn_glDrawElementsInstancedBaseInstanceEXT) (GLenum, GLsizei, GLenum, const void*, GLsizei, GLuint);
extern pfn_glDrawElementsInstancedBaseInstanceEXT fp_glDrawElementsInstancedBaseInstanceEXT;
typedef void (APIENTRYP pfn_glDrawElementsInstancedBaseVertexBaseInstanceEXT) (GLenum, GLsizei, GLenum, const void*, GLsizei, GLint, GLuint);
extern pfn_glDrawElementsInstancedBaseVertexBaseInstanceEXT fp_glDrawElementsInstancedBaseVertexBaseInstanceEXT;

 /* GL_EXT_bgra */
extern GLboolean GLAD_EXT_bgra;
#define GL_BGR_EXT                             0x80E0
#define GL_BGRA_EXT                            0x80E1

 /* GL_EXT_bindable_uniform */
extern GLboolean GLAD_EXT_bindable_uniform;
#define GL_MAX_VERTEX_BINDABLE_UNIFORMS_EXT    0x8DE2
#define GL_MAX_FRAGMENT_BINDABLE_UNIFORMS_EXT  0x8DE3
#define GL_MAX_GEOMETRY_BINDABLE_UNIFORMS_EXT  0x8DE4
#define GL_MAX_BINDABLE_UNIFORM_SIZE_EXT       0x8DED
#define GL_UNIFORM_BUFFER_EXT                  0x8DEE
#define GL_UNIFORM_BUFFER_BINDING_EXT          0x8DEF
typedef void (APIENTRYP pfn_glUniformBufferEXT) (GLuint, GLint, GLuint);
extern pfn_glUniformBufferEXT fp_glUniformBufferEXT;
typedef GLint (APIENTRYP pfn_glGetUniformBufferSizeEXT) (GLuint, GLint);
extern pfn_glGetUniformBufferSizeEXT fp_glGetUniformBufferSizeEXT;
typedef GLintptr (APIENTRYP pfn_glGetUniformOffsetEXT) (GLuint, GLint);
extern pfn_glGetUniformOffsetEXT fp_glGetUniformOffsetEXT;

 /* GL_EXT_blend_func_extended */
extern GLboolean GLAD_EXT_blend_func_extended;
#define GL_SRC1_COLOR_EXT                      0x88F9
#define GL_SRC1_ALPHA_EXT                      0x8589
#define GL_ONE_MINUS_SRC1_COLOR_EXT            0x88FA
#define GL_ONE_MINUS_SRC1_ALPHA_EXT            0x88FB
#define GL_SRC_ALPHA_SATURATE_EXT              0x0308
#define GL_LOCATION_INDEX_EXT                  0x930F
#define GL_MAX_DUAL_SOURCE_DRAW_BUFFERS_EXT    0x88FC
typedef void (APIENTRYP pfn_glBindFragDataLocationIndexedEXT) (GLuint, GLuint, GLuint, const GLchar*);
extern pfn_glBindFragDataLocationIndexedEXT fp_glBindFragDataLocationIndexedEXT;
typedef void (APIENTRYP pfn_glBindFragDataLocationEXT) (GLuint, GLuint, const GLchar*);
extern pfn_glBindFragDataLocationEXT fp_glBindFragDataLocationEXT;
typedef GLint (APIENTRYP pfn_glGetProgramResourceLocationIndexEXT) (GLuint, GLenum, const GLchar*);
extern pfn_glGetProgramResourceLocationIndexEXT fp_glGetProgramResourceLocationIndexEXT;
typedef GLint (APIENTRYP pfn_glGetFragDataIndexEXT) (GLuint, const GLchar*);
extern pfn_glGetFragDataIndexEXT fp_glGetFragDataIndexEXT;

 /* GL_EXT_blend_minmax */
extern GLboolean GLAD_EXT_blend_minmax;
#define GL_MIN_EXT                             0x8007
#define GL_MAX_EXT                             0x8008
#define GL_FUNC_ADD_EXT                        0x8006
#define GL_BLEND_EQUATION_EXT                  0x8009
typedef void (APIENTRYP pfn_glBlendEquationEXT) (GLenum);
extern pfn_glBlendEquationEXT fp_glBlendEquationEXT;

 /* GL_EXT_buffer_storage */
extern GLboolean GLAD_EXT_buffer_storage;
#define GL_MAP_PERSISTENT_BIT_EXT              0x0040
#define GL_MAP_COHERENT_BIT_EXT                0x0080
#define GL_DYNAMIC_STORAGE_BIT_EXT             0x0100
#define GL_CLIENT_STORAGE_BIT_EXT              0x0200
#define GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT  0x00004000
#define GL_BUFFER_IMMUTABLE_STORAGE_EXT        0x821F
#define GL_BUFFER_STORAGE_FLAGS_EXT            0x8220
typedef void (APIENTRYP pfn_glBufferStorageEXT) (GLenum, GLsizeiptr, const void*, GLbitfield);
extern pfn_glBufferStorageEXT fp_glBufferStorageEXT;

 /* GL_EXT_color_buffer_float */
extern GLboolean GLAD_EXT_color_buffer_float;

 /* GL_EXT_color_buffer_half_float */
extern GLboolean GLAD_EXT_color_buffer_half_float;
#define GL_RGBA16F_EXT                         0x881A
#define GL_RGB16F_EXT                          0x881B
#define GL_RG16F_EXT                           0x822F
#define GL_R16F_EXT                            0x822D
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE_EXT  0x8211
#define GL_UNSIGNED_NORMALIZED_EXT             0x8C17

 /* GL_EXT_copy_image */
extern GLboolean GLAD_EXT_copy_image;
typedef void (APIENTRYP pfn_glCopyImageSubDataEXT) (GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei);
extern pfn_glCopyImageSubDataEXT fp_glCopyImageSubDataEXT;

 /* GL_EXT_debug_label */
extern GLboolean GLAD_EXT_debug_label;
#define GL_PROGRAM_PIPELINE_OBJECT_EXT         0x8A4F
#define GL_PROGRAM_OBJECT_EXT                  0x8B40
#define GL_SHADER_OBJECT_EXT                   0x8B48
#define GL_BUFFER_OBJECT_EXT                   0x9151
#define GL_QUERY_OBJECT_EXT                    0x9153
#define GL_VERTEX_ARRAY_OBJECT_EXT             0x9154
typedef void (APIENTRYP pfn_glLabelObjectEXT) (GLenum, GLuint, GLsizei, const GLchar*);
extern pfn_glLabelObjectEXT fp_glLabelObjectEXT;
typedef void (APIENTRYP pfn_glGetObjectLabelEXT) (GLenum, GLuint, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetObjectLabelEXT fp_glGetObjectLabelEXT;

 /* GL_EXT_debug_marker */
extern GLboolean GLAD_EXT_debug_marker;
typedef void (APIENTRYP pfn_glInsertEventMarkerEXT) (GLsizei, const GLchar*);
extern pfn_glInsertEventMarkerEXT fp_glInsertEventMarkerEXT;
typedef void (APIENTRYP pfn_glPushGroupMarkerEXT) (GLsizei, const GLchar*);
extern pfn_glPushGroupMarkerEXT fp_glPushGroupMarkerEXT;
typedef void (APIENTRYP pfn_glPopGroupMarkerEXT) ();
extern pfn_glPopGroupMarkerEXT fp_glPopGroupMarkerEXT;

 /* GL_EXT_direct_state_access */
extern GLboolean GLAD_EXT_direct_state_access;
#define GL_PROGRAM_MATRIX_EXT                  0x8E2D
#define GL_TRANSPOSE_PROGRAM_MATRIX_EXT        0x8E2E
#define GL_PROGRAM_MATRIX_STACK_DEPTH_EXT      0x8E2F
typedef void (APIENTRYP pfn_glMatrixLoadfEXT) (GLenum, const GLfloat*);
extern pfn_glMatrixLoadfEXT fp_glMatrixLoadfEXT;
typedef void (APIENTRYP pfn_glMatrixLoaddEXT) (GLenum, const GLdouble*);
extern pfn_glMatrixLoaddEXT fp_glMatrixLoaddEXT;
typedef void (APIENTRYP pfn_glMatrixMultfEXT) (GLenum, const GLfloat*);
extern pfn_glMatrixMultfEXT fp_glMatrixMultfEXT;
typedef void (APIENTRYP pfn_glMatrixMultdEXT) (GLenum, const GLdouble*);
extern pfn_glMatrixMultdEXT fp_glMatrixMultdEXT;
typedef void (APIENTRYP pfn_glMatrixLoadIdentityEXT) (GLenum);
extern pfn_glMatrixLoadIdentityEXT fp_glMatrixLoadIdentityEXT;
typedef void (APIENTRYP pfn_glMatrixRotatefEXT) (GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glMatrixRotatefEXT fp_glMatrixRotatefEXT;
typedef void (APIENTRYP pfn_glMatrixRotatedEXT) (GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glMatrixRotatedEXT fp_glMatrixRotatedEXT;
typedef void (APIENTRYP pfn_glMatrixScalefEXT) (GLenum, GLfloat, GLfloat, GLfloat);
extern pfn_glMatrixScalefEXT fp_glMatrixScalefEXT;
typedef void (APIENTRYP pfn_glMatrixScaledEXT) (GLenum, GLdouble, GLdouble, GLdouble);
extern pfn_glMatrixScaledEXT fp_glMatrixScaledEXT;
typedef void (APIENTRYP pfn_glMatrixTranslatefEXT) (GLenum, GLfloat, GLfloat, GLfloat);
extern pfn_glMatrixTranslatefEXT fp_glMatrixTranslatefEXT;
typedef void (APIENTRYP pfn_glMatrixTranslatedEXT) (GLenum, GLdouble, GLdouble, GLdouble);
extern pfn_glMatrixTranslatedEXT fp_glMatrixTranslatedEXT;
typedef void (APIENTRYP pfn_glMatrixFrustumEXT) (GLenum, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glMatrixFrustumEXT fp_glMatrixFrustumEXT;
typedef void (APIENTRYP pfn_glMatrixOrthoEXT) (GLenum, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glMatrixOrthoEXT fp_glMatrixOrthoEXT;
typedef void (APIENTRYP pfn_glMatrixPopEXT) (GLenum);
extern pfn_glMatrixPopEXT fp_glMatrixPopEXT;
typedef void (APIENTRYP pfn_glMatrixPushEXT) (GLenum);
extern pfn_glMatrixPushEXT fp_glMatrixPushEXT;
typedef void (APIENTRYP pfn_glClientAttribDefaultEXT) (GLbitfield);
extern pfn_glClientAttribDefaultEXT fp_glClientAttribDefaultEXT;
typedef void (APIENTRYP pfn_glPushClientAttribDefaultEXT) (GLbitfield);
extern pfn_glPushClientAttribDefaultEXT fp_glPushClientAttribDefaultEXT;
typedef void (APIENTRYP pfn_glTextureParameterfEXT) (GLuint, GLenum, GLenum, GLfloat);
extern pfn_glTextureParameterfEXT fp_glTextureParameterfEXT;
typedef void (APIENTRYP pfn_glTextureParameterfvEXT) (GLuint, GLenum, GLenum, const GLfloat*);
extern pfn_glTextureParameterfvEXT fp_glTextureParameterfvEXT;
typedef void (APIENTRYP pfn_glTextureParameteriEXT) (GLuint, GLenum, GLenum, GLint);
extern pfn_glTextureParameteriEXT fp_glTextureParameteriEXT;
typedef void (APIENTRYP pfn_glTextureParameterivEXT) (GLuint, GLenum, GLenum, const GLint*);
extern pfn_glTextureParameterivEXT fp_glTextureParameterivEXT;
typedef void (APIENTRYP pfn_glTextureImage1DEXT) (GLuint, GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const void*);
extern pfn_glTextureImage1DEXT fp_glTextureImage1DEXT;
typedef void (APIENTRYP pfn_glTextureImage2DEXT) (GLuint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*);
extern pfn_glTextureImage2DEXT fp_glTextureImage2DEXT;
typedef void (APIENTRYP pfn_glTextureSubImage1DEXT) (GLuint, GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const void*);
extern pfn_glTextureSubImage1DEXT fp_glTextureSubImage1DEXT;
typedef void (APIENTRYP pfn_glTextureSubImage2DEXT) (GLuint, GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*);
extern pfn_glTextureSubImage2DEXT fp_glTextureSubImage2DEXT;
typedef void (APIENTRYP pfn_glCopyTextureImage1DEXT) (GLuint, GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint);
extern pfn_glCopyTextureImage1DEXT fp_glCopyTextureImage1DEXT;
typedef void (APIENTRYP pfn_glCopyTextureImage2DEXT) (GLuint, GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
extern pfn_glCopyTextureImage2DEXT fp_glCopyTextureImage2DEXT;
typedef void (APIENTRYP pfn_glCopyTextureSubImage1DEXT) (GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei);
extern pfn_glCopyTextureSubImage1DEXT fp_glCopyTextureSubImage1DEXT;
typedef void (APIENTRYP pfn_glCopyTextureSubImage2DEXT) (GLuint, GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
extern pfn_glCopyTextureSubImage2DEXT fp_glCopyTextureSubImage2DEXT;
typedef void (APIENTRYP pfn_glGetTextureImageEXT) (GLuint, GLenum, GLint, GLenum, GLenum, void*);
extern pfn_glGetTextureImageEXT fp_glGetTextureImageEXT;
typedef void (APIENTRYP pfn_glGetTextureParameterfvEXT) (GLuint, GLenum, GLenum, GLfloat*);
extern pfn_glGetTextureParameterfvEXT fp_glGetTextureParameterfvEXT;
typedef void (APIENTRYP pfn_glGetTextureParameterivEXT) (GLuint, GLenum, GLenum, GLint*);
extern pfn_glGetTextureParameterivEXT fp_glGetTextureParameterivEXT;
typedef void (APIENTRYP pfn_glGetTextureLevelParameterfvEXT) (GLuint, GLenum, GLint, GLenum, GLfloat*);
extern pfn_glGetTextureLevelParameterfvEXT fp_glGetTextureLevelParameterfvEXT;
typedef void (APIENTRYP pfn_glGetTextureLevelParameterivEXT) (GLuint, GLenum, GLint, GLenum, GLint*);
extern pfn_glGetTextureLevelParameterivEXT fp_glGetTextureLevelParameterivEXT;
typedef void (APIENTRYP pfn_glTextureImage3DEXT) (GLuint, GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*);
extern pfn_glTextureImage3DEXT fp_glTextureImage3DEXT;
typedef void (APIENTRYP pfn_glTextureSubImage3DEXT) (GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*);
extern pfn_glTextureSubImage3DEXT fp_glTextureSubImage3DEXT;
typedef void (APIENTRYP pfn_glCopyTextureSubImage3DEXT) (GLuint, GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
extern pfn_glCopyTextureSubImage3DEXT fp_glCopyTextureSubImage3DEXT;
typedef void (APIENTRYP pfn_glBindMultiTextureEXT) (GLenum, GLenum, GLuint);
extern pfn_glBindMultiTextureEXT fp_glBindMultiTextureEXT;
typedef void (APIENTRYP pfn_glMultiTexCoordPointerEXT) (GLenum, GLint, GLenum, GLsizei, const void*);
extern pfn_glMultiTexCoordPointerEXT fp_glMultiTexCoordPointerEXT;
typedef void (APIENTRYP pfn_glMultiTexEnvfEXT) (GLenum, GLenum, GLenum, GLfloat);
extern pfn_glMultiTexEnvfEXT fp_glMultiTexEnvfEXT;
typedef void (APIENTRYP pfn_glMultiTexEnvfvEXT) (GLenum, GLenum, GLenum, const GLfloat*);
extern pfn_glMultiTexEnvfvEXT fp_glMultiTexEnvfvEXT;
typedef void (APIENTRYP pfn_glMultiTexEnviEXT) (GLenum, GLenum, GLenum, GLint);
extern pfn_glMultiTexEnviEXT fp_glMultiTexEnviEXT;
typedef void (APIENTRYP pfn_glMultiTexEnvivEXT) (GLenum, GLenum, GLenum, const GLint*);
extern pfn_glMultiTexEnvivEXT fp_glMultiTexEnvivEXT;
typedef void (APIENTRYP pfn_glMultiTexGendEXT) (GLenum, GLenum, GLenum, GLdouble);
extern pfn_glMultiTexGendEXT fp_glMultiTexGendEXT;
typedef void (APIENTRYP pfn_glMultiTexGendvEXT) (GLenum, GLenum, GLenum, const GLdouble*);
extern pfn_glMultiTexGendvEXT fp_glMultiTexGendvEXT;
typedef void (APIENTRYP pfn_glMultiTexGenfEXT) (GLenum, GLenum, GLenum, GLfloat);
extern pfn_glMultiTexGenfEXT fp_glMultiTexGenfEXT;
typedef void (APIENTRYP pfn_glMultiTexGenfvEXT) (GLenum, GLenum, GLenum, const GLfloat*);
extern pfn_glMultiTexGenfvEXT fp_glMultiTexGenfvEXT;
typedef void (APIENTRYP pfn_glMultiTexGeniEXT) (GLenum, GLenum, GLenum, GLint);
extern pfn_glMultiTexGeniEXT fp_glMultiTexGeniEXT;
typedef void (APIENTRYP pfn_glMultiTexGenivEXT) (GLenum, GLenum, GLenum, const GLint*);
extern pfn_glMultiTexGenivEXT fp_glMultiTexGenivEXT;
typedef void (APIENTRYP pfn_glGetMultiTexEnvfvEXT) (GLenum, GLenum, GLenum, GLfloat*);
extern pfn_glGetMultiTexEnvfvEXT fp_glGetMultiTexEnvfvEXT;
typedef void (APIENTRYP pfn_glGetMultiTexEnvivEXT) (GLenum, GLenum, GLenum, GLint*);
extern pfn_glGetMultiTexEnvivEXT fp_glGetMultiTexEnvivEXT;
typedef void (APIENTRYP pfn_glGetMultiTexGendvEXT) (GLenum, GLenum, GLenum, GLdouble*);
extern pfn_glGetMultiTexGendvEXT fp_glGetMultiTexGendvEXT;
typedef void (APIENTRYP pfn_glGetMultiTexGenfvEXT) (GLenum, GLenum, GLenum, GLfloat*);
extern pfn_glGetMultiTexGenfvEXT fp_glGetMultiTexGenfvEXT;
typedef void (APIENTRYP pfn_glGetMultiTexGenivEXT) (GLenum, GLenum, GLenum, GLint*);
extern pfn_glGetMultiTexGenivEXT fp_glGetMultiTexGenivEXT;
typedef void (APIENTRYP pfn_glMultiTexParameteriEXT) (GLenum, GLenum, GLenum, GLint);
extern pfn_glMultiTexParameteriEXT fp_glMultiTexParameteriEXT;
typedef void (APIENTRYP pfn_glMultiTexParameterivEXT) (GLenum, GLenum, GLenum, const GLint*);
extern pfn_glMultiTexParameterivEXT fp_glMultiTexParameterivEXT;
typedef void (APIENTRYP pfn_glMultiTexParameterfEXT) (GLenum, GLenum, GLenum, GLfloat);
extern pfn_glMultiTexParameterfEXT fp_glMultiTexParameterfEXT;
typedef void (APIENTRYP pfn_glMultiTexParameterfvEXT) (GLenum, GLenum, GLenum, const GLfloat*);
extern pfn_glMultiTexParameterfvEXT fp_glMultiTexParameterfvEXT;
typedef void (APIENTRYP pfn_glMultiTexImage1DEXT) (GLenum, GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const void*);
extern pfn_glMultiTexImage1DEXT fp_glMultiTexImage1DEXT;
typedef void (APIENTRYP pfn_glMultiTexImage2DEXT) (GLenum, GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*);
extern pfn_glMultiTexImage2DEXT fp_glMultiTexImage2DEXT;
typedef void (APIENTRYP pfn_glMultiTexSubImage1DEXT) (GLenum, GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const void*);
extern pfn_glMultiTexSubImage1DEXT fp_glMultiTexSubImage1DEXT;
typedef void (APIENTRYP pfn_glMultiTexSubImage2DEXT) (GLenum, GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*);
extern pfn_glMultiTexSubImage2DEXT fp_glMultiTexSubImage2DEXT;
typedef void (APIENTRYP pfn_glCopyMultiTexImage1DEXT) (GLenum, GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint);
extern pfn_glCopyMultiTexImage1DEXT fp_glCopyMultiTexImage1DEXT;
typedef void (APIENTRYP pfn_glCopyMultiTexImage2DEXT) (GLenum, GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
extern pfn_glCopyMultiTexImage2DEXT fp_glCopyMultiTexImage2DEXT;
typedef void (APIENTRYP pfn_glCopyMultiTexSubImage1DEXT) (GLenum, GLenum, GLint, GLint, GLint, GLint, GLsizei);
extern pfn_glCopyMultiTexSubImage1DEXT fp_glCopyMultiTexSubImage1DEXT;
typedef void (APIENTRYP pfn_glCopyMultiTexSubImage2DEXT) (GLenum, GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
extern pfn_glCopyMultiTexSubImage2DEXT fp_glCopyMultiTexSubImage2DEXT;
typedef void (APIENTRYP pfn_glGetMultiTexImageEXT) (GLenum, GLenum, GLint, GLenum, GLenum, void*);
extern pfn_glGetMultiTexImageEXT fp_glGetMultiTexImageEXT;
typedef void (APIENTRYP pfn_glGetMultiTexParameterfvEXT) (GLenum, GLenum, GLenum, GLfloat*);
extern pfn_glGetMultiTexParameterfvEXT fp_glGetMultiTexParameterfvEXT;
typedef void (APIENTRYP pfn_glGetMultiTexParameterivEXT) (GLenum, GLenum, GLenum, GLint*);
extern pfn_glGetMultiTexParameterivEXT fp_glGetMultiTexParameterivEXT;
typedef void (APIENTRYP pfn_glGetMultiTexLevelParameterfvEXT) (GLenum, GLenum, GLint, GLenum, GLfloat*);
extern pfn_glGetMultiTexLevelParameterfvEXT fp_glGetMultiTexLevelParameterfvEXT;
typedef void (APIENTRYP pfn_glGetMultiTexLevelParameterivEXT) (GLenum, GLenum, GLint, GLenum, GLint*);
extern pfn_glGetMultiTexLevelParameterivEXT fp_glGetMultiTexLevelParameterivEXT;
typedef void (APIENTRYP pfn_glMultiTexImage3DEXT) (GLenum, GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*);
extern pfn_glMultiTexImage3DEXT fp_glMultiTexImage3DEXT;
typedef void (APIENTRYP pfn_glMultiTexSubImage3DEXT) (GLenum, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*);
extern pfn_glMultiTexSubImage3DEXT fp_glMultiTexSubImage3DEXT;
typedef void (APIENTRYP pfn_glCopyMultiTexSubImage3DEXT) (GLenum, GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
extern pfn_glCopyMultiTexSubImage3DEXT fp_glCopyMultiTexSubImage3DEXT;
typedef void (APIENTRYP pfn_glEnableClientStateIndexedEXT) (GLenum, GLuint);
extern pfn_glEnableClientStateIndexedEXT fp_glEnableClientStateIndexedEXT;
typedef void (APIENTRYP pfn_glDisableClientStateIndexedEXT) (GLenum, GLuint);
extern pfn_glDisableClientStateIndexedEXT fp_glDisableClientStateIndexedEXT;
typedef void (APIENTRYP pfn_glGetFloatIndexedvEXT) (GLenum, GLuint, GLfloat*);
extern pfn_glGetFloatIndexedvEXT fp_glGetFloatIndexedvEXT;
typedef void (APIENTRYP pfn_glGetDoubleIndexedvEXT) (GLenum, GLuint, GLdouble*);
extern pfn_glGetDoubleIndexedvEXT fp_glGetDoubleIndexedvEXT;
typedef void (APIENTRYP pfn_glGetPointerIndexedvEXT) (GLenum, GLuint, void**);
extern pfn_glGetPointerIndexedvEXT fp_glGetPointerIndexedvEXT;
typedef void (APIENTRYP pfn_glEnableIndexedEXT) (GLenum, GLuint);
extern pfn_glEnableIndexedEXT fp_glEnableIndexedEXT;
typedef void (APIENTRYP pfn_glDisableIndexedEXT) (GLenum, GLuint);
extern pfn_glDisableIndexedEXT fp_glDisableIndexedEXT;
typedef GLboolean (APIENTRYP pfn_glIsEnabledIndexedEXT) (GLenum, GLuint);
extern pfn_glIsEnabledIndexedEXT fp_glIsEnabledIndexedEXT;
typedef void (APIENTRYP pfn_glGetIntegerIndexedvEXT) (GLenum, GLuint, GLint*);
extern pfn_glGetIntegerIndexedvEXT fp_glGetIntegerIndexedvEXT;
typedef void (APIENTRYP pfn_glGetBooleanIndexedvEXT) (GLenum, GLuint, GLboolean*);
extern pfn_glGetBooleanIndexedvEXT fp_glGetBooleanIndexedvEXT;
typedef void (APIENTRYP pfn_glCompressedTextureImage3DEXT) (GLuint, GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const void*);
extern pfn_glCompressedTextureImage3DEXT fp_glCompressedTextureImage3DEXT;
typedef void (APIENTRYP pfn_glCompressedTextureImage2DEXT) (GLuint, GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void*);
extern pfn_glCompressedTextureImage2DEXT fp_glCompressedTextureImage2DEXT;
typedef void (APIENTRYP pfn_glCompressedTextureImage1DEXT) (GLuint, GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const void*);
extern pfn_glCompressedTextureImage1DEXT fp_glCompressedTextureImage1DEXT;
typedef void (APIENTRYP pfn_glCompressedTextureSubImage3DEXT) (GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void*);
extern pfn_glCompressedTextureSubImage3DEXT fp_glCompressedTextureSubImage3DEXT;
typedef void (APIENTRYP pfn_glCompressedTextureSubImage2DEXT) (GLuint, GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void*);
extern pfn_glCompressedTextureSubImage2DEXT fp_glCompressedTextureSubImage2DEXT;
typedef void (APIENTRYP pfn_glCompressedTextureSubImage1DEXT) (GLuint, GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const void*);
extern pfn_glCompressedTextureSubImage1DEXT fp_glCompressedTextureSubImage1DEXT;
typedef void (APIENTRYP pfn_glGetCompressedTextureImageEXT) (GLuint, GLenum, GLint, void*);
extern pfn_glGetCompressedTextureImageEXT fp_glGetCompressedTextureImageEXT;
typedef void (APIENTRYP pfn_glCompressedMultiTexImage3DEXT) (GLenum, GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const void*);
extern pfn_glCompressedMultiTexImage3DEXT fp_glCompressedMultiTexImage3DEXT;
typedef void (APIENTRYP pfn_glCompressedMultiTexImage2DEXT) (GLenum, GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void*);
extern pfn_glCompressedMultiTexImage2DEXT fp_glCompressedMultiTexImage2DEXT;
typedef void (APIENTRYP pfn_glCompressedMultiTexImage1DEXT) (GLenum, GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const void*);
extern pfn_glCompressedMultiTexImage1DEXT fp_glCompressedMultiTexImage1DEXT;
typedef void (APIENTRYP pfn_glCompressedMultiTexSubImage3DEXT) (GLenum, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void*);
extern pfn_glCompressedMultiTexSubImage3DEXT fp_glCompressedMultiTexSubImage3DEXT;
typedef void (APIENTRYP pfn_glCompressedMultiTexSubImage2DEXT) (GLenum, GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void*);
extern pfn_glCompressedMultiTexSubImage2DEXT fp_glCompressedMultiTexSubImage2DEXT;
typedef void (APIENTRYP pfn_glCompressedMultiTexSubImage1DEXT) (GLenum, GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const void*);
extern pfn_glCompressedMultiTexSubImage1DEXT fp_glCompressedMultiTexSubImage1DEXT;
typedef void (APIENTRYP pfn_glGetCompressedMultiTexImageEXT) (GLenum, GLenum, GLint, void*);
extern pfn_glGetCompressedMultiTexImageEXT fp_glGetCompressedMultiTexImageEXT;
typedef void (APIENTRYP pfn_glMatrixLoadTransposefEXT) (GLenum, const GLfloat*);
extern pfn_glMatrixLoadTransposefEXT fp_glMatrixLoadTransposefEXT;
typedef void (APIENTRYP pfn_glMatrixLoadTransposedEXT) (GLenum, const GLdouble*);
extern pfn_glMatrixLoadTransposedEXT fp_glMatrixLoadTransposedEXT;
typedef void (APIENTRYP pfn_glMatrixMultTransposefEXT) (GLenum, const GLfloat*);
extern pfn_glMatrixMultTransposefEXT fp_glMatrixMultTransposefEXT;
typedef void (APIENTRYP pfn_glMatrixMultTransposedEXT) (GLenum, const GLdouble*);
extern pfn_glMatrixMultTransposedEXT fp_glMatrixMultTransposedEXT;
typedef void (APIENTRYP pfn_glNamedBufferDataEXT) (GLuint, GLsizeiptr, const void*, GLenum);
extern pfn_glNamedBufferDataEXT fp_glNamedBufferDataEXT;
typedef void (APIENTRYP pfn_glNamedBufferSubDataEXT) (GLuint, GLintptr, GLsizeiptr, const void*);
extern pfn_glNamedBufferSubDataEXT fp_glNamedBufferSubDataEXT;
typedef void* (APIENTRYP pfn_glMapNamedBufferEXT) (GLuint, GLenum);
extern pfn_glMapNamedBufferEXT fp_glMapNamedBufferEXT;
typedef GLboolean (APIENTRYP pfn_glUnmapNamedBufferEXT) (GLuint);
extern pfn_glUnmapNamedBufferEXT fp_glUnmapNamedBufferEXT;
typedef void (APIENTRYP pfn_glGetNamedBufferParameterivEXT) (GLuint, GLenum, GLint*);
extern pfn_glGetNamedBufferParameterivEXT fp_glGetNamedBufferParameterivEXT;
typedef void (APIENTRYP pfn_glGetNamedBufferPointervEXT) (GLuint, GLenum, void**);
extern pfn_glGetNamedBufferPointervEXT fp_glGetNamedBufferPointervEXT;
typedef void (APIENTRYP pfn_glGetNamedBufferSubDataEXT) (GLuint, GLintptr, GLsizeiptr, void*);
extern pfn_glGetNamedBufferSubDataEXT fp_glGetNamedBufferSubDataEXT;
typedef void (APIENTRYP pfn_glProgramUniform1fEXT) (GLuint, GLint, GLfloat);
extern pfn_glProgramUniform1fEXT fp_glProgramUniform1fEXT;
typedef void (APIENTRYP pfn_glProgramUniform2fEXT) (GLuint, GLint, GLfloat, GLfloat);
extern pfn_glProgramUniform2fEXT fp_glProgramUniform2fEXT;
typedef void (APIENTRYP pfn_glProgramUniform3fEXT) (GLuint, GLint, GLfloat, GLfloat, GLfloat);
extern pfn_glProgramUniform3fEXT fp_glProgramUniform3fEXT;
typedef void (APIENTRYP pfn_glProgramUniform4fEXT) (GLuint, GLint, GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glProgramUniform4fEXT fp_glProgramUniform4fEXT;
typedef void (APIENTRYP pfn_glProgramUniform1iEXT) (GLuint, GLint, GLint);
extern pfn_glProgramUniform1iEXT fp_glProgramUniform1iEXT;
typedef void (APIENTRYP pfn_glProgramUniform2iEXT) (GLuint, GLint, GLint, GLint);
extern pfn_glProgramUniform2iEXT fp_glProgramUniform2iEXT;
typedef void (APIENTRYP pfn_glProgramUniform3iEXT) (GLuint, GLint, GLint, GLint, GLint);
extern pfn_glProgramUniform3iEXT fp_glProgramUniform3iEXT;
typedef void (APIENTRYP pfn_glProgramUniform4iEXT) (GLuint, GLint, GLint, GLint, GLint, GLint);
extern pfn_glProgramUniform4iEXT fp_glProgramUniform4iEXT;
typedef void (APIENTRYP pfn_glProgramUniform1fvEXT) (GLuint, GLint, GLsizei, const GLfloat*);
extern pfn_glProgramUniform1fvEXT fp_glProgramUniform1fvEXT;
typedef void (APIENTRYP pfn_glProgramUniform2fvEXT) (GLuint, GLint, GLsizei, const GLfloat*);
extern pfn_glProgramUniform2fvEXT fp_glProgramUniform2fvEXT;
typedef void (APIENTRYP pfn_glProgramUniform3fvEXT) (GLuint, GLint, GLsizei, const GLfloat*);
extern pfn_glProgramUniform3fvEXT fp_glProgramUniform3fvEXT;
typedef void (APIENTRYP pfn_glProgramUniform4fvEXT) (GLuint, GLint, GLsizei, const GLfloat*);
extern pfn_glProgramUniform4fvEXT fp_glProgramUniform4fvEXT;
typedef void (APIENTRYP pfn_glProgramUniform1ivEXT) (GLuint, GLint, GLsizei, const GLint*);
extern pfn_glProgramUniform1ivEXT fp_glProgramUniform1ivEXT;
typedef void (APIENTRYP pfn_glProgramUniform2ivEXT) (GLuint, GLint, GLsizei, const GLint*);
extern pfn_glProgramUniform2ivEXT fp_glProgramUniform2ivEXT;
typedef void (APIENTRYP pfn_glProgramUniform3ivEXT) (GLuint, GLint, GLsizei, const GLint*);
extern pfn_glProgramUniform3ivEXT fp_glProgramUniform3ivEXT;
typedef void (APIENTRYP pfn_glProgramUniform4ivEXT) (GLuint, GLint, GLsizei, const GLint*);
extern pfn_glProgramUniform4ivEXT fp_glProgramUniform4ivEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix2fvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix2fvEXT fp_glProgramUniformMatrix2fvEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix3fvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix3fvEXT fp_glProgramUniformMatrix3fvEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix4fvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix4fvEXT fp_glProgramUniformMatrix4fvEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix2x3fvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix2x3fvEXT fp_glProgramUniformMatrix2x3fvEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix3x2fvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix3x2fvEXT fp_glProgramUniformMatrix3x2fvEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix2x4fvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix2x4fvEXT fp_glProgramUniformMatrix2x4fvEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix4x2fvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix4x2fvEXT fp_glProgramUniformMatrix4x2fvEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix3x4fvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix3x4fvEXT fp_glProgramUniformMatrix3x4fvEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix4x3fvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glProgramUniformMatrix4x3fvEXT fp_glProgramUniformMatrix4x3fvEXT;
typedef void (APIENTRYP pfn_glTextureBufferEXT) (GLuint, GLenum, GLenum, GLuint);
extern pfn_glTextureBufferEXT fp_glTextureBufferEXT;
typedef void (APIENTRYP pfn_glMultiTexBufferEXT) (GLenum, GLenum, GLenum, GLuint);
extern pfn_glMultiTexBufferEXT fp_glMultiTexBufferEXT;
typedef void (APIENTRYP pfn_glTextureParameterIivEXT) (GLuint, GLenum, GLenum, const GLint*);
extern pfn_glTextureParameterIivEXT fp_glTextureParameterIivEXT;
typedef void (APIENTRYP pfn_glTextureParameterIuivEXT) (GLuint, GLenum, GLenum, const GLuint*);
extern pfn_glTextureParameterIuivEXT fp_glTextureParameterIuivEXT;
typedef void (APIENTRYP pfn_glGetTextureParameterIivEXT) (GLuint, GLenum, GLenum, GLint*);
extern pfn_glGetTextureParameterIivEXT fp_glGetTextureParameterIivEXT;
typedef void (APIENTRYP pfn_glGetTextureParameterIuivEXT) (GLuint, GLenum, GLenum, GLuint*);
extern pfn_glGetTextureParameterIuivEXT fp_glGetTextureParameterIuivEXT;
typedef void (APIENTRYP pfn_glMultiTexParameterIivEXT) (GLenum, GLenum, GLenum, const GLint*);
extern pfn_glMultiTexParameterIivEXT fp_glMultiTexParameterIivEXT;
typedef void (APIENTRYP pfn_glMultiTexParameterIuivEXT) (GLenum, GLenum, GLenum, const GLuint*);
extern pfn_glMultiTexParameterIuivEXT fp_glMultiTexParameterIuivEXT;
typedef void (APIENTRYP pfn_glGetMultiTexParameterIivEXT) (GLenum, GLenum, GLenum, GLint*);
extern pfn_glGetMultiTexParameterIivEXT fp_glGetMultiTexParameterIivEXT;
typedef void (APIENTRYP pfn_glGetMultiTexParameterIuivEXT) (GLenum, GLenum, GLenum, GLuint*);
extern pfn_glGetMultiTexParameterIuivEXT fp_glGetMultiTexParameterIuivEXT;
typedef void (APIENTRYP pfn_glProgramUniform1uiEXT) (GLuint, GLint, GLuint);
extern pfn_glProgramUniform1uiEXT fp_glProgramUniform1uiEXT;
typedef void (APIENTRYP pfn_glProgramUniform2uiEXT) (GLuint, GLint, GLuint, GLuint);
extern pfn_glProgramUniform2uiEXT fp_glProgramUniform2uiEXT;
typedef void (APIENTRYP pfn_glProgramUniform3uiEXT) (GLuint, GLint, GLuint, GLuint, GLuint);
extern pfn_glProgramUniform3uiEXT fp_glProgramUniform3uiEXT;
typedef void (APIENTRYP pfn_glProgramUniform4uiEXT) (GLuint, GLint, GLuint, GLuint, GLuint, GLuint);
extern pfn_glProgramUniform4uiEXT fp_glProgramUniform4uiEXT;
typedef void (APIENTRYP pfn_glProgramUniform1uivEXT) (GLuint, GLint, GLsizei, const GLuint*);
extern pfn_glProgramUniform1uivEXT fp_glProgramUniform1uivEXT;
typedef void (APIENTRYP pfn_glProgramUniform2uivEXT) (GLuint, GLint, GLsizei, const GLuint*);
extern pfn_glProgramUniform2uivEXT fp_glProgramUniform2uivEXT;
typedef void (APIENTRYP pfn_glProgramUniform3uivEXT) (GLuint, GLint, GLsizei, const GLuint*);
extern pfn_glProgramUniform3uivEXT fp_glProgramUniform3uivEXT;
typedef void (APIENTRYP pfn_glProgramUniform4uivEXT) (GLuint, GLint, GLsizei, const GLuint*);
extern pfn_glProgramUniform4uivEXT fp_glProgramUniform4uivEXT;
typedef void (APIENTRYP pfn_glNamedProgramLocalParameters4fvEXT) (GLuint, GLenum, GLuint, GLsizei, const GLfloat*);
extern pfn_glNamedProgramLocalParameters4fvEXT fp_glNamedProgramLocalParameters4fvEXT;
typedef void (APIENTRYP pfn_glNamedProgramLocalParameterI4iEXT) (GLuint, GLenum, GLuint, GLint, GLint, GLint, GLint);
extern pfn_glNamedProgramLocalParameterI4iEXT fp_glNamedProgramLocalParameterI4iEXT;
typedef void (APIENTRYP pfn_glNamedProgramLocalParameterI4ivEXT) (GLuint, GLenum, GLuint, const GLint*);
extern pfn_glNamedProgramLocalParameterI4ivEXT fp_glNamedProgramLocalParameterI4ivEXT;
typedef void (APIENTRYP pfn_glNamedProgramLocalParametersI4ivEXT) (GLuint, GLenum, GLuint, GLsizei, const GLint*);
extern pfn_glNamedProgramLocalParametersI4ivEXT fp_glNamedProgramLocalParametersI4ivEXT;
typedef void (APIENTRYP pfn_glNamedProgramLocalParameterI4uiEXT) (GLuint, GLenum, GLuint, GLuint, GLuint, GLuint, GLuint);
extern pfn_glNamedProgramLocalParameterI4uiEXT fp_glNamedProgramLocalParameterI4uiEXT;
typedef void (APIENTRYP pfn_glNamedProgramLocalParameterI4uivEXT) (GLuint, GLenum, GLuint, const GLuint*);
extern pfn_glNamedProgramLocalParameterI4uivEXT fp_glNamedProgramLocalParameterI4uivEXT;
typedef void (APIENTRYP pfn_glNamedProgramLocalParametersI4uivEXT) (GLuint, GLenum, GLuint, GLsizei, const GLuint*);
extern pfn_glNamedProgramLocalParametersI4uivEXT fp_glNamedProgramLocalParametersI4uivEXT;
typedef void (APIENTRYP pfn_glGetNamedProgramLocalParameterIivEXT) (GLuint, GLenum, GLuint, GLint*);
extern pfn_glGetNamedProgramLocalParameterIivEXT fp_glGetNamedProgramLocalParameterIivEXT;
typedef void (APIENTRYP pfn_glGetNamedProgramLocalParameterIuivEXT) (GLuint, GLenum, GLuint, GLuint*);
extern pfn_glGetNamedProgramLocalParameterIuivEXT fp_glGetNamedProgramLocalParameterIuivEXT;
typedef void (APIENTRYP pfn_glEnableClientStateiEXT) (GLenum, GLuint);
extern pfn_glEnableClientStateiEXT fp_glEnableClientStateiEXT;
typedef void (APIENTRYP pfn_glDisableClientStateiEXT) (GLenum, GLuint);
extern pfn_glDisableClientStateiEXT fp_glDisableClientStateiEXT;
typedef void (APIENTRYP pfn_glGetFloati_vEXT) (GLenum, GLuint, GLfloat*);
extern pfn_glGetFloati_vEXT fp_glGetFloati_vEXT;
typedef void (APIENTRYP pfn_glGetDoublei_vEXT) (GLenum, GLuint, GLdouble*);
extern pfn_glGetDoublei_vEXT fp_glGetDoublei_vEXT;
typedef void (APIENTRYP pfn_glGetPointeri_vEXT) (GLenum, GLuint, void**);
extern pfn_glGetPointeri_vEXT fp_glGetPointeri_vEXT;
typedef void (APIENTRYP pfn_glNamedProgramStringEXT) (GLuint, GLenum, GLenum, GLsizei, const void*);
extern pfn_glNamedProgramStringEXT fp_glNamedProgramStringEXT;
typedef void (APIENTRYP pfn_glNamedProgramLocalParameter4dEXT) (GLuint, GLenum, GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glNamedProgramLocalParameter4dEXT fp_glNamedProgramLocalParameter4dEXT;
typedef void (APIENTRYP pfn_glNamedProgramLocalParameter4dvEXT) (GLuint, GLenum, GLuint, const GLdouble*);
extern pfn_glNamedProgramLocalParameter4dvEXT fp_glNamedProgramLocalParameter4dvEXT;
typedef void (APIENTRYP pfn_glNamedProgramLocalParameter4fEXT) (GLuint, GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glNamedProgramLocalParameter4fEXT fp_glNamedProgramLocalParameter4fEXT;
typedef void (APIENTRYP pfn_glNamedProgramLocalParameter4fvEXT) (GLuint, GLenum, GLuint, const GLfloat*);
extern pfn_glNamedProgramLocalParameter4fvEXT fp_glNamedProgramLocalParameter4fvEXT;
typedef void (APIENTRYP pfn_glGetNamedProgramLocalParameterdvEXT) (GLuint, GLenum, GLuint, GLdouble*);
extern pfn_glGetNamedProgramLocalParameterdvEXT fp_glGetNamedProgramLocalParameterdvEXT;
typedef void (APIENTRYP pfn_glGetNamedProgramLocalParameterfvEXT) (GLuint, GLenum, GLuint, GLfloat*);
extern pfn_glGetNamedProgramLocalParameterfvEXT fp_glGetNamedProgramLocalParameterfvEXT;
typedef void (APIENTRYP pfn_glGetNamedProgramivEXT) (GLuint, GLenum, GLenum, GLint*);
extern pfn_glGetNamedProgramivEXT fp_glGetNamedProgramivEXT;
typedef void (APIENTRYP pfn_glGetNamedProgramStringEXT) (GLuint, GLenum, GLenum, void*);
extern pfn_glGetNamedProgramStringEXT fp_glGetNamedProgramStringEXT;
typedef void (APIENTRYP pfn_glNamedRenderbufferStorageEXT) (GLuint, GLenum, GLsizei, GLsizei);
extern pfn_glNamedRenderbufferStorageEXT fp_glNamedRenderbufferStorageEXT;
typedef void (APIENTRYP pfn_glGetNamedRenderbufferParameterivEXT) (GLuint, GLenum, GLint*);
extern pfn_glGetNamedRenderbufferParameterivEXT fp_glGetNamedRenderbufferParameterivEXT;
typedef void (APIENTRYP pfn_glNamedRenderbufferStorageMultisampleEXT) (GLuint, GLsizei, GLenum, GLsizei, GLsizei);
extern pfn_glNamedRenderbufferStorageMultisampleEXT fp_glNamedRenderbufferStorageMultisampleEXT;
typedef void (APIENTRYP pfn_glNamedRenderbufferStorageMultisampleCoverageEXT) (GLuint, GLsizei, GLsizei, GLenum, GLsizei, GLsizei);
extern pfn_glNamedRenderbufferStorageMultisampleCoverageEXT fp_glNamedRenderbufferStorageMultisampleCoverageEXT;
typedef GLenum (APIENTRYP pfn_glCheckNamedFramebufferStatusEXT) (GLuint, GLenum);
extern pfn_glCheckNamedFramebufferStatusEXT fp_glCheckNamedFramebufferStatusEXT;
typedef void (APIENTRYP pfn_glNamedFramebufferTexture1DEXT) (GLuint, GLenum, GLenum, GLuint, GLint);
extern pfn_glNamedFramebufferTexture1DEXT fp_glNamedFramebufferTexture1DEXT;
typedef void (APIENTRYP pfn_glNamedFramebufferTexture2DEXT) (GLuint, GLenum, GLenum, GLuint, GLint);
extern pfn_glNamedFramebufferTexture2DEXT fp_glNamedFramebufferTexture2DEXT;
typedef void (APIENTRYP pfn_glNamedFramebufferTexture3DEXT) (GLuint, GLenum, GLenum, GLuint, GLint, GLint);
extern pfn_glNamedFramebufferTexture3DEXT fp_glNamedFramebufferTexture3DEXT;
typedef void (APIENTRYP pfn_glNamedFramebufferRenderbufferEXT) (GLuint, GLenum, GLenum, GLuint);
extern pfn_glNamedFramebufferRenderbufferEXT fp_glNamedFramebufferRenderbufferEXT;
typedef void (APIENTRYP pfn_glGetNamedFramebufferAttachmentParameterivEXT) (GLuint, GLenum, GLenum, GLint*);
extern pfn_glGetNamedFramebufferAttachmentParameterivEXT fp_glGetNamedFramebufferAttachmentParameterivEXT;
typedef void (APIENTRYP pfn_glGenerateTextureMipmapEXT) (GLuint, GLenum);
extern pfn_glGenerateTextureMipmapEXT fp_glGenerateTextureMipmapEXT;
typedef void (APIENTRYP pfn_glGenerateMultiTexMipmapEXT) (GLenum, GLenum);
extern pfn_glGenerateMultiTexMipmapEXT fp_glGenerateMultiTexMipmapEXT;
typedef void (APIENTRYP pfn_glFramebufferDrawBufferEXT) (GLuint, GLenum);
extern pfn_glFramebufferDrawBufferEXT fp_glFramebufferDrawBufferEXT;
typedef void (APIENTRYP pfn_glFramebufferDrawBuffersEXT) (GLuint, GLsizei, const GLenum*);
extern pfn_glFramebufferDrawBuffersEXT fp_glFramebufferDrawBuffersEXT;
typedef void (APIENTRYP pfn_glFramebufferReadBufferEXT) (GLuint, GLenum);
extern pfn_glFramebufferReadBufferEXT fp_glFramebufferReadBufferEXT;
typedef void (APIENTRYP pfn_glGetFramebufferParameterivEXT) (GLuint, GLenum, GLint*);
extern pfn_glGetFramebufferParameterivEXT fp_glGetFramebufferParameterivEXT;
typedef void (APIENTRYP pfn_glNamedCopyBufferSubDataEXT) (GLuint, GLuint, GLintptr, GLintptr, GLsizeiptr);
extern pfn_glNamedCopyBufferSubDataEXT fp_glNamedCopyBufferSubDataEXT;
typedef void (APIENTRYP pfn_glNamedFramebufferTextureEXT) (GLuint, GLenum, GLuint, GLint);
extern pfn_glNamedFramebufferTextureEXT fp_glNamedFramebufferTextureEXT;
typedef void (APIENTRYP pfn_glNamedFramebufferTextureLayerEXT) (GLuint, GLenum, GLuint, GLint, GLint);
extern pfn_glNamedFramebufferTextureLayerEXT fp_glNamedFramebufferTextureLayerEXT;
typedef void (APIENTRYP pfn_glNamedFramebufferTextureFaceEXT) (GLuint, GLenum, GLuint, GLint, GLenum);
extern pfn_glNamedFramebufferTextureFaceEXT fp_glNamedFramebufferTextureFaceEXT;
typedef void (APIENTRYP pfn_glTextureRenderbufferEXT) (GLuint, GLenum, GLuint);
extern pfn_glTextureRenderbufferEXT fp_glTextureRenderbufferEXT;
typedef void (APIENTRYP pfn_glMultiTexRenderbufferEXT) (GLenum, GLenum, GLuint);
extern pfn_glMultiTexRenderbufferEXT fp_glMultiTexRenderbufferEXT;
typedef void (APIENTRYP pfn_glVertexArrayVertexOffsetEXT) (GLuint, GLuint, GLint, GLenum, GLsizei, GLintptr);
extern pfn_glVertexArrayVertexOffsetEXT fp_glVertexArrayVertexOffsetEXT;
typedef void (APIENTRYP pfn_glVertexArrayColorOffsetEXT) (GLuint, GLuint, GLint, GLenum, GLsizei, GLintptr);
extern pfn_glVertexArrayColorOffsetEXT fp_glVertexArrayColorOffsetEXT;
typedef void (APIENTRYP pfn_glVertexArrayEdgeFlagOffsetEXT) (GLuint, GLuint, GLsizei, GLintptr);
extern pfn_glVertexArrayEdgeFlagOffsetEXT fp_glVertexArrayEdgeFlagOffsetEXT;
typedef void (APIENTRYP pfn_glVertexArrayIndexOffsetEXT) (GLuint, GLuint, GLenum, GLsizei, GLintptr);
extern pfn_glVertexArrayIndexOffsetEXT fp_glVertexArrayIndexOffsetEXT;
typedef void (APIENTRYP pfn_glVertexArrayNormalOffsetEXT) (GLuint, GLuint, GLenum, GLsizei, GLintptr);
extern pfn_glVertexArrayNormalOffsetEXT fp_glVertexArrayNormalOffsetEXT;
typedef void (APIENTRYP pfn_glVertexArrayTexCoordOffsetEXT) (GLuint, GLuint, GLint, GLenum, GLsizei, GLintptr);
extern pfn_glVertexArrayTexCoordOffsetEXT fp_glVertexArrayTexCoordOffsetEXT;
typedef void (APIENTRYP pfn_glVertexArrayMultiTexCoordOffsetEXT) (GLuint, GLuint, GLenum, GLint, GLenum, GLsizei, GLintptr);
extern pfn_glVertexArrayMultiTexCoordOffsetEXT fp_glVertexArrayMultiTexCoordOffsetEXT;
typedef void (APIENTRYP pfn_glVertexArrayFogCoordOffsetEXT) (GLuint, GLuint, GLenum, GLsizei, GLintptr);
extern pfn_glVertexArrayFogCoordOffsetEXT fp_glVertexArrayFogCoordOffsetEXT;
typedef void (APIENTRYP pfn_glVertexArraySecondaryColorOffsetEXT) (GLuint, GLuint, GLint, GLenum, GLsizei, GLintptr);
extern pfn_glVertexArraySecondaryColorOffsetEXT fp_glVertexArraySecondaryColorOffsetEXT;
typedef void (APIENTRYP pfn_glVertexArrayVertexAttribOffsetEXT) (GLuint, GLuint, GLuint, GLint, GLenum, GLboolean, GLsizei, GLintptr);
extern pfn_glVertexArrayVertexAttribOffsetEXT fp_glVertexArrayVertexAttribOffsetEXT;
typedef void (APIENTRYP pfn_glVertexArrayVertexAttribIOffsetEXT) (GLuint, GLuint, GLuint, GLint, GLenum, GLsizei, GLintptr);
extern pfn_glVertexArrayVertexAttribIOffsetEXT fp_glVertexArrayVertexAttribIOffsetEXT;
typedef void (APIENTRYP pfn_glEnableVertexArrayEXT) (GLuint, GLenum);
extern pfn_glEnableVertexArrayEXT fp_glEnableVertexArrayEXT;
typedef void (APIENTRYP pfn_glDisableVertexArrayEXT) (GLuint, GLenum);
extern pfn_glDisableVertexArrayEXT fp_glDisableVertexArrayEXT;
typedef void (APIENTRYP pfn_glEnableVertexArrayAttribEXT) (GLuint, GLuint);
extern pfn_glEnableVertexArrayAttribEXT fp_glEnableVertexArrayAttribEXT;
typedef void (APIENTRYP pfn_glDisableVertexArrayAttribEXT) (GLuint, GLuint);
extern pfn_glDisableVertexArrayAttribEXT fp_glDisableVertexArrayAttribEXT;
typedef void (APIENTRYP pfn_glGetVertexArrayIntegervEXT) (GLuint, GLenum, GLint*);
extern pfn_glGetVertexArrayIntegervEXT fp_glGetVertexArrayIntegervEXT;
typedef void (APIENTRYP pfn_glGetVertexArrayPointervEXT) (GLuint, GLenum, void**);
extern pfn_glGetVertexArrayPointervEXT fp_glGetVertexArrayPointervEXT;
typedef void (APIENTRYP pfn_glGetVertexArrayIntegeri_vEXT) (GLuint, GLuint, GLenum, GLint*);
extern pfn_glGetVertexArrayIntegeri_vEXT fp_glGetVertexArrayIntegeri_vEXT;
typedef void (APIENTRYP pfn_glGetVertexArrayPointeri_vEXT) (GLuint, GLuint, GLenum, void**);
extern pfn_glGetVertexArrayPointeri_vEXT fp_glGetVertexArrayPointeri_vEXT;
typedef void* (APIENTRYP pfn_glMapNamedBufferRangeEXT) (GLuint, GLintptr, GLsizeiptr, GLbitfield);
extern pfn_glMapNamedBufferRangeEXT fp_glMapNamedBufferRangeEXT;
typedef void (APIENTRYP pfn_glFlushMappedNamedBufferRangeEXT) (GLuint, GLintptr, GLsizeiptr);
extern pfn_glFlushMappedNamedBufferRangeEXT fp_glFlushMappedNamedBufferRangeEXT;
typedef void (APIENTRYP pfn_glNamedBufferStorageEXT) (GLuint, GLsizeiptr, const void*, GLbitfield);
extern pfn_glNamedBufferStorageEXT fp_glNamedBufferStorageEXT;
typedef void (APIENTRYP pfn_glClearNamedBufferDataEXT) (GLuint, GLenum, GLenum, GLenum, const void*);
extern pfn_glClearNamedBufferDataEXT fp_glClearNamedBufferDataEXT;
typedef void (APIENTRYP pfn_glClearNamedBufferSubDataEXT) (GLuint, GLenum, GLsizeiptr, GLsizeiptr, GLenum, GLenum, const void*);
extern pfn_glClearNamedBufferSubDataEXT fp_glClearNamedBufferSubDataEXT;
typedef void (APIENTRYP pfn_glNamedFramebufferParameteriEXT) (GLuint, GLenum, GLint);
extern pfn_glNamedFramebufferParameteriEXT fp_glNamedFramebufferParameteriEXT;
typedef void (APIENTRYP pfn_glGetNamedFramebufferParameterivEXT) (GLuint, GLenum, GLint*);
extern pfn_glGetNamedFramebufferParameterivEXT fp_glGetNamedFramebufferParameterivEXT;
typedef void (APIENTRYP pfn_glProgramUniform1dEXT) (GLuint, GLint, GLdouble);
extern pfn_glProgramUniform1dEXT fp_glProgramUniform1dEXT;
typedef void (APIENTRYP pfn_glProgramUniform2dEXT) (GLuint, GLint, GLdouble, GLdouble);
extern pfn_glProgramUniform2dEXT fp_glProgramUniform2dEXT;
typedef void (APIENTRYP pfn_glProgramUniform3dEXT) (GLuint, GLint, GLdouble, GLdouble, GLdouble);
extern pfn_glProgramUniform3dEXT fp_glProgramUniform3dEXT;
typedef void (APIENTRYP pfn_glProgramUniform4dEXT) (GLuint, GLint, GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glProgramUniform4dEXT fp_glProgramUniform4dEXT;
typedef void (APIENTRYP pfn_glProgramUniform1dvEXT) (GLuint, GLint, GLsizei, const GLdouble*);
extern pfn_glProgramUniform1dvEXT fp_glProgramUniform1dvEXT;
typedef void (APIENTRYP pfn_glProgramUniform2dvEXT) (GLuint, GLint, GLsizei, const GLdouble*);
extern pfn_glProgramUniform2dvEXT fp_glProgramUniform2dvEXT;
typedef void (APIENTRYP pfn_glProgramUniform3dvEXT) (GLuint, GLint, GLsizei, const GLdouble*);
extern pfn_glProgramUniform3dvEXT fp_glProgramUniform3dvEXT;
typedef void (APIENTRYP pfn_glProgramUniform4dvEXT) (GLuint, GLint, GLsizei, const GLdouble*);
extern pfn_glProgramUniform4dvEXT fp_glProgramUniform4dvEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix2dvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix2dvEXT fp_glProgramUniformMatrix2dvEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix3dvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix3dvEXT fp_glProgramUniformMatrix3dvEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix4dvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix4dvEXT fp_glProgramUniformMatrix4dvEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix2x3dvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix2x3dvEXT fp_glProgramUniformMatrix2x3dvEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix2x4dvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix2x4dvEXT fp_glProgramUniformMatrix2x4dvEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix3x2dvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix3x2dvEXT fp_glProgramUniformMatrix3x2dvEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix3x4dvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix3x4dvEXT fp_glProgramUniformMatrix3x4dvEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix4x2dvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix4x2dvEXT fp_glProgramUniformMatrix4x2dvEXT;
typedef void (APIENTRYP pfn_glProgramUniformMatrix4x3dvEXT) (GLuint, GLint, GLsizei, GLboolean, const GLdouble*);
extern pfn_glProgramUniformMatrix4x3dvEXT fp_glProgramUniformMatrix4x3dvEXT;
typedef void (APIENTRYP pfn_glTextureBufferRangeEXT) (GLuint, GLenum, GLenum, GLuint, GLintptr, GLsizeiptr);
extern pfn_glTextureBufferRangeEXT fp_glTextureBufferRangeEXT;
typedef void (APIENTRYP pfn_glTextureStorage1DEXT) (GLuint, GLenum, GLsizei, GLenum, GLsizei);
extern pfn_glTextureStorage1DEXT fp_glTextureStorage1DEXT;
typedef void (APIENTRYP pfn_glTextureStorage2DEXT) (GLuint, GLenum, GLsizei, GLenum, GLsizei, GLsizei);
extern pfn_glTextureStorage2DEXT fp_glTextureStorage2DEXT;
typedef void (APIENTRYP pfn_glTextureStorage3DEXT) (GLuint, GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei);
extern pfn_glTextureStorage3DEXT fp_glTextureStorage3DEXT;
typedef void (APIENTRYP pfn_glTextureStorage2DMultisampleEXT) (GLuint, GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean);
extern pfn_glTextureStorage2DMultisampleEXT fp_glTextureStorage2DMultisampleEXT;
typedef void (APIENTRYP pfn_glTextureStorage3DMultisampleEXT) (GLuint, GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean);
extern pfn_glTextureStorage3DMultisampleEXT fp_glTextureStorage3DMultisampleEXT;
typedef void (APIENTRYP pfn_glVertexArrayBindVertexBufferEXT) (GLuint, GLuint, GLuint, GLintptr, GLsizei);
extern pfn_glVertexArrayBindVertexBufferEXT fp_glVertexArrayBindVertexBufferEXT;
typedef void (APIENTRYP pfn_glVertexArrayVertexAttribFormatEXT) (GLuint, GLuint, GLint, GLenum, GLboolean, GLuint);
extern pfn_glVertexArrayVertexAttribFormatEXT fp_glVertexArrayVertexAttribFormatEXT;
typedef void (APIENTRYP pfn_glVertexArrayVertexAttribIFormatEXT) (GLuint, GLuint, GLint, GLenum, GLuint);
extern pfn_glVertexArrayVertexAttribIFormatEXT fp_glVertexArrayVertexAttribIFormatEXT;
typedef void (APIENTRYP pfn_glVertexArrayVertexAttribLFormatEXT) (GLuint, GLuint, GLint, GLenum, GLuint);
extern pfn_glVertexArrayVertexAttribLFormatEXT fp_glVertexArrayVertexAttribLFormatEXT;
typedef void (APIENTRYP pfn_glVertexArrayVertexAttribBindingEXT) (GLuint, GLuint, GLuint);
extern pfn_glVertexArrayVertexAttribBindingEXT fp_glVertexArrayVertexAttribBindingEXT;
typedef void (APIENTRYP pfn_glVertexArrayVertexBindingDivisorEXT) (GLuint, GLuint, GLuint);
extern pfn_glVertexArrayVertexBindingDivisorEXT fp_glVertexArrayVertexBindingDivisorEXT;
typedef void (APIENTRYP pfn_glVertexArrayVertexAttribLOffsetEXT) (GLuint, GLuint, GLuint, GLint, GLenum, GLsizei, GLintptr);
extern pfn_glVertexArrayVertexAttribLOffsetEXT fp_glVertexArrayVertexAttribLOffsetEXT;
typedef void (APIENTRYP pfn_glTexturePageCommitmentEXT) (GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLboolean);
extern pfn_glTexturePageCommitmentEXT fp_glTexturePageCommitmentEXT;
typedef void (APIENTRYP pfn_glVertexArrayVertexAttribDivisorEXT) (GLuint, GLuint, GLuint);
extern pfn_glVertexArrayVertexAttribDivisorEXT fp_glVertexArrayVertexAttribDivisorEXT;

 /* GL_EXT_discard_framebuffer */
extern GLboolean GLAD_EXT_discard_framebuffer;
#define GL_COLOR_EXT                           0x1800
#define GL_DEPTH_EXT                           0x1801
#define GL_STENCIL_EXT                         0x1802
typedef void (APIENTRYP pfn_glDiscardFramebufferEXT) (GLenum, GLsizei, const GLenum*);
extern pfn_glDiscardFramebufferEXT fp_glDiscardFramebufferEXT;

 /* GL_EXT_disjoint_timer_query */
extern GLboolean GLAD_EXT_disjoint_timer_query;
#define GL_QUERY_COUNTER_BITS_EXT              0x8864
#define GL_CURRENT_QUERY_EXT                   0x8865
#define GL_QUERY_RESULT_EXT                    0x8866
#define GL_QUERY_RESULT_AVAILABLE_EXT          0x8867
#define GL_TIME_ELAPSED_EXT                    0x88BF
#define GL_TIMESTAMP_EXT                       0x8E28
#define GL_GPU_DISJOINT_EXT                    0x8FBB
typedef void (APIENTRYP pfn_glGenQueriesEXT) (GLsizei, GLuint*);
extern pfn_glGenQueriesEXT fp_glGenQueriesEXT;
typedef void (APIENTRYP pfn_glDeleteQueriesEXT) (GLsizei, const GLuint*);
extern pfn_glDeleteQueriesEXT fp_glDeleteQueriesEXT;
typedef GLboolean (APIENTRYP pfn_glIsQueryEXT) (GLuint);
extern pfn_glIsQueryEXT fp_glIsQueryEXT;
typedef void (APIENTRYP pfn_glBeginQueryEXT) (GLenum, GLuint);
extern pfn_glBeginQueryEXT fp_glBeginQueryEXT;
typedef void (APIENTRYP pfn_glEndQueryEXT) (GLenum);
extern pfn_glEndQueryEXT fp_glEndQueryEXT;
typedef void (APIENTRYP pfn_glQueryCounterEXT) (GLuint, GLenum);
extern pfn_glQueryCounterEXT fp_glQueryCounterEXT;
typedef void (APIENTRYP pfn_glGetQueryivEXT) (GLenum, GLenum, GLint*);
extern pfn_glGetQueryivEXT fp_glGetQueryivEXT;
typedef void (APIENTRYP pfn_glGetQueryObjectivEXT) (GLuint, GLenum, GLint*);
extern pfn_glGetQueryObjectivEXT fp_glGetQueryObjectivEXT;
typedef void (APIENTRYP pfn_glGetQueryObjectuivEXT) (GLuint, GLenum, GLuint*);
extern pfn_glGetQueryObjectuivEXT fp_glGetQueryObjectuivEXT;
typedef void (APIENTRYP pfn_glGetQueryObjecti64vEXT) (GLuint, GLenum, GLint64*);
extern pfn_glGetQueryObjecti64vEXT fp_glGetQueryObjecti64vEXT;
typedef void (APIENTRYP pfn_glGetQueryObjectui64vEXT) (GLuint, GLenum, GLuint64*);
extern pfn_glGetQueryObjectui64vEXT fp_glGetQueryObjectui64vEXT;

 /* GL_EXT_draw_buffers */
extern GLboolean GLAD_EXT_draw_buffers;
#define GL_MAX_COLOR_ATTACHMENTS_EXT           0x8CDF
#define GL_MAX_DRAW_BUFFERS_EXT                0x8824
#define GL_DRAW_BUFFER0_EXT                    0x8825
#define GL_DRAW_BUFFER1_EXT                    0x8826
#define GL_DRAW_BUFFER2_EXT                    0x8827
#define GL_DRAW_BUFFER3_EXT                    0x8828
#define GL_DRAW_BUFFER4_EXT                    0x8829
#define GL_DRAW_BUFFER5_EXT                    0x882A
#define GL_DRAW_BUFFER6_EXT                    0x882B
#define GL_DRAW_BUFFER7_EXT                    0x882C
#define GL_DRAW_BUFFER8_EXT                    0x882D
#define GL_DRAW_BUFFER9_EXT                    0x882E
#define GL_DRAW_BUFFER10_EXT                   0x882F
#define GL_DRAW_BUFFER11_EXT                   0x8830
#define GL_DRAW_BUFFER12_EXT                   0x8831
#define GL_DRAW_BUFFER13_EXT                   0x8832
#define GL_DRAW_BUFFER14_EXT                   0x8833
#define GL_DRAW_BUFFER15_EXT                   0x8834
#define GL_COLOR_ATTACHMENT0_EXT               0x8CE0
#define GL_COLOR_ATTACHMENT1_EXT               0x8CE1
#define GL_COLOR_ATTACHMENT2_EXT               0x8CE2
#define GL_COLOR_ATTACHMENT3_EXT               0x8CE3
#define GL_COLOR_ATTACHMENT4_EXT               0x8CE4
#define GL_COLOR_ATTACHMENT5_EXT               0x8CE5
#define GL_COLOR_ATTACHMENT6_EXT               0x8CE6
#define GL_COLOR_ATTACHMENT7_EXT               0x8CE7
#define GL_COLOR_ATTACHMENT8_EXT               0x8CE8
#define GL_COLOR_ATTACHMENT9_EXT               0x8CE9
#define GL_COLOR_ATTACHMENT10_EXT              0x8CEA
#define GL_COLOR_ATTACHMENT11_EXT              0x8CEB
#define GL_COLOR_ATTACHMENT12_EXT              0x8CEC
#define GL_COLOR_ATTACHMENT13_EXT              0x8CED
#define GL_COLOR_ATTACHMENT14_EXT              0x8CEE
#define GL_COLOR_ATTACHMENT15_EXT              0x8CEF
typedef void (APIENTRYP pfn_glDrawBuffersEXT) (GLsizei, const GLenum*);
extern pfn_glDrawBuffersEXT fp_glDrawBuffersEXT;

 /* GL_EXT_draw_buffers2 */
extern GLboolean GLAD_EXT_draw_buffers2;
typedef void (APIENTRYP pfn_glColorMaskIndexedEXT) (GLuint, GLboolean, GLboolean, GLboolean, GLboolean);
extern pfn_glColorMaskIndexedEXT fp_glColorMaskIndexedEXT;

 /* GL_EXT_draw_buffers_indexed */
extern GLboolean GLAD_EXT_draw_buffers_indexed;
typedef void (APIENTRYP pfn_glEnableiEXT) (GLenum, GLuint);
extern pfn_glEnableiEXT fp_glEnableiEXT;
typedef void (APIENTRYP pfn_glDisableiEXT) (GLenum, GLuint);
extern pfn_glDisableiEXT fp_glDisableiEXT;
typedef void (APIENTRYP pfn_glBlendEquationiEXT) (GLuint, GLenum);
extern pfn_glBlendEquationiEXT fp_glBlendEquationiEXT;
typedef void (APIENTRYP pfn_glBlendEquationSeparateiEXT) (GLuint, GLenum, GLenum);
extern pfn_glBlendEquationSeparateiEXT fp_glBlendEquationSeparateiEXT;
typedef void (APIENTRYP pfn_glBlendFunciEXT) (GLuint, GLenum, GLenum);
extern pfn_glBlendFunciEXT fp_glBlendFunciEXT;
typedef void (APIENTRYP pfn_glBlendFuncSeparateiEXT) (GLuint, GLenum, GLenum, GLenum, GLenum);
extern pfn_glBlendFuncSeparateiEXT fp_glBlendFuncSeparateiEXT;
typedef void (APIENTRYP pfn_glColorMaskiEXT) (GLuint, GLboolean, GLboolean, GLboolean, GLboolean);
extern pfn_glColorMaskiEXT fp_glColorMaskiEXT;
typedef GLboolean (APIENTRYP pfn_glIsEnablediEXT) (GLenum, GLuint);
extern pfn_glIsEnablediEXT fp_glIsEnablediEXT;

 /* GL_EXT_draw_elements_base_vertex */
extern GLboolean GLAD_EXT_draw_elements_base_vertex;
typedef void (APIENTRYP pfn_glDrawElementsBaseVertexEXT) (GLenum, GLsizei, GLenum, const void*, GLint);
extern pfn_glDrawElementsBaseVertexEXT fp_glDrawElementsBaseVertexEXT;
typedef void (APIENTRYP pfn_glDrawRangeElementsBaseVertexEXT) (GLenum, GLuint, GLuint, GLsizei, GLenum, const void*, GLint);
extern pfn_glDrawRangeElementsBaseVertexEXT fp_glDrawRangeElementsBaseVertexEXT;
typedef void (APIENTRYP pfn_glDrawElementsInstancedBaseVertexEXT) (GLenum, GLsizei, GLenum, const void*, GLsizei, GLint);
extern pfn_glDrawElementsInstancedBaseVertexEXT fp_glDrawElementsInstancedBaseVertexEXT;
typedef void (APIENTRYP pfn_glMultiDrawElementsBaseVertexEXT) (GLenum, const GLsizei*, GLenum, const void**, GLsizei, const GLint*);
extern pfn_glMultiDrawElementsBaseVertexEXT fp_glMultiDrawElementsBaseVertexEXT;

 /* GL_EXT_draw_instanced */
extern GLboolean GLAD_EXT_draw_instanced;
typedef void (APIENTRYP pfn_glDrawArraysInstancedEXT) (GLenum, GLint, GLsizei, GLsizei);
extern pfn_glDrawArraysInstancedEXT fp_glDrawArraysInstancedEXT;
typedef void (APIENTRYP pfn_glDrawElementsInstancedEXT) (GLenum, GLsizei, GLenum, const void*, GLsizei);
extern pfn_glDrawElementsInstancedEXT fp_glDrawElementsInstancedEXT;

 /* GL_EXT_float_blend */
extern GLboolean GLAD_EXT_float_blend;

 /* GL_EXT_framebuffer_blit */
extern GLboolean GLAD_EXT_framebuffer_blit;
#define GL_READ_FRAMEBUFFER_EXT                0x8CA8
#define GL_DRAW_FRAMEBUFFER_EXT                0x8CA9
#define GL_DRAW_FRAMEBUFFER_BINDING_EXT        0x8CA6
#define GL_READ_FRAMEBUFFER_BINDING_EXT        0x8CAA
typedef void (APIENTRYP pfn_glBlitFramebufferEXT) (GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
extern pfn_glBlitFramebufferEXT fp_glBlitFramebufferEXT;

 /* GL_EXT_framebuffer_multisample */
extern GLboolean GLAD_EXT_framebuffer_multisample;
#define GL_RENDERBUFFER_SAMPLES_EXT            0x8CAB
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT  0x8D56
#define GL_MAX_SAMPLES_EXT                     0x8D57
typedef void (APIENTRYP pfn_glRenderbufferStorageMultisampleEXT) (GLenum, GLsizei, GLenum, GLsizei, GLsizei);
extern pfn_glRenderbufferStorageMultisampleEXT fp_glRenderbufferStorageMultisampleEXT;

 /* GL_EXT_framebuffer_multisample_blit_scaled */
extern GLboolean GLAD_EXT_framebuffer_multisample_blit_scaled;
#define GL_SCALED_RESOLVE_FASTEST_EXT          0x90BA
#define GL_SCALED_RESOLVE_NICEST_EXT           0x90BB

 /* GL_EXT_framebuffer_object */
extern GLboolean GLAD_EXT_framebuffer_object;
#define GL_INVALID_FRAMEBUFFER_OPERATION_EXT   0x0506
#define GL_MAX_RENDERBUFFER_SIZE_EXT           0x84E8
#define GL_FRAMEBUFFER_BINDING_EXT             0x8CA6
#define GL_RENDERBUFFER_BINDING_EXT            0x8CA7
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT  0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT  0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT  0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT  0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT  0x8CD4
#define GL_FRAMEBUFFER_COMPLETE_EXT            0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT  0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT  0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT  0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT  0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT  0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT  0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT         0x8CDD
#define GL_DEPTH_ATTACHMENT_EXT                0x8D00
#define GL_STENCIL_ATTACHMENT_EXT              0x8D20
#define GL_FRAMEBUFFER_EXT                     0x8D40
#define GL_RENDERBUFFER_EXT                    0x8D41
#define GL_RENDERBUFFER_WIDTH_EXT              0x8D42
#define GL_RENDERBUFFER_HEIGHT_EXT             0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT_EXT    0x8D44
#define GL_STENCIL_INDEX1_EXT                  0x8D46
#define GL_STENCIL_INDEX4_EXT                  0x8D47
#define GL_STENCIL_INDEX8_EXT                  0x8D48
#define GL_STENCIL_INDEX16_EXT                 0x8D49
#define GL_RENDERBUFFER_RED_SIZE_EXT           0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE_EXT         0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE_EXT          0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE_EXT         0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE_EXT         0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE_EXT       0x8D55
typedef GLboolean (APIENTRYP pfn_glIsRenderbufferEXT) (GLuint);
extern pfn_glIsRenderbufferEXT fp_glIsRenderbufferEXT;
typedef void (APIENTRYP pfn_glBindRenderbufferEXT) (GLenum, GLuint);
extern pfn_glBindRenderbufferEXT fp_glBindRenderbufferEXT;
typedef void (APIENTRYP pfn_glDeleteRenderbuffersEXT) (GLsizei, const GLuint*);
extern pfn_glDeleteRenderbuffersEXT fp_glDeleteRenderbuffersEXT;
typedef void (APIENTRYP pfn_glGenRenderbuffersEXT) (GLsizei, GLuint*);
extern pfn_glGenRenderbuffersEXT fp_glGenRenderbuffersEXT;
typedef void (APIENTRYP pfn_glRenderbufferStorageEXT) (GLenum, GLenum, GLsizei, GLsizei);
extern pfn_glRenderbufferStorageEXT fp_glRenderbufferStorageEXT;
typedef void (APIENTRYP pfn_glGetRenderbufferParameterivEXT) (GLenum, GLenum, GLint*);
extern pfn_glGetRenderbufferParameterivEXT fp_glGetRenderbufferParameterivEXT;
typedef GLboolean (APIENTRYP pfn_glIsFramebufferEXT) (GLuint);
extern pfn_glIsFramebufferEXT fp_glIsFramebufferEXT;
typedef void (APIENTRYP pfn_glBindFramebufferEXT) (GLenum, GLuint);
extern pfn_glBindFramebufferEXT fp_glBindFramebufferEXT;
typedef void (APIENTRYP pfn_glDeleteFramebuffersEXT) (GLsizei, const GLuint*);
extern pfn_glDeleteFramebuffersEXT fp_glDeleteFramebuffersEXT;
typedef void (APIENTRYP pfn_glGenFramebuffersEXT) (GLsizei, GLuint*);
extern pfn_glGenFramebuffersEXT fp_glGenFramebuffersEXT;
typedef GLenum (APIENTRYP pfn_glCheckFramebufferStatusEXT) (GLenum);
extern pfn_glCheckFramebufferStatusEXT fp_glCheckFramebufferStatusEXT;
typedef void (APIENTRYP pfn_glFramebufferTexture1DEXT) (GLenum, GLenum, GLenum, GLuint, GLint);
extern pfn_glFramebufferTexture1DEXT fp_glFramebufferTexture1DEXT;
typedef void (APIENTRYP pfn_glFramebufferTexture2DEXT) (GLenum, GLenum, GLenum, GLuint, GLint);
extern pfn_glFramebufferTexture2DEXT fp_glFramebufferTexture2DEXT;
typedef void (APIENTRYP pfn_glFramebufferTexture3DEXT) (GLenum, GLenum, GLenum, GLuint, GLint, GLint);
extern pfn_glFramebufferTexture3DEXT fp_glFramebufferTexture3DEXT;
typedef void (APIENTRYP pfn_glFramebufferRenderbufferEXT) (GLenum, GLenum, GLenum, GLuint);
extern pfn_glFramebufferRenderbufferEXT fp_glFramebufferRenderbufferEXT;
typedef void (APIENTRYP pfn_glGetFramebufferAttachmentParameterivEXT) (GLenum, GLenum, GLenum, GLint*);
extern pfn_glGetFramebufferAttachmentParameterivEXT fp_glGetFramebufferAttachmentParameterivEXT;
typedef void (APIENTRYP pfn_glGenerateMipmapEXT) (GLenum);
extern pfn_glGenerateMipmapEXT fp_glGenerateMipmapEXT;

 /* GL_EXT_framebuffer_sRGB */
extern GLboolean GLAD_EXT_framebuffer_sRGB;
#define GL_FRAMEBUFFER_SRGB_EXT                0x8DB9
#define GL_FRAMEBUFFER_SRGB_CAPABLE_EXT        0x8DBA

 /* GL_EXT_geometry_point_size */
extern GLboolean GLAD_EXT_geometry_point_size;

 /* GL_EXT_geometry_shader */
extern GLboolean GLAD_EXT_geometry_shader;
#define GL_GEOMETRY_SHADER_EXT                 0x8DD9
#define GL_GEOMETRY_SHADER_BIT_EXT             0x00000004
#define GL_GEOMETRY_LINKED_VERTICES_OUT_EXT    0x8916
#define GL_GEOMETRY_LINKED_INPUT_TYPE_EXT      0x8917
#define GL_GEOMETRY_LINKED_OUTPUT_TYPE_EXT     0x8918
#define GL_GEOMETRY_SHADER_INVOCATIONS_EXT     0x887F
#define GL_LAYER_PROVOKING_VERTEX_EXT          0x825E
#define GL_LINES_ADJACENCY_EXT                 0x000A
#define GL_LINE_STRIP_ADJACENCY_EXT            0x000B
#define GL_TRIANGLES_ADJACENCY_EXT             0x000C
#define GL_TRIANGLE_STRIP_ADJACENCY_EXT        0x000D
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT  0x8DDF
#define GL_MAX_GEOMETRY_UNIFORM_BLOCKS_EXT     0x8A2C
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS_EXT  0x8A32
#define GL_MAX_GEOMETRY_INPUT_COMPONENTS_EXT   0x9123
#define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS_EXT  0x9124
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT    0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT  0x8DE1
#define GL_MAX_GEOMETRY_SHADER_INVOCATIONS_EXT  0x8E5A
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT  0x8C29
#define GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS_EXT  0x92CF
#define GL_MAX_GEOMETRY_ATOMIC_COUNTERS_EXT    0x92D5
#define GL_MAX_GEOMETRY_IMAGE_UNIFORMS_EXT     0x90CD
#define GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_EXT  0x90D7
#define GL_FIRST_VERTEX_CONVENTION_EXT         0x8E4D
#define GL_LAST_VERTEX_CONVENTION_EXT          0x8E4E
#define GL_UNDEFINED_VERTEX_EXT                0x8260
#define GL_PRIMITIVES_GENERATED_EXT            0x8C87
#define GL_FRAMEBUFFER_DEFAULT_LAYERS_EXT      0x9312
#define GL_MAX_FRAMEBUFFER_LAYERS_EXT          0x9317
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT  0x8DA8
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED_EXT  0x8DA7
#define GL_REFERENCED_BY_GEOMETRY_SHADER_EXT   0x9309
typedef void (APIENTRYP pfn_glFramebufferTextureEXT) (GLenum, GLenum, GLuint, GLint);
extern pfn_glFramebufferTextureEXT fp_glFramebufferTextureEXT;

 /* GL_EXT_geometry_shader4 */
extern GLboolean GLAD_EXT_geometry_shader4;
#define GL_GEOMETRY_VERTICES_OUT_EXT           0x8DDA
#define GL_GEOMETRY_INPUT_TYPE_EXT             0x8DDB
#define GL_GEOMETRY_OUTPUT_TYPE_EXT            0x8DDC
#define GL_MAX_GEOMETRY_VARYING_COMPONENTS_EXT  0x8DDD
#define GL_MAX_VERTEX_VARYING_COMPONENTS_EXT   0x8DDE
#define GL_MAX_VARYING_COMPONENTS_EXT          0x8B4B
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_EXT  0x8DA9
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER_EXT  0x8CD4
#define GL_PROGRAM_POINT_SIZE_EXT              0x8642
typedef void (APIENTRYP pfn_glProgramParameteriEXT) (GLuint, GLenum, GLint);
extern pfn_glProgramParameteriEXT fp_glProgramParameteriEXT;

 /* GL_EXT_gpu_program_parameters */
extern GLboolean GLAD_EXT_gpu_program_parameters;
typedef void (APIENTRYP pfn_glProgramEnvParameters4fvEXT) (GLenum, GLuint, GLsizei, const GLfloat*);
extern pfn_glProgramEnvParameters4fvEXT fp_glProgramEnvParameters4fvEXT;
typedef void (APIENTRYP pfn_glProgramLocalParameters4fvEXT) (GLenum, GLuint, GLsizei, const GLfloat*);
extern pfn_glProgramLocalParameters4fvEXT fp_glProgramLocalParameters4fvEXT;

 /* GL_EXT_gpu_shader4 */
extern GLboolean GLAD_EXT_gpu_shader4;
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER_EXT     0x88FD
#define GL_SAMPLER_1D_ARRAY_EXT                0x8DC0
#define GL_SAMPLER_2D_ARRAY_EXT                0x8DC1
#define GL_SAMPLER_BUFFER_EXT                  0x8DC2
#define GL_SAMPLER_1D_ARRAY_SHADOW_EXT         0x8DC3
#define GL_SAMPLER_2D_ARRAY_SHADOW_EXT         0x8DC4
#define GL_SAMPLER_CUBE_SHADOW_EXT             0x8DC5
#define GL_UNSIGNED_INT_VEC2_EXT               0x8DC6
#define GL_UNSIGNED_INT_VEC3_EXT               0x8DC7
#define GL_UNSIGNED_INT_VEC4_EXT               0x8DC8
#define GL_INT_SAMPLER_1D_EXT                  0x8DC9
#define GL_INT_SAMPLER_2D_EXT                  0x8DCA
#define GL_INT_SAMPLER_3D_EXT                  0x8DCB
#define GL_INT_SAMPLER_CUBE_EXT                0x8DCC
#define GL_INT_SAMPLER_2D_RECT_EXT             0x8DCD
#define GL_INT_SAMPLER_1D_ARRAY_EXT            0x8DCE
#define GL_INT_SAMPLER_2D_ARRAY_EXT            0x8DCF
#define GL_INT_SAMPLER_BUFFER_EXT              0x8DD0
#define GL_UNSIGNED_INT_SAMPLER_1D_EXT         0x8DD1
#define GL_UNSIGNED_INT_SAMPLER_2D_EXT         0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_3D_EXT         0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_CUBE_EXT       0x8DD4
#define GL_UNSIGNED_INT_SAMPLER_2D_RECT_EXT    0x8DD5
#define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY_EXT   0x8DD6
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY_EXT   0x8DD7
#define GL_UNSIGNED_INT_SAMPLER_BUFFER_EXT     0x8DD8
#define GL_MIN_PROGRAM_TEXEL_OFFSET_EXT        0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET_EXT        0x8905
typedef void (APIENTRYP pfn_glGetUniformuivEXT) (GLuint, GLint, GLuint*);
extern pfn_glGetUniformuivEXT fp_glGetUniformuivEXT;
typedef GLint (APIENTRYP pfn_glGetFragDataLocationEXT) (GLuint, const GLchar*);
extern pfn_glGetFragDataLocationEXT fp_glGetFragDataLocationEXT;
typedef void (APIENTRYP pfn_glUniform1uiEXT) (GLint, GLuint);
extern pfn_glUniform1uiEXT fp_glUniform1uiEXT;
typedef void (APIENTRYP pfn_glUniform2uiEXT) (GLint, GLuint, GLuint);
extern pfn_glUniform2uiEXT fp_glUniform2uiEXT;
typedef void (APIENTRYP pfn_glUniform3uiEXT) (GLint, GLuint, GLuint, GLuint);
extern pfn_glUniform3uiEXT fp_glUniform3uiEXT;
typedef void (APIENTRYP pfn_glUniform4uiEXT) (GLint, GLuint, GLuint, GLuint, GLuint);
extern pfn_glUniform4uiEXT fp_glUniform4uiEXT;
typedef void (APIENTRYP pfn_glUniform1uivEXT) (GLint, GLsizei, const GLuint*);
extern pfn_glUniform1uivEXT fp_glUniform1uivEXT;
typedef void (APIENTRYP pfn_glUniform2uivEXT) (GLint, GLsizei, const GLuint*);
extern pfn_glUniform2uivEXT fp_glUniform2uivEXT;
typedef void (APIENTRYP pfn_glUniform3uivEXT) (GLint, GLsizei, const GLuint*);
extern pfn_glUniform3uivEXT fp_glUniform3uivEXT;
typedef void (APIENTRYP pfn_glUniform4uivEXT) (GLint, GLsizei, const GLuint*);
extern pfn_glUniform4uivEXT fp_glUniform4uivEXT;

 /* GL_EXT_gpu_shader5 */
extern GLboolean GLAD_EXT_gpu_shader5;

 /* GL_EXT_instanced_arrays */
extern GLboolean GLAD_EXT_instanced_arrays;
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR_EXT     0x88FE
typedef void (APIENTRYP pfn_glVertexAttribDivisorEXT) (GLuint, GLuint);
extern pfn_glVertexAttribDivisorEXT fp_glVertexAttribDivisorEXT;

 /* GL_EXT_map_buffer_range */
extern GLboolean GLAD_EXT_map_buffer_range;
#define GL_MAP_READ_BIT_EXT                    0x0001
#define GL_MAP_WRITE_BIT_EXT                   0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT_EXT        0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT_EXT       0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT_EXT          0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT_EXT          0x0020
typedef void* (APIENTRYP pfn_glMapBufferRangeEXT) (GLenum, GLintptr, GLsizeiptr, GLbitfield);
extern pfn_glMapBufferRangeEXT fp_glMapBufferRangeEXT;
typedef void (APIENTRYP pfn_glFlushMappedBufferRangeEXT) (GLenum, GLintptr, GLsizeiptr);
extern pfn_glFlushMappedBufferRangeEXT fp_glFlushMappedBufferRangeEXT;

 /* GL_EXT_multi_draw_indirect */
extern GLboolean GLAD_EXT_multi_draw_indirect;
typedef void (APIENTRYP pfn_glMultiDrawArraysIndirectEXT) (GLenum, const void*, GLsizei, GLsizei);
extern pfn_glMultiDrawArraysIndirectEXT fp_glMultiDrawArraysIndirectEXT;
typedef void (APIENTRYP pfn_glMultiDrawElementsIndirectEXT) (GLenum, GLenum, const void*, GLsizei, GLsizei);
extern pfn_glMultiDrawElementsIndirectEXT fp_glMultiDrawElementsIndirectEXT;

 /* GL_EXT_multisampled_compatibility */
extern GLboolean GLAD_EXT_multisampled_compatibility;
#define GL_MULTISAMPLE_EXT                     0x809D
#define GL_SAMPLE_ALPHA_TO_ONE_EXT             0x809F

 /* GL_EXT_multisampled_render_to_texture */
extern GLboolean GLAD_EXT_multisampled_render_to_texture;
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES_EXT  0x8D6C
typedef void (APIENTRYP pfn_glFramebufferTexture2DMultisampleEXT) (GLenum, GLenum, GLenum, GLuint, GLint, GLsizei);
extern pfn_glFramebufferTexture2DMultisampleEXT fp_glFramebufferTexture2DMultisampleEXT;

 /* GL_EXT_multiview_draw_buffers */
extern GLboolean GLAD_EXT_multiview_draw_buffers;
#define GL_COLOR_ATTACHMENT_EXT                0x90F0
#define GL_MULTIVIEW_EXT                       0x90F1
#define GL_DRAW_BUFFER_EXT                     0x0C01
#define GL_READ_BUFFER_EXT                     0x0C02
#define GL_MAX_MULTIVIEW_BUFFERS_EXT           0x90F2
typedef void (APIENTRYP pfn_glReadBufferIndexedEXT) (GLenum, GLint);
extern pfn_glReadBufferIndexedEXT fp_glReadBufferIndexedEXT;
typedef void (APIENTRYP pfn_glDrawBuffersIndexedEXT) (GLint, const GLenum*, const GLint*);
extern pfn_glDrawBuffersIndexedEXT fp_glDrawBuffersIndexedEXT;
typedef void (APIENTRYP pfn_glGetIntegeri_vEXT) (GLenum, GLuint, GLint*);
extern pfn_glGetIntegeri_vEXT fp_glGetIntegeri_vEXT;

 /* GL_EXT_occlusion_query_boolean */
extern GLboolean GLAD_EXT_occlusion_query_boolean;
#define GL_ANY_SAMPLES_PASSED_EXT              0x8C2F
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT  0x8D6A

 /* GL_EXT_packed_depth_stencil */
extern GLboolean GLAD_EXT_packed_depth_stencil;
#define GL_DEPTH_STENCIL_EXT                   0x84F9
#define GL_UNSIGNED_INT_24_8_EXT               0x84FA
#define GL_DEPTH24_STENCIL8_EXT                0x88F0
#define GL_TEXTURE_STENCIL_SIZE_EXT            0x88F1

 /* GL_EXT_packed_float */
extern GLboolean GLAD_EXT_packed_float;
#define GL_R11F_G11F_B10F_EXT                  0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV_EXT    0x8C3B
#define GL_RGBA_SIGNED_COMPONENTS_EXT          0x8C3C

 /* GL_EXT_pixel_buffer_object */
extern GLboolean GLAD_EXT_pixel_buffer_object;
#define GL_PIXEL_PACK_BUFFER_EXT               0x88EB
#define GL_PIXEL_UNPACK_BUFFER_EXT             0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING_EXT       0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING_EXT     0x88EF

 /* GL_EXT_polygon_offset_clamp */
extern GLboolean GLAD_EXT_polygon_offset_clamp;
#define GL_POLYGON_OFFSET_CLAMP_EXT            0x8E1B
typedef void (APIENTRYP pfn_glPolygonOffsetClampEXT) (GLfloat, GLfloat, GLfloat);
extern pfn_glPolygonOffsetClampEXT fp_glPolygonOffsetClampEXT;

 /* GL_EXT_post_depth_coverage */
extern GLboolean GLAD_EXT_post_depth_coverage;

 /* GL_EXT_primitive_bounding_box */
extern GLboolean GLAD_EXT_primitive_bounding_box;
#define GL_PRIMITIVE_BOUNDING_BOX_EXT          0x92BE
typedef void (APIENTRYP pfn_glPrimitiveBoundingBoxEXT) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glPrimitiveBoundingBoxEXT fp_glPrimitiveBoundingBoxEXT;

 /* GL_EXT_provoking_vertex */
extern GLboolean GLAD_EXT_provoking_vertex;
#define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION_EXT  0x8E4C
#define GL_PROVOKING_VERTEX_EXT                0x8E4F
typedef void (APIENTRYP pfn_glProvokingVertexEXT) (GLenum);
extern pfn_glProvokingVertexEXT fp_glProvokingVertexEXT;

 /* GL_EXT_pvrtc_sRGB */
extern GLboolean GLAD_EXT_pvrtc_sRGB;
#define GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT    0x8A54
#define GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT    0x8A55
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT  0x8A56
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT  0x8A57
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG  0x93F0
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG  0x93F1

 /* GL_EXT_raster_multisample */
extern GLboolean GLAD_EXT_raster_multisample;
#define GL_RASTER_MULTISAMPLE_EXT              0x9327
#define GL_RASTER_SAMPLES_EXT                  0x9328
#define GL_MAX_RASTER_SAMPLES_EXT              0x9329
#define GL_RASTER_FIXED_SAMPLE_LOCATIONS_EXT   0x932A
#define GL_MULTISAMPLE_RASTERIZATION_ALLOWED_EXT  0x932B
#define GL_EFFECTIVE_RASTER_SAMPLES_EXT        0x932C
typedef void (APIENTRYP pfn_glRasterSamplesEXT) (GLuint, GLboolean);
extern pfn_glRasterSamplesEXT fp_glRasterSamplesEXT;

 /* GL_EXT_read_format_bgra */
extern GLboolean GLAD_EXT_read_format_bgra;
#define GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT      0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT      0x8366

 /* GL_EXT_render_snorm */
extern GLboolean GLAD_EXT_render_snorm;
#define GL_R16_SNORM_EXT                       0x8F98
#define GL_RG16_SNORM_EXT                      0x8F99
#define GL_RGBA16_SNORM_EXT                    0x8F9B

 /* GL_EXT_robustness */
extern GLboolean GLAD_EXT_robustness;
#define GL_GUILTY_CONTEXT_RESET_EXT            0x8253
#define GL_INNOCENT_CONTEXT_RESET_EXT          0x8254
#define GL_UNKNOWN_CONTEXT_RESET_EXT           0x8255
#define GL_CONTEXT_ROBUST_ACCESS_EXT           0x90F3
#define GL_RESET_NOTIFICATION_STRATEGY_EXT     0x8256
#define GL_LOSE_CONTEXT_ON_RESET_EXT           0x8252
#define GL_NO_RESET_NOTIFICATION_EXT           0x8261
typedef GLenum (APIENTRYP pfn_glGetGraphicsResetStatusEXT) ();
extern pfn_glGetGraphicsResetStatusEXT fp_glGetGraphicsResetStatusEXT;
typedef void (APIENTRYP pfn_glReadnPixelsEXT) (GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLsizei, void*);
extern pfn_glReadnPixelsEXT fp_glReadnPixelsEXT;
typedef void (APIENTRYP pfn_glGetnUniformfvEXT) (GLuint, GLint, GLsizei, GLfloat*);
extern pfn_glGetnUniformfvEXT fp_glGetnUniformfvEXT;
typedef void (APIENTRYP pfn_glGetnUniformivEXT) (GLuint, GLint, GLsizei, GLint*);
extern pfn_glGetnUniformivEXT fp_glGetnUniformivEXT;

 /* GL_EXT_separate_shader_objects */
extern GLboolean GLAD_EXT_separate_shader_objects;
#define GL_ACTIVE_PROGRAM_EXT                  0x8B8D
#define GL_VERTEX_SHADER_BIT_EXT               0x00000001
#define GL_FRAGMENT_SHADER_BIT_EXT             0x00000002
#define GL_ALL_SHADER_BITS_EXT                 0xFFFFFFFF
#define GL_PROGRAM_SEPARABLE_EXT               0x8258
#define GL_PROGRAM_PIPELINE_BINDING_EXT        0x825A
typedef void (APIENTRYP pfn_glUseShaderProgramEXT) (GLenum, GLuint);
extern pfn_glUseShaderProgramEXT fp_glUseShaderProgramEXT;
typedef void (APIENTRYP pfn_glActiveProgramEXT) (GLuint);
extern pfn_glActiveProgramEXT fp_glActiveProgramEXT;
typedef GLuint (APIENTRYP pfn_glCreateShaderProgramEXT) (GLenum, const GLchar*);
extern pfn_glCreateShaderProgramEXT fp_glCreateShaderProgramEXT;
typedef void (APIENTRYP pfn_glActiveShaderProgramEXT) (GLuint, GLuint);
extern pfn_glActiveShaderProgramEXT fp_glActiveShaderProgramEXT;
typedef void (APIENTRYP pfn_glBindProgramPipelineEXT) (GLuint);
extern pfn_glBindProgramPipelineEXT fp_glBindProgramPipelineEXT;
typedef GLuint (APIENTRYP pfn_glCreateShaderProgramvEXT) (GLenum, GLsizei, const GLchar**);
extern pfn_glCreateShaderProgramvEXT fp_glCreateShaderProgramvEXT;
typedef void (APIENTRYP pfn_glDeleteProgramPipelinesEXT) (GLsizei, const GLuint*);
extern pfn_glDeleteProgramPipelinesEXT fp_glDeleteProgramPipelinesEXT;
typedef void (APIENTRYP pfn_glGenProgramPipelinesEXT) (GLsizei, GLuint*);
extern pfn_glGenProgramPipelinesEXT fp_glGenProgramPipelinesEXT;
typedef void (APIENTRYP pfn_glGetProgramPipelineInfoLogEXT) (GLuint, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetProgramPipelineInfoLogEXT fp_glGetProgramPipelineInfoLogEXT;
typedef void (APIENTRYP pfn_glGetProgramPipelineivEXT) (GLuint, GLenum, GLint*);
extern pfn_glGetProgramPipelineivEXT fp_glGetProgramPipelineivEXT;
typedef GLboolean (APIENTRYP pfn_glIsProgramPipelineEXT) (GLuint);
extern pfn_glIsProgramPipelineEXT fp_glIsProgramPipelineEXT;
typedef void (APIENTRYP pfn_glUseProgramStagesEXT) (GLuint, GLbitfield, GLuint);
extern pfn_glUseProgramStagesEXT fp_glUseProgramStagesEXT;
typedef void (APIENTRYP pfn_glValidateProgramPipelineEXT) (GLuint);
extern pfn_glValidateProgramPipelineEXT fp_glValidateProgramPipelineEXT;

 /* GL_EXT_shader_framebuffer_fetch */
extern GLboolean GLAD_EXT_shader_framebuffer_fetch;
#define GL_FRAGMENT_SHADER_DISCARDS_SAMPLES_EXT  0x8A52

 /* GL_EXT_shader_image_load_formatted */
extern GLboolean GLAD_EXT_shader_image_load_formatted;

 /* GL_EXT_shader_image_load_store */
extern GLboolean GLAD_EXT_shader_image_load_store;
#define GL_MAX_IMAGE_UNITS_EXT                 0x8F38
#define GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS_EXT  0x8F39
#define GL_IMAGE_BINDING_NAME_EXT              0x8F3A
#define GL_IMAGE_BINDING_LEVEL_EXT             0x8F3B
#define GL_IMAGE_BINDING_LAYERED_EXT           0x8F3C
#define GL_IMAGE_BINDING_LAYER_EXT             0x8F3D
#define GL_IMAGE_BINDING_ACCESS_EXT            0x8F3E
#define GL_IMAGE_1D_EXT                        0x904C
#define GL_IMAGE_2D_EXT                        0x904D
#define GL_IMAGE_3D_EXT                        0x904E
#define GL_IMAGE_2D_RECT_EXT                   0x904F
#define GL_IMAGE_CUBE_EXT                      0x9050
#define GL_IMAGE_BUFFER_EXT                    0x9051
#define GL_IMAGE_1D_ARRAY_EXT                  0x9052
#define GL_IMAGE_2D_ARRAY_EXT                  0x9053
#define GL_IMAGE_CUBE_MAP_ARRAY_EXT            0x9054
#define GL_IMAGE_2D_MULTISAMPLE_EXT            0x9055
#define GL_IMAGE_2D_MULTISAMPLE_ARRAY_EXT      0x9056
#define GL_INT_IMAGE_1D_EXT                    0x9057
#define GL_INT_IMAGE_2D_EXT                    0x9058
#define GL_INT_IMAGE_3D_EXT                    0x9059
#define GL_INT_IMAGE_2D_RECT_EXT               0x905A
#define GL_INT_IMAGE_CUBE_EXT                  0x905B
#define GL_INT_IMAGE_BUFFER_EXT                0x905C
#define GL_INT_IMAGE_1D_ARRAY_EXT              0x905D
#define GL_INT_IMAGE_2D_ARRAY_EXT              0x905E
#define GL_INT_IMAGE_CUBE_MAP_ARRAY_EXT        0x905F
#define GL_INT_IMAGE_2D_MULTISAMPLE_EXT        0x9060
#define GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY_EXT  0x9061
#define GL_UNSIGNED_INT_IMAGE_1D_EXT           0x9062
#define GL_UNSIGNED_INT_IMAGE_2D_EXT           0x9063
#define GL_UNSIGNED_INT_IMAGE_3D_EXT           0x9064
#define GL_UNSIGNED_INT_IMAGE_2D_RECT_EXT      0x9065
#define GL_UNSIGNED_INT_IMAGE_CUBE_EXT         0x9066
#define GL_UNSIGNED_INT_IMAGE_BUFFER_EXT       0x9067
#define GL_UNSIGNED_INT_IMAGE_1D_ARRAY_EXT     0x9068
#define GL_UNSIGNED_INT_IMAGE_2D_ARRAY_EXT     0x9069
#define GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY_EXT  0x906A
#define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_EXT  0x906B
#define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY_EXT  0x906C
#define GL_MAX_IMAGE_SAMPLES_EXT               0x906D
#define GL_IMAGE_BINDING_FORMAT_EXT            0x906E
#define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT_EXT  0x00000001
#define GL_ELEMENT_ARRAY_BARRIER_BIT_EXT       0x00000002
#define GL_UNIFORM_BARRIER_BIT_EXT             0x00000004
#define GL_TEXTURE_FETCH_BARRIER_BIT_EXT       0x00000008
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT_EXT  0x00000020
#define GL_COMMAND_BARRIER_BIT_EXT             0x00000040
#define GL_PIXEL_BUFFER_BARRIER_BIT_EXT        0x00000080
#define GL_TEXTURE_UPDATE_BARRIER_BIT_EXT      0x00000100
#define GL_BUFFER_UPDATE_BARRIER_BIT_EXT       0x00000200
#define GL_FRAMEBUFFER_BARRIER_BIT_EXT         0x00000400
#define GL_TRANSFORM_FEEDBACK_BARRIER_BIT_EXT  0x00000800
#define GL_ATOMIC_COUNTER_BARRIER_BIT_EXT      0x00001000
#define GL_ALL_BARRIER_BITS_EXT                0xFFFFFFFF
typedef void (APIENTRYP pfn_glBindImageTextureEXT) (GLuint, GLuint, GLint, GLboolean, GLint, GLenum, GLint);
extern pfn_glBindImageTextureEXT fp_glBindImageTextureEXT;
typedef void (APIENTRYP pfn_glMemoryBarrierEXT) (GLbitfield);
extern pfn_glMemoryBarrierEXT fp_glMemoryBarrierEXT;

 /* GL_EXT_shader_implicit_conversions */
extern GLboolean GLAD_EXT_shader_implicit_conversions;

 /* GL_EXT_shader_integer_mix */
extern GLboolean GLAD_EXT_shader_integer_mix;

 /* GL_EXT_shader_io_blocks */
extern GLboolean GLAD_EXT_shader_io_blocks;

 /* GL_EXT_shader_pixel_local_storage */
extern GLboolean GLAD_EXT_shader_pixel_local_storage;
#define GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_FAST_SIZE_EXT  0x8F63
#define GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_SIZE_EXT  0x8F67
#define GL_SHADER_PIXEL_LOCAL_STORAGE_EXT      0x8F64

 /* GL_EXT_shader_texture_lod */
extern GLboolean GLAD_EXT_shader_texture_lod;

 /* GL_EXT_shadow_samplers */
extern GLboolean GLAD_EXT_shadow_samplers;
#define GL_TEXTURE_COMPARE_MODE_EXT            0x884C
#define GL_TEXTURE_COMPARE_FUNC_EXT            0x884D
#define GL_COMPARE_REF_TO_TEXTURE_EXT          0x884E
#define GL_SAMPLER_2D_SHADOW_EXT               0x8B62

 /* GL_EXT_sparse_texture */
extern GLboolean GLAD_EXT_sparse_texture;
#define GL_TEXTURE_SPARSE_EXT                  0x91A6
#define GL_VIRTUAL_PAGE_SIZE_INDEX_EXT         0x91A7
#define GL_NUM_SPARSE_LEVELS_EXT               0x91AA
#define GL_NUM_VIRTUAL_PAGE_SIZES_EXT          0x91A8
#define GL_VIRTUAL_PAGE_SIZE_X_EXT             0x9195
#define GL_VIRTUAL_PAGE_SIZE_Y_EXT             0x9196
#define GL_VIRTUAL_PAGE_SIZE_Z_EXT             0x9197
#define GL_TEXTURE_CUBE_MAP_ARRAY_OES          0x9009
#define GL_MAX_SPARSE_TEXTURE_SIZE_EXT         0x9198
#define GL_MAX_SPARSE_3D_TEXTURE_SIZE_EXT      0x9199
#define GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS_EXT  0x919A
#define GL_SPARSE_TEXTURE_FULL_ARRAY_CUBE_MIPMAPS_EXT  0x91A9
typedef void (APIENTRYP pfn_glTexPageCommitmentEXT) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLboolean);
extern pfn_glTexPageCommitmentEXT fp_glTexPageCommitmentEXT;

 /* GL_EXT_sparse_texture2 */
extern GLboolean GLAD_EXT_sparse_texture2;

 /* GL_EXT_sRGB */
extern GLboolean GLAD_EXT_sRGB;
#define GL_SRGB_EXT                            0x8C40
#define GL_SRGB_ALPHA_EXT                      0x8C42
#define GL_SRGB8_ALPHA8_EXT                    0x8C43
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING_EXT  0x8210

 /* GL_EXT_sRGB_write_control */
extern GLboolean GLAD_EXT_sRGB_write_control;

 /* GL_EXT_stencil_clear_tag */
extern GLboolean GLAD_EXT_stencil_clear_tag;
#define GL_STENCIL_TAG_BITS_EXT                0x88F2
#define GL_STENCIL_CLEAR_TAG_VALUE_EXT         0x88F3
typedef void (APIENTRYP pfn_glStencilClearTagEXT) (GLsizei, GLuint);
extern pfn_glStencilClearTagEXT fp_glStencilClearTagEXT;

 /* GL_EXT_tessellation_point_size */
extern GLboolean GLAD_EXT_tessellation_point_size;

 /* GL_EXT_tessellation_shader */
extern GLboolean GLAD_EXT_tessellation_shader;
#define GL_PATCHES_EXT                         0x000E
#define GL_PATCH_VERTICES_EXT                  0x8E72
#define GL_TESS_CONTROL_OUTPUT_VERTICES_EXT    0x8E75
#define GL_TESS_GEN_MODE_EXT                   0x8E76
#define GL_TESS_GEN_SPACING_EXT                0x8E77
#define GL_TESS_GEN_VERTEX_ORDER_EXT           0x8E78
#define GL_TESS_GEN_POINT_MODE_EXT             0x8E79
#define GL_ISOLINES_EXT                        0x8E7A
#define GL_QUADS_EXT                           0x0007
#define GL_FRACTIONAL_ODD_EXT                  0x8E7B
#define GL_FRACTIONAL_EVEN_EXT                 0x8E7C
#define GL_MAX_PATCH_VERTICES_EXT              0x8E7D
#define GL_MAX_TESS_GEN_LEVEL_EXT              0x8E7E
#define GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS_EXT  0x8E7F
#define GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS_EXT  0x8E80
#define GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS_EXT  0x8E81
#define GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS_EXT  0x8E82
#define GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS_EXT  0x8E83
#define GL_MAX_TESS_PATCH_COMPONENTS_EXT       0x8E84
#define GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS_EXT  0x8E85
#define GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS_EXT  0x8E86
#define GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS_EXT  0x8E89
#define GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS_EXT  0x8E8A
#define GL_MAX_TESS_CONTROL_INPUT_COMPONENTS_EXT  0x886C
#define GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS_EXT  0x886D
#define GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS_EXT  0x8E1E
#define GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS_EXT  0x8E1F
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS_EXT  0x92CD
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS_EXT  0x92CE
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS_EXT  0x92D3
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS_EXT  0x92D4
#define GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS_EXT  0x90CB
#define GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS_EXT  0x90CC
#define GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS_EXT  0x90D8
#define GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS_EXT  0x90D9
#define GL_IS_PER_PATCH_EXT                    0x92E7
#define GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT  0x9307
#define GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT  0x9308
#define GL_TESS_CONTROL_SHADER_EXT             0x8E88
#define GL_TESS_EVALUATION_SHADER_EXT          0x8E87
#define GL_TESS_CONTROL_SHADER_BIT_EXT         0x00000008
#define GL_TESS_EVALUATION_SHADER_BIT_EXT      0x00000010
typedef void (APIENTRYP pfn_glPatchParameteriEXT) (GLenum, GLint);
extern pfn_glPatchParameteriEXT fp_glPatchParameteriEXT;

 /* GL_EXT_texture_array */
extern GLboolean GLAD_EXT_texture_array;
#define GL_TEXTURE_1D_ARRAY_EXT                0x8C18
#define GL_PROXY_TEXTURE_1D_ARRAY_EXT          0x8C19
#define GL_TEXTURE_2D_ARRAY_EXT                0x8C1A
#define GL_PROXY_TEXTURE_2D_ARRAY_EXT          0x8C1B
#define GL_TEXTURE_BINDING_1D_ARRAY_EXT        0x8C1C
#define GL_TEXTURE_BINDING_2D_ARRAY_EXT        0x8C1D
#define GL_MAX_ARRAY_TEXTURE_LAYERS_EXT        0x88FF
#define GL_COMPARE_REF_DEPTH_TO_TEXTURE_EXT    0x884E
typedef void (APIENTRYP pfn_glFramebufferTextureLayerEXT) (GLenum, GLenum, GLuint, GLint, GLint);
extern pfn_glFramebufferTextureLayerEXT fp_glFramebufferTextureLayerEXT;

 /* GL_EXT_texture_border_clamp */
extern GLboolean GLAD_EXT_texture_border_clamp;
#define GL_TEXTURE_BORDER_COLOR_EXT            0x1004
#define GL_CLAMP_TO_BORDER_EXT                 0x812D
typedef void (APIENTRYP pfn_glTexParameterIivEXT) (GLenum, GLenum, const GLint*);
extern pfn_glTexParameterIivEXT fp_glTexParameterIivEXT;
typedef void (APIENTRYP pfn_glTexParameterIuivEXT) (GLenum, GLenum, const GLuint*);
extern pfn_glTexParameterIuivEXT fp_glTexParameterIuivEXT;
typedef void (APIENTRYP pfn_glGetTexParameterIivEXT) (GLenum, GLenum, GLint*);
extern pfn_glGetTexParameterIivEXT fp_glGetTexParameterIivEXT;
typedef void (APIENTRYP pfn_glGetTexParameterIuivEXT) (GLenum, GLenum, GLuint*);
extern pfn_glGetTexParameterIuivEXT fp_glGetTexParameterIuivEXT;
typedef void (APIENTRYP pfn_glSamplerParameterIivEXT) (GLuint, GLenum, const GLint*);
extern pfn_glSamplerParameterIivEXT fp_glSamplerParameterIivEXT;
typedef void (APIENTRYP pfn_glSamplerParameterIuivEXT) (GLuint, GLenum, const GLuint*);
extern pfn_glSamplerParameterIuivEXT fp_glSamplerParameterIuivEXT;
typedef void (APIENTRYP pfn_glGetSamplerParameterIivEXT) (GLuint, GLenum, GLint*);
extern pfn_glGetSamplerParameterIivEXT fp_glGetSamplerParameterIivEXT;
typedef void (APIENTRYP pfn_glGetSamplerParameterIuivEXT) (GLuint, GLenum, GLuint*);
extern pfn_glGetSamplerParameterIuivEXT fp_glGetSamplerParameterIuivEXT;

 /* GL_EXT_texture_buffer */
extern GLboolean GLAD_EXT_texture_buffer;
#define GL_TEXTURE_BUFFER_EXT                  0x8C2A
#define GL_TEXTURE_BUFFER_BINDING_EXT          0x8C2A
#define GL_MAX_TEXTURE_BUFFER_SIZE_EXT         0x8C2B
#define GL_TEXTURE_BINDING_BUFFER_EXT          0x8C2C
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT  0x8C2D
#define GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT_EXT  0x919F
#define GL_TEXTURE_BUFFER_OFFSET_EXT           0x919D
#define GL_TEXTURE_BUFFER_SIZE_EXT             0x919E
typedef void (APIENTRYP pfn_glTexBufferEXT) (GLenum, GLenum, GLuint);
extern pfn_glTexBufferEXT fp_glTexBufferEXT;
typedef void (APIENTRYP pfn_glTexBufferRangeEXT) (GLenum, GLenum, GLuint, GLintptr, GLsizeiptr);
extern pfn_glTexBufferRangeEXT fp_glTexBufferRangeEXT;

 /* GL_EXT_texture_buffer_object */
extern GLboolean GLAD_EXT_texture_buffer_object;
#define GL_TEXTURE_BUFFER_FORMAT_EXT           0x8C2E

 /* GL_EXT_texture_compression_dxt1 */
extern GLboolean GLAD_EXT_texture_compression_dxt1;
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT        0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT       0x83F1

 /* GL_EXT_texture_compression_latc */
extern GLboolean GLAD_EXT_texture_compression_latc;
#define GL_COMPRESSED_LUMINANCE_LATC1_EXT      0x8C70
#define GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT  0x8C71
#define GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT  0x8C72
#define GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT  0x8C73

 /* GL_EXT_texture_compression_rgtc */
extern GLboolean GLAD_EXT_texture_compression_rgtc;
#define GL_COMPRESSED_RED_RGTC1_EXT            0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1_EXT     0x8DBC
#define GL_COMPRESSED_RED_GREEN_RGTC2_EXT      0x8DBD
#define GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT  0x8DBE

 /* GL_EXT_texture_compression_s3tc */
extern GLboolean GLAD_EXT_texture_compression_s3tc;
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT       0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT       0x83F3

 /* GL_EXT_texture_cube_map */
extern GLboolean GLAD_EXT_texture_cube_map;
#define GL_NORMAL_MAP_EXT                      0x8511
#define GL_REFLECTION_MAP_EXT                  0x8512
#define GL_TEXTURE_CUBE_MAP_EXT                0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_EXT        0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT     0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT     0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT     0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT     0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT     0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT     0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_EXT          0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT       0x851C

 /* GL_EXT_texture_cube_map_array */
extern GLboolean GLAD_EXT_texture_cube_map_array;
#define GL_TEXTURE_CUBE_MAP_ARRAY_EXT          0x9009
#define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY_EXT  0x900A
#define GL_SAMPLER_CUBE_MAP_ARRAY_EXT          0x900C
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_EXT   0x900D
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY_EXT      0x900E
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_EXT  0x900F

 /* GL_EXT_texture_filter_anisotropic */
extern GLboolean GLAD_EXT_texture_filter_anisotropic;
#define GL_TEXTURE_MAX_ANISOTROPY_EXT          0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT      0x84FF

 /* GL_EXT_texture_filter_minmax */
extern GLboolean GLAD_EXT_texture_filter_minmax;

 /* GL_EXT_texture_format_BGRA8888 */
extern GLboolean GLAD_EXT_texture_format_BGRA8888;

 /* GL_EXT_texture_integer */
extern GLboolean GLAD_EXT_texture_integer;
#define GL_RGBA32UI_EXT                        0x8D70
#define GL_RGB32UI_EXT                         0x8D71
#define GL_ALPHA32UI_EXT                       0x8D72
#define GL_INTENSITY32UI_EXT                   0x8D73
#define GL_LUMINANCE32UI_EXT                   0x8D74
#define GL_LUMINANCE_ALPHA32UI_EXT             0x8D75
#define GL_RGBA16UI_EXT                        0x8D76
#define GL_RGB16UI_EXT                         0x8D77
#define GL_ALPHA16UI_EXT                       0x8D78
#define GL_INTENSITY16UI_EXT                   0x8D79
#define GL_LUMINANCE16UI_EXT                   0x8D7A
#define GL_LUMINANCE_ALPHA16UI_EXT             0x8D7B
#define GL_RGBA8UI_EXT                         0x8D7C
#define GL_RGB8UI_EXT                          0x8D7D
#define GL_ALPHA8UI_EXT                        0x8D7E
#define GL_INTENSITY8UI_EXT                    0x8D7F
#define GL_LUMINANCE8UI_EXT                    0x8D80
#define GL_LUMINANCE_ALPHA8UI_EXT              0x8D81
#define GL_RGBA32I_EXT                         0x8D82
#define GL_RGB32I_EXT                          0x8D83
#define GL_ALPHA32I_EXT                        0x8D84
#define GL_INTENSITY32I_EXT                    0x8D85
#define GL_LUMINANCE32I_EXT                    0x8D86
#define GL_LUMINANCE_ALPHA32I_EXT              0x8D87
#define GL_RGBA16I_EXT                         0x8D88
#define GL_RGB16I_EXT                          0x8D89
#define GL_ALPHA16I_EXT                        0x8D8A
#define GL_INTENSITY16I_EXT                    0x8D8B
#define GL_LUMINANCE16I_EXT                    0x8D8C
#define GL_LUMINANCE_ALPHA16I_EXT              0x8D8D
#define GL_RGBA8I_EXT                          0x8D8E
#define GL_RGB8I_EXT                           0x8D8F
#define GL_ALPHA8I_EXT                         0x8D90
#define GL_INTENSITY8I_EXT                     0x8D91
#define GL_LUMINANCE8I_EXT                     0x8D92
#define GL_LUMINANCE_ALPHA8I_EXT               0x8D93
#define GL_RED_INTEGER_EXT                     0x8D94
#define GL_GREEN_INTEGER_EXT                   0x8D95
#define GL_BLUE_INTEGER_EXT                    0x8D96
#define GL_ALPHA_INTEGER_EXT                   0x8D97
#define GL_RGB_INTEGER_EXT                     0x8D98
#define GL_RGBA_INTEGER_EXT                    0x8D99
#define GL_BGR_INTEGER_EXT                     0x8D9A
#define GL_BGRA_INTEGER_EXT                    0x8D9B
#define GL_LUMINANCE_INTEGER_EXT               0x8D9C
#define GL_LUMINANCE_ALPHA_INTEGER_EXT         0x8D9D
#define GL_RGBA_INTEGER_MODE_EXT               0x8D9E
typedef void (APIENTRYP pfn_glClearColorIiEXT) (GLint, GLint, GLint, GLint);
extern pfn_glClearColorIiEXT fp_glClearColorIiEXT;
typedef void (APIENTRYP pfn_glClearColorIuiEXT) (GLuint, GLuint, GLuint, GLuint);
extern pfn_glClearColorIuiEXT fp_glClearColorIuiEXT;

 /* GL_EXT_texture_norm16 */
extern GLboolean GLAD_EXT_texture_norm16;
#define GL_R16_EXT                             0x822A
#define GL_RG16_EXT                            0x822C
#define GL_RGBA16_EXT                          0x805B
#define GL_RGB16_EXT                           0x8054
#define GL_RGB16_SNORM_EXT                     0x8F9A

 /* GL_EXT_texture_rg */
extern GLboolean GLAD_EXT_texture_rg;
#define GL_RED_EXT                             0x1903
#define GL_RG_EXT                              0x8227
#define GL_R8_EXT                              0x8229
#define GL_RG8_EXT                             0x822B

 /* GL_EXT_texture_shared_exponent */
extern GLboolean GLAD_EXT_texture_shared_exponent;
#define GL_RGB9_E5_EXT                         0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV_EXT        0x8C3E
#define GL_TEXTURE_SHARED_SIZE_EXT             0x8C3F

 /* GL_EXT_texture_snorm */
extern GLboolean GLAD_EXT_texture_snorm;
#define GL_ALPHA_SNORM                         0x9010
#define GL_LUMINANCE_SNORM                     0x9011
#define GL_LUMINANCE_ALPHA_SNORM               0x9012
#define GL_INTENSITY_SNORM                     0x9013
#define GL_ALPHA8_SNORM                        0x9014
#define GL_LUMINANCE8_SNORM                    0x9015
#define GL_LUMINANCE8_ALPHA8_SNORM             0x9016
#define GL_INTENSITY8_SNORM                    0x9017
#define GL_ALPHA16_SNORM                       0x9018
#define GL_LUMINANCE16_SNORM                   0x9019
#define GL_LUMINANCE16_ALPHA16_SNORM           0x901A
#define GL_INTENSITY16_SNORM                   0x901B
#define GL_RED_SNORM                           0x8F90
#define GL_RG_SNORM                            0x8F91
#define GL_RGB_SNORM                           0x8F92
#define GL_RGBA_SNORM                          0x8F93

 /* GL_EXT_texture_sRGB */
extern GLboolean GLAD_EXT_texture_sRGB;
#define GL_SRGB8_EXT                           0x8C41
#define GL_SLUMINANCE_ALPHA_EXT                0x8C44
#define GL_SLUMINANCE8_ALPHA8_EXT              0x8C45
#define GL_SLUMINANCE_EXT                      0x8C46
#define GL_SLUMINANCE8_EXT                     0x8C47
#define GL_COMPRESSED_SRGB_EXT                 0x8C48
#define GL_COMPRESSED_SRGB_ALPHA_EXT           0x8C49
#define GL_COMPRESSED_SLUMINANCE_EXT           0x8C4A
#define GL_COMPRESSED_SLUMINANCE_ALPHA_EXT     0x8C4B
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT       0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT  0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT  0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT  0x8C4F

 /* GL_EXT_texture_sRGB_decode */
extern GLboolean GLAD_EXT_texture_sRGB_decode;
#define GL_TEXTURE_SRGB_DECODE_EXT             0x8A48
#define GL_DECODE_EXT                          0x8A49
#define GL_SKIP_DECODE_EXT                     0x8A4A

 /* GL_EXT_texture_sRGB_R8 */
extern GLboolean GLAD_EXT_texture_sRGB_R8;
#define GL_SR8_EXT                             0x8FBD

 /* GL_EXT_texture_sRGB_RG8 */
extern GLboolean GLAD_EXT_texture_sRGB_RG8;
#define GL_SRG8_EXT                            0x8FBE

 /* GL_EXT_texture_storage */
extern GLboolean GLAD_EXT_texture_storage;
#define GL_TEXTURE_IMMUTABLE_FORMAT_EXT        0x912F
#define GL_ALPHA8_EXT                          0x803C
#define GL_LUMINANCE8_EXT                      0x8040
#define GL_LUMINANCE8_ALPHA8_EXT               0x8045
#define GL_RGBA32F_EXT                         0x8814
#define GL_RGB32F_EXT                          0x8815
#define GL_ALPHA32F_EXT                        0x8816
#define GL_LUMINANCE32F_EXT                    0x8818
#define GL_LUMINANCE_ALPHA32F_EXT              0x8819
#define GL_ALPHA16F_EXT                        0x881C
#define GL_LUMINANCE16F_EXT                    0x881E
#define GL_LUMINANCE_ALPHA16F_EXT              0x881F
#define GL_RGB10_A2_EXT                        0x8059
#define GL_RGB10_EXT                           0x8052
#define GL_BGRA8_EXT                           0x93A1
#define GL_R32F_EXT                            0x822E
#define GL_RG32F_EXT                           0x8230
typedef void (APIENTRYP pfn_glTexStorage1DEXT) (GLenum, GLsizei, GLenum, GLsizei);
extern pfn_glTexStorage1DEXT fp_glTexStorage1DEXT;
typedef void (APIENTRYP pfn_glTexStorage2DEXT) (GLenum, GLsizei, GLenum, GLsizei, GLsizei);
extern pfn_glTexStorage2DEXT fp_glTexStorage2DEXT;
typedef void (APIENTRYP pfn_glTexStorage3DEXT) (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei);
extern pfn_glTexStorage3DEXT fp_glTexStorage3DEXT;

 /* GL_EXT_texture_swizzle */
extern GLboolean GLAD_EXT_texture_swizzle;
#define GL_TEXTURE_SWIZZLE_R_EXT               0x8E42
#define GL_TEXTURE_SWIZZLE_G_EXT               0x8E43
#define GL_TEXTURE_SWIZZLE_B_EXT               0x8E44
#define GL_TEXTURE_SWIZZLE_A_EXT               0x8E45
#define GL_TEXTURE_SWIZZLE_RGBA_EXT            0x8E46

 /* GL_EXT_texture_type_2_10_10_10_REV */
extern GLboolean GLAD_EXT_texture_type_2_10_10_10_REV;
#define GL_UNSIGNED_INT_2_10_10_10_REV_EXT     0x8368

 /* GL_EXT_texture_view */
extern GLboolean GLAD_EXT_texture_view;
#define GL_TEXTURE_VIEW_MIN_LEVEL_EXT          0x82DB
#define GL_TEXTURE_VIEW_NUM_LEVELS_EXT         0x82DC
#define GL_TEXTURE_VIEW_MIN_LAYER_EXT          0x82DD
#define GL_TEXTURE_VIEW_NUM_LAYERS_EXT         0x82DE
typedef void (APIENTRYP pfn_glTextureViewEXT) (GLuint, GLenum, GLuint, GLenum, GLuint, GLuint, GLuint, GLuint);
extern pfn_glTextureViewEXT fp_glTextureViewEXT;

 /* GL_EXT_timer_query */
extern GLboolean GLAD_EXT_timer_query;

 /* GL_EXT_transform_feedback */
extern GLboolean GLAD_EXT_transform_feedback;
#define GL_TRANSFORM_FEEDBACK_BUFFER_EXT       0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_START_EXT  0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE_EXT  0x8C85
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING_EXT  0x8C8F
#define GL_INTERLEAVED_ATTRIBS_EXT             0x8C8C
#define GL_SEPARATE_ATTRIBS_EXT                0x8C8D
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_EXT  0x8C88
#define GL_RASTERIZER_DISCARD_EXT              0x8C89
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_EXT  0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_EXT  0x8C8B
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_EXT  0x8C80
#define GL_TRANSFORM_FEEDBACK_VARYINGS_EXT     0x8C83
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE_EXT  0x8C7F
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH_EXT  0x8C76
typedef void (APIENTRYP pfn_glBeginTransformFeedbackEXT) (GLenum);
extern pfn_glBeginTransformFeedbackEXT fp_glBeginTransformFeedbackEXT;
typedef void (APIENTRYP pfn_glEndTransformFeedbackEXT) ();
extern pfn_glEndTransformFeedbackEXT fp_glEndTransformFeedbackEXT;
typedef void (APIENTRYP pfn_glBindBufferRangeEXT) (GLenum, GLuint, GLuint, GLintptr, GLsizeiptr);
extern pfn_glBindBufferRangeEXT fp_glBindBufferRangeEXT;
typedef void (APIENTRYP pfn_glBindBufferOffsetEXT) (GLenum, GLuint, GLuint, GLintptr);
extern pfn_glBindBufferOffsetEXT fp_glBindBufferOffsetEXT;
typedef void (APIENTRYP pfn_glBindBufferBaseEXT) (GLenum, GLuint, GLuint);
extern pfn_glBindBufferBaseEXT fp_glBindBufferBaseEXT;
typedef void (APIENTRYP pfn_glTransformFeedbackVaryingsEXT) (GLuint, GLsizei, const GLchar**, GLenum);
extern pfn_glTransformFeedbackVaryingsEXT fp_glTransformFeedbackVaryingsEXT;
typedef void (APIENTRYP pfn_glGetTransformFeedbackVaryingEXT) (GLuint, GLuint, GLsizei, GLsizei*, GLsizei*, GLenum*, GLchar*);
extern pfn_glGetTransformFeedbackVaryingEXT fp_glGetTransformFeedbackVaryingEXT;

 /* GL_EXT_unpack_subimage */
extern GLboolean GLAD_EXT_unpack_subimage;
#define GL_UNPACK_ROW_LENGTH_EXT               0x0CF2
#define GL_UNPACK_SKIP_ROWS_EXT                0x0CF3
#define GL_UNPACK_SKIP_PIXELS_EXT              0x0CF4

 /* GL_EXT_vertex_array_bgra */
extern GLboolean GLAD_EXT_vertex_array_bgra;

 /* GL_EXT_vertex_attrib_64bit */
extern GLboolean GLAD_EXT_vertex_attrib_64bit;
#define GL_DOUBLE_VEC2_EXT                     0x8FFC
#define GL_DOUBLE_VEC3_EXT                     0x8FFD
#define GL_DOUBLE_VEC4_EXT                     0x8FFE
#define GL_DOUBLE_MAT2_EXT                     0x8F46
#define GL_DOUBLE_MAT3_EXT                     0x8F47
#define GL_DOUBLE_MAT4_EXT                     0x8F48
#define GL_DOUBLE_MAT2x3_EXT                   0x8F49
#define GL_DOUBLE_MAT2x4_EXT                   0x8F4A
#define GL_DOUBLE_MAT3x2_EXT                   0x8F4B
#define GL_DOUBLE_MAT3x4_EXT                   0x8F4C
#define GL_DOUBLE_MAT4x2_EXT                   0x8F4D
#define GL_DOUBLE_MAT4x3_EXT                   0x8F4E
typedef void (APIENTRYP pfn_glVertexAttribL1dEXT) (GLuint, GLdouble);
extern pfn_glVertexAttribL1dEXT fp_glVertexAttribL1dEXT;
typedef void (APIENTRYP pfn_glVertexAttribL2dEXT) (GLuint, GLdouble, GLdouble);
extern pfn_glVertexAttribL2dEXT fp_glVertexAttribL2dEXT;
typedef void (APIENTRYP pfn_glVertexAttribL3dEXT) (GLuint, GLdouble, GLdouble, GLdouble);
extern pfn_glVertexAttribL3dEXT fp_glVertexAttribL3dEXT;
typedef void (APIENTRYP pfn_glVertexAttribL4dEXT) (GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
extern pfn_glVertexAttribL4dEXT fp_glVertexAttribL4dEXT;
typedef void (APIENTRYP pfn_glVertexAttribL1dvEXT) (GLuint, const GLdouble*);
extern pfn_glVertexAttribL1dvEXT fp_glVertexAttribL1dvEXT;
typedef void (APIENTRYP pfn_glVertexAttribL2dvEXT) (GLuint, const GLdouble*);
extern pfn_glVertexAttribL2dvEXT fp_glVertexAttribL2dvEXT;
typedef void (APIENTRYP pfn_glVertexAttribL3dvEXT) (GLuint, const GLdouble*);
extern pfn_glVertexAttribL3dvEXT fp_glVertexAttribL3dvEXT;
typedef void (APIENTRYP pfn_glVertexAttribL4dvEXT) (GLuint, const GLdouble*);
extern pfn_glVertexAttribL4dvEXT fp_glVertexAttribL4dvEXT;
typedef void (APIENTRYP pfn_glVertexAttribLPointerEXT) (GLuint, GLint, GLenum, GLsizei, const void*);
extern pfn_glVertexAttribLPointerEXT fp_glVertexAttribLPointerEXT;
typedef void (APIENTRYP pfn_glGetVertexAttribLdvEXT) (GLuint, GLenum, GLdouble*);
extern pfn_glGetVertexAttribLdvEXT fp_glGetVertexAttribLdvEXT;

 /* GL_EXT_x11_sync_object */
extern GLboolean GLAD_EXT_x11_sync_object;
#define GL_SYNC_X11_FENCE_EXT                  0x90E1
typedef GLsync (APIENTRYP pfn_glImportSyncEXT) (GLenum, GLintptr, GLbitfield);
extern pfn_glImportSyncEXT fp_glImportSyncEXT;

 /* GL_EXT_YUV_target */
extern GLboolean GLAD_EXT_YUV_target;
#define GL_SAMPLER_EXTERNAL_2D_Y2Y_EXT         0x8BE7
#define GL_TEXTURE_EXTERNAL_OES                0x8D65
#define GL_TEXTURE_BINDING_EXTERNAL_OES        0x8D67
#define GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES    0x8D68

 /* GL_OES_compressed_ETC1_RGB8_sub_texture */
extern GLboolean GLAD_OES_compressed_ETC1_RGB8_sub_texture;

 /* GL_OES_compressed_ETC1_RGB8_texture */
extern GLboolean GLAD_OES_compressed_ETC1_RGB8_texture;
#define GL_ETC1_RGB8_OES                       0x8D64

 /* GL_OES_copy_image */
extern GLboolean GLAD_OES_copy_image;
typedef void (APIENTRYP pfn_glCopyImageSubDataOES) (GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei);
extern pfn_glCopyImageSubDataOES fp_glCopyImageSubDataOES;

 /* GL_OES_depth24 */
extern GLboolean GLAD_OES_depth24;
#define GL_DEPTH_COMPONENT24_OES               0x81A6

 /* GL_OES_depth32 */
extern GLboolean GLAD_OES_depth32;
#define GL_DEPTH_COMPONENT32_OES               0x81A7

 /* GL_OES_depth_texture */
extern GLboolean GLAD_OES_depth_texture;

 /* GL_OES_draw_buffers_indexed */
extern GLboolean GLAD_OES_draw_buffers_indexed;
typedef void (APIENTRYP pfn_glEnableiOES) (GLenum, GLuint);
extern pfn_glEnableiOES fp_glEnableiOES;
typedef void (APIENTRYP pfn_glDisableiOES) (GLenum, GLuint);
extern pfn_glDisableiOES fp_glDisableiOES;
typedef void (APIENTRYP pfn_glBlendEquationiOES) (GLuint, GLenum);
extern pfn_glBlendEquationiOES fp_glBlendEquationiOES;
typedef void (APIENTRYP pfn_glBlendEquationSeparateiOES) (GLuint, GLenum, GLenum);
extern pfn_glBlendEquationSeparateiOES fp_glBlendEquationSeparateiOES;
typedef void (APIENTRYP pfn_glBlendFunciOES) (GLuint, GLenum, GLenum);
extern pfn_glBlendFunciOES fp_glBlendFunciOES;
typedef void (APIENTRYP pfn_glBlendFuncSeparateiOES) (GLuint, GLenum, GLenum, GLenum, GLenum);
extern pfn_glBlendFuncSeparateiOES fp_glBlendFuncSeparateiOES;
typedef void (APIENTRYP pfn_glColorMaskiOES) (GLuint, GLboolean, GLboolean, GLboolean, GLboolean);
extern pfn_glColorMaskiOES fp_glColorMaskiOES;
typedef GLboolean (APIENTRYP pfn_glIsEnablediOES) (GLenum, GLuint);
extern pfn_glIsEnablediOES fp_glIsEnablediOES;

 /* GL_OES_draw_elements_base_vertex */
extern GLboolean GLAD_OES_draw_elements_base_vertex;
typedef void (APIENTRYP pfn_glDrawElementsBaseVertexOES) (GLenum, GLsizei, GLenum, const void*, GLint);
extern pfn_glDrawElementsBaseVertexOES fp_glDrawElementsBaseVertexOES;
typedef void (APIENTRYP pfn_glDrawRangeElementsBaseVertexOES) (GLenum, GLuint, GLuint, GLsizei, GLenum, const void*, GLint);
extern pfn_glDrawRangeElementsBaseVertexOES fp_glDrawRangeElementsBaseVertexOES;
typedef void (APIENTRYP pfn_glDrawElementsInstancedBaseVertexOES) (GLenum, GLsizei, GLenum, const void*, GLsizei, GLint);
extern pfn_glDrawElementsInstancedBaseVertexOES fp_glDrawElementsInstancedBaseVertexOES;
typedef void (APIENTRYP pfn_glMultiDrawElementsBaseVertexOES) (GLenum, const GLsizei*, GLenum, const void**, GLsizei, const GLint*);
extern pfn_glMultiDrawElementsBaseVertexOES fp_glMultiDrawElementsBaseVertexOES;

 /* GL_OES_EGL_image */
extern GLboolean GLAD_OES_EGL_image;
typedef void (APIENTRYP pfn_glEGLImageTargetTexture2DOES) (GLenum, GLeglImageOES);
extern pfn_glEGLImageTargetTexture2DOES fp_glEGLImageTargetTexture2DOES;
typedef void (APIENTRYP pfn_glEGLImageTargetRenderbufferStorageOES) (GLenum, GLeglImageOES);
extern pfn_glEGLImageTargetRenderbufferStorageOES fp_glEGLImageTargetRenderbufferStorageOES;

 /* GL_OES_EGL_image_external */
extern GLboolean GLAD_OES_EGL_image_external;
#define GL_SAMPLER_EXTERNAL_OES                0x8D66

 /* GL_OES_EGL_image_external_essl3 */
extern GLboolean GLAD_OES_EGL_image_external_essl3;

 /* GL_OES_element_index_uint */
extern GLboolean GLAD_OES_element_index_uint;

 /* GL_OES_fbo_render_mipmap */
extern GLboolean GLAD_OES_fbo_render_mipmap;

 /* GL_OES_fragment_precision_high */
extern GLboolean GLAD_OES_fragment_precision_high;

 /* GL_OES_geometry_point_size */
extern GLboolean GLAD_OES_geometry_point_size;

 /* GL_OES_geometry_shader */
extern GLboolean GLAD_OES_geometry_shader;
#define GL_GEOMETRY_SHADER_OES                 0x8DD9
#define GL_GEOMETRY_SHADER_BIT_OES             0x00000004
#define GL_GEOMETRY_LINKED_VERTICES_OUT_OES    0x8916
#define GL_GEOMETRY_LINKED_INPUT_TYPE_OES      0x8917
#define GL_GEOMETRY_LINKED_OUTPUT_TYPE_OES     0x8918
#define GL_GEOMETRY_SHADER_INVOCATIONS_OES     0x887F
#define GL_LAYER_PROVOKING_VERTEX_OES          0x825E
#define GL_LINES_ADJACENCY_OES                 0x000A
#define GL_LINE_STRIP_ADJACENCY_OES            0x000B
#define GL_TRIANGLES_ADJACENCY_OES             0x000C
#define GL_TRIANGLE_STRIP_ADJACENCY_OES        0x000D
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_OES  0x8DDF
#define GL_MAX_GEOMETRY_UNIFORM_BLOCKS_OES     0x8A2C
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS_OES  0x8A32
#define GL_MAX_GEOMETRY_INPUT_COMPONENTS_OES   0x9123
#define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS_OES  0x9124
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES_OES    0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_OES  0x8DE1
#define GL_MAX_GEOMETRY_SHADER_INVOCATIONS_OES  0x8E5A
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_OES  0x8C29
#define GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS_OES  0x92CF
#define GL_MAX_GEOMETRY_ATOMIC_COUNTERS_OES    0x92D5
#define GL_MAX_GEOMETRY_IMAGE_UNIFORMS_OES     0x90CD
#define GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_OES  0x90D7
#define GL_FIRST_VERTEX_CONVENTION_OES         0x8E4D
#define GL_LAST_VERTEX_CONVENTION_OES          0x8E4E
#define GL_UNDEFINED_VERTEX_OES                0x8260
#define GL_PRIMITIVES_GENERATED_OES            0x8C87
#define GL_FRAMEBUFFER_DEFAULT_LAYERS_OES      0x9312
#define GL_MAX_FRAMEBUFFER_LAYERS_OES          0x9317
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_OES  0x8DA8
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED_OES  0x8DA7
#define GL_REFERENCED_BY_GEOMETRY_SHADER_OES   0x9309
typedef void (APIENTRYP pfn_glFramebufferTextureOES) (GLenum, GLenum, GLuint, GLint);
extern pfn_glFramebufferTextureOES fp_glFramebufferTextureOES;

 /* GL_OES_get_program_binary */
extern GLboolean GLAD_OES_get_program_binary;
#define GL_PROGRAM_BINARY_LENGTH_OES           0x8741
#define GL_NUM_PROGRAM_BINARY_FORMATS_OES      0x87FE
#define GL_PROGRAM_BINARY_FORMATS_OES          0x87FF
typedef void (APIENTRYP pfn_glGetProgramBinaryOES) (GLuint, GLsizei, GLsizei*, GLenum*, void*);
extern pfn_glGetProgramBinaryOES fp_glGetProgramBinaryOES;
typedef void (APIENTRYP pfn_glProgramBinaryOES) (GLuint, GLenum, const void*, GLint);
extern pfn_glProgramBinaryOES fp_glProgramBinaryOES;

 /* GL_OES_gpu_shader5 */
extern GLboolean GLAD_OES_gpu_shader5;

 /* GL_OES_mapbuffer */
extern GLboolean GLAD_OES_mapbuffer;
#define GL_WRITE_ONLY_OES                      0x88B9
#define GL_BUFFER_ACCESS_OES                   0x88BB
#define GL_BUFFER_MAPPED_OES                   0x88BC
#define GL_BUFFER_MAP_POINTER_OES              0x88BD
typedef void* (APIENTRYP pfn_glMapBufferOES) (GLenum, GLenum);
extern pfn_glMapBufferOES fp_glMapBufferOES;
typedef GLboolean (APIENTRYP pfn_glUnmapBufferOES) (GLenum);
extern pfn_glUnmapBufferOES fp_glUnmapBufferOES;
typedef void (APIENTRYP pfn_glGetBufferPointervOES) (GLenum, GLenum, void**);
extern pfn_glGetBufferPointervOES fp_glGetBufferPointervOES;

 /* GL_OES_packed_depth_stencil */
extern GLboolean GLAD_OES_packed_depth_stencil;
#define GL_DEPTH_STENCIL_OES                   0x84F9
#define GL_UNSIGNED_INT_24_8_OES               0x84FA
#define GL_DEPTH24_STENCIL8_OES                0x88F0

 /* GL_OES_primitive_bounding_box */
extern GLboolean GLAD_OES_primitive_bounding_box;
#define GL_PRIMITIVE_BOUNDING_BOX_OES          0x92BE
typedef void (APIENTRYP pfn_glPrimitiveBoundingBoxOES) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glPrimitiveBoundingBoxOES fp_glPrimitiveBoundingBoxOES;

 /* GL_OES_required_internalformat */
extern GLboolean GLAD_OES_required_internalformat;
#define GL_ALPHA8_OES                          0x803C
#define GL_DEPTH_COMPONENT16_OES               0x81A5
#define GL_LUMINANCE4_ALPHA4_OES               0x8043
#define GL_LUMINANCE8_ALPHA8_OES               0x8045
#define GL_LUMINANCE8_OES                      0x8040
#define GL_RGBA4_OES                           0x8056
#define GL_RGB5_A1_OES                         0x8057
#define GL_RGB565_OES                          0x8D62
#define GL_RGB8_OES                            0x8051
#define GL_RGBA8_OES                           0x8058

 /* GL_OES_rgb8_rgba8 */
extern GLboolean GLAD_OES_rgb8_rgba8;

 /* GL_OES_sample_shading */
extern GLboolean GLAD_OES_sample_shading;
#define GL_SAMPLE_SHADING_OES                  0x8C36
#define GL_MIN_SAMPLE_SHADING_VALUE_OES        0x8C37
typedef void (APIENTRYP pfn_glMinSampleShadingOES) (GLfloat);
extern pfn_glMinSampleShadingOES fp_glMinSampleShadingOES;

 /* GL_OES_sample_variables */
extern GLboolean GLAD_OES_sample_variables;

 /* GL_OES_shader_image_atomic */
extern GLboolean GLAD_OES_shader_image_atomic;

 /* GL_OES_shader_io_blocks */
extern GLboolean GLAD_OES_shader_io_blocks;

 /* GL_OES_shader_multisample_interpolation */
extern GLboolean GLAD_OES_shader_multisample_interpolation;
#define GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_OES  0x8E5B
#define GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_OES  0x8E5C
#define GL_FRAGMENT_INTERPOLATION_OFFSET_BITS_OES  0x8E5D

 /* GL_OES_standard_derivatives */
extern GLboolean GLAD_OES_standard_derivatives;
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES  0x8B8B

 /* GL_OES_stencil1 */
extern GLboolean GLAD_OES_stencil1;
#define GL_STENCIL_INDEX1_OES                  0x8D46

 /* GL_OES_stencil4 */
extern GLboolean GLAD_OES_stencil4;
#define GL_STENCIL_INDEX4_OES                  0x8D47

 /* GL_OES_surfaceless_context */
extern GLboolean GLAD_OES_surfaceless_context;
#define GL_FRAMEBUFFER_UNDEFINED_OES           0x8219

 /* GL_OES_tessellation_point_size */
extern GLboolean GLAD_OES_tessellation_point_size;

 /* GL_OES_tessellation_shader */
extern GLboolean GLAD_OES_tessellation_shader;
#define GL_PATCHES_OES                         0x000E
#define GL_PATCH_VERTICES_OES                  0x8E72
#define GL_TESS_CONTROL_OUTPUT_VERTICES_OES    0x8E75
#define GL_TESS_GEN_MODE_OES                   0x8E76
#define GL_TESS_GEN_SPACING_OES                0x8E77
#define GL_TESS_GEN_VERTEX_ORDER_OES           0x8E78
#define GL_TESS_GEN_POINT_MODE_OES             0x8E79
#define GL_ISOLINES_OES                        0x8E7A
#define GL_QUADS_OES                           0x0007
#define GL_FRACTIONAL_ODD_OES                  0x8E7B
#define GL_FRACTIONAL_EVEN_OES                 0x8E7C
#define GL_MAX_PATCH_VERTICES_OES              0x8E7D
#define GL_MAX_TESS_GEN_LEVEL_OES              0x8E7E
#define GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS_OES  0x8E7F
#define GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS_OES  0x8E80
#define GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS_OES  0x8E81
#define GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS_OES  0x8E82
#define GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS_OES  0x8E83
#define GL_MAX_TESS_PATCH_COMPONENTS_OES       0x8E84
#define GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS_OES  0x8E85
#define GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS_OES  0x8E86
#define GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS_OES  0x8E89
#define GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS_OES  0x8E8A
#define GL_MAX_TESS_CONTROL_INPUT_COMPONENTS_OES  0x886C
#define GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS_OES  0x886D
#define GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS_OES  0x8E1E
#define GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS_OES  0x8E1F
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS_OES  0x92CD
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS_OES  0x92CE
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS_OES  0x92D3
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS_OES  0x92D4
#define GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS_OES  0x90CB
#define GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS_OES  0x90CC
#define GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS_OES  0x90D8
#define GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS_OES  0x90D9
#define GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED_OES  0x8221
#define GL_IS_PER_PATCH_OES                    0x92E7
#define GL_REFERENCED_BY_TESS_CONTROL_SHADER_OES  0x9307
#define GL_REFERENCED_BY_TESS_EVALUATION_SHADER_OES  0x9308
#define GL_TESS_CONTROL_SHADER_OES             0x8E88
#define GL_TESS_EVALUATION_SHADER_OES          0x8E87
#define GL_TESS_CONTROL_SHADER_BIT_OES         0x00000008
#define GL_TESS_EVALUATION_SHADER_BIT_OES      0x00000010
typedef void (APIENTRYP pfn_glPatchParameteriOES) (GLenum, GLint);
extern pfn_glPatchParameteriOES fp_glPatchParameteriOES;

 /* GL_OES_texture_3D */
extern GLboolean GLAD_OES_texture_3D;
#define GL_TEXTURE_WRAP_R_OES                  0x8072
#define GL_TEXTURE_3D_OES                      0x806F
#define GL_TEXTURE_BINDING_3D_OES              0x806A
#define GL_MAX_3D_TEXTURE_SIZE_OES             0x8073
#define GL_SAMPLER_3D_OES                      0x8B5F
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_OES  0x8CD4
typedef void (APIENTRYP pfn_glTexImage3DOES) (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*);
extern pfn_glTexImage3DOES fp_glTexImage3DOES;
typedef void (APIENTRYP pfn_glTexSubImage3DOES) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*);
extern pfn_glTexSubImage3DOES fp_glTexSubImage3DOES;
typedef void (APIENTRYP pfn_glCopyTexSubImage3DOES) (GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
extern pfn_glCopyTexSubImage3DOES fp_glCopyTexSubImage3DOES;
typedef void (APIENTRYP pfn_glCompressedTexImage3DOES) (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const void*);
extern pfn_glCompressedTexImage3DOES fp_glCompressedTexImage3DOES;
typedef void (APIENTRYP pfn_glCompressedTexSubImage3DOES) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void*);
extern pfn_glCompressedTexSubImage3DOES fp_glCompressedTexSubImage3DOES;
typedef void (APIENTRYP pfn_glFramebufferTexture3DOES) (GLenum, GLenum, GLenum, GLuint, GLint, GLint);
extern pfn_glFramebufferTexture3DOES fp_glFramebufferTexture3DOES;

 /* GL_OES_texture_border_clamp */
extern GLboolean GLAD_OES_texture_border_clamp;
#define GL_TEXTURE_BORDER_COLOR_OES            0x1004
#define GL_CLAMP_TO_BORDER_OES                 0x812D
typedef void (APIENTRYP pfn_glTexParameterIivOES) (GLenum, GLenum, const GLint*);
extern pfn_glTexParameterIivOES fp_glTexParameterIivOES;
typedef void (APIENTRYP pfn_glTexParameterIuivOES) (GLenum, GLenum, const GLuint*);
extern pfn_glTexParameterIuivOES fp_glTexParameterIuivOES;
typedef void (APIENTRYP pfn_glGetTexParameterIivOES) (GLenum, GLenum, GLint*);
extern pfn_glGetTexParameterIivOES fp_glGetTexParameterIivOES;
typedef void (APIENTRYP pfn_glGetTexParameterIuivOES) (GLenum, GLenum, GLuint*);
extern pfn_glGetTexParameterIuivOES fp_glGetTexParameterIuivOES;
typedef void (APIENTRYP pfn_glSamplerParameterIivOES) (GLuint, GLenum, const GLint*);
extern pfn_glSamplerParameterIivOES fp_glSamplerParameterIivOES;
typedef void (APIENTRYP pfn_glSamplerParameterIuivOES) (GLuint, GLenum, const GLuint*);
extern pfn_glSamplerParameterIuivOES fp_glSamplerParameterIuivOES;
typedef void (APIENTRYP pfn_glGetSamplerParameterIivOES) (GLuint, GLenum, GLint*);
extern pfn_glGetSamplerParameterIivOES fp_glGetSamplerParameterIivOES;
typedef void (APIENTRYP pfn_glGetSamplerParameterIuivOES) (GLuint, GLenum, GLuint*);
extern pfn_glGetSamplerParameterIuivOES fp_glGetSamplerParameterIuivOES;

 /* GL_OES_texture_buffer */
extern GLboolean GLAD_OES_texture_buffer;
#define GL_TEXTURE_BUFFER_OES                  0x8C2A
#define GL_TEXTURE_BUFFER_BINDING_OES          0x8C2A
#define GL_MAX_TEXTURE_BUFFER_SIZE_OES         0x8C2B
#define GL_TEXTURE_BINDING_BUFFER_OES          0x8C2C
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING_OES  0x8C2D
#define GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT_OES  0x919F
#define GL_SAMPLER_BUFFER_OES                  0x8DC2
#define GL_INT_SAMPLER_BUFFER_OES              0x8DD0
#define GL_UNSIGNED_INT_SAMPLER_BUFFER_OES     0x8DD8
#define GL_IMAGE_BUFFER_OES                    0x9051
#define GL_INT_IMAGE_BUFFER_OES                0x905C
#define GL_UNSIGNED_INT_IMAGE_BUFFER_OES       0x9067
#define GL_TEXTURE_BUFFER_OFFSET_OES           0x919D
#define GL_TEXTURE_BUFFER_SIZE_OES             0x919E
typedef void (APIENTRYP pfn_glTexBufferOES) (GLenum, GLenum, GLuint);
extern pfn_glTexBufferOES fp_glTexBufferOES;
typedef void (APIENTRYP pfn_glTexBufferRangeOES) (GLenum, GLenum, GLuint, GLintptr, GLsizeiptr);
extern pfn_glTexBufferRangeOES fp_glTexBufferRangeOES;

 /* GL_OES_texture_compression_astc */
extern GLboolean GLAD_OES_texture_compression_astc;
#define GL_COMPRESSED_RGBA_ASTC_3x3x3_OES      0x93C0
#define GL_COMPRESSED_RGBA_ASTC_4x3x3_OES      0x93C1
#define GL_COMPRESSED_RGBA_ASTC_4x4x3_OES      0x93C2
#define GL_COMPRESSED_RGBA_ASTC_4x4x4_OES      0x93C3
#define GL_COMPRESSED_RGBA_ASTC_5x4x4_OES      0x93C4
#define GL_COMPRESSED_RGBA_ASTC_5x5x4_OES      0x93C5
#define GL_COMPRESSED_RGBA_ASTC_5x5x5_OES      0x93C6
#define GL_COMPRESSED_RGBA_ASTC_6x5x5_OES      0x93C7
#define GL_COMPRESSED_RGBA_ASTC_6x6x5_OES      0x93C8
#define GL_COMPRESSED_RGBA_ASTC_6x6x6_OES      0x93C9
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES  0x93E0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES  0x93E1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES  0x93E2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES  0x93E3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES  0x93E4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES  0x93E5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES  0x93E6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES  0x93E7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES  0x93E8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES  0x93E9

 /* GL_OES_texture_cube_map_array */
extern GLboolean GLAD_OES_texture_cube_map_array;
#define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY_OES  0x900A
#define GL_SAMPLER_CUBE_MAP_ARRAY_OES          0x900C
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_OES   0x900D
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY_OES      0x900E
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_OES  0x900F
#define GL_IMAGE_CUBE_MAP_ARRAY_OES            0x9054
#define GL_INT_IMAGE_CUBE_MAP_ARRAY_OES        0x905F
#define GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY_OES  0x906A

 /* GL_OES_texture_float */
extern GLboolean GLAD_OES_texture_float;

 /* GL_OES_texture_float_linear */
extern GLboolean GLAD_OES_texture_float_linear;

 /* GL_OES_texture_half_float */
extern GLboolean GLAD_OES_texture_half_float;
#define GL_HALF_FLOAT_OES                      0x8D61

 /* GL_OES_texture_half_float_linear */
extern GLboolean GLAD_OES_texture_half_float_linear;

 /* GL_OES_texture_npot */
extern GLboolean GLAD_OES_texture_npot;

 /* GL_OES_texture_stencil8 */
extern GLboolean GLAD_OES_texture_stencil8;
#define GL_STENCIL_INDEX_OES                   0x1901
#define GL_STENCIL_INDEX8_OES                  0x8D48

 /* GL_OES_texture_storage_multisample_2d_array */
extern GLboolean GLAD_OES_texture_storage_multisample_2d_array;
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES    0x9102
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY_OES  0x9105
#define GL_SAMPLER_2D_MULTISAMPLE_ARRAY_OES    0x910B
#define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY_OES  0x910C
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY_OES  0x910D
typedef void (APIENTRYP pfn_glTexStorage3DMultisampleOES) (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean);
extern pfn_glTexStorage3DMultisampleOES fp_glTexStorage3DMultisampleOES;

 /* GL_OES_texture_view */
extern GLboolean GLAD_OES_texture_view;
#define GL_TEXTURE_VIEW_MIN_LEVEL_OES          0x82DB
#define GL_TEXTURE_VIEW_NUM_LEVELS_OES         0x82DC
#define GL_TEXTURE_VIEW_MIN_LAYER_OES          0x82DD
#define GL_TEXTURE_VIEW_NUM_LAYERS_OES         0x82DE
typedef void (APIENTRYP pfn_glTextureViewOES) (GLuint, GLenum, GLuint, GLenum, GLuint, GLuint, GLuint, GLuint);
extern pfn_glTextureViewOES fp_glTextureViewOES;

 /* GL_OES_vertex_array_object */
extern GLboolean GLAD_OES_vertex_array_object;
#define GL_VERTEX_ARRAY_BINDING_OES            0x85B5
typedef void (APIENTRYP pfn_glBindVertexArrayOES) (GLuint);
extern pfn_glBindVertexArrayOES fp_glBindVertexArrayOES;
typedef void (APIENTRYP pfn_glDeleteVertexArraysOES) (GLsizei, const GLuint*);
extern pfn_glDeleteVertexArraysOES fp_glDeleteVertexArraysOES;
typedef void (APIENTRYP pfn_glGenVertexArraysOES) (GLsizei, GLuint*);
extern pfn_glGenVertexArraysOES fp_glGenVertexArraysOES;
typedef GLboolean (APIENTRYP pfn_glIsVertexArrayOES) (GLuint);
extern pfn_glIsVertexArrayOES fp_glIsVertexArrayOES;

 /* GL_OES_vertex_half_float */
extern GLboolean GLAD_OES_vertex_half_float;

 /* GL_OES_vertex_type_10_10_10_2 */
extern GLboolean GLAD_OES_vertex_type_10_10_10_2;
#define GL_UNSIGNED_INT_10_10_10_2_OES         0x8DF6
#define GL_INT_10_10_10_2_OES                  0x8DF7

 /* GL_AMD_blend_minmax_factor */
extern GLboolean GLAD_AMD_blend_minmax_factor;
#define GL_FACTOR_MIN_AMD                      0x901C
#define GL_FACTOR_MAX_AMD                      0x901D

 /* GL_AMD_compressed_3DC_texture */
extern GLboolean GLAD_AMD_compressed_3DC_texture;
#define GL_3DC_X_AMD                           0x87F9
#define GL_3DC_XY_AMD                          0x87FA

 /* GL_AMD_compressed_ATC_texture */
extern GLboolean GLAD_AMD_compressed_ATC_texture;
#define GL_ATC_RGB_AMD                         0x8C92
#define GL_ATC_RGBA_EXPLICIT_ALPHA_AMD         0x8C93
#define GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD     0x87EE

 /* GL_AMD_conservative_depth */
extern GLboolean GLAD_AMD_conservative_depth;

 /* GL_AMD_debug_output */
extern GLboolean GLAD_AMD_debug_output;
#define GL_MAX_DEBUG_MESSAGE_LENGTH_AMD        0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES_AMD       0x9144
#define GL_DEBUG_LOGGED_MESSAGES_AMD           0x9145
#define GL_DEBUG_SEVERITY_HIGH_AMD             0x9146
#define GL_DEBUG_SEVERITY_MEDIUM_AMD           0x9147
#define GL_DEBUG_SEVERITY_LOW_AMD              0x9148
#define GL_DEBUG_CATEGORY_API_ERROR_AMD        0x9149
#define GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD    0x914A
#define GL_DEBUG_CATEGORY_DEPRECATION_AMD      0x914B
#define GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD  0x914C
#define GL_DEBUG_CATEGORY_PERFORMANCE_AMD      0x914D
#define GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD  0x914E
#define GL_DEBUG_CATEGORY_APPLICATION_AMD      0x914F
#define GL_DEBUG_CATEGORY_OTHER_AMD            0x9150
typedef void (APIENTRYP pfn_glDebugMessageEnableAMD) (GLenum, GLenum, GLsizei, const GLuint*, GLboolean);
extern pfn_glDebugMessageEnableAMD fp_glDebugMessageEnableAMD;
typedef void (APIENTRYP pfn_glDebugMessageInsertAMD) (GLenum, GLenum, GLuint, GLsizei, const GLchar*);
extern pfn_glDebugMessageInsertAMD fp_glDebugMessageInsertAMD;
typedef void (APIENTRYP pfn_glDebugMessageCallbackAMD) (GLDEBUGPROCAMD, void*);
extern pfn_glDebugMessageCallbackAMD fp_glDebugMessageCallbackAMD;
typedef GLuint (APIENTRYP pfn_glGetDebugMessageLogAMD) (GLuint, GLsizei, GLenum*, GLuint*, GLuint*, GLsizei*, GLchar*);
extern pfn_glGetDebugMessageLogAMD fp_glGetDebugMessageLogAMD;

 /* GL_AMD_depth_clamp_separate */
extern GLboolean GLAD_AMD_depth_clamp_separate;
#define GL_DEPTH_CLAMP_NEAR_AMD                0x901E
#define GL_DEPTH_CLAMP_FAR_AMD                 0x901F

 /* GL_AMD_draw_buffers_blend */
extern GLboolean GLAD_AMD_draw_buffers_blend;
typedef void (APIENTRYP pfn_glBlendFuncIndexedAMD) (GLuint, GLenum, GLenum);
extern pfn_glBlendFuncIndexedAMD fp_glBlendFuncIndexedAMD;
typedef void (APIENTRYP pfn_glBlendFuncSeparateIndexedAMD) (GLuint, GLenum, GLenum, GLenum, GLenum);
extern pfn_glBlendFuncSeparateIndexedAMD fp_glBlendFuncSeparateIndexedAMD;
typedef void (APIENTRYP pfn_glBlendEquationIndexedAMD) (GLuint, GLenum);
extern pfn_glBlendEquationIndexedAMD fp_glBlendEquationIndexedAMD;
typedef void (APIENTRYP pfn_glBlendEquationSeparateIndexedAMD) (GLuint, GLenum, GLenum);
extern pfn_glBlendEquationSeparateIndexedAMD fp_glBlendEquationSeparateIndexedAMD;

 /* GL_AMD_gcn_shader */
extern GLboolean GLAD_AMD_gcn_shader;

 /* GL_AMD_gpu_shader_int64 */
extern GLboolean GLAD_AMD_gpu_shader_int64;
#define GL_INT64_NV                            0x140E
#define GL_UNSIGNED_INT64_NV                   0x140F
#define GL_INT8_NV                             0x8FE0
#define GL_INT8_VEC2_NV                        0x8FE1
#define GL_INT8_VEC3_NV                        0x8FE2
#define GL_INT8_VEC4_NV                        0x8FE3
#define GL_INT16_NV                            0x8FE4
#define GL_INT16_VEC2_NV                       0x8FE5
#define GL_INT16_VEC3_NV                       0x8FE6
#define GL_INT16_VEC4_NV                       0x8FE7
#define GL_INT64_VEC2_NV                       0x8FE9
#define GL_INT64_VEC3_NV                       0x8FEA
#define GL_INT64_VEC4_NV                       0x8FEB
#define GL_UNSIGNED_INT8_NV                    0x8FEC
#define GL_UNSIGNED_INT8_VEC2_NV               0x8FED
#define GL_UNSIGNED_INT8_VEC3_NV               0x8FEE
#define GL_UNSIGNED_INT8_VEC4_NV               0x8FEF
#define GL_UNSIGNED_INT16_NV                   0x8FF0
#define GL_UNSIGNED_INT16_VEC2_NV              0x8FF1
#define GL_UNSIGNED_INT16_VEC3_NV              0x8FF2
#define GL_UNSIGNED_INT16_VEC4_NV              0x8FF3
#define GL_UNSIGNED_INT64_VEC2_NV              0x8FF5
#define GL_UNSIGNED_INT64_VEC3_NV              0x8FF6
#define GL_UNSIGNED_INT64_VEC4_NV              0x8FF7
#define GL_FLOAT16_NV                          0x8FF8
#define GL_FLOAT16_VEC2_NV                     0x8FF9
#define GL_FLOAT16_VEC3_NV                     0x8FFA
#define GL_FLOAT16_VEC4_NV                     0x8FFB
typedef void (APIENTRYP pfn_glUniform1i64NV) (GLint, GLint64EXT);
extern pfn_glUniform1i64NV fp_glUniform1i64NV;
typedef void (APIENTRYP pfn_glUniform2i64NV) (GLint, GLint64EXT, GLint64EXT);
extern pfn_glUniform2i64NV fp_glUniform2i64NV;
typedef void (APIENTRYP pfn_glUniform3i64NV) (GLint, GLint64EXT, GLint64EXT, GLint64EXT);
extern pfn_glUniform3i64NV fp_glUniform3i64NV;
typedef void (APIENTRYP pfn_glUniform4i64NV) (GLint, GLint64EXT, GLint64EXT, GLint64EXT, GLint64EXT);
extern pfn_glUniform4i64NV fp_glUniform4i64NV;
typedef void (APIENTRYP pfn_glUniform1i64vNV) (GLint, GLsizei, const GLint64EXT*);
extern pfn_glUniform1i64vNV fp_glUniform1i64vNV;
typedef void (APIENTRYP pfn_glUniform2i64vNV) (GLint, GLsizei, const GLint64EXT*);
extern pfn_glUniform2i64vNV fp_glUniform2i64vNV;
typedef void (APIENTRYP pfn_glUniform3i64vNV) (GLint, GLsizei, const GLint64EXT*);
extern pfn_glUniform3i64vNV fp_glUniform3i64vNV;
typedef void (APIENTRYP pfn_glUniform4i64vNV) (GLint, GLsizei, const GLint64EXT*);
extern pfn_glUniform4i64vNV fp_glUniform4i64vNV;
typedef void (APIENTRYP pfn_glUniform1ui64NV) (GLint, GLuint64EXT);
extern pfn_glUniform1ui64NV fp_glUniform1ui64NV;
typedef void (APIENTRYP pfn_glUniform2ui64NV) (GLint, GLuint64EXT, GLuint64EXT);
extern pfn_glUniform2ui64NV fp_glUniform2ui64NV;
typedef void (APIENTRYP pfn_glUniform3ui64NV) (GLint, GLuint64EXT, GLuint64EXT, GLuint64EXT);
extern pfn_glUniform3ui64NV fp_glUniform3ui64NV;
typedef void (APIENTRYP pfn_glUniform4ui64NV) (GLint, GLuint64EXT, GLuint64EXT, GLuint64EXT, GLuint64EXT);
extern pfn_glUniform4ui64NV fp_glUniform4ui64NV;
typedef void (APIENTRYP pfn_glUniform1ui64vNV) (GLint, GLsizei, const GLuint64EXT*);
extern pfn_glUniform1ui64vNV fp_glUniform1ui64vNV;
typedef void (APIENTRYP pfn_glUniform2ui64vNV) (GLint, GLsizei, const GLuint64EXT*);
extern pfn_glUniform2ui64vNV fp_glUniform2ui64vNV;
typedef void (APIENTRYP pfn_glUniform3ui64vNV) (GLint, GLsizei, const GLuint64EXT*);
extern pfn_glUniform3ui64vNV fp_glUniform3ui64vNV;
typedef void (APIENTRYP pfn_glUniform4ui64vNV) (GLint, GLsizei, const GLuint64EXT*);
extern pfn_glUniform4ui64vNV fp_glUniform4ui64vNV;
typedef void (APIENTRYP pfn_glGetUniformi64vNV) (GLuint, GLint, GLint64EXT*);
extern pfn_glGetUniformi64vNV fp_glGetUniformi64vNV;
typedef void (APIENTRYP pfn_glGetUniformui64vNV) (GLuint, GLint, GLuint64EXT*);
extern pfn_glGetUniformui64vNV fp_glGetUniformui64vNV;
typedef void (APIENTRYP pfn_glProgramUniform1i64NV) (GLuint, GLint, GLint64EXT);
extern pfn_glProgramUniform1i64NV fp_glProgramUniform1i64NV;
typedef void (APIENTRYP pfn_glProgramUniform2i64NV) (GLuint, GLint, GLint64EXT, GLint64EXT);
extern pfn_glProgramUniform2i64NV fp_glProgramUniform2i64NV;
typedef void (APIENTRYP pfn_glProgramUniform3i64NV) (GLuint, GLint, GLint64EXT, GLint64EXT, GLint64EXT);
extern pfn_glProgramUniform3i64NV fp_glProgramUniform3i64NV;
typedef void (APIENTRYP pfn_glProgramUniform4i64NV) (GLuint, GLint, GLint64EXT, GLint64EXT, GLint64EXT, GLint64EXT);
extern pfn_glProgramUniform4i64NV fp_glProgramUniform4i64NV;
typedef void (APIENTRYP pfn_glProgramUniform1i64vNV) (GLuint, GLint, GLsizei, const GLint64EXT*);
extern pfn_glProgramUniform1i64vNV fp_glProgramUniform1i64vNV;
typedef void (APIENTRYP pfn_glProgramUniform2i64vNV) (GLuint, GLint, GLsizei, const GLint64EXT*);
extern pfn_glProgramUniform2i64vNV fp_glProgramUniform2i64vNV;
typedef void (APIENTRYP pfn_glProgramUniform3i64vNV) (GLuint, GLint, GLsizei, const GLint64EXT*);
extern pfn_glProgramUniform3i64vNV fp_glProgramUniform3i64vNV;
typedef void (APIENTRYP pfn_glProgramUniform4i64vNV) (GLuint, GLint, GLsizei, const GLint64EXT*);
extern pfn_glProgramUniform4i64vNV fp_glProgramUniform4i64vNV;
typedef void (APIENTRYP pfn_glProgramUniform1ui64NV) (GLuint, GLint, GLuint64EXT);
extern pfn_glProgramUniform1ui64NV fp_glProgramUniform1ui64NV;
typedef void (APIENTRYP pfn_glProgramUniform2ui64NV) (GLuint, GLint, GLuint64EXT, GLuint64EXT);
extern pfn_glProgramUniform2ui64NV fp_glProgramUniform2ui64NV;
typedef void (APIENTRYP pfn_glProgramUniform3ui64NV) (GLuint, GLint, GLuint64EXT, GLuint64EXT, GLuint64EXT);
extern pfn_glProgramUniform3ui64NV fp_glProgramUniform3ui64NV;
typedef void (APIENTRYP pfn_glProgramUniform4ui64NV) (GLuint, GLint, GLuint64EXT, GLuint64EXT, GLuint64EXT, GLuint64EXT);
extern pfn_glProgramUniform4ui64NV fp_glProgramUniform4ui64NV;
typedef void (APIENTRYP pfn_glProgramUniform1ui64vNV) (GLuint, GLint, GLsizei, const GLuint64EXT*);
extern pfn_glProgramUniform1ui64vNV fp_glProgramUniform1ui64vNV;
typedef void (APIENTRYP pfn_glProgramUniform2ui64vNV) (GLuint, GLint, GLsizei, const GLuint64EXT*);
extern pfn_glProgramUniform2ui64vNV fp_glProgramUniform2ui64vNV;
typedef void (APIENTRYP pfn_glProgramUniform3ui64vNV) (GLuint, GLint, GLsizei, const GLuint64EXT*);
extern pfn_glProgramUniform3ui64vNV fp_glProgramUniform3ui64vNV;
typedef void (APIENTRYP pfn_glProgramUniform4ui64vNV) (GLuint, GLint, GLsizei, const GLuint64EXT*);
extern pfn_glProgramUniform4ui64vNV fp_glProgramUniform4ui64vNV;

 /* GL_AMD_interleaved_elements */
extern GLboolean GLAD_AMD_interleaved_elements;
#define GL_VERTEX_ELEMENT_SWIZZLE_AMD          0x91A4
#define GL_VERTEX_ID_SWIZZLE_AMD               0x91A5
typedef void (APIENTRYP pfn_glVertexAttribParameteriAMD) (GLuint, GLenum, GLint);
extern pfn_glVertexAttribParameteriAMD fp_glVertexAttribParameteriAMD;

 /* GL_AMD_multi_draw_indirect */
extern GLboolean GLAD_AMD_multi_draw_indirect;
typedef void (APIENTRYP pfn_glMultiDrawArraysIndirectAMD) (GLenum, const void*, GLsizei, GLsizei);
extern pfn_glMultiDrawArraysIndirectAMD fp_glMultiDrawArraysIndirectAMD;
typedef void (APIENTRYP pfn_glMultiDrawElementsIndirectAMD) (GLenum, GLenum, const void*, GLsizei, GLsizei);
extern pfn_glMultiDrawElementsIndirectAMD fp_glMultiDrawElementsIndirectAMD;

 /* GL_AMD_name_gen_delete */
extern GLboolean GLAD_AMD_name_gen_delete;
#define GL_DATA_BUFFER_AMD                     0x9151
#define GL_PERFORMANCE_MONITOR_AMD             0x9152
#define GL_QUERY_OBJECT_AMD                    0x9153
#define GL_VERTEX_ARRAY_OBJECT_AMD             0x9154
#define GL_SAMPLER_OBJECT_AMD                  0x9155
typedef void (APIENTRYP pfn_glGenNamesAMD) (GLenum, GLuint, GLuint*);
extern pfn_glGenNamesAMD fp_glGenNamesAMD;
typedef void (APIENTRYP pfn_glDeleteNamesAMD) (GLenum, GLuint, const GLuint*);
extern pfn_glDeleteNamesAMD fp_glDeleteNamesAMD;
typedef GLboolean (APIENTRYP pfn_glIsNameAMD) (GLenum, GLuint);
extern pfn_glIsNameAMD fp_glIsNameAMD;

 /* GL_AMD_occlusion_query_event */
extern GLboolean GLAD_AMD_occlusion_query_event;
#define GL_OCCLUSION_QUERY_EVENT_MASK_AMD      0x874F
#define GL_QUERY_DEPTH_PASS_EVENT_BIT_AMD      0x00000001
#define GL_QUERY_DEPTH_FAIL_EVENT_BIT_AMD      0x00000002
#define GL_QUERY_STENCIL_FAIL_EVENT_BIT_AMD    0x00000004
#define GL_QUERY_DEPTH_BOUNDS_FAIL_EVENT_BIT_AMD  0x00000008
#define GL_QUERY_ALL_EVENT_BITS_AMD            0xFFFFFFFF
typedef void (APIENTRYP pfn_glQueryObjectParameteruiAMD) (GLenum, GLuint, GLenum, GLuint);
extern pfn_glQueryObjectParameteruiAMD fp_glQueryObjectParameteruiAMD;

 /* GL_AMD_performance_monitor */
extern GLboolean GLAD_AMD_performance_monitor;
#define GL_COUNTER_TYPE_AMD                    0x8BC0
#define GL_COUNTER_RANGE_AMD                   0x8BC1
#define GL_UNSIGNED_INT64_AMD                  0x8BC2
#define GL_PERCENTAGE_AMD                      0x8BC3
#define GL_PERFMON_RESULT_AVAILABLE_AMD        0x8BC4
#define GL_PERFMON_RESULT_SIZE_AMD             0x8BC5
#define GL_PERFMON_RESULT_AMD                  0x8BC6
typedef void (APIENTRYP pfn_glGetPerfMonitorGroupsAMD) (GLint*, GLsizei, GLuint*);
extern pfn_glGetPerfMonitorGroupsAMD fp_glGetPerfMonitorGroupsAMD;
typedef void (APIENTRYP pfn_glGetPerfMonitorCountersAMD) (GLuint, GLint*, GLint*, GLsizei, GLuint*);
extern pfn_glGetPerfMonitorCountersAMD fp_glGetPerfMonitorCountersAMD;
typedef void (APIENTRYP pfn_glGetPerfMonitorGroupStringAMD) (GLuint, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetPerfMonitorGroupStringAMD fp_glGetPerfMonitorGroupStringAMD;
typedef void (APIENTRYP pfn_glGetPerfMonitorCounterStringAMD) (GLuint, GLuint, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetPerfMonitorCounterStringAMD fp_glGetPerfMonitorCounterStringAMD;
typedef void (APIENTRYP pfn_glGetPerfMonitorCounterInfoAMD) (GLuint, GLuint, GLenum, void*);
extern pfn_glGetPerfMonitorCounterInfoAMD fp_glGetPerfMonitorCounterInfoAMD;
typedef void (APIENTRYP pfn_glGenPerfMonitorsAMD) (GLsizei, GLuint*);
extern pfn_glGenPerfMonitorsAMD fp_glGenPerfMonitorsAMD;
typedef void (APIENTRYP pfn_glDeletePerfMonitorsAMD) (GLsizei, GLuint*);
extern pfn_glDeletePerfMonitorsAMD fp_glDeletePerfMonitorsAMD;
typedef void (APIENTRYP pfn_glSelectPerfMonitorCountersAMD) (GLuint, GLboolean, GLuint, GLint, GLuint*);
extern pfn_glSelectPerfMonitorCountersAMD fp_glSelectPerfMonitorCountersAMD;
typedef void (APIENTRYP pfn_glBeginPerfMonitorAMD) (GLuint);
extern pfn_glBeginPerfMonitorAMD fp_glBeginPerfMonitorAMD;
typedef void (APIENTRYP pfn_glEndPerfMonitorAMD) (GLuint);
extern pfn_glEndPerfMonitorAMD fp_glEndPerfMonitorAMD;
typedef void (APIENTRYP pfn_glGetPerfMonitorCounterDataAMD) (GLuint, GLenum, GLsizei, GLuint*, GLint*);
extern pfn_glGetPerfMonitorCounterDataAMD fp_glGetPerfMonitorCounterDataAMD;

 /* GL_AMD_pinned_memory */
extern GLboolean GLAD_AMD_pinned_memory;
#define GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD  0x9160

 /* GL_AMD_program_binary_Z400 */
extern GLboolean GLAD_AMD_program_binary_Z400;
#define GL_Z400_BINARY_AMD                     0x8740

 /* GL_AMD_query_buffer_object */
extern GLboolean GLAD_AMD_query_buffer_object;
#define GL_QUERY_BUFFER_AMD                    0x9192
#define GL_QUERY_BUFFER_BINDING_AMD            0x9193
#define GL_QUERY_RESULT_NO_WAIT_AMD            0x9194

 /* GL_AMD_sample_positions */
extern GLboolean GLAD_AMD_sample_positions;
#define GL_SUBSAMPLE_DISTANCE_AMD              0x883F
typedef void (APIENTRYP pfn_glSetMultisamplefvAMD) (GLenum, GLuint, const GLfloat*);
extern pfn_glSetMultisamplefvAMD fp_glSetMultisamplefvAMD;

 /* GL_AMD_seamless_cubemap_per_texture */
extern GLboolean GLAD_AMD_seamless_cubemap_per_texture;

 /* GL_AMD_shader_atomic_counter_ops */
extern GLboolean GLAD_AMD_shader_atomic_counter_ops;

 /* GL_AMD_shader_stencil_export */
extern GLboolean GLAD_AMD_shader_stencil_export;

 /* GL_AMD_shader_trinary_minmax */
extern GLboolean GLAD_AMD_shader_trinary_minmax;

 /* GL_AMD_sparse_texture */
extern GLboolean GLAD_AMD_sparse_texture;
#define GL_VIRTUAL_PAGE_SIZE_X_AMD             0x9195
#define GL_VIRTUAL_PAGE_SIZE_Y_AMD             0x9196
#define GL_VIRTUAL_PAGE_SIZE_Z_AMD             0x9197
#define GL_MAX_SPARSE_TEXTURE_SIZE_AMD         0x9198
#define GL_MAX_SPARSE_3D_TEXTURE_SIZE_AMD      0x9199
#define GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS     0x919A
#define GL_MIN_SPARSE_LEVEL_AMD                0x919B
#define GL_MIN_LOD_WARNING_AMD                 0x919C
#define GL_TEXTURE_STORAGE_SPARSE_BIT_AMD      0x00000001
typedef void (APIENTRYP pfn_glTexStorageSparseAMD) (GLenum, GLenum, GLsizei, GLsizei, GLsizei, GLsizei, GLbitfield);
extern pfn_glTexStorageSparseAMD fp_glTexStorageSparseAMD;
typedef void (APIENTRYP pfn_glTextureStorageSparseAMD) (GLuint, GLenum, GLenum, GLsizei, GLsizei, GLsizei, GLsizei, GLbitfield);
extern pfn_glTextureStorageSparseAMD fp_glTextureStorageSparseAMD;

 /* GL_AMD_stencil_operation_extended */
extern GLboolean GLAD_AMD_stencil_operation_extended;
#define GL_SET_AMD                             0x874A
#define GL_REPLACE_VALUE_AMD                   0x874B
#define GL_STENCIL_OP_VALUE_AMD                0x874C
#define GL_STENCIL_BACK_OP_VALUE_AMD           0x874D
typedef void (APIENTRYP pfn_glStencilOpValueAMD) (GLenum, GLuint);
extern pfn_glStencilOpValueAMD fp_glStencilOpValueAMD;

 /* GL_AMD_texture_texture4 */
extern GLboolean GLAD_AMD_texture_texture4;

 /* GL_AMD_transform_feedback3_lines_triangles */
extern GLboolean GLAD_AMD_transform_feedback3_lines_triangles;

 /* GL_AMD_transform_feedback4 */
extern GLboolean GLAD_AMD_transform_feedback4;
#define GL_STREAM_RASTERIZATION_AMD            0x91A0

 /* GL_AMD_vertex_shader_layer */
extern GLboolean GLAD_AMD_vertex_shader_layer;

 /* GL_AMD_vertex_shader_tessellator */
extern GLboolean GLAD_AMD_vertex_shader_tessellator;
#define GL_SAMPLER_BUFFER_AMD                  0x9001
#define GL_INT_SAMPLER_BUFFER_AMD              0x9002
#define GL_UNSIGNED_INT_SAMPLER_BUFFER_AMD     0x9003
#define GL_TESSELLATION_MODE_AMD               0x9004
#define GL_TESSELLATION_FACTOR_AMD             0x9005
#define GL_DISCRETE_AMD                        0x9006
#define GL_CONTINUOUS_AMD                      0x9007
typedef void (APIENTRYP pfn_glTessellationFactorAMD) (GLfloat);
extern pfn_glTessellationFactorAMD fp_glTessellationFactorAMD;
typedef void (APIENTRYP pfn_glTessellationModeAMD) (GLenum);
extern pfn_glTessellationModeAMD fp_glTessellationModeAMD;

 /* GL_AMD_vertex_shader_viewport_index */
extern GLboolean GLAD_AMD_vertex_shader_viewport_index;

 /* GL_ANDROID_extension_pack_es31a */
extern GLboolean GLAD_ANDROID_extension_pack_es31a;

 /* GL_ANGLE_depth_texture */
extern GLboolean GLAD_ANGLE_depth_texture;

 /* GL_ANGLE_framebuffer_blit */
extern GLboolean GLAD_ANGLE_framebuffer_blit;
#define GL_READ_FRAMEBUFFER_ANGLE              0x8CA8
#define GL_DRAW_FRAMEBUFFER_ANGLE              0x8CA9
#define GL_DRAW_FRAMEBUFFER_BINDING_ANGLE      0x8CA6
#define GL_READ_FRAMEBUFFER_BINDING_ANGLE      0x8CAA
typedef void (APIENTRYP pfn_glBlitFramebufferANGLE) (GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
extern pfn_glBlitFramebufferANGLE fp_glBlitFramebufferANGLE;

 /* GL_ANGLE_framebuffer_multisample */
extern GLboolean GLAD_ANGLE_framebuffer_multisample;
#define GL_RENDERBUFFER_SAMPLES_ANGLE          0x8CAB
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_ANGLE  0x8D56
#define GL_MAX_SAMPLES_ANGLE                   0x8D57
typedef void (APIENTRYP pfn_glRenderbufferStorageMultisampleANGLE) (GLenum, GLsizei, GLenum, GLsizei, GLsizei);
extern pfn_glRenderbufferStorageMultisampleANGLE fp_glRenderbufferStorageMultisampleANGLE;

 /* GL_ANGLE_instanced_arrays */
extern GLboolean GLAD_ANGLE_instanced_arrays;
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ANGLE   0x88FE
typedef void (APIENTRYP pfn_glDrawArraysInstancedANGLE) (GLenum, GLint, GLsizei, GLsizei);
extern pfn_glDrawArraysInstancedANGLE fp_glDrawArraysInstancedANGLE;
typedef void (APIENTRYP pfn_glDrawElementsInstancedANGLE) (GLenum, GLsizei, GLenum, const void*, GLsizei);
extern pfn_glDrawElementsInstancedANGLE fp_glDrawElementsInstancedANGLE;
typedef void (APIENTRYP pfn_glVertexAttribDivisorANGLE) (GLuint, GLuint);
extern pfn_glVertexAttribDivisorANGLE fp_glVertexAttribDivisorANGLE;

 /* GL_ANGLE_pack_reverse_row_order */
extern GLboolean GLAD_ANGLE_pack_reverse_row_order;
#define GL_PACK_REVERSE_ROW_ORDER_ANGLE        0x93A4

 /* GL_ANGLE_program_binary */
extern GLboolean GLAD_ANGLE_program_binary;
#define GL_PROGRAM_BINARY_ANGLE                0x93A6

 /* GL_ANGLE_texture_compression_dxt3 */
extern GLboolean GLAD_ANGLE_texture_compression_dxt3;
#define GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE     0x83F2

 /* GL_ANGLE_texture_compression_dxt5 */
extern GLboolean GLAD_ANGLE_texture_compression_dxt5;
#define GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE     0x83F3

 /* GL_ANGLE_texture_usage */
extern GLboolean GLAD_ANGLE_texture_usage;
#define GL_TEXTURE_USAGE_ANGLE                 0x93A2
#define GL_FRAMEBUFFER_ATTACHMENT_ANGLE        0x93A3

 /* GL_ANGLE_translated_shader_source */
extern GLboolean GLAD_ANGLE_translated_shader_source;
#define GL_TRANSLATED_SHADER_SOURCE_LENGTH_ANGLE  0x93A0
typedef void (APIENTRYP pfn_glGetTranslatedShaderSourceANGLE) (GLuint, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetTranslatedShaderSourceANGLE fp_glGetTranslatedShaderSourceANGLE;

 /* GL_APPLE_aux_depth_stencil */
extern GLboolean GLAD_APPLE_aux_depth_stencil;
#define GL_AUX_DEPTH_STENCIL_APPLE             0x8A14

 /* GL_APPLE_client_storage */
extern GLboolean GLAD_APPLE_client_storage;
#define GL_UNPACK_CLIENT_STORAGE_APPLE         0x85B2

 /* GL_APPLE_clip_distance */
extern GLboolean GLAD_APPLE_clip_distance;
#define GL_MAX_CLIP_DISTANCES_APPLE            0x0D32
#define GL_CLIP_DISTANCE0_APPLE                0x3000
#define GL_CLIP_DISTANCE1_APPLE                0x3001
#define GL_CLIP_DISTANCE2_APPLE                0x3002
#define GL_CLIP_DISTANCE3_APPLE                0x3003
#define GL_CLIP_DISTANCE4_APPLE                0x3004
#define GL_CLIP_DISTANCE5_APPLE                0x3005
#define GL_CLIP_DISTANCE6_APPLE                0x3006
#define GL_CLIP_DISTANCE7_APPLE                0x3007

 /* GL_APPLE_color_buffer_packed_float */
extern GLboolean GLAD_APPLE_color_buffer_packed_float;

 /* GL_APPLE_copy_texture_levels */
extern GLboolean GLAD_APPLE_copy_texture_levels;
typedef void (APIENTRYP pfn_glCopyTextureLevelsAPPLE) (GLuint, GLuint, GLint, GLsizei);
extern pfn_glCopyTextureLevelsAPPLE fp_glCopyTextureLevelsAPPLE;

 /* GL_APPLE_element_array */
extern GLboolean GLAD_APPLE_element_array;
#define GL_ELEMENT_ARRAY_APPLE                 0x8A0C
#define GL_ELEMENT_ARRAY_TYPE_APPLE            0x8A0D
#define GL_ELEMENT_ARRAY_POINTER_APPLE         0x8A0E
typedef void (APIENTRYP pfn_glElementPointerAPPLE) (GLenum, const void*);
extern pfn_glElementPointerAPPLE fp_glElementPointerAPPLE;
typedef void (APIENTRYP pfn_glDrawElementArrayAPPLE) (GLenum, GLint, GLsizei);
extern pfn_glDrawElementArrayAPPLE fp_glDrawElementArrayAPPLE;
typedef void (APIENTRYP pfn_glDrawRangeElementArrayAPPLE) (GLenum, GLuint, GLuint, GLint, GLsizei);
extern pfn_glDrawRangeElementArrayAPPLE fp_glDrawRangeElementArrayAPPLE;
typedef void (APIENTRYP pfn_glMultiDrawElementArrayAPPLE) (GLenum, const GLint*, const GLsizei*, GLsizei);
extern pfn_glMultiDrawElementArrayAPPLE fp_glMultiDrawElementArrayAPPLE;
typedef void (APIENTRYP pfn_glMultiDrawRangeElementArrayAPPLE) (GLenum, GLuint, GLuint, const GLint*, const GLsizei*, GLsizei);
extern pfn_glMultiDrawRangeElementArrayAPPLE fp_glMultiDrawRangeElementArrayAPPLE;

 /* GL_APPLE_fence */
extern GLboolean GLAD_APPLE_fence;
#define GL_DRAW_PIXELS_APPLE                   0x8A0A
#define GL_FENCE_APPLE                         0x8A0B
typedef void (APIENTRYP pfn_glGenFencesAPPLE) (GLsizei, GLuint*);
extern pfn_glGenFencesAPPLE fp_glGenFencesAPPLE;
typedef void (APIENTRYP pfn_glDeleteFencesAPPLE) (GLsizei, const GLuint*);
extern pfn_glDeleteFencesAPPLE fp_glDeleteFencesAPPLE;
typedef void (APIENTRYP pfn_glSetFenceAPPLE) (GLuint);
extern pfn_glSetFenceAPPLE fp_glSetFenceAPPLE;
typedef GLboolean (APIENTRYP pfn_glIsFenceAPPLE) (GLuint);
extern pfn_glIsFenceAPPLE fp_glIsFenceAPPLE;
typedef GLboolean (APIENTRYP pfn_glTestFenceAPPLE) (GLuint);
extern pfn_glTestFenceAPPLE fp_glTestFenceAPPLE;
typedef void (APIENTRYP pfn_glFinishFenceAPPLE) (GLuint);
extern pfn_glFinishFenceAPPLE fp_glFinishFenceAPPLE;
typedef GLboolean (APIENTRYP pfn_glTestObjectAPPLE) (GLenum, GLuint);
extern pfn_glTestObjectAPPLE fp_glTestObjectAPPLE;
typedef void (APIENTRYP pfn_glFinishObjectAPPLE) (GLenum, GLint);
extern pfn_glFinishObjectAPPLE fp_glFinishObjectAPPLE;

 /* GL_APPLE_float_pixels */
extern GLboolean GLAD_APPLE_float_pixels;
#define GL_HALF_APPLE                          0x140B
#define GL_RGBA_FLOAT32_APPLE                  0x8814
#define GL_RGB_FLOAT32_APPLE                   0x8815
#define GL_ALPHA_FLOAT32_APPLE                 0x8816
#define GL_INTENSITY_FLOAT32_APPLE             0x8817
#define GL_LUMINANCE_FLOAT32_APPLE             0x8818
#define GL_LUMINANCE_ALPHA_FLOAT32_APPLE       0x8819
#define GL_RGBA_FLOAT16_APPLE                  0x881A
#define GL_RGB_FLOAT16_APPLE                   0x881B
#define GL_ALPHA_FLOAT16_APPLE                 0x881C
#define GL_INTENSITY_FLOAT16_APPLE             0x881D
#define GL_LUMINANCE_FLOAT16_APPLE             0x881E
#define GL_LUMINANCE_ALPHA_FLOAT16_APPLE       0x881F
#define GL_COLOR_FLOAT_APPLE                   0x8A0F

 /* GL_APPLE_flush_buffer_range */
extern GLboolean GLAD_APPLE_flush_buffer_range;
#define GL_BUFFER_SERIALIZED_MODIFY_APPLE      0x8A12
#define GL_BUFFER_FLUSHING_UNMAP_APPLE         0x8A13
typedef void (APIENTRYP pfn_glBufferParameteriAPPLE) (GLenum, GLenum, GLint);
extern pfn_glBufferParameteriAPPLE fp_glBufferParameteriAPPLE;
typedef void (APIENTRYP pfn_glFlushMappedBufferRangeAPPLE) (GLenum, GLintptr, GLsizeiptr);
extern pfn_glFlushMappedBufferRangeAPPLE fp_glFlushMappedBufferRangeAPPLE;

 /* GL_APPLE_framebuffer_multisample */
extern GLboolean GLAD_APPLE_framebuffer_multisample;
#define GL_RENDERBUFFER_SAMPLES_APPLE          0x8CAB
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_APPLE  0x8D56
#define GL_MAX_SAMPLES_APPLE                   0x8D57
#define GL_READ_FRAMEBUFFER_APPLE              0x8CA8
#define GL_DRAW_FRAMEBUFFER_APPLE              0x8CA9
#define GL_DRAW_FRAMEBUFFER_BINDING_APPLE      0x8CA6
#define GL_READ_FRAMEBUFFER_BINDING_APPLE      0x8CAA
typedef void (APIENTRYP pfn_glRenderbufferStorageMultisampleAPPLE) (GLenum, GLsizei, GLenum, GLsizei, GLsizei);
extern pfn_glRenderbufferStorageMultisampleAPPLE fp_glRenderbufferStorageMultisampleAPPLE;
typedef void (APIENTRYP pfn_glResolveMultisampleFramebufferAPPLE) ();
extern pfn_glResolveMultisampleFramebufferAPPLE fp_glResolveMultisampleFramebufferAPPLE;

 /* GL_APPLE_object_purgeable */
extern GLboolean GLAD_APPLE_object_purgeable;
#define GL_BUFFER_OBJECT_APPLE                 0x85B3
#define GL_RELEASED_APPLE                      0x8A19
#define GL_VOLATILE_APPLE                      0x8A1A
#define GL_RETAINED_APPLE                      0x8A1B
#define GL_UNDEFINED_APPLE                     0x8A1C
#define GL_PURGEABLE_APPLE                     0x8A1D
typedef GLenum (APIENTRYP pfn_glObjectPurgeableAPPLE) (GLenum, GLuint, GLenum);
extern pfn_glObjectPurgeableAPPLE fp_glObjectPurgeableAPPLE;
typedef GLenum (APIENTRYP pfn_glObjectUnpurgeableAPPLE) (GLenum, GLuint, GLenum);
extern pfn_glObjectUnpurgeableAPPLE fp_glObjectUnpurgeableAPPLE;
typedef void (APIENTRYP pfn_glGetObjectParameterivAPPLE) (GLenum, GLuint, GLenum, GLint*);
extern pfn_glGetObjectParameterivAPPLE fp_glGetObjectParameterivAPPLE;

 /* GL_APPLE_rgb_422 */
extern GLboolean GLAD_APPLE_rgb_422;
#define GL_RGB_422_APPLE                       0x8A1F
#define GL_UNSIGNED_SHORT_8_8_APPLE            0x85BA
#define GL_UNSIGNED_SHORT_8_8_REV_APPLE        0x85BB
#define GL_RGB_RAW_422_APPLE                   0x8A51

 /* GL_APPLE_row_bytes */
extern GLboolean GLAD_APPLE_row_bytes;
#define GL_PACK_ROW_BYTES_APPLE                0x8A15
#define GL_UNPACK_ROW_BYTES_APPLE              0x8A16

 /* GL_APPLE_sync */
extern GLboolean GLAD_APPLE_sync;
#define GL_SYNC_OBJECT_APPLE                   0x8A53
#define GL_MAX_SERVER_WAIT_TIMEOUT_APPLE       0x9111
#define GL_OBJECT_TYPE_APPLE                   0x9112
#define GL_SYNC_CONDITION_APPLE                0x9113
#define GL_SYNC_STATUS_APPLE                   0x9114
#define GL_SYNC_FLAGS_APPLE                    0x9115
#define GL_SYNC_FENCE_APPLE                    0x9116
#define GL_SYNC_GPU_COMMANDS_COMPLETE_APPLE    0x9117
#define GL_UNSIGNALED_APPLE                    0x9118
#define GL_SIGNALED_APPLE                      0x9119
#define GL_ALREADY_SIGNALED_APPLE              0x911A
#define GL_TIMEOUT_EXPIRED_APPLE               0x911B
#define GL_CONDITION_SATISFIED_APPLE           0x911C
#define GL_WAIT_FAILED_APPLE                   0x911D
#define GL_SYNC_FLUSH_COMMANDS_BIT_APPLE       0x00000001
#define GL_TIMEOUT_IGNORED_APPLE               0xFFFFFFFFFFFFFFFF
typedef GLsync (APIENTRYP pfn_glFenceSyncAPPLE) (GLenum, GLbitfield);
extern pfn_glFenceSyncAPPLE fp_glFenceSyncAPPLE;
typedef GLboolean (APIENTRYP pfn_glIsSyncAPPLE) (GLsync);
extern pfn_glIsSyncAPPLE fp_glIsSyncAPPLE;
typedef void (APIENTRYP pfn_glDeleteSyncAPPLE) (GLsync);
extern pfn_glDeleteSyncAPPLE fp_glDeleteSyncAPPLE;
typedef GLenum (APIENTRYP pfn_glClientWaitSyncAPPLE) (GLsync, GLbitfield, GLuint64);
extern pfn_glClientWaitSyncAPPLE fp_glClientWaitSyncAPPLE;
typedef void (APIENTRYP pfn_glWaitSyncAPPLE) (GLsync, GLbitfield, GLuint64);
extern pfn_glWaitSyncAPPLE fp_glWaitSyncAPPLE;
typedef void (APIENTRYP pfn_glGetInteger64vAPPLE) (GLenum, GLint64*);
extern pfn_glGetInteger64vAPPLE fp_glGetInteger64vAPPLE;
typedef void (APIENTRYP pfn_glGetSyncivAPPLE) (GLsync, GLenum, GLsizei, GLsizei*, GLint*);
extern pfn_glGetSyncivAPPLE fp_glGetSyncivAPPLE;

 /* GL_APPLE_texture_format_BGRA8888 */
extern GLboolean GLAD_APPLE_texture_format_BGRA8888;

 /* GL_APPLE_texture_max_level */
extern GLboolean GLAD_APPLE_texture_max_level;
#define GL_TEXTURE_MAX_LEVEL_APPLE             0x813D

 /* GL_APPLE_texture_packed_float */
extern GLboolean GLAD_APPLE_texture_packed_float;
#define GL_UNSIGNED_INT_10F_11F_11F_REV_APPLE  0x8C3B
#define GL_UNSIGNED_INT_5_9_9_9_REV_APPLE      0x8C3E
#define GL_R11F_G11F_B10F_APPLE                0x8C3A
#define GL_RGB9_E5_APPLE                       0x8C3D

 /* GL_APPLE_texture_range */
extern GLboolean GLAD_APPLE_texture_range;
#define GL_TEXTURE_RANGE_LENGTH_APPLE          0x85B7
#define GL_TEXTURE_RANGE_POINTER_APPLE         0x85B8
#define GL_TEXTURE_STORAGE_HINT_APPLE          0x85BC
#define GL_STORAGE_PRIVATE_APPLE               0x85BD
#define GL_STORAGE_CACHED_APPLE                0x85BE
#define GL_STORAGE_SHARED_APPLE                0x85BF
typedef void (APIENTRYP pfn_glTextureRangeAPPLE) (GLenum, GLsizei, const void*);
extern pfn_glTextureRangeAPPLE fp_glTextureRangeAPPLE;
typedef void (APIENTRYP pfn_glGetTexParameterPointervAPPLE) (GLenum, GLenum, void**);
extern pfn_glGetTexParameterPointervAPPLE fp_glGetTexParameterPointervAPPLE;

 /* GL_APPLE_vertex_array_object */
extern GLboolean GLAD_APPLE_vertex_array_object;
#define GL_VERTEX_ARRAY_BINDING_APPLE          0x85B5
typedef void (APIENTRYP pfn_glBindVertexArrayAPPLE) (GLuint);
extern pfn_glBindVertexArrayAPPLE fp_glBindVertexArrayAPPLE;
typedef void (APIENTRYP pfn_glDeleteVertexArraysAPPLE) (GLsizei, const GLuint*);
extern pfn_glDeleteVertexArraysAPPLE fp_glDeleteVertexArraysAPPLE;
typedef void (APIENTRYP pfn_glGenVertexArraysAPPLE) (GLsizei, GLuint*);
extern pfn_glGenVertexArraysAPPLE fp_glGenVertexArraysAPPLE;
typedef GLboolean (APIENTRYP pfn_glIsVertexArrayAPPLE) (GLuint);
extern pfn_glIsVertexArrayAPPLE fp_glIsVertexArrayAPPLE;

 /* GL_APPLE_vertex_array_range */
extern GLboolean GLAD_APPLE_vertex_array_range;
#define GL_VERTEX_ARRAY_RANGE_APPLE            0x851D
#define GL_VERTEX_ARRAY_RANGE_LENGTH_APPLE     0x851E
#define GL_VERTEX_ARRAY_STORAGE_HINT_APPLE     0x851F
#define GL_VERTEX_ARRAY_RANGE_POINTER_APPLE    0x8521
#define GL_STORAGE_CLIENT_APPLE                0x85B4
typedef void (APIENTRYP pfn_glVertexArrayRangeAPPLE) (GLsizei, void*);
extern pfn_glVertexArrayRangeAPPLE fp_glVertexArrayRangeAPPLE;
typedef void (APIENTRYP pfn_glFlushVertexArrayRangeAPPLE) (GLsizei, void*);
extern pfn_glFlushVertexArrayRangeAPPLE fp_glFlushVertexArrayRangeAPPLE;
typedef void (APIENTRYP pfn_glVertexArrayParameteriAPPLE) (GLenum, GLint);
extern pfn_glVertexArrayParameteriAPPLE fp_glVertexArrayParameteriAPPLE;

 /* GL_APPLE_vertex_program_evaluators */
extern GLboolean GLAD_APPLE_vertex_program_evaluators;
#define GL_VERTEX_ATTRIB_MAP1_APPLE            0x8A00
#define GL_VERTEX_ATTRIB_MAP2_APPLE            0x8A01
#define GL_VERTEX_ATTRIB_MAP1_SIZE_APPLE       0x8A02
#define GL_VERTEX_ATTRIB_MAP1_COEFF_APPLE      0x8A03
#define GL_VERTEX_ATTRIB_MAP1_ORDER_APPLE      0x8A04
#define GL_VERTEX_ATTRIB_MAP1_DOMAIN_APPLE     0x8A05
#define GL_VERTEX_ATTRIB_MAP2_SIZE_APPLE       0x8A06
#define GL_VERTEX_ATTRIB_MAP2_COEFF_APPLE      0x8A07
#define GL_VERTEX_ATTRIB_MAP2_ORDER_APPLE      0x8A08
#define GL_VERTEX_ATTRIB_MAP2_DOMAIN_APPLE     0x8A09
typedef void (APIENTRYP pfn_glEnableVertexAttribAPPLE) (GLuint, GLenum);
extern pfn_glEnableVertexAttribAPPLE fp_glEnableVertexAttribAPPLE;
typedef void (APIENTRYP pfn_glDisableVertexAttribAPPLE) (GLuint, GLenum);
extern pfn_glDisableVertexAttribAPPLE fp_glDisableVertexAttribAPPLE;
typedef GLboolean (APIENTRYP pfn_glIsVertexAttribEnabledAPPLE) (GLuint, GLenum);
extern pfn_glIsVertexAttribEnabledAPPLE fp_glIsVertexAttribEnabledAPPLE;
typedef void (APIENTRYP pfn_glMapVertexAttrib1dAPPLE) (GLuint, GLuint, GLdouble, GLdouble, GLint, GLint, const GLdouble*);
extern pfn_glMapVertexAttrib1dAPPLE fp_glMapVertexAttrib1dAPPLE;
typedef void (APIENTRYP pfn_glMapVertexAttrib1fAPPLE) (GLuint, GLuint, GLfloat, GLfloat, GLint, GLint, const GLfloat*);
extern pfn_glMapVertexAttrib1fAPPLE fp_glMapVertexAttrib1fAPPLE;
typedef void (APIENTRYP pfn_glMapVertexAttrib2dAPPLE) (GLuint, GLuint, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, const GLdouble*);
extern pfn_glMapVertexAttrib2dAPPLE fp_glMapVertexAttrib2dAPPLE;
typedef void (APIENTRYP pfn_glMapVertexAttrib2fAPPLE) (GLuint, GLuint, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, const GLfloat*);
extern pfn_glMapVertexAttrib2fAPPLE fp_glMapVertexAttrib2fAPPLE;

 /* GL_APPLE_ycbcr_422 */
extern GLboolean GLAD_APPLE_ycbcr_422;
#define GL_YCBCR_422_APPLE                     0x85B9

 /* GL_ARM_mali_program_binary */
extern GLboolean GLAD_ARM_mali_program_binary;
#define GL_MALI_PROGRAM_BINARY_ARM             0x8F61

 /* GL_ARM_mali_shader_binary */
extern GLboolean GLAD_ARM_mali_shader_binary;
#define GL_MALI_SHADER_BINARY_ARM              0x8F60

 /* GL_ARM_rgba8 */
extern GLboolean GLAD_ARM_rgba8;

 /* GL_ARM_shader_framebuffer_fetch */
extern GLboolean GLAD_ARM_shader_framebuffer_fetch;
#define GL_FETCH_PER_SAMPLE_ARM                0x8F65
#define GL_FRAGMENT_SHADER_FRAMEBUFFER_FETCH_MRT_ARM  0x8F66

 /* GL_ARM_shader_framebuffer_fetch_depth_stencil */
extern GLboolean GLAD_ARM_shader_framebuffer_fetch_depth_stencil;

 /* GL_ATI_meminfo */
extern GLboolean GLAD_ATI_meminfo;
#define GL_VBO_FREE_MEMORY_ATI                 0x87FB
#define GL_TEXTURE_FREE_MEMORY_ATI             0x87FC
#define GL_RENDERBUFFER_FREE_MEMORY_ATI        0x87FD

 /* GL_ATI_pixel_format_float */
extern GLboolean GLAD_ATI_pixel_format_float;
#define GL_RGBA_FLOAT_MODE_ATI                 0x8820
#define GL_COLOR_CLEAR_UNCLAMPED_VALUE_ATI     0x8835

 /* GL_DMP_program_binary */
extern GLboolean GLAD_DMP_program_binary;
#define GL_SMAPHS30_PROGRAM_BINARY_DMP         0x9251
#define GL_SMAPHS_PROGRAM_BINARY_DMP           0x9252
#define GL_DMP_PROGRAM_BINARY_DMP              0x9253

 /* GL_DMP_shader_binary */
extern GLboolean GLAD_DMP_shader_binary;
#define GL_SHADER_BINARY_DMP                   0x9250

 /* GL_FJ_shader_binary_GCCSO */
extern GLboolean GLAD_FJ_shader_binary_GCCSO;
#define GL_GCCSO_SHADER_BINARY_FJ              0x9260

 /* GL_GREMEDY_frame_terminator */
extern GLboolean GLAD_GREMEDY_frame_terminator;
typedef void (APIENTRYP pfn_glFrameTerminatorGREMEDY) ();
extern pfn_glFrameTerminatorGREMEDY fp_glFrameTerminatorGREMEDY;

 /* GL_GREMEDY_string_marker */
extern GLboolean GLAD_GREMEDY_string_marker;
typedef void (APIENTRYP pfn_glStringMarkerGREMEDY) (GLsizei, const void*);
extern pfn_glStringMarkerGREMEDY fp_glStringMarkerGREMEDY;

 /* GL_IMG_multisampled_render_to_texture */
extern GLboolean GLAD_IMG_multisampled_render_to_texture;
#define GL_RENDERBUFFER_SAMPLES_IMG            0x9133
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_IMG  0x9134
#define GL_MAX_SAMPLES_IMG                     0x9135
#define GL_TEXTURE_SAMPLES_IMG                 0x9136
typedef void (APIENTRYP pfn_glRenderbufferStorageMultisampleIMG) (GLenum, GLsizei, GLenum, GLsizei, GLsizei);
extern pfn_glRenderbufferStorageMultisampleIMG fp_glRenderbufferStorageMultisampleIMG;
typedef void (APIENTRYP pfn_glFramebufferTexture2DMultisampleIMG) (GLenum, GLenum, GLenum, GLuint, GLint, GLsizei);
extern pfn_glFramebufferTexture2DMultisampleIMG fp_glFramebufferTexture2DMultisampleIMG;

 /* GL_IMG_program_binary */
extern GLboolean GLAD_IMG_program_binary;
#define GL_SGX_PROGRAM_BINARY_IMG              0x9130

 /* GL_IMG_read_format */
extern GLboolean GLAD_IMG_read_format;
#define GL_BGRA_IMG                            0x80E1
#define GL_UNSIGNED_SHORT_4_4_4_4_REV_IMG      0x8365

 /* GL_IMG_shader_binary */
extern GLboolean GLAD_IMG_shader_binary;
#define GL_SGX_BINARY_IMG                      0x8C0A

 /* GL_IMG_texture_compression_pvrtc */
extern GLboolean GLAD_IMG_texture_compression_pvrtc;
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG     0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG     0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG    0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG    0x8C03

 /* GL_IMG_texture_compression_pvrtc2 */
extern GLboolean GLAD_IMG_texture_compression_pvrtc2;
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG    0x9137
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG    0x9138

 /* GL_IMG_texture_filter_cubic */
extern GLboolean GLAD_IMG_texture_filter_cubic;
#define GL_CUBIC_IMG                           0x9139
#define GL_CUBIC_MIPMAP_NEAREST_IMG            0x913A
#define GL_CUBIC_MIPMAP_LINEAR_IMG             0x913B

 /* GL_INGR_blend_func_separate */
extern GLboolean GLAD_INGR_blend_func_separate;
typedef void (APIENTRYP pfn_glBlendFuncSeparateINGR) (GLenum, GLenum, GLenum, GLenum);
extern pfn_glBlendFuncSeparateINGR fp_glBlendFuncSeparateINGR;

 /* GL_INTEL_fragment_shader_ordering */
extern GLboolean GLAD_INTEL_fragment_shader_ordering;

 /* GL_INTEL_framebuffer_CMAA */
extern GLboolean GLAD_INTEL_framebuffer_CMAA;
typedef void (APIENTRYP pfn_glApplyFramebufferAttachmentCMAAINTEL) ();
extern pfn_glApplyFramebufferAttachmentCMAAINTEL fp_glApplyFramebufferAttachmentCMAAINTEL;

 /* GL_INTEL_map_texture */
extern GLboolean GLAD_INTEL_map_texture;
#define GL_TEXTURE_MEMORY_LAYOUT_INTEL         0x83FF
#define GL_LAYOUT_DEFAULT_INTEL                0
#define GL_LAYOUT_LINEAR_INTEL                 1
#define GL_LAYOUT_LINEAR_CPU_CACHED_INTEL      2
typedef void (APIENTRYP pfn_glSyncTextureINTEL) (GLuint);
extern pfn_glSyncTextureINTEL fp_glSyncTextureINTEL;
typedef void (APIENTRYP pfn_glUnmapTexture2DINTEL) (GLuint, GLint);
extern pfn_glUnmapTexture2DINTEL fp_glUnmapTexture2DINTEL;
typedef void* (APIENTRYP pfn_glMapTexture2DINTEL) (GLuint, GLint, GLbitfield, GLint*, GLenum*);
extern pfn_glMapTexture2DINTEL fp_glMapTexture2DINTEL;

 /* GL_INTEL_performance_query */
extern GLboolean GLAD_INTEL_performance_query;
#define GL_PERFQUERY_SINGLE_CONTEXT_INTEL      0x00000000
#define GL_PERFQUERY_GLOBAL_CONTEXT_INTEL      0x00000001
#define GL_PERFQUERY_WAIT_INTEL                0x83FB
#define GL_PERFQUERY_FLUSH_INTEL               0x83FA
#define GL_PERFQUERY_DONOT_FLUSH_INTEL         0x83F9
#define GL_PERFQUERY_COUNTER_EVENT_INTEL       0x94F0
#define GL_PERFQUERY_COUNTER_DURATION_NORM_INTEL  0x94F1
#define GL_PERFQUERY_COUNTER_DURATION_RAW_INTEL  0x94F2
#define GL_PERFQUERY_COUNTER_THROUGHPUT_INTEL  0x94F3
#define GL_PERFQUERY_COUNTER_RAW_INTEL         0x94F4
#define GL_PERFQUERY_COUNTER_TIMESTAMP_INTEL   0x94F5
#define GL_PERFQUERY_COUNTER_DATA_UINT32_INTEL  0x94F8
#define GL_PERFQUERY_COUNTER_DATA_UINT64_INTEL  0x94F9
#define GL_PERFQUERY_COUNTER_DATA_FLOAT_INTEL  0x94FA
#define GL_PERFQUERY_COUNTER_DATA_DOUBLE_INTEL  0x94FB
#define GL_PERFQUERY_COUNTER_DATA_BOOL32_INTEL  0x94FC
#define GL_PERFQUERY_QUERY_NAME_LENGTH_MAX_INTEL  0x94FD
#define GL_PERFQUERY_COUNTER_NAME_LENGTH_MAX_INTEL  0x94FE
#define GL_PERFQUERY_COUNTER_DESC_LENGTH_MAX_INTEL  0x94FF
#define GL_PERFQUERY_GPA_EXTENDED_COUNTERS_INTEL  0x9500
typedef void (APIENTRYP pfn_glBeginPerfQueryINTEL) (GLuint);
extern pfn_glBeginPerfQueryINTEL fp_glBeginPerfQueryINTEL;
typedef void (APIENTRYP pfn_glCreatePerfQueryINTEL) (GLuint, GLuint*);
extern pfn_glCreatePerfQueryINTEL fp_glCreatePerfQueryINTEL;
typedef void (APIENTRYP pfn_glDeletePerfQueryINTEL) (GLuint);
extern pfn_glDeletePerfQueryINTEL fp_glDeletePerfQueryINTEL;
typedef void (APIENTRYP pfn_glEndPerfQueryINTEL) (GLuint);
extern pfn_glEndPerfQueryINTEL fp_glEndPerfQueryINTEL;
typedef void (APIENTRYP pfn_glGetFirstPerfQueryIdINTEL) (GLuint*);
extern pfn_glGetFirstPerfQueryIdINTEL fp_glGetFirstPerfQueryIdINTEL;
typedef void (APIENTRYP pfn_glGetNextPerfQueryIdINTEL) (GLuint, GLuint*);
extern pfn_glGetNextPerfQueryIdINTEL fp_glGetNextPerfQueryIdINTEL;
typedef void (APIENTRYP pfn_glGetPerfCounterInfoINTEL) (GLuint, GLuint, GLuint, GLchar*, GLuint, GLchar*, GLuint*, GLuint*, GLuint*, GLuint*, GLuint64*);
extern pfn_glGetPerfCounterInfoINTEL fp_glGetPerfCounterInfoINTEL;
typedef void (APIENTRYP pfn_glGetPerfQueryDataINTEL) (GLuint, GLuint, GLsizei, GLvoid*, GLuint*);
extern pfn_glGetPerfQueryDataINTEL fp_glGetPerfQueryDataINTEL;
typedef void (APIENTRYP pfn_glGetPerfQueryIdByNameINTEL) (GLchar*, GLuint*);
extern pfn_glGetPerfQueryIdByNameINTEL fp_glGetPerfQueryIdByNameINTEL;
typedef void (APIENTRYP pfn_glGetPerfQueryInfoINTEL) (GLuint, GLuint, GLchar*, GLuint*, GLuint*, GLuint*, GLuint*);
extern pfn_glGetPerfQueryInfoINTEL fp_glGetPerfQueryInfoINTEL;

 /* GL_MESA_pack_invert */
extern GLboolean GLAD_MESA_pack_invert;
#define GL_PACK_INVERT_MESA                    0x8758

 /* GL_MESA_ycbcr_texture */
extern GLboolean GLAD_MESA_ycbcr_texture;
#define GL_UNSIGNED_SHORT_8_8_MESA             0x85BA
#define GL_UNSIGNED_SHORT_8_8_REV_MESA         0x85BB
#define GL_YCBCR_MESA                          0x8757

 /* GL_MESAX_texture_stack */
extern GLboolean GLAD_MESAX_texture_stack;
#define GL_TEXTURE_1D_STACK_MESAX              0x8759
#define GL_TEXTURE_2D_STACK_MESAX              0x875A
#define GL_PROXY_TEXTURE_1D_STACK_MESAX        0x875B
#define GL_PROXY_TEXTURE_2D_STACK_MESAX        0x875C
#define GL_TEXTURE_1D_STACK_BINDING_MESAX      0x875D
#define GL_TEXTURE_2D_STACK_BINDING_MESAX      0x875E

 /* GL_NV_bindless_multi_draw_indirect */
extern GLboolean GLAD_NV_bindless_multi_draw_indirect;
typedef void (APIENTRYP pfn_glMultiDrawArraysIndirectBindlessNV) (GLenum, const void*, GLsizei, GLsizei, GLint);
extern pfn_glMultiDrawArraysIndirectBindlessNV fp_glMultiDrawArraysIndirectBindlessNV;
typedef void (APIENTRYP pfn_glMultiDrawElementsIndirectBindlessNV) (GLenum, GLenum, const void*, GLsizei, GLsizei, GLint);
extern pfn_glMultiDrawElementsIndirectBindlessNV fp_glMultiDrawElementsIndirectBindlessNV;

 /* GL_NV_bindless_multi_draw_indirect_count */
extern GLboolean GLAD_NV_bindless_multi_draw_indirect_count;
typedef void (APIENTRYP pfn_glMultiDrawArraysIndirectBindlessCountNV) (GLenum, const void*, GLsizei, GLsizei, GLsizei, GLint);
extern pfn_glMultiDrawArraysIndirectBindlessCountNV fp_glMultiDrawArraysIndirectBindlessCountNV;
typedef void (APIENTRYP pfn_glMultiDrawElementsIndirectBindlessCountNV) (GLenum, GLenum, const void*, GLsizei, GLsizei, GLsizei, GLint);
extern pfn_glMultiDrawElementsIndirectBindlessCountNV fp_glMultiDrawElementsIndirectBindlessCountNV;

 /* GL_NV_bindless_texture */
extern GLboolean GLAD_NV_bindless_texture;
typedef GLuint64 (APIENTRYP pfn_glGetTextureHandleNV) (GLuint);
extern pfn_glGetTextureHandleNV fp_glGetTextureHandleNV;
typedef GLuint64 (APIENTRYP pfn_glGetTextureSamplerHandleNV) (GLuint, GLuint);
extern pfn_glGetTextureSamplerHandleNV fp_glGetTextureSamplerHandleNV;
typedef void (APIENTRYP pfn_glMakeTextureHandleResidentNV) (GLuint64);
extern pfn_glMakeTextureHandleResidentNV fp_glMakeTextureHandleResidentNV;
typedef void (APIENTRYP pfn_glMakeTextureHandleNonResidentNV) (GLuint64);
extern pfn_glMakeTextureHandleNonResidentNV fp_glMakeTextureHandleNonResidentNV;
typedef GLuint64 (APIENTRYP pfn_glGetImageHandleNV) (GLuint, GLint, GLboolean, GLint, GLenum);
extern pfn_glGetImageHandleNV fp_glGetImageHandleNV;
typedef void (APIENTRYP pfn_glMakeImageHandleResidentNV) (GLuint64, GLenum);
extern pfn_glMakeImageHandleResidentNV fp_glMakeImageHandleResidentNV;
typedef void (APIENTRYP pfn_glMakeImageHandleNonResidentNV) (GLuint64);
extern pfn_glMakeImageHandleNonResidentNV fp_glMakeImageHandleNonResidentNV;
typedef void (APIENTRYP pfn_glUniformHandleui64NV) (GLint, GLuint64);
extern pfn_glUniformHandleui64NV fp_glUniformHandleui64NV;
typedef void (APIENTRYP pfn_glUniformHandleui64vNV) (GLint, GLsizei, const GLuint64*);
extern pfn_glUniformHandleui64vNV fp_glUniformHandleui64vNV;
typedef void (APIENTRYP pfn_glProgramUniformHandleui64NV) (GLuint, GLint, GLuint64);
extern pfn_glProgramUniformHandleui64NV fp_glProgramUniformHandleui64NV;
typedef void (APIENTRYP pfn_glProgramUniformHandleui64vNV) (GLuint, GLint, GLsizei, const GLuint64*);
extern pfn_glProgramUniformHandleui64vNV fp_glProgramUniformHandleui64vNV;
typedef GLboolean (APIENTRYP pfn_glIsTextureHandleResidentNV) (GLuint64);
extern pfn_glIsTextureHandleResidentNV fp_glIsTextureHandleResidentNV;
typedef GLboolean (APIENTRYP pfn_glIsImageHandleResidentNV) (GLuint64);
extern pfn_glIsImageHandleResidentNV fp_glIsImageHandleResidentNV;

 /* GL_NV_blend_equation_advanced */
extern GLboolean GLAD_NV_blend_equation_advanced;
#define GL_BLEND_OVERLAP_NV                    0x9281
#define GL_BLEND_PREMULTIPLIED_SRC_NV          0x9280
#define GL_BLUE_NV                             0x1905
#define GL_COLORBURN_NV                        0x929A
#define GL_COLORDODGE_NV                       0x9299
#define GL_CONJOINT_NV                         0x9284
#define GL_CONTRAST_NV                         0x92A1
#define GL_DARKEN_NV                           0x9297
#define GL_DIFFERENCE_NV                       0x929E
#define GL_DISJOINT_NV                         0x9283
#define GL_DST_ATOP_NV                         0x928F
#define GL_DST_IN_NV                           0x928B
#define GL_DST_NV                              0x9287
#define GL_DST_OUT_NV                          0x928D
#define GL_DST_OVER_NV                         0x9289
#define GL_EXCLUSION_NV                        0x92A0
#define GL_GREEN_NV                            0x1904
#define GL_HARDLIGHT_NV                        0x929B
#define GL_HARDMIX_NV                          0x92A9
#define GL_HSL_COLOR_NV                        0x92AF
#define GL_HSL_HUE_NV                          0x92AD
#define GL_HSL_LUMINOSITY_NV                   0x92B0
#define GL_HSL_SATURATION_NV                   0x92AE
#define GL_INVERT_OVG_NV                       0x92B4
#define GL_INVERT_RGB_NV                       0x92A3
#define GL_LIGHTEN_NV                          0x9298
#define GL_LINEARBURN_NV                       0x92A5
#define GL_LINEARDODGE_NV                      0x92A4
#define GL_LINEARLIGHT_NV                      0x92A7
#define GL_MINUS_CLAMPED_NV                    0x92B3
#define GL_MINUS_NV                            0x929F
#define GL_MULTIPLY_NV                         0x9294
#define GL_OVERLAY_NV                          0x9296
#define GL_PINLIGHT_NV                         0x92A8
#define GL_PLUS_CLAMPED_ALPHA_NV               0x92B2
#define GL_PLUS_CLAMPED_NV                     0x92B1
#define GL_PLUS_DARKER_NV                      0x9292
#define GL_PLUS_NV                             0x9291
#define GL_RED_NV                              0x1903
#define GL_SCREEN_NV                           0x9295
#define GL_SOFTLIGHT_NV                        0x929C
#define GL_SRC_ATOP_NV                         0x928E
#define GL_SRC_IN_NV                           0x928A
#define GL_SRC_NV                              0x9286
#define GL_SRC_OUT_NV                          0x928C
#define GL_SRC_OVER_NV                         0x9288
#define GL_UNCORRELATED_NV                     0x9282
#define GL_VIVIDLIGHT_NV                       0x92A6
#define GL_XOR_NV                              0x1506
typedef void (APIENTRYP pfn_glBlendParameteriNV) (GLenum, GLint);
extern pfn_glBlendParameteriNV fp_glBlendParameteriNV;
typedef void (APIENTRYP pfn_glBlendBarrierNV) ();
extern pfn_glBlendBarrierNV fp_glBlendBarrierNV;

 /* GL_NV_blend_equation_advanced_coherent */
extern GLboolean GLAD_NV_blend_equation_advanced_coherent;
#define GL_BLEND_ADVANCED_COHERENT_NV          0x9285

 /* GL_NV_command_list */
extern GLboolean GLAD_NV_command_list;
#define GL_TERMINATE_SEQUENCE_COMMAND_NV       0x0000
#define GL_NOP_COMMAND_NV                      0x0001
#define GL_DRAW_ELEMENTS_COMMAND_NV            0x0002
#define GL_DRAW_ARRAYS_COMMAND_NV              0x0003
#define GL_DRAW_ELEMENTS_STRIP_COMMAND_NV      0x0004
#define GL_DRAW_ARRAYS_STRIP_COMMAND_NV        0x0005
#define GL_DRAW_ELEMENTS_INSTANCED_COMMAND_NV  0x0006
#define GL_DRAW_ARRAYS_INSTANCED_COMMAND_NV    0x0007
#define GL_ELEMENT_ADDRESS_COMMAND_NV          0x0008
#define GL_ATTRIBUTE_ADDRESS_COMMAND_NV        0x0009
#define GL_UNIFORM_ADDRESS_COMMAND_NV          0x000A
#define GL_BLEND_COLOR_COMMAND_NV              0x000B
#define GL_STENCIL_REF_COMMAND_NV              0x000C
#define GL_LINE_WIDTH_COMMAND_NV               0x000D
#define GL_POLYGON_OFFSET_COMMAND_NV           0x000E
#define GL_ALPHA_REF_COMMAND_NV                0x000F
#define GL_VIEWPORT_COMMAND_NV                 0x0010
#define GL_SCISSOR_COMMAND_NV                  0x0011
#define GL_FRONT_FACE_COMMAND_NV               0x0012
typedef void (APIENTRYP pfn_glCreateStatesNV) (GLsizei, GLuint*);
extern pfn_glCreateStatesNV fp_glCreateStatesNV;
typedef void (APIENTRYP pfn_glDeleteStatesNV) (GLsizei, const GLuint*);
extern pfn_glDeleteStatesNV fp_glDeleteStatesNV;
typedef GLboolean (APIENTRYP pfn_glIsStateNV) (GLuint);
extern pfn_glIsStateNV fp_glIsStateNV;
typedef void (APIENTRYP pfn_glStateCaptureNV) (GLuint, GLenum);
extern pfn_glStateCaptureNV fp_glStateCaptureNV;
typedef GLuint (APIENTRYP pfn_glGetCommandHeaderNV) (GLenum, GLuint);
extern pfn_glGetCommandHeaderNV fp_glGetCommandHeaderNV;
typedef GLushort (APIENTRYP pfn_glGetStageIndexNV) (GLenum);
extern pfn_glGetStageIndexNV fp_glGetStageIndexNV;
typedef void (APIENTRYP pfn_glDrawCommandsNV) (GLenum, GLuint, const GLintptr*, const GLsizei*, GLuint);
extern pfn_glDrawCommandsNV fp_glDrawCommandsNV;
typedef void (APIENTRYP pfn_glDrawCommandsAddressNV) (GLenum, const GLuint64*, const GLsizei*, GLuint);
extern pfn_glDrawCommandsAddressNV fp_glDrawCommandsAddressNV;
typedef void (APIENTRYP pfn_glDrawCommandsStatesNV) (GLuint, const GLintptr*, const GLsizei*, const GLuint*, const GLuint*, GLuint);
extern pfn_glDrawCommandsStatesNV fp_glDrawCommandsStatesNV;
typedef void (APIENTRYP pfn_glDrawCommandsStatesAddressNV) (const GLuint64*, const GLsizei*, const GLuint*, const GLuint*, GLuint);
extern pfn_glDrawCommandsStatesAddressNV fp_glDrawCommandsStatesAddressNV;
typedef void (APIENTRYP pfn_glCreateCommandListsNV) (GLsizei, GLuint*);
extern pfn_glCreateCommandListsNV fp_glCreateCommandListsNV;
typedef void (APIENTRYP pfn_glDeleteCommandListsNV) (GLsizei, const GLuint*);
extern pfn_glDeleteCommandListsNV fp_glDeleteCommandListsNV;
typedef GLboolean (APIENTRYP pfn_glIsCommandListNV) (GLuint);
extern pfn_glIsCommandListNV fp_glIsCommandListNV;
typedef void (APIENTRYP pfn_glListDrawCommandsStatesClientNV) (GLuint, GLuint, const void**, const GLsizei*, const GLuint*, const GLuint*, GLuint);
extern pfn_glListDrawCommandsStatesClientNV fp_glListDrawCommandsStatesClientNV;
typedef void (APIENTRYP pfn_glCommandListSegmentsNV) (GLuint, GLuint);
extern pfn_glCommandListSegmentsNV fp_glCommandListSegmentsNV;
typedef void (APIENTRYP pfn_glCompileCommandListNV) (GLuint);
extern pfn_glCompileCommandListNV fp_glCompileCommandListNV;
typedef void (APIENTRYP pfn_glCallCommandListNV) (GLuint);
extern pfn_glCallCommandListNV fp_glCallCommandListNV;

 /* GL_NV_compute_program5 */
extern GLboolean GLAD_NV_compute_program5;
#define GL_COMPUTE_PROGRAM_NV                  0x90FB
#define GL_COMPUTE_PROGRAM_PARAMETER_BUFFER_NV  0x90FC

 /* GL_NV_conditional_render */
extern GLboolean GLAD_NV_conditional_render;
#define GL_QUERY_WAIT_NV                       0x8E13
#define GL_QUERY_NO_WAIT_NV                    0x8E14
#define GL_QUERY_BY_REGION_WAIT_NV             0x8E15
#define GL_QUERY_BY_REGION_NO_WAIT_NV          0x8E16
typedef void (APIENTRYP pfn_glBeginConditionalRenderNV) (GLuint, GLenum);
extern pfn_glBeginConditionalRenderNV fp_glBeginConditionalRenderNV;
typedef void (APIENTRYP pfn_glEndConditionalRenderNV) ();
extern pfn_glEndConditionalRenderNV fp_glEndConditionalRenderNV;

 /* GL_NV_conservative_raster */
extern GLboolean GLAD_NV_conservative_raster;
#define GL_CONSERVATIVE_RASTERIZATION_NV       0x9346
#define GL_SUBPIXEL_PRECISION_BIAS_X_BITS_NV   0x9347
#define GL_SUBPIXEL_PRECISION_BIAS_Y_BITS_NV   0x9348
#define GL_MAX_SUBPIXEL_PRECISION_BIAS_BITS_NV  0x9349
typedef void (APIENTRYP pfn_glSubpixelPrecisionBiasNV) (GLuint, GLuint);
extern pfn_glSubpixelPrecisionBiasNV fp_glSubpixelPrecisionBiasNV;

 /* GL_NV_conservative_raster_dilate */
extern GLboolean GLAD_NV_conservative_raster_dilate;
#define GL_CONSERVATIVE_RASTER_DILATE_NV       0x9379
#define GL_CONSERVATIVE_RASTER_DILATE_RANGE_NV  0x937A
#define GL_CONSERVATIVE_RASTER_DILATE_GRANULARITY_NV  0x937B
typedef void (APIENTRYP pfn_glConservativeRasterParameterfNV) (GLenum, GLfloat);
extern pfn_glConservativeRasterParameterfNV fp_glConservativeRasterParameterfNV;

 /* GL_NV_copy_buffer */
extern GLboolean GLAD_NV_copy_buffer;
#define GL_COPY_READ_BUFFER_NV                 0x8F36
#define GL_COPY_WRITE_BUFFER_NV                0x8F37
typedef void (APIENTRYP pfn_glCopyBufferSubDataNV) (GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr);
extern pfn_glCopyBufferSubDataNV fp_glCopyBufferSubDataNV;

 /* GL_NV_copy_image */
extern GLboolean GLAD_NV_copy_image;
typedef void (APIENTRYP pfn_glCopyImageSubDataNV) (GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei);
extern pfn_glCopyImageSubDataNV fp_glCopyImageSubDataNV;

 /* GL_NV_coverage_sample */
extern GLboolean GLAD_NV_coverage_sample;
#define GL_COVERAGE_COMPONENT_NV               0x8ED0
#define GL_COVERAGE_COMPONENT4_NV              0x8ED1
#define GL_COVERAGE_ATTACHMENT_NV              0x8ED2
#define GL_COVERAGE_BUFFERS_NV                 0x8ED3
#define GL_COVERAGE_SAMPLES_NV                 0x8ED4
#define GL_COVERAGE_ALL_FRAGMENTS_NV           0x8ED5
#define GL_COVERAGE_EDGE_FRAGMENTS_NV          0x8ED6
#define GL_COVERAGE_AUTOMATIC_NV               0x8ED7
#define GL_COVERAGE_BUFFER_BIT_NV              0x00008000
typedef void (APIENTRYP pfn_glCoverageMaskNV) (GLboolean);
extern pfn_glCoverageMaskNV fp_glCoverageMaskNV;
typedef void (APIENTRYP pfn_glCoverageOperationNV) (GLenum);
extern pfn_glCoverageOperationNV fp_glCoverageOperationNV;

 /* GL_NV_deep_texture3D */
extern GLboolean GLAD_NV_deep_texture3D;
#define GL_MAX_DEEP_3D_TEXTURE_WIDTH_HEIGHT_NV  0x90D0
#define GL_MAX_DEEP_3D_TEXTURE_DEPTH_NV        0x90D1

 /* GL_NV_depth_buffer_float */
extern GLboolean GLAD_NV_depth_buffer_float;
#define GL_DEPTH_COMPONENT32F_NV               0x8DAB
#define GL_DEPTH32F_STENCIL8_NV                0x8DAC
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV_NV   0x8DAD
#define GL_DEPTH_BUFFER_FLOAT_MODE_NV          0x8DAF
typedef void (APIENTRYP pfn_glDepthRangedNV) (GLdouble, GLdouble);
extern pfn_glDepthRangedNV fp_glDepthRangedNV;
typedef void (APIENTRYP pfn_glClearDepthdNV) (GLdouble);
extern pfn_glClearDepthdNV fp_glClearDepthdNV;
typedef void (APIENTRYP pfn_glDepthBoundsdNV) (GLdouble, GLdouble);
extern pfn_glDepthBoundsdNV fp_glDepthBoundsdNV;

 /* GL_NV_depth_clamp */
extern GLboolean GLAD_NV_depth_clamp;
#define GL_DEPTH_CLAMP_NV                      0x864F

 /* GL_NV_depth_nonlinear */
extern GLboolean GLAD_NV_depth_nonlinear;
#define GL_DEPTH_COMPONENT16_NONLINEAR_NV      0x8E2C

 /* GL_NV_draw_buffers */
extern GLboolean GLAD_NV_draw_buffers;
#define GL_MAX_DRAW_BUFFERS_NV                 0x8824
#define GL_DRAW_BUFFER0_NV                     0x8825
#define GL_DRAW_BUFFER1_NV                     0x8826
#define GL_DRAW_BUFFER2_NV                     0x8827
#define GL_DRAW_BUFFER3_NV                     0x8828
#define GL_DRAW_BUFFER4_NV                     0x8829
#define GL_DRAW_BUFFER5_NV                     0x882A
#define GL_DRAW_BUFFER6_NV                     0x882B
#define GL_DRAW_BUFFER7_NV                     0x882C
#define GL_DRAW_BUFFER8_NV                     0x882D
#define GL_DRAW_BUFFER9_NV                     0x882E
#define GL_DRAW_BUFFER10_NV                    0x882F
#define GL_DRAW_BUFFER11_NV                    0x8830
#define GL_DRAW_BUFFER12_NV                    0x8831
#define GL_DRAW_BUFFER13_NV                    0x8832
#define GL_DRAW_BUFFER14_NV                    0x8833
#define GL_DRAW_BUFFER15_NV                    0x8834
#define GL_COLOR_ATTACHMENT0_NV                0x8CE0
#define GL_COLOR_ATTACHMENT1_NV                0x8CE1
#define GL_COLOR_ATTACHMENT2_NV                0x8CE2
#define GL_COLOR_ATTACHMENT3_NV                0x8CE3
#define GL_COLOR_ATTACHMENT4_NV                0x8CE4
#define GL_COLOR_ATTACHMENT5_NV                0x8CE5
#define GL_COLOR_ATTACHMENT6_NV                0x8CE6
#define GL_COLOR_ATTACHMENT7_NV                0x8CE7
#define GL_COLOR_ATTACHMENT8_NV                0x8CE8
#define GL_COLOR_ATTACHMENT9_NV                0x8CE9
#define GL_COLOR_ATTACHMENT10_NV               0x8CEA
#define GL_COLOR_ATTACHMENT11_NV               0x8CEB
#define GL_COLOR_ATTACHMENT12_NV               0x8CEC
#define GL_COLOR_ATTACHMENT13_NV               0x8CED
#define GL_COLOR_ATTACHMENT14_NV               0x8CEE
#define GL_COLOR_ATTACHMENT15_NV               0x8CEF
typedef void (APIENTRYP pfn_glDrawBuffersNV) (GLsizei, const GLenum*);
extern pfn_glDrawBuffersNV fp_glDrawBuffersNV;

 /* GL_NV_draw_instanced */
extern GLboolean GLAD_NV_draw_instanced;
typedef void (APIENTRYP pfn_glDrawArraysInstancedNV) (GLenum, GLint, GLsizei, GLsizei);
extern pfn_glDrawArraysInstancedNV fp_glDrawArraysInstancedNV;
typedef void (APIENTRYP pfn_glDrawElementsInstancedNV) (GLenum, GLsizei, GLenum, const void*, GLsizei);
extern pfn_glDrawElementsInstancedNV fp_glDrawElementsInstancedNV;

 /* GL_NV_draw_texture */
extern GLboolean GLAD_NV_draw_texture;
typedef void (APIENTRYP pfn_glDrawTextureNV) (GLuint, GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glDrawTextureNV fp_glDrawTextureNV;

 /* GL_NV_explicit_attrib_location */
extern GLboolean GLAD_NV_explicit_attrib_location;

 /* GL_NV_explicit_multisample */
extern GLboolean GLAD_NV_explicit_multisample;
#define GL_SAMPLE_POSITION_NV                  0x8E50
#define GL_SAMPLE_MASK_NV                      0x8E51
#define GL_SAMPLE_MASK_VALUE_NV                0x8E52
#define GL_TEXTURE_BINDING_RENDERBUFFER_NV     0x8E53
#define GL_TEXTURE_RENDERBUFFER_DATA_STORE_BINDING_NV  0x8E54
#define GL_TEXTURE_RENDERBUFFER_NV             0x8E55
#define GL_SAMPLER_RENDERBUFFER_NV             0x8E56
#define GL_INT_SAMPLER_RENDERBUFFER_NV         0x8E57
#define GL_UNSIGNED_INT_SAMPLER_RENDERBUFFER_NV  0x8E58
#define GL_MAX_SAMPLE_MASK_WORDS_NV            0x8E59
typedef void (APIENTRYP pfn_glGetMultisamplefvNV) (GLenum, GLuint, GLfloat*);
extern pfn_glGetMultisamplefvNV fp_glGetMultisamplefvNV;
typedef void (APIENTRYP pfn_glSampleMaskIndexedNV) (GLuint, GLbitfield);
extern pfn_glSampleMaskIndexedNV fp_glSampleMaskIndexedNV;
typedef void (APIENTRYP pfn_glTexRenderbufferNV) (GLenum, GLuint);
extern pfn_glTexRenderbufferNV fp_glTexRenderbufferNV;

 /* GL_NV_fbo_color_attachments */
extern GLboolean GLAD_NV_fbo_color_attachments;
#define GL_MAX_COLOR_ATTACHMENTS_NV            0x8CDF

 /* GL_NV_fence */
extern GLboolean GLAD_NV_fence;
#define GL_ALL_COMPLETED_NV                    0x84F2
#define GL_FENCE_STATUS_NV                     0x84F3
#define GL_FENCE_CONDITION_NV                  0x84F4
typedef void (APIENTRYP pfn_glDeleteFencesNV) (GLsizei, const GLuint*);
extern pfn_glDeleteFencesNV fp_glDeleteFencesNV;
typedef void (APIENTRYP pfn_glGenFencesNV) (GLsizei, GLuint*);
extern pfn_glGenFencesNV fp_glGenFencesNV;
typedef GLboolean (APIENTRYP pfn_glIsFenceNV) (GLuint);
extern pfn_glIsFenceNV fp_glIsFenceNV;
typedef GLboolean (APIENTRYP pfn_glTestFenceNV) (GLuint);
extern pfn_glTestFenceNV fp_glTestFenceNV;
typedef void (APIENTRYP pfn_glGetFenceivNV) (GLuint, GLenum, GLint*);
extern pfn_glGetFenceivNV fp_glGetFenceivNV;
typedef void (APIENTRYP pfn_glFinishFenceNV) (GLuint);
extern pfn_glFinishFenceNV fp_glFinishFenceNV;
typedef void (APIENTRYP pfn_glSetFenceNV) (GLuint, GLenum);
extern pfn_glSetFenceNV fp_glSetFenceNV;

 /* GL_NV_fill_rectangle */
extern GLboolean GLAD_NV_fill_rectangle;
#define GL_FILL_RECTANGLE_NV                   0x933C

 /* GL_NV_float_buffer */
extern GLboolean GLAD_NV_float_buffer;
#define GL_FLOAT_R_NV                          0x8880
#define GL_FLOAT_RG_NV                         0x8881
#define GL_FLOAT_RGB_NV                        0x8882
#define GL_FLOAT_RGBA_NV                       0x8883
#define GL_FLOAT_R16_NV                        0x8884
#define GL_FLOAT_R32_NV                        0x8885
#define GL_FLOAT_RG16_NV                       0x8886
#define GL_FLOAT_RG32_NV                       0x8887
#define GL_FLOAT_RGB16_NV                      0x8888
#define GL_FLOAT_RGB32_NV                      0x8889
#define GL_FLOAT_RGBA16_NV                     0x888A
#define GL_FLOAT_RGBA32_NV                     0x888B
#define GL_TEXTURE_FLOAT_COMPONENTS_NV         0x888C
#define GL_FLOAT_CLEAR_COLOR_VALUE_NV          0x888D
#define GL_FLOAT_RGBA_MODE_NV                  0x888E

 /* GL_NV_fragment_coverage_to_color */
extern GLboolean GLAD_NV_fragment_coverage_to_color;
#define GL_FRAGMENT_COVERAGE_TO_COLOR_NV       0x92DD
#define GL_FRAGMENT_COVERAGE_COLOR_NV          0x92DE
typedef void (APIENTRYP pfn_glFragmentCoverageColorNV) (GLuint);
extern pfn_glFragmentCoverageColorNV fp_glFragmentCoverageColorNV;

 /* GL_NV_fragment_program4 */
extern GLboolean GLAD_NV_fragment_program4;

 /* GL_NV_fragment_shader_interlock */
extern GLboolean GLAD_NV_fragment_shader_interlock;

 /* GL_NV_framebuffer_blit */
extern GLboolean GLAD_NV_framebuffer_blit;
#define GL_READ_FRAMEBUFFER_NV                 0x8CA8
#define GL_DRAW_FRAMEBUFFER_NV                 0x8CA9
#define GL_DRAW_FRAMEBUFFER_BINDING_NV         0x8CA6
#define GL_READ_FRAMEBUFFER_BINDING_NV         0x8CAA
typedef void (APIENTRYP pfn_glBlitFramebufferNV) (GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
extern pfn_glBlitFramebufferNV fp_glBlitFramebufferNV;

 /* GL_NV_framebuffer_mixed_samples */
extern GLboolean GLAD_NV_framebuffer_mixed_samples;
#define GL_COVERAGE_MODULATION_TABLE_NV        0x9331
#define GL_COLOR_SAMPLES_NV                    0x8E20
#define GL_DEPTH_SAMPLES_NV                    0x932D
#define GL_STENCIL_SAMPLES_NV                  0x932E
#define GL_MIXED_DEPTH_SAMPLES_SUPPORTED_NV    0x932F
#define GL_MIXED_STENCIL_SAMPLES_SUPPORTED_NV  0x9330
#define GL_COVERAGE_MODULATION_NV              0x9332
#define GL_COVERAGE_MODULATION_TABLE_SIZE_NV   0x9333
typedef void (APIENTRYP pfn_glCoverageModulationTableNV) (GLsizei, const GLfloat*);
extern pfn_glCoverageModulationTableNV fp_glCoverageModulationTableNV;
typedef void (APIENTRYP pfn_glGetCoverageModulationTableNV) (GLsizei, GLfloat*);
extern pfn_glGetCoverageModulationTableNV fp_glGetCoverageModulationTableNV;
typedef void (APIENTRYP pfn_glCoverageModulationNV) (GLenum);
extern pfn_glCoverageModulationNV fp_glCoverageModulationNV;

 /* GL_NV_framebuffer_multisample */
extern GLboolean GLAD_NV_framebuffer_multisample;
#define GL_RENDERBUFFER_SAMPLES_NV             0x8CAB
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_NV  0x8D56
#define GL_MAX_SAMPLES_NV                      0x8D57
typedef void (APIENTRYP pfn_glRenderbufferStorageMultisampleNV) (GLenum, GLsizei, GLenum, GLsizei, GLsizei);
extern pfn_glRenderbufferStorageMultisampleNV fp_glRenderbufferStorageMultisampleNV;

 /* GL_NV_framebuffer_multisample_coverage */
extern GLboolean GLAD_NV_framebuffer_multisample_coverage;
#define GL_RENDERBUFFER_COVERAGE_SAMPLES_NV    0x8CAB
#define GL_RENDERBUFFER_COLOR_SAMPLES_NV       0x8E10
#define GL_MAX_MULTISAMPLE_COVERAGE_MODES_NV   0x8E11
#define GL_MULTISAMPLE_COVERAGE_MODES_NV       0x8E12
typedef void (APIENTRYP pfn_glRenderbufferStorageMultisampleCoverageNV) (GLenum, GLsizei, GLsizei, GLenum, GLsizei, GLsizei);
extern pfn_glRenderbufferStorageMultisampleCoverageNV fp_glRenderbufferStorageMultisampleCoverageNV;

 /* GL_NV_generate_mipmap_sRGB */
extern GLboolean GLAD_NV_generate_mipmap_sRGB;

 /* GL_NV_geometry_program4 */
extern GLboolean GLAD_NV_geometry_program4;
#define GL_GEOMETRY_PROGRAM_NV                 0x8C26
#define GL_MAX_PROGRAM_OUTPUT_VERTICES_NV      0x8C27
#define GL_MAX_PROGRAM_TOTAL_OUTPUT_COMPONENTS_NV  0x8C28
typedef void (APIENTRYP pfn_glProgramVertexLimitNV) (GLenum, GLint);
extern pfn_glProgramVertexLimitNV fp_glProgramVertexLimitNV;
typedef void (APIENTRYP pfn_glFramebufferTextureFaceEXT) (GLenum, GLenum, GLuint, GLint, GLenum);
extern pfn_glFramebufferTextureFaceEXT fp_glFramebufferTextureFaceEXT;

 /* GL_NV_geometry_shader4 */
extern GLboolean GLAD_NV_geometry_shader4;

 /* GL_NV_geometry_shader_passthrough */
extern GLboolean GLAD_NV_geometry_shader_passthrough;

 /* GL_NV_gpu_program4 */
extern GLboolean GLAD_NV_gpu_program4;
#define GL_MIN_PROGRAM_TEXEL_OFFSET_NV         0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET_NV         0x8905
#define GL_PROGRAM_ATTRIB_COMPONENTS_NV        0x8906
#define GL_PROGRAM_RESULT_COMPONENTS_NV        0x8907
#define GL_MAX_PROGRAM_ATTRIB_COMPONENTS_NV    0x8908
#define GL_MAX_PROGRAM_RESULT_COMPONENTS_NV    0x8909
#define GL_MAX_PROGRAM_GENERIC_ATTRIBS_NV      0x8DA5
#define GL_MAX_PROGRAM_GENERIC_RESULTS_NV      0x8DA6
typedef void (APIENTRYP pfn_glProgramLocalParameterI4iNV) (GLenum, GLuint, GLint, GLint, GLint, GLint);
extern pfn_glProgramLocalParameterI4iNV fp_glProgramLocalParameterI4iNV;
typedef void (APIENTRYP pfn_glProgramLocalParameterI4ivNV) (GLenum, GLuint, const GLint*);
extern pfn_glProgramLocalParameterI4ivNV fp_glProgramLocalParameterI4ivNV;
typedef void (APIENTRYP pfn_glProgramLocalParametersI4ivNV) (GLenum, GLuint, GLsizei, const GLint*);
extern pfn_glProgramLocalParametersI4ivNV fp_glProgramLocalParametersI4ivNV;
typedef void (APIENTRYP pfn_glProgramLocalParameterI4uiNV) (GLenum, GLuint, GLuint, GLuint, GLuint, GLuint);
extern pfn_glProgramLocalParameterI4uiNV fp_glProgramLocalParameterI4uiNV;
typedef void (APIENTRYP pfn_glProgramLocalParameterI4uivNV) (GLenum, GLuint, const GLuint*);
extern pfn_glProgramLocalParameterI4uivNV fp_glProgramLocalParameterI4uivNV;
typedef void (APIENTRYP pfn_glProgramLocalParametersI4uivNV) (GLenum, GLuint, GLsizei, const GLuint*);
extern pfn_glProgramLocalParametersI4uivNV fp_glProgramLocalParametersI4uivNV;
typedef void (APIENTRYP pfn_glProgramEnvParameterI4iNV) (GLenum, GLuint, GLint, GLint, GLint, GLint);
extern pfn_glProgramEnvParameterI4iNV fp_glProgramEnvParameterI4iNV;
typedef void (APIENTRYP pfn_glProgramEnvParameterI4ivNV) (GLenum, GLuint, const GLint*);
extern pfn_glProgramEnvParameterI4ivNV fp_glProgramEnvParameterI4ivNV;
typedef void (APIENTRYP pfn_glProgramEnvParametersI4ivNV) (GLenum, GLuint, GLsizei, const GLint*);
extern pfn_glProgramEnvParametersI4ivNV fp_glProgramEnvParametersI4ivNV;
typedef void (APIENTRYP pfn_glProgramEnvParameterI4uiNV) (GLenum, GLuint, GLuint, GLuint, GLuint, GLuint);
extern pfn_glProgramEnvParameterI4uiNV fp_glProgramEnvParameterI4uiNV;
typedef void (APIENTRYP pfn_glProgramEnvParameterI4uivNV) (GLenum, GLuint, const GLuint*);
extern pfn_glProgramEnvParameterI4uivNV fp_glProgramEnvParameterI4uivNV;
typedef void (APIENTRYP pfn_glProgramEnvParametersI4uivNV) (GLenum, GLuint, GLsizei, const GLuint*);
extern pfn_glProgramEnvParametersI4uivNV fp_glProgramEnvParametersI4uivNV;
typedef void (APIENTRYP pfn_glGetProgramLocalParameterIivNV) (GLenum, GLuint, GLint*);
extern pfn_glGetProgramLocalParameterIivNV fp_glGetProgramLocalParameterIivNV;
typedef void (APIENTRYP pfn_glGetProgramLocalParameterIuivNV) (GLenum, GLuint, GLuint*);
extern pfn_glGetProgramLocalParameterIuivNV fp_glGetProgramLocalParameterIuivNV;
typedef void (APIENTRYP pfn_glGetProgramEnvParameterIivNV) (GLenum, GLuint, GLint*);
extern pfn_glGetProgramEnvParameterIivNV fp_glGetProgramEnvParameterIivNV;
typedef void (APIENTRYP pfn_glGetProgramEnvParameterIuivNV) (GLenum, GLuint, GLuint*);
extern pfn_glGetProgramEnvParameterIuivNV fp_glGetProgramEnvParameterIuivNV;

 /* GL_NV_gpu_program5 */
extern GLboolean GLAD_NV_gpu_program5;
#define GL_MAX_GEOMETRY_PROGRAM_INVOCATIONS_NV  0x8E5A
#define GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_NV  0x8E5B
#define GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_NV  0x8E5C
#define GL_FRAGMENT_PROGRAM_INTERPOLATION_OFFSET_BITS_NV  0x8E5D
#define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_NV  0x8E5E
#define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_NV  0x8E5F
#define GL_MAX_PROGRAM_SUBROUTINE_PARAMETERS_NV  0x8F44
#define GL_MAX_PROGRAM_SUBROUTINE_NUM_NV       0x8F45
typedef void (APIENTRYP pfn_glProgramSubroutineParametersuivNV) (GLenum, GLsizei, const GLuint*);
extern pfn_glProgramSubroutineParametersuivNV fp_glProgramSubroutineParametersuivNV;
typedef void (APIENTRYP pfn_glGetProgramSubroutineParameteruivNV) (GLenum, GLuint, GLuint*);
extern pfn_glGetProgramSubroutineParameteruivNV fp_glGetProgramSubroutineParameteruivNV;

 /* GL_NV_gpu_program5_mem_extended */
extern GLboolean GLAD_NV_gpu_program5_mem_extended;

 /* GL_NV_gpu_shader5 */
extern GLboolean GLAD_NV_gpu_shader5;

 /* GL_NV_half_float */
extern GLboolean GLAD_NV_half_float;
#define GL_HALF_FLOAT_NV                       0x140B
typedef void (APIENTRYP pfn_glVertex2hNV) (GLhalfNV, GLhalfNV);
extern pfn_glVertex2hNV fp_glVertex2hNV;
typedef void (APIENTRYP pfn_glVertex2hvNV) (const GLhalfNV*);
extern pfn_glVertex2hvNV fp_glVertex2hvNV;
typedef void (APIENTRYP pfn_glVertex3hNV) (GLhalfNV, GLhalfNV, GLhalfNV);
extern pfn_glVertex3hNV fp_glVertex3hNV;
typedef void (APIENTRYP pfn_glVertex3hvNV) (const GLhalfNV*);
extern pfn_glVertex3hvNV fp_glVertex3hvNV;
typedef void (APIENTRYP pfn_glVertex4hNV) (GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
extern pfn_glVertex4hNV fp_glVertex4hNV;
typedef void (APIENTRYP pfn_glVertex4hvNV) (const GLhalfNV*);
extern pfn_glVertex4hvNV fp_glVertex4hvNV;
typedef void (APIENTRYP pfn_glNormal3hNV) (GLhalfNV, GLhalfNV, GLhalfNV);
extern pfn_glNormal3hNV fp_glNormal3hNV;
typedef void (APIENTRYP pfn_glNormal3hvNV) (const GLhalfNV*);
extern pfn_glNormal3hvNV fp_glNormal3hvNV;
typedef void (APIENTRYP pfn_glColor3hNV) (GLhalfNV, GLhalfNV, GLhalfNV);
extern pfn_glColor3hNV fp_glColor3hNV;
typedef void (APIENTRYP pfn_glColor3hvNV) (const GLhalfNV*);
extern pfn_glColor3hvNV fp_glColor3hvNV;
typedef void (APIENTRYP pfn_glColor4hNV) (GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
extern pfn_glColor4hNV fp_glColor4hNV;
typedef void (APIENTRYP pfn_glColor4hvNV) (const GLhalfNV*);
extern pfn_glColor4hvNV fp_glColor4hvNV;
typedef void (APIENTRYP pfn_glTexCoord1hNV) (GLhalfNV);
extern pfn_glTexCoord1hNV fp_glTexCoord1hNV;
typedef void (APIENTRYP pfn_glTexCoord1hvNV) (const GLhalfNV*);
extern pfn_glTexCoord1hvNV fp_glTexCoord1hvNV;
typedef void (APIENTRYP pfn_glTexCoord2hNV) (GLhalfNV, GLhalfNV);
extern pfn_glTexCoord2hNV fp_glTexCoord2hNV;
typedef void (APIENTRYP pfn_glTexCoord2hvNV) (const GLhalfNV*);
extern pfn_glTexCoord2hvNV fp_glTexCoord2hvNV;
typedef void (APIENTRYP pfn_glTexCoord3hNV) (GLhalfNV, GLhalfNV, GLhalfNV);
extern pfn_glTexCoord3hNV fp_glTexCoord3hNV;
typedef void (APIENTRYP pfn_glTexCoord3hvNV) (const GLhalfNV*);
extern pfn_glTexCoord3hvNV fp_glTexCoord3hvNV;
typedef void (APIENTRYP pfn_glTexCoord4hNV) (GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
extern pfn_glTexCoord4hNV fp_glTexCoord4hNV;
typedef void (APIENTRYP pfn_glTexCoord4hvNV) (const GLhalfNV*);
extern pfn_glTexCoord4hvNV fp_glTexCoord4hvNV;
typedef void (APIENTRYP pfn_glMultiTexCoord1hNV) (GLenum, GLhalfNV);
extern pfn_glMultiTexCoord1hNV fp_glMultiTexCoord1hNV;
typedef void (APIENTRYP pfn_glMultiTexCoord1hvNV) (GLenum, const GLhalfNV*);
extern pfn_glMultiTexCoord1hvNV fp_glMultiTexCoord1hvNV;
typedef void (APIENTRYP pfn_glMultiTexCoord2hNV) (GLenum, GLhalfNV, GLhalfNV);
extern pfn_glMultiTexCoord2hNV fp_glMultiTexCoord2hNV;
typedef void (APIENTRYP pfn_glMultiTexCoord2hvNV) (GLenum, const GLhalfNV*);
extern pfn_glMultiTexCoord2hvNV fp_glMultiTexCoord2hvNV;
typedef void (APIENTRYP pfn_glMultiTexCoord3hNV) (GLenum, GLhalfNV, GLhalfNV, GLhalfNV);
extern pfn_glMultiTexCoord3hNV fp_glMultiTexCoord3hNV;
typedef void (APIENTRYP pfn_glMultiTexCoord3hvNV) (GLenum, const GLhalfNV*);
extern pfn_glMultiTexCoord3hvNV fp_glMultiTexCoord3hvNV;
typedef void (APIENTRYP pfn_glMultiTexCoord4hNV) (GLenum, GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
extern pfn_glMultiTexCoord4hNV fp_glMultiTexCoord4hNV;
typedef void (APIENTRYP pfn_glMultiTexCoord4hvNV) (GLenum, const GLhalfNV*);
extern pfn_glMultiTexCoord4hvNV fp_glMultiTexCoord4hvNV;
typedef void (APIENTRYP pfn_glFogCoordhNV) (GLhalfNV);
extern pfn_glFogCoordhNV fp_glFogCoordhNV;
typedef void (APIENTRYP pfn_glFogCoordhvNV) (const GLhalfNV*);
extern pfn_glFogCoordhvNV fp_glFogCoordhvNV;
typedef void (APIENTRYP pfn_glSecondaryColor3hNV) (GLhalfNV, GLhalfNV, GLhalfNV);
extern pfn_glSecondaryColor3hNV fp_glSecondaryColor3hNV;
typedef void (APIENTRYP pfn_glSecondaryColor3hvNV) (const GLhalfNV*);
extern pfn_glSecondaryColor3hvNV fp_glSecondaryColor3hvNV;
typedef void (APIENTRYP pfn_glVertexWeighthNV) (GLhalfNV);
extern pfn_glVertexWeighthNV fp_glVertexWeighthNV;
typedef void (APIENTRYP pfn_glVertexWeighthvNV) (const GLhalfNV*);
extern pfn_glVertexWeighthvNV fp_glVertexWeighthvNV;
typedef void (APIENTRYP pfn_glVertexAttrib1hNV) (GLuint, GLhalfNV);
extern pfn_glVertexAttrib1hNV fp_glVertexAttrib1hNV;
typedef void (APIENTRYP pfn_glVertexAttrib1hvNV) (GLuint, const GLhalfNV*);
extern pfn_glVertexAttrib1hvNV fp_glVertexAttrib1hvNV;
typedef void (APIENTRYP pfn_glVertexAttrib2hNV) (GLuint, GLhalfNV, GLhalfNV);
extern pfn_glVertexAttrib2hNV fp_glVertexAttrib2hNV;
typedef void (APIENTRYP pfn_glVertexAttrib2hvNV) (GLuint, const GLhalfNV*);
extern pfn_glVertexAttrib2hvNV fp_glVertexAttrib2hvNV;
typedef void (APIENTRYP pfn_glVertexAttrib3hNV) (GLuint, GLhalfNV, GLhalfNV, GLhalfNV);
extern pfn_glVertexAttrib3hNV fp_glVertexAttrib3hNV;
typedef void (APIENTRYP pfn_glVertexAttrib3hvNV) (GLuint, const GLhalfNV*);
extern pfn_glVertexAttrib3hvNV fp_glVertexAttrib3hvNV;
typedef void (APIENTRYP pfn_glVertexAttrib4hNV) (GLuint, GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
extern pfn_glVertexAttrib4hNV fp_glVertexAttrib4hNV;
typedef void (APIENTRYP pfn_glVertexAttrib4hvNV) (GLuint, const GLhalfNV*);
extern pfn_glVertexAttrib4hvNV fp_glVertexAttrib4hvNV;
typedef void (APIENTRYP pfn_glVertexAttribs1hvNV) (GLuint, GLsizei, const GLhalfNV*);
extern pfn_glVertexAttribs1hvNV fp_glVertexAttribs1hvNV;
typedef void (APIENTRYP pfn_glVertexAttribs2hvNV) (GLuint, GLsizei, const GLhalfNV*);
extern pfn_glVertexAttribs2hvNV fp_glVertexAttribs2hvNV;
typedef void (APIENTRYP pfn_glVertexAttribs3hvNV) (GLuint, GLsizei, const GLhalfNV*);
extern pfn_glVertexAttribs3hvNV fp_glVertexAttribs3hvNV;
typedef void (APIENTRYP pfn_glVertexAttribs4hvNV) (GLuint, GLsizei, const GLhalfNV*);
extern pfn_glVertexAttribs4hvNV fp_glVertexAttribs4hvNV;

 /* GL_NV_image_formats */
extern GLboolean GLAD_NV_image_formats;

 /* GL_NV_instanced_arrays */
extern GLboolean GLAD_NV_instanced_arrays;
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR_NV      0x88FE
typedef void (APIENTRYP pfn_glVertexAttribDivisorNV) (GLuint, GLuint);
extern pfn_glVertexAttribDivisorNV fp_glVertexAttribDivisorNV;

 /* GL_NV_internalformat_sample_query */
extern GLboolean GLAD_NV_internalformat_sample_query;
#define GL_MULTISAMPLES_NV                     0x9371
#define GL_SUPERSAMPLE_SCALE_X_NV              0x9372
#define GL_SUPERSAMPLE_SCALE_Y_NV              0x9373
#define GL_CONFORMANT_NV                       0x9374
typedef void (APIENTRYP pfn_glGetInternalformatSampleivNV) (GLenum, GLenum, GLsizei, GLenum, GLsizei, GLint*);
extern pfn_glGetInternalformatSampleivNV fp_glGetInternalformatSampleivNV;

 /* GL_NV_multisample_coverage */
extern GLboolean GLAD_NV_multisample_coverage;
#define GL_SAMPLES_ARB                         0x80A9

 /* GL_NV_non_square_matrices */
extern GLboolean GLAD_NV_non_square_matrices;
#define GL_FLOAT_MAT2x3_NV                     0x8B65
#define GL_FLOAT_MAT2x4_NV                     0x8B66
#define GL_FLOAT_MAT3x2_NV                     0x8B67
#define GL_FLOAT_MAT3x4_NV                     0x8B68
#define GL_FLOAT_MAT4x2_NV                     0x8B69
#define GL_FLOAT_MAT4x3_NV                     0x8B6A
typedef void (APIENTRYP pfn_glUniformMatrix2x3fvNV) (GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glUniformMatrix2x3fvNV fp_glUniformMatrix2x3fvNV;
typedef void (APIENTRYP pfn_glUniformMatrix3x2fvNV) (GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glUniformMatrix3x2fvNV fp_glUniformMatrix3x2fvNV;
typedef void (APIENTRYP pfn_glUniformMatrix2x4fvNV) (GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glUniformMatrix2x4fvNV fp_glUniformMatrix2x4fvNV;
typedef void (APIENTRYP pfn_glUniformMatrix4x2fvNV) (GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glUniformMatrix4x2fvNV fp_glUniformMatrix4x2fvNV;
typedef void (APIENTRYP pfn_glUniformMatrix3x4fvNV) (GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glUniformMatrix3x4fvNV fp_glUniformMatrix3x4fvNV;
typedef void (APIENTRYP pfn_glUniformMatrix4x3fvNV) (GLint, GLsizei, GLboolean, const GLfloat*);
extern pfn_glUniformMatrix4x3fvNV fp_glUniformMatrix4x3fvNV;

 /* GL_NV_occlusion_query */
extern GLboolean GLAD_NV_occlusion_query;
#define GL_PIXEL_COUNTER_BITS_NV               0x8864
#define GL_CURRENT_OCCLUSION_QUERY_ID_NV       0x8865
#define GL_PIXEL_COUNT_NV                      0x8866
#define GL_PIXEL_COUNT_AVAILABLE_NV            0x8867
typedef void (APIENTRYP pfn_glGenOcclusionQueriesNV) (GLsizei, GLuint*);
extern pfn_glGenOcclusionQueriesNV fp_glGenOcclusionQueriesNV;
typedef void (APIENTRYP pfn_glDeleteOcclusionQueriesNV) (GLsizei, const GLuint*);
extern pfn_glDeleteOcclusionQueriesNV fp_glDeleteOcclusionQueriesNV;
typedef GLboolean (APIENTRYP pfn_glIsOcclusionQueryNV) (GLuint);
extern pfn_glIsOcclusionQueryNV fp_glIsOcclusionQueryNV;
typedef void (APIENTRYP pfn_glBeginOcclusionQueryNV) (GLuint);
extern pfn_glBeginOcclusionQueryNV fp_glBeginOcclusionQueryNV;
typedef void (APIENTRYP pfn_glEndOcclusionQueryNV) ();
extern pfn_glEndOcclusionQueryNV fp_glEndOcclusionQueryNV;
typedef void (APIENTRYP pfn_glGetOcclusionQueryivNV) (GLuint, GLenum, GLint*);
extern pfn_glGetOcclusionQueryivNV fp_glGetOcclusionQueryivNV;
typedef void (APIENTRYP pfn_glGetOcclusionQueryuivNV) (GLuint, GLenum, GLuint*);
extern pfn_glGetOcclusionQueryuivNV fp_glGetOcclusionQueryuivNV;

 /* GL_NV_parameter_buffer_object */
extern GLboolean GLAD_NV_parameter_buffer_object;
#define GL_MAX_PROGRAM_PARAMETER_BUFFER_BINDINGS_NV  0x8DA0
#define GL_MAX_PROGRAM_PARAMETER_BUFFER_SIZE_NV  0x8DA1
#define GL_VERTEX_PROGRAM_PARAMETER_BUFFER_NV  0x8DA2
#define GL_GEOMETRY_PROGRAM_PARAMETER_BUFFER_NV  0x8DA3
#define GL_FRAGMENT_PROGRAM_PARAMETER_BUFFER_NV  0x8DA4
typedef void (APIENTRYP pfn_glProgramBufferParametersfvNV) (GLenum, GLuint, GLuint, GLsizei, const GLfloat*);
extern pfn_glProgramBufferParametersfvNV fp_glProgramBufferParametersfvNV;
typedef void (APIENTRYP pfn_glProgramBufferParametersIivNV) (GLenum, GLuint, GLuint, GLsizei, const GLint*);
extern pfn_glProgramBufferParametersIivNV fp_glProgramBufferParametersIivNV;
typedef void (APIENTRYP pfn_glProgramBufferParametersIuivNV) (GLenum, GLuint, GLuint, GLsizei, const GLuint*);
extern pfn_glProgramBufferParametersIuivNV fp_glProgramBufferParametersIuivNV;

 /* GL_NV_parameter_buffer_object2 */
extern GLboolean GLAD_NV_parameter_buffer_object2;

 /* GL_NV_path_rendering */
extern GLboolean GLAD_NV_path_rendering;
#define GL_PATH_FORMAT_SVG_NV                  0x9070
#define GL_PATH_FORMAT_PS_NV                   0x9071
#define GL_STANDARD_FONT_NAME_NV               0x9072
#define GL_SYSTEM_FONT_NAME_NV                 0x9073
#define GL_FILE_NAME_NV                        0x9074
#define GL_PATH_STROKE_WIDTH_NV                0x9075
#define GL_PATH_END_CAPS_NV                    0x9076
#define GL_PATH_INITIAL_END_CAP_NV             0x9077
#define GL_PATH_TERMINAL_END_CAP_NV            0x9078
#define GL_PATH_JOIN_STYLE_NV                  0x9079
#define GL_PATH_MITER_LIMIT_NV                 0x907A
#define GL_PATH_DASH_CAPS_NV                   0x907B
#define GL_PATH_INITIAL_DASH_CAP_NV            0x907C
#define GL_PATH_TERMINAL_DASH_CAP_NV           0x907D
#define GL_PATH_DASH_OFFSET_NV                 0x907E
#define GL_PATH_CLIENT_LENGTH_NV               0x907F
#define GL_PATH_FILL_MODE_NV                   0x9080
#define GL_PATH_FILL_MASK_NV                   0x9081
#define GL_PATH_FILL_COVER_MODE_NV             0x9082
#define GL_PATH_STROKE_COVER_MODE_NV           0x9083
#define GL_PATH_STROKE_MASK_NV                 0x9084
#define GL_COUNT_UP_NV                         0x9088
#define GL_COUNT_DOWN_NV                       0x9089
#define GL_PATH_OBJECT_BOUNDING_BOX_NV         0x908A
#define GL_CONVEX_HULL_NV                      0x908B
#define GL_BOUNDING_BOX_NV                     0x908D
#define GL_TRANSLATE_X_NV                      0x908E
#define GL_TRANSLATE_Y_NV                      0x908F
#define GL_TRANSLATE_2D_NV                     0x9090
#define GL_TRANSLATE_3D_NV                     0x9091
#define GL_AFFINE_2D_NV                        0x9092
#define GL_AFFINE_3D_NV                        0x9094
#define GL_TRANSPOSE_AFFINE_2D_NV              0x9096
#define GL_TRANSPOSE_AFFINE_3D_NV              0x9098
#define GL_UTF8_NV                             0x909A
#define GL_UTF16_NV                            0x909B
#define GL_BOUNDING_BOX_OF_BOUNDING_BOXES_NV   0x909C
#define GL_PATH_COMMAND_COUNT_NV               0x909D
#define GL_PATH_COORD_COUNT_NV                 0x909E
#define GL_PATH_DASH_ARRAY_COUNT_NV            0x909F
#define GL_PATH_COMPUTED_LENGTH_NV             0x90A0
#define GL_PATH_FILL_BOUNDING_BOX_NV           0x90A1
#define GL_PATH_STROKE_BOUNDING_BOX_NV         0x90A2
#define GL_SQUARE_NV                           0x90A3
#define GL_ROUND_NV                            0x90A4
#define GL_TRIANGULAR_NV                       0x90A5
#define GL_BEVEL_NV                            0x90A6
#define GL_MITER_REVERT_NV                     0x90A7
#define GL_MITER_TRUNCATE_NV                   0x90A8
#define GL_SKIP_MISSING_GLYPH_NV               0x90A9
#define GL_USE_MISSING_GLYPH_NV                0x90AA
#define GL_PATH_ERROR_POSITION_NV              0x90AB
#define GL_ACCUM_ADJACENT_PAIRS_NV             0x90AD
#define GL_ADJACENT_PAIRS_NV                   0x90AE
#define GL_FIRST_TO_REST_NV                    0x90AF
#define GL_PATH_GEN_MODE_NV                    0x90B0
#define GL_PATH_GEN_COEFF_NV                   0x90B1
#define GL_PATH_GEN_COMPONENTS_NV              0x90B3
#define GL_PATH_STENCIL_FUNC_NV                0x90B7
#define GL_PATH_STENCIL_REF_NV                 0x90B8
#define GL_PATH_STENCIL_VALUE_MASK_NV          0x90B9
#define GL_PATH_STENCIL_DEPTH_OFFSET_FACTOR_NV  0x90BD
#define GL_PATH_STENCIL_DEPTH_OFFSET_UNITS_NV  0x90BE
#define GL_PATH_COVER_DEPTH_FUNC_NV            0x90BF
#define GL_PATH_DASH_OFFSET_RESET_NV           0x90B4
#define GL_MOVE_TO_RESETS_NV                   0x90B5
#define GL_MOVE_TO_CONTINUES_NV                0x90B6
#define GL_CLOSE_PATH_NV                       0x00
#define GL_MOVE_TO_NV                          0x02
#define GL_RELATIVE_MOVE_TO_NV                 0x03
#define GL_LINE_TO_NV                          0x04
#define GL_RELATIVE_LINE_TO_NV                 0x05
#define GL_HORIZONTAL_LINE_TO_NV               0x06
#define GL_RELATIVE_HORIZONTAL_LINE_TO_NV      0x07
#define GL_VERTICAL_LINE_TO_NV                 0x08
#define GL_RELATIVE_VERTICAL_LINE_TO_NV        0x09
#define GL_QUADRATIC_CURVE_TO_NV               0x0A
#define GL_RELATIVE_QUADRATIC_CURVE_TO_NV      0x0B
#define GL_CUBIC_CURVE_TO_NV                   0x0C
#define GL_RELATIVE_CUBIC_CURVE_TO_NV          0x0D
#define GL_SMOOTH_QUADRATIC_CURVE_TO_NV        0x0E
#define GL_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO_NV  0x0F
#define GL_SMOOTH_CUBIC_CURVE_TO_NV            0x10
#define GL_RELATIVE_SMOOTH_CUBIC_CURVE_TO_NV   0x11
#define GL_SMALL_CCW_ARC_TO_NV                 0x12
#define GL_RELATIVE_SMALL_CCW_ARC_TO_NV        0x13
#define GL_SMALL_CW_ARC_TO_NV                  0x14
#define GL_RELATIVE_SMALL_CW_ARC_TO_NV         0x15
#define GL_LARGE_CCW_ARC_TO_NV                 0x16
#define GL_RELATIVE_LARGE_CCW_ARC_TO_NV        0x17
#define GL_LARGE_CW_ARC_TO_NV                  0x18
#define GL_RELATIVE_LARGE_CW_ARC_TO_NV         0x19
#define GL_RESTART_PATH_NV                     0xF0
#define GL_DUP_FIRST_CUBIC_CURVE_TO_NV         0xF2
#define GL_DUP_LAST_CUBIC_CURVE_TO_NV          0xF4
#define GL_RECT_NV                             0xF6
#define GL_CIRCULAR_CCW_ARC_TO_NV              0xF8
#define GL_CIRCULAR_CW_ARC_TO_NV               0xFA
#define GL_CIRCULAR_TANGENT_ARC_TO_NV          0xFC
#define GL_ARC_TO_NV                           0xFE
#define GL_RELATIVE_ARC_TO_NV                  0xFF
#define GL_BOLD_BIT_NV                         0x01
#define GL_ITALIC_BIT_NV                       0x02
#define GL_GLYPH_WIDTH_BIT_NV                  0x01
#define GL_GLYPH_HEIGHT_BIT_NV                 0x02
#define GL_GLYPH_HORIZONTAL_BEARING_X_BIT_NV   0x04
#define GL_GLYPH_HORIZONTAL_BEARING_Y_BIT_NV   0x08
#define GL_GLYPH_HORIZONTAL_BEARING_ADVANCE_BIT_NV  0x10
#define GL_GLYPH_VERTICAL_BEARING_X_BIT_NV     0x20
#define GL_GLYPH_VERTICAL_BEARING_Y_BIT_NV     0x40
#define GL_GLYPH_VERTICAL_BEARING_ADVANCE_BIT_NV  0x80
#define GL_GLYPH_HAS_KERNING_BIT_NV            0x100
#define GL_FONT_X_MIN_BOUNDS_BIT_NV            0x00010000
#define GL_FONT_Y_MIN_BOUNDS_BIT_NV            0x00020000
#define GL_FONT_X_MAX_BOUNDS_BIT_NV            0x00040000
#define GL_FONT_Y_MAX_BOUNDS_BIT_NV            0x00080000
#define GL_FONT_UNITS_PER_EM_BIT_NV            0x00100000
#define GL_FONT_ASCENDER_BIT_NV                0x00200000
#define GL_FONT_DESCENDER_BIT_NV               0x00400000
#define GL_FONT_HEIGHT_BIT_NV                  0x00800000
#define GL_FONT_MAX_ADVANCE_WIDTH_BIT_NV       0x01000000
#define GL_FONT_MAX_ADVANCE_HEIGHT_BIT_NV      0x02000000
#define GL_FONT_UNDERLINE_POSITION_BIT_NV      0x04000000
#define GL_FONT_UNDERLINE_THICKNESS_BIT_NV     0x08000000
#define GL_FONT_HAS_KERNING_BIT_NV             0x10000000
#define GL_ROUNDED_RECT_NV                     0xE8
#define GL_RELATIVE_ROUNDED_RECT_NV            0xE9
#define GL_ROUNDED_RECT2_NV                    0xEA
#define GL_RELATIVE_ROUNDED_RECT2_NV           0xEB
#define GL_ROUNDED_RECT4_NV                    0xEC
#define GL_RELATIVE_ROUNDED_RECT4_NV           0xED
#define GL_ROUNDED_RECT8_NV                    0xEE
#define GL_RELATIVE_ROUNDED_RECT8_NV           0xEF
#define GL_RELATIVE_RECT_NV                    0xF7
#define GL_FONT_GLYPHS_AVAILABLE_NV            0x9368
#define GL_FONT_TARGET_UNAVAILABLE_NV          0x9369
#define GL_FONT_UNAVAILABLE_NV                 0x936A
#define GL_FONT_UNINTELLIGIBLE_NV              0x936B
#define GL_CONIC_CURVE_TO_NV                   0x1A
#define GL_RELATIVE_CONIC_CURVE_TO_NV          0x1B
#define GL_FONT_NUM_GLYPH_INDICES_BIT_NV       0x20000000
#define GL_STANDARD_FONT_FORMAT_NV             0x936C
#define GL_2_BYTES_NV                          0x1407
#define GL_3_BYTES_NV                          0x1408
#define GL_4_BYTES_NV                          0x1409
#define GL_EYE_LINEAR_NV                       0x2400
#define GL_OBJECT_LINEAR_NV                    0x2401
#define GL_CONSTANT_NV                         0x8576
#define GL_PATH_FOG_GEN_MODE_NV                0x90AC
#define GL_PRIMARY_COLOR_NV                    0x852C
#define GL_SECONDARY_COLOR_NV                  0x852D
#define GL_PATH_GEN_COLOR_FORMAT_NV            0x90B2
#define GL_PATH_PROJECTION_NV                  0x1701
#define GL_PATH_MODELVIEW_NV                   0x1700
#define GL_PATH_MODELVIEW_STACK_DEPTH_NV       0x0BA3
#define GL_PATH_MODELVIEW_MATRIX_NV            0x0BA6
#define GL_PATH_MAX_MODELVIEW_STACK_DEPTH_NV   0x0D36
#define GL_PATH_TRANSPOSE_MODELVIEW_MATRIX_NV  0x84E3
#define GL_PATH_PROJECTION_STACK_DEPTH_NV      0x0BA4
#define GL_PATH_PROJECTION_MATRIX_NV           0x0BA7
#define GL_PATH_MAX_PROJECTION_STACK_DEPTH_NV  0x0D38
#define GL_PATH_TRANSPOSE_PROJECTION_MATRIX_NV  0x84E4
#define GL_FRAGMENT_INPUT_NV                   0x936D
typedef GLuint (APIENTRYP pfn_glGenPathsNV) (GLsizei);
extern pfn_glGenPathsNV fp_glGenPathsNV;
typedef void (APIENTRYP pfn_glDeletePathsNV) (GLuint, GLsizei);
extern pfn_glDeletePathsNV fp_glDeletePathsNV;
typedef GLboolean (APIENTRYP pfn_glIsPathNV) (GLuint);
extern pfn_glIsPathNV fp_glIsPathNV;
typedef void (APIENTRYP pfn_glPathCommandsNV) (GLuint, GLsizei, const GLubyte*, GLsizei, GLenum, const void*);
extern pfn_glPathCommandsNV fp_glPathCommandsNV;
typedef void (APIENTRYP pfn_glPathCoordsNV) (GLuint, GLsizei, GLenum, const void*);
extern pfn_glPathCoordsNV fp_glPathCoordsNV;
typedef void (APIENTRYP pfn_glPathSubCommandsNV) (GLuint, GLsizei, GLsizei, GLsizei, const GLubyte*, GLsizei, GLenum, const void*);
extern pfn_glPathSubCommandsNV fp_glPathSubCommandsNV;
typedef void (APIENTRYP pfn_glPathSubCoordsNV) (GLuint, GLsizei, GLsizei, GLenum, const void*);
extern pfn_glPathSubCoordsNV fp_glPathSubCoordsNV;
typedef void (APIENTRYP pfn_glPathStringNV) (GLuint, GLenum, GLsizei, const void*);
extern pfn_glPathStringNV fp_glPathStringNV;
typedef void (APIENTRYP pfn_glPathGlyphsNV) (GLuint, GLenum, const void*, GLbitfield, GLsizei, GLenum, const void*, GLenum, GLuint, GLfloat);
extern pfn_glPathGlyphsNV fp_glPathGlyphsNV;
typedef void (APIENTRYP pfn_glPathGlyphRangeNV) (GLuint, GLenum, const void*, GLbitfield, GLuint, GLsizei, GLenum, GLuint, GLfloat);
extern pfn_glPathGlyphRangeNV fp_glPathGlyphRangeNV;
typedef void (APIENTRYP pfn_glWeightPathsNV) (GLuint, GLsizei, const GLuint*, const GLfloat*);
extern pfn_glWeightPathsNV fp_glWeightPathsNV;
typedef void (APIENTRYP pfn_glCopyPathNV) (GLuint, GLuint);
extern pfn_glCopyPathNV fp_glCopyPathNV;
typedef void (APIENTRYP pfn_glInterpolatePathsNV) (GLuint, GLuint, GLuint, GLfloat);
extern pfn_glInterpolatePathsNV fp_glInterpolatePathsNV;
typedef void (APIENTRYP pfn_glTransformPathNV) (GLuint, GLuint, GLenum, const GLfloat*);
extern pfn_glTransformPathNV fp_glTransformPathNV;
typedef void (APIENTRYP pfn_glPathParameterivNV) (GLuint, GLenum, const GLint*);
extern pfn_glPathParameterivNV fp_glPathParameterivNV;
typedef void (APIENTRYP pfn_glPathParameteriNV) (GLuint, GLenum, GLint);
extern pfn_glPathParameteriNV fp_glPathParameteriNV;
typedef void (APIENTRYP pfn_glPathParameterfvNV) (GLuint, GLenum, const GLfloat*);
extern pfn_glPathParameterfvNV fp_glPathParameterfvNV;
typedef void (APIENTRYP pfn_glPathParameterfNV) (GLuint, GLenum, GLfloat);
extern pfn_glPathParameterfNV fp_glPathParameterfNV;
typedef void (APIENTRYP pfn_glPathDashArrayNV) (GLuint, GLsizei, const GLfloat*);
extern pfn_glPathDashArrayNV fp_glPathDashArrayNV;
typedef void (APIENTRYP pfn_glPathStencilFuncNV) (GLenum, GLint, GLuint);
extern pfn_glPathStencilFuncNV fp_glPathStencilFuncNV;
typedef void (APIENTRYP pfn_glPathStencilDepthOffsetNV) (GLfloat, GLfloat);
extern pfn_glPathStencilDepthOffsetNV fp_glPathStencilDepthOffsetNV;
typedef void (APIENTRYP pfn_glStencilFillPathNV) (GLuint, GLenum, GLuint);
extern pfn_glStencilFillPathNV fp_glStencilFillPathNV;
typedef void (APIENTRYP pfn_glStencilStrokePathNV) (GLuint, GLint, GLuint);
extern pfn_glStencilStrokePathNV fp_glStencilStrokePathNV;
typedef void (APIENTRYP pfn_glStencilFillPathInstancedNV) (GLsizei, GLenum, const void*, GLuint, GLenum, GLuint, GLenum, const GLfloat*);
extern pfn_glStencilFillPathInstancedNV fp_glStencilFillPathInstancedNV;
typedef void (APIENTRYP pfn_glStencilStrokePathInstancedNV) (GLsizei, GLenum, const void*, GLuint, GLint, GLuint, GLenum, const GLfloat*);
extern pfn_glStencilStrokePathInstancedNV fp_glStencilStrokePathInstancedNV;
typedef void (APIENTRYP pfn_glPathCoverDepthFuncNV) (GLenum);
extern pfn_glPathCoverDepthFuncNV fp_glPathCoverDepthFuncNV;
typedef void (APIENTRYP pfn_glCoverFillPathNV) (GLuint, GLenum);
extern pfn_glCoverFillPathNV fp_glCoverFillPathNV;
typedef void (APIENTRYP pfn_glCoverStrokePathNV) (GLuint, GLenum);
extern pfn_glCoverStrokePathNV fp_glCoverStrokePathNV;
typedef void (APIENTRYP pfn_glCoverFillPathInstancedNV) (GLsizei, GLenum, const void*, GLuint, GLenum, GLenum, const GLfloat*);
extern pfn_glCoverFillPathInstancedNV fp_glCoverFillPathInstancedNV;
typedef void (APIENTRYP pfn_glCoverStrokePathInstancedNV) (GLsizei, GLenum, const void*, GLuint, GLenum, GLenum, const GLfloat*);
extern pfn_glCoverStrokePathInstancedNV fp_glCoverStrokePathInstancedNV;
typedef void (APIENTRYP pfn_glGetPathParameterivNV) (GLuint, GLenum, GLint*);
extern pfn_glGetPathParameterivNV fp_glGetPathParameterivNV;
typedef void (APIENTRYP pfn_glGetPathParameterfvNV) (GLuint, GLenum, GLfloat*);
extern pfn_glGetPathParameterfvNV fp_glGetPathParameterfvNV;
typedef void (APIENTRYP pfn_glGetPathCommandsNV) (GLuint, GLubyte*);
extern pfn_glGetPathCommandsNV fp_glGetPathCommandsNV;
typedef void (APIENTRYP pfn_glGetPathCoordsNV) (GLuint, GLfloat*);
extern pfn_glGetPathCoordsNV fp_glGetPathCoordsNV;
typedef void (APIENTRYP pfn_glGetPathDashArrayNV) (GLuint, GLfloat*);
extern pfn_glGetPathDashArrayNV fp_glGetPathDashArrayNV;
typedef void (APIENTRYP pfn_glGetPathMetricsNV) (GLbitfield, GLsizei, GLenum, const void*, GLuint, GLsizei, GLfloat*);
extern pfn_glGetPathMetricsNV fp_glGetPathMetricsNV;
typedef void (APIENTRYP pfn_glGetPathMetricRangeNV) (GLbitfield, GLuint, GLsizei, GLsizei, GLfloat*);
extern pfn_glGetPathMetricRangeNV fp_glGetPathMetricRangeNV;
typedef void (APIENTRYP pfn_glGetPathSpacingNV) (GLenum, GLsizei, GLenum, const void*, GLuint, GLfloat, GLfloat, GLenum, GLfloat*);
extern pfn_glGetPathSpacingNV fp_glGetPathSpacingNV;
typedef GLboolean (APIENTRYP pfn_glIsPointInFillPathNV) (GLuint, GLuint, GLfloat, GLfloat);
extern pfn_glIsPointInFillPathNV fp_glIsPointInFillPathNV;
typedef GLboolean (APIENTRYP pfn_glIsPointInStrokePathNV) (GLuint, GLfloat, GLfloat);
extern pfn_glIsPointInStrokePathNV fp_glIsPointInStrokePathNV;
typedef GLfloat (APIENTRYP pfn_glGetPathLengthNV) (GLuint, GLsizei, GLsizei);
extern pfn_glGetPathLengthNV fp_glGetPathLengthNV;
typedef GLboolean (APIENTRYP pfn_glPointAlongPathNV) (GLuint, GLsizei, GLsizei, GLfloat, GLfloat*, GLfloat*, GLfloat*, GLfloat*);
extern pfn_glPointAlongPathNV fp_glPointAlongPathNV;
typedef void (APIENTRYP pfn_glMatrixLoad3x2fNV) (GLenum, const GLfloat*);
extern pfn_glMatrixLoad3x2fNV fp_glMatrixLoad3x2fNV;
typedef void (APIENTRYP pfn_glMatrixLoad3x3fNV) (GLenum, const GLfloat*);
extern pfn_glMatrixLoad3x3fNV fp_glMatrixLoad3x3fNV;
typedef void (APIENTRYP pfn_glMatrixLoadTranspose3x3fNV) (GLenum, const GLfloat*);
extern pfn_glMatrixLoadTranspose3x3fNV fp_glMatrixLoadTranspose3x3fNV;
typedef void (APIENTRYP pfn_glMatrixMult3x2fNV) (GLenum, const GLfloat*);
extern pfn_glMatrixMult3x2fNV fp_glMatrixMult3x2fNV;
typedef void (APIENTRYP pfn_glMatrixMult3x3fNV) (GLenum, const GLfloat*);
extern pfn_glMatrixMult3x3fNV fp_glMatrixMult3x3fNV;
typedef void (APIENTRYP pfn_glMatrixMultTranspose3x3fNV) (GLenum, const GLfloat*);
extern pfn_glMatrixMultTranspose3x3fNV fp_glMatrixMultTranspose3x3fNV;
typedef void (APIENTRYP pfn_glStencilThenCoverFillPathNV) (GLuint, GLenum, GLuint, GLenum);
extern pfn_glStencilThenCoverFillPathNV fp_glStencilThenCoverFillPathNV;
typedef void (APIENTRYP pfn_glStencilThenCoverStrokePathNV) (GLuint, GLint, GLuint, GLenum);
extern pfn_glStencilThenCoverStrokePathNV fp_glStencilThenCoverStrokePathNV;
typedef void (APIENTRYP pfn_glStencilThenCoverFillPathInstancedNV) (GLsizei, GLenum, const void*, GLuint, GLenum, GLuint, GLenum, GLenum, const GLfloat*);
extern pfn_glStencilThenCoverFillPathInstancedNV fp_glStencilThenCoverFillPathInstancedNV;
typedef void (APIENTRYP pfn_glStencilThenCoverStrokePathInstancedNV) (GLsizei, GLenum, const void*, GLuint, GLint, GLuint, GLenum, GLenum, const GLfloat*);
extern pfn_glStencilThenCoverStrokePathInstancedNV fp_glStencilThenCoverStrokePathInstancedNV;
typedef GLenum (APIENTRYP pfn_glPathGlyphIndexRangeNV) (GLenum, const void*, GLbitfield, GLuint, GLfloat, GLuint);
extern pfn_glPathGlyphIndexRangeNV fp_glPathGlyphIndexRangeNV;
typedef GLenum (APIENTRYP pfn_glPathGlyphIndexArrayNV) (GLuint, GLenum, const void*, GLbitfield, GLuint, GLsizei, GLuint, GLfloat);
extern pfn_glPathGlyphIndexArrayNV fp_glPathGlyphIndexArrayNV;
typedef GLenum (APIENTRYP pfn_glPathMemoryGlyphIndexArrayNV) (GLuint, GLenum, GLsizeiptr, const void*, GLsizei, GLuint, GLsizei, GLuint, GLfloat);
extern pfn_glPathMemoryGlyphIndexArrayNV fp_glPathMemoryGlyphIndexArrayNV;
typedef void (APIENTRYP pfn_glProgramPathFragmentInputGenNV) (GLuint, GLint, GLenum, GLint, const GLfloat*);
extern pfn_glProgramPathFragmentInputGenNV fp_glProgramPathFragmentInputGenNV;
typedef void (APIENTRYP pfn_glGetProgramResourcefvNV) (GLuint, GLenum, GLuint, GLsizei, const GLenum*, GLsizei, GLsizei*, GLfloat*);
extern pfn_glGetProgramResourcefvNV fp_glGetProgramResourcefvNV;
typedef void (APIENTRYP pfn_glPathColorGenNV) (GLenum, GLenum, GLenum, const GLfloat*);
extern pfn_glPathColorGenNV fp_glPathColorGenNV;
typedef void (APIENTRYP pfn_glPathTexGenNV) (GLenum, GLenum, GLint, const GLfloat*);
extern pfn_glPathTexGenNV fp_glPathTexGenNV;
typedef void (APIENTRYP pfn_glPathFogGenNV) (GLenum);
extern pfn_glPathFogGenNV fp_glPathFogGenNV;
typedef void (APIENTRYP pfn_glGetPathColorGenivNV) (GLenum, GLenum, GLint*);
extern pfn_glGetPathColorGenivNV fp_glGetPathColorGenivNV;
typedef void (APIENTRYP pfn_glGetPathColorGenfvNV) (GLenum, GLenum, GLfloat*);
extern pfn_glGetPathColorGenfvNV fp_glGetPathColorGenfvNV;
typedef void (APIENTRYP pfn_glGetPathTexGenivNV) (GLenum, GLenum, GLint*);
extern pfn_glGetPathTexGenivNV fp_glGetPathTexGenivNV;
typedef void (APIENTRYP pfn_glGetPathTexGenfvNV) (GLenum, GLenum, GLfloat*);
extern pfn_glGetPathTexGenfvNV fp_glGetPathTexGenfvNV;

 /* GL_NV_path_rendering_shared_edge */
extern GLboolean GLAD_NV_path_rendering_shared_edge;
#define GL_SHARED_EDGE_NV                      0xC0

 /* GL_NV_polygon_mode */
extern GLboolean GLAD_NV_polygon_mode;
#define GL_POLYGON_MODE_NV                     0x0B40
#define GL_POLYGON_OFFSET_POINT_NV             0x2A01
#define GL_POLYGON_OFFSET_LINE_NV              0x2A02
#define GL_POINT_NV                            0x1B00
#define GL_LINE_NV                             0x1B01
#define GL_FILL_NV                             0x1B02
typedef void (APIENTRYP pfn_glPolygonModeNV) (GLenum, GLenum);
extern pfn_glPolygonModeNV fp_glPolygonModeNV;

 /* GL_NV_present_video */
extern GLboolean GLAD_NV_present_video;
#define GL_FRAME_NV                            0x8E26
#define GL_FIELDS_NV                           0x8E27
#define GL_CURRENT_TIME_NV                     0x8E28
#define GL_NUM_FILL_STREAMS_NV                 0x8E29
#define GL_PRESENT_TIME_NV                     0x8E2A
#define GL_PRESENT_DURATION_NV                 0x8E2B
typedef void (APIENTRYP pfn_glPresentFrameKeyedNV) (GLuint, GLuint64EXT, GLuint, GLuint, GLenum, GLenum, GLuint, GLuint, GLenum, GLuint, GLuint);
extern pfn_glPresentFrameKeyedNV fp_glPresentFrameKeyedNV;
typedef void (APIENTRYP pfn_glPresentFrameDualFillNV) (GLuint, GLuint64EXT, GLuint, GLuint, GLenum, GLenum, GLuint, GLenum, GLuint, GLenum, GLuint, GLenum, GLuint);
extern pfn_glPresentFrameDualFillNV fp_glPresentFrameDualFillNV;
typedef void (APIENTRYP pfn_glGetVideoivNV) (GLuint, GLenum, GLint*);
extern pfn_glGetVideoivNV fp_glGetVideoivNV;
typedef void (APIENTRYP pfn_glGetVideouivNV) (GLuint, GLenum, GLuint*);
extern pfn_glGetVideouivNV fp_glGetVideouivNV;
typedef void (APIENTRYP pfn_glGetVideoi64vNV) (GLuint, GLenum, GLint64EXT*);
extern pfn_glGetVideoi64vNV fp_glGetVideoi64vNV;
typedef void (APIENTRYP pfn_glGetVideoui64vNV) (GLuint, GLenum, GLuint64EXT*);
extern pfn_glGetVideoui64vNV fp_glGetVideoui64vNV;

 /* GL_NV_primitive_restart */
extern GLboolean GLAD_NV_primitive_restart;
#define GL_PRIMITIVE_RESTART_NV                0x8558
#define GL_PRIMITIVE_RESTART_INDEX_NV          0x8559
typedef void (APIENTRYP pfn_glPrimitiveRestartNV) ();
extern pfn_glPrimitiveRestartNV fp_glPrimitiveRestartNV;
typedef void (APIENTRYP pfn_glPrimitiveRestartIndexNV) (GLuint);
extern pfn_glPrimitiveRestartIndexNV fp_glPrimitiveRestartIndexNV;

 /* GL_NV_read_buffer */
extern GLboolean GLAD_NV_read_buffer;
#define GL_READ_BUFFER_NV                      0x0C02
typedef void (APIENTRYP pfn_glReadBufferNV) (GLenum);
extern pfn_glReadBufferNV fp_glReadBufferNV;

 /* GL_NV_read_buffer_front */
extern GLboolean GLAD_NV_read_buffer_front;

 /* GL_NV_read_depth */
extern GLboolean GLAD_NV_read_depth;

 /* GL_NV_read_depth_stencil */
extern GLboolean GLAD_NV_read_depth_stencil;

 /* GL_NV_read_stencil */
extern GLboolean GLAD_NV_read_stencil;

 /* GL_NV_sample_locations */
extern GLboolean GLAD_NV_sample_locations;
#define GL_SAMPLE_LOCATION_SUBPIXEL_BITS_NV    0x933D
#define GL_SAMPLE_LOCATION_PIXEL_GRID_WIDTH_NV  0x933E
#define GL_SAMPLE_LOCATION_PIXEL_GRID_HEIGHT_NV  0x933F
#define GL_PROGRAMMABLE_SAMPLE_LOCATION_TABLE_SIZE_NV  0x9340
#define GL_SAMPLE_LOCATION_NV                  0x8E50
#define GL_PROGRAMMABLE_SAMPLE_LOCATION_NV     0x9341
#define GL_FRAMEBUFFER_PROGRAMMABLE_SAMPLE_LOCATIONS_NV  0x9342
#define GL_FRAMEBUFFER_SAMPLE_LOCATION_PIXEL_GRID_NV  0x9343
typedef void (APIENTRYP pfn_glFramebufferSampleLocationsfvNV) (GLenum, GLuint, GLsizei, const GLfloat*);
extern pfn_glFramebufferSampleLocationsfvNV fp_glFramebufferSampleLocationsfvNV;
typedef void (APIENTRYP pfn_glNamedFramebufferSampleLocationsfvNV) (GLuint, GLuint, GLsizei, const GLfloat*);
extern pfn_glNamedFramebufferSampleLocationsfvNV fp_glNamedFramebufferSampleLocationsfvNV;
typedef void (APIENTRYP pfn_glResolveDepthValuesNV) ();
extern pfn_glResolveDepthValuesNV fp_glResolveDepthValuesNV;

 /* GL_NV_sample_mask_override_coverage */
extern GLboolean GLAD_NV_sample_mask_override_coverage;

 /* GL_NV_shader_atomic_counters */
extern GLboolean GLAD_NV_shader_atomic_counters;

 /* GL_NV_shader_atomic_float */
extern GLboolean GLAD_NV_shader_atomic_float;

 /* GL_NV_shader_atomic_fp16_vector */
extern GLboolean GLAD_NV_shader_atomic_fp16_vector;

 /* GL_NV_shader_atomic_int64 */
extern GLboolean GLAD_NV_shader_atomic_int64;

 /* GL_NV_shader_buffer_load */
extern GLboolean GLAD_NV_shader_buffer_load;
#define GL_BUFFER_GPU_ADDRESS_NV               0x8F1D
#define GL_GPU_ADDRESS_NV                      0x8F34
#define GL_MAX_SHADER_BUFFER_ADDRESS_NV        0x8F35
typedef void (APIENTRYP pfn_glMakeBufferResidentNV) (GLenum, GLenum);
extern pfn_glMakeBufferResidentNV fp_glMakeBufferResidentNV;
typedef void (APIENTRYP pfn_glMakeBufferNonResidentNV) (GLenum);
extern pfn_glMakeBufferNonResidentNV fp_glMakeBufferNonResidentNV;
typedef GLboolean (APIENTRYP pfn_glIsBufferResidentNV) (GLenum);
extern pfn_glIsBufferResidentNV fp_glIsBufferResidentNV;
typedef void (APIENTRYP pfn_glMakeNamedBufferResidentNV) (GLuint, GLenum);
extern pfn_glMakeNamedBufferResidentNV fp_glMakeNamedBufferResidentNV;
typedef void (APIENTRYP pfn_glMakeNamedBufferNonResidentNV) (GLuint);
extern pfn_glMakeNamedBufferNonResidentNV fp_glMakeNamedBufferNonResidentNV;
typedef GLboolean (APIENTRYP pfn_glIsNamedBufferResidentNV) (GLuint);
extern pfn_glIsNamedBufferResidentNV fp_glIsNamedBufferResidentNV;
typedef void (APIENTRYP pfn_glGetBufferParameterui64vNV) (GLenum, GLenum, GLuint64EXT*);
extern pfn_glGetBufferParameterui64vNV fp_glGetBufferParameterui64vNV;
typedef void (APIENTRYP pfn_glGetNamedBufferParameterui64vNV) (GLuint, GLenum, GLuint64EXT*);
extern pfn_glGetNamedBufferParameterui64vNV fp_glGetNamedBufferParameterui64vNV;
typedef void (APIENTRYP pfn_glGetIntegerui64vNV) (GLenum, GLuint64EXT*);
extern pfn_glGetIntegerui64vNV fp_glGetIntegerui64vNV;
typedef void (APIENTRYP pfn_glUniformui64NV) (GLint, GLuint64EXT);
extern pfn_glUniformui64NV fp_glUniformui64NV;
typedef void (APIENTRYP pfn_glUniformui64vNV) (GLint, GLsizei, const GLuint64EXT*);
extern pfn_glUniformui64vNV fp_glUniformui64vNV;
typedef void (APIENTRYP pfn_glProgramUniformui64NV) (GLuint, GLint, GLuint64EXT);
extern pfn_glProgramUniformui64NV fp_glProgramUniformui64NV;
typedef void (APIENTRYP pfn_glProgramUniformui64vNV) (GLuint, GLint, GLsizei, const GLuint64EXT*);
extern pfn_glProgramUniformui64vNV fp_glProgramUniformui64vNV;

 /* GL_NV_shader_buffer_store */
extern GLboolean GLAD_NV_shader_buffer_store;
#define GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV  0x00000010

 /* GL_NV_shader_noperspective_interpolation */
extern GLboolean GLAD_NV_shader_noperspective_interpolation;

 /* GL_NV_shader_storage_buffer_object */
extern GLboolean GLAD_NV_shader_storage_buffer_object;

 /* GL_NV_shader_thread_group */
extern GLboolean GLAD_NV_shader_thread_group;
#define GL_WARP_SIZE_NV                        0x9339
#define GL_WARPS_PER_SM_NV                     0x933A
#define GL_SM_COUNT_NV                         0x933B

 /* GL_NV_shader_thread_shuffle */
extern GLboolean GLAD_NV_shader_thread_shuffle;

 /* GL_NV_shadow_samplers_array */
extern GLboolean GLAD_NV_shadow_samplers_array;
#define GL_SAMPLER_2D_ARRAY_SHADOW_NV          0x8DC4

 /* GL_NV_shadow_samplers_cube */
extern GLboolean GLAD_NV_shadow_samplers_cube;
#define GL_SAMPLER_CUBE_SHADOW_NV              0x8DC5

 /* GL_NV_sRGB_formats */
extern GLboolean GLAD_NV_sRGB_formats;
#define GL_SLUMINANCE_NV                       0x8C46
#define GL_SLUMINANCE_ALPHA_NV                 0x8C44
#define GL_SRGB8_NV                            0x8C41
#define GL_SLUMINANCE8_NV                      0x8C47
#define GL_SLUMINANCE8_ALPHA8_NV               0x8C45
#define GL_COMPRESSED_SRGB_S3TC_DXT1_NV        0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_NV  0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_NV  0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_NV  0x8C4F
#define GL_ETC1_SRGB8_NV                       0x88EE

 /* GL_NV_tessellation_program5 */
extern GLboolean GLAD_NV_tessellation_program5;
#define GL_MAX_PROGRAM_PATCH_ATTRIBS_NV        0x86D8
#define GL_TESS_CONTROL_PROGRAM_NV             0x891E
#define GL_TESS_EVALUATION_PROGRAM_NV          0x891F
#define GL_TESS_CONTROL_PROGRAM_PARAMETER_BUFFER_NV  0x8C74
#define GL_TESS_EVALUATION_PROGRAM_PARAMETER_BUFFER_NV  0x8C75

 /* GL_NV_texture_barrier */
extern GLboolean GLAD_NV_texture_barrier;
typedef void (APIENTRYP pfn_glTextureBarrierNV) ();
extern pfn_glTextureBarrierNV fp_glTextureBarrierNV;

 /* GL_NV_texture_border_clamp */
extern GLboolean GLAD_NV_texture_border_clamp;
#define GL_TEXTURE_BORDER_COLOR_NV             0x1004
#define GL_CLAMP_TO_BORDER_NV                  0x812D

 /* GL_NV_texture_compression_s3tc_update */
extern GLboolean GLAD_NV_texture_compression_s3tc_update;

 /* GL_NV_texture_multisample */
extern GLboolean GLAD_NV_texture_multisample;
#define GL_TEXTURE_COVERAGE_SAMPLES_NV         0x9045
#define GL_TEXTURE_COLOR_SAMPLES_NV            0x9046
typedef void (APIENTRYP pfn_glTexImage2DMultisampleCoverageNV) (GLenum, GLsizei, GLsizei, GLint, GLsizei, GLsizei, GLboolean);
extern pfn_glTexImage2DMultisampleCoverageNV fp_glTexImage2DMultisampleCoverageNV;
typedef void (APIENTRYP pfn_glTexImage3DMultisampleCoverageNV) (GLenum, GLsizei, GLsizei, GLint, GLsizei, GLsizei, GLsizei, GLboolean);
extern pfn_glTexImage3DMultisampleCoverageNV fp_glTexImage3DMultisampleCoverageNV;
typedef void (APIENTRYP pfn_glTextureImage2DMultisampleNV) (GLuint, GLenum, GLsizei, GLint, GLsizei, GLsizei, GLboolean);
extern pfn_glTextureImage2DMultisampleNV fp_glTextureImage2DMultisampleNV;
typedef void (APIENTRYP pfn_glTextureImage3DMultisampleNV) (GLuint, GLenum, GLsizei, GLint, GLsizei, GLsizei, GLsizei, GLboolean);
extern pfn_glTextureImage3DMultisampleNV fp_glTextureImage3DMultisampleNV;
typedef void (APIENTRYP pfn_glTextureImage2DMultisampleCoverageNV) (GLuint, GLenum, GLsizei, GLsizei, GLint, GLsizei, GLsizei, GLboolean);
extern pfn_glTextureImage2DMultisampleCoverageNV fp_glTextureImage2DMultisampleCoverageNV;
typedef void (APIENTRYP pfn_glTextureImage3DMultisampleCoverageNV) (GLuint, GLenum, GLsizei, GLsizei, GLint, GLsizei, GLsizei, GLsizei, GLboolean);
extern pfn_glTextureImage3DMultisampleCoverageNV fp_glTextureImage3DMultisampleCoverageNV;

 /* GL_NV_texture_npot_2D_mipmap */
extern GLboolean GLAD_NV_texture_npot_2D_mipmap;

 /* GL_NV_transform_feedback */
extern GLboolean GLAD_NV_transform_feedback;
#define GL_BACK_PRIMARY_COLOR_NV               0x8C77
#define GL_BACK_SECONDARY_COLOR_NV             0x8C78
#define GL_TEXTURE_COORD_NV                    0x8C79
#define GL_CLIP_DISTANCE_NV                    0x8C7A
#define GL_VERTEX_ID_NV                        0x8C7B
#define GL_PRIMITIVE_ID_NV                     0x8C7C
#define GL_GENERIC_ATTRIB_NV                   0x8C7D
#define GL_TRANSFORM_FEEDBACK_ATTRIBS_NV       0x8C7E
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE_NV   0x8C7F
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_NV  0x8C80
#define GL_ACTIVE_VARYINGS_NV                  0x8C81
#define GL_ACTIVE_VARYING_MAX_LENGTH_NV        0x8C82
#define GL_TRANSFORM_FEEDBACK_VARYINGS_NV      0x8C83
#define GL_TRANSFORM_FEEDBACK_BUFFER_START_NV  0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE_NV   0x8C85
#define GL_TRANSFORM_FEEDBACK_RECORD_NV        0x8C86
#define GL_PRIMITIVES_GENERATED_NV             0x8C87
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV  0x8C88
#define GL_RASTERIZER_DISCARD_NV               0x8C89
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_NV  0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_NV  0x8C8B
#define GL_INTERLEAVED_ATTRIBS_NV              0x8C8C
#define GL_SEPARATE_ATTRIBS_NV                 0x8C8D
#define GL_TRANSFORM_FEEDBACK_BUFFER_NV        0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING_NV  0x8C8F
#define GL_LAYER_NV                            0x8DAA
#define GL_NEXT_BUFFER_NV                      -2
#define GL_SKIP_COMPONENTS4_NV                 -3
#define GL_SKIP_COMPONENTS3_NV                 -4
#define GL_SKIP_COMPONENTS2_NV                 -5
#define GL_SKIP_COMPONENTS1_NV                 -6
typedef void (APIENTRYP pfn_glBeginTransformFeedbackNV) (GLenum);
extern pfn_glBeginTransformFeedbackNV fp_glBeginTransformFeedbackNV;
typedef void (APIENTRYP pfn_glEndTransformFeedbackNV) ();
extern pfn_glEndTransformFeedbackNV fp_glEndTransformFeedbackNV;
typedef void (APIENTRYP pfn_glTransformFeedbackAttribsNV) (GLsizei, const GLint*, GLenum);
extern pfn_glTransformFeedbackAttribsNV fp_glTransformFeedbackAttribsNV;
typedef void (APIENTRYP pfn_glBindBufferRangeNV) (GLenum, GLuint, GLuint, GLintptr, GLsizeiptr);
extern pfn_glBindBufferRangeNV fp_glBindBufferRangeNV;
typedef void (APIENTRYP pfn_glBindBufferOffsetNV) (GLenum, GLuint, GLuint, GLintptr);
extern pfn_glBindBufferOffsetNV fp_glBindBufferOffsetNV;
typedef void (APIENTRYP pfn_glBindBufferBaseNV) (GLenum, GLuint, GLuint);
extern pfn_glBindBufferBaseNV fp_glBindBufferBaseNV;
typedef void (APIENTRYP pfn_glTransformFeedbackVaryingsNV) (GLuint, GLsizei, const GLint*, GLenum);
extern pfn_glTransformFeedbackVaryingsNV fp_glTransformFeedbackVaryingsNV;
typedef void (APIENTRYP pfn_glActiveVaryingNV) (GLuint, const GLchar*);
extern pfn_glActiveVaryingNV fp_glActiveVaryingNV;
typedef GLint (APIENTRYP pfn_glGetVaryingLocationNV) (GLuint, const GLchar*);
extern pfn_glGetVaryingLocationNV fp_glGetVaryingLocationNV;
typedef void (APIENTRYP pfn_glGetActiveVaryingNV) (GLuint, GLuint, GLsizei, GLsizei*, GLsizei*, GLenum*, GLchar*);
extern pfn_glGetActiveVaryingNV fp_glGetActiveVaryingNV;
typedef void (APIENTRYP pfn_glGetTransformFeedbackVaryingNV) (GLuint, GLuint, GLint*);
extern pfn_glGetTransformFeedbackVaryingNV fp_glGetTransformFeedbackVaryingNV;
typedef void (APIENTRYP pfn_glTransformFeedbackStreamAttribsNV) (GLsizei, const GLint*, GLsizei, const GLint*, GLenum);
extern pfn_glTransformFeedbackStreamAttribsNV fp_glTransformFeedbackStreamAttribsNV;

 /* GL_NV_transform_feedback2 */
extern GLboolean GLAD_NV_transform_feedback2;
#define GL_TRANSFORM_FEEDBACK_NV               0x8E22
#define GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED_NV  0x8E23
#define GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE_NV  0x8E24
#define GL_TRANSFORM_FEEDBACK_BINDING_NV       0x8E25
typedef void (APIENTRYP pfn_glBindTransformFeedbackNV) (GLenum, GLuint);
extern pfn_glBindTransformFeedbackNV fp_glBindTransformFeedbackNV;
typedef void (APIENTRYP pfn_glDeleteTransformFeedbacksNV) (GLsizei, const GLuint*);
extern pfn_glDeleteTransformFeedbacksNV fp_glDeleteTransformFeedbacksNV;
typedef void (APIENTRYP pfn_glGenTransformFeedbacksNV) (GLsizei, GLuint*);
extern pfn_glGenTransformFeedbacksNV fp_glGenTransformFeedbacksNV;
typedef GLboolean (APIENTRYP pfn_glIsTransformFeedbackNV) (GLuint);
extern pfn_glIsTransformFeedbackNV fp_glIsTransformFeedbackNV;
typedef void (APIENTRYP pfn_glPauseTransformFeedbackNV) ();
extern pfn_glPauseTransformFeedbackNV fp_glPauseTransformFeedbackNV;
typedef void (APIENTRYP pfn_glResumeTransformFeedbackNV) ();
extern pfn_glResumeTransformFeedbackNV fp_glResumeTransformFeedbackNV;
typedef void (APIENTRYP pfn_glDrawTransformFeedbackNV) (GLenum, GLuint);
extern pfn_glDrawTransformFeedbackNV fp_glDrawTransformFeedbackNV;

 /* GL_NV_uniform_buffer_unified_memory */
extern GLboolean GLAD_NV_uniform_buffer_unified_memory;
#define GL_UNIFORM_BUFFER_UNIFIED_NV           0x936E
#define GL_UNIFORM_BUFFER_ADDRESS_NV           0x936F
#define GL_UNIFORM_BUFFER_LENGTH_NV            0x9370

 /* GL_NV_vdpau_interop */
extern GLboolean GLAD_NV_vdpau_interop;
#define GL_SURFACE_STATE_NV                    0x86EB
#define GL_SURFACE_REGISTERED_NV               0x86FD
#define GL_SURFACE_MAPPED_NV                   0x8700
#define GL_WRITE_DISCARD_NV                    0x88BE
typedef void (APIENTRYP pfn_glVDPAUInitNV) (const void*, const void*);
extern pfn_glVDPAUInitNV fp_glVDPAUInitNV;
typedef void (APIENTRYP pfn_glVDPAUFiniNV) ();
extern pfn_glVDPAUFiniNV fp_glVDPAUFiniNV;
typedef GLvdpauSurfaceNV (APIENTRYP pfn_glVDPAURegisterVideoSurfaceNV) (const void*, GLenum, GLsizei, const GLuint*);
extern pfn_glVDPAURegisterVideoSurfaceNV fp_glVDPAURegisterVideoSurfaceNV;
typedef GLvdpauSurfaceNV (APIENTRYP pfn_glVDPAURegisterOutputSurfaceNV) (const void*, GLenum, GLsizei, const GLuint*);
extern pfn_glVDPAURegisterOutputSurfaceNV fp_glVDPAURegisterOutputSurfaceNV;
typedef GLboolean (APIENTRYP pfn_glVDPAUIsSurfaceNV) (GLvdpauSurfaceNV);
extern pfn_glVDPAUIsSurfaceNV fp_glVDPAUIsSurfaceNV;
typedef void (APIENTRYP pfn_glVDPAUUnregisterSurfaceNV) (GLvdpauSurfaceNV);
extern pfn_glVDPAUUnregisterSurfaceNV fp_glVDPAUUnregisterSurfaceNV;
typedef void (APIENTRYP pfn_glVDPAUGetSurfaceivNV) (GLvdpauSurfaceNV, GLenum, GLsizei, GLsizei*, GLint*);
extern pfn_glVDPAUGetSurfaceivNV fp_glVDPAUGetSurfaceivNV;
typedef void (APIENTRYP pfn_glVDPAUSurfaceAccessNV) (GLvdpauSurfaceNV, GLenum);
extern pfn_glVDPAUSurfaceAccessNV fp_glVDPAUSurfaceAccessNV;
typedef void (APIENTRYP pfn_glVDPAUMapSurfacesNV) (GLsizei, const GLvdpauSurfaceNV*);
extern pfn_glVDPAUMapSurfacesNV fp_glVDPAUMapSurfacesNV;
typedef void (APIENTRYP pfn_glVDPAUUnmapSurfacesNV) (GLsizei, const GLvdpauSurfaceNV*);
extern pfn_glVDPAUUnmapSurfacesNV fp_glVDPAUUnmapSurfacesNV;

 /* GL_NV_vertex_attrib_integer_64bit */
extern GLboolean GLAD_NV_vertex_attrib_integer_64bit;
typedef void (APIENTRYP pfn_glVertexAttribL1i64NV) (GLuint, GLint64EXT);
extern pfn_glVertexAttribL1i64NV fp_glVertexAttribL1i64NV;
typedef void (APIENTRYP pfn_glVertexAttribL2i64NV) (GLuint, GLint64EXT, GLint64EXT);
extern pfn_glVertexAttribL2i64NV fp_glVertexAttribL2i64NV;
typedef void (APIENTRYP pfn_glVertexAttribL3i64NV) (GLuint, GLint64EXT, GLint64EXT, GLint64EXT);
extern pfn_glVertexAttribL3i64NV fp_glVertexAttribL3i64NV;
typedef void (APIENTRYP pfn_glVertexAttribL4i64NV) (GLuint, GLint64EXT, GLint64EXT, GLint64EXT, GLint64EXT);
extern pfn_glVertexAttribL4i64NV fp_glVertexAttribL4i64NV;
typedef void (APIENTRYP pfn_glVertexAttribL1i64vNV) (GLuint, const GLint64EXT*);
extern pfn_glVertexAttribL1i64vNV fp_glVertexAttribL1i64vNV;
typedef void (APIENTRYP pfn_glVertexAttribL2i64vNV) (GLuint, const GLint64EXT*);
extern pfn_glVertexAttribL2i64vNV fp_glVertexAttribL2i64vNV;
typedef void (APIENTRYP pfn_glVertexAttribL3i64vNV) (GLuint, const GLint64EXT*);
extern pfn_glVertexAttribL3i64vNV fp_glVertexAttribL3i64vNV;
typedef void (APIENTRYP pfn_glVertexAttribL4i64vNV) (GLuint, const GLint64EXT*);
extern pfn_glVertexAttribL4i64vNV fp_glVertexAttribL4i64vNV;
typedef void (APIENTRYP pfn_glVertexAttribL1ui64NV) (GLuint, GLuint64EXT);
extern pfn_glVertexAttribL1ui64NV fp_glVertexAttribL1ui64NV;
typedef void (APIENTRYP pfn_glVertexAttribL2ui64NV) (GLuint, GLuint64EXT, GLuint64EXT);
extern pfn_glVertexAttribL2ui64NV fp_glVertexAttribL2ui64NV;
typedef void (APIENTRYP pfn_glVertexAttribL3ui64NV) (GLuint, GLuint64EXT, GLuint64EXT, GLuint64EXT);
extern pfn_glVertexAttribL3ui64NV fp_glVertexAttribL3ui64NV;
typedef void (APIENTRYP pfn_glVertexAttribL4ui64NV) (GLuint, GLuint64EXT, GLuint64EXT, GLuint64EXT, GLuint64EXT);
extern pfn_glVertexAttribL4ui64NV fp_glVertexAttribL4ui64NV;
typedef void (APIENTRYP pfn_glVertexAttribL1ui64vNV) (GLuint, const GLuint64EXT*);
extern pfn_glVertexAttribL1ui64vNV fp_glVertexAttribL1ui64vNV;
typedef void (APIENTRYP pfn_glVertexAttribL2ui64vNV) (GLuint, const GLuint64EXT*);
extern pfn_glVertexAttribL2ui64vNV fp_glVertexAttribL2ui64vNV;
typedef void (APIENTRYP pfn_glVertexAttribL3ui64vNV) (GLuint, const GLuint64EXT*);
extern pfn_glVertexAttribL3ui64vNV fp_glVertexAttribL3ui64vNV;
typedef void (APIENTRYP pfn_glVertexAttribL4ui64vNV) (GLuint, const GLuint64EXT*);
extern pfn_glVertexAttribL4ui64vNV fp_glVertexAttribL4ui64vNV;
typedef void (APIENTRYP pfn_glGetVertexAttribLi64vNV) (GLuint, GLenum, GLint64EXT*);
extern pfn_glGetVertexAttribLi64vNV fp_glGetVertexAttribLi64vNV;
typedef void (APIENTRYP pfn_glGetVertexAttribLui64vNV) (GLuint, GLenum, GLuint64EXT*);
extern pfn_glGetVertexAttribLui64vNV fp_glGetVertexAttribLui64vNV;
typedef void (APIENTRYP pfn_glVertexAttribLFormatNV) (GLuint, GLint, GLenum, GLsizei);
extern pfn_glVertexAttribLFormatNV fp_glVertexAttribLFormatNV;

 /* GL_NV_vertex_buffer_unified_memory */
extern GLboolean GLAD_NV_vertex_buffer_unified_memory;
#define GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV      0x8F1E
#define GL_ELEMENT_ARRAY_UNIFIED_NV            0x8F1F
#define GL_VERTEX_ATTRIB_ARRAY_ADDRESS_NV      0x8F20
#define GL_VERTEX_ARRAY_ADDRESS_NV             0x8F21
#define GL_NORMAL_ARRAY_ADDRESS_NV             0x8F22
#define GL_COLOR_ARRAY_ADDRESS_NV              0x8F23
#define GL_INDEX_ARRAY_ADDRESS_NV              0x8F24
#define GL_TEXTURE_COORD_ARRAY_ADDRESS_NV      0x8F25
#define GL_EDGE_FLAG_ARRAY_ADDRESS_NV          0x8F26
#define GL_SECONDARY_COLOR_ARRAY_ADDRESS_NV    0x8F27
#define GL_FOG_COORD_ARRAY_ADDRESS_NV          0x8F28
#define GL_ELEMENT_ARRAY_ADDRESS_NV            0x8F29
#define GL_VERTEX_ATTRIB_ARRAY_LENGTH_NV       0x8F2A
#define GL_VERTEX_ARRAY_LENGTH_NV              0x8F2B
#define GL_NORMAL_ARRAY_LENGTH_NV              0x8F2C
#define GL_COLOR_ARRAY_LENGTH_NV               0x8F2D
#define GL_INDEX_ARRAY_LENGTH_NV               0x8F2E
#define GL_TEXTURE_COORD_ARRAY_LENGTH_NV       0x8F2F
#define GL_EDGE_FLAG_ARRAY_LENGTH_NV           0x8F30
#define GL_SECONDARY_COLOR_ARRAY_LENGTH_NV     0x8F31
#define GL_FOG_COORD_ARRAY_LENGTH_NV           0x8F32
#define GL_ELEMENT_ARRAY_LENGTH_NV             0x8F33
#define GL_DRAW_INDIRECT_UNIFIED_NV            0x8F40
#define GL_DRAW_INDIRECT_ADDRESS_NV            0x8F41
#define GL_DRAW_INDIRECT_LENGTH_NV             0x8F42
typedef void (APIENTRYP pfn_glBufferAddressRangeNV) (GLenum, GLuint, GLuint64EXT, GLsizeiptr);
extern pfn_glBufferAddressRangeNV fp_glBufferAddressRangeNV;
typedef void (APIENTRYP pfn_glVertexFormatNV) (GLint, GLenum, GLsizei);
extern pfn_glVertexFormatNV fp_glVertexFormatNV;
typedef void (APIENTRYP pfn_glNormalFormatNV) (GLenum, GLsizei);
extern pfn_glNormalFormatNV fp_glNormalFormatNV;
typedef void (APIENTRYP pfn_glColorFormatNV) (GLint, GLenum, GLsizei);
extern pfn_glColorFormatNV fp_glColorFormatNV;
typedef void (APIENTRYP pfn_glIndexFormatNV) (GLenum, GLsizei);
extern pfn_glIndexFormatNV fp_glIndexFormatNV;
typedef void (APIENTRYP pfn_glTexCoordFormatNV) (GLint, GLenum, GLsizei);
extern pfn_glTexCoordFormatNV fp_glTexCoordFormatNV;
typedef void (APIENTRYP pfn_glEdgeFlagFormatNV) (GLsizei);
extern pfn_glEdgeFlagFormatNV fp_glEdgeFlagFormatNV;
typedef void (APIENTRYP pfn_glSecondaryColorFormatNV) (GLint, GLenum, GLsizei);
extern pfn_glSecondaryColorFormatNV fp_glSecondaryColorFormatNV;
typedef void (APIENTRYP pfn_glFogCoordFormatNV) (GLenum, GLsizei);
extern pfn_glFogCoordFormatNV fp_glFogCoordFormatNV;
typedef void (APIENTRYP pfn_glVertexAttribFormatNV) (GLuint, GLint, GLenum, GLboolean, GLsizei);
extern pfn_glVertexAttribFormatNV fp_glVertexAttribFormatNV;
typedef void (APIENTRYP pfn_glVertexAttribIFormatNV) (GLuint, GLint, GLenum, GLsizei);
extern pfn_glVertexAttribIFormatNV fp_glVertexAttribIFormatNV;
typedef void (APIENTRYP pfn_glGetIntegerui64i_vNV) (GLenum, GLuint, GLuint64EXT*);
extern pfn_glGetIntegerui64i_vNV fp_glGetIntegerui64i_vNV;

 /* GL_NV_vertex_program4 */
extern GLboolean GLAD_NV_vertex_program4;
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER_NV      0x88FD
typedef void (APIENTRYP pfn_glVertexAttribI1iEXT) (GLuint, GLint);
extern pfn_glVertexAttribI1iEXT fp_glVertexAttribI1iEXT;
typedef void (APIENTRYP pfn_glVertexAttribI2iEXT) (GLuint, GLint, GLint);
extern pfn_glVertexAttribI2iEXT fp_glVertexAttribI2iEXT;
typedef void (APIENTRYP pfn_glVertexAttribI3iEXT) (GLuint, GLint, GLint, GLint);
extern pfn_glVertexAttribI3iEXT fp_glVertexAttribI3iEXT;
typedef void (APIENTRYP pfn_glVertexAttribI4iEXT) (GLuint, GLint, GLint, GLint, GLint);
extern pfn_glVertexAttribI4iEXT fp_glVertexAttribI4iEXT;
typedef void (APIENTRYP pfn_glVertexAttribI1uiEXT) (GLuint, GLuint);
extern pfn_glVertexAttribI1uiEXT fp_glVertexAttribI1uiEXT;
typedef void (APIENTRYP pfn_glVertexAttribI2uiEXT) (GLuint, GLuint, GLuint);
extern pfn_glVertexAttribI2uiEXT fp_glVertexAttribI2uiEXT;
typedef void (APIENTRYP pfn_glVertexAttribI3uiEXT) (GLuint, GLuint, GLuint, GLuint);
extern pfn_glVertexAttribI3uiEXT fp_glVertexAttribI3uiEXT;
typedef void (APIENTRYP pfn_glVertexAttribI4uiEXT) (GLuint, GLuint, GLuint, GLuint, GLuint);
extern pfn_glVertexAttribI4uiEXT fp_glVertexAttribI4uiEXT;
typedef void (APIENTRYP pfn_glVertexAttribI1ivEXT) (GLuint, const GLint*);
extern pfn_glVertexAttribI1ivEXT fp_glVertexAttribI1ivEXT;
typedef void (APIENTRYP pfn_glVertexAttribI2ivEXT) (GLuint, const GLint*);
extern pfn_glVertexAttribI2ivEXT fp_glVertexAttribI2ivEXT;
typedef void (APIENTRYP pfn_glVertexAttribI3ivEXT) (GLuint, const GLint*);
extern pfn_glVertexAttribI3ivEXT fp_glVertexAttribI3ivEXT;
typedef void (APIENTRYP pfn_glVertexAttribI4ivEXT) (GLuint, const GLint*);
extern pfn_glVertexAttribI4ivEXT fp_glVertexAttribI4ivEXT;
typedef void (APIENTRYP pfn_glVertexAttribI1uivEXT) (GLuint, const GLuint*);
extern pfn_glVertexAttribI1uivEXT fp_glVertexAttribI1uivEXT;
typedef void (APIENTRYP pfn_glVertexAttribI2uivEXT) (GLuint, const GLuint*);
extern pfn_glVertexAttribI2uivEXT fp_glVertexAttribI2uivEXT;
typedef void (APIENTRYP pfn_glVertexAttribI3uivEXT) (GLuint, const GLuint*);
extern pfn_glVertexAttribI3uivEXT fp_glVertexAttribI3uivEXT;
typedef void (APIENTRYP pfn_glVertexAttribI4uivEXT) (GLuint, const GLuint*);
extern pfn_glVertexAttribI4uivEXT fp_glVertexAttribI4uivEXT;
typedef void (APIENTRYP pfn_glVertexAttribI4bvEXT) (GLuint, const GLbyte*);
extern pfn_glVertexAttribI4bvEXT fp_glVertexAttribI4bvEXT;
typedef void (APIENTRYP pfn_glVertexAttribI4svEXT) (GLuint, const GLshort*);
extern pfn_glVertexAttribI4svEXT fp_glVertexAttribI4svEXT;
typedef void (APIENTRYP pfn_glVertexAttribI4ubvEXT) (GLuint, const GLubyte*);
extern pfn_glVertexAttribI4ubvEXT fp_glVertexAttribI4ubvEXT;
typedef void (APIENTRYP pfn_glVertexAttribI4usvEXT) (GLuint, const GLushort*);
extern pfn_glVertexAttribI4usvEXT fp_glVertexAttribI4usvEXT;
typedef void (APIENTRYP pfn_glVertexAttribIPointerEXT) (GLuint, GLint, GLenum, GLsizei, const void*);
extern pfn_glVertexAttribIPointerEXT fp_glVertexAttribIPointerEXT;
typedef void (APIENTRYP pfn_glGetVertexAttribIivEXT) (GLuint, GLenum, GLint*);
extern pfn_glGetVertexAttribIivEXT fp_glGetVertexAttribIivEXT;
typedef void (APIENTRYP pfn_glGetVertexAttribIuivEXT) (GLuint, GLenum, GLuint*);
extern pfn_glGetVertexAttribIuivEXT fp_glGetVertexAttribIuivEXT;

 /* GL_NV_video_capture */
extern GLboolean GLAD_NV_video_capture;
#define GL_VIDEO_BUFFER_NV                     0x9020
#define GL_VIDEO_BUFFER_BINDING_NV             0x9021
#define GL_FIELD_UPPER_NV                      0x9022
#define GL_FIELD_LOWER_NV                      0x9023
#define GL_NUM_VIDEO_CAPTURE_STREAMS_NV        0x9024
#define GL_NEXT_VIDEO_CAPTURE_BUFFER_STATUS_NV  0x9025
#define GL_VIDEO_CAPTURE_TO_422_SUPPORTED_NV   0x9026
#define GL_LAST_VIDEO_CAPTURE_STATUS_NV        0x9027
#define GL_VIDEO_BUFFER_PITCH_NV               0x9028
#define GL_VIDEO_COLOR_CONVERSION_MATRIX_NV    0x9029
#define GL_VIDEO_COLOR_CONVERSION_MAX_NV       0x902A
#define GL_VIDEO_COLOR_CONVERSION_MIN_NV       0x902B
#define GL_VIDEO_COLOR_CONVERSION_OFFSET_NV    0x902C
#define GL_VIDEO_BUFFER_INTERNAL_FORMAT_NV     0x902D
#define GL_PARTIAL_SUCCESS_NV                  0x902E
#define GL_SUCCESS_NV                          0x902F
#define GL_FAILURE_NV                          0x9030
#define GL_YCBYCR8_422_NV                      0x9031
#define GL_YCBAYCR8A_4224_NV                   0x9032
#define GL_Z6Y10Z6CB10Z6Y10Z6CR10_422_NV       0x9033
#define GL_Z6Y10Z6CB10Z6A10Z6Y10Z6CR10Z6A10_4224_NV  0x9034
#define GL_Z4Y12Z4CB12Z4Y12Z4CR12_422_NV       0x9035
#define GL_Z4Y12Z4CB12Z4A12Z4Y12Z4CR12Z4A12_4224_NV  0x9036
#define GL_Z4Y12Z4CB12Z4CR12_444_NV            0x9037
#define GL_VIDEO_CAPTURE_FRAME_WIDTH_NV        0x9038
#define GL_VIDEO_CAPTURE_FRAME_HEIGHT_NV       0x9039
#define GL_VIDEO_CAPTURE_FIELD_UPPER_HEIGHT_NV  0x903A
#define GL_VIDEO_CAPTURE_FIELD_LOWER_HEIGHT_NV  0x903B
#define GL_VIDEO_CAPTURE_SURFACE_ORIGIN_NV     0x903C
typedef void (APIENTRYP pfn_glBeginVideoCaptureNV) (GLuint);
extern pfn_glBeginVideoCaptureNV fp_glBeginVideoCaptureNV;
typedef void (APIENTRYP pfn_glBindVideoCaptureStreamBufferNV) (GLuint, GLuint, GLenum, GLintptrARB);
extern pfn_glBindVideoCaptureStreamBufferNV fp_glBindVideoCaptureStreamBufferNV;
typedef void (APIENTRYP pfn_glBindVideoCaptureStreamTextureNV) (GLuint, GLuint, GLenum, GLenum, GLuint);
extern pfn_glBindVideoCaptureStreamTextureNV fp_glBindVideoCaptureStreamTextureNV;
typedef void (APIENTRYP pfn_glEndVideoCaptureNV) (GLuint);
extern pfn_glEndVideoCaptureNV fp_glEndVideoCaptureNV;
typedef void (APIENTRYP pfn_glGetVideoCaptureivNV) (GLuint, GLenum, GLint*);
extern pfn_glGetVideoCaptureivNV fp_glGetVideoCaptureivNV;
typedef void (APIENTRYP pfn_glGetVideoCaptureStreamivNV) (GLuint, GLuint, GLenum, GLint*);
extern pfn_glGetVideoCaptureStreamivNV fp_glGetVideoCaptureStreamivNV;
typedef void (APIENTRYP pfn_glGetVideoCaptureStreamfvNV) (GLuint, GLuint, GLenum, GLfloat*);
extern pfn_glGetVideoCaptureStreamfvNV fp_glGetVideoCaptureStreamfvNV;
typedef void (APIENTRYP pfn_glGetVideoCaptureStreamdvNV) (GLuint, GLuint, GLenum, GLdouble*);
extern pfn_glGetVideoCaptureStreamdvNV fp_glGetVideoCaptureStreamdvNV;
typedef GLenum (APIENTRYP pfn_glVideoCaptureNV) (GLuint, GLuint*, GLuint64EXT*);
extern pfn_glVideoCaptureNV fp_glVideoCaptureNV;
typedef void (APIENTRYP pfn_glVideoCaptureStreamParameterivNV) (GLuint, GLuint, GLenum, const GLint*);
extern pfn_glVideoCaptureStreamParameterivNV fp_glVideoCaptureStreamParameterivNV;
typedef void (APIENTRYP pfn_glVideoCaptureStreamParameterfvNV) (GLuint, GLuint, GLenum, const GLfloat*);
extern pfn_glVideoCaptureStreamParameterfvNV fp_glVideoCaptureStreamParameterfvNV;
typedef void (APIENTRYP pfn_glVideoCaptureStreamParameterdvNV) (GLuint, GLuint, GLenum, const GLdouble*);
extern pfn_glVideoCaptureStreamParameterdvNV fp_glVideoCaptureStreamParameterdvNV;

 /* GL_NV_viewport_array */
extern GLboolean GLAD_NV_viewport_array;
#define GL_MAX_VIEWPORTS_NV                    0x825B
#define GL_VIEWPORT_SUBPIXEL_BITS_NV           0x825C
#define GL_VIEWPORT_BOUNDS_RANGE_NV            0x825D
#define GL_VIEWPORT_INDEX_PROVOKING_VERTEX_NV  0x825F
typedef void (APIENTRYP pfn_glViewportArrayvNV) (GLuint, GLsizei, const GLfloat*);
extern pfn_glViewportArrayvNV fp_glViewportArrayvNV;
typedef void (APIENTRYP pfn_glViewportIndexedfNV) (GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
extern pfn_glViewportIndexedfNV fp_glViewportIndexedfNV;
typedef void (APIENTRYP pfn_glViewportIndexedfvNV) (GLuint, const GLfloat*);
extern pfn_glViewportIndexedfvNV fp_glViewportIndexedfvNV;
typedef void (APIENTRYP pfn_glScissorArrayvNV) (GLuint, GLsizei, const GLint*);
extern pfn_glScissorArrayvNV fp_glScissorArrayvNV;
typedef void (APIENTRYP pfn_glScissorIndexedNV) (GLuint, GLint, GLint, GLsizei, GLsizei);
extern pfn_glScissorIndexedNV fp_glScissorIndexedNV;
typedef void (APIENTRYP pfn_glScissorIndexedvNV) (GLuint, const GLint*);
extern pfn_glScissorIndexedvNV fp_glScissorIndexedvNV;
typedef void (APIENTRYP pfn_glDepthRangeArrayfvNV) (GLuint, GLsizei, const GLfloat*);
extern pfn_glDepthRangeArrayfvNV fp_glDepthRangeArrayfvNV;
typedef void (APIENTRYP pfn_glDepthRangeIndexedfNV) (GLuint, GLfloat, GLfloat);
extern pfn_glDepthRangeIndexedfNV fp_glDepthRangeIndexedfNV;
typedef void (APIENTRYP pfn_glGetFloati_vNV) (GLenum, GLuint, GLfloat*);
extern pfn_glGetFloati_vNV fp_glGetFloati_vNV;
typedef void (APIENTRYP pfn_glEnableiNV) (GLenum, GLuint);
extern pfn_glEnableiNV fp_glEnableiNV;
typedef void (APIENTRYP pfn_glDisableiNV) (GLenum, GLuint);
extern pfn_glDisableiNV fp_glDisableiNV;
typedef GLboolean (APIENTRYP pfn_glIsEnablediNV) (GLenum, GLuint);
extern pfn_glIsEnablediNV fp_glIsEnablediNV;

 /* GL_NV_viewport_array2 */
extern GLboolean GLAD_NV_viewport_array2;

 /* GL_NVX_conditional_render */
extern GLboolean GLAD_NVX_conditional_render;
typedef void (APIENTRYP pfn_glBeginConditionalRenderNVX) (GLuint);
extern pfn_glBeginConditionalRenderNVX fp_glBeginConditionalRenderNVX;
typedef void (APIENTRYP pfn_glEndConditionalRenderNVX) ();
extern pfn_glEndConditionalRenderNVX fp_glEndConditionalRenderNVX;

 /* GL_NVX_gpu_memory_info */
extern GLboolean GLAD_NVX_gpu_memory_info;
#define GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX  0x9047
#define GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX  0x9048
#define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX  0x9049
#define GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX  0x904A
#define GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX  0x904B

 /* GL_OVR_multiview */
extern GLboolean GLAD_OVR_multiview;
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_NUM_VIEWS_OVR  0x9630
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_BASE_VIEW_INDEX_OVR  0x9632
#define GL_MAX_VIEWS_OVR                       0x9631
typedef void (APIENTRYP pfn_glFramebufferTextureMultiviewOVR) (GLenum, GLenum, GLuint, GLint, GLint, GLsizei);
extern pfn_glFramebufferTextureMultiviewOVR fp_glFramebufferTextureMultiviewOVR;

 /* GL_OVR_multiview2 */
extern GLboolean GLAD_OVR_multiview2;

 /* GL_OVR_multiview_multisampled_render_to_texture */
extern GLboolean GLAD_OVR_multiview_multisampled_render_to_texture;
typedef void (APIENTRYP pfn_glFramebufferTextureMultisampleMultiviewOVR) (GLenum, GLenum, GLuint, GLint, GLsizei, GLint, GLsizei);
extern pfn_glFramebufferTextureMultisampleMultiviewOVR fp_glFramebufferTextureMultisampleMultiviewOVR;

 /* GL_QCOM_alpha_test */
extern GLboolean GLAD_QCOM_alpha_test;
#define GL_ALPHA_TEST_QCOM                     0x0BC0
#define GL_ALPHA_TEST_FUNC_QCOM                0x0BC1
#define GL_ALPHA_TEST_REF_QCOM                 0x0BC2
typedef void (APIENTRYP pfn_glAlphaFuncQCOM) (GLenum, GLclampf);
extern pfn_glAlphaFuncQCOM fp_glAlphaFuncQCOM;

 /* GL_QCOM_binning_control */
extern GLboolean GLAD_QCOM_binning_control;
#define GL_BINNING_CONTROL_HINT_QCOM           0x8FB0
#define GL_CPU_OPTIMIZED_QCOM                  0x8FB1
#define GL_GPU_OPTIMIZED_QCOM                  0x8FB2
#define GL_RENDER_DIRECT_TO_FRAMEBUFFER_QCOM   0x8FB3

 /* GL_QCOM_driver_control */
extern GLboolean GLAD_QCOM_driver_control;
typedef void (APIENTRYP pfn_glGetDriverControlsQCOM) (GLint*, GLsizei, GLuint*);
extern pfn_glGetDriverControlsQCOM fp_glGetDriverControlsQCOM;
typedef void (APIENTRYP pfn_glGetDriverControlStringQCOM) (GLuint, GLsizei, GLsizei*, GLchar*);
extern pfn_glGetDriverControlStringQCOM fp_glGetDriverControlStringQCOM;
typedef void (APIENTRYP pfn_glEnableDriverControlQCOM) (GLuint);
extern pfn_glEnableDriverControlQCOM fp_glEnableDriverControlQCOM;
typedef void (APIENTRYP pfn_glDisableDriverControlQCOM) (GLuint);
extern pfn_glDisableDriverControlQCOM fp_glDisableDriverControlQCOM;

 /* GL_QCOM_extended_get */
extern GLboolean GLAD_QCOM_extended_get;
#define GL_TEXTURE_WIDTH_QCOM                  0x8BD2
#define GL_TEXTURE_HEIGHT_QCOM                 0x8BD3
#define GL_TEXTURE_DEPTH_QCOM                  0x8BD4
#define GL_TEXTURE_INTERNAL_FORMAT_QCOM        0x8BD5
#define GL_TEXTURE_FORMAT_QCOM                 0x8BD6
#define GL_TEXTURE_TYPE_QCOM                   0x8BD7
#define GL_TEXTURE_IMAGE_VALID_QCOM            0x8BD8
#define GL_TEXTURE_NUM_LEVELS_QCOM             0x8BD9
#define GL_TEXTURE_TARGET_QCOM                 0x8BDA
#define GL_TEXTURE_OBJECT_VALID_QCOM           0x8BDB
#define GL_STATE_RESTORE                       0x8BDC
typedef void (APIENTRYP pfn_glExtGetTexturesQCOM) (GLuint*, GLint, GLint*);
extern pfn_glExtGetTexturesQCOM fp_glExtGetTexturesQCOM;
typedef void (APIENTRYP pfn_glExtGetBuffersQCOM) (GLuint*, GLint, GLint*);
extern pfn_glExtGetBuffersQCOM fp_glExtGetBuffersQCOM;
typedef void (APIENTRYP pfn_glExtGetRenderbuffersQCOM) (GLuint*, GLint, GLint*);
extern pfn_glExtGetRenderbuffersQCOM fp_glExtGetRenderbuffersQCOM;
typedef void (APIENTRYP pfn_glExtGetFramebuffersQCOM) (GLuint*, GLint, GLint*);
extern pfn_glExtGetFramebuffersQCOM fp_glExtGetFramebuffersQCOM;
typedef void (APIENTRYP pfn_glExtGetTexLevelParameterivQCOM) (GLuint, GLenum, GLint, GLenum, GLint*);
extern pfn_glExtGetTexLevelParameterivQCOM fp_glExtGetTexLevelParameterivQCOM;
typedef void (APIENTRYP pfn_glExtTexObjectStateOverrideiQCOM) (GLenum, GLenum, GLint);
extern pfn_glExtTexObjectStateOverrideiQCOM fp_glExtTexObjectStateOverrideiQCOM;
typedef void (APIENTRYP pfn_glExtGetTexSubImageQCOM) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, void*);
extern pfn_glExtGetTexSubImageQCOM fp_glExtGetTexSubImageQCOM;
typedef void (APIENTRYP pfn_glExtGetBufferPointervQCOM) (GLenum, void**);
extern pfn_glExtGetBufferPointervQCOM fp_glExtGetBufferPointervQCOM;

 /* GL_QCOM_extended_get2 */
extern GLboolean GLAD_QCOM_extended_get2;
typedef void (APIENTRYP pfn_glExtGetShadersQCOM) (GLuint*, GLint, GLint*);
extern pfn_glExtGetShadersQCOM fp_glExtGetShadersQCOM;
typedef void (APIENTRYP pfn_glExtGetProgramsQCOM) (GLuint*, GLint, GLint*);
extern pfn_glExtGetProgramsQCOM fp_glExtGetProgramsQCOM;
typedef GLboolean (APIENTRYP pfn_glExtIsProgramBinaryQCOM) (GLuint);
extern pfn_glExtIsProgramBinaryQCOM fp_glExtIsProgramBinaryQCOM;
typedef void (APIENTRYP pfn_glExtGetProgramBinarySourceQCOM) (GLuint, GLenum, GLchar*, GLint*);
extern pfn_glExtGetProgramBinarySourceQCOM fp_glExtGetProgramBinarySourceQCOM;

 /* GL_QCOM_perfmon_global_mode */
extern GLboolean GLAD_QCOM_perfmon_global_mode;
#define GL_PERFMON_GLOBAL_MODE_QCOM            0x8FA0

 /* GL_QCOM_tiled_rendering */
extern GLboolean GLAD_QCOM_tiled_rendering;
#define GL_COLOR_BUFFER_BIT0_QCOM              0x00000001
#define GL_COLOR_BUFFER_BIT1_QCOM              0x00000002
#define GL_COLOR_BUFFER_BIT2_QCOM              0x00000004
#define GL_COLOR_BUFFER_BIT3_QCOM              0x00000008
#define GL_COLOR_BUFFER_BIT4_QCOM              0x00000010
#define GL_COLOR_BUFFER_BIT5_QCOM              0x00000020
#define GL_COLOR_BUFFER_BIT6_QCOM              0x00000040
#define GL_COLOR_BUFFER_BIT7_QCOM              0x00000080
#define GL_DEPTH_BUFFER_BIT0_QCOM              0x00000100
#define GL_DEPTH_BUFFER_BIT1_QCOM              0x00000200
#define GL_DEPTH_BUFFER_BIT2_QCOM              0x00000400
#define GL_DEPTH_BUFFER_BIT3_QCOM              0x00000800
#define GL_DEPTH_BUFFER_BIT4_QCOM              0x00001000
#define GL_DEPTH_BUFFER_BIT5_QCOM              0x00002000
#define GL_DEPTH_BUFFER_BIT6_QCOM              0x00004000
#define GL_DEPTH_BUFFER_BIT7_QCOM              0x00008000
#define GL_STENCIL_BUFFER_BIT0_QCOM            0x00010000
#define GL_STENCIL_BUFFER_BIT1_QCOM            0x00020000
#define GL_STENCIL_BUFFER_BIT2_QCOM            0x00040000
#define GL_STENCIL_BUFFER_BIT3_QCOM            0x00080000
#define GL_STENCIL_BUFFER_BIT4_QCOM            0x00100000
#define GL_STENCIL_BUFFER_BIT5_QCOM            0x00200000
#define GL_STENCIL_BUFFER_BIT6_QCOM            0x00400000
#define GL_STENCIL_BUFFER_BIT7_QCOM            0x00800000
#define GL_MULTISAMPLE_BUFFER_BIT0_QCOM        0x01000000
#define GL_MULTISAMPLE_BUFFER_BIT1_QCOM        0x02000000
#define GL_MULTISAMPLE_BUFFER_BIT2_QCOM        0x04000000
#define GL_MULTISAMPLE_BUFFER_BIT3_QCOM        0x08000000
#define GL_MULTISAMPLE_BUFFER_BIT4_QCOM        0x10000000
#define GL_MULTISAMPLE_BUFFER_BIT5_QCOM        0x20000000
#define GL_MULTISAMPLE_BUFFER_BIT6_QCOM        0x40000000
#define GL_MULTISAMPLE_BUFFER_BIT7_QCOM        0x80000000
typedef void (APIENTRYP pfn_glStartTilingQCOM) (GLuint, GLuint, GLuint, GLuint, GLbitfield);
extern pfn_glStartTilingQCOM fp_glStartTilingQCOM;
typedef void (APIENTRYP pfn_glEndTilingQCOM) (GLbitfield);
extern pfn_glEndTilingQCOM fp_glEndTilingQCOM;

 /* GL_QCOM_writeonly_rendering */
extern GLboolean GLAD_QCOM_writeonly_rendering;
#define GL_WRITEONLY_RENDERING_QCOM            0x8823

 /* GL_VIV_shader_binary */
extern GLboolean GLAD_VIV_shader_binary;
#define GL_SHADER_BINARY_VIV                   0x8FC4

} /* namespace glad */

#endif /* __glad_hpp_ */
