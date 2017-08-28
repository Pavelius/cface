#include "crt.h"
#include "base.h"

int bscompareas(const void* p1, const void* p2)
{
	return *((int*)p1) - *((int*)p2);
}

int bscompareds(const void* p1, const void* p2)
{
	return *((int*)p2) - *((int*)p1);
}

int bscomparenm(const void* p1, const void* p2)
{
	const char* e1 = bsgets(*((int*)p1), Name);
	const char* e2 = bsgets(*((int*)p2), Name);
	if(!e1)
		return -1;
	else if(!e2)
		return 1;
	return strcmp(e1, e2);
}

int* bssort(int* elements)
{
	int count = zlen(elements);
	qsort(elements, zlen(elements), sizeof(elements[0]), bscomparenm);
	return elements;
}

int bsfind(int rec, int i1, int v1, int i2, int v2)
{
	int last = bsget(rec, Last);
	while(rec<=last)
	{
		if(bsget(rec, i1)==v1 && bsget(rec, i2)==v2)
			return rec;
		rec++;
	}
	return 0;
}

bool bshierarhy(int rec, int par, int id)
{
	while(rec)
	{
		if(rec==par)
			return true;
		rec = bsget(rec, id);
	}
	return false;
}

int bsfirstby(int rec, int par, int id)
{
	while(rec)
	{
		int r1 = bsget(rec, id);
		if(r1==par)
			return rec;
		rec = r1;
	}
	return 0;
}

int* bsselect(int* result, int r1, int r2)
{
	int* a = result;
	if(r2==-1)
		r2 = bsget(r1, Last);
	for(int i = r1; i<=r2; i++)
		*a++ = i;
	*a = 0;
	return result;
}

int* bsselect(int* result, int r1, int r2, int i1, int v1)
{
	int* a = result;
	if(r2==-1)
		r2 = bsget(r1, Last);
	for(int i = r1; i<=r2; i++)
	{
		if(bsget(i, i1)!=v1)
			continue;
		*a++ = i;
	}
	*a = 0;
	return result;
}

int* bsselect(int* result, const int* source, int i1, int v1)
{
	int* p = result;
	while(true)
	{
		int r1 = *source++;
		if(!r1)
			break;
		if(bsget(r1, i1) != v1)
			continue;
		*p++ = r1;
	}
	*p++ = 0;
	return result;
}

void bsadd(int rec, int id, int value)
{
	bsset(rec, id, bsget(rec, id) + value);
}

void bsadd(int rec, int id, int value, int minimum)
{
	bsset(rec, id, imax(bsget(rec, id) + value, minimum));
}

void bsadd(int rec, int id, int value, int minimum, int maximum)
{
	bsset(rec, id, imin(imax(bsget(rec, id) + value, minimum), maximum));
}

int* bsuniq(int* result)
{
	int* s = result;
	int* p = result;
	while(true)
	{
		int v = *s++;
		if(!v)
			break;
		bool has_found = false;
		for(int* p1 = result; p1<p; p1++)
		{
			if(*p1==v)
			{
				has_found = true;
				break;
			}
		}
		if(!has_found)
			*p++ = v;
	}
	*p = 0;
	return result;
}