#pragma once

struct stringtree {
	const char*		id;
	const char*		text;
	stringtree*		elements;
	//
	operator bool() const { return id != 0; }
	const char*		get(const char* id) const;
	stringtree*		find(const char* id) const;
};