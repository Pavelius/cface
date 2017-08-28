#include "tagdata.h"

tagdata global_strings;

int getsid(const char* text)
{
	return global_strings.add(text);
}

const char* getstr(int id)
{
	return global_strings.get(id);
}