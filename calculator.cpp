/* Copyright 2016 by Pavel Chistyakov
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "calculator.h"
#include "crt.h"

static const char* next(const char* p)
{
	return zskipspcr(p);
}

static const char* skip(const char* p, char sym)
{
	if(p[0] == sym)
		p = next(p + 1);
	return p;
}

static const char* expression(const char* p, int& e1, calculator& e);

static const char* unary(const char* p, int& e1, calculator& e)
{
	if(!p)
		return 0;
	switch(*p)
	{
	case '-':
		p = next(p + 1);
		p = unary(p, e1, e);
		if(!p)
			return 0;
		e1 = -e1;
		return p;
	case '+':
		p++;
		p = next(p + 1);
		return unary(p, e1, e);
	case '!':
		p = next(p + 1);
		p = unary(p, e1, e);
		if(!p)
			return 0;
		e1 = e1 ? 0 : 1;
		return p;
	case '(':
		p = next(p + 1);
		p = expression(p, e1, e);
		if(!p)
			return 0;
		if(p[0]==')')
			p = next(p + 1);
		return p;
	default:
		if(isnum(*p))
		{
			if(e.stop)
			{
				int e2;
				return next(psnum(p, e2));
			}
			return next(psnum(p, e1));
		}
		else
		{
			char temp[260];
			p = psidn(p, temp);
			p = next(p);
			if(p[0] == '(')
			{
				p = next(p + 1);
				int e2;
				calculator ec;
				ec.parent = &e;
				int index = 0;
				while(p && p[0] && p[0] != ')')
				{
					p = expression(p, e2, e);
					if(index < sizeof(ec.variables) / sizeof(ec.variables[0]))
						ec.variables[index++].value = e2;
					if(p[0] == ')')
						break;
					p = skip(p, ',');
				}
				if(p[0] == ')')
					p = next(p + 1);
				if(!e.stop)
					e1 = e.getfunction(temp, e, ec, index);
			}
			else
			{
				if(!e.stop)
				{
					e1 = 0;
					if(temp[0])
					{
						auto pv = e.find(temp);
						if(pv)
							e1 = pv->value;
						else
							e1 = calculator::getconstant(temp);
					}
				}
			}
			return p;
		}
	}
}

static const char* indirect(const char* p, int& e1, calculator& e)
{
	p = unary(p, e1, e);
	while(p && p[0] == '.')
	{
		char temp[260];
		p = psidn(p + 1, temp);
		p = next(p);
		if(!e.stop)
			e1 = calculator::getindirection(e1, temp);
	}
	return p;
}

static const char* multiplication(const char* p, int& e1, calculator& e)
{
	p = indirect(p, e1, e);
	while(p && (p[0] == '*' || p[0] == '/' || p[0] == '%') && p[1] != '=')
	{
		int e2;
		char t1 = p[0];
		p = next(p + 1);
		p = indirect(p, e2, e);
		if(!e.stop)
		{
			switch(t1)
			{
			case '*': e1 *= e2; break;
			case '/': e1 /= e2; break;
			case '%': e1 %= e2; break;
			}
		}
	}
	return p;
}

static const char* addiction(const char* p, int& e1, calculator& e)
{
	p = multiplication(p, e1, e);
	while(p && (p[0] == '+' || p[0] == '-') && p[1] != '=')
	{
		int e2;
		char t1 = p[0];
		p = next(p + 1);
		p = multiplication(p, e2, e);
		if(!e.stop)
		{
			switch(t1)
			{
			case '+': e1 += e2; break;
			case '-': e1 -= e2; break;
			}
		}
	}
	return p;
}

static const char* binary_cond(const char* p, int& e1, calculator& e)
{
	p = addiction(p, e1, e);
	while(p && ((p[0] == '>' && p[1] != '>') || (p[0] == '<' && p[1] != '<') || (p[0] == '=' && p[1] == '=') || (p[0] == '!' && p[1] == '=')))
	{
		int e2;
		char t1 = *p++;
		char t2 = 0;
		if(p[0] == '=')
			t2 = *p++;
		p = next(p);
		p = addiction(p, e2, e);
		if(!e.stop)
		{
			switch(t1)
			{
			case '<':
				if(t2 == '=')
					e1 = e1 <= e2;
				else
					e1 = e1 < e2;
				break;
			case '>':
				if(t2 == '=')
					e1 = e1 >= e2;
				else
					e1 = e1 > e2;
				break;
			case '=': e1 = (e1 == e2); break;
			case '!': e1 = (e1 != e2); break;
			}
		}
	}
	return p;
}

static const char* binary_and(const char* p, int& e1, calculator& e)
{
	p = binary_cond(p, e1, e);
	while(p && p[0] == '&' && p[1] != '&')
	{
		int e2;
		p = next(p + 2);
		p = binary_cond(p, e2, e);
		if(!e.stop)
			e1 &= e2;
	}
	return p;
}

static const char* binary_xor(const char* p, int& e1, calculator& e)
{
	p = binary_and(p, e1, e);
	while(p && p[0] == '^')
	{
		int e2;
		p = next(p + 1);
		p = binary_and(p, e2, e);
		if(!e.stop)
			e1 ^= e2;
	}
	return p;
}

static const char* binary_or(const char* p, int& e1, calculator& e)
{
	p = binary_xor(p, e1, e);
	while(p && p[0] == '|' && p[1] != '|')
	{
		int e2;
		p = next(p + 1);
		p = binary_xor(p, e2, e);
		if(!e.stop)
			e1 |= e2;
	}
	return p;
}

static const char* binary_shift(const char* p, int& e1, calculator& e)
{
	p = binary_or(p, e1, e);
	while(p && ((p[0] == '>' && p[1] == '>') || (p[0] == '<' && p[1] == '<')))
	{
		int e2;
		char t1 = p[0];
		p = next(p + 2);
		p = binary_or(p, e2, e);
		if(!e.stop)
		{
			switch(t1)
			{
			case '<': e1 <<= e2; break;
			case '>': e1 >>= e2; break;
			}
		}
	}
	return p;
}

static const char* logical_and(const char* p, int& e1, calculator& e)
{
	p = binary_shift(p, e1, e);
	while(p && p[0] == '&' && p[1] == '&')
	{
		int e2;
		p = next(p + 2);
		p = binary_shift(p, e2, e);
		if(!e.stop)
			e1 = e1 && e2;
	}
	return p;
}

static const char* logical_or(const char* p, int& e1, calculator& e)
{
	p = logical_and(p, e1, e);
	while(p && p[0] == '|' && p[1] == '|')
	{
		int e2;
		p = next(p + 2);
		p = logical_and(p, e2, e);
		if(!e.stop)
			e1 = e1 || e2;
	}
	return p;
}

static const char* expression(const char* p, int& e1, calculator& e)
{
	p = logical_or(p, e1, e);
	while(p && p[0] == '?')
	{
		bool stop = e.stop;
		bool correct = (e1 != 0);
		p = next(p + 1);
		e.stop = !correct;
		p = expression(p, e1, e);
		p = skip(p, ':');
		e.stop = correct;
		p = expression(p, e1, e);
		e.stop = stop;
	}
	return p;
}

calculator::calculator()
{
	clear();
}

void calculator::clear()
{
	memset(this, 0, sizeof(calculator));
}

calculator::var* calculator::find(const char* name)
{
	for(auto& e : variables)
	{
		if(!e.name)
			return 0;
		if(strcmp(e.name, name) == 0)
			return &e;
	}
	return 0;
}

void calculator::add(const char* name, int value)
{
	auto p = find(name);
	if(!p)
	{
		for(auto& e : variables)
		{
			if(e.name)
				continue;
			p = &e;
			break;
		}
	}
	if(p)
	{
		p->name = name;
		p->value = value;
	}
}

int calculator::evalute(const char* string)
{
	int e1 = -1;
	auto p = expression(string, e1, *this);
	return e1;
}