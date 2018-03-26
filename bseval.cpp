#include "adat.h"
#include "bseval.h"
#include "bsdata.h"
#include "crt.h"

struct bstop {

	const char*	name;
	const bsreq* type;
	int	value, size, count;
	bool islvalue;

	constexpr bstop() : name(""), value(0), type(number_type), size(sizeof(int)), count(1), islvalue(false) {}
	operator bool() const { return type != 0; }
	bool isvalue() const { return !islvalue; }
	
	void set(int value) {
		this->value = value;
		type = number_type;
	}

	void set(const char* value) {
		this->value = (int)value;
		type = text_type;
	}

};

class bseval {

	const char* p;
	const char* p_error;
	bool stop;
	bsval base;
	adat<bstop, 128> locals;
	int local_base;
	char buffer[4096];

	void error(bsparse_error_s id, ...) {
		p_error = p;
		p = 0;
	}

	void next(const char* p) {
		this->p = zskipspcr(p);
	}

	void skip(char sym) {
		if(p[0] != sym) {
			char temp[2] = {sym, 0};
			error(ErrorExpectedSymbol1p, temp);
		}
		next(p + 1);
	}

	void dereference(bstop& e1) {
		if(e1.islvalue) {
			e1.value = (int)e1.type->get((void*)e1.value);
			e1.islvalue = false;
		}
	}

	void unary(bstop& e1) {
		if(!p)
			return;
		switch(*p) {
		case '-':
			next(p + 1);
			unary(e1);
			if(!p)
				break;
			if(!stop) {
				dereference(e1);
				e1.value = -e1.value;
			}
			break;
		case '+':
			next(p + 1);
			unary(e1);
			return;
		case '!':
			next(p + 1);
			unary(e1);
			if(!p)
				break;
			if(!stop) {
				dereference(e1);
				e1.value = e1.value ? 0 : 1;
			}
			break;
		case '(':
			next(p + 1);
			expression(e1);
			if(!p)
				break;
			if(p[0] == ')')
				next(p + 1);
			break;
		default:
			if(isnum(*p)) {
				int value = 0;
				next(psnum(p, value));
				if(!stop)
					e1.set(value);
			} else {
				p = psidn(p, buffer);
				next(p);
				if(p[0] == '(') {
					next(p + 1);
					auto count_push = locals.count;
					while(p && p[0] && p[0] != ')') {
						bstop e2;
						expression(e2);
						locals.add(e2);
						if(p[0] == ')')
							break;
						skip(',');
					}
					if(p[0] == ')')
						next(p + 1);
					if(!stop) {
					}
					locals.count = count_push;
				} else {
					if(!stop) {
						e1.value = 0;
					}
				}
			}
			break;
		}
	}

	void indirect(bstop& e1, void* data, const bsreq* type) {
		auto pf = type->find(buffer);
		if(!pf)
			error(ErrorNotFoundMember1pInBase2p, buffer, "");
		else {
			e1.value = (int)pf->ptr(data);
			e1.type = pf;
		}
	}

	void indirect(bstop& e1) {
		unary(e1);
		while(p && p[0] == '.') {
			p = psidn(p + 1, buffer);
			next(p);
			if(!stop) {
				dereference(e1);
				indirect(e1, (void*)e1.value, e1.type);
			}
		}
	}

	void multiplication(bstop& e1) {
		indirect(e1);
		while(p && (p[0] == '*' || p[0] == '/' || p[0] == '%') && p[1] != '=') {
			bstop e2;
			char t1 = p[0];
			next(p + 1);
			indirect(e2);
			if(!stop) {
				dereference(e1);
				dereference(e2);
				switch(t1) {
				case '*': e1.value *= e2.value; break;
				case '/': e1.value /= e2.value; break;
				case '%': e1.value %= e2.value; break;
				}
			}
		}
	}

