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
    	if(end(s,"��") || end(s,"��"))
    	{
			zcpy(r, "��");
			return result;
    	}
    	else if(end(s,"��"))
    	{
			zcpy(r, "��");
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
    	if(end(s,"��"))
    	{
			zcpy(r, "���");
			return result;
    	}
    	else if(end(s,"��"))
    	{
			zcpy(r, "��");
			return result;
    	}
    	else if(end(s,"�") || end(s,"�"))
    	{
			zcpy(r, "��");
			return result;
    	}
    	else if(end(s,"�"))
    	{
			zcpy(r, "��");
			return result;
    	}
    	else if(end(s,"�"))
    	{
			zcpy(r, "��");
			return result;
    	}
		else if(end(s, "�"))
		{
			zcpy(r, "���");
			return result;
		}
		*r++ = *s++;
    }
    zcpy(r, "��");
	return result;
}

char* grammar::noun::pluar::of(char* r, const char* s)
{
	char* result = r;
	while(true)
	{
		if(*s == 0)
		{
			zcpy(r, "��");
			return result;
		}
		else if(end(s, "�"))
		{
			zcpy(r, "�");
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
			zcpy(r, "�");
			return result;
		}
		else if(end(s, "�"))
		{
			zcpy(r, "�");
			return result;
		}
		else if(end(s, "�"))
		{
			zcpy(r, "�");
			return result;
		}
		else if(end(s, "�") || end(s, "�"))
		{
			zcpy(r, "�");
			return result;
		}
		else if(end(s, "�"))
		{
			zcpy(r, "�");
			return result;
		}
		*r++ = *s++;
    }
	*r = 0;
	return result;
}