#include "xsref.h"
#include "xsbase.h"
#include "xsglobal.h"

#pragma once

enum xserrors {
	ErrorXSNoError,
	ErrorXSNotFoundCommand, ErrorXSNotFoundField, ErrorXSExpected,
};

struct xsparse
{
	struct fileinfo
	{
		const char*		source;
		const char*		url;
		const fileinfo*	previous;
		xsparse*		parent;
		fileinfo(const char* url, xsparse* parent);
		~fileinfo();
		operator bool() const { return source != 0; }
		bool			isloaded(const char* url) const;
	};
	char				buffer[128 * 256];
	int					value;
	const xsfield*		value_type;
	const char			*p;
	const fileinfo*		file;
	xsparse();
	bool				isentity(const char* id);
	virtual bool		parseidentifier(xsbase* create_base = 0);
	virtual void		read();
	bool				read(const char* filename);
	void				readattribute(xsref e);
	void				readelements(xsref e);
	const xsfield*		readfield(const xsfield* fields);
	void				readfolder(const char* filename);
	void				readidentifier();
	void				readnumber();
	void				readtrail();
	void				readvalue(xsbase* create_base = 0);
	void				setvalue(xsref e, const xsfield* field, int index = 0);
	static const char*	skipline(const char* p);
	void				skipws();
	virtual void		status(xserrors error, ...);
};