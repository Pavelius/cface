//Copyright 2017 by Pavel Chistyakov
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include "xsfield.h"

extern "C" int strcmp(const char* s1, const char* s2);

xsfield number_type[2] = {{"number"}, {0}};
xsfield text_type[2] = {{"text"}, {0}};
xsfield reference_type[2] = {{"reference"}, {0}};
xsfield xsfield_type[] = {
	BSREQ(xsfield, id, text_type),
	BSREQ(xsfield, offset, number_type),
	BSREQ(xsfield, size, number_type),
	BSREQ(xsfield, lenght, number_type),
	BSREQ(xsfield, count, number_type),
	BSREQ(xsfield, reference, number_type),
	BSREQ(xsfield, type, xsfield_type),
	{0}
};

const xsfield* xsfield::find(const char* name) const
{
	if(!this)
		return 0;
	for(auto p = this; p->id; p++)
	{
		if(strcmp(p->id, name) == 0)
			return p;
	}
	return 0;
}

int xsfield::get(const void* p) const
{
	switch(size)
	{
	case sizeof(char) : return *((char*)p);
	case sizeof(short) : return *((short*)p);
	default: return *((int*)p);
	}
}

void xsfield::set(const void* p, int value) const
{
	switch(size)
	{
	case sizeof(char) : *((char*)p) = value; break;
	case sizeof(short) : *((short*)p) = value; break;
	default: *((int*)p) = value; break;
	}
}