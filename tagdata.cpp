#include "crt.h"
#include "tagdata.h"

void tagdata::clear() {
	indecies.clear();
	strings.clear();
}

int tagdata::find(const char* name) {
	auto count = indecies.getcount();
	auto data = strings.begin();
	for(unsigned i = 0; i < count; i++) {
		if(strcmp(data + indecies[i], name) == 0)
			return i + 1;
	}
	return 0;
}

int tagdata::add(const char* name) {
	if(!name || name[0] == 0)
		return 0;
	auto i = find(name);
	if(i)
		return i;
	auto m = strings.getcount();
	for(auto p = name; *p; p++)
		strings.add(*p);
	strings.add(0);
	indecies.add(m);
	return indecies.getcount();
}

const char* tagdata::get(int id) const {
	if(!id || id > (int)indecies.getcount())
		return "";
	return strings.begin() + indecies[id-1];
}