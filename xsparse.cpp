#include "crt.h"
#include "io.h"
#include "xsparse.h"

xsparse::fileinfo::fileinfo(const char* url, xsparse* parent) :source(0), url(szdup(url)), parent(parent)
{
	source = loadt(url);
	previous = parent->file;
	parent->file = this;
}

xsparse::fileinfo::~fileinfo()
{
	if(source)
		delete source;
	source = 0;
	parent->file = previous;
}

bool xsparse::fileinfo::isloaded(const char* url) const
{
	for(auto p = this; p; p = p->previous)
	{
		if(strcmp(p->url, url) == 0)
			return true;
	}
	return false;
}

xsparse::xsparse() : p(0), value(0), value_type(0), file(0)
{
	buffer[0] = 0;
}

void xsparse::skipws()
{
	while(*p)
	{
		if(p[0] == 9 || p[0] == 0x20)
		{
			p++;
			continue;
		}
		else if(p[0] == '\\')
		{
			p++;
			if(p[0] == 10 || p[0] == 13)
				p = szskipcr(p);
			else
				p++;
			continue;
		}
		else if(p[0] == '/' && p[1] == '/')
		{
			// Comments
			p = skipline(p + 2);
			continue;
		}
		break;
	}
}

const char* xsparse::skipline(const char* p)
{
	while(p[0] && p[0] != 10 && p[0] != 13)
		p++;
	return szskipcr(p);
}

void xsparse::readnumber()
{
	value = sz2num(p, &p);
	value_type = number_type;
	skipws();
}

void xsparse::readidentifier()
{
	auto pb = buffer;
	auto pe = pb + sizeof(buffer) - 1;
	while(*p && ((*p >= '0' && *p <= '9') || *p == '_' || ischa(*p)))
	{
		if(pb<pe)
			*pb++ = *p;
		p++;
	}
	*pb = 0;
	skipws();
}

void xsparse::readtrail()
{
	auto pb = buffer;
	auto pe = pb + sizeof(buffer) - 1;
	while(true)
	{
		auto sym = *p;
		if(!sym)
			break;
		if(sym == '\n' || sym == '\r')
		{
			while(*p == '\n' || *p == '\r')
			{
				p = szskipcr(p);
				skipws();
			}
			if(*p == '#')
				break;
			if(pb != buffer && pb < pe)
				*pb++ = '\n';
			continue;
		}
		if(pb < pe)
			*pb++ = sym;
		p++;
	}
	*pb = 0;
}

bool xsparse::parseidentifier(xsbase* create_base)
{
	if(!buffer[0])
		return false;
	auto e = create_base ? create_base->find("id", buffer) : xsbase::getref(buffer);
	if(!e && create_base)
	{
		if(create_base->fields->iskey() && create_base->fields->type == text_type)
		{
			e.fields = create_base->fields;
			e.object = (void*)create_base->add();
			e.set("id", (int)szdup(buffer));
		}
		else if(*p!='(')
			status(ErrorXSNotFoundField, buffer);
	}
	if(!e)
	{
		auto xs = xsbase::find(buffer);
		if(xs)
		{
			e.object = (void*)xs->fields;
			e.fields = xsfield_type;
		}
	}
	value = (int)e.object;
	value_type = e.fields;
	return true;
}

void xsparse::readvalue(xsbase* create_base)
{
	value = 0;
	value_type = 0;
	buffer[0] = 0;
	auto pb = buffer;
	auto pe = pb + sizeof(buffer) - 1;
	if(*p == '\"')
	{
		p++;
		while(*p)
		{
			if(*p == '\"')
			{
				p++;
				break;
			}
			if(*p == '\\')
			{
				p++;
				switch(*p)
				{
				case 'n':
					if(pb < pe)
						*pb++ = '\n';
					break;
				case 'r':
					if(pb < pe)
						*pb++ = '\r';
					break;
				default:
					if(pb < pe)
						*pb++ = *p;
					break;
				}
			}
			else if(pb < pe)
				*pb++ = *p;
			p++;
		}
		*pb = 0;
		skipws();
		value_type = text_type;
		if(create_base)
			parseidentifier(create_base);
	}
	else if(*p == '-' || isnum(*p))
	{
		readnumber();
		if(create_base)
		{
			if(value < 0)
				value = 0;
			if((int)create_base->count <= value)
				create_base->count = value + 1;
			value = (int)create_base->get(value);
			value_type = create_base->fields;
		}
	}
	else if(ischa(*p))
	{
		readidentifier();
		parseidentifier(create_base);
	}
}

const xsfield* xsparse::readfield(const xsfield* fields)
{
	readidentifier();
	if(!buffer[0])
		return 0;
	auto pf = fields->find(buffer);
	if(!pf)
		status(ErrorXSNotFoundField);
	return pf;
}

