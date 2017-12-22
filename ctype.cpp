#include "agrw.h"
#include "ctype.h"

ctype ctype::root;
ctype ctype::pointers;
ctype ctype::ints;
ctype ctype::intu;
ctype ctype::intsu;
ctype ctype::intss;
ctype ctype::chars;
ctype ctype::charu;
ctype ctype::charp;
ctype ctype::voidx;
ctype ctype::voidp;
ctype ctype::boolx;

static agrw<ctype> globals;

void* ctype::operator new(unsigned size)
{
	for(auto p = &globals; p; p = p->next)
	{
		for(auto& e : *p)
		{
			if(!e)
				return &e;
		}
	}
	return globals.add();
}

void ctype::operator delete(void* p)
{
	((ctype*)p)->name = 0;
}

static void create(ctype& e, const char* name)
{
	e.clear();
	e.name = szdup(name);
}

static void pointer(ctype& e, const ctype& result)
{
	e.clear();
	e.name = ctype::pointers.name;
	e.result = (ctype*)&result;
	if(ctype::pointers.child)
		seqlast(ctype::pointers.child)->next = &e;
	else
		ctype::pointers.child = &e;
}

void ctype::clear()
{
	memset(this, 0, sizeof(ctype));
}

void ctype::initialize()
{
	globals.clear();
	create(root, ".");
	create(pointers, "*");
	//
	create(chars, "char");
	create(charu, "unsigned char");
	create(intss, "short");
	create(intsu, "unsigned short");
	create(ints, "int");
	create(intu, "unsigned int");
	create(voidx, "void");
	create(boolx, "bool");
	pointer(charp, chars);
	pointer(voidp, voidx);
}

ctype* ctype::find(const char* name)
{
	for(ctype* p = child; p; p = p->next)
	{
		if(strcmp(p->name, name)==0)
			return p;
	}
	return 0;
}

ctype* ctype::dereference() const
{
	if(!this || name!=pointers.name)
		return &voidx;
	return result;
}

ctype* ctype::reference() const
{
	for(ctype* m = ctype::pointers.child; m; m = m->next)
	{
		if(m->result==this)
			return m;
	}
	ctype* n = new ctype;
	pointer(*n, *this);
	return n;
}

char* ctype::tostring(char* buffer) const
{
	if(name==pointers.name)
	{
		dereference()->tostring(buffer);
		zcat(buffer, "*");
	}
	else
	{
		zcpy(buffer, name);
		if(count)
			szprint(zend(buffer), "[%1i]", count);
	}
	return buffer;
}

ctype* ctype::add(unsigned flags, const char* name, ctype& result)
{
	name = szdup(name);
	ctype* n = find(name);
	if(!n)
	{
		n = new ctype;
		if(child)
			seqlast(child)->next = n;
		else
			child = n;
		n->clear();
	}
	n->name = name;
	n->result = &result;
	n->flags = flags;
	return n;
}