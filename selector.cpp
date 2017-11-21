#include "crt.h"
#include "selector.h"

bool selector::match(int v1) const
{
	switch(operation)
	{
	case Equal:
		return v1 == value;
	case NotEqual:
		return v1 != value;
	case Lesser:
		return v1<value;
	case LesserEqual:
		return v1 <= value;
	case Greater:
		return v1>value;
	case GreaterEqual:
		return v1 >= value;
	case Between:
		return v1 >= value && v1 <= value_to;
	case InList:
		return false;
	case NotInList:
		return true;
	default:
		return false;
	}
}

bool selector::match(const char* v1) const
{
	switch(operation)
	{
	case Equal:
		return szcmpi(v1, (const char*)value) == 0;
	case NotEqual:
		return szcmpi(v1, (const char*)value) == 0;
	case Lesser:
		return szcmpi(v1, (const char*)value)<0;
	case LesserEqual:
		return szcmpi(v1, (const char*)value) <= 0;
	case Greater:
		return szcmpi(v1, (const char*)value)>0;
	case GreaterEqual:
		return szcmpi(v1, (const char*)value) >= 0;
	case Between:
		return szcmpi(v1, (const char*)value) >= 0
			&& szcmpi(v1, (const char*)value_to) <= 0;
	case InList:
		return false;
	case NotInList:
		return true;
	default:
		return false;
	}
}