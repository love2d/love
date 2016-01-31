
/**
 * The MIT License (MIT)
 * 
 * Copyright (c) 2013 David Herberth, modified by Alex Szpakowski
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
 

#ifndef __glad_funcs_hpp_
#define __glad_funcs_hpp_

#include "glad.hpp"

namespace glad {


/* GL_VERSION_1_0 */
inline void glCullFace(GLenum mode) { fp_glCullFace(mode); }
inline void glFrontFace(GLenum mode) { fp_glFrontFace(mode); }
inline void glHint(GLenum target, GLenum mode) { fp_glHint(target, mode); }
inline void glLineWidth(GLfloat width) { fp_glLineWidth(width); }
inline void glPointSize(GLfloat size) { fp_glPointSize(size); }
inline void glPolygonMode(GLenum face, GLenum mode) { fp_glPolygonMode(face, mode); }
inline void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) { fp_glScissor(x, y, width, height); }
inline void glTexParameterf(GLenum target, GLenum pname, GLfloat param) { fp_glTexParameterf(target, pname, param); }
inline void glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params) { fp_glTexParameterfv(target, pname, params); }
inline void glTexParameteri(GLenum target, GLenum pname, GLint param) { fp_glTexParameteri(target, pname, param); }
inline void glTexParameteriv(GLenum target, GLenum pname, const GLint* params) { fp_glTexParameteriv(target, pname, params); }
inline void glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels) { fp_glTexImage1D(target, level, internalformat, width, border, format, type, pixels); }
inline void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels) { fp_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels); }
inline void glDrawBuffer(GLenum buf) { fp_glDrawBuffer(buf); }
inline void glClear(GLbitfield mask) { fp_glClear(mask); }
inline void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) { fp_glClearColor(red, green, blue, alpha); }
inline void glClearStencil(GLint s) { fp_glClearStencil(s); }
inline void glClearDepth(GLdouble depth) { fp_glClearDepth(depth); }
inline void glStencilMask(GLuint mask) { fp_glStencilMask(mask); }
inline void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) { fp_glColorMask(red, green, blue, alpha); }
inline void glDepthMask(GLboolean flag) { fp_glDepthMask(flag); }
inline void glDisable(GLenum cap) { fp_glDisable(cap); }
inline void glEnable(GLenum cap) { fp_glEnable(cap); }
inline void glFinish() { fp_glFinish(); }
inline void glFlush() { fp_glFlush(); }
inline void glBlendFunc(GLenum sfactor, GLenum dfactor) { fp_glBlendFunc(sfactor, dfactor); }
inline void glLogicOp(GLenum opcode) { fp_glLogicOp(opcode); }
inline void glStencilFunc(GLenum func, GLint ref, GLuint mask) { fp_glStencilFunc(func, ref, mask); }
inline void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) { fp_glStencilOp(fail, zfail, zpass); }
inline void glDepthFunc(GLenum func) { fp_glDepthFunc(func); }
inline void glPixelStoref(GLenum pname, GLfloat param) { fp_glPixelStoref(pname, param); }
inline void glPixelStorei(GLenum pname, GLint param) { fp_glPixelStorei(pname, param); }
inline void glReadBuffer(GLenum src) { fp_glReadBuffer(src); }
inline void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels) { fp_glReadPixels(x, y, width, height, format, type, pixels); }
inline void glGetBooleanv(GLenum pname, GLboolean* data) { fp_glGetBooleanv(pname, data); }
inline void glGetDoublev(GLenum pname, GLdouble* data) { fp_glGetDoublev(pname, data); }
inline GLenum glGetError() { return fp_glGetError(); }
inline void glGetFloatv(GLenum pname, GLfloat* data) { fp_glGetFloatv(pname, data); }
inline void glGetIntegerv(GLenum pname, GLint* data) { fp_glGetIntegerv(pname, data); }
inline const GLubyte* glGetString(GLenum name) { return fp_glGetString(name); }
inline void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void* pixels) { fp_glGetTexImage(target, level, format, type, pixels); }
inline void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params) { fp_glGetTexParameterfv(target, pname, params); }
inline void glGetTexParameteriv(GLenum target, GLenum pname, GLint* params) { fp_glGetTexParameteriv(target, pname, params); }
inline void glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params) { fp_glGetTexLevelParameterfv(target, level, pname, params); }
inline void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params) { fp_glGetTexLevelParameteriv(target, level, pname, params); }
inline GLboolean glIsEnabled(GLenum cap) { return fp_glIsEnabled(cap); }
inline void glDepthRange(GLdouble near, GLdouble far) { fp_glDepthRange(near, far); }
inline void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) { fp_glViewport(x, y, width, height); }
inline void glNewList(GLuint list, GLenum mode) { fp_glNewList(list, mode); }
inline void glEndList() { fp_glEndList(); }
inline void glCallList(GLuint list) { fp_glCallList(list); }
inline void glCallLists(GLsizei n, GLenum type, const void* lists) { fp_glCallLists(n, type, lists); }
inline void glDeleteLists(GLuint list, GLsizei range) { fp_glDeleteLists(list, range); }
inline GLuint glGenLists(GLsizei range) { return fp_glGenLists(range); }
inline void glListBase(GLuint base) { fp_glListBase(base); }
inline void glBegin(GLenum mode) { fp_glBegin(mode); }
inline void glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte* bitmap) { fp_glBitmap(width, height, xorig, yorig, xmove, ymove, bitmap); }
inline void glColor3b(GLbyte red, GLbyte green, GLbyte blue) { fp_glColor3b(red, green, blue); }
inline void glColor3bv(const GLbyte* v) { fp_glColor3bv(v); }
inline void glColor3d(GLdouble red, GLdouble green, GLdouble blue) { fp_glColor3d(red, green, blue); }
inline void glColor3dv(const GLdouble* v) { fp_glColor3dv(v); }
inline void glColor3f(GLfloat red, GLfloat green, GLfloat blue) { fp_glColor3f(red, green, blue); }
inline void glColor3fv(const GLfloat* v) { fp_glColor3fv(v); }
inline void glColor3i(GLint red, GLint green, GLint blue) { fp_glColor3i(red, green, blue); }
inline void glColor3iv(const GLint* v) { fp_glColor3iv(v); }
inline void glColor3s(GLshort red, GLshort green, GLshort blue) { fp_glColor3s(red, green, blue); }
inline void glColor3sv(const GLshort* v) { fp_glColor3sv(v); }
inline void glColor3ub(GLubyte red, GLubyte green, GLubyte blue) { fp_glColor3ub(red, green, blue); }
inline void glColor3ubv(const GLubyte* v) { fp_glColor3ubv(v); }
inline void glColor3ui(GLuint red, GLuint green, GLuint blue) { fp_glColor3ui(red, green, blue); }
inline void glColor3uiv(const GLuint* v) { fp_glColor3uiv(v); }
inline void glColor3us(GLushort red, GLushort green, GLushort blue) { fp_glColor3us(red, green, blue); }
inline void glColor3usv(const GLushort* v) { fp_glColor3usv(v); }
inline void glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha) { fp_glColor4b(red, green, blue, alpha); }
inline void glColor4bv(const GLbyte* v) { fp_glColor4bv(v); }
inline void glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha) { fp_glColor4d(red, green, blue, alpha); }
inline void glColor4dv(const GLdouble* v) { fp_glColor4dv(v); }
inline void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) { fp_glColor4f(red, green, blue, alpha); }
inline void glColor4fv(const GLfloat* v) { fp_glColor4fv(v); }
inline void glColor4i(GLint red, GLint green, GLint blue, GLint alpha) { fp_glColor4i(red, green, blue, alpha); }
inline void glColor4iv(const GLint* v) { fp_glColor4iv(v); }
inline void glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha) { fp_glColor4s(red, green, blue, alpha); }
inline void glColor4sv(const GLshort* v) { fp_glColor4sv(v); }
inline void glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha) { fp_glColor4ub(red, green, blue, alpha); }
inline void glColor4ubv(const GLubyte* v) { fp_glColor4ubv(v); }
inline void glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha) { fp_glColor4ui(red, green, blue, alpha); }
inline void glColor4uiv(const GLuint* v) { fp_glColor4uiv(v); }
inline void glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha) { fp_glColor4us(red, green, blue, alpha); }
inline void glColor4usv(const GLushort* v) { fp_glColor4usv(v); }
inline void glEdgeFlag(GLboolean flag) { fp_glEdgeFlag(flag); }
inline void glEdgeFlagv(const GLboolean* flag) { fp_glEdgeFlagv(flag); }
inline void glEnd() { fp_glEnd(); }
inline void glIndexd(GLdouble c) { fp_glIndexd(c); }
inline void glIndexdv(const GLdouble* c) { fp_glIndexdv(c); }
inline void glIndexf(GLfloat c) { fp_glIndexf(c); }
inline void glIndexfv(const GLfloat* c) { fp_glIndexfv(c); }
inline void glIndexi(GLint c) { fp_glIndexi(c); }
inline void glIndexiv(const GLint* c) { fp_glIndexiv(c); }
inline void glIndexs(GLshort c) { fp_glIndexs(c); }
inline void glIndexsv(const GLshort* c) { fp_glIndexsv(c); }
inline void glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz) { fp_glNormal3b(nx, ny, nz); }
inline void glNormal3bv(const GLbyte* v) { fp_glNormal3bv(v); }
inline void glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz) { fp_glNormal3d(nx, ny, nz); }
inline void glNormal3dv(const GLdouble* v) { fp_glNormal3dv(v); }
inline void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) { fp_glNormal3f(nx, ny, nz); }
inline void glNormal3fv(const GLfloat* v) { fp_glNormal3fv(v); }
inline void glNormal3i(GLint nx, GLint ny, GLint nz) { fp_glNormal3i(nx, ny, nz); }
inline void glNormal3iv(const GLint* v) { fp_glNormal3iv(v); }
inline void glNormal3s(GLshort nx, GLshort ny, GLshort nz) { fp_glNormal3s(nx, ny, nz); }
inline void glNormal3sv(const GLshort* v) { fp_glNormal3sv(v); }
inline void glRasterPos2d(GLdouble x, GLdouble y) { fp_glRasterPos2d(x, y); }
inline void glRasterPos2dv(const GLdouble* v) { fp_glRasterPos2dv(v); }
inline void glRasterPos2f(GLfloat x, GLfloat y) { fp_glRasterPos2f(x, y); }
inline void glRasterPos2fv(const GLfloat* v) { fp_glRasterPos2fv(v); }
inline void glRasterPos2i(GLint x, GLint y) { fp_glRasterPos2i(x, y); }
inline void glRasterPos2iv(const GLint* v) { fp_glRasterPos2iv(v); }
inline void glRasterPos2s(GLshort x, GLshort y) { fp_glRasterPos2s(x, y); }
inline void glRasterPos2sv(const GLshort* v) { fp_glRasterPos2sv(v); }
inline void glRasterPos3d(GLdouble x, GLdouble y, GLdouble z) { fp_glRasterPos3d(x, y, z); }
inline void glRasterPos3dv(const GLdouble* v) { fp_glRasterPos3dv(v); }
inline void glRasterPos3f(GLfloat x, GLfloat y, GLfloat z) { fp_glRasterPos3f(x, y, z); }
inline void glRasterPos3fv(const GLfloat* v) { fp_glRasterPos3fv(v); }
inline void glRasterPos3i(GLint x, GLint y, GLint z) { fp_glRasterPos3i(x, y, z); }
inline void glRasterPos3iv(const GLint* v) { fp_glRasterPos3iv(v); }
inline void glRasterPos3s(GLshort x, GLshort y, GLshort z) { fp_glRasterPos3s(x, y, z); }
inline void glRasterPos3sv(const GLshort* v) { fp_glRasterPos3sv(v); }
inline void glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w) { fp_glRasterPos4d(x, y, z, w); }
inline void glRasterPos4dv(const GLdouble* v) { fp_glRasterPos4dv(v); }
inline void glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) { fp_glRasterPos4f(x, y, z, w); }
inline void glRasterPos4fv(const GLfloat* v) { fp_glRasterPos4fv(v); }
inline void glRasterPos4i(GLint x, GLint y, GLint z, GLint w) { fp_glRasterPos4i(x, y, z, w); }
inline void glRasterPos4iv(const GLint* v) { fp_glRasterPos4iv(v); }
inline void glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w) { fp_glRasterPos4s(x, y, z, w); }
inline void glRasterPos4sv(const GLshort* v) { fp_glRasterPos4sv(v); }
inline void glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2) { fp_glRectd(x1, y1, x2, y2); }
inline void glRectdv(const GLdouble* v1, const GLdouble* v2) { fp_glRectdv(v1, v2); }
inline void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) { fp_glRectf(x1, y1, x2, y2); }
inline void glRectfv(const GLfloat* v1, const GLfloat* v2) { fp_glRectfv(v1, v2); }
inline void glRecti(GLint x1, GLint y1, GLint x2, GLint y2) { fp_glRecti(x1, y1, x2, y2); }
inline void glRectiv(const GLint* v1, const GLint* v2) { fp_glRectiv(v1, v2); }
inline void glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2) { fp_glRects(x1, y1, x2, y2); }
inline void glRectsv(const GLshort* v1, const GLshort* v2) { fp_glRectsv(v1, v2); }
inline void glTexCoord1d(GLdouble s) { fp_glTexCoord1d(s); }
inline void glTexCoord1dv(const GLdouble* v) { fp_glTexCoord1dv(v); }
inline void glTexCoord1f(GLfloat s) { fp_glTexCoord1f(s); }
inline void glTexCoord1fv(const GLfloat* v) { fp_glTexCoord1fv(v); }
inline void glTexCoord1i(GLint s) { fp_glTexCoord1i(s); }
inline void glTexCoord1iv(const GLint* v) { fp_glTexCoord1iv(v); }
inline void glTexCoord1s(GLshort s) { fp_glTexCoord1s(s); }
inline void glTexCoord1sv(const GLshort* v) { fp_glTexCoord1sv(v); }
inline void glTexCoord2d(GLdouble s, GLdouble t) { fp_glTexCoord2d(s, t); }
inline void glTexCoord2dv(const GLdouble* v) { fp_glTexCoord2dv(v); }
inline void glTexCoord2f(GLfloat s, GLfloat t) { fp_glTexCoord2f(s, t); }
inline void glTexCoord2fv(const GLfloat* v) { fp_glTexCoord2fv(v); }
inline void glTexCoord2i(GLint s, GLint t) { fp_glTexCoord2i(s, t); }
inline void glTexCoord2iv(const GLint* v) { fp_glTexCoord2iv(v); }
inline void glTexCoord2s(GLshort s, GLshort t) { fp_glTexCoord2s(s, t); }
inline void glTexCoord2sv(const GLshort* v) { fp_glTexCoord2sv(v); }
inline void glTexCoord3d(GLdouble s, GLdouble t, GLdouble r) { fp_glTexCoord3d(s, t, r); }
inline void glTexCoord3dv(const GLdouble* v) { fp_glTexCoord3dv(v); }
inline void glTexCoord3f(GLfloat s, GLfloat t, GLfloat r) { fp_glTexCoord3f(s, t, r); }
inline void glTexCoord3fv(const GLfloat* v) { fp_glTexCoord3fv(v); }
inline void glTexCoord3i(GLint s, GLint t, GLint r) { fp_glTexCoord3i(s, t, r); }
inline void glTexCoord3iv(const GLint* v) { fp_glTexCoord3iv(v); }
inline void glTexCoord3s(GLshort s, GLshort t, GLshort r) { fp_glTexCoord3s(s, t, r); }
inline void glTexCoord3sv(const GLshort* v) { fp_glTexCoord3sv(v); }
inline void glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q) { fp_glTexCoord4d(s, t, r, q); }
inline void glTexCoord4dv(const GLdouble* v) { fp_glTexCoord4dv(v); }
inline void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q) { fp_glTexCoord4f(s, t, r, q); }
inline void glTexCoord4fv(const GLfloat* v) { fp_glTexCoord4fv(v); }
inline void glTexCoord4i(GLint s, GLint t, GLint r, GLint q) { fp_glTexCoord4i(s, t, r, q); }
inline void glTexCoord4iv(const GLint* v) { fp_glTexCoord4iv(v); }
inline void glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q) { fp_glTexCoord4s(s, t, r, q); }
inline void glTexCoord4sv(const GLshort* v) { fp_glTexCoord4sv(v); }
inline void glVertex2d(GLdouble x, GLdouble y) { fp_glVertex2d(x, y); }
inline void glVertex2dv(const GLdouble* v) { fp_glVertex2dv(v); }
inline void glVertex2f(GLfloat x, GLfloat y) { fp_glVertex2f(x, y); }
inline void glVertex2fv(const GLfloat* v) { fp_glVertex2fv(v); }
inline void glVertex2i(GLint x, GLint y) { fp_glVertex2i(x, y); }
inline void glVertex2iv(const GLint* v) { fp_glVertex2iv(v); }
inline void glVertex2s(GLshort x, GLshort y) { fp_glVertex2s(x, y); }
inline void glVertex2sv(const GLshort* v) { fp_glVertex2sv(v); }
inline void glVertex3d(GLdouble x, GLdouble y, GLdouble z) { fp_glVertex3d(x, y, z); }
inline void glVertex3dv(const GLdouble* v) { fp_glVertex3dv(v); }
inline void glVertex3f(GLfloat x, GLfloat y, GLfloat z) { fp_glVertex3f(x, y, z); }
inline void glVertex3fv(const GLfloat* v) { fp_glVertex3fv(v); }
inline void glVertex3i(GLint x, GLint y, GLint z) { fp_glVertex3i(x, y, z); }
inline void glVertex3iv(const GLint* v) { fp_glVertex3iv(v); }
inline void glVertex3s(GLshort x, GLshort y, GLshort z) { fp_glVertex3s(x, y, z); }
inline void glVertex3sv(const GLshort* v) { fp_glVertex3sv(v); }
inline void glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w) { fp_glVertex4d(x, y, z, w); }
inline void glVertex4dv(const GLdouble* v) { fp_glVertex4dv(v); }
inline void glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) { fp_glVertex4f(x, y, z, w); }
inline void glVertex4fv(const GLfloat* v) { fp_glVertex4fv(v); }
inline void glVertex4i(GLint x, GLint y, GLint z, GLint w) { fp_glVertex4i(x, y, z, w); }
inline void glVertex4iv(const GLint* v) { fp_glVertex4iv(v); }
inline void glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w) { fp_glVertex4s(x, y, z, w); }
inline void glVertex4sv(const GLshort* v) { fp_glVertex4sv(v); }
inline void glClipPlane(GLenum plane, const GLdouble* equation) { fp_glClipPlane(plane, equation); }
inline void glColorMaterial(GLenum face, GLenum mode) { fp_glColorMaterial(face, mode); }
inline void glFogf(GLenum pname, GLfloat param) { fp_glFogf(pname, param); }
inline void glFogfv(GLenum pname, const GLfloat* params) { fp_glFogfv(pname, params); }
inline void glFogi(GLenum pname, GLint param) { fp_glFogi(pname, param); }
inline void glFogiv(GLenum pname, const GLint* params) { fp_glFogiv(pname, params); }
inline void glLightf(GLenum light, GLenum pname, GLfloat param) { fp_glLightf(light, pname, param); }
inline void glLightfv(GLenum light, GLenum pname, const GLfloat* params) { fp_glLightfv(light, pname, params); }
inline void glLighti(GLenum light, GLenum pname, GLint param) { fp_glLighti(light, pname, param); }
inline void glLightiv(GLenum light, GLenum pname, const GLint* params) { fp_glLightiv(light, pname, params); }
inline void glLightModelf(GLenum pname, GLfloat param) { fp_glLightModelf(pname, param); }
inline void glLightModelfv(GLenum pname, const GLfloat* params) { fp_glLightModelfv(pname, params); }
inline void glLightModeli(GLenum pname, GLint param) { fp_glLightModeli(pname, param); }
inline void glLightModeliv(GLenum pname, const GLint* params) { fp_glLightModeliv(pname, params); }
inline void glLineStipple(GLint factor, GLushort pattern) { fp_glLineStipple(factor, pattern); }
inline void glMaterialf(GLenum face, GLenum pname, GLfloat param) { fp_glMaterialf(face, pname, param); }
inline void glMaterialfv(GLenum face, GLenum pname, const GLfloat* params) { fp_glMaterialfv(face, pname, params); }
inline void glMateriali(GLenum face, GLenum pname, GLint param) { fp_glMateriali(face, pname, param); }
inline void glMaterialiv(GLenum face, GLenum pname, const GLint* params) { fp_glMaterialiv(face, pname, params); }
inline void glPolygonStipple(const GLubyte* mask) { fp_glPolygonStipple(mask); }
inline void glShadeModel(GLenum mode) { fp_glShadeModel(mode); }
inline void glTexEnvf(GLenum target, GLenum pname, GLfloat param) { fp_glTexEnvf(target, pname, param); }
inline void glTexEnvfv(GLenum target, GLenum pname, const GLfloat* params) { fp_glTexEnvfv(target, pname, params); }
inline void glTexEnvi(GLenum target, GLenum pname, GLint param) { fp_glTexEnvi(target, pname, param); }
inline void glTexEnviv(GLenum target, GLenum pname, const GLint* params) { fp_glTexEnviv(target, pname, params); }
inline void glTexGend(GLenum coord, GLenum pname, GLdouble param) { fp_glTexGend(coord, pname, param); }
inline void glTexGendv(GLenum coord, GLenum pname, const GLdouble* params) { fp_glTexGendv(coord, pname, params); }
inline void glTexGenf(GLenum coord, GLenum pname, GLfloat param) { fp_glTexGenf(coord, pname, param); }
inline void glTexGenfv(GLenum coord, GLenum pname, const GLfloat* params) { fp_glTexGenfv(coord, pname, params); }
inline void glTexGeni(GLenum coord, GLenum pname, GLint param) { fp_glTexGeni(coord, pname, param); }
inline void glTexGeniv(GLenum coord, GLenum pname, const GLint* params) { fp_glTexGeniv(coord, pname, params); }
inline void glFeedbackBuffer(GLsizei size, GLenum type, GLfloat* buffer) { fp_glFeedbackBuffer(size, type, buffer); }
inline void glSelectBuffer(GLsizei size, GLuint* buffer) { fp_glSelectBuffer(size, buffer); }
inline GLint glRenderMode(GLenum mode) { return fp_glRenderMode(mode); }
inline void glInitNames() { fp_glInitNames(); }
inline void glLoadName(GLuint name) { fp_glLoadName(name); }
inline void glPassThrough(GLfloat token) { fp_glPassThrough(token); }
inline void glPopName() { fp_glPopName(); }
inline void glPushName(GLuint name) { fp_glPushName(name); }
inline void glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) { fp_glClearAccum(red, green, blue, alpha); }
inline void glClearIndex(GLfloat c) { fp_glClearIndex(c); }
inline void glIndexMask(GLuint mask) { fp_glIndexMask(mask); }
inline void glAccum(GLenum op, GLfloat value) { fp_glAccum(op, value); }
inline void glPopAttrib() { fp_glPopAttrib(); }
inline void glPushAttrib(GLbitfield mask) { fp_glPushAttrib(mask); }
inline void glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble* points) { fp_glMap1d(target, u1, u2, stride, order, points); }
inline void glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat* points) { fp_glMap1f(target, u1, u2, stride, order, points); }
inline void glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble* points) { fp_glMap2d(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points); }
inline void glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat* points) { fp_glMap2f(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points); }
inline void glMapGrid1d(GLint un, GLdouble u1, GLdouble u2) { fp_glMapGrid1d(un, u1, u2); }
inline void glMapGrid1f(GLint un, GLfloat u1, GLfloat u2) { fp_glMapGrid1f(un, u1, u2); }
inline void glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2) { fp_glMapGrid2d(un, u1, u2, vn, v1, v2); }
inline void glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2) { fp_glMapGrid2f(un, u1, u2, vn, v1, v2); }
inline void glEvalCoord1d(GLdouble u) { fp_glEvalCoord1d(u); }
inline void glEvalCoord1dv(const GLdouble* u) { fp_glEvalCoord1dv(u); }
inline void glEvalCoord1f(GLfloat u) { fp_glEvalCoord1f(u); }
inline void glEvalCoord1fv(const GLfloat* u) { fp_glEvalCoord1fv(u); }
inline void glEvalCoord2d(GLdouble u, GLdouble v) { fp_glEvalCoord2d(u, v); }
inline void glEvalCoord2dv(const GLdouble* u) { fp_glEvalCoord2dv(u); }
inline void glEvalCoord2f(GLfloat u, GLfloat v) { fp_glEvalCoord2f(u, v); }
inline void glEvalCoord2fv(const GLfloat* u) { fp_glEvalCoord2fv(u); }
inline void glEvalMesh1(GLenum mode, GLint i1, GLint i2) { fp_glEvalMesh1(mode, i1, i2); }
inline void glEvalPoint1(GLint i) { fp_glEvalPoint1(i); }
inline void glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2) { fp_glEvalMesh2(mode, i1, i2, j1, j2); }
inline void glEvalPoint2(GLint i, GLint j) { fp_glEvalPoint2(i, j); }
inline void glAlphaFunc(GLenum func, GLfloat ref) { fp_glAlphaFunc(func, ref); }
inline void glPixelZoom(GLfloat xfactor, GLfloat yfactor) { fp_glPixelZoom(xfactor, yfactor); }
inline void glPixelTransferf(GLenum pname, GLfloat param) { fp_glPixelTransferf(pname, param); }
inline void glPixelTransferi(GLenum pname, GLint param) { fp_glPixelTransferi(pname, param); }
inline void glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat* values) { fp_glPixelMapfv(map, mapsize, values); }
inline void glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint* values) { fp_glPixelMapuiv(map, mapsize, values); }
inline void glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort* values) { fp_glPixelMapusv(map, mapsize, values); }
inline void glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type) { fp_glCopyPixels(x, y, width, height, type); }
inline void glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) { fp_glDrawPixels(width, height, format, type, pixels); }
inline void glGetClipPlane(GLenum plane, GLdouble* equation) { fp_glGetClipPlane(plane, equation); }
inline void glGetLightfv(GLenum light, GLenum pname, GLfloat* params) { fp_glGetLightfv(light, pname, params); }
inline void glGetLightiv(GLenum light, GLenum pname, GLint* params) { fp_glGetLightiv(light, pname, params); }
inline void glGetMapdv(GLenum target, GLenum query, GLdouble* v) { fp_glGetMapdv(target, query, v); }
inline void glGetMapfv(GLenum target, GLenum query, GLfloat* v) { fp_glGetMapfv(target, query, v); }
inline void glGetMapiv(GLenum target, GLenum query, GLint* v) { fp_glGetMapiv(target, query, v); }
inline void glGetMaterialfv(GLenum face, GLenum pname, GLfloat* params) { fp_glGetMaterialfv(face, pname, params); }
inline void glGetMaterialiv(GLenum face, GLenum pname, GLint* params) { fp_glGetMaterialiv(face, pname, params); }
inline void glGetPixelMapfv(GLenum map, GLfloat* values) { fp_glGetPixelMapfv(map, values); }
inline void glGetPixelMapuiv(GLenum map, GLuint* values) { fp_glGetPixelMapuiv(map, values); }
inline void glGetPixelMapusv(GLenum map, GLushort* values) { fp_glGetPixelMapusv(map, values); }
inline void glGetPolygonStipple(GLubyte* mask) { fp_glGetPolygonStipple(mask); }
inline void glGetTexEnvfv(GLenum target, GLenum pname, GLfloat* params) { fp_glGetTexEnvfv(target, pname, params); }
inline void glGetTexEnviv(GLenum target, GLenum pname, GLint* params) { fp_glGetTexEnviv(target, pname, params); }
inline void glGetTexGendv(GLenum coord, GLenum pname, GLdouble* params) { fp_glGetTexGendv(coord, pname, params); }
inline void glGetTexGenfv(GLenum coord, GLenum pname, GLfloat* params) { fp_glGetTexGenfv(coord, pname, params); }
inline void glGetTexGeniv(GLenum coord, GLenum pname, GLint* params) { fp_glGetTexGeniv(coord, pname, params); }
inline GLboolean glIsList(GLuint list) { return fp_glIsList(list); }
inline void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) { fp_glFrustum(left, right, bottom, top, zNear, zFar); }
inline void glLoadIdentity() { fp_glLoadIdentity(); }
inline void glLoadMatrixf(const GLfloat* m) { fp_glLoadMatrixf(m); }
inline void glLoadMatrixd(const GLdouble* m) { fp_glLoadMatrixd(m); }
inline void glMatrixMode(GLenum mode) { fp_glMatrixMode(mode); }
inline void glMultMatrixf(const GLfloat* m) { fp_glMultMatrixf(m); }
inline void glMultMatrixd(const GLdouble* m) { fp_glMultMatrixd(m); }
inline void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) { fp_glOrtho(left, right, bottom, top, zNear, zFar); }
inline void glPopMatrix() { fp_glPopMatrix(); }
inline void glPushMatrix() { fp_glPushMatrix(); }
inline void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z) { fp_glRotated(angle, x, y, z); }
inline void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) { fp_glRotatef(angle, x, y, z); }
inline void glScaled(GLdouble x, GLdouble y, GLdouble z) { fp_glScaled(x, y, z); }
inline void glScalef(GLfloat x, GLfloat y, GLfloat z) { fp_glScalef(x, y, z); }
inline void glTranslated(GLdouble x, GLdouble y, GLdouble z) { fp_glTranslated(x, y, z); }
inline void glTranslatef(GLfloat x, GLfloat y, GLfloat z) { fp_glTranslatef(x, y, z); }

/* GL_VERSION_1_1 */
inline void glDrawArrays(GLenum mode, GLint first, GLsizei count) { fp_glDrawArrays(mode, first, count); }
inline void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) { fp_glDrawElements(mode, count, type, indices); }
inline void glGetPointerv(GLenum pname, void** params) { fp_glGetPointerv(pname, params); }
inline void glPolygonOffset(GLfloat factor, GLfloat units) { fp_glPolygonOffset(factor, units); }
inline void glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border) { fp_glCopyTexImage1D(target, level, internalformat, x, y, width, border); }
inline void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) { fp_glCopyTexImage2D(target, level, internalformat, x, y, width, height, border); }
inline void glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) { fp_glCopyTexSubImage1D(target, level, xoffset, x, y, width); }
inline void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) { fp_glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height); }
inline void glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels) { fp_glTexSubImage1D(target, level, xoffset, width, format, type, pixels); }
inline void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) { fp_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels); }
inline void glBindTexture(GLenum target, GLuint texture) { fp_glBindTexture(target, texture); }
inline void glDeleteTextures(GLsizei n, const GLuint* textures) { fp_glDeleteTextures(n, textures); }
inline void glGenTextures(GLsizei n, GLuint* textures) { fp_glGenTextures(n, textures); }
inline GLboolean glIsTexture(GLuint texture) { return fp_glIsTexture(texture); }
inline void glArrayElement(GLint i) { fp_glArrayElement(i); }
inline void glColorPointer(GLint size, GLenum type, GLsizei stride, const void* pointer) { fp_glColorPointer(size, type, stride, pointer); }
inline void glDisableClientState(GLenum array) { fp_glDisableClientState(array); }
inline void glEdgeFlagPointer(GLsizei stride, const void* pointer) { fp_glEdgeFlagPointer(stride, pointer); }
inline void glEnableClientState(GLenum array) { fp_glEnableClientState(array); }
inline void glIndexPointer(GLenum type, GLsizei stride, const void* pointer) { fp_glIndexPointer(type, stride, pointer); }
inline void glInterleavedArrays(GLenum format, GLsizei stride, const void* pointer) { fp_glInterleavedArrays(format, stride, pointer); }
inline void glNormalPointer(GLenum type, GLsizei stride, const void* pointer) { fp_glNormalPointer(type, stride, pointer); }
inline void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void* pointer) { fp_glTexCoordPointer(size, type, stride, pointer); }
inline void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void* pointer) { fp_glVertexPointer(size, type, stride, pointer); }
inline GLboolean glAreTexturesResident(GLsizei n, const GLuint* textures, GLboolean* residences) { return fp_glAreTexturesResident(n, textures, residences); }
inline void glPrioritizeTextures(GLsizei n, const GLuint* textures, const GLfloat* priorities) { fp_glPrioritizeTextures(n, textures, priorities); }
inline void glIndexub(GLubyte c) { fp_glIndexub(c); }
inline void glIndexubv(const GLubyte* c) { fp_glIndexubv(c); }
inline void glPopClientAttrib() { fp_glPopClientAttrib(); }
inline void glPushClientAttrib(GLbitfield mask) { fp_glPushClientAttrib(mask); }

/* GL_VERSION_1_2 */
inline void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices) { fp_glDrawRangeElements(mode, start, end, count, type, indices); }
inline void glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) { fp_glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels); }
inline void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) { fp_glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels); }
inline void glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) { fp_glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height); }

/* GL_VERSION_1_3 */
inline void glActiveTexture(GLenum texture) { fp_glActiveTexture(texture); }
inline void glSampleCoverage(GLfloat value, GLboolean invert) { fp_glSampleCoverage(value, invert); }
inline void glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data) { fp_glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data); }
inline void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data) { fp_glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data); }
inline void glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* data) { fp_glCompressedTexImage1D(target, level, internalformat, width, border, imageSize, data); }
inline void glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data) { fp_glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data); }
inline void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data) { fp_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data); }
inline void glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data) { fp_glCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data); }
inline void glGetCompressedTexImage(GLenum target, GLint level, void* img) { fp_glGetCompressedTexImage(target, level, img); }
inline void glClientActiveTexture(GLenum texture) { fp_glClientActiveTexture(texture); }
inline void glMultiTexCoord1d(GLenum target, GLdouble s) { fp_glMultiTexCoord1d(target, s); }
inline void glMultiTexCoord1dv(GLenum target, const GLdouble* v) { fp_glMultiTexCoord1dv(target, v); }
inline void glMultiTexCoord1f(GLenum target, GLfloat s) { fp_glMultiTexCoord1f(target, s); }
inline void glMultiTexCoord1fv(GLenum target, const GLfloat* v) { fp_glMultiTexCoord1fv(target, v); }
inline void glMultiTexCoord1i(GLenum target, GLint s) { fp_glMultiTexCoord1i(target, s); }
inline void glMultiTexCoord1iv(GLenum target, const GLint* v) { fp_glMultiTexCoord1iv(target, v); }
inline void glMultiTexCoord1s(GLenum target, GLshort s) { fp_glMultiTexCoord1s(target, s); }
inline void glMultiTexCoord1sv(GLenum target, const GLshort* v) { fp_glMultiTexCoord1sv(target, v); }
inline void glMultiTexCoord2d(GLenum target, GLdouble s, GLdouble t) { fp_glMultiTexCoord2d(target, s, t); }
inline void glMultiTexCoord2dv(GLenum target, const GLdouble* v) { fp_glMultiTexCoord2dv(target, v); }
inline void glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t) { fp_glMultiTexCoord2f(target, s, t); }
inline void glMultiTexCoord2fv(GLenum target, const GLfloat* v) { fp_glMultiTexCoord2fv(target, v); }
inline void glMultiTexCoord2i(GLenum target, GLint s, GLint t) { fp_glMultiTexCoord2i(target, s, t); }
inline void glMultiTexCoord2iv(GLenum target, const GLint* v) { fp_glMultiTexCoord2iv(target, v); }
inline void glMultiTexCoord2s(GLenum target, GLshort s, GLshort t) { fp_glMultiTexCoord2s(target, s, t); }
inline void glMultiTexCoord2sv(GLenum target, const GLshort* v) { fp_glMultiTexCoord2sv(target, v); }
inline void glMultiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r) { fp_glMultiTexCoord3d(target, s, t, r); }
inline void glMultiTexCoord3dv(GLenum target, const GLdouble* v) { fp_glMultiTexCoord3dv(target, v); }
inline void glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r) { fp_glMultiTexCoord3f(target, s, t, r); }
inline void glMultiTexCoord3fv(GLenum target, const GLfloat* v) { fp_glMultiTexCoord3fv(target, v); }
inline void glMultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r) { fp_glMultiTexCoord3i(target, s, t, r); }
inline void glMultiTexCoord3iv(GLenum target, const GLint* v) { fp_glMultiTexCoord3iv(target, v); }
inline void glMultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r) { fp_glMultiTexCoord3s(target, s, t, r); }
inline void glMultiTexCoord3sv(GLenum target, const GLshort* v) { fp_glMultiTexCoord3sv(target, v); }
inline void glMultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q) { fp_glMultiTexCoord4d(target, s, t, r, q); }
inline void glMultiTexCoord4dv(GLenum target, const GLdouble* v) { fp_glMultiTexCoord4dv(target, v); }
inline void glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) { fp_glMultiTexCoord4f(target, s, t, r, q); }
inline void glMultiTexCoord4fv(GLenum target, const GLfloat* v) { fp_glMultiTexCoord4fv(target, v); }
inline void glMultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q) { fp_glMultiTexCoord4i(target, s, t, r, q); }
inline void glMultiTexCoord4iv(GLenum target, const GLint* v) { fp_glMultiTexCoord4iv(target, v); }
inline void glMultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q) { fp_glMultiTexCoord4s(target, s, t, r, q); }
inline void glMultiTexCoord4sv(GLenum target, const GLshort* v) { fp_glMultiTexCoord4sv(target, v); }
inline void glLoadTransposeMatrixf(const GLfloat* m) { fp_glLoadTransposeMatrixf(m); }
inline void glLoadTransposeMatrixd(const GLdouble* m) { fp_glLoadTransposeMatrixd(m); }
inline void glMultTransposeMatrixf(const GLfloat* m) { fp_glMultTransposeMatrixf(m); }
inline void glMultTransposeMatrixd(const GLdouble* m) { fp_glMultTransposeMatrixd(m); }

