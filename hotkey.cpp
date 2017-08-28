#include "hotkey.h"

int mapkey(hotkey* keys, int id)
{
	if(!id)
		return 0;
	if(!keys)
		return id;
	for(hotkey* p = keys; p->key; p++)
	{
		if(p->key == id)
			return p->id;
	}
	return id;
}