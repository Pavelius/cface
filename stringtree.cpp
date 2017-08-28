#include "crt.h"
#include "stringtree.h"

stringtree* stringtree::find(const char* id) const
{
	if(!this)
		return 0;
	for(auto p = this; *p; p++)
	{
		// empthy string is reference to base type
		if(p->id[0] == 0)
		{
			// reference to base type
			auto p1 = p->elements->find(id);
			if(p1)
				return p1;
		}
		else if(strcmp(p->id, id) == 0)
			return const_cast<stringtree*>(p);
	}
	return 0;
}

const char* stringtree::get(const char* id) const
{
	auto p = find(id);
	if(!p)
		return "";
	return p->text;
}