/* GL_VERSION_1_4 */
inline void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) { fp_glBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha); }
inline void glMultiDrawArrays(GLenum mode, const GLint* first, const GLsizei* count, GLsizei drawcount) { fp_glMultiDrawArrays(mode, first, count, drawcount); }
inline void glMultiDrawElements(GLenum mode, const GLsizei* count, GLenum type, const void** indices, GLsizei drawcount) { fp_glMultiDrawElements(mode, count, type, indices, drawcount); }
inline void glPointParameterf(GLenum pname, GLfloat param) { fp_glPointParameterf(pname, param); }
inline void glPointParameterfv(GLenum pname, const GLfloat* params) { fp_glPointParameterfv(pname, params); }
inline void glPointParameteri(GLenum pname, GLint param) { fp_glPointParameteri(pname, param); }
inline void glPointParameteriv(GLenum pname, const GLint* params) { fp_glPointParameteriv(pname, params); }
inline void glFogCoordf(GLfloat coord) { fp_glFogCoordf(coord); }
inline void glFogCoordfv(const GLfloat* coord) { fp_glFogCoordfv(coord); }
inline void glFogCoordd(GLdouble coord) { fp_glFogCoordd(coord); }
inline void glFogCoorddv(const GLdouble* coord) { fp_glFogCoorddv(coord); }
inline void glFogCoordPointer(GLenum type, GLsizei stride, const void* pointer) { fp_glFogCoordPointer(type, stride, pointer); }
inline void glSecondaryColor3b(GLbyte red, GLbyte green, GLbyte blue) { fp_glSecondaryColor3b(red, green, blue); }
inline void glSecondaryColor3bv(const GLbyte* v) { fp_glSecondaryColor3bv(v); }
inline void glSecondaryColor3d(GLdouble red, GLdouble green, GLdouble blue) { fp_glSecondaryColor3d(red, green, blue); }
inline void glSecondaryColor3dv(const GLdouble* v) { fp_glSecondaryColor3dv(v); }
inline void glSecondaryColor3f(GLfloat red, GLfloat green, GLfloat blue) { fp_glSecondaryColor3f(red, green, blue); }
inline void glSecondaryColor3fv(const GLfloat* v) { fp_glSecondaryColor3fv(v); }
inline void glSecondaryColor3i(GLint red, GLint green, GLint blue) { fp_glSecondaryColor3i(red, green, blue); }
inline void glSecondaryColor3iv(const GLint* v) { fp_glSecondaryColor3iv(v); }
inline void glSecondaryColor3s(GLshort red, GLshort green, GLshort blue) { fp_glSecondaryColor3s(red, green, blue); }
inline void glSecondaryColor3sv(const GLshort* v) { fp_glSecondaryColor3sv(v); }
inline void glSecondaryColor3ub(GLubyte red, GLubyte green, GLubyte blue) { fp_glSecondaryColor3ub(red, green, blue); }
inline void glSecondaryColor3ubv(const GLubyte* v) { fp_glSecondaryColor3ubv(v); }
inline void glSecondaryColor3ui(GLuint red, GLuint green, GLuint blue) { fp_glSecondaryColor3ui(red, green, blue); }
inline void glSecondaryColor3uiv(const GLuint* v) { fp_glSecondaryColor3uiv(v); }
inline void glSecondaryColor3us(GLushort red, GLushort green, GLushort blue) { fp_glSecondaryColor3us(red, green, blue); }
inline void glSecondaryColor3usv(const GLushort* v) { fp_glSecondaryColor3usv(v); }
inline void glSecondaryColorPointer(GLint size, GLenum type, GLsizei stride, const void* pointer) { fp_glSecondaryColorPointer(size, type, stride, pointer); }
inline void glWindowPos2d(GLdouble x, GLdouble y) { fp_glWindowPos2d(x, y); }
inline void glWindowPos2dv(const GLdouble* v) { fp_glWindowPos2dv(v); }
inline void glWindowPos2f(GLfloat x, GLfloat y) { fp_glWindowPos2f(x, y); }
inline void glWindowPos2fv(const GLfloat* v) { fp_glWindowPos2fv(v); }
inline void glWindowPos2i(GLint x, GLint y) { fp_glWindowPos2i(x, y); }
inline void glWindowPos2iv(const GLint* v) { fp_glWindowPos2iv(v); }
inline void glWindowPos2s(GLshort x, GLshort y) { fp_glWindowPos2s(x, y); }
inline void glWindowPos2sv(const GLshort* v) { fp_glWindowPos2sv(v); }
inline void glWindowPos3d(GLdouble x, GLdouble y, GLdouble z) { fp_glWindowPos3d(x, y, z); }
inline void glWindowPos3dv(const GLdouble* v) { fp_glWindowPos3dv(v); }
inline void glWindowPos3f(GLfloat x, GLfloat y, GLfloat z) { fp_glWindowPos3f(x, y, z); }
inline void glWindowPos3fv(const GLfloat* v) { fp_glWindowPos3fv(v); }
inline void glWindowPos3i(GLint x, GLint y, GLint z) { fp_glWindowPos3i(x, y, z); }
inline void glWindowPos3iv(const GLint* v) { fp_glWindowPos3iv(v); }
inline void glWindowPos3s(GLshort x, GLshort y, GLshort z) { fp_glWindowPos3s(x, y, z); }
inline void glWindowPos3sv(const GLshort* v) { fp_glWindowPos3sv(v); }
inline void glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) { fp_glBlendColor(red, green, blue, alpha); }
inline void glBlendEquation(GLenum mode) { fp_glBlendEquation(mode); }

/* GL_VERSION_1_5 */
inline void glGenQueries(GLsizei n, GLuint* ids) { fp_glGenQueries(n, ids); }
inline void glDeleteQueries(GLsizei n, const GLuint* ids) { fp_glDeleteQueries(n, ids); }
inline GLboolean glIsQuery(GLuint id) { return fp_glIsQuery(id); }
inline void glBeginQuery(GLenum target, GLuint id) { fp_glBeginQuery(target, id); }
inline void glEndQuery(GLenum target) { fp_glEndQuery(target); }
inline void glGetQueryiv(GLenum target, GLenum pname, GLint* params) { fp_glGetQueryiv(target, pname, params); }
inline void glGetQueryObjectiv(GLuint id, GLenum pname, GLint* params) { fp_glGetQueryObjectiv(id, pname, params); }
inline void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params) { fp_glGetQueryObjectuiv(id, pname, params); }
inline void glBindBuffer(GLenum target, GLuint buffer) { fp_glBindBuffer(target, buffer); }
inline void glDeleteBuffers(GLsizei n, const GLuint* buffers) { fp_glDeleteBuffers(n, buffers); }
inline void glGenBuffers(GLsizei n, GLuint* buffers) { fp_glGenBuffers(n, buffers); }
inline GLboolean glIsBuffer(GLuint buffer) { return fp_glIsBuffer(buffer); }
inline void glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) { fp_glBufferData(target, size, data, usage); }
inline void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) { fp_glBufferSubData(target, offset, size, data); }
inline void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void* data) { fp_glGetBufferSubData(target, offset, size, data); }
inline void* glMapBuffer(GLenum target, GLenum access) { return fp_glMapBuffer(target, access); }
inline GLboolean glUnmapBuffer(GLenum target) { return fp_glUnmapBuffer(target); }
inline void glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params) { fp_glGetBufferParameteriv(target, pname, params); }
inline void glGetBufferPointerv(GLenum target, GLenum pname, void** params) { fp_glGetBufferPointerv(target, pname, params); }

/* GL_VERSION_2_0 */
inline void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha) { fp_glBlendEquationSeparate(modeRGB, modeAlpha); }
inline void glDrawBuffers(GLsizei n, const GLenum* bufs) { fp_glDrawBuffers(n, bufs); }
inline void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) { fp_glStencilOpSeparate(face, sfail, dpfail, dppass); }
inline void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) { fp_glStencilFuncSeparate(face, func, ref, mask); }
inline void glStencilMaskSeparate(GLenum face, GLuint mask) { fp_glStencilMaskSeparate(face, mask); }
inline void glAttachShader(GLuint program, GLuint shader) { fp_glAttachShader(program, shader); }
inline void glBindAttribLocation(GLuint program, GLuint index, const GLchar* name) { fp_glBindAttribLocation(program, index, name); }
inline void glCompileShader(GLuint shader) { fp_glCompileShader(shader); }
inline GLuint glCreateProgram() { return fp_glCreateProgram(); }
inline GLuint glCreateShader(GLenum type) { return fp_glCreateShader(type); }
inline void glDeleteProgram(GLuint program) { fp_glDeleteProgram(program); }
inline void glDeleteShader(GLuint shader) { fp_glDeleteShader(shader); }
inline void glDetachShader(GLuint program, GLuint shader) { fp_glDetachShader(program, shader); }
inline void glDisableVertexAttribArray(GLuint index) { fp_glDisableVertexAttribArray(index); }
inline void glEnableVertexAttribArray(GLuint index) { fp_glEnableVertexAttribArray(index); }
inline void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) { fp_glGetActiveAttrib(program, index, bufSize, length, size, type, name); }
inline void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) { fp_glGetActiveUniform(program, index, bufSize, length, size, type, name); }
inline void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders) { fp_glGetAttachedShaders(program, maxCount, count, shaders); }
inline GLint glGetAttribLocation(GLuint program, const GLchar* name) { return fp_glGetAttribLocation(program, name); }
inline void glGetProgramiv(GLuint program, GLenum pname, GLint* params) { fp_glGetProgramiv(program, pname, params); }
inline void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog) { fp_glGetProgramInfoLog(program, bufSize, length, infoLog); }
inline void glGetShaderiv(GLuint shader, GLenum pname, GLint* params) { fp_glGetShaderiv(shader, pname, params); }
inline void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog) { fp_glGetShaderInfoLog(shader, bufSize, length, infoLog); }
inline void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source) { fp_glGetShaderSource(shader, bufSize, length, source); }
inline GLint glGetUniformLocation(GLuint program, const GLchar* name) { return fp_glGetUniformLocation(program, name); }
inline void glGetUniformfv(GLuint program, GLint location, GLfloat* params) { fp_glGetUniformfv(program, location, params); }
inline void glGetUniformiv(GLuint program, GLint location, GLint* params) { fp_glGetUniformiv(program, location, params); }
inline void glGetVertexAttribdv(GLuint index, GLenum pname, GLdouble* params) { fp_glGetVertexAttribdv(index, pname, params); }
inline void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params) { fp_glGetVertexAttribfv(index, pname, params); }
inline void glGetVertexAttribiv(GLuint index, GLenum pname, GLint* params) { fp_glGetVertexAttribiv(index, pname, params); }
inline void glGetVertexAttribPointerv(GLuint index, GLenum pname, void** pointer) { fp_glGetVertexAttribPointerv(index, pname, pointer); }
inline GLboolean glIsProgram(GLuint program) { return fp_glIsProgram(program); }
inline GLboolean glIsShader(GLuint shader) { return fp_glIsShader(shader); }
inline void glLinkProgram(GLuint program) { fp_glLinkProgram(program); }
inline void glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length) { fp_glShaderSource(shader, count, string, length); }
inline void glUseProgram(GLuint program) { fp_glUseProgram(program); }
inline void glUniform1f(GLint location, GLfloat v0) { fp_glUniform1f(location, v0); }
inline void glUniform2f(GLint location, GLfloat v0, GLfloat v1) { fp_glUniform2f(location, v0, v1); }
inline void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) { fp_glUniform3f(location, v0, v1, v2); }
inline void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) { fp_glUniform4f(location, v0, v1, v2, v3); }
inline void glUniform1i(GLint location, GLint v0) { fp_glUniform1i(location, v0); }
inline void glUniform2i(GLint location, GLint v0, GLint v1) { fp_glUniform2i(location, v0, v1); }
inline void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) { fp_glUniform3i(location, v0, v1, v2); }
inline void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) { fp_glUniform4i(location, v0, v1, v2, v3); }
inline void glUniform1fv(GLint location, GLsizei count, const GLfloat* value) { fp_glUniform1fv(location, count, value); }
inline void glUniform2fv(GLint location, GLsizei count, const GLfloat* value) { fp_glUniform2fv(location, count, value); }
inline void glUniform3fv(GLint location, GLsizei count, const GLfloat* value) { fp_glUniform3fv(location, count, value); }
inline void glUniform4fv(GLint location, GLsizei count, const GLfloat* value) { fp_glUniform4fv(location, count, value); }
inline void glUniform1iv(GLint location, GLsizei count, const GLint* value) { fp_glUniform1iv(location, count, value); }
inline void glUniform2iv(GLint location, GLsizei count, const GLint* value) { fp_glUniform2iv(location, count, value); }
inline void glUniform3iv(GLint location, GLsizei count, const GLint* value) { fp_glUniform3iv(location, count, value); }
inline void glUniform4iv(GLint location, GLsizei count, const GLint* value) { fp_glUniform4iv(location, count, value); }
inline void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glUniformMatrix2fv(location, count, transpose, value); }
inline void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glUniformMatrix3fv(location, count, transpose, value); }
inline void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glUniformMatrix4fv(location, count, transpose, value); }
inline void glValidateProgram(GLuint program) { fp_glValidateProgram(program); }
inline void glVertexAttrib1d(GLuint index, GLdouble x) { fp_glVertexAttrib1d(index, x); }
inline void glVertexAttrib1dv(GLuint index, const GLdouble* v) { fp_glVertexAttrib1dv(index, v); }
inline void glVertexAttrib1f(GLuint index, GLfloat x) { fp_glVertexAttrib1f(index, x); }
inline void glVertexAttrib1fv(GLuint index, const GLfloat* v) { fp_glVertexAttrib1fv(index, v); }
inline void glVertexAttrib1s(GLuint index, GLshort x) { fp_glVertexAttrib1s(index, x); }
inline void glVertexAttrib1sv(GLuint index, const GLshort* v) { fp_glVertexAttrib1sv(index, v); }
inline void glVertexAttrib2d(GLuint index, GLdouble x, GLdouble y) { fp_glVertexAttrib2d(index, x, y); }
inline void glVertexAttrib2dv(GLuint index, const GLdouble* v) { fp_glVertexAttrib2dv(index, v); }
inline void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y) { fp_glVertexAttrib2f(index, x, y); }
inline void glVertexAttrib2fv(GLuint index, const GLfloat* v) { fp_glVertexAttrib2fv(index, v); }
inline void glVertexAttrib2s(GLuint index, GLshort x, GLshort y) { fp_glVertexAttrib2s(index, x, y); }
inline void glVertexAttrib2sv(GLuint index, const GLshort* v) { fp_glVertexAttrib2sv(index, v); }
inline void glVertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z) { fp_glVertexAttrib3d(index, x, y, z); }
inline void glVertexAttrib3dv(GLuint index, const GLdouble* v) { fp_glVertexAttrib3dv(index, v); }
inline void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z) { fp_glVertexAttrib3f(index, x, y, z); }
inline void glVertexAttrib3fv(GLuint index, const GLfloat* v) { fp_glVertexAttrib3fv(index, v); }
inline void glVertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z) { fp_glVertexAttrib3s(index, x, y, z); }
inline void glVertexAttrib3sv(GLuint index, const GLshort* v) { fp_glVertexAttrib3sv(index, v); }
inline void glVertexAttrib4Nbv(GLuint index, const GLbyte* v) { fp_glVertexAttrib4Nbv(index, v); }
inline void glVertexAttrib4Niv(GLuint index, const GLint* v) { fp_glVertexAttrib4Niv(index, v); }
inline void glVertexAttrib4Nsv(GLuint index, const GLshort* v) { fp_glVertexAttrib4Nsv(index, v); }
inline void glVertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w) { fp_glVertexAttrib4Nub(index, x, y, z, w); }
inline void glVertexAttrib4Nubv(GLuint index, const GLubyte* v) { fp_glVertexAttrib4Nubv(index, v); }
inline void glVertexAttrib4Nuiv(GLuint index, const GLuint* v) { fp_glVertexAttrib4Nuiv(index, v); }
inline void glVertexAttrib4Nusv(GLuint index, const GLushort* v) { fp_glVertexAttrib4Nusv(index, v); }
inline void glVertexAttrib4bv(GLuint index, const GLbyte* v) { fp_glVertexAttrib4bv(index, v); }
inline void glVertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) { fp_glVertexAttrib4d(index, x, y, z, w); }
inline void glVertexAttrib4dv(GLuint index, const GLdouble* v) { fp_glVertexAttrib4dv(index, v); }
inline void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) { fp_glVertexAttrib4f(index, x, y, z, w); }
inline void glVertexAttrib4fv(GLuint index, const GLfloat* v) { fp_glVertexAttrib4fv(index, v); }
inline void glVertexAttrib4iv(GLuint index, const GLint* v) { fp_glVertexAttrib4iv(index, v); }
inline void glVertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w) { fp_glVertexAttrib4s(index, x, y, z, w); }
inline void glVertexAttrib4sv(GLuint index, const GLshort* v) { fp_glVertexAttrib4sv(index, v); }
inline void glVertexAttrib4ubv(GLuint index, const GLubyte* v) { fp_glVertexAttrib4ubv(index, v); }
inline void glVertexAttrib4uiv(GLuint index, const GLuint* v) { fp_glVertexAttrib4uiv(index, v); }
inline void glVertexAttrib4usv(GLuint index, const GLushort* v) { fp_glVertexAttrib4usv(index, v); }
inline void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) { fp_glVertexAttribPointer(index, size, type, normalized, stride, pointer); }

/* GL_VERSION_2_1 */
inline void glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glUniformMatrix2x3fv(location, count, transpose, value); }
inline void glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glUniformMatrix3x2fv(location, count, transpose, value); }
inline void glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glUniformMatrix2x4fv(location, count, transpose, value); }
inline void glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glUniformMatrix4x2fv(location, count, transpose, value); }
inline void glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glUniformMatrix3x4fv(location, count, transpose, value); }
inline void glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glUniformMatrix4x3fv(location, count, transpose, value); }

/* GL_VERSION_3_0 */
inline void glColorMaski(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a) { fp_glColorMaski(index, r, g, b, a); }
inline void glGetBooleani_v(GLenum target, GLuint index, GLboolean* data) { fp_glGetBooleani_v(target, index, data); }
inline void glGetIntegeri_v(GLenum target, GLuint index, GLint* data) { fp_glGetIntegeri_v(target, index, data); }
inline void glEnablei(GLenum target, GLuint index) { fp_glEnablei(target, index); }
inline void glDisablei(GLenum target, GLuint index) { fp_glDisablei(target, index); }
inline GLboolean glIsEnabledi(GLenum target, GLuint index) { return fp_glIsEnabledi(target, index); }
inline void glBeginTransformFeedback(GLenum primitiveMode) { fp_glBeginTransformFeedback(primitiveMode); }
inline void glEndTransformFeedback() { fp_glEndTransformFeedback(); }
inline void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) { fp_glBindBufferRange(target, index, buffer, offset, size); }
inline void glBindBufferBase(GLenum target, GLuint index, GLuint buffer) { fp_glBindBufferBase(target, index, buffer); }
inline void glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar** varyings, GLenum bufferMode) { fp_glTransformFeedbackVaryings(program, count, varyings, bufferMode); }
inline void glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name) { fp_glGetTransformFeedbackVarying(program, index, bufSize, length, size, type, name); }
inline void glClampColor(GLenum target, GLenum clamp) { fp_glClampColor(target, clamp); }
inline void glBeginConditionalRender(GLuint id, GLenum mode) { fp_glBeginConditionalRender(id, mode); }
inline void glEndConditionalRender() { fp_glEndConditionalRender(); }
inline void glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) { fp_glVertexAttribIPointer(index, size, type, stride, pointer); }
inline void glGetVertexAttribIiv(GLuint index, GLenum pname, GLint* params) { fp_glGetVertexAttribIiv(index, pname, params); }
inline void glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params) { fp_glGetVertexAttribIuiv(index, pname, params); }
inline void glVertexAttribI1i(GLuint index, GLint x) { fp_glVertexAttribI1i(index, x); }
inline void glVertexAttribI2i(GLuint index, GLint x, GLint y) { fp_glVertexAttribI2i(index, x, y); }
inline void glVertexAttribI3i(GLuint index, GLint x, GLint y, GLint z) { fp_glVertexAttribI3i(index, x, y, z); }
inline void glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w) { fp_glVertexAttribI4i(index, x, y, z, w); }
inline void glVertexAttribI1ui(GLuint index, GLuint x) { fp_glVertexAttribI1ui(index, x); }
inline void glVertexAttribI2ui(GLuint index, GLuint x, GLuint y) { fp_glVertexAttribI2ui(index, x, y); }
inline void glVertexAttribI3ui(GLuint index, GLuint x, GLuint y, GLuint z) { fp_glVertexAttribI3ui(index, x, y, z); }
inline void glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) { fp_glVertexAttribI4ui(index, x, y, z, w); }
inline void glVertexAttribI1iv(GLuint index, const GLint* v) { fp_glVertexAttribI1iv(index, v); }
inline void glVertexAttribI2iv(GLuint index, const GLint* v) { fp_glVertexAttribI2iv(index, v); }
inline void glVertexAttribI3iv(GLuint index, const GLint* v) { fp_glVertexAttribI3iv(index, v); }
inline void glVertexAttribI4iv(GLuint index, const GLint* v) { fp_glVertexAttribI4iv(index, v); }
inline void glVertexAttribI1uiv(GLuint index, const GLuint* v) { fp_glVertexAttribI1uiv(index, v); }
inline void glVertexAttribI2uiv(GLuint index, const GLuint* v) { fp_glVertexAttribI2uiv(index, v); }
inline void glVertexAttribI3uiv(GLuint index, const GLuint* v) { fp_glVertexAttribI3uiv(index, v); }
inline void glVertexAttribI4uiv(GLuint index, const GLuint* v) { fp_glVertexAttribI4uiv(index, v); }
inline void glVertexAttribI4bv(GLuint index, const GLbyte* v) { fp_glVertexAttribI4bv(index, v); }
inline void glVertexAttribI4sv(GLuint index, const GLshort* v) { fp_glVertexAttribI4sv(index, v); }
inline void glVertexAttribI4ubv(GLuint index, const GLubyte* v) { fp_glVertexAttribI4ubv(index, v); }
inline void glVertexAttribI4usv(GLuint index, const GLushort* v) { fp_glVertexAttribI4usv(index, v); }
inline void glGetUniformuiv(GLuint program, GLint location, GLuint* params) { fp_glGetUniformuiv(program, location, params); }
inline void glBindFragDataLocation(GLuint program, GLuint color, const GLchar* name) { fp_glBindFragDataLocation(program, color, name); }
inline GLint glGetFragDataLocation(GLuint program, const GLchar* name) { return fp_glGetFragDataLocation(program, name); }
inline void glUniform1ui(GLint location, GLuint v0) { fp_glUniform1ui(location, v0); }
inline void glUniform2ui(GLint location, GLuint v0, GLuint v1) { fp_glUniform2ui(location, v0, v1); }
inline void glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2) { fp_glUniform3ui(location, v0, v1, v2); }
inline void glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) { fp_glUniform4ui(location, v0, v1, v2, v3); }
inline void glUniform1uiv(GLint location, GLsizei count, const GLuint* value) { fp_glUniform1uiv(location, count, value); }
inline void glUniform2uiv(GLint location, GLsizei count, const GLuint* value) { fp_glUniform2uiv(location, count, value); }
inline void glUniform3uiv(GLint location, GLsizei count, const GLuint* value) { fp_glUniform3uiv(location, count, value); }
inline void glUniform4uiv(GLint location, GLsizei count, const GLuint* value) { fp_glUniform4uiv(location, count, value); }
inline void glTexParameterIiv(GLenum target, GLenum pname, const GLint* params) { fp_glTexParameterIiv(target, pname, params); }
inline void glTexParameterIuiv(GLenum target, GLenum pname, const GLuint* params) { fp_glTexParameterIuiv(target, pname, params); }
inline void glGetTexParameterIiv(GLenum target, GLenum pname, GLint* params) { fp_glGetTexParameterIiv(target, pname, params); }
inline void glGetTexParameterIuiv(GLenum target, GLenum pname, GLuint* params) { fp_glGetTexParameterIuiv(target, pname, params); }
inline void glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value) { fp_glClearBufferiv(buffer, drawbuffer, value); }
inline void glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value) { fp_glClearBufferuiv(buffer, drawbuffer, value); }
inline void glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value) { fp_glClearBufferfv(buffer, drawbuffer, value); }
inline void glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) { fp_glClearBufferfi(buffer, drawbuffer, depth, stencil); }
inline const GLubyte* glGetStringi(GLenum name, GLuint index) { return fp_glGetStringi(name, index); }
inline GLboolean glIsRenderbuffer(GLuint renderbuffer) { return fp_glIsRenderbuffer(renderbuffer); }
inline void glBindRenderbuffer(GLenum target, GLuint renderbuffer) { fp_glBindRenderbuffer(target, renderbuffer); }
inline void glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers) { fp_glDeleteRenderbuffers(n, renderbuffers); }
inline void glGenRenderbuffers(GLsizei n, GLuint* renderbuffers) { fp_glGenRenderbuffers(n, renderbuffers); }
inline void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) { fp_glRenderbufferStorage(target, internalformat, width, height); }
inline void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params) { fp_glGetRenderbufferParameteriv(target, pname, params); }
inline GLboolean glIsFramebuffer(GLuint framebuffer) { return fp_glIsFramebuffer(framebuffer); }
inline void glBindFramebuffer(GLenum target, GLuint framebuffer) { fp_glBindFramebuffer(target, framebuffer); }
inline void glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers) { fp_glDeleteFramebuffers(n, framebuffers); }
inline void glGenFramebuffers(GLsizei n, GLuint* framebuffers) { fp_glGenFramebuffers(n, framebuffers); }
inline GLenum glCheckFramebufferStatus(GLenum target) { return fp_glCheckFramebufferStatus(target); }
inline void glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) { fp_glFramebufferTexture1D(target, attachment, textarget, texture, level); }
inline void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) { fp_glFramebufferTexture2D(target, attachment, textarget, texture, level); }
inline void glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) { fp_glFramebufferTexture3D(target, attachment, textarget, texture, level, zoffset); }
inline void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) { fp_glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer); }
inline void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params) { fp_glGetFramebufferAttachmentParameteriv(target, attachment, pname, params); }
inline void glGenerateMipmap(GLenum target) { fp_glGenerateMipmap(target); }
inline void glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) { fp_glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter); }
inline void glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) { fp_glRenderbufferStorageMultisample(target, samples, internalformat, width, height); }
inline void glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) { fp_glFramebufferTextureLayer(target, attachment, texture, level, layer); }
inline void* glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) { return fp_glMapBufferRange(target, offset, length, access); }
inline void glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length) { fp_glFlushMappedBufferRange(target, offset, length); }
inline void glBindVertexArray(GLuint array) { fp_glBindVertexArray(array); }
inline void glDeleteVertexArrays(GLsizei n, const GLuint* arrays) { fp_glDeleteVertexArrays(n, arrays); }
inline void glGenVertexArrays(GLsizei n, GLuint* arrays) { fp_glGenVertexArrays(n, arrays); }
inline GLboolean glIsVertexArray(GLuint array) { return fp_glIsVertexArray(array); }

/* GL_VERSION_3_1 */
inline void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) { fp_glDrawArraysInstanced(mode, first, count, instancecount); }
inline void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount) { fp_glDrawElementsInstanced(mode, count, type, indices, instancecount); }
inline void glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer) { fp_glTexBuffer(target, internalformat, buffer); }
inline void glPrimitiveRestartIndex(GLuint index) { fp_glPrimitiveRestartIndex(index); }
inline void glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) { fp_glCopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size); }
inline void glGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar** uniformNames, GLuint* uniformIndices) { fp_glGetUniformIndices(program, uniformCount, uniformNames, uniformIndices); }
inline void glGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params) { fp_glGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params); }
inline void glGetActiveUniformName(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformName) { fp_glGetActiveUniformName(program, uniformIndex, bufSize, length, uniformName); }
inline GLuint glGetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName) { return fp_glGetUniformBlockIndex(program, uniformBlockName); }
inline void glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params) { fp_glGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params); }
inline void glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName) { fp_glGetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName); }
inline void glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) { fp_glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding); }

/* GL_VERSION_3_2 */
inline void glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex) { fp_glDrawElementsBaseVertex(mode, count, type, indices, basevertex); }
inline void glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices, GLint basevertex) { fp_glDrawRangeElementsBaseVertex(mode, start, end, count, type, indices, basevertex); }
inline void glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex) { fp_glDrawElementsInstancedBaseVertex(mode, count, type, indices, instancecount, basevertex); }
inline void glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei* count, GLenum type, const void** indices, GLsizei drawcount, const GLint* basevertex) { fp_glMultiDrawElementsBaseVertex(mode, count, type, indices, drawcount, basevertex); }
inline void glProvokingVertex(GLenum mode) { fp_glProvokingVertex(mode); }
inline GLsync glFenceSync(GLenum condition, GLbitfield flags) { return fp_glFenceSync(condition, flags); }
inline GLboolean glIsSync(GLsync sync) { return fp_glIsSync(sync); }
inline void glDeleteSync(GLsync sync) { fp_glDeleteSync(sync); }
inline GLenum glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) { return fp_glClientWaitSync(sync, flags, timeout); }
inline void glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) { fp_glWaitSync(sync, flags, timeout); }
inline void glGetInteger64v(GLenum pname, GLint64* data) { fp_glGetInteger64v(pname, data); }
inline void glGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values) { fp_glGetSynciv(sync, pname, bufSize, length, values); }
inline void glGetInteger64i_v(GLenum target, GLuint index, GLint64* data) { fp_glGetInteger64i_v(target, index, data); }
inline void glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64* params) { fp_glGetBufferParameteri64v(target, pname, params); }
inline void glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level) { fp_glFramebufferTexture(target, attachment, texture, level); }
inline void glTexImage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) { fp_glTexImage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations); }
inline void glTexImage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) { fp_glTexImage3DMultisample(target, samples, internalformat, width, height, depth, fixedsamplelocations); }
inline void glGetMultisamplefv(GLenum pname, GLuint index, GLfloat* val) { fp_glGetMultisamplefv(pname, index, val); }
inline void glSampleMaski(GLuint maskNumber, GLbitfield mask) { fp_glSampleMaski(maskNumber, mask); }

/* GL_VERSION_3_3 */
inline void glBindFragDataLocationIndexed(GLuint program, GLuint colorNumber, GLuint index, const GLchar* name) { fp_glBindFragDataLocationIndexed(program, colorNumber, index, name); }
inline GLint glGetFragDataIndex(GLuint program, const GLchar* name) { return fp_glGetFragDataIndex(program, name); }
inline void glGenSamplers(GLsizei count, GLuint* samplers) { fp_glGenSamplers(count, samplers); }
inline void glDeleteSamplers(GLsizei count, const GLuint* samplers) { fp_glDeleteSamplers(count, samplers); }
inline GLboolean glIsSampler(GLuint sampler) { return fp_glIsSampler(sampler); }
inline void glBindSampler(GLuint unit, GLuint sampler) { fp_glBindSampler(unit, sampler); }
inline void glSamplerParameteri(GLuint sampler, GLenum pname, GLint param) { fp_glSamplerParameteri(sampler, pname, param); }
inline void glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint* param) { fp_glSamplerParameteriv(sampler, pname, param); }
inline void glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param) { fp_glSamplerParameterf(sampler, pname, param); }
inline void glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* param) { fp_glSamplerParameterfv(sampler, pname, param); }
inline void glSamplerParameterIiv(GLuint sampler, GLenum pname, const GLint* param) { fp_glSamplerParameterIiv(sampler, pname, param); }
inline void glSamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint* param) { fp_glSamplerParameterIuiv(sampler, pname, param); }
inline void glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint* params) { fp_glGetSamplerParameteriv(sampler, pname, params); }
inline void glGetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint* params) { fp_glGetSamplerParameterIiv(sampler, pname, params); }
inline void glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat* params) { fp_glGetSamplerParameterfv(sampler, pname, params); }
inline void glGetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint* params) { fp_glGetSamplerParameterIuiv(sampler, pname, params); }
inline void glQueryCounter(GLuint id, GLenum target) { fp_glQueryCounter(id, target); }
inline void glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64* params) { fp_glGetQueryObjecti64v(id, pname, params); }
inline void glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64* params) { fp_glGetQueryObjectui64v(id, pname, params); }
inline void glVertexAttribDivisor(GLuint index, GLuint divisor) { fp_glVertexAttribDivisor(index, divisor); }
inline void glVertexAttribP1ui(GLuint index, GLenum type, GLboolean normalized, GLuint value) { fp_glVertexAttribP1ui(index, type, normalized, value); }
inline void glVertexAttribP1uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value) { fp_glVertexAttribP1uiv(index, type, normalized, value); }
inline void glVertexAttribP2ui(GLuint index, GLenum type, GLboolean normalized, GLuint value) { fp_glVertexAttribP2ui(index, type, normalized, value); }
inline void glVertexAttribP2uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value) { fp_glVertexAttribP2uiv(index, type, normalized, value); }
inline void glVertexAttribP3ui(GLuint index, GLenum type, GLboolean normalized, GLuint value) { fp_glVertexAttribP3ui(index, type, normalized, value); }
inline void glVertexAttribP3uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value) { fp_glVertexAttribP3uiv(index, type, normalized, value); }
inline void glVertexAttribP4ui(GLuint index, GLenum type, GLboolean normalized, GLuint value) { fp_glVertexAttribP4ui(index, type, normalized, value); }
inline void glVertexAttribP4uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value) { fp_glVertexAttribP4uiv(index, type, normalized, value); }
inline void glVertexP2ui(GLenum type, GLuint value) { fp_glVertexP2ui(type, value); }
inline void glVertexP2uiv(GLenum type, const GLuint* value) { fp_glVertexP2uiv(type, value); }
inline void glVertexP3ui(GLenum type, GLuint value) { fp_glVertexP3ui(type, value); }
inline void glVertexP3uiv(GLenum type, const GLuint* value) { fp_glVertexP3uiv(type, value); }
inline void glVertexP4ui(GLenum type, GLuint value) { fp_glVertexP4ui(type, value); }
inline void glVertexP4uiv(GLenum type, const GLuint* value) { fp_glVertexP4uiv(type, value); }
inline void glTexCoordP1ui(GLenum type, GLuint coords) { fp_glTexCoordP1ui(type, coords); }
inline void glTexCoordP1uiv(GLenum type, const GLuint* coords) { fp_glTexCoordP1uiv(type, coords); }
inline void glTexCoordP2ui(GLenum type, GLuint coords) { fp_glTexCoordP2ui(type, coords); }
inline void glTexCoordP2uiv(GLenum type, const GLuint* coords) { fp_glTexCoordP2uiv(type, coords); }
inline void glTexCoordP3ui(GLenum type, GLuint coords) { fp_glTexCoordP3ui(type, coords); }
inline void glTexCoordP3uiv(GLenum type, const GLuint* coords) { fp_glTexCoordP3uiv(type, coords); }
inline void glTexCoordP4ui(GLenum type, GLuint coords) { fp_glTexCoordP4ui(type, coords); }
inline void glTexCoordP4uiv(GLenum type, const GLuint* coords) { fp_glTexCoordP4uiv(type, coords); }
inline void glMultiTexCoordP1ui(GLenum texture, GLenum type, GLuint coords) { fp_glMultiTexCoordP1ui(texture, type, coords); }
inline void glMultiTexCoordP1uiv(GLenum texture, GLenum type, const GLuint* coords) { fp_glMultiTexCoordP1uiv(texture, type, coords); }
inline void glMultiTexCoordP2ui(GLenum texture, GLenum type, GLuint coords) { fp_glMultiTexCoordP2ui(texture, type, coords); }
inline void glMultiTexCoordP2uiv(GLenum texture, GLenum type, const GLuint* coords) { fp_glMultiTexCoordP2uiv(texture, type, coords); }
inline void glMultiTexCoordP3ui(GLenum texture, GLenum type, GLuint coords) { fp_glMultiTexCoordP3ui(texture, type, coords); }
inline void glMultiTexCoordP3uiv(GLenum texture, GLenum type, const GLuint* coords) { fp_glMultiTexCoordP3uiv(texture, type, coords); }
inline void glMultiTexCoordP4ui(GLenum texture, GLenum type, GLuint coords) { fp_glMultiTexCoordP4ui(texture, type, coords); }
inline void glMultiTexCoordP4uiv(GLenum texture, GLenum type, const GLuint* coords) { fp_glMultiTexCoordP4uiv(texture, type, coords); }
inline void glNormalP3ui(GLenum type, GLuint coords) { fp_glNormalP3ui(type, coords); }
inline void glNormalP3uiv(GLenum type, const GLuint* coords) { fp_glNormalP3uiv(type, coords); }
inline void glColorP3ui(GLenum type, GLuint color) { fp_glColorP3ui(type, color); }
inline void glColorP3uiv(GLenum type, const GLuint* color) { fp_glColorP3uiv(type, color); }
inline void glColorP4ui(GLenum type, GLuint color) { fp_glColorP4ui(type, color); }
inline void glColorP4uiv(GLenum type, const GLuint* color) { fp_glColorP4uiv(type, color); }
inline void glSecondaryColorP3ui(GLenum type, GLuint color) { fp_glSecondaryColorP3ui(type, color); }
inline void glSecondaryColorP3uiv(GLenum type, const GLuint* color) { fp_glSecondaryColorP3uiv(type, color); }

