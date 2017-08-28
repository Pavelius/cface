#include "base.h"
#include "calculator.h"

int calculator::getconstant(const char* name)
{
	return bstag(name);
}

int calculator::getfunction(const char* name, calculator& e, calculator& parameters, int count)
{
	return 0;
}

int	calculator::getindirection(int e1, const char* name)
{
	return bsget(e1, bstag(name));
}