void xsparse::setvalue(xsref e, const xsfield* field, int index)
{
	if(!field)
		return;
	if(field->type == number_type)
	{
		if(value_type == number_type)
			e.set(field->id, value, index);
		if(value_type == text_type)
			e.set(field->id, sz2num(buffer), index);
		else
		{
			auto xs = xsbase::find(value_type);
			if(xs)
				e.set(field->id, xs->indexof((void*)value));
		}
	}
	else if(field->type == text_type)
	{
		if(buffer[0])
			e.set(field->id, (int)szdup(buffer), index);
	}
	else if(field->reference==0)
	{
		// Случай объекта
		if(field->type)
		{
			int count = zlen(field->type);
			if(count > 0)
			{
				xsref e1 = e.getr(field->id, index / count);
				if(e1.fields)
					setvalue(e1, e1.fields + index%count);
			}
		}
	}
	else
	{
		if(value_type == number_type)
		{
			auto xs = xsbase::find(field->type);
			if(xs)
				value = (int)xs->get(value);
		}
		e.set(field->id, value, index);
	}
}

void xsparse::readattribute(xsref e)
{
	auto pf = readfield(e.fields);
	if(*p == '(')
	{
		p++;
		skipws();
		int index = 0;
		while(*p)
		{
			readvalue();
			setvalue(e, pf, index);
			if(*p != ',')
				break;
			p++;
			skipws();
			index++;
		}
		if(*p == ')')
		{
			p++;
			skipws();
		}
	}
	else if(pf && pf->type == number_type)
		e.set(pf->id, 1);
	else
	{
		status(ErrorXSExpected, "(");
		p = skipline(p);
	}
}

bool xsparse::isentity(const char* id)
{
	if(p[0] != '#')
		return false;
	auto ps = p + 1;
	while(*ps)
	{
		if(*id == 0)
		{
			if(ischa(*ps) || isnum(*ps) || *ps == '_')
				return false;
			p = ps;
			skipws();
			return true;
		}
		if(*ps++ != *id++)
			return false;
	}
	return false;
}

void xsparse::readelements(xsref parent)
{
	auto elements = parent.fields->find("elements");
	if(!elements)
		return;
	auto xse = xsbase::find(elements->type);
	if(!xse)
		return;
	parent.set("elements", 0);
	bool need_save_element = true;
	while(isentity("element"))
	{
		xsref ee = {xse->fields, xse->add()};
		if(need_save_element)
		{
			need_save_element = false;
			parent.set("elements", (int)ee.object);
		}
		while(*p && *p != 10 && *p != 13)
			readattribute(ee);
		readtrail();
		auto pf = ee.fields->find("text");
		if(pf)
			setvalue(ee, pf);
	}
	xsref ee = {xse->fields, xse->add()};
	ee.clear();
}

void xsparse::read()
{
	p = file->source;
	while(p[0] == '#')
	{
		p = p + 1;
		readidentifier();
		if(strcmp(buffer, "include")==0)
		{
			readvalue();
			if(value_type == text_type && !file->isloaded(buffer))
			{
				fileinfo e(buffer, this);
				if(e)
				{
					auto p_old = p;
					read();
					p = p_old;
				}
			}
			p = skipline(p);
			skipws();
			continue;
		}
		auto xs = xsbase::find(buffer);
		if(xs)
		{
			const char* pbf = p;
			readvalue(xs);
			xsref e = {value_type, (void*)value};
			if(*p == '(')
			{
				p = pbf; // Roll back
				e = {xs->fields, xs->add()};
			}
			while(*p && *p != 10 && *p != 13)
				readattribute(e);
			readtrail();
			auto pf = e.fields->find("text");
			if(pf)
				setvalue(e, pf);
			readelements(e);
		}
		else
		{
			auto e = xsglobal::getref(buffer);
			if(e)
			{
				while(*p && *p != 10 && *p != 13)
					readattribute(e);
				readtrail();
				auto pf = e.fields->find("text");
				if(pf)
					setvalue(e, pf);
			}
			else
			{
				status(ErrorXSNotFoundCommand);
				p = skipline(p);
				readtrail();
			}
		}
	}
	p = 0;
}

bool xsparse::read(const char* filename)
{
	fileinfo e(filename, this);
	if(!e)
		return false;
	read();
	return true;
}

void xsparse::readfolder(const char* filename)
{
	xsparse parser;
	for(io::file::find fs("classes"); fs; fs.next())
	{
		if(fs.name()[0] == '.')
			continue;
		auto p = szext(fs.name());
		if(!p || szcmpi(p, "txt") != 0)
			continue;
		parser.read(fs.fullname(buffer));
	}
}

void xsparse::status(xserrors error, ...)
{
}