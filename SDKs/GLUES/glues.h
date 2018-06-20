/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice including the dates of first publication and
 * either this permission notice or a reference to
 * http://oss.sgi.com/projects/FreeB/
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * SILICON GRAPHICS, INC. BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of Silicon Graphics, Inc.
 * shall not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written uethorization from
 * Silicon Graphics, Inc.
 *
 * OpenGL ES 1.0 CM port of part of GLU by Mike Gorchak <mike@malva.ua>
 */

#ifndef __glues_h__
#define __glues_h__

#if defined(__USE_SDL_GLES__)
   #include <SDL/SDL_opengles.h>
   #define GLAPI GL_API
#elif defined (__QNXNTO__)
   #include <GLES/gl.h>
#else
   #error "Platform is unsupported"
#endif

#ifndef APIENTRYP
   #define APIENTRYP APIENTRY *
#endif /* APIENTRYP */

#ifdef __cplusplus
   extern "C" {
#endif

/*************************************************************/

/* Boolean */
#define GLU_FALSE                          0
#define GLU_TRUE                           1

/* Version */
#define GLU_VERSION_1_1                    1
#define GLU_VERSION_1_2                    1
#define GLU_VERSION_1_3                    1

/* StringName */
#define GLU_VERSION                        100800
#define GLU_EXTENSIONS                     100801

/* ErrorCode */
#define GLU_INVALID_ENUM                   100900
#define GLU_INVALID_VALUE                  100901
#define GLU_OUT_OF_MEMORY                  100902
#define GLU_INCOMPATIBLE_GL_VERSION        100903
#define GLU_INVALID_OPERATION              100904

/* QuadricDrawStyle */
#define GLU_POINT                          100010
#define GLU_LINE                           100011
#define GLU_FILL                           100012
#define GLU_SILHOUETTE                     100013

/* QuadricCallback */
#define GLU_ERROR                          100103

/* QuadricNormal */
#define GLU_SMOOTH                         100000
#define GLU_FLAT                           100001
#define GLU_NONE                           100002

/* QuadricOrientation */
#define GLU_OUTSIDE                        100020
#define GLU_INSIDE                         100021

/*************************************************************/


#ifdef __cplusplus
class GLUquadric;
#else
typedef struct GLUquadric GLUquadric;
#endif

typedef GLUquadric GLUquadricObj;

/* Internal convenience typedefs */
typedef void (APIENTRYP _GLUfuncptr)();

GLAPI GLboolean APIENTRY gluCheckExtension(const GLubyte* extName, const GLubyte* extString);
GLAPI void APIENTRY gluCylinder(GLUquadric* quad, GLfloat base, GLfloat top, GLfloat height, GLint slices, GLint stacks);
GLAPI void APIENTRY gluDeleteQuadric(GLUquadric* quad);
GLAPI void APIENTRY gluDisk(GLUquadric* quad, GLfloat inner, GLfloat outer, GLint slices, GLint loops);
GLAPI const GLubyte* APIENTRY gluErrorString(GLenum error);
GLAPI const GLubyte * APIENTRY gluGetString(GLenum name);
GLAPI void APIENTRY gluLookAt(GLfloat eyeX, GLfloat eyeY, GLfloat eyeZ, GLfloat centerX, GLfloat centerY, GLfloat centerZ, GLfloat upX, GLfloat upY, GLfloat upZ);
GLAPI GLUquadric* APIENTRY gluNewQuadric(void);
GLAPI void APIENTRY gluOrtho2D(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top);
GLAPI void APIENTRY gluPartialDisk(GLUquadric* quad, GLfloat inner, GLfloat outer, GLint slices, GLint loops, GLfloat start, GLfloat sweep);
GLAPI void APIENTRY gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);
GLAPI void APIENTRY gluPickMatrix(GLfloat x, GLfloat y, GLfloat delX, GLfloat delY, GLint *viewport);
GLAPI GLint APIENTRY gluProject(GLfloat objX, GLfloat objY, GLfloat objZ, const GLfloat *model, const GLfloat *proj, const GLint *view, GLfloat* winX, GLfloat* winY, GLfloat* winZ);
GLAPI void APIENTRY gluQuadricCallback(GLUquadric* quad, GLenum which, _GLUfuncptr CallBackFunc);
GLAPI void APIENTRY gluQuadricDrawStyle(GLUquadric* quad, GLenum draw);
GLAPI void APIENTRY gluQuadricNormals(GLUquadric* quad, GLenum normal);
GLAPI void APIENTRY gluQuadricOrientation(GLUquadric* quad, GLenum orientation);
GLAPI void APIENTRY gluQuadricTexture(GLUquadric* quad, GLboolean texture);
GLAPI void APIENTRY gluSphere(GLUquadric* quad, GLfloat radius, GLint slices, GLint stacks);
GLAPI GLint APIENTRY gluUnProject(GLfloat winX, GLfloat winY, GLfloat winZ, const GLfloat *model, const GLfloat *proj, const GLint *view, GLfloat* objX, GLfloat* objY, GLfloat* objZ);
GLAPI GLint APIENTRY gluUnProject4(GLfloat winX, GLfloat winY, GLfloat winZ, GLfloat clipW, const GLfloat *model, const GLfloat *proj, const GLint *view, GLfloat nearVal, GLfloat farVal, GLfloat* objX, GLfloat* objY, GLfloat* objZ, GLfloat* objW);
GLAPI GLint APIENTRY gluScaleImage(GLenum format, GLsizei widthin,
                             GLsizei heightin, GLenum typein,
                             const void* datain, GLsizei widthout,
                             GLsizei heightout, GLenum typeout, void* dataout);
GLAPI GLint APIENTRY gluBuild2DMipmapLevels(GLenum target, GLint internalFormat,
                             GLsizei width, GLsizei height, GLenum format,
                             GLenum type, GLint userLevel, GLint baseLevel,
                             GLint maxLevel, const void *data);
GLAPI GLint APIENTRY gluBuild2DMipmaps(GLenum target, GLint internalFormat,
                             GLsizei width, GLsizei height, GLenum format,
                             GLenum type, const void* data);

#ifdef __cplusplus
}
#endif

#endif /* __glues_h__ */
