#pragma once

struct string
{
	const char*		text;
	const char*		end;
	//
	string() : text(""), end(text) {}
	string(const char* text);
	string(const char* text, int size) : text(text), end(text+size) {}
	operator bool() { return text && text!=end; }
	bool			operator==(const string& e) const;
	bool			operator!=(const string& e) const;
	bool			operator!=(const char* e) const;
	//
	void			clear() { text = ""; end = text; }
	static string	empthy;
	int				lenght() const { return end-text; }
	inline char*	paste(char* p1) const { return paste(p1, lenght()); }
	char*			paste(char* p1, int max_size) const;
	const char*		tostring() const;
	string			trimr() const;
};