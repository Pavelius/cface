#include "color.h"

#pragma once

struct settings
{
	enum types {
		Group,
		Bool, Int, Radio,
		Button, Color,
		Text, TextPtr, UrlFolder, UrlFolderPtr,
		Control,
	};
	const char*		identifier;
	const char*		name[2];
	settings*		next;
	types			type;
	int				value;
	void*			data;
	int				priority;
	void(*e_execute)();
	bool(*e_visible)(settings& e);
	//
	void*			operator new(unsigned size);
	void			operator delete(void* p);
	//
	settings&		add(const char* name_en, const char* name_ru, bool& value);
	settings&		add(const char* name_en, const char* name_ru, int& value);
	settings&		add(const char* name_en, const char* name_ru, int& value, int current);
	settings&		add(const char* name_en, const char* name_ru, char* value, types type = Text);
	settings&		add(const char* name_en, const char* name_ru, char** value, types type = Text);
	settings&		add(const char* name_en, const char* name_ru, color& value);
	settings&		add(const char* name_en, const char* name_ru, void(*fn)());
	void			clear();
	settings*		child();
	settings&		execute(void(*fn)());
	settings*		find(const char* identifier); // Find setting by identifier
	settings&		gr(const char* name_en, const char* name_ru, int priority = 0); // Add new group
	settings&		maximum(int value);
	settings&		visible(bool(*fn)(settings& e));
	static settings	root;
};