/* GL_VERSION_4_0 */
inline void glMinSampleShading(GLfloat value) { fp_glMinSampleShading(value); }
inline void glBlendEquationi(GLuint buf, GLenum mode) { fp_glBlendEquationi(buf, mode); }
inline void glBlendEquationSeparatei(GLuint buf, GLenum modeRGB, GLenum modeAlpha) { fp_glBlendEquationSeparatei(buf, modeRGB, modeAlpha); }
inline void glBlendFunci(GLuint buf, GLenum src, GLenum dst) { fp_glBlendFunci(buf, src, dst); }
inline void glBlendFuncSeparatei(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) { fp_glBlendFuncSeparatei(buf, srcRGB, dstRGB, srcAlpha, dstAlpha); }
inline void glDrawArraysIndirect(GLenum mode, const void* indirect) { fp_glDrawArraysIndirect(mode, indirect); }
inline void glDrawElementsIndirect(GLenum mode, GLenum type, const void* indirect) { fp_glDrawElementsIndirect(mode, type, indirect); }
inline void glUniform1d(GLint location, GLdouble x) { fp_glUniform1d(location, x); }
inline void glUniform2d(GLint location, GLdouble x, GLdouble y) { fp_glUniform2d(location, x, y); }
inline void glUniform3d(GLint location, GLdouble x, GLdouble y, GLdouble z) { fp_glUniform3d(location, x, y, z); }
inline void glUniform4d(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w) { fp_glUniform4d(location, x, y, z, w); }
inline void glUniform1dv(GLint location, GLsizei count, const GLdouble* value) { fp_glUniform1dv(location, count, value); }
inline void glUniform2dv(GLint location, GLsizei count, const GLdouble* value) { fp_glUniform2dv(location, count, value); }
inline void glUniform3dv(GLint location, GLsizei count, const GLdouble* value) { fp_glUniform3dv(location, count, value); }
inline void glUniform4dv(GLint location, GLsizei count, const GLdouble* value) { fp_glUniform4dv(location, count, value); }
inline void glUniformMatrix2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glUniformMatrix2dv(location, count, transpose, value); }
inline void glUniformMatrix3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glUniformMatrix3dv(location, count, transpose, value); }
inline void glUniformMatrix4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glUniformMatrix4dv(location, count, transpose, value); }
inline void glUniformMatrix2x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glUniformMatrix2x3dv(location, count, transpose, value); }
inline void glUniformMatrix2x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glUniformMatrix2x4dv(location, count, transpose, value); }
inline void glUniformMatrix3x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glUniformMatrix3x2dv(location, count, transpose, value); }
inline void glUniformMatrix3x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glUniformMatrix3x4dv(location, count, transpose, value); }
inline void glUniformMatrix4x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glUniformMatrix4x2dv(location, count, transpose, value); }
inline void glUniformMatrix4x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glUniformMatrix4x3dv(location, count, transpose, value); }
inline void glGetUniformdv(GLuint program, GLint location, GLdouble* params) { fp_glGetUniformdv(program, location, params); }
inline GLint glGetSubroutineUniformLocation(GLuint program, GLenum shadertype, const GLchar* name) { return fp_glGetSubroutineUniformLocation(program, shadertype, name); }
inline GLuint glGetSubroutineIndex(GLuint program, GLenum shadertype, const GLchar* name) { return fp_glGetSubroutineIndex(program, shadertype, name); }
inline void glGetActiveSubroutineUniformiv(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint* values) { fp_glGetActiveSubroutineUniformiv(program, shadertype, index, pname, values); }
inline void glGetActiveSubroutineUniformName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei* length, GLchar* name) { fp_glGetActiveSubroutineUniformName(program, shadertype, index, bufsize, length, name); }
inline void glGetActiveSubroutineName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei* length, GLchar* name) { fp_glGetActiveSubroutineName(program, shadertype, index, bufsize, length, name); }
inline void glUniformSubroutinesuiv(GLenum shadertype, GLsizei count, const GLuint* indices) { fp_glUniformSubroutinesuiv(shadertype, count, indices); }
inline void glGetUniformSubroutineuiv(GLenum shadertype, GLint location, GLuint* params) { fp_glGetUniformSubroutineuiv(shadertype, location, params); }
inline void glGetProgramStageiv(GLuint program, GLenum shadertype, GLenum pname, GLint* values) { fp_glGetProgramStageiv(program, shadertype, pname, values); }
inline void glPatchParameteri(GLenum pname, GLint value) { fp_glPatchParameteri(pname, value); }
inline void glPatchParameterfv(GLenum pname, const GLfloat* values) { fp_glPatchParameterfv(pname, values); }
inline void glBindTransformFeedback(GLenum target, GLuint id) { fp_glBindTransformFeedback(target, id); }
inline void glDeleteTransformFeedbacks(GLsizei n, const GLuint* ids) { fp_glDeleteTransformFeedbacks(n, ids); }
inline void glGenTransformFeedbacks(GLsizei n, GLuint* ids) { fp_glGenTransformFeedbacks(n, ids); }
inline GLboolean glIsTransformFeedback(GLuint id) { return fp_glIsTransformFeedback(id); }
inline void glPauseTransformFeedback() { fp_glPauseTransformFeedback(); }
inline void glResumeTransformFeedback() { fp_glResumeTransformFeedback(); }
inline void glDrawTransformFeedback(GLenum mode, GLuint id) { fp_glDrawTransformFeedback(mode, id); }
inline void glDrawTransformFeedbackStream(GLenum mode, GLuint id, GLuint stream) { fp_glDrawTransformFeedbackStream(mode, id, stream); }
inline void glBeginQueryIndexed(GLenum target, GLuint index, GLuint id) { fp_glBeginQueryIndexed(target, index, id); }
inline void glEndQueryIndexed(GLenum target, GLuint index) { fp_glEndQueryIndexed(target, index); }
inline void glGetQueryIndexediv(GLenum target, GLuint index, GLenum pname, GLint* params) { fp_glGetQueryIndexediv(target, index, pname, params); }

/* GL_VERSION_4_1 */
inline void glReleaseShaderCompiler() { fp_glReleaseShaderCompiler(); }
inline void glShaderBinary(GLsizei count, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length) { fp_glShaderBinary(count, shaders, binaryformat, binary, length); }
inline void glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision) { fp_glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision); }
inline void glDepthRangef(GLfloat n, GLfloat f) { fp_glDepthRangef(n, f); }
inline void glClearDepthf(GLfloat d) { fp_glClearDepthf(d); }
inline void glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, void* binary) { fp_glGetProgramBinary(program, bufSize, length, binaryFormat, binary); }
inline void glProgramBinary(GLuint program, GLenum binaryFormat, const void* binary, GLsizei length) { fp_glProgramBinary(program, binaryFormat, binary, length); }
inline void glProgramParameteri(GLuint program, GLenum pname, GLint value) { fp_glProgramParameteri(program, pname, value); }
inline void glUseProgramStages(GLuint pipeline, GLbitfield stages, GLuint program) { fp_glUseProgramStages(pipeline, stages, program); }
inline void glActiveShaderProgram(GLuint pipeline, GLuint program) { fp_glActiveShaderProgram(pipeline, program); }
inline GLuint glCreateShaderProgramv(GLenum type, GLsizei count, const GLchar** strings) { return fp_glCreateShaderProgramv(type, count, strings); }
inline void glBindProgramPipeline(GLuint pipeline) { fp_glBindProgramPipeline(pipeline); }
inline void glDeleteProgramPipelines(GLsizei n, const GLuint* pipelines) { fp_glDeleteProgramPipelines(n, pipelines); }
inline void glGenProgramPipelines(GLsizei n, GLuint* pipelines) { fp_glGenProgramPipelines(n, pipelines); }
inline GLboolean glIsProgramPipeline(GLuint pipeline) { return fp_glIsProgramPipeline(pipeline); }
inline void glGetProgramPipelineiv(GLuint pipeline, GLenum pname, GLint* params) { fp_glGetProgramPipelineiv(pipeline, pname, params); }
inline void glProgramUniform1i(GLuint program, GLint location, GLint v0) { fp_glProgramUniform1i(program, location, v0); }
inline void glProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint* value) { fp_glProgramUniform1iv(program, location, count, value); }
inline void glProgramUniform1f(GLuint program, GLint location, GLfloat v0) { fp_glProgramUniform1f(program, location, v0); }
inline void glProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat* value) { fp_glProgramUniform1fv(program, location, count, value); }
inline void glProgramUniform1d(GLuint program, GLint location, GLdouble v0) { fp_glProgramUniform1d(program, location, v0); }
inline void glProgramUniform1dv(GLuint program, GLint location, GLsizei count, const GLdouble* value) { fp_glProgramUniform1dv(program, location, count, value); }
inline void glProgramUniform1ui(GLuint program, GLint location, GLuint v0) { fp_glProgramUniform1ui(program, location, v0); }
inline void glProgramUniform1uiv(GLuint program, GLint location, GLsizei count, const GLuint* value) { fp_glProgramUniform1uiv(program, location, count, value); }
inline void glProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1) { fp_glProgramUniform2i(program, location, v0, v1); }
inline void glProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint* value) { fp_glProgramUniform2iv(program, location, count, value); }
inline void glProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1) { fp_glProgramUniform2f(program, location, v0, v1); }
inline void glProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat* value) { fp_glProgramUniform2fv(program, location, count, value); }
inline void glProgramUniform2d(GLuint program, GLint location, GLdouble v0, GLdouble v1) { fp_glProgramUniform2d(program, location, v0, v1); }
inline void glProgramUniform2dv(GLuint program, GLint location, GLsizei count, const GLdouble* value) { fp_glProgramUniform2dv(program, location, count, value); }
inline void glProgramUniform2ui(GLuint program, GLint location, GLuint v0, GLuint v1) { fp_glProgramUniform2ui(program, location, v0, v1); }
inline void glProgramUniform2uiv(GLuint program, GLint location, GLsizei count, const GLuint* value) { fp_glProgramUniform2uiv(program, location, count, value); }
inline void glProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2) { fp_glProgramUniform3i(program, location, v0, v1, v2); }
inline void glProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint* value) { fp_glProgramUniform3iv(program, location, count, value); }
inline void glProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) { fp_glProgramUniform3f(program, location, v0, v1, v2); }
inline void glProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat* value) { fp_glProgramUniform3fv(program, location, count, value); }
inline void glProgramUniform3d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2) { fp_glProgramUniform3d(program, location, v0, v1, v2); }
inline void glProgramUniform3dv(GLuint program, GLint location, GLsizei count, const GLdouble* value) { fp_glProgramUniform3dv(program, location, count, value); }
inline void glProgramUniform3ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2) { fp_glProgramUniform3ui(program, location, v0, v1, v2); }
inline void glProgramUniform3uiv(GLuint program, GLint location, GLsizei count, const GLuint* value) { fp_glProgramUniform3uiv(program, location, count, value); }
inline void glProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) { fp_glProgramUniform4i(program, location, v0, v1, v2, v3); }
inline void glProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint* value) { fp_glProgramUniform4iv(program, location, count, value); }
inline void glProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) { fp_glProgramUniform4f(program, location, v0, v1, v2, v3); }
inline void glProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat* value) { fp_glProgramUniform4fv(program, location, count, value); }
inline void glProgramUniform4d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3) { fp_glProgramUniform4d(program, location, v0, v1, v2, v3); }
inline void glProgramUniform4dv(GLuint program, GLint location, GLsizei count, const GLdouble* value) { fp_glProgramUniform4dv(program, location, count, value); }
inline void glProgramUniform4ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) { fp_glProgramUniform4ui(program, location, v0, v1, v2, v3); }
inline void glProgramUniform4uiv(GLuint program, GLint location, GLsizei count, const GLuint* value) { fp_glProgramUniform4uiv(program, location, count, value); }
inline void glProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix2fv(program, location, count, transpose, value); }
inline void glProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix3fv(program, location, count, transpose, value); }
inline void glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix4fv(program, location, count, transpose, value); }
inline void glProgramUniformMatrix2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix2dv(program, location, count, transpose, value); }
inline void glProgramUniformMatrix3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix3dv(program, location, count, transpose, value); }
inline void glProgramUniformMatrix4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix4dv(program, location, count, transpose, value); }
inline void glProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix2x3fv(program, location, count, transpose, value); }
inline void glProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix3x2fv(program, location, count, transpose, value); }
inline void glProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix2x4fv(program, location, count, transpose, value); }
inline void glProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix4x2fv(program, location, count, transpose, value); }
inline void glProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix3x4fv(program, location, count, transpose, value); }
inline void glProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix4x3fv(program, location, count, transpose, value); }
inline void glProgramUniformMatrix2x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix2x3dv(program, location, count, transpose, value); }
inline void glProgramUniformMatrix3x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix3x2dv(program, location, count, transpose, value); }
inline void glProgramUniformMatrix2x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix2x4dv(program, location, count, transpose, value); }
inline void glProgramUniformMatrix4x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix4x2dv(program, location, count, transpose, value); }
inline void glProgramUniformMatrix3x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix3x4dv(program, location, count, transpose, value); }
inline void glProgramUniformMatrix4x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix4x3dv(program, location, count, transpose, value); }
inline void glValidateProgramPipeline(GLuint pipeline) { fp_glValidateProgramPipeline(pipeline); }
inline void glGetProgramPipelineInfoLog(GLuint pipeline, GLsizei bufSize, GLsizei* length, GLchar* infoLog) { fp_glGetProgramPipelineInfoLog(pipeline, bufSize, length, infoLog); }
inline void glVertexAttribL1d(GLuint index, GLdouble x) { fp_glVertexAttribL1d(index, x); }
inline void glVertexAttribL2d(GLuint index, GLdouble x, GLdouble y) { fp_glVertexAttribL2d(index, x, y); }
inline void glVertexAttribL3d(GLuint index, GLdouble x, GLdouble y, GLdouble z) { fp_glVertexAttribL3d(index, x, y, z); }
inline void glVertexAttribL4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) { fp_glVertexAttribL4d(index, x, y, z, w); }
inline void glVertexAttribL1dv(GLuint index, const GLdouble* v) { fp_glVertexAttribL1dv(index, v); }
inline void glVertexAttribL2dv(GLuint index, const GLdouble* v) { fp_glVertexAttribL2dv(index, v); }
inline void glVertexAttribL3dv(GLuint index, const GLdouble* v) { fp_glVertexAttribL3dv(index, v); }
inline void glVertexAttribL4dv(GLuint index, const GLdouble* v) { fp_glVertexAttribL4dv(index, v); }
inline void glVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) { fp_glVertexAttribLPointer(index, size, type, stride, pointer); }
inline void glGetVertexAttribLdv(GLuint index, GLenum pname, GLdouble* params) { fp_glGetVertexAttribLdv(index, pname, params); }
inline void glViewportArrayv(GLuint first, GLsizei count, const GLfloat* v) { fp_glViewportArrayv(first, count, v); }
inline void glViewportIndexedf(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h) { fp_glViewportIndexedf(index, x, y, w, h); }
inline void glViewportIndexedfv(GLuint index, const GLfloat* v) { fp_glViewportIndexedfv(index, v); }
inline void glScissorArrayv(GLuint first, GLsizei count, const GLint* v) { fp_glScissorArrayv(first, count, v); }
inline void glScissorIndexed(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height) { fp_glScissorIndexed(index, left, bottom, width, height); }
inline void glScissorIndexedv(GLuint index, const GLint* v) { fp_glScissorIndexedv(index, v); }
inline void glDepthRangeArrayv(GLuint first, GLsizei count, const GLdouble* v) { fp_glDepthRangeArrayv(first, count, v); }
inline void glDepthRangeIndexed(GLuint index, GLdouble n, GLdouble f) { fp_glDepthRangeIndexed(index, n, f); }
inline void glGetFloati_v(GLenum target, GLuint index, GLfloat* data) { fp_glGetFloati_v(target, index, data); }
inline void glGetDoublei_v(GLenum target, GLuint index, GLdouble* data) { fp_glGetDoublei_v(target, index, data); }

/* GL_VERSION_4_2 */
inline void glDrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance) { fp_glDrawArraysInstancedBaseInstance(mode, first, count, instancecount, baseinstance); }
inline void glDrawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLuint baseinstance) { fp_glDrawElementsInstancedBaseInstance(mode, count, type, indices, instancecount, baseinstance); }
inline void glDrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance) { fp_glDrawElementsInstancedBaseVertexBaseInstance(mode, count, type, indices, instancecount, basevertex, baseinstance); }
inline void glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params) { fp_glGetInternalformativ(target, internalformat, pname, bufSize, params); }
inline void glGetActiveAtomicCounterBufferiv(GLuint program, GLuint bufferIndex, GLenum pname, GLint* params) { fp_glGetActiveAtomicCounterBufferiv(program, bufferIndex, pname, params); }
inline void glBindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format) { fp_glBindImageTexture(unit, texture, level, layered, layer, access, format); }
inline void glMemoryBarrier(GLbitfield barriers) { fp_glMemoryBarrier(barriers); }
inline void glTexStorage1D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width) { fp_glTexStorage1D(target, levels, internalformat, width); }
inline void glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) { fp_glTexStorage2D(target, levels, internalformat, width, height); }
inline void glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) { fp_glTexStorage3D(target, levels, internalformat, width, height, depth); }
inline void glDrawTransformFeedbackInstanced(GLenum mode, GLuint id, GLsizei instancecount) { fp_glDrawTransformFeedbackInstanced(mode, id, instancecount); }
inline void glDrawTransformFeedbackStreamInstanced(GLenum mode, GLuint id, GLuint stream, GLsizei instancecount) { fp_glDrawTransformFeedbackStreamInstanced(mode, id, stream, instancecount); }

/* GL_VERSION_4_3 */
inline void glClearBufferData(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void* data) { fp_glClearBufferData(target, internalformat, format, type, data); }
inline void glClearBufferSubData(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data) { fp_glClearBufferSubData(target, internalformat, offset, size, format, type, data); }
inline void glDispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z) { fp_glDispatchCompute(num_groups_x, num_groups_y, num_groups_z); }
inline void glDispatchComputeIndirect(GLintptr indirect) { fp_glDispatchComputeIndirect(indirect); }
inline void glCopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth) { fp_glCopyImageSubData(srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth); }
inline void glFramebufferParameteri(GLenum target, GLenum pname, GLint param) { fp_glFramebufferParameteri(target, pname, param); }
inline void glGetFramebufferParameteriv(GLenum target, GLenum pname, GLint* params) { fp_glGetFramebufferParameteriv(target, pname, params); }
inline void glGetInternalformati64v(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64* params) { fp_glGetInternalformati64v(target, internalformat, pname, bufSize, params); }
inline void glInvalidateTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth) { fp_glInvalidateTexSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth); }
inline void glInvalidateTexImage(GLuint texture, GLint level) { fp_glInvalidateTexImage(texture, level); }
inline void glInvalidateBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr length) { fp_glInvalidateBufferSubData(buffer, offset, length); }
inline void glInvalidateBufferData(GLuint buffer) { fp_glInvalidateBufferData(buffer); }
inline void glInvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments) { fp_glInvalidateFramebuffer(target, numAttachments, attachments); }
inline void glInvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height) { fp_glInvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height); }
inline void glMultiDrawArraysIndirect(GLenum mode, const void* indirect, GLsizei drawcount, GLsizei stride) { fp_glMultiDrawArraysIndirect(mode, indirect, drawcount, stride); }
inline void glMultiDrawElementsIndirect(GLenum mode, GLenum type, const void* indirect, GLsizei drawcount, GLsizei stride) { fp_glMultiDrawElementsIndirect(mode, type, indirect, drawcount, stride); }
inline void glGetProgramInterfaceiv(GLuint program, GLenum programInterface, GLenum pname, GLint* params) { fp_glGetProgramInterfaceiv(program, programInterface, pname, params); }
inline GLuint glGetProgramResourceIndex(GLuint program, GLenum programInterface, const GLchar* name) { return fp_glGetProgramResourceIndex(program, programInterface, name); }
inline void glGetProgramResourceName(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name) { fp_glGetProgramResourceName(program, programInterface, index, bufSize, length, name); }
inline void glGetProgramResourceiv(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei bufSize, GLsizei* length, GLint* params) { fp_glGetProgramResourceiv(program, programInterface, index, propCount, props, bufSize, length, params); }
inline GLint glGetProgramResourceLocation(GLuint program, GLenum programInterface, const GLchar* name) { return fp_glGetProgramResourceLocation(program, programInterface, name); }
inline GLint glGetProgramResourceLocationIndex(GLuint program, GLenum programInterface, const GLchar* name) { return fp_glGetProgramResourceLocationIndex(program, programInterface, name); }
inline void glShaderStorageBlockBinding(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding) { fp_glShaderStorageBlockBinding(program, storageBlockIndex, storageBlockBinding); }
inline void glTexBufferRange(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size) { fp_glTexBufferRange(target, internalformat, buffer, offset, size); }
inline void glTexStorage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) { fp_glTexStorage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations); }
inline void glTexStorage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) { fp_glTexStorage3DMultisample(target, samples, internalformat, width, height, depth, fixedsamplelocations); }
inline void glTextureView(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers) { fp_glTextureView(texture, target, origtexture, internalformat, minlevel, numlevels, minlayer, numlayers); }
inline void glBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) { fp_glBindVertexBuffer(bindingindex, buffer, offset, stride); }
inline void glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset) { fp_glVertexAttribFormat(attribindex, size, type, normalized, relativeoffset); }
inline void glVertexAttribIFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) { fp_glVertexAttribIFormat(attribindex, size, type, relativeoffset); }
inline void glVertexAttribLFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) { fp_glVertexAttribLFormat(attribindex, size, type, relativeoffset); }
inline void glVertexAttribBinding(GLuint attribindex, GLuint bindingindex) { fp_glVertexAttribBinding(attribindex, bindingindex); }
inline void glVertexBindingDivisor(GLuint bindingindex, GLuint divisor) { fp_glVertexBindingDivisor(bindingindex, divisor); }
inline void glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled) { fp_glDebugMessageControl(source, type, severity, count, ids, enabled); }
inline void glDebugMessageInsert(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf) { fp_glDebugMessageInsert(source, type, id, severity, length, buf); }
inline void glDebugMessageCallback(GLDEBUGPROC callback, const void* userParam) { fp_glDebugMessageCallback(callback, userParam); }
inline GLuint glGetDebugMessageLog(GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog) { return fp_glGetDebugMessageLog(count, bufSize, sources, types, ids, severities, lengths, messageLog); }
inline void glPushDebugGroup(GLenum source, GLuint id, GLsizei length, const GLchar* message) { fp_glPushDebugGroup(source, id, length, message); }
inline void glPopDebugGroup() { fp_glPopDebugGroup(); }
inline void glObjectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar* label) { fp_glObjectLabel(identifier, name, length, label); }
inline void glGetObjectLabel(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei* length, GLchar* label) { fp_glGetObjectLabel(identifier, name, bufSize, length, label); }
inline void glObjectPtrLabel(const void* ptr, GLsizei length, const GLchar* label) { fp_glObjectPtrLabel(ptr, length, label); }
inline void glGetObjectPtrLabel(const void* ptr, GLsizei bufSize, GLsizei* length, GLchar* label) { fp_glGetObjectPtrLabel(ptr, bufSize, length, label); }

/* GL_VERSION_4_4 */
inline void glBufferStorage(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags) { fp_glBufferStorage(target, size, data, flags); }
inline void glClearTexImage(GLuint texture, GLint level, GLenum format, GLenum type, const void* data) { fp_glClearTexImage(texture, level, format, type, data); }
inline void glClearTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* data) { fp_glClearTexSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, data); }
inline void glBindBuffersBase(GLenum target, GLuint first, GLsizei count, const GLuint* buffers) { fp_glBindBuffersBase(target, first, count, buffers); }
inline void glBindBuffersRange(GLenum target, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizeiptr* sizes) { fp_glBindBuffersRange(target, first, count, buffers, offsets, sizes); }
inline void glBindTextures(GLuint first, GLsizei count, const GLuint* textures) { fp_glBindTextures(first, count, textures); }
inline void glBindSamplers(GLuint first, GLsizei count, const GLuint* samplers) { fp_glBindSamplers(first, count, samplers); }
inline void glBindImageTextures(GLuint first, GLsizei count, const GLuint* textures) { fp_glBindImageTextures(first, count, textures); }
inline void glBindVertexBuffers(GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides) { fp_glBindVertexBuffers(first, count, buffers, offsets, strides); }

/* GL_VERSION_4_5 */
inline void glClipControl(GLenum origin, GLenum depth) { fp_glClipControl(origin, depth); }
inline void glCreateTransformFeedbacks(GLsizei n, GLuint* ids) { fp_glCreateTransformFeedbacks(n, ids); }
inline void glTransformFeedbackBufferBase(GLuint xfb, GLuint index, GLuint buffer) { fp_glTransformFeedbackBufferBase(xfb, index, buffer); }
inline void glTransformFeedbackBufferRange(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) { fp_glTransformFeedbackBufferRange(xfb, index, buffer, offset, size); }
inline void glGetTransformFeedbackiv(GLuint xfb, GLenum pname, GLint* param) { fp_glGetTransformFeedbackiv(xfb, pname, param); }
inline void glGetTransformFeedbacki_v(GLuint xfb, GLenum pname, GLuint index, GLint* param) { fp_glGetTransformFeedbacki_v(xfb, pname, index, param); }
inline void glGetTransformFeedbacki64_v(GLuint xfb, GLenum pname, GLuint index, GLint64* param) { fp_glGetTransformFeedbacki64_v(xfb, pname, index, param); }
inline void glCreateBuffers(GLsizei n, GLuint* buffers) { fp_glCreateBuffers(n, buffers); }
inline void glNamedBufferStorage(GLuint buffer, GLsizeiptr size, const void* data, GLbitfield flags) { fp_glNamedBufferStorage(buffer, size, data, flags); }
inline void glNamedBufferData(GLuint buffer, GLsizeiptr size, const void* data, GLenum usage) { fp_glNamedBufferData(buffer, size, data, usage); }
inline void glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, const void* data) { fp_glNamedBufferSubData(buffer, offset, size, data); }
inline void glCopyNamedBufferSubData(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) { fp_glCopyNamedBufferSubData(readBuffer, writeBuffer, readOffset, writeOffset, size); }
inline void glClearNamedBufferData(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void* data) { fp_glClearNamedBufferData(buffer, internalformat, format, type, data); }
inline void glClearNamedBufferSubData(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data) { fp_glClearNamedBufferSubData(buffer, internalformat, offset, size, format, type, data); }
inline void* glMapNamedBuffer(GLuint buffer, GLenum access) { return fp_glMapNamedBuffer(buffer, access); }
inline void* glMapNamedBufferRange(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access) { return fp_glMapNamedBufferRange(buffer, offset, length, access); }
inline GLboolean glUnmapNamedBuffer(GLuint buffer) { return fp_glUnmapNamedBuffer(buffer); }
inline void glFlushMappedNamedBufferRange(GLuint buffer, GLintptr offset, GLsizeiptr length) { fp_glFlushMappedNamedBufferRange(buffer, offset, length); }
inline void glGetNamedBufferParameteriv(GLuint buffer, GLenum pname, GLint* params) { fp_glGetNamedBufferParameteriv(buffer, pname, params); }
inline void glGetNamedBufferParameteri64v(GLuint buffer, GLenum pname, GLint64* params) { fp_glGetNamedBufferParameteri64v(buffer, pname, params); }
inline void glGetNamedBufferPointerv(GLuint buffer, GLenum pname, void** params) { fp_glGetNamedBufferPointerv(buffer, pname, params); }
inline void glGetNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, void* data) { fp_glGetNamedBufferSubData(buffer, offset, size, data); }
inline void glCreateFramebuffers(GLsizei n, GLuint* framebuffers) { fp_glCreateFramebuffers(n, framebuffers); }
inline void glNamedFramebufferRenderbuffer(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) { fp_glNamedFramebufferRenderbuffer(framebuffer, attachment, renderbuffertarget, renderbuffer); }
inline void glNamedFramebufferParameteri(GLuint framebuffer, GLenum pname, GLint param) { fp_glNamedFramebufferParameteri(framebuffer, pname, param); }
inline void glNamedFramebufferTexture(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level) { fp_glNamedFramebufferTexture(framebuffer, attachment, texture, level); }
inline void glNamedFramebufferTextureLayer(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer) { fp_glNamedFramebufferTextureLayer(framebuffer, attachment, texture, level, layer); }
inline void glNamedFramebufferDrawBuffer(GLuint framebuffer, GLenum buf) { fp_glNamedFramebufferDrawBuffer(framebuffer, buf); }
inline void glNamedFramebufferDrawBuffers(GLuint framebuffer, GLsizei n, const GLenum* bufs) { fp_glNamedFramebufferDrawBuffers(framebuffer, n, bufs); }
inline void glNamedFramebufferReadBuffer(GLuint framebuffer, GLenum src) { fp_glNamedFramebufferReadBuffer(framebuffer, src); }
inline void glInvalidateNamedFramebufferData(GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments) { fp_glInvalidateNamedFramebufferData(framebuffer, numAttachments, attachments); }
inline void glInvalidateNamedFramebufferSubData(GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height) { fp_glInvalidateNamedFramebufferSubData(framebuffer, numAttachments, attachments, x, y, width, height); }
inline void glClearNamedFramebufferiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint* value) { fp_glClearNamedFramebufferiv(framebuffer, buffer, drawbuffer, value); }
inline void glClearNamedFramebufferuiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint* value) { fp_glClearNamedFramebufferuiv(framebuffer, buffer, drawbuffer, value); }
inline void glClearNamedFramebufferfv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat* value) { fp_glClearNamedFramebufferfv(framebuffer, buffer, drawbuffer, value); }
inline void glClearNamedFramebufferfi(GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) { fp_glClearNamedFramebufferfi(framebuffer, buffer, drawbuffer, depth, stencil); }
inline void glBlitNamedFramebuffer(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) { fp_glBlitNamedFramebuffer(readFramebuffer, drawFramebuffer, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter); }
inline GLenum glCheckNamedFramebufferStatus(GLuint framebuffer, GLenum target) { return fp_glCheckNamedFramebufferStatus(framebuffer, target); }
inline void glGetNamedFramebufferParameteriv(GLuint framebuffer, GLenum pname, GLint* param) { fp_glGetNamedFramebufferParameteriv(framebuffer, pname, param); }
inline void glGetNamedFramebufferAttachmentParameteriv(GLuint framebuffer, GLenum attachment, GLenum pname, GLint* params) { fp_glGetNamedFramebufferAttachmentParameteriv(framebuffer, attachment, pname, params); }
inline void glCreateRenderbuffers(GLsizei n, GLuint* renderbuffers) { fp_glCreateRenderbuffers(n, renderbuffers); }
inline void glNamedRenderbufferStorage(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height) { fp_glNamedRenderbufferStorage(renderbuffer, internalformat, width, height); }
inline void glNamedRenderbufferStorageMultisample(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) { fp_glNamedRenderbufferStorageMultisample(renderbuffer, samples, internalformat, width, height); }
inline void glGetNamedRenderbufferParameteriv(GLuint renderbuffer, GLenum pname, GLint* params) { fp_glGetNamedRenderbufferParameteriv(renderbuffer, pname, params); }
inline void glCreateTextures(GLenum target, GLsizei n, GLuint* textures) { fp_glCreateTextures(target, n, textures); }
inline void glTextureBuffer(GLuint texture, GLenum internalformat, GLuint buffer) { fp_glTextureBuffer(texture, internalformat, buffer); }
inline void glTextureBufferRange(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size) { fp_glTextureBufferRange(texture, internalformat, buffer, offset, size); }
inline void glTextureStorage1D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width) { fp_glTextureStorage1D(texture, levels, internalformat, width); }
inline void glTextureStorage2D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) { fp_glTextureStorage2D(texture, levels, internalformat, width, height); }
inline void glTextureStorage3D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) { fp_glTextureStorage3D(texture, levels, internalformat, width, height, depth); }
inline void glTextureStorage2DMultisample(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) { fp_glTextureStorage2DMultisample(texture, samples, internalformat, width, height, fixedsamplelocations); }
inline void glTextureStorage3DMultisample(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) { fp_glTextureStorage3DMultisample(texture, samples, internalformat, width, height, depth, fixedsamplelocations); }
inline void glTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels) { fp_glTextureSubImage1D(texture, level, xoffset, width, format, type, pixels); }
inline void glTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) { fp_glTextureSubImage2D(texture, level, xoffset, yoffset, width, height, format, type, pixels); }
inline void glTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) { fp_glTextureSubImage3D(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels); }
inline void glCompressedTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data) { fp_glCompressedTextureSubImage1D(texture, level, xoffset, width, format, imageSize, data); }
inline void glCompressedTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data) { fp_glCompressedTextureSubImage2D(texture, level, xoffset, yoffset, width, height, format, imageSize, data); }
inline void glCompressedTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data) { fp_glCompressedTextureSubImage3D(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data); }
inline void glCopyTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) { fp_glCopyTextureSubImage1D(texture, level, xoffset, x, y, width); }
inline void glCopyTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) { fp_glCopyTextureSubImage2D(texture, level, xoffset, yoffset, x, y, width, height); }
inline void glCopyTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) { fp_glCopyTextureSubImage3D(texture, level, xoffset, yoffset, zoffset, x, y, width, height); }
inline void glTextureParameterf(GLuint texture, GLenum pname, GLfloat param) { fp_glTextureParameterf(texture, pname, param); }
inline void glTextureParameterfv(GLuint texture, GLenum pname, const GLfloat* param) { fp_glTextureParameterfv(texture, pname, param); }
inline void glTextureParameteri(GLuint texture, GLenum pname, GLint param) { fp_glTextureParameteri(texture, pname, param); }
inline void glTextureParameterIiv(GLuint texture, GLenum pname, const GLint* params) { fp_glTextureParameterIiv(texture, pname, params); }
inline void glTextureParameterIuiv(GLuint texture, GLenum pname, const GLuint* params) { fp_glTextureParameterIuiv(texture, pname, params); }
inline void glTextureParameteriv(GLuint texture, GLenum pname, const GLint* param) { fp_glTextureParameteriv(texture, pname, param); }
inline void glGenerateTextureMipmap(GLuint texture) { fp_glGenerateTextureMipmap(texture); }
inline void glBindTextureUnit(GLuint unit, GLuint texture) { fp_glBindTextureUnit(unit, texture); }
inline void glGetTextureImage(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* pixels) { fp_glGetTextureImage(texture, level, format, type, bufSize, pixels); }
inline void glGetCompressedTextureImage(GLuint texture, GLint level, GLsizei bufSize, void* pixels) { fp_glGetCompressedTextureImage(texture, level, bufSize, pixels); }
inline void glGetTextureLevelParameterfv(GLuint texture, GLint level, GLenum pname, GLfloat* params) { fp_glGetTextureLevelParameterfv(texture, level, pname, params); }
inline void glGetTextureLevelParameteriv(GLuint texture, GLint level, GLenum pname, GLint* params) { fp_glGetTextureLevelParameteriv(texture, level, pname, params); }
inline void glGetTextureParameterfv(GLuint texture, GLenum pname, GLfloat* params) { fp_glGetTextureParameterfv(texture, pname, params); }
inline void glGetTextureParameterIiv(GLuint texture, GLenum pname, GLint* params) { fp_glGetTextureParameterIiv(texture, pname, params); }
inline void glGetTextureParameterIuiv(GLuint texture, GLenum pname, GLuint* params) { fp_glGetTextureParameterIuiv(texture, pname, params); }
inline void glGetTextureParameteriv(GLuint texture, GLenum pname, GLint* params) { fp_glGetTextureParameteriv(texture, pname, params); }
inline void glCreateVertexArrays(GLsizei n, GLuint* arrays) { fp_glCreateVertexArrays(n, arrays); }
inline void glDisableVertexArrayAttrib(GLuint vaobj, GLuint index) { fp_glDisableVertexArrayAttrib(vaobj, index); }
inline void glEnableVertexArrayAttrib(GLuint vaobj, GLuint index) { fp_glEnableVertexArrayAttrib(vaobj, index); }
inline void glVertexArrayElementBuffer(GLuint vaobj, GLuint buffer) { fp_glVertexArrayElementBuffer(vaobj, buffer); }
inline void glVertexArrayVertexBuffer(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) { fp_glVertexArrayVertexBuffer(vaobj, bindingindex, buffer, offset, stride); }
inline void glVertexArrayVertexBuffers(GLuint vaobj, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides) { fp_glVertexArrayVertexBuffers(vaobj, first, count, buffers, offsets, strides); }
inline void glVertexArrayAttribBinding(GLuint vaobj, GLuint attribindex, GLuint bindingindex) { fp_glVertexArrayAttribBinding(vaobj, attribindex, bindingindex); }
inline void glVertexArrayAttribFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset) { fp_glVertexArrayAttribFormat(vaobj, attribindex, size, type, normalized, relativeoffset); }
inline void glVertexArrayAttribIFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) { fp_glVertexArrayAttribIFormat(vaobj, attribindex, size, type, relativeoffset); }
inline void glVertexArrayAttribLFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) { fp_glVertexArrayAttribLFormat(vaobj, attribindex, size, type, relativeoffset); }
inline void glVertexArrayBindingDivisor(GLuint vaobj, GLuint bindingindex, GLuint divisor) { fp_glVertexArrayBindingDivisor(vaobj, bindingindex, divisor); }
inline void glGetVertexArrayiv(GLuint vaobj, GLenum pname, GLint* param) { fp_glGetVertexArrayiv(vaobj, pname, param); }
inline void glGetVertexArrayIndexediv(GLuint vaobj, GLuint index, GLenum pname, GLint* param) { fp_glGetVertexArrayIndexediv(vaobj, index, pname, param); }
inline void glGetVertexArrayIndexed64iv(GLuint vaobj, GLuint index, GLenum pname, GLint64* param) { fp_glGetVertexArrayIndexed64iv(vaobj, index, pname, param); }
inline void glCreateSamplers(GLsizei n, GLuint* samplers) { fp_glCreateSamplers(n, samplers); }
inline void glCreateProgramPipelines(GLsizei n, GLuint* pipelines) { fp_glCreateProgramPipelines(n, pipelines); }
inline void glCreateQueries(GLenum target, GLsizei n, GLuint* ids) { fp_glCreateQueries(target, n, ids); }
inline void glGetQueryBufferObjecti64v(GLuint id, GLuint buffer, GLenum pname, GLintptr offset) { fp_glGetQueryBufferObjecti64v(id, buffer, pname, offset); }
inline void glGetQueryBufferObjectiv(GLuint id, GLuint buffer, GLenum pname, GLintptr offset) { fp_glGetQueryBufferObjectiv(id, buffer, pname, offset); }
inline void glGetQueryBufferObjectui64v(GLuint id, GLuint buffer, GLenum pname, GLintptr offset) { fp_glGetQueryBufferObjectui64v(id, buffer, pname, offset); }
inline void glGetQueryBufferObjectuiv(GLuint id, GLuint buffer, GLenum pname, GLintptr offset) { fp_glGetQueryBufferObjectuiv(id, buffer, pname, offset); }
inline void glMemoryBarrierByRegion(GLbitfield barriers) { fp_glMemoryBarrierByRegion(barriers); }
inline void glGetTextureSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void* pixels) { fp_glGetTextureSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, bufSize, pixels); }
inline void glGetCompressedTextureSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void* pixels) { fp_glGetCompressedTextureSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth, bufSize, pixels); }
inline GLenum glGetGraphicsResetStatus() { return fp_glGetGraphicsResetStatus(); }
inline void glGetnCompressedTexImage(GLenum target, GLint lod, GLsizei bufSize, void* pixels) { fp_glGetnCompressedTexImage(target, lod, bufSize, pixels); }
inline void glGetnTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* pixels) { fp_glGetnTexImage(target, level, format, type, bufSize, pixels); }
inline void glGetnUniformdv(GLuint program, GLint location, GLsizei bufSize, GLdouble* params) { fp_glGetnUniformdv(program, location, bufSize, params); }
inline void glGetnUniformfv(GLuint program, GLint location, GLsizei bufSize, GLfloat* params) { fp_glGetnUniformfv(program, location, bufSize, params); }
inline void glGetnUniformiv(GLuint program, GLint location, GLsizei bufSize, GLint* params) { fp_glGetnUniformiv(program, location, bufSize, params); }
inline void glGetnUniformuiv(GLuint program, GLint location, GLsizei bufSize, GLuint* params) { fp_glGetnUniformuiv(program, location, bufSize, params); }
inline void glReadnPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data) { fp_glReadnPixels(x, y, width, height, format, type, bufSize, data); }
inline void glGetnMapdv(GLenum target, GLenum query, GLsizei bufSize, GLdouble* v) { fp_glGetnMapdv(target, query, bufSize, v); }
inline void glGetnMapfv(GLenum target, GLenum query, GLsizei bufSize, GLfloat* v) { fp_glGetnMapfv(target, query, bufSize, v); }
inline void glGetnMapiv(GLenum target, GLenum query, GLsizei bufSize, GLint* v) { fp_glGetnMapiv(target, query, bufSize, v); }
inline void glGetnPixelMapfv(GLenum map, GLsizei bufSize, GLfloat* values) { fp_glGetnPixelMapfv(map, bufSize, values); }
inline void glGetnPixelMapuiv(GLenum map, GLsizei bufSize, GLuint* values) { fp_glGetnPixelMapuiv(map, bufSize, values); }
inline void glGetnPixelMapusv(GLenum map, GLsizei bufSize, GLushort* values) { fp_glGetnPixelMapusv(map, bufSize, values); }
inline void glGetnPolygonStipple(GLsizei bufSize, GLubyte* pattern) { fp_glGetnPolygonStipple(bufSize, pattern); }
inline void glGetnColorTable(GLenum target, GLenum format, GLenum type, GLsizei bufSize, void* table) { fp_glGetnColorTable(target, format, type, bufSize, table); }
inline void glGetnConvolutionFilter(GLenum target, GLenum format, GLenum type, GLsizei bufSize, void* image) { fp_glGetnConvolutionFilter(target, format, type, bufSize, image); }
inline void glGetnSeparableFilter(GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, void* row, GLsizei columnBufSize, void* column, void* span) { fp_glGetnSeparableFilter(target, format, type, rowBufSize, row, columnBufSize, column, span); }
inline void glGetnHistogram(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void* values) { fp_glGetnHistogram(target, reset, format, type, bufSize, values); }
inline void glGetnMinmax(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void* values) { fp_glGetnMinmax(target, reset, format, type, bufSize, values); }
inline void glTextureBarrier() { fp_glTextureBarrier(); }

