#include "autogrow.h"
#include "crt.h"
#include "widget_settings.h"

settings settings::root;

void* settings::operator new(unsigned size)
{
	static autogrow<settings> globals;
	return globals.add();
}

void settings::operator delete(void* p)
{
	((settings*)p)->identifier = 0;
}

settings* settings::child()
{
	if(type != Group)
		return 0;
	return (settings*)data;
}

void settings::clear()
{
	settings* t = child();
	if(t)
		delete t;
	data = 0;
	memset(name, 0, sizeof(name));
	identifier = 0;
	seqclear(this);
}

settings* settings::find(const char* identifier)
{
	auto p = this;
	while(p)
	{
		if(strcmp(p->identifier,identifier)==0)
			return p;
		p = p->next;
	}
	return 0;
}

static settings& add_element(settings* e, const char* name_en, const char* name_ru, settings::types type, void* data)
{
	if(e->type != settings::Group)
		return *e;
	settings* e1;
	name_en = szdup(name_en);
	name_ru = szdup(name_ru);
	const char* identifier = sztag(name_en);
	if(e->data)
	{
		e1 = static_cast<settings*>(e->data)->find(identifier);
		if(e1)
		{
			if(e1->type != type)
			{
				if(e1->type == settings::Group)
					delete (settings*)e->data;
				e1->type = type;
			}
			if(e1->type != settings::Group)
				e1->data = data;
			return *e1;
		}
	}
	e1 = new settings;
	e1->name[0] = name_en;
	e1->name[1] = name_ru;
	e1->identifier = identifier;
	e1->type = type;
	e1->data = data;
	if(e->data)
		seqlast((settings*)e->data)->next = e1;
	else
		e->data = e1;
	return *e1;
}

settings& settings::gr(const char* name_en, const char* name_ru, int priority)
{
	if(!root.identifier)
	{
		root.identifier = szdup("Root");
		root.name[0] = root.identifier;
		root.name[1] = "Корень";
		root.type = Group;
	}
	if(priority == 0)
	{
		if(strcmp(name_en, "General") == 0)
			priority = 1;
		else
			priority = 10;
	}
	settings& e = add_element(this, name_en, name_ru, Group, 0);
	e.priority = priority;
	return e;
}

settings& settings::add(const char* name_en, const char* name_ru, bool& value)
{
	return add_element(this, name_en, name_ru, Bool, &value);
}

settings& settings::add(const char* name_en, const char* name_ru, int& value)
{
	settings& e = add_element(this, name_en, name_ru, Int, &value);
	e.value = 100;
	return e;
}

settings& settings::add(const char* name_en, const char* name_ru, char* value, types type)
{
	settings& e = add_element(this, name_en, name_ru, type, value);
	e.value = 260;
	return e;
}

settings& settings::add(const char* name_en, const char* name_ru, char** value, types type)
{
	settings& e = add_element(this, name_en, name_ru, type, value);
	e.value = 260;
	return e;
}

settings& settings::add(const char* name_en, const char* name_ru, int& value, int current)
{
	settings& e = add_element(this, name_en, name_ru, Radio, &value);
	e.value = current;
	return e;
}

settings& settings::add(const char* name_en, const char* name_ru, color& value)
{
	return add_element(this, name_en, name_ru, Color, &value);
}

settings& settings::add(const char* name_en, const char* name_ru, void(*fn)())
{
	settings& e = add_element(this, name_en, name_ru, Button, 0);
	e.e_execute = fn;
	return e;
}

settings& settings::execute(void(*fn)())
{
	e_execute = fn;
	return *this;
}

settings& settings::visible(bool(*fn)(settings& e))
{
	e_visible = fn;
	return *this;
}

settings& settings::maximum(int value)
{
	switch(type)
	{
	case Int:
	case Text:
	case TextPtr:
	case UrlFolder:
	case UrlFolderPtr:
		this->value = value;
		break;
	default:
		break;
	}
	return *this;
}