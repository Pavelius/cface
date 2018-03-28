#include "crt.h"
#include "string.h"

string::string(const char* text) : text(text), end(text + zlen(text)) {
}

char* string::paste(char* p1, int max_size) const {
	int m = imin(max_size, lenght());
	memcpy(p1, text, m);
	p1[m] = 0;
	return p1;
}

bool string::operator==(const string& e) const {
	int size = end - text;
	if((e.end - e.text) != size)
		return false;
	const char* p1 = text;
	const char* p2 = e.text;
	if(p1 == p2)
		return true;
	while(p1 < end) {
		if(*p1++ != *p2++)
			return false;
	}
	return true;
}

bool string::operator!=(const string& e) const {
	int size = end - text;
	if((e.end - e.text) != size)
		return true;
	const char* p1 = text;
	const char* p2 = e.text;
	return memcmp(p1, p2, size) == 0;
}

bool string::operator!=(const char* p2) const {
	const char* p1 = text;
	while(p1 < end) {
		if(*p1++ != *p2++)
			return true;
	}
	return false;
}

string string::trimr() const {
	string res = *this;
	while(res.end > res.text) {
		switch(*res.end) {
		case '\n':
		case '\r':
		case ' ':
		case '\t':
			res.end--;
			continue;
		}
		break;
	}
	return res;
}

const char* string::tostring() const {
	char temp[1024];
	paste(temp, sizeof(temp));
	return szdup(temp);
}