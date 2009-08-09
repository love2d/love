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

#ifndef LOVE_MEMORY_DATA_H
#define LOVE_MEMORY_DATA_H

// LOVE
#include "Data.h"

namespace love
{	
	/**
	* Allows files to be embedded into LOVE and used in Lua. This
	* class assumes the referenced memory area is static, which means
	* it will not attempt to delete the memory.
	**/
	class MemoryData : public Data
	{
	private:

		/**
		* Pointer to the memory area.
		**/
		void * data;

		/**
		* Size of the memory pointed to.
		**/
		int size; 

	public:

		/**
		* Creates a new MemoryData.
		* @param data Pointer to the static memory. 
		* @param size Size of the memory data.
		**/
		MemoryData(void * data, int size);

		/**
		* Destructor. Does NOTHING.
		**/
		virtual ~MemoryData();

		// Implements Data.
		void * getData() const;
		int getSize() const;

	}; // MemoryData
} // love

#endif // LOVE_MEMORY_DATA_H
