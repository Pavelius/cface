///////////////////////////////////////////////////////////////
// Code Editor (and code analyzer)
//
// Copyright (c) 2013 Pavel Chistiakov
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "crt.h"
#include "lexer.h"

struct type
{
	string				name;
	unsigned			flags;
	type*				next;
	type*				child;
	type*				result;
	//
	static type			root;
	static type			xthis;
	static type			xchar;
	static type			uchar;
	static type			usint;
	static type			xsint;
	static type			xint;
	static type			uint;
	static type			xbool;
	static type			xvoid;
	static type			pointers;
	//
	lexer::type*		add(string name, lexer::type* result, unsigned flags);
	static lexer::type*	add(type** parent, string name, lexer::type* result, unsigned flags);
	type*				dereference() const;
	char*				getname(char* buffer) const;
	static void			initialize();
	type*				find(string name);
	type*				findtype(string name);
	type*				reference() const;
};

lexer::type lexer::type::root;
lexer::type lexer::type::xchar;
lexer::type lexer::type::xsint;
lexer::type lexer::type::xint;
lexer::type lexer::type::xvoid;
lexer::type lexer::type::xbool;
lexer::type lexer::type::xthis;
lexer::type lexer::type::uchar;
lexer::type lexer::type::usint;
lexer::type lexer::type::uint;
lexer::type lexer::type::pointers;
static string pointer_name("*");
autogrow<lexer::type> lexer_types;

static void create(lexer::type& e, string name)
{
	e.name = name;
	e.flags = 0;
	e.child = 0;
	e.next = 0;
	e.result = 0;
}

lexer::type* lexer::type::add(string name, lexer::type* result, unsigned flags)
{
	auto p = find(name);
	if(!p)
	{
		p = lexer_types.add();
		create(*p, name);
		p->result = result;
		p->flags = flags;
		if(child)
			seqlast(child)->next = p;
		else
			child = p;
	}
	return p;
}

lexer::type* lexer::type::add(lexer::type** parent, string name, lexer::type* result, unsigned flags)
{
	for(auto p = *parent; p; p = p->next)
	{
		if(p->name == name)
			return p;
	}
	auto p = lexer_types.add();
	create(*p, name);
	p->result = result;
	p->flags = flags;
	if(*parent)
		seqlast(*parent)->next = p;
	else
		*parent = p;
	return p;
}

void lexer::type::initialize()
{
	lexer_types.clear();
	//
	create(root, ".");
	create(pointers, "*");
	//
	create(xthis, "this");
	create(xchar, "char");
	create(uchar, "unsigned char");
	create(xsint, "short");
	create(usint, "unsigned short");
	create(xint, "int");
	create(uint, "unsigned int");
	create(xvoid, "void");
	create(xbool, "bool");
}

lexer::type* lexer::type::find(string name)
{
	for(type* p = child; p; p = p->next)
	{
		if(p->name==name)
			return p;
	}
	return 0;
}

lexer::type* lexer::type::findtype(string name)
{
	for(type* p = child; p; p = p->next)
	{
		if(p->flags != LexerType)
			return 0;
		if(p->name == name)
			return p;
	}
	return 0;
}

lexer::type* lexer::type::dereference() const
{
	if(!this)
		return 0;
	if(name!=pointer_name)
		return 0;
	return result;
}

lexer::type* lexer::type::reference() const
{
	for(type* m = type::pointers.child; m; m = m->next)
	{
		if(m->result==this)
			return m;
	}
	type* n = new type;
	create(*n, pointer_name);
	n->result = (lexer::type*)this;
	if(type::pointers.child)
		seqlast(type::pointers.child)->next = n;
	else
		type::pointers.child = n;
	return n;
}

char* lexer::type::getname(char* buffer) const
{
	if(name==pointer_name)
	{
		dereference()->getname(buffer);
		zcat(buffer, "*");
	}
	else
		name.paste(buffer);
	return buffer;
}