#include "crt.h"

static const char* skipdsp(const char* source)
{
	while (*source == '.' || *source == ' ' || *source == '/' || *source == '\'')
		source++;
	return source;
}

unsigned mkdate(int year, int month, int day, int hour, int minute)
{
	return (((1461 * (year - 1600 + (month - 14) / 12)) / 4
		+ (367 * (month - 2 - 12 * ((month - 14) / 12))) / 12
		- (3 * ((year - 1600 + 100 + (month - 14) / 12) / 100)) / 4
		+ day - 32075) * 1440
		+ hour * 60 + minute);
}

unsigned monthb(unsigned d)
{
	return mkdate(getyear(d), getmonth(d), 1, 0, 0);
}

unsigned monthe(unsigned d)
{
	int y = getyear(d);
	int m = getmonth(d);
	return mkdate(y, m, getmonthdaymax(m,y), 23, 59);
}

unsigned dayb(unsigned d)
{
	return mkdate(getyear(d), getmonth(d), getday(d), 0, 0);
}

unsigned daye(unsigned d)
{
	return mkdate(getyear(d), getmonth(d), getday(d), 23, 59);
}

unsigned weekb(unsigned d)
{
	return d;
}

bool isleap(int year)
{
	return (year % 400) == 0 || (((year % 100) != 0) && (year % 4) == 0);
}

int	getyear(unsigned d)
{
	unsigned ell, n, i, j;
	ell = (d / 1440) + 68569;
	n = (4 * ell) / 146097;
	ell = ell - (146097 * n + 3) / 4;
	i = (4000 * (ell + 1)) / 1461001;
	ell = ell - (1461 * i) / 4 + 31;
	j = (80 * ell) / 2447;
	ell = j / 11;
	return (int)(100 * (n - 49) + i + ell + 6400);
}

int	getmonth(unsigned d)
{
	unsigned ell, n, i, j;
	ell = (d / 1440) + 68569;
	n = (4 * ell) / 146097;
	ell = ell - (146097 * n + 3) / 4;
	i = (4000 * (ell + 1)) / 1461001;
	ell = ell - (1461 * i) / 4 + 31;
	j = (80 * ell) / 2447;
	ell = j / 11;
	return (int)(j + 2 - (12 * ell));
}

int getmonthdaymax(int month, int year)
{
	if (month == 0 || month == 2 || month == 4 || month == 6 || month == 7 || month == 9 || month == 11)
		return 31;
	else if (month == 3 || month == 5 || month == 8 || month == 10)
		return 30;
	if (year % 4 == 0)
	{
		if (year % 100 == 0)
		{
			if (year % 400 == 0)
				return 29;
			return 28;
		}
		return 29;
	}
	return 28;
}

int	getday(unsigned d)
{
	unsigned ell, n, i, j;
	ell = (d / 1440) + 68569;
	n = (4 * ell) / 146097;
	ell = ell - (146097 * n + 3) / 4;
	i = (4000 * (ell + 1)) / 1461001;
	ell = ell - (1461 * i) / 4 + 31;
	j = (80 * ell) / 2447;
	return (int)(ell - (2447 * j) / 80);
}

const char* getstrfdat(char* result, unsigned d, bool show_time)
{
	result[0] = 0;
	if(!d)
		return result;
	sznum(result, getday(d), 2);
	zcpy(zend(result), ".");
	sznum(zend(result), getmonth(d), 2);
	zcpy(zend(result), ".");
	sznum(zend(result), getyear(d), 4);
	if(show_time)
	{
		zcpy(zend(result), " ");
		sznum(zend(result), gethour(d), 2);
		zcpy(zend(result), ":");
		sznum(zend(result), getminute(d), 2);
	}
	return result;
}

unsigned getdatfstr(const char* source)
{
	unsigned d = getdate();
	int day = sz2num(source, &source);
	source = skipdsp(source);
	int month = sz2num(source, &source);
	if(!month)
		month = getmonth(d);
	source = skipdsp(source);
	int year = sz2num(source, &source);
	if(!year)
		year = getyear(d);
	return mkdate(year, month, day, 0, 0);
}