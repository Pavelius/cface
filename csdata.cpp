#include "csdata.h"

using namespace csdata;

bsreq typeinfo_type[] = {
	BSREQ(typeinfo, id, text_type),
	BSREQ(typeinfo, name, text_type),
	BSREQ(typeinfo, parent, typeinfo_type),
{}
};
adat<typeinfo, 256> typeinfo_data; BSMETA(typeinfo);

typeinfo csdata::type_root[] = {"type", "���"};
typeinfo csdata::enum_root[] = {"enum", "������������"};

bsreq requisit_type[] = {
	BSREQ(requisit, id, text_type),
	BSREQ(requisit, name, text_type),
	BSREQ(requisit, parent, typeinfo_type),
	BSREQ(requisit, type, typeinfo_type),
{}
};
adat<requisit, 8196> requisit_data; BSMETA(requisit);