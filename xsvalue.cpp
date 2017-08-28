#include "xsvalue.h"

int xsvalue::get() const
{
	switch(type)
	{
	case Value8: return *((char*)data);
	case Value16: return *((short*)data);
	case Value32: return *((int*)data);
	default: return 0;
	}
}

const char* xsvalue::gets() const
{
	switch(type)
	{
	case ValueText: return *((const char**)data);
	default: return "";
	}
}

unsigned xsvalue::getsize() const
{
	switch(type)
	{
	case Value8: return sizeof(char);
	case Value16: return sizeof(short);
	case Value32: return sizeof(int);
	case ValueText: return sizeof(const char*);
	default: return 0;
	}
}