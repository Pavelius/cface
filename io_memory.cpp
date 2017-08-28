#include "crt.h"
#include "io.h"

io::memory::memory(void* data, int size) : data((unsigned char*)data), pos(0), size(size)
{
}

int io::memory::read(void* p, int size)
{
	if(pos>=this->size)
		return 0;
	if(pos+size<this->size)
		size = this->size - pos;
	memcpy(p, data+pos, size);
    return size;
}

int io::memory::write(const void* p, int size)
{
	if(pos>=this->size)
		return 0;
	if(pos+size<this->size)
		size = this->size - pos;
	memcpy(data+pos, p, size);
    return size;
}

int io::memory::seek(int pos, int rel)
{
	switch(rel)
	{
	case SeekCur:
		this->pos = imin(size, imax(0, this->pos + pos));
		break;
	case SeekEnd:
		this->pos = imin(size, imax(0, size+pos));
		break;
	case SeekSet:
		this->pos = imin(size, imax(0, pos));
		break;
	}
    return this->pos;
}