/* GL_ES_VERSION_3_2 */
inline void glBlendBarrier() { fp_glBlendBarrier(); }
inline void glPrimitiveBoundingBox(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW) { fp_glPrimitiveBoundingBox(minX, minY, minZ, minW, maxX, maxY, maxZ, maxW); }

/* GL_KHR_blend_equation_advanced */
inline void glBlendBarrierKHR() { fp_glBlendBarrierKHR(); }

/* GL_KHR_debug */
inline void glDebugMessageControlKHR(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled) { fp_glDebugMessageControlKHR(source, type, severity, count, ids, enabled); }
inline void glDebugMessageInsertKHR(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf) { fp_glDebugMessageInsertKHR(source, type, id, severity, length, buf); }
inline void glDebugMessageCallbackKHR(GLDEBUGPROCKHR callback, const void* userParam) { fp_glDebugMessageCallbackKHR(callback, userParam); }
inline GLuint glGetDebugMessageLogKHR(GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog) { return fp_glGetDebugMessageLogKHR(count, bufSize, sources, types, ids, severities, lengths, messageLog); }
inline void glPushDebugGroupKHR(GLenum source, GLuint id, GLsizei length, const GLchar* message) { fp_glPushDebugGroupKHR(source, id, length, message); }
inline void glPopDebugGroupKHR() { fp_glPopDebugGroupKHR(); }
inline void glObjectLabelKHR(GLenum identifier, GLuint name, GLsizei length, const GLchar* label) { fp_glObjectLabelKHR(identifier, name, length, label); }
inline void glGetObjectLabelKHR(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei* length, GLchar* label) { fp_glGetObjectLabelKHR(identifier, name, bufSize, length, label); }
inline void glObjectPtrLabelKHR(const void* ptr, GLsizei length, const GLchar* label) { fp_glObjectPtrLabelKHR(ptr, length, label); }
inline void glGetObjectPtrLabelKHR(const void* ptr, GLsizei bufSize, GLsizei* length, GLchar* label) { fp_glGetObjectPtrLabelKHR(ptr, bufSize, length, label); }
inline void glGetPointervKHR(GLenum pname, void** params) { fp_glGetPointervKHR(pname, params); }

/* GL_KHR_robustness */
inline GLenum glGetGraphicsResetStatusKHR() { return fp_glGetGraphicsResetStatusKHR(); }
inline void glReadnPixelsKHR(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data) { fp_glReadnPixelsKHR(x, y, width, height, format, type, bufSize, data); }
inline void glGetnUniformfvKHR(GLuint program, GLint location, GLsizei bufSize, GLfloat* params) { fp_glGetnUniformfvKHR(program, location, bufSize, params); }
inline void glGetnUniformivKHR(GLuint program, GLint location, GLsizei bufSize, GLint* params) { fp_glGetnUniformivKHR(program, location, bufSize, params); }
inline void glGetnUniformuivKHR(GLuint program, GLint location, GLsizei bufSize, GLuint* params) { fp_glGetnUniformuivKHR(program, location, bufSize, params); }

/* GL_ARB_bindless_texture */
inline GLuint64 glGetTextureHandleARB(GLuint texture) { return fp_glGetTextureHandleARB(texture); }
inline GLuint64 glGetTextureSamplerHandleARB(GLuint texture, GLuint sampler) { return fp_glGetTextureSamplerHandleARB(texture, sampler); }
inline void glMakeTextureHandleResidentARB(GLuint64 handle) { fp_glMakeTextureHandleResidentARB(handle); }
inline void glMakeTextureHandleNonResidentARB(GLuint64 handle) { fp_glMakeTextureHandleNonResidentARB(handle); }
inline GLuint64 glGetImageHandleARB(GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format) { return fp_glGetImageHandleARB(texture, level, layered, layer, format); }
inline void glMakeImageHandleResidentARB(GLuint64 handle, GLenum access) { fp_glMakeImageHandleResidentARB(handle, access); }
inline void glMakeImageHandleNonResidentARB(GLuint64 handle) { fp_glMakeImageHandleNonResidentARB(handle); }
inline void glUniformHandleui64ARB(GLint location, GLuint64 value) { fp_glUniformHandleui64ARB(location, value); }
inline void glUniformHandleui64vARB(GLint location, GLsizei count, const GLuint64* value) { fp_glUniformHandleui64vARB(location, count, value); }
inline void glProgramUniformHandleui64ARB(GLuint program, GLint location, GLuint64 value) { fp_glProgramUniformHandleui64ARB(program, location, value); }
inline void glProgramUniformHandleui64vARB(GLuint program, GLint location, GLsizei count, const GLuint64* values) { fp_glProgramUniformHandleui64vARB(program, location, count, values); }
inline GLboolean glIsTextureHandleResidentARB(GLuint64 handle) { return fp_glIsTextureHandleResidentARB(handle); }
inline GLboolean glIsImageHandleResidentARB(GLuint64 handle) { return fp_glIsImageHandleResidentARB(handle); }
inline void glVertexAttribL1ui64ARB(GLuint index, GLuint64EXT x) { fp_glVertexAttribL1ui64ARB(index, x); }
inline void glVertexAttribL1ui64vARB(GLuint index, const GLuint64EXT* v) { fp_glVertexAttribL1ui64vARB(index, v); }
inline void glGetVertexAttribLui64vARB(GLuint index, GLenum pname, GLuint64EXT* params) { fp_glGetVertexAttribLui64vARB(index, pname, params); }

/* GL_ARB_cl_event */
inline GLsync glCreateSyncFromCLeventARB(struct _cl_context* context, struct _cl_event* event, GLbitfield flags) { return fp_glCreateSyncFromCLeventARB(context, event, flags); }

/* GL_ARB_color_buffer_float */
inline void glClampColorARB(GLenum target, GLenum clamp) { fp_glClampColorARB(target, clamp); }

/* GL_ARB_compute_variable_group_size */
inline void glDispatchComputeGroupSizeARB(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z, GLuint group_size_x, GLuint group_size_y, GLuint group_size_z) { fp_glDispatchComputeGroupSizeARB(num_groups_x, num_groups_y, num_groups_z, group_size_x, group_size_y, group_size_z); }

/* GL_ARB_debug_output */
inline void glDebugMessageControlARB(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled) { fp_glDebugMessageControlARB(source, type, severity, count, ids, enabled); }
inline void glDebugMessageInsertARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf) { fp_glDebugMessageInsertARB(source, type, id, severity, length, buf); }
inline void glDebugMessageCallbackARB(GLDEBUGPROCARB callback, const void* userParam) { fp_glDebugMessageCallbackARB(callback, userParam); }
inline GLuint glGetDebugMessageLogARB(GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog) { return fp_glGetDebugMessageLogARB(count, bufSize, sources, types, ids, severities, lengths, messageLog); }

/* GL_ARB_draw_buffers_blend */
inline void glBlendEquationiARB(GLuint buf, GLenum mode) { fp_glBlendEquationiARB(buf, mode); }
inline void glBlendEquationSeparateiARB(GLuint buf, GLenum modeRGB, GLenum modeAlpha) { fp_glBlendEquationSeparateiARB(buf, modeRGB, modeAlpha); }
inline void glBlendFunciARB(GLuint buf, GLenum src, GLenum dst) { fp_glBlendFunciARB(buf, src, dst); }
inline void glBlendFuncSeparateiARB(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) { fp_glBlendFuncSeparateiARB(buf, srcRGB, dstRGB, srcAlpha, dstAlpha); }

/* GL_ARB_draw_instanced */
inline void glDrawArraysInstancedARB(GLenum mode, GLint first, GLsizei count, GLsizei primcount) { fp_glDrawArraysInstancedARB(mode, first, count, primcount); }
inline void glDrawElementsInstancedARB(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount) { fp_glDrawElementsInstancedARB(mode, count, type, indices, primcount); }

/* GL_ARB_ES3_2_compatibility */
inline void glPrimitiveBoundingBoxARB(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW) { fp_glPrimitiveBoundingBoxARB(minX, minY, minZ, minW, maxX, maxY, maxZ, maxW); }

/* GL_ARB_geometry_shader4 */
inline void glProgramParameteriARB(GLuint program, GLenum pname, GLint value) { fp_glProgramParameteriARB(program, pname, value); }
inline void glFramebufferTextureARB(GLenum target, GLenum attachment, GLuint texture, GLint level) { fp_glFramebufferTextureARB(target, attachment, texture, level); }
inline void glFramebufferTextureLayerARB(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) { fp_glFramebufferTextureLayerARB(target, attachment, texture, level, layer); }
inline void glFramebufferTextureFaceARB(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face) { fp_glFramebufferTextureFaceARB(target, attachment, texture, level, face); }

/* GL_ARB_gpu_shader_int64 */
inline void glUniform1i64ARB(GLint location, GLint64 x) { fp_glUniform1i64ARB(location, x); }
inline void glUniform2i64ARB(GLint location, GLint64 x, GLint64 y) { fp_glUniform2i64ARB(location, x, y); }
inline void glUniform3i64ARB(GLint location, GLint64 x, GLint64 y, GLint64 z) { fp_glUniform3i64ARB(location, x, y, z); }
inline void glUniform4i64ARB(GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w) { fp_glUniform4i64ARB(location, x, y, z, w); }
inline void glUniform1i64vARB(GLint location, GLsizei count, const GLint64* value) { fp_glUniform1i64vARB(location, count, value); }
inline void glUniform2i64vARB(GLint location, GLsizei count, const GLint64* value) { fp_glUniform2i64vARB(location, count, value); }
inline void glUniform3i64vARB(GLint location, GLsizei count, const GLint64* value) { fp_glUniform3i64vARB(location, count, value); }
inline void glUniform4i64vARB(GLint location, GLsizei count, const GLint64* value) { fp_glUniform4i64vARB(location, count, value); }
inline void glUniform1ui64ARB(GLint location, GLuint64 x) { fp_glUniform1ui64ARB(location, x); }
inline void glUniform2ui64ARB(GLint location, GLuint64 x, GLuint64 y) { fp_glUniform2ui64ARB(location, x, y); }
inline void glUniform3ui64ARB(GLint location, GLuint64 x, GLuint64 y, GLuint64 z) { fp_glUniform3ui64ARB(location, x, y, z); }
inline void glUniform4ui64ARB(GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w) { fp_glUniform4ui64ARB(location, x, y, z, w); }
inline void glUniform1ui64vARB(GLint location, GLsizei count, const GLuint64* value) { fp_glUniform1ui64vARB(location, count, value); }
inline void glUniform2ui64vARB(GLint location, GLsizei count, const GLuint64* value) { fp_glUniform2ui64vARB(location, count, value); }
inline void glUniform3ui64vARB(GLint location, GLsizei count, const GLuint64* value) { fp_glUniform3ui64vARB(location, count, value); }
inline void glUniform4ui64vARB(GLint location, GLsizei count, const GLuint64* value) { fp_glUniform4ui64vARB(location, count, value); }
inline void glGetUniformi64vARB(GLuint program, GLint location, GLint64* params) { fp_glGetUniformi64vARB(program, location, params); }
inline void glGetUniformui64vARB(GLuint program, GLint location, GLuint64* params) { fp_glGetUniformui64vARB(program, location, params); }
inline void glGetnUniformi64vARB(GLuint program, GLint location, GLsizei bufSize, GLint64* params) { fp_glGetnUniformi64vARB(program, location, bufSize, params); }
inline void glGetnUniformui64vARB(GLuint program, GLint location, GLsizei bufSize, GLuint64* params) { fp_glGetnUniformui64vARB(program, location, bufSize, params); }
inline void glProgramUniform1i64ARB(GLuint program, GLint location, GLint64 x) { fp_glProgramUniform1i64ARB(program, location, x); }
inline void glProgramUniform2i64ARB(GLuint program, GLint location, GLint64 x, GLint64 y) { fp_glProgramUniform2i64ARB(program, location, x, y); }
inline void glProgramUniform3i64ARB(GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z) { fp_glProgramUniform3i64ARB(program, location, x, y, z); }
inline void glProgramUniform4i64ARB(GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w) { fp_glProgramUniform4i64ARB(program, location, x, y, z, w); }
inline void glProgramUniform1i64vARB(GLuint program, GLint location, GLsizei count, const GLint64* value) { fp_glProgramUniform1i64vARB(program, location, count, value); }
inline void glProgramUniform2i64vARB(GLuint program, GLint location, GLsizei count, const GLint64* value) { fp_glProgramUniform2i64vARB(program, location, count, value); }
inline void glProgramUniform3i64vARB(GLuint program, GLint location, GLsizei count, const GLint64* value) { fp_glProgramUniform3i64vARB(program, location, count, value); }
inline void glProgramUniform4i64vARB(GLuint program, GLint location, GLsizei count, const GLint64* value) { fp_glProgramUniform4i64vARB(program, location, count, value); }
inline void glProgramUniform1ui64ARB(GLuint program, GLint location, GLuint64 x) { fp_glProgramUniform1ui64ARB(program, location, x); }
inline void glProgramUniform2ui64ARB(GLuint program, GLint location, GLuint64 x, GLuint64 y) { fp_glProgramUniform2ui64ARB(program, location, x, y); }
inline void glProgramUniform3ui64ARB(GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z) { fp_glProgramUniform3ui64ARB(program, location, x, y, z); }
inline void glProgramUniform4ui64ARB(GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w) { fp_glProgramUniform4ui64ARB(program, location, x, y, z, w); }
inline void glProgramUniform1ui64vARB(GLuint program, GLint location, GLsizei count, const GLuint64* value) { fp_glProgramUniform1ui64vARB(program, location, count, value); }
inline void glProgramUniform2ui64vARB(GLuint program, GLint location, GLsizei count, const GLuint64* value) { fp_glProgramUniform2ui64vARB(program, location, count, value); }
inline void glProgramUniform3ui64vARB(GLuint program, GLint location, GLsizei count, const GLuint64* value) { fp_glProgramUniform3ui64vARB(program, location, count, value); }
inline void glProgramUniform4ui64vARB(GLuint program, GLint location, GLsizei count, const GLuint64* value) { fp_glProgramUniform4ui64vARB(program, location, count, value); }

/* GL_ARB_indirect_parameters */
inline void glMultiDrawArraysIndirectCountARB(GLenum mode, GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride) { fp_glMultiDrawArraysIndirectCountARB(mode, indirect, drawcount, maxdrawcount, stride); }
inline void glMultiDrawElementsIndirectCountARB(GLenum mode, GLenum type, GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride) { fp_glMultiDrawElementsIndirectCountARB(mode, type, indirect, drawcount, maxdrawcount, stride); }

/* GL_ARB_instanced_arrays */
inline void glVertexAttribDivisorARB(GLuint index, GLuint divisor) { fp_glVertexAttribDivisorARB(index, divisor); }

/* GL_ARB_parallel_shader_compile */
inline void glMaxShaderCompilerThreadsARB(GLuint count) { fp_glMaxShaderCompilerThreadsARB(count); }

/* GL_ARB_robustness */
inline GLenum glGetGraphicsResetStatusARB() { return fp_glGetGraphicsResetStatusARB(); }
inline void glGetnTexImageARB(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* img) { fp_glGetnTexImageARB(target, level, format, type, bufSize, img); }
inline void glReadnPixelsARB(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data) { fp_glReadnPixelsARB(x, y, width, height, format, type, bufSize, data); }
inline void glGetnCompressedTexImageARB(GLenum target, GLint lod, GLsizei bufSize, void* img) { fp_glGetnCompressedTexImageARB(target, lod, bufSize, img); }
inline void glGetnUniformfvARB(GLuint program, GLint location, GLsizei bufSize, GLfloat* params) { fp_glGetnUniformfvARB(program, location, bufSize, params); }
inline void glGetnUniformivARB(GLuint program, GLint location, GLsizei bufSize, GLint* params) { fp_glGetnUniformivARB(program, location, bufSize, params); }
inline void glGetnUniformuivARB(GLuint program, GLint location, GLsizei bufSize, GLuint* params) { fp_glGetnUniformuivARB(program, location, bufSize, params); }
inline void glGetnUniformdvARB(GLuint program, GLint location, GLsizei bufSize, GLdouble* params) { fp_glGetnUniformdvARB(program, location, bufSize, params); }
inline void glGetnMapdvARB(GLenum target, GLenum query, GLsizei bufSize, GLdouble* v) { fp_glGetnMapdvARB(target, query, bufSize, v); }
inline void glGetnMapfvARB(GLenum target, GLenum query, GLsizei bufSize, GLfloat* v) { fp_glGetnMapfvARB(target, query, bufSize, v); }
inline void glGetnMapivARB(GLenum target, GLenum query, GLsizei bufSize, GLint* v) { fp_glGetnMapivARB(target, query, bufSize, v); }
inline void glGetnPixelMapfvARB(GLenum map, GLsizei bufSize, GLfloat* values) { fp_glGetnPixelMapfvARB(map, bufSize, values); }
inline void glGetnPixelMapuivARB(GLenum map, GLsizei bufSize, GLuint* values) { fp_glGetnPixelMapuivARB(map, bufSize, values); }
inline void glGetnPixelMapusvARB(GLenum map, GLsizei bufSize, GLushort* values) { fp_glGetnPixelMapusvARB(map, bufSize, values); }
inline void glGetnPolygonStippleARB(GLsizei bufSize, GLubyte* pattern) { fp_glGetnPolygonStippleARB(bufSize, pattern); }
inline void glGetnColorTableARB(GLenum target, GLenum format, GLenum type, GLsizei bufSize, void* table) { fp_glGetnColorTableARB(target, format, type, bufSize, table); }
inline void glGetnConvolutionFilterARB(GLenum target, GLenum format, GLenum type, GLsizei bufSize, void* image) { fp_glGetnConvolutionFilterARB(target, format, type, bufSize, image); }
inline void glGetnSeparableFilterARB(GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, void* row, GLsizei columnBufSize, void* column, void* span) { fp_glGetnSeparableFilterARB(target, format, type, rowBufSize, row, columnBufSize, column, span); }
inline void glGetnHistogramARB(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void* values) { fp_glGetnHistogramARB(target, reset, format, type, bufSize, values); }
inline void glGetnMinmaxARB(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void* values) { fp_glGetnMinmaxARB(target, reset, format, type, bufSize, values); }

/* GL_ARB_sample_locations */
inline void glFramebufferSampleLocationsfvARB(GLenum target, GLuint start, GLsizei count, const GLfloat* v) { fp_glFramebufferSampleLocationsfvARB(target, start, count, v); }
inline void glNamedFramebufferSampleLocationsfvARB(GLuint framebuffer, GLuint start, GLsizei count, const GLfloat* v) { fp_glNamedFramebufferSampleLocationsfvARB(framebuffer, start, count, v); }
inline void glEvaluateDepthValuesARB() { fp_glEvaluateDepthValuesARB(); }

/* GL_ARB_sample_shading */
inline void glMinSampleShadingARB(GLfloat value) { fp_glMinSampleShadingARB(value); }

/* GL_ARB_shading_language_include */
inline void glNamedStringARB(GLenum type, GLint namelen, const GLchar* name, GLint stringlen, const GLchar* string) { fp_glNamedStringARB(type, namelen, name, stringlen, string); }
inline void glDeleteNamedStringARB(GLint namelen, const GLchar* name) { fp_glDeleteNamedStringARB(namelen, name); }
inline void glCompileShaderIncludeARB(GLuint shader, GLsizei count, const GLchar** path, const GLint* length) { fp_glCompileShaderIncludeARB(shader, count, path, length); }
inline GLboolean glIsNamedStringARB(GLint namelen, const GLchar* name) { return fp_glIsNamedStringARB(namelen, name); }
inline void glGetNamedStringARB(GLint namelen, const GLchar* name, GLsizei bufSize, GLint* stringlen, GLchar* string) { fp_glGetNamedStringARB(namelen, name, bufSize, stringlen, string); }
inline void glGetNamedStringivARB(GLint namelen, const GLchar* name, GLenum pname, GLint* params) { fp_glGetNamedStringivARB(namelen, name, pname, params); }

/* GL_ARB_sparse_buffer */
inline void glBufferPageCommitmentARB(GLenum target, GLintptr offset, GLsizeiptr size, GLboolean commit) { fp_glBufferPageCommitmentARB(target, offset, size, commit); }
inline void glNamedBufferPageCommitmentEXT(GLuint buffer, GLintptr offset, GLsizeiptr size, GLboolean commit) { fp_glNamedBufferPageCommitmentEXT(buffer, offset, size, commit); }
inline void glNamedBufferPageCommitmentARB(GLuint buffer, GLintptr offset, GLsizeiptr size, GLboolean commit) { fp_glNamedBufferPageCommitmentARB(buffer, offset, size, commit); }

/* GL_ARB_sparse_texture */
inline void glTexPageCommitmentARB(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit) { fp_glTexPageCommitmentARB(target, level, xoffset, yoffset, zoffset, width, height, depth, commit); }

/* GL_ARB_texture_buffer_object */
inline void glTexBufferARB(GLenum target, GLenum internalformat, GLuint buffer) { fp_glTexBufferARB(target, internalformat, buffer); }

/* GL_EXT_base_instance */
inline void glDrawArraysInstancedBaseInstanceEXT(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance) { fp_glDrawArraysInstancedBaseInstanceEXT(mode, first, count, instancecount, baseinstance); }
inline void glDrawElementsInstancedBaseInstanceEXT(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLuint baseinstance) { fp_glDrawElementsInstancedBaseInstanceEXT(mode, count, type, indices, instancecount, baseinstance); }
inline void glDrawElementsInstancedBaseVertexBaseInstanceEXT(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance) { fp_glDrawElementsInstancedBaseVertexBaseInstanceEXT(mode, count, type, indices, instancecount, basevertex, baseinstance); }

/* GL_EXT_bindable_uniform */
inline void glUniformBufferEXT(GLuint program, GLint location, GLuint buffer) { fp_glUniformBufferEXT(program, location, buffer); }
inline GLint glGetUniformBufferSizeEXT(GLuint program, GLint location) { return fp_glGetUniformBufferSizeEXT(program, location); }
inline GLintptr glGetUniformOffsetEXT(GLuint program, GLint location) { return fp_glGetUniformOffsetEXT(program, location); }

/* GL_EXT_blend_func_extended */
inline void glBindFragDataLocationIndexedEXT(GLuint program, GLuint colorNumber, GLuint index, const GLchar* name) { fp_glBindFragDataLocationIndexedEXT(program, colorNumber, index, name); }
inline void glBindFragDataLocationEXT(GLuint program, GLuint color, const GLchar* name) { fp_glBindFragDataLocationEXT(program, color, name); }
inline GLint glGetProgramResourceLocationIndexEXT(GLuint program, GLenum programInterface, const GLchar* name) { return fp_glGetProgramResourceLocationIndexEXT(program, programInterface, name); }
inline GLint glGetFragDataIndexEXT(GLuint program, const GLchar* name) { return fp_glGetFragDataIndexEXT(program, name); }

/* GL_EXT_blend_minmax */
inline void glBlendEquationEXT(GLenum mode) { fp_glBlendEquationEXT(mode); }

/* GL_EXT_buffer_storage */
inline void glBufferStorageEXT(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags) { fp_glBufferStorageEXT(target, size, data, flags); }

/* GL_EXT_copy_image */
inline void glCopyImageSubDataEXT(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth) { fp_glCopyImageSubDataEXT(srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth); }

/* GL_EXT_debug_label */
inline void glLabelObjectEXT(GLenum type, GLuint object, GLsizei length, const GLchar* label) { fp_glLabelObjectEXT(type, object, length, label); }
inline void glGetObjectLabelEXT(GLenum type, GLuint object, GLsizei bufSize, GLsizei* length, GLchar* label) { fp_glGetObjectLabelEXT(type, object, bufSize, length, label); }

/* GL_EXT_debug_marker */
inline void glInsertEventMarkerEXT(GLsizei length, const GLchar* marker) { fp_glInsertEventMarkerEXT(length, marker); }
inline void glPushGroupMarkerEXT(GLsizei length, const GLchar* marker) { fp_glPushGroupMarkerEXT(length, marker); }
inline void glPopGroupMarkerEXT() { fp_glPopGroupMarkerEXT(); }

