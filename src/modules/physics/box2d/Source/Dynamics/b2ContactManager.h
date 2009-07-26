/*
* Copyright (c) 2006-2007 Erin Catto http://www.gphysics.com
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

#ifndef B2_CONTACT_MANAGER_H
#define B2_CONTACT_MANAGER_H

#include "../Collision/b2BroadPhase.h"
#include "../Dynamics/Contacts/b2NullContact.h"

class b2World;
class b2Contact;
struct b2TimeStep;

// Delegate of b2World.
class b2ContactManager : public b2PairCallback
{
public:
	b2ContactManager() : m_world(NULL), m_destroyImmediate(false) {}

	// Implements PairCallback
	void* PairAdded(void* proxyUserData1, void* proxyUserData2);

	// Implements PairCallback
	void PairRemoved(void* proxyUserData1, void* proxyUserData2, void* pairUserData);

	void Destroy(b2Contact* c);

	void Collide();

	b2World* m_world;

	// This lets us provide broadphase proxy pair user data for
	// contacts that shouldn't exist.
	b2NullContact m_nullContact;

	bool m_destroyImmediate;
};

#endif
