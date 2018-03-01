#include "crt.h"
#include "io.h"

class bscpp {

	const char*	p;
	char buffer[256 * 256];

	bool isvalid() {
		return *p != 0;
	}

	bool linefeed() {
		if(*p == '\n') {
			p++;
			if(*p == '\r')
				p++;
			return true;
		} else if(*p == '\r') {
			p++;
			if(*p == '\n')
				p++;
			return true;
		} else
			return false;
	}

	bool symbol(const char* sym) {
		auto i = 0;
		while(sym[i]) {
			if(p[i] != sym[i])
				return false;
			i++;
		}
		p += i;
		return true;
	}

	bool keyword(const char* sym) {
		if(!symbol(sym))
			return false;
		skipws();
		return true;
	}

	void skipline() {
		while(*p) {
			if(p[0] == '\\' && (p[1] == 10 || p[1] == 13)) {
				p++;
				linefeed();
				continue;
			}
			if(linefeed())
				return;
			p++;
		}
	}

	bool whitespace() {
		if(*p == '\t' || *p == ' ') {
			p++;
			return true;
		} else
			return false;
	}

	bool comment() {
		if(p[0] == '/' && p[1] == '/') {
			p += 2;
			skipline();
			return true;
		}
		return false;
	}

	void skipws() {
		while(*p) {
			if(whitespace())
				continue;
			if(linefeed())
				continue;
			if(comment())
				continue;
			if(p[0] == '\\' && (p[1] == 10 || p[1] == 13)) {
				p++; // Screen by linefeed
				continue;
			}
			break;
		}
	}

	bool readidentifier() {
		if((p[0] >= 'a' && p[0] <= 'z') || (p[0] >= 'A' && p[0] <= 'Z')) {
			auto ps = buffer;
			auto pe = ps + sizeof(buffer) / sizeof(buffer[0]) - 1;
			while(*p && ((*p >= '0' && *p <= '9') || *p == '_' || ischa(*p))) {
				if(ps < pe)
					*ps++ = *p;
				p++;
			}
			ps[0] = 0;
		} else
			return false;
		return true;
	}

	const char* cr() {
		return "\r\n";
	}

public:

	bscpp(const char* p) : p(p) {
	}


	bool parsemsg(io::stream& po) {
		char class_name[512];
		po << "#include \"bsdata.h\"" << cr();
		po << "#include \"messages.h\"" << cr();
		po << cr();
		skipws();
		while(isvalid()) {
			if(symbol("#") || keyword("extern")) {
				skipline();
				skipws();
				continue;
			}
			if(!keyword("struct"))
				return false;
			if(!readidentifier())
				return false;
			skipws();
			if(true) {
				zcpy(class_name, buffer);
				char* pz = (char*)zchr(class_name, '_');
				if(pz)
					pz[0] = 0;
			}
			po << "bsreq " << class_name << "_type[] = {" << cr();
			if(!keyword("{"))
				return false;
			while(*p) {
				if(!keyword("const char*"))
					return false;
				if(!readidentifier())
					return false;
				po << "\tBSREQ(" << class_name << "_info, " << buffer << ", text_type)," << cr();
				if(!keyword(";"))
					return false;
				if(*p == '}') {
					p++; skipws();
					break;
				}
			}
			if(!keyword(";"))
				return false;
			po << "{}," << cr() << "};" << cr() << class_name << "_info " << class_name << "; BSGLOB(" << class_name << ");" << cr();
		}
		return true;
	}

};

bool cpp_parsemsg(const char* url, const char* out_url) {
	io::file po(out_url, StreamWrite | StreamText);
	if(!po)
		return false;
	auto p = loadt(url);
	if(!p)
		return false;
	bscpp e(p);
	e.parsemsg(po);
	delete p;
	return true;
}