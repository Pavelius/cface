///////////////////////////////////////////////////////////////
// Lexical scanner
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

#include "cface/crt.h"
#include "lexer.h"

bool lexer::match(const char* text, const char* name)
{
	while(*name)
	{
		if(*name++ != *text++)
			return false;
	}
	if(ischa(*text) || isnum(*text) || *text == '_')
		return false;
	return true;
}

static int cmplex(const char* text, const char* name)
{
	while(*name)
	{
		if(*name != *text)
			return *text - *name;
	}
	if(ischa(*text) || isnum(*text) || *text == '_')
		return 1;
	return 0;
}

bool is_valid_identifier_symbol(char sym)
{
	return ischa(sym) || isnum(sym) || sym == '_';
}

static bool equal(const char* p1, const char* p2, int count, bool is_operator)
{
	while(count-- > 0)
	{
		if(*p1++ != *p2++)
			return false;
	}
	if(!is_operator && is_valid_identifier_symbol(*p1))
		return false;
	return true;
}

static const lexer::keyword* find_keyword(const char* text, lexer::keyword* base0, unsigned count)
{
	auto base = base0;
	auto last = base0 + count - 1;
	for(unsigned lim = count; lim != 0; lim >>= 1)
	{
		auto p = base + (lim >> 1);
		int cmp = text[0] - p->name[0];
		if(cmp == 0)
		{
			bool is_operator = !is_valid_identifier_symbol(text[0]);
			// Найдем хвост записей
			while(p < last && p[1].name[0] == text[0])
				p++;
			// От хвоста к началу найдем первое совпадение
			while(true)
			{
				if(equal(text, p->name, p->count, is_operator))
					return p;
				if(p <= base0 || p[-1].name[0] != text[0])
					return 0;
				p--;
			}
		}
		if(cmp > 0)
		{	/* key > p: move right */
			base = p + 1;
			lim--;
		} /* else move left */
	}
	return 0;
}

static int compare(const void* e1, const void* e2)
{
	return strcmp(((const lexer::keyword*)e1)->name,
		((const lexer::keyword*)e2)->name);
}

bool lexer::issym(char e)
{
    return ischa(e) || e == '_';
}

const lexer::keyword* lexer::find(lexer_tokens t) const
{
    for(auto& e : keywords)
    {
        if(e.token==t)
            return &e;
    }
    return 0;
}

const lexer::keyword* lexer::match(const char* name) const
{
	return find_keyword(name, keywords.data, keywords.count);
}

const char* lexer::word(const char* p, lexer_tokens& token, const lexer* scheme)
{
    if(!p[0])
    {
		token = LexerEof;
        return p;
    }
	else if(token==LexerCommentMultiline || (p[0]=='/' && p[1]=='*'))
    {
        // Cpp-style comments folds on 2 or more strings
        // So we need some special tags that fix this situation
        if(p[0]=='/' && p[1]=='*')
            p += 2;
		token = LexerComment;
        while(true)
        {
            if(*p==0)
                break;
            if(p[0]==13 || p[0]==10)
            {
				token = LexerCommentMultiline;
				p = szskipcr(p);
                break;
            }
            if(p[0]=='*' && p[1]=='/')
            {
                p += 2;
                break;
            }
            p++;
        }
        return p;
    }
	else if((token==LexerStringMultiline)
            || p[0]=='\"' || p[0]=='\''
            || ((p[0]=='L' || p[0]=='u') && p[1]=='\"'))
    {
        char e;
		if(token==LexerStringMultiline)
            e = '\"';
        else
            e = *p++;
		token = LexerString;
        while(true)
        {
            if(*p == e)
                return p + 1;
            else if(*p == '\n')
                return p + 1;
            else if(*p == '\r')
            {
                p++;
                if(*p == '\n')
                    p++;
                return p;
            }
            else if(*p == '\\')
            {
                p++;
                if(*p=='\n' || *p=='\r')
                {
					token = LexerStringMultiline;
                    return p;
                }
            }
            else if(*p == 0)
                return p;
            p++;
        }
    }
    else if(p[0]==' ' || p[0]=='\t')
    {
        // Standart white-spaces is tab symbol and space.
        // Spaces symbols count 'many as one'
		token = LexerWhiteSpace;
        p++;
        while(*p == ' ')
            p++;
    }
    else if(p[0]=='\n')
    {
		token = LexerLineFeed;
        p++;
    }
    else if(p[0]=='\r')
    {
		token = LexerLineFeed;
        p++;
        if(p[0]=='\n')
            p++;
    }
    else if(p[0]>='0' && p[0]<='9')
    {
		token = LexerNumber;
        if(p[0] == '0' && p[1] == 'x')
        {
            p += 2;
            while(true)
            {
                char a = *p;
                if((a >= '0' && a <= '9')
                        || (a >= 'A' && a <= 'F')
                        || (a >= 'a' && a <= 'f'))
                {
                    p++;
                    continue;
                }
                break;
            }
        }
        else
        {
            while(*p>='0' && *p<='9')
                p++;
            // Floating points have special format
            if(*p=='.')
            {
                p++;
                while(*p>='0' && *p<='9')
                    p++;
            }
            if(*p == 'f')
                p++;
            if(*p == 'L')
                p++;
        }
    }
    else if(p[0]=='/' && p[1]=='/')
    {
        p += 2;
		token = LexerComment;
        while(*p!=0 && *p!='\r' && *p!='\n')
            p++;
    }
    else if(p[0]=='#')
    {
        p++;
        while(*p && *p!='\n' && *p!='\r')
        {
            if(*p=='\\')
            {
                p++;
                if(*p==13)
                {
                    p = szskipcr(p);
                    continue;
                }
                else if(*p==10)
                {
                    p++;
                    continue;
                }
            }
            p++;
        }
		token = LexerPreprocessor;
    }
    else
    {
		if(scheme)
		{
			const keyword* k = scheme->match(p);
			if(k)
			{
				token = lexer_tokens(k->token);
				return p + k->count;
			}
		}
        if(issym(*p))
        {
            token = LexerIdentifier;
            while(issym(*p) || isnum(*p))
                p++;
            return p;
        }
		token = LexerOperator;
        return p + 1;
    }
    return p;
}

const char* lexer::prev(const char* p0, const char* p, lexer_tokens& t) const
{
	if(!p)
	{
		t = LexerEof;
		return 0;
	}
	const char* pr = 0;
	const char* p1 = szlineb(p0,szskipcrr(p0,szlineb(p0, p)));
    while(p1<p)
	{
		pr = p1;
		p1 = word(p1, t, this);
	}
	return pr;
}

lexer* lexer::first;

lexer::lexer(const char* name, const char** extensions, keyword* source) : name(name), extensions(extensions)
{
	seqlink(this);
	keywords.data = source;
	keywords.count = zlen(source);
	qsort(keywords.data, keywords.count, sizeof(keywords.data[0]), compare);
	for(auto& e : keywords)
		e.count = zlen(e.name);
}