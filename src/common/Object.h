/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

#ifndef LOVE_OBJECT_H
#define LOVE_OBJECT_H

#include <atomic>

namespace love
{

/**
 * Superclass for all object that should be able to  cross the Lua/C border
 * (this pertains to most objects).
 *
 * This class is an alternative to using smart pointers; it contains retain/release
 * methods, and will delete itself with the reference count hits zero. The wrapper
 * code assumes that all userdata inherits from this class.
 **/
class Object
{
public:

	/**
	 * Constructor. Sets reference count to one.
	 **/
	Object();
	Object(const Object &other);

	/**
	 * Destructor.
	 **/
	virtual ~Object() = 0;

	/**
	 * Gets the reference count of this Object.
	 * @returns The reference count.
	 **/
	int getReferenceCount() const;

	/**
	 * Retains the Object, i.e. increases the
	 * reference count by one.
	 **/
	virtual void retain();

	/**
	 * Releases one reference to the Object, i.e. decrements the
	 * reference count by one, and potentially deletes the Object
	 * if there are no more references.
	 **/
	virtual void release();

private:

	// The reference count.
	std::atomic<int> count;

}; // Object

/**
 * Partial re-implementation + specialization of std::shared_ptr. We can't
 * use C++11's stdlib yet...
 **/
template <typename T>
class StrongRef
{
public:

	StrongRef()
	: object(nullptr)
	{
	}

	StrongRef(T *obj)
	: object(obj)
	{
		if (object) object->retain();
	}

	StrongRef(const StrongRef &other)
	: object(other.get())
	{
		if (object) object->retain();
	}

	~StrongRef()
	{
		if (object) object->release();
	}

	StrongRef &operator = (const StrongRef &other)
	{
		set(other.get());
		return *this;
	}

	T *operator->() const
	{
		return object;
	}

	operator bool() const
	{
		return object != nullptr;
	}

	operator T*() const
	{
		return object;
	}

	void set(T *obj)
	{
		if (obj) obj->retain();
		if (object) object->release();
		object = obj;
	}

	T *get() const
	{
		return object;
	}

private:

	T *object;

}; // StrongRef

} // love

#endif // LOVE_OBJECT_H