/* GL_EXT_direct_state_access */
inline void glMatrixLoadfEXT(GLenum mode, const GLfloat* m) { fp_glMatrixLoadfEXT(mode, m); }
inline void glMatrixLoaddEXT(GLenum mode, const GLdouble* m) { fp_glMatrixLoaddEXT(mode, m); }
inline void glMatrixMultfEXT(GLenum mode, const GLfloat* m) { fp_glMatrixMultfEXT(mode, m); }
inline void glMatrixMultdEXT(GLenum mode, const GLdouble* m) { fp_glMatrixMultdEXT(mode, m); }
inline void glMatrixLoadIdentityEXT(GLenum mode) { fp_glMatrixLoadIdentityEXT(mode); }
inline void glMatrixRotatefEXT(GLenum mode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z) { fp_glMatrixRotatefEXT(mode, angle, x, y, z); }
inline void glMatrixRotatedEXT(GLenum mode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z) { fp_glMatrixRotatedEXT(mode, angle, x, y, z); }
inline void glMatrixScalefEXT(GLenum mode, GLfloat x, GLfloat y, GLfloat z) { fp_glMatrixScalefEXT(mode, x, y, z); }
inline void glMatrixScaledEXT(GLenum mode, GLdouble x, GLdouble y, GLdouble z) { fp_glMatrixScaledEXT(mode, x, y, z); }
inline void glMatrixTranslatefEXT(GLenum mode, GLfloat x, GLfloat y, GLfloat z) { fp_glMatrixTranslatefEXT(mode, x, y, z); }
inline void glMatrixTranslatedEXT(GLenum mode, GLdouble x, GLdouble y, GLdouble z) { fp_glMatrixTranslatedEXT(mode, x, y, z); }
inline void glMatrixFrustumEXT(GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) { fp_glMatrixFrustumEXT(mode, left, right, bottom, top, zNear, zFar); }
inline void glMatrixOrthoEXT(GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) { fp_glMatrixOrthoEXT(mode, left, right, bottom, top, zNear, zFar); }
inline void glMatrixPopEXT(GLenum mode) { fp_glMatrixPopEXT(mode); }
inline void glMatrixPushEXT(GLenum mode) { fp_glMatrixPushEXT(mode); }
inline void glClientAttribDefaultEXT(GLbitfield mask) { fp_glClientAttribDefaultEXT(mask); }
inline void glPushClientAttribDefaultEXT(GLbitfield mask) { fp_glPushClientAttribDefaultEXT(mask); }
inline void glTextureParameterfEXT(GLuint texture, GLenum target, GLenum pname, GLfloat param) { fp_glTextureParameterfEXT(texture, target, pname, param); }
inline void glTextureParameterfvEXT(GLuint texture, GLenum target, GLenum pname, const GLfloat* params) { fp_glTextureParameterfvEXT(texture, target, pname, params); }
inline void glTextureParameteriEXT(GLuint texture, GLenum target, GLenum pname, GLint param) { fp_glTextureParameteriEXT(texture, target, pname, param); }
inline void glTextureParameterivEXT(GLuint texture, GLenum target, GLenum pname, const GLint* params) { fp_glTextureParameterivEXT(texture, target, pname, params); }
inline void glTextureImage1DEXT(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels) { fp_glTextureImage1DEXT(texture, target, level, internalformat, width, border, format, type, pixels); }
inline void glTextureImage2DEXT(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels) { fp_glTextureImage2DEXT(texture, target, level, internalformat, width, height, border, format, type, pixels); }
inline void glTextureSubImage1DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels) { fp_glTextureSubImage1DEXT(texture, target, level, xoffset, width, format, type, pixels); }
inline void glTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) { fp_glTextureSubImage2DEXT(texture, target, level, xoffset, yoffset, width, height, format, type, pixels); }
inline void glCopyTextureImage1DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border) { fp_glCopyTextureImage1DEXT(texture, target, level, internalformat, x, y, width, border); }
inline void glCopyTextureImage2DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) { fp_glCopyTextureImage2DEXT(texture, target, level, internalformat, x, y, width, height, border); }
inline void glCopyTextureSubImage1DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) { fp_glCopyTextureSubImage1DEXT(texture, target, level, xoffset, x, y, width); }
inline void glCopyTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) { fp_glCopyTextureSubImage2DEXT(texture, target, level, xoffset, yoffset, x, y, width, height); }
inline void glGetTextureImageEXT(GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, void* pixels) { fp_glGetTextureImageEXT(texture, target, level, format, type, pixels); }
inline void glGetTextureParameterfvEXT(GLuint texture, GLenum target, GLenum pname, GLfloat* params) { fp_glGetTextureParameterfvEXT(texture, target, pname, params); }
inline void glGetTextureParameterivEXT(GLuint texture, GLenum target, GLenum pname, GLint* params) { fp_glGetTextureParameterivEXT(texture, target, pname, params); }
inline void glGetTextureLevelParameterfvEXT(GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat* params) { fp_glGetTextureLevelParameterfvEXT(texture, target, level, pname, params); }
inline void glGetTextureLevelParameterivEXT(GLuint texture, GLenum target, GLint level, GLenum pname, GLint* params) { fp_glGetTextureLevelParameterivEXT(texture, target, level, pname, params); }
inline void glTextureImage3DEXT(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) { fp_glTextureImage3DEXT(texture, target, level, internalformat, width, height, depth, border, format, type, pixels); }
inline void glTextureSubImage3DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) { fp_glTextureSubImage3DEXT(texture, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels); }
inline void glCopyTextureSubImage3DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) { fp_glCopyTextureSubImage3DEXT(texture, target, level, xoffset, yoffset, zoffset, x, y, width, height); }
inline void glBindMultiTextureEXT(GLenum texunit, GLenum target, GLuint texture) { fp_glBindMultiTextureEXT(texunit, target, texture); }
inline void glMultiTexCoordPointerEXT(GLenum texunit, GLint size, GLenum type, GLsizei stride, const void* pointer) { fp_glMultiTexCoordPointerEXT(texunit, size, type, stride, pointer); }
inline void glMultiTexEnvfEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat param) { fp_glMultiTexEnvfEXT(texunit, target, pname, param); }
inline void glMultiTexEnvfvEXT(GLenum texunit, GLenum target, GLenum pname, const GLfloat* params) { fp_glMultiTexEnvfvEXT(texunit, target, pname, params); }
inline void glMultiTexEnviEXT(GLenum texunit, GLenum target, GLenum pname, GLint param) { fp_glMultiTexEnviEXT(texunit, target, pname, param); }
inline void glMultiTexEnvivEXT(GLenum texunit, GLenum target, GLenum pname, const GLint* params) { fp_glMultiTexEnvivEXT(texunit, target, pname, params); }
inline void glMultiTexGendEXT(GLenum texunit, GLenum coord, GLenum pname, GLdouble param) { fp_glMultiTexGendEXT(texunit, coord, pname, param); }
inline void glMultiTexGendvEXT(GLenum texunit, GLenum coord, GLenum pname, const GLdouble* params) { fp_glMultiTexGendvEXT(texunit, coord, pname, params); }
inline void glMultiTexGenfEXT(GLenum texunit, GLenum coord, GLenum pname, GLfloat param) { fp_glMultiTexGenfEXT(texunit, coord, pname, param); }
inline void glMultiTexGenfvEXT(GLenum texunit, GLenum coord, GLenum pname, const GLfloat* params) { fp_glMultiTexGenfvEXT(texunit, coord, pname, params); }
inline void glMultiTexGeniEXT(GLenum texunit, GLenum coord, GLenum pname, GLint param) { fp_glMultiTexGeniEXT(texunit, coord, pname, param); }
inline void glMultiTexGenivEXT(GLenum texunit, GLenum coord, GLenum pname, const GLint* params) { fp_glMultiTexGenivEXT(texunit, coord, pname, params); }
inline void glGetMultiTexEnvfvEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat* params) { fp_glGetMultiTexEnvfvEXT(texunit, target, pname, params); }
inline void glGetMultiTexEnvivEXT(GLenum texunit, GLenum target, GLenum pname, GLint* params) { fp_glGetMultiTexEnvivEXT(texunit, target, pname, params); }
inline void glGetMultiTexGendvEXT(GLenum texunit, GLenum coord, GLenum pname, GLdouble* params) { fp_glGetMultiTexGendvEXT(texunit, coord, pname, params); }
inline void glGetMultiTexGenfvEXT(GLenum texunit, GLenum coord, GLenum pname, GLfloat* params) { fp_glGetMultiTexGenfvEXT(texunit, coord, pname, params); }
inline void glGetMultiTexGenivEXT(GLenum texunit, GLenum coord, GLenum pname, GLint* params) { fp_glGetMultiTexGenivEXT(texunit, coord, pname, params); }
inline void glMultiTexParameteriEXT(GLenum texunit, GLenum target, GLenum pname, GLint param) { fp_glMultiTexParameteriEXT(texunit, target, pname, param); }
inline void glMultiTexParameterivEXT(GLenum texunit, GLenum target, GLenum pname, const GLint* params) { fp_glMultiTexParameterivEXT(texunit, target, pname, params); }
inline void glMultiTexParameterfEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat param) { fp_glMultiTexParameterfEXT(texunit, target, pname, param); }
inline void glMultiTexParameterfvEXT(GLenum texunit, GLenum target, GLenum pname, const GLfloat* params) { fp_glMultiTexParameterfvEXT(texunit, target, pname, params); }
inline void glMultiTexImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels) { fp_glMultiTexImage1DEXT(texunit, target, level, internalformat, width, border, format, type, pixels); }
inline void glMultiTexImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels) { fp_glMultiTexImage2DEXT(texunit, target, level, internalformat, width, height, border, format, type, pixels); }
inline void glMultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels) { fp_glMultiTexSubImage1DEXT(texunit, target, level, xoffset, width, format, type, pixels); }
inline void glMultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) { fp_glMultiTexSubImage2DEXT(texunit, target, level, xoffset, yoffset, width, height, format, type, pixels); }
inline void glCopyMultiTexImage1DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border) { fp_glCopyMultiTexImage1DEXT(texunit, target, level, internalformat, x, y, width, border); }
inline void glCopyMultiTexImage2DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) { fp_glCopyMultiTexImage2DEXT(texunit, target, level, internalformat, x, y, width, height, border); }
inline void glCopyMultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) { fp_glCopyMultiTexSubImage1DEXT(texunit, target, level, xoffset, x, y, width); }
inline void glCopyMultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) { fp_glCopyMultiTexSubImage2DEXT(texunit, target, level, xoffset, yoffset, x, y, width, height); }
inline void glGetMultiTexImageEXT(GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, void* pixels) { fp_glGetMultiTexImageEXT(texunit, target, level, format, type, pixels); }
inline void glGetMultiTexParameterfvEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat* params) { fp_glGetMultiTexParameterfvEXT(texunit, target, pname, params); }
inline void glGetMultiTexParameterivEXT(GLenum texunit, GLenum target, GLenum pname, GLint* params) { fp_glGetMultiTexParameterivEXT(texunit, target, pname, params); }
inline void glGetMultiTexLevelParameterfvEXT(GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat* params) { fp_glGetMultiTexLevelParameterfvEXT(texunit, target, level, pname, params); }
inline void glGetMultiTexLevelParameterivEXT(GLenum texunit, GLenum target, GLint level, GLenum pname, GLint* params) { fp_glGetMultiTexLevelParameterivEXT(texunit, target, level, pname, params); }
inline void glMultiTexImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) { fp_glMultiTexImage3DEXT(texunit, target, level, internalformat, width, height, depth, border, format, type, pixels); }
inline void glMultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) { fp_glMultiTexSubImage3DEXT(texunit, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels); }
inline void glCopyMultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) { fp_glCopyMultiTexSubImage3DEXT(texunit, target, level, xoffset, yoffset, zoffset, x, y, width, height); }
inline void glEnableClientStateIndexedEXT(GLenum array, GLuint index) { fp_glEnableClientStateIndexedEXT(array, index); }
inline void glDisableClientStateIndexedEXT(GLenum array, GLuint index) { fp_glDisableClientStateIndexedEXT(array, index); }
inline void glGetFloatIndexedvEXT(GLenum target, GLuint index, GLfloat* data) { fp_glGetFloatIndexedvEXT(target, index, data); }
inline void glGetDoubleIndexedvEXT(GLenum target, GLuint index, GLdouble* data) { fp_glGetDoubleIndexedvEXT(target, index, data); }
inline void glGetPointerIndexedvEXT(GLenum target, GLuint index, void** data) { fp_glGetPointerIndexedvEXT(target, index, data); }
inline void glEnableIndexedEXT(GLenum target, GLuint index) { fp_glEnableIndexedEXT(target, index); }
inline void glDisableIndexedEXT(GLenum target, GLuint index) { fp_glDisableIndexedEXT(target, index); }
inline GLboolean glIsEnabledIndexedEXT(GLenum target, GLuint index) { return fp_glIsEnabledIndexedEXT(target, index); }
inline void glGetIntegerIndexedvEXT(GLenum target, GLuint index, GLint* data) { fp_glGetIntegerIndexedvEXT(target, index, data); }
inline void glGetBooleanIndexedvEXT(GLenum target, GLuint index, GLboolean* data) { fp_glGetBooleanIndexedvEXT(target, index, data); }
inline void glCompressedTextureImage3DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* bits) { fp_glCompressedTextureImage3DEXT(texture, target, level, internalformat, width, height, depth, border, imageSize, bits); }
inline void glCompressedTextureImage2DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* bits) { fp_glCompressedTextureImage2DEXT(texture, target, level, internalformat, width, height, border, imageSize, bits); }
inline void glCompressedTextureImage1DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* bits) { fp_glCompressedTextureImage1DEXT(texture, target, level, internalformat, width, border, imageSize, bits); }
inline void glCompressedTextureSubImage3DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* bits) { fp_glCompressedTextureSubImage3DEXT(texture, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, bits); }
inline void glCompressedTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* bits) { fp_glCompressedTextureSubImage2DEXT(texture, target, level, xoffset, yoffset, width, height, format, imageSize, bits); }
inline void glCompressedTextureSubImage1DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* bits) { fp_glCompressedTextureSubImage1DEXT(texture, target, level, xoffset, width, format, imageSize, bits); }
inline void glGetCompressedTextureImageEXT(GLuint texture, GLenum target, GLint lod, void* img) { fp_glGetCompressedTextureImageEXT(texture, target, lod, img); }
inline void glCompressedMultiTexImage3DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* bits) { fp_glCompressedMultiTexImage3DEXT(texunit, target, level, internalformat, width, height, depth, border, imageSize, bits); }
inline void glCompressedMultiTexImage2DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* bits) { fp_glCompressedMultiTexImage2DEXT(texunit, target, level, internalformat, width, height, border, imageSize, bits); }
inline void glCompressedMultiTexImage1DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* bits) { fp_glCompressedMultiTexImage1DEXT(texunit, target, level, internalformat, width, border, imageSize, bits); }
inline void glCompressedMultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* bits) { fp_glCompressedMultiTexSubImage3DEXT(texunit, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, bits); }
inline void glCompressedMultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* bits) { fp_glCompressedMultiTexSubImage2DEXT(texunit, target, level, xoffset, yoffset, width, height, format, imageSize, bits); }
inline void glCompressedMultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* bits) { fp_glCompressedMultiTexSubImage1DEXT(texunit, target, level, xoffset, width, format, imageSize, bits); }
inline void glGetCompressedMultiTexImageEXT(GLenum texunit, GLenum target, GLint lod, void* img) { fp_glGetCompressedMultiTexImageEXT(texunit, target, lod, img); }
inline void glMatrixLoadTransposefEXT(GLenum mode, const GLfloat* m) { fp_glMatrixLoadTransposefEXT(mode, m); }
inline void glMatrixLoadTransposedEXT(GLenum mode, const GLdouble* m) { fp_glMatrixLoadTransposedEXT(mode, m); }
inline void glMatrixMultTransposefEXT(GLenum mode, const GLfloat* m) { fp_glMatrixMultTransposefEXT(mode, m); }
inline void glMatrixMultTransposedEXT(GLenum mode, const GLdouble* m) { fp_glMatrixMultTransposedEXT(mode, m); }
inline void glNamedBufferDataEXT(GLuint buffer, GLsizeiptr size, const void* data, GLenum usage) { fp_glNamedBufferDataEXT(buffer, size, data, usage); }
inline void glNamedBufferSubDataEXT(GLuint buffer, GLintptr offset, GLsizeiptr size, const void* data) { fp_glNamedBufferSubDataEXT(buffer, offset, size, data); }
inline void* glMapNamedBufferEXT(GLuint buffer, GLenum access) { return fp_glMapNamedBufferEXT(buffer, access); }
inline GLboolean glUnmapNamedBufferEXT(GLuint buffer) { return fp_glUnmapNamedBufferEXT(buffer); }
inline void glGetNamedBufferParameterivEXT(GLuint buffer, GLenum pname, GLint* params) { fp_glGetNamedBufferParameterivEXT(buffer, pname, params); }
inline void glGetNamedBufferPointervEXT(GLuint buffer, GLenum pname, void** params) { fp_glGetNamedBufferPointervEXT(buffer, pname, params); }
inline void glGetNamedBufferSubDataEXT(GLuint buffer, GLintptr offset, GLsizeiptr size, void* data) { fp_glGetNamedBufferSubDataEXT(buffer, offset, size, data); }
inline void glProgramUniform1fEXT(GLuint program, GLint location, GLfloat v0) { fp_glProgramUniform1fEXT(program, location, v0); }
inline void glProgramUniform2fEXT(GLuint program, GLint location, GLfloat v0, GLfloat v1) { fp_glProgramUniform2fEXT(program, location, v0, v1); }
inline void glProgramUniform3fEXT(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) { fp_glProgramUniform3fEXT(program, location, v0, v1, v2); }
inline void glProgramUniform4fEXT(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) { fp_glProgramUniform4fEXT(program, location, v0, v1, v2, v3); }
inline void glProgramUniform1iEXT(GLuint program, GLint location, GLint v0) { fp_glProgramUniform1iEXT(program, location, v0); }
inline void glProgramUniform2iEXT(GLuint program, GLint location, GLint v0, GLint v1) { fp_glProgramUniform2iEXT(program, location, v0, v1); }
inline void glProgramUniform3iEXT(GLuint program, GLint location, GLint v0, GLint v1, GLint v2) { fp_glProgramUniform3iEXT(program, location, v0, v1, v2); }
inline void glProgramUniform4iEXT(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) { fp_glProgramUniform4iEXT(program, location, v0, v1, v2, v3); }
inline void glProgramUniform1fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value) { fp_glProgramUniform1fvEXT(program, location, count, value); }
inline void glProgramUniform2fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value) { fp_glProgramUniform2fvEXT(program, location, count, value); }
inline void glProgramUniform3fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value) { fp_glProgramUniform3fvEXT(program, location, count, value); }
inline void glProgramUniform4fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value) { fp_glProgramUniform4fvEXT(program, location, count, value); }
inline void glProgramUniform1ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value) { fp_glProgramUniform1ivEXT(program, location, count, value); }
inline void glProgramUniform2ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value) { fp_glProgramUniform2ivEXT(program, location, count, value); }
inline void glProgramUniform3ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value) { fp_glProgramUniform3ivEXT(program, location, count, value); }
inline void glProgramUniform4ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value) { fp_glProgramUniform4ivEXT(program, location, count, value); }
inline void glProgramUniformMatrix2fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix2fvEXT(program, location, count, transpose, value); }
inline void glProgramUniformMatrix3fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix3fvEXT(program, location, count, transpose, value); }
inline void glProgramUniformMatrix4fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix4fvEXT(program, location, count, transpose, value); }
inline void glProgramUniformMatrix2x3fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix2x3fvEXT(program, location, count, transpose, value); }
inline void glProgramUniformMatrix3x2fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix3x2fvEXT(program, location, count, transpose, value); }
inline void glProgramUniformMatrix2x4fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix2x4fvEXT(program, location, count, transpose, value); }
inline void glProgramUniformMatrix4x2fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix4x2fvEXT(program, location, count, transpose, value); }
inline void glProgramUniformMatrix3x4fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix3x4fvEXT(program, location, count, transpose, value); }
inline void glProgramUniformMatrix4x3fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glProgramUniformMatrix4x3fvEXT(program, location, count, transpose, value); }
inline void glTextureBufferEXT(GLuint texture, GLenum target, GLenum internalformat, GLuint buffer) { fp_glTextureBufferEXT(texture, target, internalformat, buffer); }
inline void glMultiTexBufferEXT(GLenum texunit, GLenum target, GLenum internalformat, GLuint buffer) { fp_glMultiTexBufferEXT(texunit, target, internalformat, buffer); }
inline void glTextureParameterIivEXT(GLuint texture, GLenum target, GLenum pname, const GLint* params) { fp_glTextureParameterIivEXT(texture, target, pname, params); }
inline void glTextureParameterIuivEXT(GLuint texture, GLenum target, GLenum pname, const GLuint* params) { fp_glTextureParameterIuivEXT(texture, target, pname, params); }
inline void glGetTextureParameterIivEXT(GLuint texture, GLenum target, GLenum pname, GLint* params) { fp_glGetTextureParameterIivEXT(texture, target, pname, params); }
inline void glGetTextureParameterIuivEXT(GLuint texture, GLenum target, GLenum pname, GLuint* params) { fp_glGetTextureParameterIuivEXT(texture, target, pname, params); }
inline void glMultiTexParameterIivEXT(GLenum texunit, GLenum target, GLenum pname, const GLint* params) { fp_glMultiTexParameterIivEXT(texunit, target, pname, params); }
inline void glMultiTexParameterIuivEXT(GLenum texunit, GLenum target, GLenum pname, const GLuint* params) { fp_glMultiTexParameterIuivEXT(texunit, target, pname, params); }
inline void glGetMultiTexParameterIivEXT(GLenum texunit, GLenum target, GLenum pname, GLint* params) { fp_glGetMultiTexParameterIivEXT(texunit, target, pname, params); }
inline void glGetMultiTexParameterIuivEXT(GLenum texunit, GLenum target, GLenum pname, GLuint* params) { fp_glGetMultiTexParameterIuivEXT(texunit, target, pname, params); }
inline void glProgramUniform1uiEXT(GLuint program, GLint location, GLuint v0) { fp_glProgramUniform1uiEXT(program, location, v0); }
inline void glProgramUniform2uiEXT(GLuint program, GLint location, GLuint v0, GLuint v1) { fp_glProgramUniform2uiEXT(program, location, v0, v1); }
inline void glProgramUniform3uiEXT(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2) { fp_glProgramUniform3uiEXT(program, location, v0, v1, v2); }
inline void glProgramUniform4uiEXT(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) { fp_glProgramUniform4uiEXT(program, location, v0, v1, v2, v3); }
inline void glProgramUniform1uivEXT(GLuint program, GLint location, GLsizei count, const GLuint* value) { fp_glProgramUniform1uivEXT(program, location, count, value); }
inline void glProgramUniform2uivEXT(GLuint program, GLint location, GLsizei count, const GLuint* value) { fp_glProgramUniform2uivEXT(program, location, count, value); }
inline void glProgramUniform3uivEXT(GLuint program, GLint location, GLsizei count, const GLuint* value) { fp_glProgramUniform3uivEXT(program, location, count, value); }
inline void glProgramUniform4uivEXT(GLuint program, GLint location, GLsizei count, const GLuint* value) { fp_glProgramUniform4uivEXT(program, location, count, value); }
inline void glNamedProgramLocalParameters4fvEXT(GLuint program, GLenum target, GLuint index, GLsizei count, const GLfloat* params) { fp_glNamedProgramLocalParameters4fvEXT(program, target, index, count, params); }
inline void glNamedProgramLocalParameterI4iEXT(GLuint program, GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w) { fp_glNamedProgramLocalParameterI4iEXT(program, target, index, x, y, z, w); }
inline void glNamedProgramLocalParameterI4ivEXT(GLuint program, GLenum target, GLuint index, const GLint* params) { fp_glNamedProgramLocalParameterI4ivEXT(program, target, index, params); }
inline void glNamedProgramLocalParametersI4ivEXT(GLuint program, GLenum target, GLuint index, GLsizei count, const GLint* params) { fp_glNamedProgramLocalParametersI4ivEXT(program, target, index, count, params); }
inline void glNamedProgramLocalParameterI4uiEXT(GLuint program, GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) { fp_glNamedProgramLocalParameterI4uiEXT(program, target, index, x, y, z, w); }
inline void glNamedProgramLocalParameterI4uivEXT(GLuint program, GLenum target, GLuint index, const GLuint* params) { fp_glNamedProgramLocalParameterI4uivEXT(program, target, index, params); }
inline void glNamedProgramLocalParametersI4uivEXT(GLuint program, GLenum target, GLuint index, GLsizei count, const GLuint* params) { fp_glNamedProgramLocalParametersI4uivEXT(program, target, index, count, params); }
inline void glGetNamedProgramLocalParameterIivEXT(GLuint program, GLenum target, GLuint index, GLint* params) { fp_glGetNamedProgramLocalParameterIivEXT(program, target, index, params); }
inline void glGetNamedProgramLocalParameterIuivEXT(GLuint program, GLenum target, GLuint index, GLuint* params) { fp_glGetNamedProgramLocalParameterIuivEXT(program, target, index, params); }
inline void glEnableClientStateiEXT(GLenum array, GLuint index) { fp_glEnableClientStateiEXT(array, index); }
inline void glDisableClientStateiEXT(GLenum array, GLuint index) { fp_glDisableClientStateiEXT(array, index); }
inline void glGetFloati_vEXT(GLenum pname, GLuint index, GLfloat* params) { fp_glGetFloati_vEXT(pname, index, params); }
inline void glGetDoublei_vEXT(GLenum pname, GLuint index, GLdouble* params) { fp_glGetDoublei_vEXT(pname, index, params); }
inline void glGetPointeri_vEXT(GLenum pname, GLuint index, void** params) { fp_glGetPointeri_vEXT(pname, index, params); }
inline void glNamedProgramStringEXT(GLuint program, GLenum target, GLenum format, GLsizei len, const void* string) { fp_glNamedProgramStringEXT(program, target, format, len, string); }
inline void glNamedProgramLocalParameter4dEXT(GLuint program, GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) { fp_glNamedProgramLocalParameter4dEXT(program, target, index, x, y, z, w); }
inline void glNamedProgramLocalParameter4dvEXT(GLuint program, GLenum target, GLuint index, const GLdouble* params) { fp_glNamedProgramLocalParameter4dvEXT(program, target, index, params); }
inline void glNamedProgramLocalParameter4fEXT(GLuint program, GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) { fp_glNamedProgramLocalParameter4fEXT(program, target, index, x, y, z, w); }
inline void glNamedProgramLocalParameter4fvEXT(GLuint program, GLenum target, GLuint index, const GLfloat* params) { fp_glNamedProgramLocalParameter4fvEXT(program, target, index, params); }
inline void glGetNamedProgramLocalParameterdvEXT(GLuint program, GLenum target, GLuint index, GLdouble* params) { fp_glGetNamedProgramLocalParameterdvEXT(program, target, index, params); }
inline void glGetNamedProgramLocalParameterfvEXT(GLuint program, GLenum target, GLuint index, GLfloat* params) { fp_glGetNamedProgramLocalParameterfvEXT(program, target, index, params); }
inline void glGetNamedProgramivEXT(GLuint program, GLenum target, GLenum pname, GLint* params) { fp_glGetNamedProgramivEXT(program, target, pname, params); }
inline void glGetNamedProgramStringEXT(GLuint program, GLenum target, GLenum pname, void* string) { fp_glGetNamedProgramStringEXT(program, target, pname, string); }
inline void glNamedRenderbufferStorageEXT(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height) { fp_glNamedRenderbufferStorageEXT(renderbuffer, internalformat, width, height); }
inline void glGetNamedRenderbufferParameterivEXT(GLuint renderbuffer, GLenum pname, GLint* params) { fp_glGetNamedRenderbufferParameterivEXT(renderbuffer, pname, params); }
inline void glNamedRenderbufferStorageMultisampleEXT(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) { fp_glNamedRenderbufferStorageMultisampleEXT(renderbuffer, samples, internalformat, width, height); }
inline void glNamedRenderbufferStorageMultisampleCoverageEXT(GLuint renderbuffer, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height) { fp_glNamedRenderbufferStorageMultisampleCoverageEXT(renderbuffer, coverageSamples, colorSamples, internalformat, width, height); }
inline GLenum glCheckNamedFramebufferStatusEXT(GLuint framebuffer, GLenum target) { return fp_glCheckNamedFramebufferStatusEXT(framebuffer, target); }
inline void glNamedFramebufferTexture1DEXT(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level) { fp_glNamedFramebufferTexture1DEXT(framebuffer, attachment, textarget, texture, level); }
inline void glNamedFramebufferTexture2DEXT(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level) { fp_glNamedFramebufferTexture2DEXT(framebuffer, attachment, textarget, texture, level); }
inline void glNamedFramebufferTexture3DEXT(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) { fp_glNamedFramebufferTexture3DEXT(framebuffer, attachment, textarget, texture, level, zoffset); }
inline void glNamedFramebufferRenderbufferEXT(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) { fp_glNamedFramebufferRenderbufferEXT(framebuffer, attachment, renderbuffertarget, renderbuffer); }
inline void glGetNamedFramebufferAttachmentParameterivEXT(GLuint framebuffer, GLenum attachment, GLenum pname, GLint* params) { fp_glGetNamedFramebufferAttachmentParameterivEXT(framebuffer, attachment, pname, params); }
inline void glGenerateTextureMipmapEXT(GLuint texture, GLenum target) { fp_glGenerateTextureMipmapEXT(texture, target); }
inline void glGenerateMultiTexMipmapEXT(GLenum texunit, GLenum target) { fp_glGenerateMultiTexMipmapEXT(texunit, target); }
inline void glFramebufferDrawBufferEXT(GLuint framebuffer, GLenum mode) { fp_glFramebufferDrawBufferEXT(framebuffer, mode); }
inline void glFramebufferDrawBuffersEXT(GLuint framebuffer, GLsizei n, const GLenum* bufs) { fp_glFramebufferDrawBuffersEXT(framebuffer, n, bufs); }
inline void glFramebufferReadBufferEXT(GLuint framebuffer, GLenum mode) { fp_glFramebufferReadBufferEXT(framebuffer, mode); }
inline void glGetFramebufferParameterivEXT(GLuint framebuffer, GLenum pname, GLint* params) { fp_glGetFramebufferParameterivEXT(framebuffer, pname, params); }
inline void glNamedCopyBufferSubDataEXT(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) { fp_glNamedCopyBufferSubDataEXT(readBuffer, writeBuffer, readOffset, writeOffset, size); }
inline void glNamedFramebufferTextureEXT(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level) { fp_glNamedFramebufferTextureEXT(framebuffer, attachment, texture, level); }
inline void glNamedFramebufferTextureLayerEXT(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer) { fp_glNamedFramebufferTextureLayerEXT(framebuffer, attachment, texture, level, layer); }
inline void glNamedFramebufferTextureFaceEXT(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLenum face) { fp_glNamedFramebufferTextureFaceEXT(framebuffer, attachment, texture, level, face); }
inline void glTextureRenderbufferEXT(GLuint texture, GLenum target, GLuint renderbuffer) { fp_glTextureRenderbufferEXT(texture, target, renderbuffer); }
inline void glMultiTexRenderbufferEXT(GLenum texunit, GLenum target, GLuint renderbuffer) { fp_glMultiTexRenderbufferEXT(texunit, target, renderbuffer); }
inline void glVertexArrayVertexOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset) { fp_glVertexArrayVertexOffsetEXT(vaobj, buffer, size, type, stride, offset); }
inline void glVertexArrayColorOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset) { fp_glVertexArrayColorOffsetEXT(vaobj, buffer, size, type, stride, offset); }
inline void glVertexArrayEdgeFlagOffsetEXT(GLuint vaobj, GLuint buffer, GLsizei stride, GLintptr offset) { fp_glVertexArrayEdgeFlagOffsetEXT(vaobj, buffer, stride, offset); }
inline void glVertexArrayIndexOffsetEXT(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset) { fp_glVertexArrayIndexOffsetEXT(vaobj, buffer, type, stride, offset); }
inline void glVertexArrayNormalOffsetEXT(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset) { fp_glVertexArrayNormalOffsetEXT(vaobj, buffer, type, stride, offset); }
inline void glVertexArrayTexCoordOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset) { fp_glVertexArrayTexCoordOffsetEXT(vaobj, buffer, size, type, stride, offset); }
inline void glVertexArrayMultiTexCoordOffsetEXT(GLuint vaobj, GLuint buffer, GLenum texunit, GLint size, GLenum type, GLsizei stride, GLintptr offset) { fp_glVertexArrayMultiTexCoordOffsetEXT(vaobj, buffer, texunit, size, type, stride, offset); }
inline void glVertexArrayFogCoordOffsetEXT(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset) { fp_glVertexArrayFogCoordOffsetEXT(vaobj, buffer, type, stride, offset); }
inline void glVertexArraySecondaryColorOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset) { fp_glVertexArraySecondaryColorOffsetEXT(vaobj, buffer, size, type, stride, offset); }
inline void glVertexArrayVertexAttribOffsetEXT(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLintptr offset) { fp_glVertexArrayVertexAttribOffsetEXT(vaobj, buffer, index, size, type, normalized, stride, offset); }
inline void glVertexArrayVertexAttribIOffsetEXT(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset) { fp_glVertexArrayVertexAttribIOffsetEXT(vaobj, buffer, index, size, type, stride, offset); }
inline void glEnableVertexArrayEXT(GLuint vaobj, GLenum array) { fp_glEnableVertexArrayEXT(vaobj, array); }
inline void glDisableVertexArrayEXT(GLuint vaobj, GLenum array) { fp_glDisableVertexArrayEXT(vaobj, array); }
inline void glEnableVertexArrayAttribEXT(GLuint vaobj, GLuint index) { fp_glEnableVertexArrayAttribEXT(vaobj, index); }
inline void glDisableVertexArrayAttribEXT(GLuint vaobj, GLuint index) { fp_glDisableVertexArrayAttribEXT(vaobj, index); }
inline void glGetVertexArrayIntegervEXT(GLuint vaobj, GLenum pname, GLint* param) { fp_glGetVertexArrayIntegervEXT(vaobj, pname, param); }
inline void glGetVertexArrayPointervEXT(GLuint vaobj, GLenum pname, void** param) { fp_glGetVertexArrayPointervEXT(vaobj, pname, param); }
inline void glGetVertexArrayIntegeri_vEXT(GLuint vaobj, GLuint index, GLenum pname, GLint* param) { fp_glGetVertexArrayIntegeri_vEXT(vaobj, index, pname, param); }
inline void glGetVertexArrayPointeri_vEXT(GLuint vaobj, GLuint index, GLenum pname, void** param) { fp_glGetVertexArrayPointeri_vEXT(vaobj, index, pname, param); }
inline void* glMapNamedBufferRangeEXT(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access) { return fp_glMapNamedBufferRangeEXT(buffer, offset, length, access); }
inline void glFlushMappedNamedBufferRangeEXT(GLuint buffer, GLintptr offset, GLsizeiptr length) { fp_glFlushMappedNamedBufferRangeEXT(buffer, offset, length); }
inline void glNamedBufferStorageEXT(GLuint buffer, GLsizeiptr size, const void* data, GLbitfield flags) { fp_glNamedBufferStorageEXT(buffer, size, data, flags); }
inline void glClearNamedBufferDataEXT(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void* data) { fp_glClearNamedBufferDataEXT(buffer, internalformat, format, type, data); }
inline void glClearNamedBufferSubDataEXT(GLuint buffer, GLenum internalformat, GLsizeiptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data) { fp_glClearNamedBufferSubDataEXT(buffer, internalformat, offset, size, format, type, data); }
inline void glNamedFramebufferParameteriEXT(GLuint framebuffer, GLenum pname, GLint param) { fp_glNamedFramebufferParameteriEXT(framebuffer, pname, param); }
inline void glGetNamedFramebufferParameterivEXT(GLuint framebuffer, GLenum pname, GLint* params) { fp_glGetNamedFramebufferParameterivEXT(framebuffer, pname, params); }
inline void glProgramUniform1dEXT(GLuint program, GLint location, GLdouble x) { fp_glProgramUniform1dEXT(program, location, x); }
inline void glProgramUniform2dEXT(GLuint program, GLint location, GLdouble x, GLdouble y) { fp_glProgramUniform2dEXT(program, location, x, y); }
inline void glProgramUniform3dEXT(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z) { fp_glProgramUniform3dEXT(program, location, x, y, z); }
inline void glProgramUniform4dEXT(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w) { fp_glProgramUniform4dEXT(program, location, x, y, z, w); }
inline void glProgramUniform1dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble* value) { fp_glProgramUniform1dvEXT(program, location, count, value); }
inline void glProgramUniform2dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble* value) { fp_glProgramUniform2dvEXT(program, location, count, value); }
inline void glProgramUniform3dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble* value) { fp_glProgramUniform3dvEXT(program, location, count, value); }
inline void glProgramUniform4dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble* value) { fp_glProgramUniform4dvEXT(program, location, count, value); }
inline void glProgramUniformMatrix2dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix2dvEXT(program, location, count, transpose, value); }
inline void glProgramUniformMatrix3dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix3dvEXT(program, location, count, transpose, value); }
inline void glProgramUniformMatrix4dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix4dvEXT(program, location, count, transpose, value); }
inline void glProgramUniformMatrix2x3dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix2x3dvEXT(program, location, count, transpose, value); }
inline void glProgramUniformMatrix2x4dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix2x4dvEXT(program, location, count, transpose, value); }
inline void glProgramUniformMatrix3x2dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix3x2dvEXT(program, location, count, transpose, value); }
inline void glProgramUniformMatrix3x4dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix3x4dvEXT(program, location, count, transpose, value); }
inline void glProgramUniformMatrix4x2dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix4x2dvEXT(program, location, count, transpose, value); }
inline void glProgramUniformMatrix4x3dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) { fp_glProgramUniformMatrix4x3dvEXT(program, location, count, transpose, value); }
inline void glTextureBufferRangeEXT(GLuint texture, GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size) { fp_glTextureBufferRangeEXT(texture, target, internalformat, buffer, offset, size); }
inline void glTextureStorage1DEXT(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width) { fp_glTextureStorage1DEXT(texture, target, levels, internalformat, width); }
inline void glTextureStorage2DEXT(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) { fp_glTextureStorage2DEXT(texture, target, levels, internalformat, width, height); }
inline void glTextureStorage3DEXT(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) { fp_glTextureStorage3DEXT(texture, target, levels, internalformat, width, height, depth); }
inline void glTextureStorage2DMultisampleEXT(GLuint texture, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) { fp_glTextureStorage2DMultisampleEXT(texture, target, samples, internalformat, width, height, fixedsamplelocations); }
inline void glTextureStorage3DMultisampleEXT(GLuint texture, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) { fp_glTextureStorage3DMultisampleEXT(texture, target, samples, internalformat, width, height, depth, fixedsamplelocations); }
inline void glVertexArrayBindVertexBufferEXT(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) { fp_glVertexArrayBindVertexBufferEXT(vaobj, bindingindex, buffer, offset, stride); }
inline void glVertexArrayVertexAttribFormatEXT(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset) { fp_glVertexArrayVertexAttribFormatEXT(vaobj, attribindex, size, type, normalized, relativeoffset); }
inline void glVertexArrayVertexAttribIFormatEXT(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) { fp_glVertexArrayVertexAttribIFormatEXT(vaobj, attribindex, size, type, relativeoffset); }
inline void glVertexArrayVertexAttribLFormatEXT(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) { fp_glVertexArrayVertexAttribLFormatEXT(vaobj, attribindex, size, type, relativeoffset); }
inline void glVertexArrayVertexAttribBindingEXT(GLuint vaobj, GLuint attribindex, GLuint bindingindex) { fp_glVertexArrayVertexAttribBindingEXT(vaobj, attribindex, bindingindex); }
inline void glVertexArrayVertexBindingDivisorEXT(GLuint vaobj, GLuint bindingindex, GLuint divisor) { fp_glVertexArrayVertexBindingDivisorEXT(vaobj, bindingindex, divisor); }
inline void glVertexArrayVertexAttribLOffsetEXT(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset) { fp_glVertexArrayVertexAttribLOffsetEXT(vaobj, buffer, index, size, type, stride, offset); }
inline void glTexturePageCommitmentEXT(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit) { fp_glTexturePageCommitmentEXT(texture, level, xoffset, yoffset, zoffset, width, height, depth, commit); }
inline void glVertexArrayVertexAttribDivisorEXT(GLuint vaobj, GLuint index, GLuint divisor) { fp_glVertexArrayVertexAttribDivisorEXT(vaobj, index, divisor); }

/* GL_EXT_discard_framebuffer */
inline void glDiscardFramebufferEXT(GLenum target, GLsizei numAttachments, const GLenum* attachments) { fp_glDiscardFramebufferEXT(target, numAttachments, attachments); }

/* GL_EXT_disjoint_timer_query */
inline void glGenQueriesEXT(GLsizei n, GLuint* ids) { fp_glGenQueriesEXT(n, ids); }
inline void glDeleteQueriesEXT(GLsizei n, const GLuint* ids) { fp_glDeleteQueriesEXT(n, ids); }
inline GLboolean glIsQueryEXT(GLuint id) { return fp_glIsQueryEXT(id); }
inline void glBeginQueryEXT(GLenum target, GLuint id) { fp_glBeginQueryEXT(target, id); }
inline void glEndQueryEXT(GLenum target) { fp_glEndQueryEXT(target); }
inline void glQueryCounterEXT(GLuint id, GLenum target) { fp_glQueryCounterEXT(id, target); }
inline void glGetQueryivEXT(GLenum target, GLenum pname, GLint* params) { fp_glGetQueryivEXT(target, pname, params); }
inline void glGetQueryObjectivEXT(GLuint id, GLenum pname, GLint* params) { fp_glGetQueryObjectivEXT(id, pname, params); }
inline void glGetQueryObjectuivEXT(GLuint id, GLenum pname, GLuint* params) { fp_glGetQueryObjectuivEXT(id, pname, params); }
inline void glGetQueryObjecti64vEXT(GLuint id, GLenum pname, GLint64* params) { fp_glGetQueryObjecti64vEXT(id, pname, params); }
inline void glGetQueryObjectui64vEXT(GLuint id, GLenum pname, GLuint64* params) { fp_glGetQueryObjectui64vEXT(id, pname, params); }

/* GL_EXT_draw_buffers */
inline void glDrawBuffersEXT(GLsizei n, const GLenum* bufs) { fp_glDrawBuffersEXT(n, bufs); }

/* GL_EXT_draw_buffers2 */
inline void glColorMaskIndexedEXT(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a) { fp_glColorMaskIndexedEXT(index, r, g, b, a); }

/* GL_EXT_draw_buffers_indexed */
inline void glEnableiEXT(GLenum target, GLuint index) { fp_glEnableiEXT(target, index); }
inline void glDisableiEXT(GLenum target, GLuint index) { fp_glDisableiEXT(target, index); }
inline void glBlendEquationiEXT(GLuint buf, GLenum mode) { fp_glBlendEquationiEXT(buf, mode); }
inline void glBlendEquationSeparateiEXT(GLuint buf, GLenum modeRGB, GLenum modeAlpha) { fp_glBlendEquationSeparateiEXT(buf, modeRGB, modeAlpha); }
inline void glBlendFunciEXT(GLuint buf, GLenum src, GLenum dst) { fp_glBlendFunciEXT(buf, src, dst); }
inline void glBlendFuncSeparateiEXT(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) { fp_glBlendFuncSeparateiEXT(buf, srcRGB, dstRGB, srcAlpha, dstAlpha); }
inline void glColorMaskiEXT(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a) { fp_glColorMaskiEXT(index, r, g, b, a); }
inline GLboolean glIsEnablediEXT(GLenum target, GLuint index) { return fp_glIsEnablediEXT(target, index); }

