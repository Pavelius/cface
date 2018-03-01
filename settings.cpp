#include "agrw.h"
#include "crt.h"
#include "settings.h"

settings settings::root;
static agrw<settings, 128>	globals;

settings* settings::child() {
	if(type != Group)
		return 0;
	return (settings*)data;
}

settings* settings::find(const char* identifier) {
	auto p = this;
	while(p) {
		if(strcmp(p->identifier, identifier) == 0)
			return p;
		p = p->next;
	}
	return 0;
}

static settings& add_element(settings* e, const char* name_ru, settings::types type, void* data) {
	if(e->type != settings::Group)
		return *e;
	settings* e1;
	name_ru = szdup(name_ru);
	const char* identifier = sztag(name_ru);
	if(e->data) {
		e1 = static_cast<settings*>(e->data)->find(identifier);
		if(e1) {
			if(e1->type != type) {
				if(e1->type == settings::Group)
					delete (settings*)e->data;
				e1->type = type;
			}
			if(e1->type != settings::Group)
				e1->data = data;
			return *e1;
		}
	}
	e1 = globals.add();
	e1->name = name_ru;
	e1->identifier = identifier;
	e1->type = type;
	e1->data = data;
	if(e->data)
		seqlast((settings*)e->data)->next = e1;
	else
		e->data = e1;
	return *e1;
}

settings& settings::gr(const char* name_ru, int priority) {
	if(!root.identifier) {
		root.identifier = szdup("Root");
		root.name = root.identifier;
		root.type = Group;
	}
	if(priority == 0) {
		if(strcmp(name, "Общие") == 0)
			priority = 1;
		else
			priority = 10;
	}
	settings& e = add_element(this, name_ru, Group, 0);
	e.priority = priority;
	return e;
}

settings& settings::add(const char* name_ru, bool& value) {
	return add_element(this, name_ru, Bool, &value);
}

settings& settings::add(const char* name_ru, int& value) {
	settings& e = add_element(this, name_ru, Int, &value);
	e.value = 100;
	return e;
}

settings& settings::add(const char* name_ru, const char* &value, types type) {
	settings& e = add_element(this, name_ru, type, &value);
	e.value = 260;
	return e;
}

settings& settings::add(const char* name_ru, int& value, int current) {
	settings& e = add_element(this, name_ru, Radio, &value);
	e.value = current;
	return e;
}

settings& settings::add(const char* name_ru, color& value) {
	return add_element(this, name_ru, Color, &value);
}

settings& settings::add(const char* name_ru, void(*fn)()) {
	auto& e = add_element(this, name_ru, Button, 0);
	e.e_execute = fn;
	return e;
}

settings& settings::add(const char* name_ru, draw::control& value) {
	auto& e = add_element(this, name_ru, Control, &value);
	e.priority = 10;
	return e;
}

settings& settings::visible(bool(*fn)(settings& e)) {
	e_visible = fn;
	return *this;
}

settings& settings::maximum(int value) {
	switch(type) {
	case Int:
	case TextPtr:
	case UrlFolderPtr:
		this->value = value;
		break;
	default:
		break;
	}
	return *this;
}