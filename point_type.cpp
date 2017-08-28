#include "point.h"
#include "xsfield.h"

xsfield point_type[] = {
	BSREQ(point, x, number_type),
	BSREQ(point, y, number_type),
	{0}
};