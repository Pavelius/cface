#include "color.h"

#pragma once

namespace draw {
	struct control;
}
struct settings {
	enum types {
		Group,
		Bool, Int, Radio,
		Button, Color,
		TextPtr,
		UrlFolderPtr,
		Control,
	};
	const char*		identifier;
	const char*		name;
	settings*		next;
	types			type;
	int				value;
	void*			data;
	int				priority;
	void(*e_execute)();
	bool(*e_visible)(settings& e);
	//
	settings&		add(const char* name_ru, bool& value);
	settings&		add(const char* name_ru, int& value);
	settings&		add(const char* name_ru, int& value, int current);
	settings&		add(const char* name_ru, const char* &value, types type = TextPtr);
	settings&		add(const char* name_ru, color& value);
	settings&		add(const char* name_ru, draw::control& value);
	settings&		add(const char* name_ru, void(*fn)());
	settings*		child();
	settings&		execute(void(*fn)());
	settings*		find(const char* identifier); // Find setting by identifier
	settings&		gr(const char* name_ru, int priority = 0); // Add new group
	settings&		maximum(int value);
	settings&		visible(bool(*fn)(settings& e));
	static settings	root;
};