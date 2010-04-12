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

#ifndef LOVE_IMAGE_ENCODED_IMAGE_DATA_H
#define LOVE_IMAGE_ENCODED_IMAGE_DATA_H

// LOVE
#include <common/Data.h>
#include <common/StringMap.h>

namespace love
{	
namespace image
{
	/**
	 * Represents encoded pixel data. 
	 **/
	class EncodedImageData : public Data
	{
	public:
		
		enum Format 
		{
			FORMAT_TGA = 1,
			FORMAT_BMP,
			FORMAT_MAX_ENUM
		};

		/**
		* Constructor.
		**/
		EncodedImageData(void * data, Format format, int size);
		
		/**
		* Destructor.
		**/
		virtual ~EncodedImageData(){};
		
		// Implements Data.
		void * getData() const;
		int getSize() const;
		
		/**
		* Get the format the data is encoded in.
		**/
		Format getFormat() const;

		static bool getConstant(const char * in, Format & out);
		static bool getConstant(Format in, const char *& out);
		
	private:
		
		/**
		* Actual data.
		**/
		void * data;
		
		/**
		* Size of the data.
		**/
		int size;
		
		/**
		 * Image format.
		 **/
		Format format;

		static StringMap<Format, FORMAT_MAX_ENUM>::Entry formatEntries[];
		static StringMap<Format, FORMAT_MAX_ENUM> formats;
		
	}; // EncodedImageData
	
} // image
} // love

#endif // LOVE_IMAGE_ENCODED_IMAGE_DATA_H
