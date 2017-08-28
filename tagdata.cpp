#include "crt.h"
#include "tagdata.h"

tagdata::tagdata()
{
	indecies.initialize();
	strings.initialize();
}

tagdata::~tagdata()
{
	clear();
}

void tagdata::clear()
{
	indecies.clear();
	strings.clear();
}

int tagdata::find(const char* name)
{
	for(int i = 0; i < indecies.count; i++)
	{
		if(strcmp(strings.data + indecies.data[i], name) == 0)
			return i + 1;
	}
	return 0;
}

int tagdata::add(const char* name)
{
	if(!name || name[0] == 0)
		return 0;
	auto i = find(name);
	if(i)
		return i;
	auto n = zlen(name) + 1;
	strings.reserve(strings.count + n);
	auto p = strings.data + strings.count;
	memcpy(p, name, n);
	indecies.reserve();
	indecies.data[indecies.count++] = strings.count;
	strings.count += n;
	return indecies.count;
}

const char* tagdata::get(int id) const
{
	if(!id || id > indecies.count)
		return "";
	auto i = indecies.data[id-1];
	if(i >= strings.count)
		return "";
	return strings.data + i;
}