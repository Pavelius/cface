#include "crt.h"
#include "stringlib.h"

inline const char* skipws(const char* p)
{
	while(*p == ' ' || *p == '\t')
		p++;
	return p;
}

inline const char* skip(const char* p, char sym)
{
	while(*p && *p!=sym)
		p++;
	return p;
}

stringlib::stringlib()
{
	clear();
}

stringlib::stringlib(const char* source)
{
	buffer.count = 0;
	clear();
	parse(source);
}

const char* stringlib::puttext(const char* p1, const char* p2)
{
	int original = buffer.count;
	while(p1 < p2)
	{
		if(buffer.count < sizeof(buffer.data) - 1)
			buffer.data[buffer.count++] = *p1;
		p1++;
	}
	buffer.data[buffer.count] = 0;
	if(buffer.count < sizeof(buffer.data) - 1)
		buffer.count++;
	return buffer.data + original;
}

void stringlib::parse(const char* p)
{
	stringkey* ps;
	if(!p)
		return;
	while(*p)
	{
		ps = add();
		ps->id = 0;
		ps->text = 0;
		if(!ps)
			return;
		p = skipws(p);
		if(*p == '{')
		{
			p = skipws(p + 1);
			if(*p != '}')
			{
				ps->id = p;
				p = skip(p, '}');
				ps->id = puttext(ps->id, p);
				p = skipws(p + 1);
			}
		}
		ps->text = p;
		p = skip(p, '\n');
		ps->text = puttext(ps->text, p);
		p = szskipcr(p);
	}
}

const stringkey* stringlib::find(const char* id) const
{
	for(auto& e : *this)
	{
		if(!e.id)
			continue;
		if(strcmp(e.id, id)==0)
			return &e;
	}
	return 0;
}

char* stringlib::printv(char* result, const char* id, const char* param, const char* prefix, const char* postfix) const
{
	auto p = find(id);
	result[0] = 0;
	if(!p)
		return result;
	if(prefix)
		zcat(result, prefix);
	szprintv(zend(result), p->text, param);
	if(postfix)
		zcat(result, postfix);
	return result;
}

char* stringlib::printn(char* result, const char* id, ...) const
{
	return printv(result, id, xva_start(id), 0, " ");
}

char* stringlib::print(char* result, const char* id, ...) const
{
	return printv(result, id, xva_start(id), 0, "\n");
}

char* stringlib::header(char* result, const char* id, ...) const
{
	return printv(result, id, xva_start(id), "###", "\n");
}

const char* stringlib::gettext(const char* id) const
{
	auto p = find(id);
	if(p)
		return p->text;
	return "";
}