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

// The pair manager is used by the broad-phase to quickly add/remove/find pairs
// of overlapping proxies. It is based closely on code provided by Pierre Terdiman.
// http://www.codercorner.com/IncrementalSAP.txt

#ifndef B2_PAIR_MANAGER_H
#define B2_PAIR_MANAGER_H

#include "../Common/b2Settings.h"
#include "../Common/b2Math.h"

#include <climits>

class b2BroadPhase;
struct b2Proxy;

const uint16 b2_nullPair = USHRT_MAX;
const uint16 b2_nullProxy = USHRT_MAX;
const int32 b2_tableCapacity = b2_maxPairs;	// must be a power of two
const int32 b2_tableMask = b2_tableCapacity - 1;

struct b2Pair
{
	enum
	{
		e_pairBuffered	= 0x0001,
		e_pairRemoved	= 0x0002,
		e_pairFinal		= 0x0004,
	};

	void SetBuffered()		{ status |= e_pairBuffered; }
	void ClearBuffered()	{ status &= ~e_pairBuffered; }
	bool IsBuffered()		{ return (status & e_pairBuffered) == e_pairBuffered; }

	void SetRemoved()		{ status |= e_pairRemoved; }
	void ClearRemoved()		{ status &= ~e_pairRemoved; }
	bool IsRemoved()		{ return (status & e_pairRemoved) == e_pairRemoved; }

	void SetFinal()		{ status |= e_pairFinal; }
	bool IsFinal()		{ return (status & e_pairFinal) == e_pairFinal; }

	void* userData;
	uint16 proxyId1;
	uint16 proxyId2;
	uint16 next;
	uint16 status;
};

struct b2BufferedPair
{
	uint16 proxyId1;
	uint16 proxyId2;
};

class b2PairCallback
{
public:
	virtual ~b2PairCallback() {}

	// This should return the new pair user data. It is ok if the
	// user data is null.
	virtual void* PairAdded(void* proxyUserData1, void* proxyUserData2) = 0;

	// This should free the pair's user data. In extreme circumstances, it is possible
	// this will be called with null pairUserData because the pair never existed.
	virtual void PairRemoved(void* proxyUserData1, void* proxyUserData2, void* pairUserData) = 0;
};

class b2PairManager
{
public:
	b2PairManager();

	void Initialize(b2BroadPhase* broadPhase, b2PairCallback* callback);

	void AddBufferedPair(int32 proxyId1, int32 proxyId2);
	void RemoveBufferedPair(int32 proxyId1, int32 proxyId2);

	void Commit();

private:
	b2Pair* Find(int32 proxyId1, int32 proxyId2);
	b2Pair* Find(int32 proxyId1, int32 proxyId2, uint32 hashValue);

	b2Pair* AddPair(int32 proxyId1, int32 proxyId2);
	void* RemovePair(int32 proxyId1, int32 proxyId2);

	void ValidateBuffer();
	void ValidateTable();

public:
	b2BroadPhase *m_broadPhase;
	b2PairCallback *m_callback;
	b2Pair m_pairs[b2_maxPairs];
	uint16 m_freePair;
	int32 m_pairCount;

	b2BufferedPair m_pairBuffer[b2_maxPairs];
	int32 m_pairBufferCount;

	uint16 m_hashTable[b2_tableCapacity];
};

#endif
