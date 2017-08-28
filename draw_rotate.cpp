/* Copyright 2013 by Pavel Chistyakov
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "draw.h"

struct aar_pnt
{
	double	x, y;
	inline	aar_pnt() {}
	inline	aar_pnt(double x, double y) :x(x), y(y) {}
};

struct aar_dblrgbquad
{
	double	red, green, blue, alpha;
};

struct aar_indll
{
	aar_indll*	next;
	int			ind;
};

//Prevent Float Errors with Cos and Sin
static double aar_cos(double degrees)
{
	double ret;
	double off = (degrees / 30 - round(degrees / 30));
	if(off < .0000001 && off > -.0000001)
	{
		int idegrees = (int)round(degrees);
		idegrees = (idegrees < 0) ? (360 - (-idegrees % 360)) : (idegrees % 360);
		switch(idegrees)
		{
		case 0: ret = 1.0; break;
		case 30: ret = 0.866025403784439; break;
		case 60: ret = 0.5; break;
		case 90: ret = 0.0; break;
		case 120: ret = -0.5; break;
		case 150: ret = -0.866025403784439; break;
		case 180: ret = -1.0; break;
		case 210: ret = -0.866025403784439; break;
		case 240: ret = -0.5; break;
		case 270: ret = 0.0; break;
		case 300: ret = 0.5; break;
		case 330: ret = 0.866025403784439; break;
		case 360: ret = 1.0; break;
		default: ret = cos(degrees * 3.14159265358979 / 180);  // it shouldn't get here
		}
		return ret;
	}
	else
		return cos(degrees * 3.14159265358979 / 180);
}

draw::surface& rotate(draw::surface& dc, double rotation, int bgcolor, bool autoblend)
{
	aar_pnt	polyoverlap[16];
	aar_pnt	polysorted[16];
	aar_pnt	corners[4];

	double dx[] = {0.0, 1.0, 1.0, 0.0};
	double dy[] = {0.0, 0.0, 1.0, 1.0};
	for(int i = 0; i < 4; i++)
	{
		corners[i].x = dx[i];
		corners[i].y = dy[i];
	}

	//Get rotation between [0, 360)
	int mult = (int)rotation / 360;
	if(rotation >= 0)
		rotation = rotation - 360.0 * mult;
	else
		rotation = rotation - 360.0 * (mult - 1);

	//Calculate the cos and sin values that will be used throughout the program
	coss = aar_cos(rotation);
	sins = aar_sin(rotation);

	return dorotate(src, rotation, callbackfunc, bgcolor, autoblend);
}