/* GL_EXT_draw_elements_base_vertex */
inline void glDrawElementsBaseVertexEXT(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex) { fp_glDrawElementsBaseVertexEXT(mode, count, type, indices, basevertex); }
inline void glDrawRangeElementsBaseVertexEXT(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices, GLint basevertex) { fp_glDrawRangeElementsBaseVertexEXT(mode, start, end, count, type, indices, basevertex); }
inline void glDrawElementsInstancedBaseVertexEXT(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex) { fp_glDrawElementsInstancedBaseVertexEXT(mode, count, type, indices, instancecount, basevertex); }
inline void glMultiDrawElementsBaseVertexEXT(GLenum mode, const GLsizei* count, GLenum type, const void** indices, GLsizei primcount, const GLint* basevertex) { fp_glMultiDrawElementsBaseVertexEXT(mode, count, type, indices, primcount, basevertex); }

/* GL_EXT_draw_instanced */
inline void glDrawArraysInstancedEXT(GLenum mode, GLint start, GLsizei count, GLsizei primcount) { fp_glDrawArraysInstancedEXT(mode, start, count, primcount); }
inline void glDrawElementsInstancedEXT(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount) { fp_glDrawElementsInstancedEXT(mode, count, type, indices, primcount); }

/* GL_EXT_framebuffer_blit */
inline void glBlitFramebufferEXT(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) { fp_glBlitFramebufferEXT(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter); }

/* GL_EXT_framebuffer_multisample */
inline void glRenderbufferStorageMultisampleEXT(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) { fp_glRenderbufferStorageMultisampleEXT(target, samples, internalformat, width, height); }

/* GL_EXT_framebuffer_object */
inline GLboolean glIsRenderbufferEXT(GLuint renderbuffer) { return fp_glIsRenderbufferEXT(renderbuffer); }
inline void glBindRenderbufferEXT(GLenum target, GLuint renderbuffer) { fp_glBindRenderbufferEXT(target, renderbuffer); }
inline void glDeleteRenderbuffersEXT(GLsizei n, const GLuint* renderbuffers) { fp_glDeleteRenderbuffersEXT(n, renderbuffers); }
inline void glGenRenderbuffersEXT(GLsizei n, GLuint* renderbuffers) { fp_glGenRenderbuffersEXT(n, renderbuffers); }
inline void glRenderbufferStorageEXT(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) { fp_glRenderbufferStorageEXT(target, internalformat, width, height); }
inline void glGetRenderbufferParameterivEXT(GLenum target, GLenum pname, GLint* params) { fp_glGetRenderbufferParameterivEXT(target, pname, params); }
inline GLboolean glIsFramebufferEXT(GLuint framebuffer) { return fp_glIsFramebufferEXT(framebuffer); }
inline void glBindFramebufferEXT(GLenum target, GLuint framebuffer) { fp_glBindFramebufferEXT(target, framebuffer); }
inline void glDeleteFramebuffersEXT(GLsizei n, const GLuint* framebuffers) { fp_glDeleteFramebuffersEXT(n, framebuffers); }
inline void glGenFramebuffersEXT(GLsizei n, GLuint* framebuffers) { fp_glGenFramebuffersEXT(n, framebuffers); }
inline GLenum glCheckFramebufferStatusEXT(GLenum target) { return fp_glCheckFramebufferStatusEXT(target); }
inline void glFramebufferTexture1DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) { fp_glFramebufferTexture1DEXT(target, attachment, textarget, texture, level); }
inline void glFramebufferTexture2DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) { fp_glFramebufferTexture2DEXT(target, attachment, textarget, texture, level); }
inline void glFramebufferTexture3DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) { fp_glFramebufferTexture3DEXT(target, attachment, textarget, texture, level, zoffset); }
inline void glFramebufferRenderbufferEXT(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) { fp_glFramebufferRenderbufferEXT(target, attachment, renderbuffertarget, renderbuffer); }
inline void glGetFramebufferAttachmentParameterivEXT(GLenum target, GLenum attachment, GLenum pname, GLint* params) { fp_glGetFramebufferAttachmentParameterivEXT(target, attachment, pname, params); }
inline void glGenerateMipmapEXT(GLenum target) { fp_glGenerateMipmapEXT(target); }

/* GL_EXT_geometry_shader */
inline void glFramebufferTextureEXT(GLenum target, GLenum attachment, GLuint texture, GLint level) { fp_glFramebufferTextureEXT(target, attachment, texture, level); }

/* GL_EXT_geometry_shader4 */
inline void glProgramParameteriEXT(GLuint program, GLenum pname, GLint value) { fp_glProgramParameteriEXT(program, pname, value); }

/* GL_EXT_gpu_program_parameters */
inline void glProgramEnvParameters4fvEXT(GLenum target, GLuint index, GLsizei count, const GLfloat* params) { fp_glProgramEnvParameters4fvEXT(target, index, count, params); }
inline void glProgramLocalParameters4fvEXT(GLenum target, GLuint index, GLsizei count, const GLfloat* params) { fp_glProgramLocalParameters4fvEXT(target, index, count, params); }

/* GL_EXT_gpu_shader4 */
inline void glGetUniformuivEXT(GLuint program, GLint location, GLuint* params) { fp_glGetUniformuivEXT(program, location, params); }
inline GLint glGetFragDataLocationEXT(GLuint program, const GLchar* name) { return fp_glGetFragDataLocationEXT(program, name); }
inline void glUniform1uiEXT(GLint location, GLuint v0) { fp_glUniform1uiEXT(location, v0); }
inline void glUniform2uiEXT(GLint location, GLuint v0, GLuint v1) { fp_glUniform2uiEXT(location, v0, v1); }
inline void glUniform3uiEXT(GLint location, GLuint v0, GLuint v1, GLuint v2) { fp_glUniform3uiEXT(location, v0, v1, v2); }
inline void glUniform4uiEXT(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) { fp_glUniform4uiEXT(location, v0, v1, v2, v3); }
inline void glUniform1uivEXT(GLint location, GLsizei count, const GLuint* value) { fp_glUniform1uivEXT(location, count, value); }
inline void glUniform2uivEXT(GLint location, GLsizei count, const GLuint* value) { fp_glUniform2uivEXT(location, count, value); }
inline void glUniform3uivEXT(GLint location, GLsizei count, const GLuint* value) { fp_glUniform3uivEXT(location, count, value); }
inline void glUniform4uivEXT(GLint location, GLsizei count, const GLuint* value) { fp_glUniform4uivEXT(location, count, value); }

/* GL_EXT_instanced_arrays */
inline void glVertexAttribDivisorEXT(GLuint index, GLuint divisor) { fp_glVertexAttribDivisorEXT(index, divisor); }

/* GL_EXT_map_buffer_range */
inline void* glMapBufferRangeEXT(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) { return fp_glMapBufferRangeEXT(target, offset, length, access); }
inline void glFlushMappedBufferRangeEXT(GLenum target, GLintptr offset, GLsizeiptr length) { fp_glFlushMappedBufferRangeEXT(target, offset, length); }

/* GL_EXT_multi_draw_indirect */
inline void glMultiDrawArraysIndirectEXT(GLenum mode, const void* indirect, GLsizei drawcount, GLsizei stride) { fp_glMultiDrawArraysIndirectEXT(mode, indirect, drawcount, stride); }
inline void glMultiDrawElementsIndirectEXT(GLenum mode, GLenum type, const void* indirect, GLsizei drawcount, GLsizei stride) { fp_glMultiDrawElementsIndirectEXT(mode, type, indirect, drawcount, stride); }

/* GL_EXT_multisampled_render_to_texture */
inline void glFramebufferTexture2DMultisampleEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples) { fp_glFramebufferTexture2DMultisampleEXT(target, attachment, textarget, texture, level, samples); }

/* GL_EXT_multiview_draw_buffers */
inline void glReadBufferIndexedEXT(GLenum src, GLint index) { fp_glReadBufferIndexedEXT(src, index); }
inline void glDrawBuffersIndexedEXT(GLint n, const GLenum* location, const GLint* indices) { fp_glDrawBuffersIndexedEXT(n, location, indices); }
inline void glGetIntegeri_vEXT(GLenum target, GLuint index, GLint* data) { fp_glGetIntegeri_vEXT(target, index, data); }

/* GL_EXT_polygon_offset_clamp */
inline void glPolygonOffsetClampEXT(GLfloat factor, GLfloat units, GLfloat clamp) { fp_glPolygonOffsetClampEXT(factor, units, clamp); }

/* GL_EXT_primitive_bounding_box */
inline void glPrimitiveBoundingBoxEXT(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW) { fp_glPrimitiveBoundingBoxEXT(minX, minY, minZ, minW, maxX, maxY, maxZ, maxW); }

/* GL_EXT_provoking_vertex */
inline void glProvokingVertexEXT(GLenum mode) { fp_glProvokingVertexEXT(mode); }

/* GL_EXT_raster_multisample */
inline void glRasterSamplesEXT(GLuint samples, GLboolean fixedsamplelocations) { fp_glRasterSamplesEXT(samples, fixedsamplelocations); }

/* GL_EXT_robustness */
inline GLenum glGetGraphicsResetStatusEXT() { return fp_glGetGraphicsResetStatusEXT(); }
inline void glReadnPixelsEXT(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data) { fp_glReadnPixelsEXT(x, y, width, height, format, type, bufSize, data); }
inline void glGetnUniformfvEXT(GLuint program, GLint location, GLsizei bufSize, GLfloat* params) { fp_glGetnUniformfvEXT(program, location, bufSize, params); }
inline void glGetnUniformivEXT(GLuint program, GLint location, GLsizei bufSize, GLint* params) { fp_glGetnUniformivEXT(program, location, bufSize, params); }

/* GL_EXT_separate_shader_objects */
inline void glUseShaderProgramEXT(GLenum type, GLuint program) { fp_glUseShaderProgramEXT(type, program); }
inline void glActiveProgramEXT(GLuint program) { fp_glActiveProgramEXT(program); }
inline GLuint glCreateShaderProgramEXT(GLenum type, const GLchar* string) { return fp_glCreateShaderProgramEXT(type, string); }
inline void glActiveShaderProgramEXT(GLuint pipeline, GLuint program) { fp_glActiveShaderProgramEXT(pipeline, program); }
inline void glBindProgramPipelineEXT(GLuint pipeline) { fp_glBindProgramPipelineEXT(pipeline); }
inline GLuint glCreateShaderProgramvEXT(GLenum type, GLsizei count, const GLchar** strings) { return fp_glCreateShaderProgramvEXT(type, count, strings); }
inline void glDeleteProgramPipelinesEXT(GLsizei n, const GLuint* pipelines) { fp_glDeleteProgramPipelinesEXT(n, pipelines); }
inline void glGenProgramPipelinesEXT(GLsizei n, GLuint* pipelines) { fp_glGenProgramPipelinesEXT(n, pipelines); }
inline void glGetProgramPipelineInfoLogEXT(GLuint pipeline, GLsizei bufSize, GLsizei* length, GLchar* infoLog) { fp_glGetProgramPipelineInfoLogEXT(pipeline, bufSize, length, infoLog); }
inline void glGetProgramPipelineivEXT(GLuint pipeline, GLenum pname, GLint* params) { fp_glGetProgramPipelineivEXT(pipeline, pname, params); }
inline GLboolean glIsProgramPipelineEXT(GLuint pipeline) { return fp_glIsProgramPipelineEXT(pipeline); }
inline void glUseProgramStagesEXT(GLuint pipeline, GLbitfield stages, GLuint program) { fp_glUseProgramStagesEXT(pipeline, stages, program); }
inline void glValidateProgramPipelineEXT(GLuint pipeline) { fp_glValidateProgramPipelineEXT(pipeline); }

/* GL_EXT_shader_image_load_store */
inline void glBindImageTextureEXT(GLuint index, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLint format) { fp_glBindImageTextureEXT(index, texture, level, layered, layer, access, format); }
inline void glMemoryBarrierEXT(GLbitfield barriers) { fp_glMemoryBarrierEXT(barriers); }

/* GL_EXT_sparse_texture */
inline void glTexPageCommitmentEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit) { fp_glTexPageCommitmentEXT(target, level, xoffset, yoffset, zoffset, width, height, depth, commit); }

/* GL_EXT_stencil_clear_tag */
inline void glStencilClearTagEXT(GLsizei stencilTagBits, GLuint stencilClearTag) { fp_glStencilClearTagEXT(stencilTagBits, stencilClearTag); }

/* GL_EXT_tessellation_shader */
inline void glPatchParameteriEXT(GLenum pname, GLint value) { fp_glPatchParameteriEXT(pname, value); }

/* GL_EXT_texture_array */
inline void glFramebufferTextureLayerEXT(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) { fp_glFramebufferTextureLayerEXT(target, attachment, texture, level, layer); }

/* GL_EXT_texture_border_clamp */
inline void glTexParameterIivEXT(GLenum target, GLenum pname, const GLint* params) { fp_glTexParameterIivEXT(target, pname, params); }
inline void glTexParameterIuivEXT(GLenum target, GLenum pname, const GLuint* params) { fp_glTexParameterIuivEXT(target, pname, params); }
inline void glGetTexParameterIivEXT(GLenum target, GLenum pname, GLint* params) { fp_glGetTexParameterIivEXT(target, pname, params); }
inline void glGetTexParameterIuivEXT(GLenum target, GLenum pname, GLuint* params) { fp_glGetTexParameterIuivEXT(target, pname, params); }
inline void glSamplerParameterIivEXT(GLuint sampler, GLenum pname, const GLint* param) { fp_glSamplerParameterIivEXT(sampler, pname, param); }
inline void glSamplerParameterIuivEXT(GLuint sampler, GLenum pname, const GLuint* param) { fp_glSamplerParameterIuivEXT(sampler, pname, param); }
inline void glGetSamplerParameterIivEXT(GLuint sampler, GLenum pname, GLint* params) { fp_glGetSamplerParameterIivEXT(sampler, pname, params); }
inline void glGetSamplerParameterIuivEXT(GLuint sampler, GLenum pname, GLuint* params) { fp_glGetSamplerParameterIuivEXT(sampler, pname, params); }

/* GL_EXT_texture_buffer */
inline void glTexBufferEXT(GLenum target, GLenum internalformat, GLuint buffer) { fp_glTexBufferEXT(target, internalformat, buffer); }
inline void glTexBufferRangeEXT(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size) { fp_glTexBufferRangeEXT(target, internalformat, buffer, offset, size); }

/* GL_EXT_texture_integer */
inline void glClearColorIiEXT(GLint red, GLint green, GLint blue, GLint alpha) { fp_glClearColorIiEXT(red, green, blue, alpha); }
inline void glClearColorIuiEXT(GLuint red, GLuint green, GLuint blue, GLuint alpha) { fp_glClearColorIuiEXT(red, green, blue, alpha); }

/* GL_EXT_texture_storage */
inline void glTexStorage1DEXT(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width) { fp_glTexStorage1DEXT(target, levels, internalformat, width); }
inline void glTexStorage2DEXT(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) { fp_glTexStorage2DEXT(target, levels, internalformat, width, height); }
inline void glTexStorage3DEXT(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) { fp_glTexStorage3DEXT(target, levels, internalformat, width, height, depth); }

/* GL_EXT_texture_view */
inline void glTextureViewEXT(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers) { fp_glTextureViewEXT(texture, target, origtexture, internalformat, minlevel, numlevels, minlayer, numlayers); }

/* GL_EXT_transform_feedback */
inline void glBeginTransformFeedbackEXT(GLenum primitiveMode) { fp_glBeginTransformFeedbackEXT(primitiveMode); }
inline void glEndTransformFeedbackEXT() { fp_glEndTransformFeedbackEXT(); }
inline void glBindBufferRangeEXT(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) { fp_glBindBufferRangeEXT(target, index, buffer, offset, size); }
inline void glBindBufferOffsetEXT(GLenum target, GLuint index, GLuint buffer, GLintptr offset) { fp_glBindBufferOffsetEXT(target, index, buffer, offset); }
inline void glBindBufferBaseEXT(GLenum target, GLuint index, GLuint buffer) { fp_glBindBufferBaseEXT(target, index, buffer); }
inline void glTransformFeedbackVaryingsEXT(GLuint program, GLsizei count, const GLchar** varyings, GLenum bufferMode) { fp_glTransformFeedbackVaryingsEXT(program, count, varyings, bufferMode); }
inline void glGetTransformFeedbackVaryingEXT(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name) { fp_glGetTransformFeedbackVaryingEXT(program, index, bufSize, length, size, type, name); }

/* GL_EXT_vertex_attrib_64bit */
inline void glVertexAttribL1dEXT(GLuint index, GLdouble x) { fp_glVertexAttribL1dEXT(index, x); }
inline void glVertexAttribL2dEXT(GLuint index, GLdouble x, GLdouble y) { fp_glVertexAttribL2dEXT(index, x, y); }
inline void glVertexAttribL3dEXT(GLuint index, GLdouble x, GLdouble y, GLdouble z) { fp_glVertexAttribL3dEXT(index, x, y, z); }
inline void glVertexAttribL4dEXT(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) { fp_glVertexAttribL4dEXT(index, x, y, z, w); }
inline void glVertexAttribL1dvEXT(GLuint index, const GLdouble* v) { fp_glVertexAttribL1dvEXT(index, v); }
inline void glVertexAttribL2dvEXT(GLuint index, const GLdouble* v) { fp_glVertexAttribL2dvEXT(index, v); }
inline void glVertexAttribL3dvEXT(GLuint index, const GLdouble* v) { fp_glVertexAttribL3dvEXT(index, v); }
inline void glVertexAttribL4dvEXT(GLuint index, const GLdouble* v) { fp_glVertexAttribL4dvEXT(index, v); }
inline void glVertexAttribLPointerEXT(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) { fp_glVertexAttribLPointerEXT(index, size, type, stride, pointer); }
inline void glGetVertexAttribLdvEXT(GLuint index, GLenum pname, GLdouble* params) { fp_glGetVertexAttribLdvEXT(index, pname, params); }

/* GL_EXT_x11_sync_object */
inline GLsync glImportSyncEXT(GLenum external_sync_type, GLintptr external_sync, GLbitfield flags) { return fp_glImportSyncEXT(external_sync_type, external_sync, flags); }

/* GL_OES_copy_image */
inline void glCopyImageSubDataOES(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth) { fp_glCopyImageSubDataOES(srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth); }

/* GL_OES_draw_buffers_indexed */
inline void glEnableiOES(GLenum target, GLuint index) { fp_glEnableiOES(target, index); }
inline void glDisableiOES(GLenum target, GLuint index) { fp_glDisableiOES(target, index); }
inline void glBlendEquationiOES(GLuint buf, GLenum mode) { fp_glBlendEquationiOES(buf, mode); }
inline void glBlendEquationSeparateiOES(GLuint buf, GLenum modeRGB, GLenum modeAlpha) { fp_glBlendEquationSeparateiOES(buf, modeRGB, modeAlpha); }
inline void glBlendFunciOES(GLuint buf, GLenum src, GLenum dst) { fp_glBlendFunciOES(buf, src, dst); }
inline void glBlendFuncSeparateiOES(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) { fp_glBlendFuncSeparateiOES(buf, srcRGB, dstRGB, srcAlpha, dstAlpha); }
inline void glColorMaskiOES(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a) { fp_glColorMaskiOES(index, r, g, b, a); }
inline GLboolean glIsEnablediOES(GLenum target, GLuint index) { return fp_glIsEnablediOES(target, index); }

/* GL_OES_draw_elements_base_vertex */
inline void glDrawElementsBaseVertexOES(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex) { fp_glDrawElementsBaseVertexOES(mode, count, type, indices, basevertex); }
inline void glDrawRangeElementsBaseVertexOES(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices, GLint basevertex) { fp_glDrawRangeElementsBaseVertexOES(mode, start, end, count, type, indices, basevertex); }
inline void glDrawElementsInstancedBaseVertexOES(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex) { fp_glDrawElementsInstancedBaseVertexOES(mode, count, type, indices, instancecount, basevertex); }
inline void glMultiDrawElementsBaseVertexOES(GLenum mode, const GLsizei* count, GLenum type, const void** indices, GLsizei primcount, const GLint* basevertex) { fp_glMultiDrawElementsBaseVertexOES(mode, count, type, indices, primcount, basevertex); }

/* GL_OES_EGL_image */
inline void glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image) { fp_glEGLImageTargetTexture2DOES(target, image); }
inline void glEGLImageTargetRenderbufferStorageOES(GLenum target, GLeglImageOES image) { fp_glEGLImageTargetRenderbufferStorageOES(target, image); }

/* GL_OES_geometry_shader */
inline void glFramebufferTextureOES(GLenum target, GLenum attachment, GLuint texture, GLint level) { fp_glFramebufferTextureOES(target, attachment, texture, level); }

/* GL_OES_get_program_binary */
inline void glGetProgramBinaryOES(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, void* binary) { fp_glGetProgramBinaryOES(program, bufSize, length, binaryFormat, binary); }
inline void glProgramBinaryOES(GLuint program, GLenum binaryFormat, const void* binary, GLint length) { fp_glProgramBinaryOES(program, binaryFormat, binary, length); }

/* GL_OES_mapbuffer */
inline void* glMapBufferOES(GLenum target, GLenum access) { return fp_glMapBufferOES(target, access); }
inline GLboolean glUnmapBufferOES(GLenum target) { return fp_glUnmapBufferOES(target); }
inline void glGetBufferPointervOES(GLenum target, GLenum pname, void** params) { fp_glGetBufferPointervOES(target, pname, params); }

/* GL_OES_primitive_bounding_box */
inline void glPrimitiveBoundingBoxOES(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW) { fp_glPrimitiveBoundingBoxOES(minX, minY, minZ, minW, maxX, maxY, maxZ, maxW); }

/* GL_OES_sample_shading */
inline void glMinSampleShadingOES(GLfloat value) { fp_glMinSampleShadingOES(value); }

/* GL_OES_tessellation_shader */
inline void glPatchParameteriOES(GLenum pname, GLint value) { fp_glPatchParameteriOES(pname, value); }

/* GL_OES_texture_3D */
inline void glTexImage3DOES(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) { fp_glTexImage3DOES(target, level, internalformat, width, height, depth, border, format, type, pixels); }
inline void glTexSubImage3DOES(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) { fp_glTexSubImage3DOES(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels); }
inline void glCopyTexSubImage3DOES(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) { fp_glCopyTexSubImage3DOES(target, level, xoffset, yoffset, zoffset, x, y, width, height); }
inline void glCompressedTexImage3DOES(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data) { fp_glCompressedTexImage3DOES(target, level, internalformat, width, height, depth, border, imageSize, data); }
inline void glCompressedTexSubImage3DOES(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data) { fp_glCompressedTexSubImage3DOES(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data); }
inline void glFramebufferTexture3DOES(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) { fp_glFramebufferTexture3DOES(target, attachment, textarget, texture, level, zoffset); }

/* GL_OES_texture_border_clamp */
inline void glTexParameterIivOES(GLenum target, GLenum pname, const GLint* params) { fp_glTexParameterIivOES(target, pname, params); }
inline void glTexParameterIuivOES(GLenum target, GLenum pname, const GLuint* params) { fp_glTexParameterIuivOES(target, pname, params); }
inline void glGetTexParameterIivOES(GLenum target, GLenum pname, GLint* params) { fp_glGetTexParameterIivOES(target, pname, params); }
inline void glGetTexParameterIuivOES(GLenum target, GLenum pname, GLuint* params) { fp_glGetTexParameterIuivOES(target, pname, params); }
inline void glSamplerParameterIivOES(GLuint sampler, GLenum pname, const GLint* param) { fp_glSamplerParameterIivOES(sampler, pname, param); }
inline void glSamplerParameterIuivOES(GLuint sampler, GLenum pname, const GLuint* param) { fp_glSamplerParameterIuivOES(sampler, pname, param); }
inline void glGetSamplerParameterIivOES(GLuint sampler, GLenum pname, GLint* params) { fp_glGetSamplerParameterIivOES(sampler, pname, params); }
inline void glGetSamplerParameterIuivOES(GLuint sampler, GLenum pname, GLuint* params) { fp_glGetSamplerParameterIuivOES(sampler, pname, params); }

/* GL_OES_texture_buffer */
inline void glTexBufferOES(GLenum target, GLenum internalformat, GLuint buffer) { fp_glTexBufferOES(target, internalformat, buffer); }
inline void glTexBufferRangeOES(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size) { fp_glTexBufferRangeOES(target, internalformat, buffer, offset, size); }

/* GL_OES_texture_storage_multisample_2d_array */
inline void glTexStorage3DMultisampleOES(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) { fp_glTexStorage3DMultisampleOES(target, samples, internalformat, width, height, depth, fixedsamplelocations); }

/* GL_OES_texture_view */
inline void glTextureViewOES(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers) { fp_glTextureViewOES(texture, target, origtexture, internalformat, minlevel, numlevels, minlayer, numlayers); }

/* GL_OES_vertex_array_object */
inline void glBindVertexArrayOES(GLuint array) { fp_glBindVertexArrayOES(array); }
inline void glDeleteVertexArraysOES(GLsizei n, const GLuint* arrays) { fp_glDeleteVertexArraysOES(n, arrays); }
inline void glGenVertexArraysOES(GLsizei n, GLuint* arrays) { fp_glGenVertexArraysOES(n, arrays); }
inline GLboolean glIsVertexArrayOES(GLuint array) { return fp_glIsVertexArrayOES(array); }

/* GL_AMD_debug_output */
inline void glDebugMessageEnableAMD(GLenum category, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled) { fp_glDebugMessageEnableAMD(category, severity, count, ids, enabled); }
inline void glDebugMessageInsertAMD(GLenum category, GLenum severity, GLuint id, GLsizei length, const GLchar* buf) { fp_glDebugMessageInsertAMD(category, severity, id, length, buf); }
inline void glDebugMessageCallbackAMD(GLDEBUGPROCAMD callback, void* userParam) { fp_glDebugMessageCallbackAMD(callback, userParam); }
inline GLuint glGetDebugMessageLogAMD(GLuint count, GLsizei bufsize, GLenum* categories, GLuint* severities, GLuint* ids, GLsizei* lengths, GLchar* message) { return fp_glGetDebugMessageLogAMD(count, bufsize, categories, severities, ids, lengths, message); }

/* GL_AMD_draw_buffers_blend */
inline void glBlendFuncIndexedAMD(GLuint buf, GLenum src, GLenum dst) { fp_glBlendFuncIndexedAMD(buf, src, dst); }
inline void glBlendFuncSeparateIndexedAMD(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) { fp_glBlendFuncSeparateIndexedAMD(buf, srcRGB, dstRGB, srcAlpha, dstAlpha); }
inline void glBlendEquationIndexedAMD(GLuint buf, GLenum mode) { fp_glBlendEquationIndexedAMD(buf, mode); }
inline void glBlendEquationSeparateIndexedAMD(GLuint buf, GLenum modeRGB, GLenum modeAlpha) { fp_glBlendEquationSeparateIndexedAMD(buf, modeRGB, modeAlpha); }

/* GL_AMD_gpu_shader_int64 */
inline void glUniform1i64NV(GLint location, GLint64EXT x) { fp_glUniform1i64NV(location, x); }
inline void glUniform2i64NV(GLint location, GLint64EXT x, GLint64EXT y) { fp_glUniform2i64NV(location, x, y); }
inline void glUniform3i64NV(GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z) { fp_glUniform3i64NV(location, x, y, z); }
inline void glUniform4i64NV(GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w) { fp_glUniform4i64NV(location, x, y, z, w); }
inline void glUniform1i64vNV(GLint location, GLsizei count, const GLint64EXT* value) { fp_glUniform1i64vNV(location, count, value); }
inline void glUniform2i64vNV(GLint location, GLsizei count, const GLint64EXT* value) { fp_glUniform2i64vNV(location, count, value); }
inline void glUniform3i64vNV(GLint location, GLsizei count, const GLint64EXT* value) { fp_glUniform3i64vNV(location, count, value); }
inline void glUniform4i64vNV(GLint location, GLsizei count, const GLint64EXT* value) { fp_glUniform4i64vNV(location, count, value); }
inline void glUniform1ui64NV(GLint location, GLuint64EXT x) { fp_glUniform1ui64NV(location, x); }
inline void glUniform2ui64NV(GLint location, GLuint64EXT x, GLuint64EXT y) { fp_glUniform2ui64NV(location, x, y); }
inline void glUniform3ui64NV(GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z) { fp_glUniform3ui64NV(location, x, y, z); }
inline void glUniform4ui64NV(GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w) { fp_glUniform4ui64NV(location, x, y, z, w); }
inline void glUniform1ui64vNV(GLint location, GLsizei count, const GLuint64EXT* value) { fp_glUniform1ui64vNV(location, count, value); }
inline void glUniform2ui64vNV(GLint location, GLsizei count, const GLuint64EXT* value) { fp_glUniform2ui64vNV(location, count, value); }
inline void glUniform3ui64vNV(GLint location, GLsizei count, const GLuint64EXT* value) { fp_glUniform3ui64vNV(location, count, value); }
inline void glUniform4ui64vNV(GLint location, GLsizei count, const GLuint64EXT* value) { fp_glUniform4ui64vNV(location, count, value); }
inline void glGetUniformi64vNV(GLuint program, GLint location, GLint64EXT* params) { fp_glGetUniformi64vNV(program, location, params); }
inline void glGetUniformui64vNV(GLuint program, GLint location, GLuint64EXT* params) { fp_glGetUniformui64vNV(program, location, params); }
inline void glProgramUniform1i64NV(GLuint program, GLint location, GLint64EXT x) { fp_glProgramUniform1i64NV(program, location, x); }
inline void glProgramUniform2i64NV(GLuint program, GLint location, GLint64EXT x, GLint64EXT y) { fp_glProgramUniform2i64NV(program, location, x, y); }
inline void glProgramUniform3i64NV(GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z) { fp_glProgramUniform3i64NV(program, location, x, y, z); }
inline void glProgramUniform4i64NV(GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w) { fp_glProgramUniform4i64NV(program, location, x, y, z, w); }
inline void glProgramUniform1i64vNV(GLuint program, GLint location, GLsizei count, const GLint64EXT* value) { fp_glProgramUniform1i64vNV(program, location, count, value); }
inline void glProgramUniform2i64vNV(GLuint program, GLint location, GLsizei count, const GLint64EXT* value) { fp_glProgramUniform2i64vNV(program, location, count, value); }
inline void glProgramUniform3i64vNV(GLuint program, GLint location, GLsizei count, const GLint64EXT* value) { fp_glProgramUniform3i64vNV(program, location, count, value); }
inline void glProgramUniform4i64vNV(GLuint program, GLint location, GLsizei count, const GLint64EXT* value) { fp_glProgramUniform4i64vNV(program, location, count, value); }
inline void glProgramUniform1ui64NV(GLuint program, GLint location, GLuint64EXT x) { fp_glProgramUniform1ui64NV(program, location, x); }
inline void glProgramUniform2ui64NV(GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y) { fp_glProgramUniform2ui64NV(program, location, x, y); }
inline void glProgramUniform3ui64NV(GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z) { fp_glProgramUniform3ui64NV(program, location, x, y, z); }
inline void glProgramUniform4ui64NV(GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w) { fp_glProgramUniform4ui64NV(program, location, x, y, z, w); }
inline void glProgramUniform1ui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value) { fp_glProgramUniform1ui64vNV(program, location, count, value); }
inline void glProgramUniform2ui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value) { fp_glProgramUniform2ui64vNV(program, location, count, value); }
inline void glProgramUniform3ui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value) { fp_glProgramUniform3ui64vNV(program, location, count, value); }
inline void glProgramUniform4ui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value) { fp_glProgramUniform4ui64vNV(program, location, count, value); }

/* GL_AMD_interleaved_elements */
inline void glVertexAttribParameteriAMD(GLuint index, GLenum pname, GLint param) { fp_glVertexAttribParameteriAMD(index, pname, param); }

/* GL_AMD_multi_draw_indirect */
inline void glMultiDrawArraysIndirectAMD(GLenum mode, const void* indirect, GLsizei primcount, GLsizei stride) { fp_glMultiDrawArraysIndirectAMD(mode, indirect, primcount, stride); }
inline void glMultiDrawElementsIndirectAMD(GLenum mode, GLenum type, const void* indirect, GLsizei primcount, GLsizei stride) { fp_glMultiDrawElementsIndirectAMD(mode, type, indirect, primcount, stride); }

/* GL_AMD_name_gen_delete */
inline void glGenNamesAMD(GLenum identifier, GLuint num, GLuint* names) { fp_glGenNamesAMD(identifier, num, names); }
inline void glDeleteNamesAMD(GLenum identifier, GLuint num, const GLuint* names) { fp_glDeleteNamesAMD(identifier, num, names); }
inline GLboolean glIsNameAMD(GLenum identifier, GLuint name) { return fp_glIsNameAMD(identifier, name); }

/* GL_AMD_occlusion_query_event */
inline void glQueryObjectParameteruiAMD(GLenum target, GLuint id, GLenum pname, GLuint param) { fp_glQueryObjectParameteruiAMD(target, id, pname, param); }

/* GL_AMD_performance_monitor */
inline void glGetPerfMonitorGroupsAMD(GLint* numGroups, GLsizei groupsSize, GLuint* groups) { fp_glGetPerfMonitorGroupsAMD(numGroups, groupsSize, groups); }
inline void glGetPerfMonitorCountersAMD(GLuint group, GLint* numCounters, GLint* maxActiveCounters, GLsizei counterSize, GLuint* counters) { fp_glGetPerfMonitorCountersAMD(group, numCounters, maxActiveCounters, counterSize, counters); }
inline void glGetPerfMonitorGroupStringAMD(GLuint group, GLsizei bufSize, GLsizei* length, GLchar* groupString) { fp_glGetPerfMonitorGroupStringAMD(group, bufSize, length, groupString); }
inline void glGetPerfMonitorCounterStringAMD(GLuint group, GLuint counter, GLsizei bufSize, GLsizei* length, GLchar* counterString) { fp_glGetPerfMonitorCounterStringAMD(group, counter, bufSize, length, counterString); }
inline void glGetPerfMonitorCounterInfoAMD(GLuint group, GLuint counter, GLenum pname, void* data) { fp_glGetPerfMonitorCounterInfoAMD(group, counter, pname, data); }
inline void glGenPerfMonitorsAMD(GLsizei n, GLuint* monitors) { fp_glGenPerfMonitorsAMD(n, monitors); }
inline void glDeletePerfMonitorsAMD(GLsizei n, GLuint* monitors) { fp_glDeletePerfMonitorsAMD(n, monitors); }
inline void glSelectPerfMonitorCountersAMD(GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint* counterList) { fp_glSelectPerfMonitorCountersAMD(monitor, enable, group, numCounters, counterList); }
inline void glBeginPerfMonitorAMD(GLuint monitor) { fp_glBeginPerfMonitorAMD(monitor); }
inline void glEndPerfMonitorAMD(GLuint monitor) { fp_glEndPerfMonitorAMD(monitor); }
inline void glGetPerfMonitorCounterDataAMD(GLuint monitor, GLenum pname, GLsizei dataSize, GLuint* data, GLint* bytesWritten) { fp_glGetPerfMonitorCounterDataAMD(monitor, pname, dataSize, data, bytesWritten); }

/* GL_AMD_sample_positions */
inline void glSetMultisamplefvAMD(GLenum pname, GLuint index, const GLfloat* val) { fp_glSetMultisamplefvAMD(pname, index, val); }

/* GL_AMD_sparse_texture */
inline void glTexStorageSparseAMD(GLenum target, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLsizei layers, GLbitfield flags) { fp_glTexStorageSparseAMD(target, internalFormat, width, height, depth, layers, flags); }
inline void glTextureStorageSparseAMD(GLuint texture, GLenum target, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLsizei layers, GLbitfield flags) { fp_glTextureStorageSparseAMD(texture, target, internalFormat, width, height, depth, layers, flags); }

/* GL_AMD_stencil_operation_extended */
inline void glStencilOpValueAMD(GLenum face, GLuint value) { fp_glStencilOpValueAMD(face, value); }

/* GL_AMD_vertex_shader_tessellator */
inline void glTessellationFactorAMD(GLfloat factor) { fp_glTessellationFactorAMD(factor); }
inline void glTessellationModeAMD(GLenum mode) { fp_glTessellationModeAMD(mode); }

/* GL_ANGLE_framebuffer_blit */
inline void glBlitFramebufferANGLE(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) { fp_glBlitFramebufferANGLE(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter); }

/* GL_ANGLE_framebuffer_multisample */
inline void glRenderbufferStorageMultisampleANGLE(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) { fp_glRenderbufferStorageMultisampleANGLE(target, samples, internalformat, width, height); }

