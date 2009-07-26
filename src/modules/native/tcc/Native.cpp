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

#include "Native.h"

#include <cstring>
#include <cstdlib>

// The API which should be made available to TCC.
#include "api.h"

namespace love
{
namespace native
{
namespace tcc
{

	Compiler::Compiler()
		: state(0), buffer(0)
	{
		state = tcc_new();

		if(!state) 
		{
			throw love::Exception("Could not create TCC state.");
		}

		tcc_set_output_type(state, TCC_OUTPUT_MEMORY);

		// Add APIs here.
		for(const tcc_function * f = tcc_api; (*f).name != 0; f++)
		{
			tcc_add_symbol(state, (*f).name, (*f).fn);
		}
	}

	Compiler::~Compiler()
	{
		if(state != 0)
			tcc_delete(state);

		if(buffer != 0)
			free(buffer);
	}

	bool Compiler::compile(const char ** str, int size)
	{
		if(state == 0)
			return false;

		for(int i =0; i<size; i++)
		{
			if (tcc_compile_string(state, str[i]) == -1)
				return false;
		}

		if(!relocate())
			return false;

		return true;
	}

	bool Compiler::relocate()
	{
		if(state == 0)
			return false;

		int size = tcc_relocate(state, 0);

		if(size == -1)
			return false;

		buffer = malloc(size);

		if(buffer == 0)
			return false;

		if(tcc_relocate(state, buffer) == -1)
		{
			free(buffer);
			buffer = 0;
			return false;
		}

		return true;
	}

	void * Compiler::getSymbol(const char * sym)
	{
		if(state == 0)
			return 0;

		return tcc_get_symbol(state, sym);
	}


	Native::Native()
	{
	}

	Native::~Native()
	{
		for(int i = 0; i<(int)compilers.size(); i++)
			delete compilers[i];
	}

	const char * Native::getName() const
	{
		return "love.native.tcc";
	}

	bool Native::compile(const char ** str, int size)
	{
		// Create a new compiler.
		Compiler * c = new Compiler();

		// Compile the code.
		if(!c->compile(str, size))
		{
			delete c;
			return false;
		}

		compilers.push_back(c);

		return true;
	}

	void * Native::getSymbol(const char * sym)
	{
		for(int i = 0; i<(int)compilers.size(); i++)
		{
			void * s = compilers[i]->getSymbol(sym);

			if(s != 0)
				return s;
		}

		return 0;
	}

} // tcc
} // native
} // love
