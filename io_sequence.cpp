#include "crt.h"
#include "io.h"

io::sequence::sequence(io::stream& parent) :parent(parent), cashed_count(0) {
}

int	io::sequence::read(void* result, int count) {
	auto r = 0;
	auto c = imin(cashed_count, count);
	if(c) {
		memcpy(result, cashed, c);
		cashed_count -= c;
		if(cashed_count)
			memcpy(cashed, cashed + c, cashed_count);
		r += c;
	}
	count -= c;
	if(count)
		r += parent.read((char*)result + c, count);
	return r;
}

int	io::sequence::write(const void* result, int count) {
	return parent.write(result, count);
}

int io::sequence::seek(int count, int rel) {
	cashed_count = 0;
	return parent.seek(count, rel);
}

bool io::sequence::match(const char* value) {
	int count = zlen(value);
	makecashe(count + 1);
	if(cashed_count < count)
		return false;
	if(memcmp(cashed, value, count) != 0)
		return false;
	char sym = cashed[count];
	if(ischa(sym) || sym == '_')
		return false;
	cashed_count += count;
	return true;
}

void io::sequence::makecashe(int count) {
	if(count > sizeof(cashed) / sizeof(cashed[0]))
		count = sizeof(cashed) / sizeof(cashed[0]);
	if(cashed_count > count)
		return;
	cashed_count += parent.read(cashed + cashed_count, count - cashed_count);
}