#include "crt.h"
#include "xsglobal.h"

xsglobal* xsglobal::first;

xsglobal::xsglobal(const char* id, void* object, const xsfield* fields) : id(id)
{
	xsref::object = object;
	xsref::fields = fields;
	seqlink(this);
}

xsref xsglobal::getref(const char* id)
{
	for(auto xs = first; xs; xs = xs->next)
	{
		if(strcmp(xs->id, id) == 0)
			return *xs;
	}
	return{0};
}

xsglobal* xsglobal::find(const char* id)
{
	for(auto xs = first; xs; xs = xs->next)
	{
		if(strcmp(xs->id, id) == 0)
			return xs;
	}
	return 0;
}