/* GL_ANGLE_instanced_arrays */
inline void glDrawArraysInstancedANGLE(GLenum mode, GLint first, GLsizei count, GLsizei primcount) { fp_glDrawArraysInstancedANGLE(mode, first, count, primcount); }
inline void glDrawElementsInstancedANGLE(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount) { fp_glDrawElementsInstancedANGLE(mode, count, type, indices, primcount); }
inline void glVertexAttribDivisorANGLE(GLuint index, GLuint divisor) { fp_glVertexAttribDivisorANGLE(index, divisor); }

/* GL_ANGLE_translated_shader_source */
inline void glGetTranslatedShaderSourceANGLE(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source) { fp_glGetTranslatedShaderSourceANGLE(shader, bufsize, length, source); }

/* GL_APPLE_copy_texture_levels */
inline void glCopyTextureLevelsAPPLE(GLuint destinationTexture, GLuint sourceTexture, GLint sourceBaseLevel, GLsizei sourceLevelCount) { fp_glCopyTextureLevelsAPPLE(destinationTexture, sourceTexture, sourceBaseLevel, sourceLevelCount); }

/* GL_APPLE_element_array */
inline void glElementPointerAPPLE(GLenum type, const void* pointer) { fp_glElementPointerAPPLE(type, pointer); }
inline void glDrawElementArrayAPPLE(GLenum mode, GLint first, GLsizei count) { fp_glDrawElementArrayAPPLE(mode, first, count); }
inline void glDrawRangeElementArrayAPPLE(GLenum mode, GLuint start, GLuint end, GLint first, GLsizei count) { fp_glDrawRangeElementArrayAPPLE(mode, start, end, first, count); }
inline void glMultiDrawElementArrayAPPLE(GLenum mode, const GLint* first, const GLsizei* count, GLsizei primcount) { fp_glMultiDrawElementArrayAPPLE(mode, first, count, primcount); }
inline void glMultiDrawRangeElementArrayAPPLE(GLenum mode, GLuint start, GLuint end, const GLint* first, const GLsizei* count, GLsizei primcount) { fp_glMultiDrawRangeElementArrayAPPLE(mode, start, end, first, count, primcount); }

/* GL_APPLE_fence */
inline void glGenFencesAPPLE(GLsizei n, GLuint* fences) { fp_glGenFencesAPPLE(n, fences); }
inline void glDeleteFencesAPPLE(GLsizei n, const GLuint* fences) { fp_glDeleteFencesAPPLE(n, fences); }
inline void glSetFenceAPPLE(GLuint fence) { fp_glSetFenceAPPLE(fence); }
inline GLboolean glIsFenceAPPLE(GLuint fence) { return fp_glIsFenceAPPLE(fence); }
inline GLboolean glTestFenceAPPLE(GLuint fence) { return fp_glTestFenceAPPLE(fence); }
inline void glFinishFenceAPPLE(GLuint fence) { fp_glFinishFenceAPPLE(fence); }
inline GLboolean glTestObjectAPPLE(GLenum object, GLuint name) { return fp_glTestObjectAPPLE(object, name); }
inline void glFinishObjectAPPLE(GLenum object, GLint name) { fp_glFinishObjectAPPLE(object, name); }

/* GL_APPLE_flush_buffer_range */
inline void glBufferParameteriAPPLE(GLenum target, GLenum pname, GLint param) { fp_glBufferParameteriAPPLE(target, pname, param); }
inline void glFlushMappedBufferRangeAPPLE(GLenum target, GLintptr offset, GLsizeiptr size) { fp_glFlushMappedBufferRangeAPPLE(target, offset, size); }

/* GL_APPLE_framebuffer_multisample */
inline void glRenderbufferStorageMultisampleAPPLE(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) { fp_glRenderbufferStorageMultisampleAPPLE(target, samples, internalformat, width, height); }
inline void glResolveMultisampleFramebufferAPPLE() { fp_glResolveMultisampleFramebufferAPPLE(); }

/* GL_APPLE_object_purgeable */
inline GLenum glObjectPurgeableAPPLE(GLenum objectType, GLuint name, GLenum option) { return fp_glObjectPurgeableAPPLE(objectType, name, option); }
inline GLenum glObjectUnpurgeableAPPLE(GLenum objectType, GLuint name, GLenum option) { return fp_glObjectUnpurgeableAPPLE(objectType, name, option); }
inline void glGetObjectParameterivAPPLE(GLenum objectType, GLuint name, GLenum pname, GLint* params) { fp_glGetObjectParameterivAPPLE(objectType, name, pname, params); }

/* GL_APPLE_sync */
inline GLsync glFenceSyncAPPLE(GLenum condition, GLbitfield flags) { return fp_glFenceSyncAPPLE(condition, flags); }
inline GLboolean glIsSyncAPPLE(GLsync sync) { return fp_glIsSyncAPPLE(sync); }
inline void glDeleteSyncAPPLE(GLsync sync) { fp_glDeleteSyncAPPLE(sync); }
inline GLenum glClientWaitSyncAPPLE(GLsync sync, GLbitfield flags, GLuint64 timeout) { return fp_glClientWaitSyncAPPLE(sync, flags, timeout); }
inline void glWaitSyncAPPLE(GLsync sync, GLbitfield flags, GLuint64 timeout) { fp_glWaitSyncAPPLE(sync, flags, timeout); }
inline void glGetInteger64vAPPLE(GLenum pname, GLint64* params) { fp_glGetInteger64vAPPLE(pname, params); }
inline void glGetSyncivAPPLE(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values) { fp_glGetSyncivAPPLE(sync, pname, bufSize, length, values); }

/* GL_APPLE_texture_range */
inline void glTextureRangeAPPLE(GLenum target, GLsizei length, const void* pointer) { fp_glTextureRangeAPPLE(target, length, pointer); }
inline void glGetTexParameterPointervAPPLE(GLenum target, GLenum pname, void** params) { fp_glGetTexParameterPointervAPPLE(target, pname, params); }

/* GL_APPLE_vertex_array_object */
inline void glBindVertexArrayAPPLE(GLuint array) { fp_glBindVertexArrayAPPLE(array); }
inline void glDeleteVertexArraysAPPLE(GLsizei n, const GLuint* arrays) { fp_glDeleteVertexArraysAPPLE(n, arrays); }
inline void glGenVertexArraysAPPLE(GLsizei n, GLuint* arrays) { fp_glGenVertexArraysAPPLE(n, arrays); }
inline GLboolean glIsVertexArrayAPPLE(GLuint array) { return fp_glIsVertexArrayAPPLE(array); }

/* GL_APPLE_vertex_array_range */
inline void glVertexArrayRangeAPPLE(GLsizei length, void* pointer) { fp_glVertexArrayRangeAPPLE(length, pointer); }
inline void glFlushVertexArrayRangeAPPLE(GLsizei length, void* pointer) { fp_glFlushVertexArrayRangeAPPLE(length, pointer); }
inline void glVertexArrayParameteriAPPLE(GLenum pname, GLint param) { fp_glVertexArrayParameteriAPPLE(pname, param); }

/* GL_APPLE_vertex_program_evaluators */
inline void glEnableVertexAttribAPPLE(GLuint index, GLenum pname) { fp_glEnableVertexAttribAPPLE(index, pname); }
inline void glDisableVertexAttribAPPLE(GLuint index, GLenum pname) { fp_glDisableVertexAttribAPPLE(index, pname); }
inline GLboolean glIsVertexAttribEnabledAPPLE(GLuint index, GLenum pname) { return fp_glIsVertexAttribEnabledAPPLE(index, pname); }
inline void glMapVertexAttrib1dAPPLE(GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble* points) { fp_glMapVertexAttrib1dAPPLE(index, size, u1, u2, stride, order, points); }
inline void glMapVertexAttrib1fAPPLE(GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat* points) { fp_glMapVertexAttrib1fAPPLE(index, size, u1, u2, stride, order, points); }
inline void glMapVertexAttrib2dAPPLE(GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble* points) { fp_glMapVertexAttrib2dAPPLE(index, size, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points); }
inline void glMapVertexAttrib2fAPPLE(GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat* points) { fp_glMapVertexAttrib2fAPPLE(index, size, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points); }

/* GL_GREMEDY_frame_terminator */
inline void glFrameTerminatorGREMEDY() { fp_glFrameTerminatorGREMEDY(); }

/* GL_GREMEDY_string_marker */
inline void glStringMarkerGREMEDY(GLsizei len, const void* string) { fp_glStringMarkerGREMEDY(len, string); }

/* GL_IMG_multisampled_render_to_texture */
inline void glRenderbufferStorageMultisampleIMG(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) { fp_glRenderbufferStorageMultisampleIMG(target, samples, internalformat, width, height); }
inline void glFramebufferTexture2DMultisampleIMG(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples) { fp_glFramebufferTexture2DMultisampleIMG(target, attachment, textarget, texture, level, samples); }

/* GL_INGR_blend_func_separate */
inline void glBlendFuncSeparateINGR(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) { fp_glBlendFuncSeparateINGR(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha); }

/* GL_INTEL_framebuffer_CMAA */
inline void glApplyFramebufferAttachmentCMAAINTEL() { fp_glApplyFramebufferAttachmentCMAAINTEL(); }

/* GL_INTEL_map_texture */
inline void glSyncTextureINTEL(GLuint texture) { fp_glSyncTextureINTEL(texture); }
inline void glUnmapTexture2DINTEL(GLuint texture, GLint level) { fp_glUnmapTexture2DINTEL(texture, level); }
inline void* glMapTexture2DINTEL(GLuint texture, GLint level, GLbitfield access, GLint* stride, GLenum* layout) { return fp_glMapTexture2DINTEL(texture, level, access, stride, layout); }

/* GL_INTEL_performance_query */
inline void glBeginPerfQueryINTEL(GLuint queryHandle) { fp_glBeginPerfQueryINTEL(queryHandle); }
inline void glCreatePerfQueryINTEL(GLuint queryId, GLuint* queryHandle) { fp_glCreatePerfQueryINTEL(queryId, queryHandle); }
inline void glDeletePerfQueryINTEL(GLuint queryHandle) { fp_glDeletePerfQueryINTEL(queryHandle); }
inline void glEndPerfQueryINTEL(GLuint queryHandle) { fp_glEndPerfQueryINTEL(queryHandle); }
inline void glGetFirstPerfQueryIdINTEL(GLuint* queryId) { fp_glGetFirstPerfQueryIdINTEL(queryId); }
inline void glGetNextPerfQueryIdINTEL(GLuint queryId, GLuint* nextQueryId) { fp_glGetNextPerfQueryIdINTEL(queryId, nextQueryId); }
inline void glGetPerfCounterInfoINTEL(GLuint queryId, GLuint counterId, GLuint counterNameLength, GLchar* counterName, GLuint counterDescLength, GLchar* counterDesc, GLuint* counterOffset, GLuint* counterDataSize, GLuint* counterTypeEnum, GLuint* counterDataTypeEnum, GLuint64* rawCounterMaxValue) { fp_glGetPerfCounterInfoINTEL(queryId, counterId, counterNameLength, counterName, counterDescLength, counterDesc, counterOffset, counterDataSize, counterTypeEnum, counterDataTypeEnum, rawCounterMaxValue); }
inline void glGetPerfQueryDataINTEL(GLuint queryHandle, GLuint flags, GLsizei dataSize, GLvoid* data, GLuint* bytesWritten) { fp_glGetPerfQueryDataINTEL(queryHandle, flags, dataSize, data, bytesWritten); }
inline void glGetPerfQueryIdByNameINTEL(GLchar* queryName, GLuint* queryId) { fp_glGetPerfQueryIdByNameINTEL(queryName, queryId); }
inline void glGetPerfQueryInfoINTEL(GLuint queryId, GLuint queryNameLength, GLchar* queryName, GLuint* dataSize, GLuint* noCounters, GLuint* noInstances, GLuint* capsMask) { fp_glGetPerfQueryInfoINTEL(queryId, queryNameLength, queryName, dataSize, noCounters, noInstances, capsMask); }

/* GL_NV_bindless_multi_draw_indirect */
inline void glMultiDrawArraysIndirectBindlessNV(GLenum mode, const void* indirect, GLsizei drawCount, GLsizei stride, GLint vertexBufferCount) { fp_glMultiDrawArraysIndirectBindlessNV(mode, indirect, drawCount, stride, vertexBufferCount); }
inline void glMultiDrawElementsIndirectBindlessNV(GLenum mode, GLenum type, const void* indirect, GLsizei drawCount, GLsizei stride, GLint vertexBufferCount) { fp_glMultiDrawElementsIndirectBindlessNV(mode, type, indirect, drawCount, stride, vertexBufferCount); }

/* GL_NV_bindless_multi_draw_indirect_count */
inline void glMultiDrawArraysIndirectBindlessCountNV(GLenum mode, const void* indirect, GLsizei drawCount, GLsizei maxDrawCount, GLsizei stride, GLint vertexBufferCount) { fp_glMultiDrawArraysIndirectBindlessCountNV(mode, indirect, drawCount, maxDrawCount, stride, vertexBufferCount); }
inline void glMultiDrawElementsIndirectBindlessCountNV(GLenum mode, GLenum type, const void* indirect, GLsizei drawCount, GLsizei maxDrawCount, GLsizei stride, GLint vertexBufferCount) { fp_glMultiDrawElementsIndirectBindlessCountNV(mode, type, indirect, drawCount, maxDrawCount, stride, vertexBufferCount); }

/* GL_NV_bindless_texture */
inline GLuint64 glGetTextureHandleNV(GLuint texture) { return fp_glGetTextureHandleNV(texture); }
inline GLuint64 glGetTextureSamplerHandleNV(GLuint texture, GLuint sampler) { return fp_glGetTextureSamplerHandleNV(texture, sampler); }
inline void glMakeTextureHandleResidentNV(GLuint64 handle) { fp_glMakeTextureHandleResidentNV(handle); }
inline void glMakeTextureHandleNonResidentNV(GLuint64 handle) { fp_glMakeTextureHandleNonResidentNV(handle); }
inline GLuint64 glGetImageHandleNV(GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format) { return fp_glGetImageHandleNV(texture, level, layered, layer, format); }
inline void glMakeImageHandleResidentNV(GLuint64 handle, GLenum access) { fp_glMakeImageHandleResidentNV(handle, access); }
inline void glMakeImageHandleNonResidentNV(GLuint64 handle) { fp_glMakeImageHandleNonResidentNV(handle); }
inline void glUniformHandleui64NV(GLint location, GLuint64 value) { fp_glUniformHandleui64NV(location, value); }
inline void glUniformHandleui64vNV(GLint location, GLsizei count, const GLuint64* value) { fp_glUniformHandleui64vNV(location, count, value); }
inline void glProgramUniformHandleui64NV(GLuint program, GLint location, GLuint64 value) { fp_glProgramUniformHandleui64NV(program, location, value); }
inline void glProgramUniformHandleui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64* values) { fp_glProgramUniformHandleui64vNV(program, location, count, values); }
inline GLboolean glIsTextureHandleResidentNV(GLuint64 handle) { return fp_glIsTextureHandleResidentNV(handle); }
inline GLboolean glIsImageHandleResidentNV(GLuint64 handle) { return fp_glIsImageHandleResidentNV(handle); }

/* GL_NV_blend_equation_advanced */
inline void glBlendParameteriNV(GLenum pname, GLint value) { fp_glBlendParameteriNV(pname, value); }
inline void glBlendBarrierNV() { fp_glBlendBarrierNV(); }

/* GL_NV_command_list */
inline void glCreateStatesNV(GLsizei n, GLuint* states) { fp_glCreateStatesNV(n, states); }
inline void glDeleteStatesNV(GLsizei n, const GLuint* states) { fp_glDeleteStatesNV(n, states); }
inline GLboolean glIsStateNV(GLuint state) { return fp_glIsStateNV(state); }
inline void glStateCaptureNV(GLuint state, GLenum mode) { fp_glStateCaptureNV(state, mode); }
inline GLuint glGetCommandHeaderNV(GLenum tokenID, GLuint size) { return fp_glGetCommandHeaderNV(tokenID, size); }
inline GLushort glGetStageIndexNV(GLenum shadertype) { return fp_glGetStageIndexNV(shadertype); }
inline void glDrawCommandsNV(GLenum primitiveMode, GLuint buffer, const GLintptr* indirects, const GLsizei* sizes, GLuint count) { fp_glDrawCommandsNV(primitiveMode, buffer, indirects, sizes, count); }
inline void glDrawCommandsAddressNV(GLenum primitiveMode, const GLuint64* indirects, const GLsizei* sizes, GLuint count) { fp_glDrawCommandsAddressNV(primitiveMode, indirects, sizes, count); }
inline void glDrawCommandsStatesNV(GLuint buffer, const GLintptr* indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count) { fp_glDrawCommandsStatesNV(buffer, indirects, sizes, states, fbos, count); }
inline void glDrawCommandsStatesAddressNV(const GLuint64* indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count) { fp_glDrawCommandsStatesAddressNV(indirects, sizes, states, fbos, count); }
inline void glCreateCommandListsNV(GLsizei n, GLuint* lists) { fp_glCreateCommandListsNV(n, lists); }
inline void glDeleteCommandListsNV(GLsizei n, const GLuint* lists) { fp_glDeleteCommandListsNV(n, lists); }
inline GLboolean glIsCommandListNV(GLuint list) { return fp_glIsCommandListNV(list); }
inline void glListDrawCommandsStatesClientNV(GLuint list, GLuint segment, const void** indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count) { fp_glListDrawCommandsStatesClientNV(list, segment, indirects, sizes, states, fbos, count); }
inline void glCommandListSegmentsNV(GLuint list, GLuint segments) { fp_glCommandListSegmentsNV(list, segments); }
inline void glCompileCommandListNV(GLuint list) { fp_glCompileCommandListNV(list); }
inline void glCallCommandListNV(GLuint list) { fp_glCallCommandListNV(list); }

/* GL_NV_conditional_render */
inline void glBeginConditionalRenderNV(GLuint id, GLenum mode) { fp_glBeginConditionalRenderNV(id, mode); }
inline void glEndConditionalRenderNV() { fp_glEndConditionalRenderNV(); }

/* GL_NV_conservative_raster */
inline void glSubpixelPrecisionBiasNV(GLuint xbits, GLuint ybits) { fp_glSubpixelPrecisionBiasNV(xbits, ybits); }

/* GL_NV_conservative_raster_dilate */
inline void glConservativeRasterParameterfNV(GLenum pname, GLfloat value) { fp_glConservativeRasterParameterfNV(pname, value); }

/* GL_NV_copy_buffer */
inline void glCopyBufferSubDataNV(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) { fp_glCopyBufferSubDataNV(readTarget, writeTarget, readOffset, writeOffset, size); }

/* GL_NV_copy_image */
inline void glCopyImageSubDataNV(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei width, GLsizei height, GLsizei depth) { fp_glCopyImageSubDataNV(srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, width, height, depth); }

/* GL_NV_coverage_sample */
inline void glCoverageMaskNV(GLboolean mask) { fp_glCoverageMaskNV(mask); }
inline void glCoverageOperationNV(GLenum operation) { fp_glCoverageOperationNV(operation); }

/* GL_NV_depth_buffer_float */
inline void glDepthRangedNV(GLdouble zNear, GLdouble zFar) { fp_glDepthRangedNV(zNear, zFar); }
inline void glClearDepthdNV(GLdouble depth) { fp_glClearDepthdNV(depth); }
inline void glDepthBoundsdNV(GLdouble zmin, GLdouble zmax) { fp_glDepthBoundsdNV(zmin, zmax); }

/* GL_NV_draw_buffers */
inline void glDrawBuffersNV(GLsizei n, const GLenum* bufs) { fp_glDrawBuffersNV(n, bufs); }

/* GL_NV_draw_instanced */
inline void glDrawArraysInstancedNV(GLenum mode, GLint first, GLsizei count, GLsizei primcount) { fp_glDrawArraysInstancedNV(mode, first, count, primcount); }
inline void glDrawElementsInstancedNV(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount) { fp_glDrawElementsInstancedNV(mode, count, type, indices, primcount); }

/* GL_NV_draw_texture */
inline void glDrawTextureNV(GLuint texture, GLuint sampler, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLfloat z, GLfloat s0, GLfloat t0, GLfloat s1, GLfloat t1) { fp_glDrawTextureNV(texture, sampler, x0, y0, x1, y1, z, s0, t0, s1, t1); }

/* GL_NV_explicit_multisample */
inline void glGetMultisamplefvNV(GLenum pname, GLuint index, GLfloat* val) { fp_glGetMultisamplefvNV(pname, index, val); }
inline void glSampleMaskIndexedNV(GLuint index, GLbitfield mask) { fp_glSampleMaskIndexedNV(index, mask); }
inline void glTexRenderbufferNV(GLenum target, GLuint renderbuffer) { fp_glTexRenderbufferNV(target, renderbuffer); }

/* GL_NV_fence */
inline void glDeleteFencesNV(GLsizei n, const GLuint* fences) { fp_glDeleteFencesNV(n, fences); }
inline void glGenFencesNV(GLsizei n, GLuint* fences) { fp_glGenFencesNV(n, fences); }
inline GLboolean glIsFenceNV(GLuint fence) { return fp_glIsFenceNV(fence); }
inline GLboolean glTestFenceNV(GLuint fence) { return fp_glTestFenceNV(fence); }
inline void glGetFenceivNV(GLuint fence, GLenum pname, GLint* params) { fp_glGetFenceivNV(fence, pname, params); }
inline void glFinishFenceNV(GLuint fence) { fp_glFinishFenceNV(fence); }
inline void glSetFenceNV(GLuint fence, GLenum condition) { fp_glSetFenceNV(fence, condition); }

/* GL_NV_fragment_coverage_to_color */
inline void glFragmentCoverageColorNV(GLuint color) { fp_glFragmentCoverageColorNV(color); }

/* GL_NV_framebuffer_blit */
inline void glBlitFramebufferNV(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) { fp_glBlitFramebufferNV(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter); }

/* GL_NV_framebuffer_mixed_samples */
inline void glCoverageModulationTableNV(GLsizei n, const GLfloat* v) { fp_glCoverageModulationTableNV(n, v); }
inline void glGetCoverageModulationTableNV(GLsizei bufsize, GLfloat* v) { fp_glGetCoverageModulationTableNV(bufsize, v); }
inline void glCoverageModulationNV(GLenum components) { fp_glCoverageModulationNV(components); }

/* GL_NV_framebuffer_multisample */
inline void glRenderbufferStorageMultisampleNV(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) { fp_glRenderbufferStorageMultisampleNV(target, samples, internalformat, width, height); }

/* GL_NV_framebuffer_multisample_coverage */
inline void glRenderbufferStorageMultisampleCoverageNV(GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height) { fp_glRenderbufferStorageMultisampleCoverageNV(target, coverageSamples, colorSamples, internalformat, width, height); }

/* GL_NV_geometry_program4 */
inline void glProgramVertexLimitNV(GLenum target, GLint limit) { fp_glProgramVertexLimitNV(target, limit); }
inline void glFramebufferTextureFaceEXT(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face) { fp_glFramebufferTextureFaceEXT(target, attachment, texture, level, face); }

/* GL_NV_gpu_program4 */
inline void glProgramLocalParameterI4iNV(GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w) { fp_glProgramLocalParameterI4iNV(target, index, x, y, z, w); }
inline void glProgramLocalParameterI4ivNV(GLenum target, GLuint index, const GLint* params) { fp_glProgramLocalParameterI4ivNV(target, index, params); }
inline void glProgramLocalParametersI4ivNV(GLenum target, GLuint index, GLsizei count, const GLint* params) { fp_glProgramLocalParametersI4ivNV(target, index, count, params); }
inline void glProgramLocalParameterI4uiNV(GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) { fp_glProgramLocalParameterI4uiNV(target, index, x, y, z, w); }
inline void glProgramLocalParameterI4uivNV(GLenum target, GLuint index, const GLuint* params) { fp_glProgramLocalParameterI4uivNV(target, index, params); }
inline void glProgramLocalParametersI4uivNV(GLenum target, GLuint index, GLsizei count, const GLuint* params) { fp_glProgramLocalParametersI4uivNV(target, index, count, params); }
inline void glProgramEnvParameterI4iNV(GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w) { fp_glProgramEnvParameterI4iNV(target, index, x, y, z, w); }
inline void glProgramEnvParameterI4ivNV(GLenum target, GLuint index, const GLint* params) { fp_glProgramEnvParameterI4ivNV(target, index, params); }
inline void glProgramEnvParametersI4ivNV(GLenum target, GLuint index, GLsizei count, const GLint* params) { fp_glProgramEnvParametersI4ivNV(target, index, count, params); }
inline void glProgramEnvParameterI4uiNV(GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) { fp_glProgramEnvParameterI4uiNV(target, index, x, y, z, w); }
inline void glProgramEnvParameterI4uivNV(GLenum target, GLuint index, const GLuint* params) { fp_glProgramEnvParameterI4uivNV(target, index, params); }
inline void glProgramEnvParametersI4uivNV(GLenum target, GLuint index, GLsizei count, const GLuint* params) { fp_glProgramEnvParametersI4uivNV(target, index, count, params); }
inline void glGetProgramLocalParameterIivNV(GLenum target, GLuint index, GLint* params) { fp_glGetProgramLocalParameterIivNV(target, index, params); }
inline void glGetProgramLocalParameterIuivNV(GLenum target, GLuint index, GLuint* params) { fp_glGetProgramLocalParameterIuivNV(target, index, params); }
inline void glGetProgramEnvParameterIivNV(GLenum target, GLuint index, GLint* params) { fp_glGetProgramEnvParameterIivNV(target, index, params); }
inline void glGetProgramEnvParameterIuivNV(GLenum target, GLuint index, GLuint* params) { fp_glGetProgramEnvParameterIuivNV(target, index, params); }

/* GL_NV_gpu_program5 */
inline void glProgramSubroutineParametersuivNV(GLenum target, GLsizei count, const GLuint* params) { fp_glProgramSubroutineParametersuivNV(target, count, params); }
inline void glGetProgramSubroutineParameteruivNV(GLenum target, GLuint index, GLuint* param) { fp_glGetProgramSubroutineParameteruivNV(target, index, param); }

/* GL_NV_half_float */
inline void glVertex2hNV(GLhalfNV x, GLhalfNV y) { fp_glVertex2hNV(x, y); }
inline void glVertex2hvNV(const GLhalfNV* v) { fp_glVertex2hvNV(v); }
inline void glVertex3hNV(GLhalfNV x, GLhalfNV y, GLhalfNV z) { fp_glVertex3hNV(x, y, z); }
inline void glVertex3hvNV(const GLhalfNV* v) { fp_glVertex3hvNV(v); }
inline void glVertex4hNV(GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w) { fp_glVertex4hNV(x, y, z, w); }
inline void glVertex4hvNV(const GLhalfNV* v) { fp_glVertex4hvNV(v); }
inline void glNormal3hNV(GLhalfNV nx, GLhalfNV ny, GLhalfNV nz) { fp_glNormal3hNV(nx, ny, nz); }
inline void glNormal3hvNV(const GLhalfNV* v) { fp_glNormal3hvNV(v); }
inline void glColor3hNV(GLhalfNV red, GLhalfNV green, GLhalfNV blue) { fp_glColor3hNV(red, green, blue); }
inline void glColor3hvNV(const GLhalfNV* v) { fp_glColor3hvNV(v); }
inline void glColor4hNV(GLhalfNV red, GLhalfNV green, GLhalfNV blue, GLhalfNV alpha) { fp_glColor4hNV(red, green, blue, alpha); }
inline void glColor4hvNV(const GLhalfNV* v) { fp_glColor4hvNV(v); }
inline void glTexCoord1hNV(GLhalfNV s) { fp_glTexCoord1hNV(s); }
inline void glTexCoord1hvNV(const GLhalfNV* v) { fp_glTexCoord1hvNV(v); }
inline void glTexCoord2hNV(GLhalfNV s, GLhalfNV t) { fp_glTexCoord2hNV(s, t); }
inline void glTexCoord2hvNV(const GLhalfNV* v) { fp_glTexCoord2hvNV(v); }
inline void glTexCoord3hNV(GLhalfNV s, GLhalfNV t, GLhalfNV r) { fp_glTexCoord3hNV(s, t, r); }
inline void glTexCoord3hvNV(const GLhalfNV* v) { fp_glTexCoord3hvNV(v); }
inline void glTexCoord4hNV(GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q) { fp_glTexCoord4hNV(s, t, r, q); }
inline void glTexCoord4hvNV(const GLhalfNV* v) { fp_glTexCoord4hvNV(v); }
inline void glMultiTexCoord1hNV(GLenum target, GLhalfNV s) { fp_glMultiTexCoord1hNV(target, s); }
inline void glMultiTexCoord1hvNV(GLenum target, const GLhalfNV* v) { fp_glMultiTexCoord1hvNV(target, v); }
inline void glMultiTexCoord2hNV(GLenum target, GLhalfNV s, GLhalfNV t) { fp_glMultiTexCoord2hNV(target, s, t); }
inline void glMultiTexCoord2hvNV(GLenum target, const GLhalfNV* v) { fp_glMultiTexCoord2hvNV(target, v); }
inline void glMultiTexCoord3hNV(GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r) { fp_glMultiTexCoord3hNV(target, s, t, r); }
inline void glMultiTexCoord3hvNV(GLenum target, const GLhalfNV* v) { fp_glMultiTexCoord3hvNV(target, v); }
inline void glMultiTexCoord4hNV(GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q) { fp_glMultiTexCoord4hNV(target, s, t, r, q); }
inline void glMultiTexCoord4hvNV(GLenum target, const GLhalfNV* v) { fp_glMultiTexCoord4hvNV(target, v); }
inline void glFogCoordhNV(GLhalfNV fog) { fp_glFogCoordhNV(fog); }
inline void glFogCoordhvNV(const GLhalfNV* fog) { fp_glFogCoordhvNV(fog); }
inline void glSecondaryColor3hNV(GLhalfNV red, GLhalfNV green, GLhalfNV blue) { fp_glSecondaryColor3hNV(red, green, blue); }
inline void glSecondaryColor3hvNV(const GLhalfNV* v) { fp_glSecondaryColor3hvNV(v); }
inline void glVertexWeighthNV(GLhalfNV weight) { fp_glVertexWeighthNV(weight); }
inline void glVertexWeighthvNV(const GLhalfNV* weight) { fp_glVertexWeighthvNV(weight); }
inline void glVertexAttrib1hNV(GLuint index, GLhalfNV x) { fp_glVertexAttrib1hNV(index, x); }
inline void glVertexAttrib1hvNV(GLuint index, const GLhalfNV* v) { fp_glVertexAttrib1hvNV(index, v); }
inline void glVertexAttrib2hNV(GLuint index, GLhalfNV x, GLhalfNV y) { fp_glVertexAttrib2hNV(index, x, y); }
inline void glVertexAttrib2hvNV(GLuint index, const GLhalfNV* v) { fp_glVertexAttrib2hvNV(index, v); }
inline void glVertexAttrib3hNV(GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z) { fp_glVertexAttrib3hNV(index, x, y, z); }
inline void glVertexAttrib3hvNV(GLuint index, const GLhalfNV* v) { fp_glVertexAttrib3hvNV(index, v); }
inline void glVertexAttrib4hNV(GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w) { fp_glVertexAttrib4hNV(index, x, y, z, w); }
inline void glVertexAttrib4hvNV(GLuint index, const GLhalfNV* v) { fp_glVertexAttrib4hvNV(index, v); }
inline void glVertexAttribs1hvNV(GLuint index, GLsizei n, const GLhalfNV* v) { fp_glVertexAttribs1hvNV(index, n, v); }
inline void glVertexAttribs2hvNV(GLuint index, GLsizei n, const GLhalfNV* v) { fp_glVertexAttribs2hvNV(index, n, v); }
inline void glVertexAttribs3hvNV(GLuint index, GLsizei n, const GLhalfNV* v) { fp_glVertexAttribs3hvNV(index, n, v); }
inline void glVertexAttribs4hvNV(GLuint index, GLsizei n, const GLhalfNV* v) { fp_glVertexAttribs4hvNV(index, n, v); }

/* GL_NV_instanced_arrays */
inline void glVertexAttribDivisorNV(GLuint index, GLuint divisor) { fp_glVertexAttribDivisorNV(index, divisor); }

/* GL_NV_internalformat_sample_query */
inline void glGetInternalformatSampleivNV(GLenum target, GLenum internalformat, GLsizei samples, GLenum pname, GLsizei bufSize, GLint* params) { fp_glGetInternalformatSampleivNV(target, internalformat, samples, pname, bufSize, params); }

