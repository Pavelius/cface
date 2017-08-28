#include "crt.h"
#include "amem.h"

static int optimal(unsigned need_count)
{
	const unsigned mc = 256 * 256 * 256;
	unsigned m = 64;
	while(m < mc)
	{
		if(need_count < m)
			return m;
		m = m << 1;
	}
	return m;
}

amem::amem(unsigned size)
	: data(0), count(0), size(size), count_maximum(0)
{
}

amem::~amem()
{
	if(data)
		delete data;
	data = 0;
	count = 0;
	count_maximum = 0;
}

char* amem::begin() const
{
	return (char*)data;
}

void* amem::add(const void* element)
{
	if(!reserve(count + 1))
		return 0;
	void* p = (char*)data + size*count;
	if(element)
		memcpy(p, element, size);
	else
		memset(p, 0, size);
	count++;
	return p;
}

void amem::clear()
{
	if(data)
		delete data;
	data = 0;
	count = 0;
	count_maximum = 0;
}

void* amem::get(int index) const
{
	return begin() + index*size;
}

unsigned amem::getcount() const
{
	return count;
}

bool amem::reserve(unsigned count)
{
	if(!size)
		return false;
	if(data)
	{
		if(count_maximum >= count)
			return true;
		if(!rmblock(data))
			return false;
	}
	count_maximum = optimal(count);
	data = rmreserve(data, count_maximum*size);
	return true;
}

void amem::setup(unsigned size)
{
	clear();
	this->size = size;
}

void* amem::insert(int index, const void* object)
{
	if(!reserve(count + 1))
		return 0;
	memmove(begin() + (index + 1)*size, begin() + index*size, (count - index)*size);
	void* p = (char*)data + index*size;
	if(object)
		memcpy(p, object, size);
	else
		memset(p, 0, size);
	count++;
	return p;
}

int amem::indexof(const void* element) const
{
	if(size && element >= begin() && element < (char*)data + count*size)
		return ((char*)element - (char*)data) / size;
	return -1;
}

void amem::swap(int i1, int i2)
{
	auto p1 = (char*)data + i1*size;
	auto p2 = (char*)data + i2*size;
	auto pz = p1 + size;
	while(p1<pz)
		iswap(*p1++, *p2++);
}

void amem::remove(int index, int elements_count)
{
	if(((unsigned)index) >= count)
		return;
	if((unsigned)index < count - 1)
		memcpy((char*)data + index*size,
		(char*)data + (index + elements_count)*size,
			(count - (index + elements_count))*size);
	count -= elements_count;
}