#include "crt.h"
#include "drawable.h"

drawable::plugin* drawable::plugin::first;

drawable::plugin::plugin()
{
	seqlink(this);
}

static int compare_zorder(const void* p1, const void* p2)
{
	drawable* e1 = *((drawable**)p1);
	drawable* e2 = *((drawable**)p2);
	point s1 = e1->getzpos();
	point s2 = e2->getzpos();
	if(s1.y!=s2.y)
		return s1.y - s2.y;
	if(s1.x!=s2.x)
		return s1.x - s2.x;
	return e1->priority() - e2->priority();
}

drawable** dworder(drawable** da, int count)
{
	qsort(da, count, sizeof(da[0]), compare_zorder);
	return da;
}

drawable** dwselect(drawable** da, unsigned flags)
{
	auto pe = da;
	pe[0] = 0;
	for(auto p = drawable::plugin::first; p; p = p->next)
	{
		p->selecting(pe, flags);
		pe = zend(pe);
	}
	return da;
}

void dwupdate()
{
	for(auto p = drawable::plugin::first; p; p = p->next)
		p->updating();
}

int dwclipping(drawable** da, rect rc, point camera)
{
	drawable** pd = da;
	rc.move(camera.x, camera.y);
	for(drawable** ps = da; *ps; ps++)
	{
		if(!(*ps)->getrect().intersect(rc))
			continue;
		*pd++ = *ps;
	}
	*pd = 0;
	return pd-da;
}

void dwpaint(drawable** da, rect screen, point camera)
{
	camera.x -= screen.x1;
	camera.y -= screen.x1;
	while(*da)
	{
		drawable* p = *da++;
		p->painting(camera);
	}
}

drawable* dwfind(drawable** da, rect screen, point camera, point hitpoint)
{
	if(!hitpoint.in(screen))
		return 0;
	camera.x -= screen.x1;
	camera.y -= screen.x1;
	drawable* result = 0;
	while(*da)
	{
		drawable* p = *da++;
		if(p->hittest(camera, hitpoint))
			result = p;
	}
	return result;
}