/* GL_NV_non_square_matrices */
inline void glUniformMatrix2x3fvNV(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glUniformMatrix2x3fvNV(location, count, transpose, value); }
inline void glUniformMatrix3x2fvNV(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glUniformMatrix3x2fvNV(location, count, transpose, value); }
inline void glUniformMatrix2x4fvNV(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glUniformMatrix2x4fvNV(location, count, transpose, value); }
inline void glUniformMatrix4x2fvNV(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glUniformMatrix4x2fvNV(location, count, transpose, value); }
inline void glUniformMatrix3x4fvNV(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glUniformMatrix3x4fvNV(location, count, transpose, value); }
inline void glUniformMatrix4x3fvNV(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { fp_glUniformMatrix4x3fvNV(location, count, transpose, value); }

/* GL_NV_occlusion_query */
inline void glGenOcclusionQueriesNV(GLsizei n, GLuint* ids) { fp_glGenOcclusionQueriesNV(n, ids); }
inline void glDeleteOcclusionQueriesNV(GLsizei n, const GLuint* ids) { fp_glDeleteOcclusionQueriesNV(n, ids); }
inline GLboolean glIsOcclusionQueryNV(GLuint id) { return fp_glIsOcclusionQueryNV(id); }
inline void glBeginOcclusionQueryNV(GLuint id) { fp_glBeginOcclusionQueryNV(id); }
inline void glEndOcclusionQueryNV() { fp_glEndOcclusionQueryNV(); }
inline void glGetOcclusionQueryivNV(GLuint id, GLenum pname, GLint* params) { fp_glGetOcclusionQueryivNV(id, pname, params); }
inline void glGetOcclusionQueryuivNV(GLuint id, GLenum pname, GLuint* params) { fp_glGetOcclusionQueryuivNV(id, pname, params); }

/* GL_NV_parameter_buffer_object */
inline void glProgramBufferParametersfvNV(GLenum target, GLuint bindingIndex, GLuint wordIndex, GLsizei count, const GLfloat* params) { fp_glProgramBufferParametersfvNV(target, bindingIndex, wordIndex, count, params); }
inline void glProgramBufferParametersIivNV(GLenum target, GLuint bindingIndex, GLuint wordIndex, GLsizei count, const GLint* params) { fp_glProgramBufferParametersIivNV(target, bindingIndex, wordIndex, count, params); }
inline void glProgramBufferParametersIuivNV(GLenum target, GLuint bindingIndex, GLuint wordIndex, GLsizei count, const GLuint* params) { fp_glProgramBufferParametersIuivNV(target, bindingIndex, wordIndex, count, params); }

/* GL_NV_path_rendering */
inline GLuint glGenPathsNV(GLsizei range) { return fp_glGenPathsNV(range); }
inline void glDeletePathsNV(GLuint path, GLsizei range) { fp_glDeletePathsNV(path, range); }
inline GLboolean glIsPathNV(GLuint path) { return fp_glIsPathNV(path); }
inline void glPathCommandsNV(GLuint path, GLsizei numCommands, const GLubyte* commands, GLsizei numCoords, GLenum coordType, const void* coords) { fp_glPathCommandsNV(path, numCommands, commands, numCoords, coordType, coords); }
inline void glPathCoordsNV(GLuint path, GLsizei numCoords, GLenum coordType, const void* coords) { fp_glPathCoordsNV(path, numCoords, coordType, coords); }
inline void glPathSubCommandsNV(GLuint path, GLsizei commandStart, GLsizei commandsToDelete, GLsizei numCommands, const GLubyte* commands, GLsizei numCoords, GLenum coordType, const void* coords) { fp_glPathSubCommandsNV(path, commandStart, commandsToDelete, numCommands, commands, numCoords, coordType, coords); }
inline void glPathSubCoordsNV(GLuint path, GLsizei coordStart, GLsizei numCoords, GLenum coordType, const void* coords) { fp_glPathSubCoordsNV(path, coordStart, numCoords, coordType, coords); }
inline void glPathStringNV(GLuint path, GLenum format, GLsizei length, const void* pathString) { fp_glPathStringNV(path, format, length, pathString); }
inline void glPathGlyphsNV(GLuint firstPathName, GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLsizei numGlyphs, GLenum type, const void* charcodes, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale) { fp_glPathGlyphsNV(firstPathName, fontTarget, fontName, fontStyle, numGlyphs, type, charcodes, handleMissingGlyphs, pathParameterTemplate, emScale); }
inline void glPathGlyphRangeNV(GLuint firstPathName, GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLuint firstGlyph, GLsizei numGlyphs, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale) { fp_glPathGlyphRangeNV(firstPathName, fontTarget, fontName, fontStyle, firstGlyph, numGlyphs, handleMissingGlyphs, pathParameterTemplate, emScale); }
inline void glWeightPathsNV(GLuint resultPath, GLsizei numPaths, const GLuint* paths, const GLfloat* weights) { fp_glWeightPathsNV(resultPath, numPaths, paths, weights); }
inline void glCopyPathNV(GLuint resultPath, GLuint srcPath) { fp_glCopyPathNV(resultPath, srcPath); }
inline void glInterpolatePathsNV(GLuint resultPath, GLuint pathA, GLuint pathB, GLfloat weight) { fp_glInterpolatePathsNV(resultPath, pathA, pathB, weight); }
inline void glTransformPathNV(GLuint resultPath, GLuint srcPath, GLenum transformType, const GLfloat* transformValues) { fp_glTransformPathNV(resultPath, srcPath, transformType, transformValues); }
inline void glPathParameterivNV(GLuint path, GLenum pname, const GLint* value) { fp_glPathParameterivNV(path, pname, value); }
inline void glPathParameteriNV(GLuint path, GLenum pname, GLint value) { fp_glPathParameteriNV(path, pname, value); }
inline void glPathParameterfvNV(GLuint path, GLenum pname, const GLfloat* value) { fp_glPathParameterfvNV(path, pname, value); }
inline void glPathParameterfNV(GLuint path, GLenum pname, GLfloat value) { fp_glPathParameterfNV(path, pname, value); }
inline void glPathDashArrayNV(GLuint path, GLsizei dashCount, const GLfloat* dashArray) { fp_glPathDashArrayNV(path, dashCount, dashArray); }
inline void glPathStencilFuncNV(GLenum func, GLint ref, GLuint mask) { fp_glPathStencilFuncNV(func, ref, mask); }
inline void glPathStencilDepthOffsetNV(GLfloat factor, GLfloat units) { fp_glPathStencilDepthOffsetNV(factor, units); }
inline void glStencilFillPathNV(GLuint path, GLenum fillMode, GLuint mask) { fp_glStencilFillPathNV(path, fillMode, mask); }
inline void glStencilStrokePathNV(GLuint path, GLint reference, GLuint mask) { fp_glStencilStrokePathNV(path, reference, mask); }
inline void glStencilFillPathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum transformType, const GLfloat* transformValues) { fp_glStencilFillPathInstancedNV(numPaths, pathNameType, paths, pathBase, fillMode, mask, transformType, transformValues); }
inline void glStencilStrokePathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLint reference, GLuint mask, GLenum transformType, const GLfloat* transformValues) { fp_glStencilStrokePathInstancedNV(numPaths, pathNameType, paths, pathBase, reference, mask, transformType, transformValues); }
inline void glPathCoverDepthFuncNV(GLenum func) { fp_glPathCoverDepthFuncNV(func); }
inline void glCoverFillPathNV(GLuint path, GLenum coverMode) { fp_glCoverFillPathNV(path, coverMode); }
inline void glCoverStrokePathNV(GLuint path, GLenum coverMode) { fp_glCoverStrokePathNV(path, coverMode); }
inline void glCoverFillPathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat* transformValues) { fp_glCoverFillPathInstancedNV(numPaths, pathNameType, paths, pathBase, coverMode, transformType, transformValues); }
inline void glCoverStrokePathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat* transformValues) { fp_glCoverStrokePathInstancedNV(numPaths, pathNameType, paths, pathBase, coverMode, transformType, transformValues); }
inline void glGetPathParameterivNV(GLuint path, GLenum pname, GLint* value) { fp_glGetPathParameterivNV(path, pname, value); }
inline void glGetPathParameterfvNV(GLuint path, GLenum pname, GLfloat* value) { fp_glGetPathParameterfvNV(path, pname, value); }
inline void glGetPathCommandsNV(GLuint path, GLubyte* commands) { fp_glGetPathCommandsNV(path, commands); }
inline void glGetPathCoordsNV(GLuint path, GLfloat* coords) { fp_glGetPathCoordsNV(path, coords); }
inline void glGetPathDashArrayNV(GLuint path, GLfloat* dashArray) { fp_glGetPathDashArrayNV(path, dashArray); }
inline void glGetPathMetricsNV(GLbitfield metricQueryMask, GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLsizei stride, GLfloat* metrics) { fp_glGetPathMetricsNV(metricQueryMask, numPaths, pathNameType, paths, pathBase, stride, metrics); }
inline void glGetPathMetricRangeNV(GLbitfield metricQueryMask, GLuint firstPathName, GLsizei numPaths, GLsizei stride, GLfloat* metrics) { fp_glGetPathMetricRangeNV(metricQueryMask, firstPathName, numPaths, stride, metrics); }
inline void glGetPathSpacingNV(GLenum pathListMode, GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLfloat advanceScale, GLfloat kerningScale, GLenum transformType, GLfloat* returnedSpacing) { fp_glGetPathSpacingNV(pathListMode, numPaths, pathNameType, paths, pathBase, advanceScale, kerningScale, transformType, returnedSpacing); }
inline GLboolean glIsPointInFillPathNV(GLuint path, GLuint mask, GLfloat x, GLfloat y) { return fp_glIsPointInFillPathNV(path, mask, x, y); }
inline GLboolean glIsPointInStrokePathNV(GLuint path, GLfloat x, GLfloat y) { return fp_glIsPointInStrokePathNV(path, x, y); }
inline GLfloat glGetPathLengthNV(GLuint path, GLsizei startSegment, GLsizei numSegments) { return fp_glGetPathLengthNV(path, startSegment, numSegments); }
inline GLboolean glPointAlongPathNV(GLuint path, GLsizei startSegment, GLsizei numSegments, GLfloat distance, GLfloat* x, GLfloat* y, GLfloat* tangentX, GLfloat* tangentY) { return fp_glPointAlongPathNV(path, startSegment, numSegments, distance, x, y, tangentX, tangentY); }
inline void glMatrixLoad3x2fNV(GLenum matrixMode, const GLfloat* m) { fp_glMatrixLoad3x2fNV(matrixMode, m); }
inline void glMatrixLoad3x3fNV(GLenum matrixMode, const GLfloat* m) { fp_glMatrixLoad3x3fNV(matrixMode, m); }
inline void glMatrixLoadTranspose3x3fNV(GLenum matrixMode, const GLfloat* m) { fp_glMatrixLoadTranspose3x3fNV(matrixMode, m); }
inline void glMatrixMult3x2fNV(GLenum matrixMode, const GLfloat* m) { fp_glMatrixMult3x2fNV(matrixMode, m); }
inline void glMatrixMult3x3fNV(GLenum matrixMode, const GLfloat* m) { fp_glMatrixMult3x3fNV(matrixMode, m); }
inline void glMatrixMultTranspose3x3fNV(GLenum matrixMode, const GLfloat* m) { fp_glMatrixMultTranspose3x3fNV(matrixMode, m); }
inline void glStencilThenCoverFillPathNV(GLuint path, GLenum fillMode, GLuint mask, GLenum coverMode) { fp_glStencilThenCoverFillPathNV(path, fillMode, mask, coverMode); }
inline void glStencilThenCoverStrokePathNV(GLuint path, GLint reference, GLuint mask, GLenum coverMode) { fp_glStencilThenCoverStrokePathNV(path, reference, mask, coverMode); }
inline void glStencilThenCoverFillPathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat* transformValues) { fp_glStencilThenCoverFillPathInstancedNV(numPaths, pathNameType, paths, pathBase, fillMode, mask, coverMode, transformType, transformValues); }
inline void glStencilThenCoverStrokePathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLint reference, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat* transformValues) { fp_glStencilThenCoverStrokePathInstancedNV(numPaths, pathNameType, paths, pathBase, reference, mask, coverMode, transformType, transformValues); }
inline GLenum glPathGlyphIndexRangeNV(GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLuint pathParameterTemplate, GLfloat emScale, GLuint baseAndCount) { return fp_glPathGlyphIndexRangeNV(fontTarget, fontName, fontStyle, pathParameterTemplate, emScale, baseAndCount); }
inline GLenum glPathGlyphIndexArrayNV(GLuint firstPathName, GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale) { return fp_glPathGlyphIndexArrayNV(firstPathName, fontTarget, fontName, fontStyle, firstGlyphIndex, numGlyphs, pathParameterTemplate, emScale); }
inline GLenum glPathMemoryGlyphIndexArrayNV(GLuint firstPathName, GLenum fontTarget, GLsizeiptr fontSize, const void* fontData, GLsizei faceIndex, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale) { return fp_glPathMemoryGlyphIndexArrayNV(firstPathName, fontTarget, fontSize, fontData, faceIndex, firstGlyphIndex, numGlyphs, pathParameterTemplate, emScale); }
inline void glProgramPathFragmentInputGenNV(GLuint program, GLint location, GLenum genMode, GLint components, const GLfloat* coeffs) { fp_glProgramPathFragmentInputGenNV(program, location, genMode, components, coeffs); }
inline void glGetProgramResourcefvNV(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei bufSize, GLsizei* length, GLfloat* params) { fp_glGetProgramResourcefvNV(program, programInterface, index, propCount, props, bufSize, length, params); }
inline void glPathColorGenNV(GLenum color, GLenum genMode, GLenum colorFormat, const GLfloat* coeffs) { fp_glPathColorGenNV(color, genMode, colorFormat, coeffs); }
inline void glPathTexGenNV(GLenum texCoordSet, GLenum genMode, GLint components, const GLfloat* coeffs) { fp_glPathTexGenNV(texCoordSet, genMode, components, coeffs); }
inline void glPathFogGenNV(GLenum genMode) { fp_glPathFogGenNV(genMode); }
inline void glGetPathColorGenivNV(GLenum color, GLenum pname, GLint* value) { fp_glGetPathColorGenivNV(color, pname, value); }
inline void glGetPathColorGenfvNV(GLenum color, GLenum pname, GLfloat* value) { fp_glGetPathColorGenfvNV(color, pname, value); }
inline void glGetPathTexGenivNV(GLenum texCoordSet, GLenum pname, GLint* value) { fp_glGetPathTexGenivNV(texCoordSet, pname, value); }
inline void glGetPathTexGenfvNV(GLenum texCoordSet, GLenum pname, GLfloat* value) { fp_glGetPathTexGenfvNV(texCoordSet, pname, value); }

/* GL_NV_polygon_mode */
inline void glPolygonModeNV(GLenum face, GLenum mode) { fp_glPolygonModeNV(face, mode); }

/* GL_NV_present_video */
inline void glPresentFrameKeyedNV(GLuint video_slot, GLuint64EXT minPresentTime, GLuint beginPresentTimeId, GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0, GLuint key0, GLenum target1, GLuint fill1, GLuint key1) { fp_glPresentFrameKeyedNV(video_slot, minPresentTime, beginPresentTimeId, presentDurationId, type, target0, fill0, key0, target1, fill1, key1); }
inline void glPresentFrameDualFillNV(GLuint video_slot, GLuint64EXT minPresentTime, GLuint beginPresentTimeId, GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0, GLenum target1, GLuint fill1, GLenum target2, GLuint fill2, GLenum target3, GLuint fill3) { fp_glPresentFrameDualFillNV(video_slot, minPresentTime, beginPresentTimeId, presentDurationId, type, target0, fill0, target1, fill1, target2, fill2, target3, fill3); }
inline void glGetVideoivNV(GLuint video_slot, GLenum pname, GLint* params) { fp_glGetVideoivNV(video_slot, pname, params); }
inline void glGetVideouivNV(GLuint video_slot, GLenum pname, GLuint* params) { fp_glGetVideouivNV(video_slot, pname, params); }
inline void glGetVideoi64vNV(GLuint video_slot, GLenum pname, GLint64EXT* params) { fp_glGetVideoi64vNV(video_slot, pname, params); }
inline void glGetVideoui64vNV(GLuint video_slot, GLenum pname, GLuint64EXT* params) { fp_glGetVideoui64vNV(video_slot, pname, params); }

/* GL_NV_primitive_restart */
inline void glPrimitiveRestartNV() { fp_glPrimitiveRestartNV(); }
inline void glPrimitiveRestartIndexNV(GLuint index) { fp_glPrimitiveRestartIndexNV(index); }

/* GL_NV_read_buffer */
inline void glReadBufferNV(GLenum mode) { fp_glReadBufferNV(mode); }

/* GL_NV_sample_locations */
inline void glFramebufferSampleLocationsfvNV(GLenum target, GLuint start, GLsizei count, const GLfloat* v) { fp_glFramebufferSampleLocationsfvNV(target, start, count, v); }
inline void glNamedFramebufferSampleLocationsfvNV(GLuint framebuffer, GLuint start, GLsizei count, const GLfloat* v) { fp_glNamedFramebufferSampleLocationsfvNV(framebuffer, start, count, v); }
inline void glResolveDepthValuesNV() { fp_glResolveDepthValuesNV(); }

/* GL_NV_shader_buffer_load */
inline void glMakeBufferResidentNV(GLenum target, GLenum access) { fp_glMakeBufferResidentNV(target, access); }
inline void glMakeBufferNonResidentNV(GLenum target) { fp_glMakeBufferNonResidentNV(target); }
inline GLboolean glIsBufferResidentNV(GLenum target) { return fp_glIsBufferResidentNV(target); }
inline void glMakeNamedBufferResidentNV(GLuint buffer, GLenum access) { fp_glMakeNamedBufferResidentNV(buffer, access); }
inline void glMakeNamedBufferNonResidentNV(GLuint buffer) { fp_glMakeNamedBufferNonResidentNV(buffer); }
inline GLboolean glIsNamedBufferResidentNV(GLuint buffer) { return fp_glIsNamedBufferResidentNV(buffer); }
inline void glGetBufferParameterui64vNV(GLenum target, GLenum pname, GLuint64EXT* params) { fp_glGetBufferParameterui64vNV(target, pname, params); }
inline void glGetNamedBufferParameterui64vNV(GLuint buffer, GLenum pname, GLuint64EXT* params) { fp_glGetNamedBufferParameterui64vNV(buffer, pname, params); }
inline void glGetIntegerui64vNV(GLenum value, GLuint64EXT* result) { fp_glGetIntegerui64vNV(value, result); }
inline void glUniformui64NV(GLint location, GLuint64EXT value) { fp_glUniformui64NV(location, value); }
inline void glUniformui64vNV(GLint location, GLsizei count, const GLuint64EXT* value) { fp_glUniformui64vNV(location, count, value); }
inline void glProgramUniformui64NV(GLuint program, GLint location, GLuint64EXT value) { fp_glProgramUniformui64NV(program, location, value); }
inline void glProgramUniformui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value) { fp_glProgramUniformui64vNV(program, location, count, value); }

/* GL_NV_texture_barrier */
inline void glTextureBarrierNV() { fp_glTextureBarrierNV(); }

/* GL_NV_texture_multisample */
inline void glTexImage2DMultisampleCoverageNV(GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations) { fp_glTexImage2DMultisampleCoverageNV(target, coverageSamples, colorSamples, internalFormat, width, height, fixedSampleLocations); }
inline void glTexImage3DMultisampleCoverageNV(GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations) { fp_glTexImage3DMultisampleCoverageNV(target, coverageSamples, colorSamples, internalFormat, width, height, depth, fixedSampleLocations); }
inline void glTextureImage2DMultisampleNV(GLuint texture, GLenum target, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations) { fp_glTextureImage2DMultisampleNV(texture, target, samples, internalFormat, width, height, fixedSampleLocations); }
inline void glTextureImage3DMultisampleNV(GLuint texture, GLenum target, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations) { fp_glTextureImage3DMultisampleNV(texture, target, samples, internalFormat, width, height, depth, fixedSampleLocations); }
inline void glTextureImage2DMultisampleCoverageNV(GLuint texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations) { fp_glTextureImage2DMultisampleCoverageNV(texture, target, coverageSamples, colorSamples, internalFormat, width, height, fixedSampleLocations); }
inline void glTextureImage3DMultisampleCoverageNV(GLuint texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations) { fp_glTextureImage3DMultisampleCoverageNV(texture, target, coverageSamples, colorSamples, internalFormat, width, height, depth, fixedSampleLocations); }

/* GL_NV_transform_feedback */
inline void glBeginTransformFeedbackNV(GLenum primitiveMode) { fp_glBeginTransformFeedbackNV(primitiveMode); }
inline void glEndTransformFeedbackNV() { fp_glEndTransformFeedbackNV(); }
inline void glTransformFeedbackAttribsNV(GLsizei count, const GLint* attribs, GLenum bufferMode) { fp_glTransformFeedbackAttribsNV(count, attribs, bufferMode); }
inline void glBindBufferRangeNV(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) { fp_glBindBufferRangeNV(target, index, buffer, offset, size); }
inline void glBindBufferOffsetNV(GLenum target, GLuint index, GLuint buffer, GLintptr offset) { fp_glBindBufferOffsetNV(target, index, buffer, offset); }
inline void glBindBufferBaseNV(GLenum target, GLuint index, GLuint buffer) { fp_glBindBufferBaseNV(target, index, buffer); }
inline void glTransformFeedbackVaryingsNV(GLuint program, GLsizei count, const GLint* locations, GLenum bufferMode) { fp_glTransformFeedbackVaryingsNV(program, count, locations, bufferMode); }
inline void glActiveVaryingNV(GLuint program, const GLchar* name) { fp_glActiveVaryingNV(program, name); }
inline GLint glGetVaryingLocationNV(GLuint program, const GLchar* name) { return fp_glGetVaryingLocationNV(program, name); }
inline void glGetActiveVaryingNV(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name) { fp_glGetActiveVaryingNV(program, index, bufSize, length, size, type, name); }
inline void glGetTransformFeedbackVaryingNV(GLuint program, GLuint index, GLint* location) { fp_glGetTransformFeedbackVaryingNV(program, index, location); }
inline void glTransformFeedbackStreamAttribsNV(GLsizei count, const GLint* attribs, GLsizei nbuffers, const GLint* bufstreams, GLenum bufferMode) { fp_glTransformFeedbackStreamAttribsNV(count, attribs, nbuffers, bufstreams, bufferMode); }

/* GL_NV_transform_feedback2 */
inline void glBindTransformFeedbackNV(GLenum target, GLuint id) { fp_glBindTransformFeedbackNV(target, id); }
inline void glDeleteTransformFeedbacksNV(GLsizei n, const GLuint* ids) { fp_glDeleteTransformFeedbacksNV(n, ids); }
inline void glGenTransformFeedbacksNV(GLsizei n, GLuint* ids) { fp_glGenTransformFeedbacksNV(n, ids); }
inline GLboolean glIsTransformFeedbackNV(GLuint id) { return fp_glIsTransformFeedbackNV(id); }
inline void glPauseTransformFeedbackNV() { fp_glPauseTransformFeedbackNV(); }
inline void glResumeTransformFeedbackNV() { fp_glResumeTransformFeedbackNV(); }
inline void glDrawTransformFeedbackNV(GLenum mode, GLuint id) { fp_glDrawTransformFeedbackNV(mode, id); }

/* GL_NV_vdpau_interop */
inline void glVDPAUInitNV(const void* vdpDevice, const void* getProcAddress) { fp_glVDPAUInitNV(vdpDevice, getProcAddress); }
inline void glVDPAUFiniNV() { fp_glVDPAUFiniNV(); }
inline GLvdpauSurfaceNV glVDPAURegisterVideoSurfaceNV(const void* vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint* textureNames) { return fp_glVDPAURegisterVideoSurfaceNV(vdpSurface, target, numTextureNames, textureNames); }
inline GLvdpauSurfaceNV glVDPAURegisterOutputSurfaceNV(const void* vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint* textureNames) { return fp_glVDPAURegisterOutputSurfaceNV(vdpSurface, target, numTextureNames, textureNames); }
inline GLboolean glVDPAUIsSurfaceNV(GLvdpauSurfaceNV surface) { return fp_glVDPAUIsSurfaceNV(surface); }
inline void glVDPAUUnregisterSurfaceNV(GLvdpauSurfaceNV surface) { fp_glVDPAUUnregisterSurfaceNV(surface); }
inline void glVDPAUGetSurfaceivNV(GLvdpauSurfaceNV surface, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values) { fp_glVDPAUGetSurfaceivNV(surface, pname, bufSize, length, values); }
inline void glVDPAUSurfaceAccessNV(GLvdpauSurfaceNV surface, GLenum access) { fp_glVDPAUSurfaceAccessNV(surface, access); }
inline void glVDPAUMapSurfacesNV(GLsizei numSurfaces, const GLvdpauSurfaceNV* surfaces) { fp_glVDPAUMapSurfacesNV(numSurfaces, surfaces); }
inline void glVDPAUUnmapSurfacesNV(GLsizei numSurface, const GLvdpauSurfaceNV* surfaces) { fp_glVDPAUUnmapSurfacesNV(numSurface, surfaces); }

/* GL_NV_vertex_attrib_integer_64bit */
inline void glVertexAttribL1i64NV(GLuint index, GLint64EXT x) { fp_glVertexAttribL1i64NV(index, x); }
inline void glVertexAttribL2i64NV(GLuint index, GLint64EXT x, GLint64EXT y) { fp_glVertexAttribL2i64NV(index, x, y); }
inline void glVertexAttribL3i64NV(GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z) { fp_glVertexAttribL3i64NV(index, x, y, z); }
inline void glVertexAttribL4i64NV(GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w) { fp_glVertexAttribL4i64NV(index, x, y, z, w); }
inline void glVertexAttribL1i64vNV(GLuint index, const GLint64EXT* v) { fp_glVertexAttribL1i64vNV(index, v); }
inline void glVertexAttribL2i64vNV(GLuint index, const GLint64EXT* v) { fp_glVertexAttribL2i64vNV(index, v); }
inline void glVertexAttribL3i64vNV(GLuint index, const GLint64EXT* v) { fp_glVertexAttribL3i64vNV(index, v); }
inline void glVertexAttribL4i64vNV(GLuint index, const GLint64EXT* v) { fp_glVertexAttribL4i64vNV(index, v); }
inline void glVertexAttribL1ui64NV(GLuint index, GLuint64EXT x) { fp_glVertexAttribL1ui64NV(index, x); }
inline void glVertexAttribL2ui64NV(GLuint index, GLuint64EXT x, GLuint64EXT y) { fp_glVertexAttribL2ui64NV(index, x, y); }
inline void glVertexAttribL3ui64NV(GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z) { fp_glVertexAttribL3ui64NV(index, x, y, z); }
inline void glVertexAttribL4ui64NV(GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w) { fp_glVertexAttribL4ui64NV(index, x, y, z, w); }
inline void glVertexAttribL1ui64vNV(GLuint index, const GLuint64EXT* v) { fp_glVertexAttribL1ui64vNV(index, v); }
inline void glVertexAttribL2ui64vNV(GLuint index, const GLuint64EXT* v) { fp_glVertexAttribL2ui64vNV(index, v); }
inline void glVertexAttribL3ui64vNV(GLuint index, const GLuint64EXT* v) { fp_glVertexAttribL3ui64vNV(index, v); }
inline void glVertexAttribL4ui64vNV(GLuint index, const GLuint64EXT* v) { fp_glVertexAttribL4ui64vNV(index, v); }
inline void glGetVertexAttribLi64vNV(GLuint index, GLenum pname, GLint64EXT* params) { fp_glGetVertexAttribLi64vNV(index, pname, params); }
inline void glGetVertexAttribLui64vNV(GLuint index, GLenum pname, GLuint64EXT* params) { fp_glGetVertexAttribLui64vNV(index, pname, params); }
inline void glVertexAttribLFormatNV(GLuint index, GLint size, GLenum type, GLsizei stride) { fp_glVertexAttribLFormatNV(index, size, type, stride); }

/* GL_NV_vertex_buffer_unified_memory */
inline void glBufferAddressRangeNV(GLenum pname, GLuint index, GLuint64EXT address, GLsizeiptr length) { fp_glBufferAddressRangeNV(pname, index, address, length); }
inline void glVertexFormatNV(GLint size, GLenum type, GLsizei stride) { fp_glVertexFormatNV(size, type, stride); }
inline void glNormalFormatNV(GLenum type, GLsizei stride) { fp_glNormalFormatNV(type, stride); }
inline void glColorFormatNV(GLint size, GLenum type, GLsizei stride) { fp_glColorFormatNV(size, type, stride); }
inline void glIndexFormatNV(GLenum type, GLsizei stride) { fp_glIndexFormatNV(type, stride); }
inline void glTexCoordFormatNV(GLint size, GLenum type, GLsizei stride) { fp_glTexCoordFormatNV(size, type, stride); }
inline void glEdgeFlagFormatNV(GLsizei stride) { fp_glEdgeFlagFormatNV(stride); }
inline void glSecondaryColorFormatNV(GLint size, GLenum type, GLsizei stride) { fp_glSecondaryColorFormatNV(size, type, stride); }
inline void glFogCoordFormatNV(GLenum type, GLsizei stride) { fp_glFogCoordFormatNV(type, stride); }
inline void glVertexAttribFormatNV(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride) { fp_glVertexAttribFormatNV(index, size, type, normalized, stride); }
inline void glVertexAttribIFormatNV(GLuint index, GLint size, GLenum type, GLsizei stride) { fp_glVertexAttribIFormatNV(index, size, type, stride); }
inline void glGetIntegerui64i_vNV(GLenum value, GLuint index, GLuint64EXT* result) { fp_glGetIntegerui64i_vNV(value, index, result); }

/* GL_NV_vertex_program4 */
inline void glVertexAttribI1iEXT(GLuint index, GLint x) { fp_glVertexAttribI1iEXT(index, x); }
inline void glVertexAttribI2iEXT(GLuint index, GLint x, GLint y) { fp_glVertexAttribI2iEXT(index, x, y); }
inline void glVertexAttribI3iEXT(GLuint index, GLint x, GLint y, GLint z) { fp_glVertexAttribI3iEXT(index, x, y, z); }
inline void glVertexAttribI4iEXT(GLuint index, GLint x, GLint y, GLint z, GLint w) { fp_glVertexAttribI4iEXT(index, x, y, z, w); }
inline void glVertexAttribI1uiEXT(GLuint index, GLuint x) { fp_glVertexAttribI1uiEXT(index, x); }
inline void glVertexAttribI2uiEXT(GLuint index, GLuint x, GLuint y) { fp_glVertexAttribI2uiEXT(index, x, y); }
inline void glVertexAttribI3uiEXT(GLuint index, GLuint x, GLuint y, GLuint z) { fp_glVertexAttribI3uiEXT(index, x, y, z); }
inline void glVertexAttribI4uiEXT(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) { fp_glVertexAttribI4uiEXT(index, x, y, z, w); }
inline void glVertexAttribI1ivEXT(GLuint index, const GLint* v) { fp_glVertexAttribI1ivEXT(index, v); }
inline void glVertexAttribI2ivEXT(GLuint index, const GLint* v) { fp_glVertexAttribI2ivEXT(index, v); }
inline void glVertexAttribI3ivEXT(GLuint index, const GLint* v) { fp_glVertexAttribI3ivEXT(index, v); }
inline void glVertexAttribI4ivEXT(GLuint index, const GLint* v) { fp_glVertexAttribI4ivEXT(index, v); }
inline void glVertexAttribI1uivEXT(GLuint index, const GLuint* v) { fp_glVertexAttribI1uivEXT(index, v); }
inline void glVertexAttribI2uivEXT(GLuint index, const GLuint* v) { fp_glVertexAttribI2uivEXT(index, v); }
inline void glVertexAttribI3uivEXT(GLuint index, const GLuint* v) { fp_glVertexAttribI3uivEXT(index, v); }
inline void glVertexAttribI4uivEXT(GLuint index, const GLuint* v) { fp_glVertexAttribI4uivEXT(index, v); }
inline void glVertexAttribI4bvEXT(GLuint index, const GLbyte* v) { fp_glVertexAttribI4bvEXT(index, v); }
inline void glVertexAttribI4svEXT(GLuint index, const GLshort* v) { fp_glVertexAttribI4svEXT(index, v); }
inline void glVertexAttribI4ubvEXT(GLuint index, const GLubyte* v) { fp_glVertexAttribI4ubvEXT(index, v); }
inline void glVertexAttribI4usvEXT(GLuint index, const GLushort* v) { fp_glVertexAttribI4usvEXT(index, v); }
inline void glVertexAttribIPointerEXT(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) { fp_glVertexAttribIPointerEXT(index, size, type, stride, pointer); }
inline void glGetVertexAttribIivEXT(GLuint index, GLenum pname, GLint* params) { fp_glGetVertexAttribIivEXT(index, pname, params); }
inline void glGetVertexAttribIuivEXT(GLuint index, GLenum pname, GLuint* params) { fp_glGetVertexAttribIuivEXT(index, pname, params); }

/* GL_NV_video_capture */
inline void glBeginVideoCaptureNV(GLuint video_capture_slot) { fp_glBeginVideoCaptureNV(video_capture_slot); }
inline void glBindVideoCaptureStreamBufferNV(GLuint video_capture_slot, GLuint stream, GLenum frame_region, GLintptrARB offset) { fp_glBindVideoCaptureStreamBufferNV(video_capture_slot, stream, frame_region, offset); }
inline void glBindVideoCaptureStreamTextureNV(GLuint video_capture_slot, GLuint stream, GLenum frame_region, GLenum target, GLuint texture) { fp_glBindVideoCaptureStreamTextureNV(video_capture_slot, stream, frame_region, target, texture); }
inline void glEndVideoCaptureNV(GLuint video_capture_slot) { fp_glEndVideoCaptureNV(video_capture_slot); }
inline void glGetVideoCaptureivNV(GLuint video_capture_slot, GLenum pname, GLint* params) { fp_glGetVideoCaptureivNV(video_capture_slot, pname, params); }
inline void glGetVideoCaptureStreamivNV(GLuint video_capture_slot, GLuint stream, GLenum pname, GLint* params) { fp_glGetVideoCaptureStreamivNV(video_capture_slot, stream, pname, params); }
inline void glGetVideoCaptureStreamfvNV(GLuint video_capture_slot, GLuint stream, GLenum pname, GLfloat* params) { fp_glGetVideoCaptureStreamfvNV(video_capture_slot, stream, pname, params); }
inline void glGetVideoCaptureStreamdvNV(GLuint video_capture_slot, GLuint stream, GLenum pname, GLdouble* params) { fp_glGetVideoCaptureStreamdvNV(video_capture_slot, stream, pname, params); }
inline GLenum glVideoCaptureNV(GLuint video_capture_slot, GLuint* sequence_num, GLuint64EXT* capture_time) { return fp_glVideoCaptureNV(video_capture_slot, sequence_num, capture_time); }
inline void glVideoCaptureStreamParameterivNV(GLuint video_capture_slot, GLuint stream, GLenum pname, const GLint* params) { fp_glVideoCaptureStreamParameterivNV(video_capture_slot, stream, pname, params); }
inline void glVideoCaptureStreamParameterfvNV(GLuint video_capture_slot, GLuint stream, GLenum pname, const GLfloat* params) { fp_glVideoCaptureStreamParameterfvNV(video_capture_slot, stream, pname, params); }
inline void glVideoCaptureStreamParameterdvNV(GLuint video_capture_slot, GLuint stream, GLenum pname, const GLdouble* params) { fp_glVideoCaptureStreamParameterdvNV(video_capture_slot, stream, pname, params); }

/* GL_NV_viewport_array */
inline void glViewportArrayvNV(GLuint first, GLsizei count, const GLfloat* v) { fp_glViewportArrayvNV(first, count, v); }
inline void glViewportIndexedfNV(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h) { fp_glViewportIndexedfNV(index, x, y, w, h); }
inline void glViewportIndexedfvNV(GLuint index, const GLfloat* v) { fp_glViewportIndexedfvNV(index, v); }
inline void glScissorArrayvNV(GLuint first, GLsizei count, const GLint* v) { fp_glScissorArrayvNV(first, count, v); }
inline void glScissorIndexedNV(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height) { fp_glScissorIndexedNV(index, left, bottom, width, height); }
inline void glScissorIndexedvNV(GLuint index, const GLint* v) { fp_glScissorIndexedvNV(index, v); }
inline void glDepthRangeArrayfvNV(GLuint first, GLsizei count, const GLfloat* v) { fp_glDepthRangeArrayfvNV(first, count, v); }
inline void glDepthRangeIndexedfNV(GLuint index, GLfloat n, GLfloat f) { fp_glDepthRangeIndexedfNV(index, n, f); }
inline void glGetFloati_vNV(GLenum target, GLuint index, GLfloat* data) { fp_glGetFloati_vNV(target, index, data); }
inline void glEnableiNV(GLenum target, GLuint index) { fp_glEnableiNV(target, index); }
inline void glDisableiNV(GLenum target, GLuint index) { fp_glDisableiNV(target, index); }
inline GLboolean glIsEnablediNV(GLenum target, GLuint index) { return fp_glIsEnablediNV(target, index); }

/* GL_NVX_conditional_render */
inline void glBeginConditionalRenderNVX(GLuint id) { fp_glBeginConditionalRenderNVX(id); }
inline void glEndConditionalRenderNVX() { fp_glEndConditionalRenderNVX(); }

/* GL_OVR_multiview */
inline void glFramebufferTextureMultiviewOVR(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews) { fp_glFramebufferTextureMultiviewOVR(target, attachment, texture, level, baseViewIndex, numViews); }

/* GL_OVR_multiview_multisampled_render_to_texture */
inline void glFramebufferTextureMultisampleMultiviewOVR(GLenum target, GLenum attachment, GLuint texture, GLint level, GLsizei samples, GLint baseViewIndex, GLsizei numViews) { fp_glFramebufferTextureMultisampleMultiviewOVR(target, attachment, texture, level, samples, baseViewIndex, numViews); }

/* GL_QCOM_alpha_test */
inline void glAlphaFuncQCOM(GLenum func, GLclampf ref) { fp_glAlphaFuncQCOM(func, ref); }

/* GL_QCOM_driver_control */
inline void glGetDriverControlsQCOM(GLint* num, GLsizei size, GLuint* driverControls) { fp_glGetDriverControlsQCOM(num, size, driverControls); }
inline void glGetDriverControlStringQCOM(GLuint driverControl, GLsizei bufSize, GLsizei* length, GLchar* driverControlString) { fp_glGetDriverControlStringQCOM(driverControl, bufSize, length, driverControlString); }
inline void glEnableDriverControlQCOM(GLuint driverControl) { fp_glEnableDriverControlQCOM(driverControl); }
inline void glDisableDriverControlQCOM(GLuint driverControl) { fp_glDisableDriverControlQCOM(driverControl); }

/* GL_QCOM_extended_get */
inline void glExtGetTexturesQCOM(GLuint* textures, GLint maxTextures, GLint* numTextures) { fp_glExtGetTexturesQCOM(textures, maxTextures, numTextures); }
inline void glExtGetBuffersQCOM(GLuint* buffers, GLint maxBuffers, GLint* numBuffers) { fp_glExtGetBuffersQCOM(buffers, maxBuffers, numBuffers); }
inline void glExtGetRenderbuffersQCOM(GLuint* renderbuffers, GLint maxRenderbuffers, GLint* numRenderbuffers) { fp_glExtGetRenderbuffersQCOM(renderbuffers, maxRenderbuffers, numRenderbuffers); }
inline void glExtGetFramebuffersQCOM(GLuint* framebuffers, GLint maxFramebuffers, GLint* numFramebuffers) { fp_glExtGetFramebuffersQCOM(framebuffers, maxFramebuffers, numFramebuffers); }
inline void glExtGetTexLevelParameterivQCOM(GLuint texture, GLenum face, GLint level, GLenum pname, GLint* params) { fp_glExtGetTexLevelParameterivQCOM(texture, face, level, pname, params); }
inline void glExtTexObjectStateOverrideiQCOM(GLenum target, GLenum pname, GLint param) { fp_glExtTexObjectStateOverrideiQCOM(target, pname, param); }
inline void glExtGetTexSubImageQCOM(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, void* texels) { fp_glExtGetTexSubImageQCOM(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, texels); }
inline void glExtGetBufferPointervQCOM(GLenum target, void** params) { fp_glExtGetBufferPointervQCOM(target, params); }

/* GL_QCOM_extended_get2 */
inline void glExtGetShadersQCOM(GLuint* shaders, GLint maxShaders, GLint* numShaders) { fp_glExtGetShadersQCOM(shaders, maxShaders, numShaders); }
inline void glExtGetProgramsQCOM(GLuint* programs, GLint maxPrograms, GLint* numPrograms) { fp_glExtGetProgramsQCOM(programs, maxPrograms, numPrograms); }
inline GLboolean glExtIsProgramBinaryQCOM(GLuint program) { return fp_glExtIsProgramBinaryQCOM(program); }
inline void glExtGetProgramBinarySourceQCOM(GLuint program, GLenum shadertype, GLchar* source, GLint* length) { fp_glExtGetProgramBinarySourceQCOM(program, shadertype, source, length); }

/* GL_QCOM_tiled_rendering */
inline void glStartTilingQCOM(GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask) { fp_glStartTilingQCOM(x, y, width, height, preserveMask); }
inline void glEndTilingQCOM(GLbitfield preserveMask) { fp_glEndTilingQCOM(preserveMask); }

} /* namespace glad */

#endif /* __glad_funcs_hpp_ */
