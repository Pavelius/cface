#include "crt.h"
#include "grammar.h"

inline bool end(const char* s, const char* v)
{
	while(*v)
	{
		if(*s++!=*v++)
			return false;
	}
	return *s==*v;
}

const char* grammar::adjective::by(char* r, const char* s)
{
    const char* result = r;
    while(*s)
    {
    	if(end(s,"ый") || end(s,"ой"))
    	{
			zcpy(r, "ым");
			return result;
    	}
    	else if(end(s,"ий"))
    	{
			zcpy(r, "им");
			return result;
    	}
        *r++ = *s++;
    }
	*r++ = 0;
	return result;
}

char* grammar::noun::by(char* r, const char* s)
{
    char* result = r;
    while(*s)
    {
    	if(end(s,"ий"))
    	{
			zcpy(r, "ием");
			return result;
    	}
    	else if(end(s,"ий"))
    	{
			zcpy(r, "им");
			return result;
    	}
    	else if(end(s,"й") || end(s,"ь"))
    	{
			zcpy(r, "ем");
			return result;
    	}
    	else if(end(s,"е"))
    	{
			zcpy(r, "ем");
			return result;
    	}
    	else if(end(s,"а"))
    	{
			zcpy(r, "ой");
			return result;
    	}
		else if(end(s, "ч"))
		{
			zcpy(r, "чем");
			return result;
		}
		*r++ = *s++;
    }
    zcpy(r, "ом");
	return result;
}

char* grammar::noun::pluar::of(char* r, const char* s)
{
	char* result = r;
	while(true)
	{
		if(*s == 0)
		{
			zcpy(r, "ов");
			return result;
		}
		else if(end(s, "и"))
		{
			zcpy(r, "и");
			return result;
		}
		*r++ = *s++;
	}
	*r = 0;
	return result;
}

char* grammar::noun::of(char* r, const char* s)
{
    char* result = r;
    while(true)
    {
    	if(*s==0)
		{
			zcpy(r, "а");
			return result;
		}
		else if(end(s, "ь"))
		{
			zcpy(r, "я");
			return result;
		}
		else if(end(s, "о"))
		{
			zcpy(r, "о");
			return result;
		}
		else if(end(s, "я") || end(s, "и"))
		{
			zcpy(r, "и");
			return result;
		}
		else if(end(s, "а"))
		{
			zcpy(r, "ы");
			return result;
		}
		*r++ = *s++;
    }
	*r = 0;
	return result;
}