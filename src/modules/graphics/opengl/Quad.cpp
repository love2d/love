/**
* Copyright (c) 2006-2011 LOVE Development Team
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

#include "Quad.h"
#include <common/Matrix.h>

// STD
#include <cstring> // For memcpy

namespace love
{
namespace graphics
{
namespace opengl
{
	Quad::Quad(const Viewport & v, int sw, int sh)
		: sw(sw), sh(sh)
	{
		memset(vertices, 255, sizeof(vertex)*4);
		refresh(v, sw, sh);
	}

	Quad::~Quad()
	{
	}

	void Quad::refresh(const Viewport & v, int sw, int sh)
	{
		sw = next_p2(sw);
		sh = next_p2(sh);
		viewport = v;

		vertices[0].x = 0; 
		vertices[0].y = 0;
		vertices[1].x = 0; 
		vertices[1].y = (float)v.h;
		vertices[2].x = (float)v.w; 
		vertices[2].y = (float)v.h;
		vertices[3].x = (float)v.w; 
		vertices[3].y = 0;
		
		vertices[0].s = (float)v.x/(float)sw; 
		vertices[0].t = (float)v.y/(float)sh;
		vertices[1].s = (float)v.x/(float)sw; 
		vertices[1].t = (float)(v.y+v.h)/(float)sh;
		vertices[2].s = (float)(v.x+v.w)/(float)sw; 
		vertices[2].t = (float)(v.y+v.h)/(float)sh;
		vertices[3].s = (float)(v.x+v.w)/(float)sw; 
		vertices[3].t = (float)v.y/(float)sh;
	}

	void Quad::setViewport(const Quad::Viewport & v)
	{
		refresh(v, sw, sh);
	}

	Quad::Viewport Quad::getViewport() const
	{
		return viewport;	
	}
	
	void Quad::flip(bool x, bool y)
	{
		vertex temp[4];
		if (x)
		{
			memcpy(temp, vertices, sizeof(vertex)*4);
			vertices[0].s = temp[3].s; vertices[0].t = temp[3].t;
			vertices[1].s = temp[2].s; vertices[1].t = temp[2].t;
			vertices[2].s = temp[1].s; vertices[2].t = temp[1].t;
			vertices[3].s = temp[0].s; vertices[3].t = temp[0].t;
		}
		if (y)
		{
			memcpy(temp, vertices, sizeof(vertex)*4);
			vertices[0].s = temp[1].s; vertices[0].t = temp[1].t;
			vertices[1].s = temp[0].s; vertices[1].t = temp[0].t;
			vertices[2].s = temp[3].s; vertices[2].t = temp[3].t;
			vertices[3].s = temp[2].s; vertices[3].t = temp[2].t;
		}
	}

	const vertex * Quad::getVertices() const
	{
		return vertices;
	}

} // opengl
} // graphics
} // love
