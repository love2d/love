/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

#pragma once

#include "common/Data.h"

namespace love
{
namespace data
{

/**
 * Contains a reference to a subsection of an existing Data object.
 **/
class DataView : public love::Data
{
public:

	static love::Type type;

	DataView(Data *data, size_t offset, size_t size);
	DataView(const DataView &d);
	virtual ~DataView();

	// Implements Data.
	DataView *clone() const override;
	void *getData() const override;
	size_t getSize() const override;

private:

	StrongRef<Data> data;
	size_t offset;
	size_t size;

}; // DataView

} // data
} // love