	void addiction(bstop& e1) {
		multiplication(e1);
		while(p && (p[0] == '+' || p[0] == '-') && p[1] != '=') {
			bstop e2;
			char t1 = p[0];
			next(p + 1);
			multiplication(e2);
			if(!stop) {
				dereference(e1);
				dereference(e2);
				switch(t1) {
				case '+': e1.value += e2.value; break;
				case '-': e1.value -= e2.value; break;
				}
			}
		}
	}

	void binary_cond(bstop& e1) {
		addiction(e1);
		while(p && ((p[0] == '>' && p[1] != '>') || (p[0] == '<' && p[1] != '<') || (p[0] == '=' && p[1] == '=') || (p[0] == '!' && p[1] == '='))) {
			bstop e2;
			char t1 = *p++;
			char t2 = 0;
			if(p[0] == '=')
				t2 = *p++;
			next(p);
			addiction(e2);
			if(!stop) {
				dereference(e1);
				dereference(e2);
				switch(t1) {
				case '<':
					if(t2 == '=')
						e1.value = e1.value <= e2.value;
					else
						e1.value = e1.value < e2.value;
					break;
				case '>':
					if(t2 == '=')
						e1.value = e1.value >= e2.value;
					else
						e1.value = e1.value > e2.value;
					break;
				case '=': e1.value = (e1.value == e2.value); break;
				case '!': e1.value = (e1.value != e2.value); break;
				}
			}
		}
	}

	void binary_and(bstop& e1) {
		binary_cond(e1);
		while(p && p[0] == '&' && p[1] != '&') {
			bstop e2;
			next(p + 2);
			binary_cond(e2);
			if(!stop) {
				dereference(e1);
				dereference(e2);
				e1.value &= e2.value;
			}
		}
	}

	void binary_xor(bstop& e1) {
		binary_and(e1);
		while(p && p[0] == '^') {
			bstop e2;
			next(p + 1);
			binary_and(e2);
			if(!stop) {
				dereference(e1);
				dereference(e2);
				e1.value ^= e2.value;
			}
		}
	}

	void binary_or(bstop& e1) {
		binary_xor(e1);
		while(p && p[0] == '|' && p[1] != '|') {
			bstop e2;
			next(p + 1);
			binary_xor(e2);
			if(!stop) {
				dereference(e1);
				dereference(e2);
				e1.value |= e2.value;
			}
		}
	}

	void binary_shift(bstop& e1) {
		binary_or(e1);
		while(p && ((p[0] == '>' && p[1] == '>') || (p[0] == '<' && p[1] == '<'))) {
			bstop e2;
			char t1 = p[0];
			next(p + 2);
			binary_or(e2);
			if(!stop) {
				dereference(e1);
				dereference(e2);
				switch(t1) {
				case '<': e1.value <<= e2.value; break;
				case '>': e1.value >>= e2.value; break;
				}
			}
		}
	}

	void logical_and(bstop& e1) {
		binary_shift(e1);
		while(p && p[0] == '&' && p[1] == '&') {
			bstop e2;
			next(p + 2);
			binary_shift(e2);
			if(!stop) {
				dereference(e1);
				dereference(e2);
				e1.value = e1.value && e2.value;
			}
		}
	}

	void logical_or(bstop& e1) {
		logical_and(e1);
		while(p && p[0] == '|' && p[1] == '|') {
			bstop e2;
			next(p + 2);
			logical_and(e2);
			if(!stop) {
				dereference(e1);
				dereference(e2);
				e1.value = e1.value || e2.value;
			}
		}
	}

public:

	void expression(bstop& e1) {
		logical_or(e1);
		while(p && p[0] == '?') {
			bool push_stop = this->stop;
			bool correct = (e1.value != 0);
			next(p + 1);
			this->stop = !correct || push_stop;
			expression(e1);
			skip(':');
			this->stop = correct || push_stop;
			expression(e1);
			this->stop = push_stop;
		}
	}

	bseval(const char* p) : p(p), stop(false), p_error(0), local_base(0) {
	}

};

int bsdata::evalute(const char* code) {
	bstop result;
	bseval interpreter(code);
	interpreter.expression(result);
	return result.value;
}