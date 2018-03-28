#pragma once

struct string {
	const char*		text;
	const char*		end;
	//
	constexpr string() : text(""), end(text) {}
	constexpr string(const char* text, int size) : text(text), end(text + size) {}
	string(const char* text);
	operator bool() { return text != end; }
	bool			operator==(const string& e) const;
	bool			operator!=(const string& e) const;
	bool			operator!=(const char* e) const;
	//
	void			clear() { text = ""; end = text; }
	int				lenght() const { return end - text; }
	inline char*	paste(char* p1) const { return paste(p1, lenght()); }
	char*			paste(char* p1, int max_size) const;
	const char*		tostring() const;
	string			trimr() const;
};