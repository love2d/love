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

#ifndef LOVE_GRAPHICS_OPENGL_FONT_H
#define LOVE_GRAPHICS_OPENGL_FONT_H

// STD
#include <string>

// LOVE
#include <common/Object.h>
#include <font/FontData.h>
#include "Glyph.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	class Font : public Object, public Volatile
	{
	private:
		
		enum FontType
		{
			FONT_TRUETYPE = 1,
			FONT_IMAGE,
			FONT_UNKNOWN
		};

		int height;
		float lineHeight;
		float mSpacing; // modifies the spacing by multiplying it with this value
		Glyph ** glyphs;
		GLuint list; // the list of glyphs, for quicker drawing
		FontType type;

	public:
		static const unsigned int MAX_CHARS = 256;
		// The widths of each character.
		int widths[MAX_CHARS];
		// The spacing of each character.
		int spacing[MAX_CHARS];
		// The X-bearing of each character.
		int bearingX[MAX_CHARS];
		// The Y-bearing of each character.
		int bearingY[MAX_CHARS];

		/**
		* Default constructor.
		*
		* @param data The font data to construct from.
		**/
		Font(love::font::FontData * data);

		virtual ~Font();

		/**
		* Prints the text at the designated position with rotation and scaling.
		*
		* @param text A string.
		* @param x The x-coordinate.
		* @param y The y-coordinate.
		* @param angle The amount of rotation.
		**/
		void print(std::string text, float x, float y, float angle = 0.0f, float sx = 1.0f, float sy = 1.0f) const;

		/**
		* Prints the character at the designated position.
		*
		* @param character A character.
		* @param x The x-coordinate.
		* @param y The y-coordinate.
		**/
		void print(char character, float x, float y) const;

		/**
		* Returns the height of the font.
		**/
		float getHeight() const;

		/**
		* Returns the width of the passed string.
		*
		* @param line A line of text.
		**/
		int getWidth(const std::string & line) const;
		int getWidth(const char * line) const;

		/**
		* Returns the width of the passed character.
		*
		* @param character A character.
		**/
		int getWidth(const char character) const;

		/**
		 * Returns the maximal width of a wrapped string
		 * and optionally the number of lines
		 *
		 * @param line A line of text
		 * @param wrap The number of pixels to wrap at
		 * @param lines Optional output of the number of lines needed
		 **/
		int getWrap(const std::string & line, float wrap, int *lines = 0) const;
		int getWrap(const char * line, float wrap, int *lines = 0) const;

		/**
		* Sets the line height (which should be a number to multiply the font size by,
		* example: line height = 1.2 and size = 12 means that rendered line height = 12*1.2)
		* @param height The new line height.
		**/
		void setLineHeight(float height);

		/**
		* Returns the line height.
		**/
		float getLineHeight() const;

		/**
		* Sets the spacing modifier (changes the spacing between the characters the
		* same way that the line height does [multiplication]).
		* Note: The spacing must be set BEFORE the font is loaded to have any effect.
		* @param amount The amount of modification.
		**/
		void setSpacing(float amount);

		/**
		* Returns the spacing modifier.
		**/
		float getSpacing() const;
		
		// Implements Volatile.
		bool loadVolatile();
		void unloadVolatile();	

	}; // Font

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_FONT_H
