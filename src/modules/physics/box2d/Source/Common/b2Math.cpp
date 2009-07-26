/*
* Copyright (c) 2007 Erin Catto http://www.gphysics.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include "b2Math.h"

const b2Vec2 b2Vec2_zero(0.0f, 0.0f);
const b2Mat22 b2Mat22_identity(1.0f, 0.0f, 0.0f, 1.0f);
const b2XForm b2XForm_identity(b2Vec2_zero, b2Mat22_identity);

void b2Sweep::GetXForm(b2XForm* xf, float32 t) const
{
	// center = p + R * localCenter
	if (1.0f - t0 > B2_FLT_EPSILON)
	{
		float32 alpha = (t - t0) / (1.0f - t0);
		xf->position = (1.0f - alpha) * c0 + alpha * c;
		float32 angle = (1.0f - alpha) * a0 + alpha * a;
		xf->R.Set(angle);
	}
	else
	{
		xf->position = c;
		xf->R.Set(a);
	}

	// Shift to origin
	xf->position -= b2Mul(xf->R, localCenter);
}

void b2Sweep::Advance(float32 t)
{
	if (t0 < t && 1.0f - t0 > B2_FLT_EPSILON)
	{
		float32 alpha = (t - t0) / (1.0f - t0);
		c0 = (1.0f - alpha) * c0 + alpha * c;
		a0 = (1.0f - alpha) * a0 + alpha * a;
		t0 = t;
	}
}
