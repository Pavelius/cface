#include "bsdata.h"

#pragma once

struct bsparse {
	char			buffer[128 * 256];
	int				value;
	const bsreq*	value_type;
	void*			value_object;
	void*			parent_object;
	const bsreq*	parent_type;
	const char		*p;
	//
	bsparse(const char* source);
	//
	void			parse();
protected:
	void			clearvalue();
	void			error();
	bool			islinefeed() const;
	bool			readidentifier();
	bool			readfields(void* object, const bsreq* fields);
	bool			readsubrecord();
	bool			readrecord();
	bool			readreq(void* object, const bsreq* req, unsigned index);
	void			readstring(const char end);
	void			readtrail();
	bool			readvalue(const bsreq* hint_type, bool create);
	void			skip();
	bool			skip(char sym);
	bool			skip(const char* sym);
	void			skipline();
	void			skipws();
	void			storevalue(void* object, const bsreq* req, unsigned index);
};