#include "point.h"
#include "bsreq.h"

bsreq point_type[] = {
	BSREQ(point, x, number_type),
	BSREQ(point, y, number_type),
	{0}
};