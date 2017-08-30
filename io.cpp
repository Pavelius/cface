#include "crt.h"
#include "io.h"

io::stream& io::stream::operator<<(const int n)
{
	char temp[32];
	sznum(temp, n);
	return *this << temp;
}

io::stream&	io::stream::operator<<(const char* t)
{
	if(!t)
		return *this;
	// ѕриведем формат строки из стандартной кодировки
	while(*t)
	{
		char temp[8];
		char* s1 = temp;
		unsigned u = szget(&t);
		szput(&s1, u, CPUTF8);
		write(temp, s1 - temp);
	}
	return *this;
}

void io::stream::write(const char* text)
{
	int m = text ? zlen(text) : 0;
	write(m);
	if(m)
		write(text, m);
}

void io::stream::write(int value)
{
	write(&value, sizeof(value));
}

void io::stream::write(bool value)
{
	write(&value, sizeof(value));
}

template<> void io::stream::write<char>(const char& value)
{
	stream::write((int)value);
}

void io::stream::read(int& value)
{
	read(&value, sizeof(value));
}

unsigned io::stream::getsize()
{
	unsigned s = seek(0, SeekCur);
	unsigned r = seek(0, SeekEnd);
	seek(s, SeekSet);
	return r;
}

int io::stream::readtext(char* result, unsigned size)
{
	unsigned m = 0;
	result[0] = 0;
	read(&m, sizeof(m));
	if(!m)
		return 0;
	size = imin(m, size - 1);
	read(result, size);
	result[size] = 0;
	m -= size;
	if(m)
		seek(m, SeekCur);
	return size;
}

void* loadb(const char* url, int* size, int additional)
{
	void* p = 0;
	if(size)
		*size = 0;
	if(!url || url[0] == 0)
		return 0;
	io::file file(url, StreamRead);
	if(!file)
		return 0;
	int s = file.seek(0, SeekEnd) + additional;
	file.seek(0, SeekSet);
	p = new char[s];
	memset(p, 0, s);
	file.read(p, s);
	if(size)
		*size = s;
	return p;
}

char* loadt(const char* url, int* size)
{
	int s1;
	if(size)
		*size = 0;
	unsigned char* p = (unsigned char*)loadb(url, &s1, 1);
	if(!p)
		return 0;
	if(p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF)
	{
		// UTF8
		// ѕерекодируем блок через декодировщик. ћожет быть только меньше,
		// так как системна€ кодировка ANSI
		szencode((char*)p, s1, metrics::code, (char*)p + 3, s1, CPUTF8);
	}
	if(size)
		*size = s1;
	return (char*)p;
}