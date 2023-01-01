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

#include "DataView.h"
#include "common/Exception.h"
#include "common/int.h"

namespace love
{
namespace data
{

love::Type DataView::type("DataView", &Data::type);

DataView::DataView(Data *data, size_t offset, size_t size)
	: data(data)
	, offset(offset)
	, size(size)
{
	if (offset >= data->getSize() || size > data->getSize() || offset > data->getSize() - size)
		throw love::Exception("Offset and size of Data View must fit within the original Data's size.");

	if (size == 0)
		throw love::Exception("DataView size must be greater than 0.");
}

DataView::DataView(const DataView &d)
	: data(d.data)
	, offset(d.offset)
	, size(d.size)
{
}

DataView::~DataView()
{
}

DataView *DataView::clone() const
{
	return new DataView(*this);
}

void *DataView::getData() const
{
	return (uint8 *) data->getData() + offset;
}

size_t DataView::getSize() const
{
	return size;
}

} // data
} // love
