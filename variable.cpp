#include "variable.h"

void variable::clear()
{
	object = 0;
	size = 0;
}

int variable::get() const
{
	if(!object)
		return 0;
	switch(size)
	{
	case sizeof(char) : return *((char*)object);
	case sizeof(short) : return *((short*)object);
	case sizeof(int) : return *((int*)object);
	default: return 0;
	}
}

void variable::set(int value)
{
	if(!object)
		return;
	switch(size)
	{
	case sizeof(char) : *((char*)object) = value; break;
	case sizeof(short) : *((short*)object) = value; break;
	case sizeof(int) : *((int*)object) = value; break;
	default: break;
	}
}