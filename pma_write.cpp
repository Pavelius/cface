#include "cface/crt.h"
#include "cface/io.h"
#include "cface/tagdata.h"
#include "pma.h"

void write(io::stream& file, tagdata& e)
{
	pma header;
	setsignature(header.name, "STR");
	header.count = e.indecies.count;
	header.size = sizeof(pma) + header.count * sizeof(int);
	int string_offset = header.size;
	header.size += e.strings.count;
	file.write(&header, sizeof(header));
	for(int i = 0; i < e.indecies.count; i++)
		file.write(e.indecies.data[i] + string_offset);
	file.write(e.strings.data, e.strings.count * sizeof(e.strings.data[0]));
}