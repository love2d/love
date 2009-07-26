/**
* Copyright (c) 2006-2009 LOVE Development Team
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

// LOVE
#include <filesystem/File.h>
#include <graphics/Volatile.h>

namespace love
{
namespace graphics
{
namespace opengl
{
	class Font : public Object, public Volatile
	{
	protected:

		love::filesystem::File * file;

		int size;
		float lineHeight;
		float mSpacing; // modifies the spacing by multiplying it with this value

	public:
		static const unsigned int MAX_CHARS = 256;
		// The widths of each character.
		int widths[MAX_CHARS];
		int spacing[MAX_CHARS];

		/**
		* Default constructor.
		*
		* @param file The file containing the OpenGLFont data.
		* @param size The size of the OpenGLFont.
		**/
		Font(int size);
		
		virtual ~Font();

		virtual bool load() = 0;
		virtual void unload() = 0;

		/**
		* Prints the text at the designated position.
		*
		* @param text A string.
		* @param x The x-coordinate.
		* @param y The y-coordinate.
		**/
		virtual void print(std::string text, float x, float y) const = 0;

		/**
		* Prints the text at the designated position with rotation and scaling.
		*
		* @param text A string.
		* @param x The x-coordinate.
		* @param y The y-coordinate.
		* @param angle The amount of rotation.
		**/
		virtual void print(std::string text, float x, float y, float angle, float sx, float sy) const = 0;

		/**
		* Prints the character at the designated position.
		*
		* @param character A character.
		* @param x The x-coordinate.
		* @param y The y-coordinate.
		**/
		virtual void print(char character, float x, float y) const  = 0;

		/**
		* Returns the height of the font.
		**/
		virtual float getHeight() const;

		/**
		* Returns the width of the passed string.
		*
		* @param line A line of text.
		**/
		virtual float getWidth(const std::string & line) const;
		virtual float getWidth(const char * line) const;
		
		/**
		* Returns the width of the passed character.
		*
		* @param character A character.
		**/
		virtual float getWidth(const char character) const;

		/**
		* Sets the line height (which should be a number to multiply the font size by,
		* example: line height = 1.2 and size = 12 means that rendered line height = 12*1.2)
		* @param height The new line height.
		**/
		void setLineHeight(float height);

		/**
		* Returns the line height.
		**/
		virtual float getLineHeight() const;

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

	}; // Font

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_FONT_H
