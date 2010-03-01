/**
* Copyright (c) 2006-2010 LOVE Development Team
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
**/

#include "TrueTypeFont.h"

#include <SDL_opengl.h>

#include <common/Exception.h>

#include <math.h>
#include <iostream>

using std::string;

namespace love
{
namespace graphics
{
namespace opengl
{
	inline int TrueTypeFont::next_p2(int num)
	{
		int powered = 2;
		while(powered < num) powered <<= 1;
		return powered;
	}

	inline void TrueTypeFont::pushScreenCoordinateMatrix()
	{
		glPushAttrib(GL_TRANSFORM_BIT);
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(viewport[0],viewport[2],viewport[1],viewport[3]);
		glPopAttrib();
	}

	inline void TrueTypeFont::popProjectionMatrix()
	{
		glPushAttrib(GL_TRANSFORM_BIT);
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glPopAttrib();
	}

	void TrueTypeFont::createList(FT_Face face, unsigned short character)
	{
		if( FT_Load_Glyph(face, FT_Get_Char_Index(face, character), FT_LOAD_DEFAULT) )
			std::cerr << "TrueTypeFont Loading vm->error: FT_Load_Glyph failed." << std::endl;

		FT_Glyph glyph;
		if( FT_Get_Glyph(face->glyph, &glyph) )
			std::cerr << "TrueTypeFont Loading vm->error: FT_Get_Glyph failed." << std::endl;

		FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
		FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

		FT_Bitmap& bitmap = bitmap_glyph->bitmap; //just to make things easier

		widths[character] = face->glyph->advance.x >> 6;
		int w = next_p2(bitmap.width);
		int h = next_p2(bitmap.rows);

		if(bitmap.rows > trueHeight)
			trueHeight = bitmap.rows;

		GLubyte* expandedData = new GLubyte[ 2 * w * h];

 		for(int j = 0; j < h; j++) for(int i = 0; i < w; i++)
		{
			expandedData[2 * (i + j * w)] = MAX_CHARS-1;
			expandedData[2 * (i + j * w) + 1] = (i >= bitmap.width || j >= bitmap.rows) ? 0 : bitmap.buffer[i + bitmap.width * j];
		}

		glBindTexture(GL_TEXTURE_2D, textures[character]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Rude adds:
		// (You're welcome, Mike.)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expandedData);



		delete [] expandedData; //no longer needed

		glNewList(list + character, GL_COMPILE);

		glBindTexture(GL_TEXTURE_2D, textures[character]);

		glPushMatrix();

		glTranslatef((float)bitmap_glyph->left, -(float)bitmap_glyph->top, 0);
		//glTranslatef(0, (float)bitmap_glyph->top-bitmap.rows, 0);

		float	x=(float)bitmap.width / (float)w,
				y=(float)bitmap.rows / (float)h;

		glBegin(GL_QUADS);
			glTexCoord2d(0, 0); glVertex2f(0, 0);
			glTexCoord2d(0, y); glVertex2f(0, (float)bitmap.rows);
			glTexCoord2d(x, y); glVertex2f((float)bitmap.width, (float)bitmap.rows);
			glTexCoord2d(x, 0); glVertex2f((float)bitmap.width, 0);
		glEnd();
		glPopMatrix();
		glTranslatef((float)(face->glyph->advance.x >> 6) ,0,0);

		glEndList();

		FT_Done_Glyph(glyph);
	}

	TrueTypeFont::TrueTypeFont(Data * data, int size)
		: Font(size), data(data), textures(0), list(0)
	{
		data->retain();
	}

	TrueTypeFont::~TrueTypeFont()
	{
		unload();
		data->release();
	}

	void TrueTypeFont::print(string text, float x, float y) const
	{
		glPushMatrix();
		glTranslatef(ceil(x), ceil(y), 0.0f); // + getHeight() to make the x,y coordiantes the top left corner
		GLuint TrueTypeFont = list;
		glListBase(TrueTypeFont);
		glCallLists((int)text.length(), GL_UNSIGNED_BYTE, text.c_str());
		glPopMatrix();
	}

	void TrueTypeFont::print(std::string text, float x, float y, float angle, float sx, float sy) const
	{
		glPushMatrix();

		glTranslatef(ceil(x), ceil(y), 0.0f);
		glRotatef(angle * 57.29578f, 0, 0, 1.0f);
		glScalef(sx, sy, 1.0f);

		GLuint TrueTypeFont = list;
		glListBase(TrueTypeFont);
		glCallLists((int)text.length(), GL_UNSIGNED_BYTE, text.c_str());

		glPopMatrix();
	}

	void TrueTypeFont::print(char character, float x, float y) const
	{
		glPushMatrix();
		glTranslatef(ceil(x), ceil(y), 0.0f);
		GLuint TrueTypeFont = list;
		glListBase(TrueTypeFont);
		glCallList(list + (int)character);
		glPopMatrix();
	}

	float TrueTypeFont::getHeight() const
	{
		return (float)trueHeight;
	}

	float TrueTypeFont::getLineHeight() const
	{
		return Font::getLineHeight() * 1.25f;
	}

	bool TrueTypeFont::load()
	{
		return loadVolatile();
	}

	void TrueTypeFont::unload()
	{
		unloadVolatile();
	}

	bool TrueTypeFont::loadVolatile()
	{
		trueHeight = size;


		textures = (unsigned int *)(new GLuint[MAX_CHARS]);
		for(unsigned int i = 0; i != MAX_CHARS; i++) widths[i] = 0;

		FT_Library library;
		if( FT_Init_FreeType(&library) ) {
			std::cerr << "TrueTypeFont Loading error: FT_Init_FreeType failed." << std::endl;
			throw love::Exception("TrueTypeFont Loading error: FT_Init_FreeType failed.");
		}
		FT_Face face;
		if( FT_New_Memory_Face( library,
								(const FT_Byte *)data->getData(),	/* first byte in memory */
								data->getSize(),					/* size in bytes        */
								0,									/* face_index           */
							   &face )) {
			std::cerr << "TrueTypeFont Loading error: FT_New_Face failed (there is probably a problem with your font file)." << std::endl;
			throw love::Exception("TrueTypeFont Loading error: FT_New_Face failed (there is probably a problem with your font file).");
		}
		//FT_Set_Char_Size(face, size << 6, size << 6, 96, 96);
		FT_Set_Pixel_Sizes(face, size, size);

		list = glGenLists(MAX_CHARS);
		glGenTextures(MAX_CHARS, (GLuint*)textures);
		for(unsigned short i = 0; i < MAX_CHARS; i++)
			createList(face, i);

		FT_Done_Face(face);
		FT_Done_FreeType(library); //all done

		return true;
	}

	void TrueTypeFont::unloadVolatile()
	{
		if(list != 0)
			glDeleteLists(list, MAX_CHARS);
		if(textures != 0)
			glDeleteTextures(MAX_CHARS, (const GLuint*)textures);

		// Cleanup plz.
		if(textures != 0)
			delete [] textures;
		textures = 0;
		list = 0;
	}

} // opengl
} // graphics
} // love
