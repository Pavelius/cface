#include "crt.h"
#include "adat.h"
#include "aref.h"
#include "bsdata.h"

struct bseval {
	
	struct reg {
		const bsreq* type;
		int			count, size;
		int			value;
		bool		islvalue;

		constexpr reg() : value(0), type(number_type), size(sizeof(int)), count(1), islvalue(false) {}
		constexpr reg(const char* id, const bsreq* type = number_type, int count = 1) : value(0), type(type), size(sizeof(int)), count(1), islvalue(type != number_type && type != text_type) {}
		operator bool() const { return type != 0; }
		
		void set(int value) {
			this->value = value;
			type = number_type;
		}

		void set(const char* value) {
			this->value = (int)value;
			type = text_type;
		}

		bool isnumber() const {
			return type == number_type;
		}

		bool istext() const {
			return type == text_type;
		}

		bool isscalar() const {
			return isnumber() || istext();
		}

	};

	static const int identifier_size = 64;
	const char*		p;
	bool			stop;
	bsval			base;
	int				local_base;
	adat<reg, 256>	locals;
	aref<bsfunc>	functions;
	const char**	parameters;

	void error(bsparse_error_s id, ...) {
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

	void dereference(reg& e1) {
		if(e1.islvalue) {
			// ¬начале мы получим значени€ указател€, если он есть
			for(auto i = e1.type->reference; e1.value && i > 0; i--)
				e1.value = *((int*)e1.value);
			// “еперь дл€ скал€рных типов получим само значение
			if(e1.isscalar()) {
				switch(e1.size) {
				case sizeof(char) : e1.value = *((char*)e1.value); break;
				case sizeof(short) : e1.value = *((short*)e1.value); break;
				case sizeof(int) : e1.value = *((int*)e1.value); break;
				}
			}
			e1.islvalue = false;
		}
	}

	void setindirect(reg& result, const char* id, void* data, const bsreq* type) {
		auto pf = type->find(id);
		if(pf) {
			result.value = (int)pf->ptr(data);
			result.type = pf->type;
			result.size = pf->size;
			result.count = pf->count;
			result.islvalue = true;
		} else
			error(ErrorNotFoundMember1pInBase2p, id, "");
	}

	bool setparameter(reg& result, const char* id) {
		if(parameters) {
			auto param_count = locals.count - local_base;
			auto pe = parameters + param_count;
			for(auto p = parameters; p < pe && *p; p++) {
				if(strcmp(*p, id) == 0) {
					result = *(locals.data + local_base + (p - parameters));
					return true;
				}
			}
		}
		return false;
	}

	bsfunc* getfunction(const char* id) {
		for(auto& e : functions) {
			if(strcmp(e.id, id) == 0)
				return &e;
		}
		return 0;
	}

	void calling(reg& result, const char* id, reg* parameters, unsigned count) {
		auto pf = getfunction(id);
		if(!pf)
			return;
	}

	void unary(reg& e1) {
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
				char identifier[identifier_size];
				p = psidn(p, identifier, identifier + sizeof(identifier) - 1);
				next(p);
				if(p[0] == '(') {
					next(p + 1);
					auto base_push = local_base;
					local_base = locals.count;
					while(p && p[0] && p[0] != ')') {
						reg e2; expression(e2);
						locals.add(e2);
						if(p[0] == ')')
							break;
						skip(',');
					}
					if(p[0] == ')')
						next(p + 1);
					if(!stop)
						calling(e1, identifier, locals.data + local_base, locals.count - local_base);
					locals.count = local_base;
					local_base = base_push;
				} else {
					if(!stop) {
						if(setparameter(e1, identifier))
							break;
						setindirect(e1, identifier, base.data, base.type);
					}
				}
			}
			break;
		}
	}

	void indirect(reg& e1) {
		unary(e1);
		while(p && p[0] == '.') {
			char identifier[identifier_size];
			p = psidn(p + 1, identifier, identifier + sizeof(identifier) - 1);
			next(p);
			if(!stop) {
				dereference(e1);
				setindirect(e1, identifier, (void*)e1.value, e1.type);
			}
		}
	}

	void multiplication(reg& e1) {
		indirect(e1);
		while(p && (p[0] == '*' || p[0] == '/' || p[0] == '%') && p[1] != '=') {
			reg e2;
			char t1 = p[0];
			next(p + 1);
			indirect(e2);
			if(!stop) {
				dereference(e1);
				dereference(e2);
				if(e1.type == number_type && e2.type == number_type) {
					switch(t1) {
					case '*': e1.value *= e2.value; break;
					case '/': e1.value /= e2.value; break;
					case '%': e1.value %= e2.value; break;
					}
				}
			}
		}
	}

	void addiction(reg& e1) {
		multiplication(e1);
		while(p && (p[0] == '+' || p[0] == '-') && p[1] != '=') {
			reg e2;
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

	void binary_cond(reg& e1) {
		addiction(e1);
		while(p && ((p[0] == '>' && p[1] != '>') || (p[0] == '<' && p[1] != '<') || (p[0] == '=' && p[1] == '=') || (p[0] == '!' && p[1] == '='))) {
			reg e2;
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
						e1.value = (e1.value >= e2.value);
					else
						e1.value = (e1.value > e2.value);
					break;
				case '=': e1.value = (e1.value == e2.value); break;
				case '!': e1.value = (e1.value != e2.value); break;
				}
			}
		}
	}

	void binary_and(reg& e1) {
		binary_cond(e1);
		while(p && p[0] == '&' && p[1] != '&') {
			reg e2;
			next(p + 2);
			binary_cond(e2);
			if(!stop) {
				dereference(e1);
				dereference(e2);
				e1.value &= e2.value;
			}
		}
	}

	void binary_xor(reg& e1) {
		binary_and(e1);
		while(p && p[0] == '^') {
			reg e2;
			next(p + 1);
			binary_and(e2);
			if(!stop) {
				dereference(e1);
				dereference(e2);
				e1.value ^= e2.value;
			}
		}
	}

	void binary_or(reg& e1) {
		binary_xor(e1);
		while(p && p[0] == '|' && p[1] != '|') {
			reg e2;
			next(p + 1);
			binary_xor(e2);
			if(!stop) {
				dereference(e1);
				dereference(e2);
				e1.value |= e2.value;
			}
		}
	}

	void binary_shift(reg& e1) {
		binary_or(e1);
		while(p && ((p[0] == '>' && p[1] == '>') || (p[0] == '<' && p[1] == '<'))) {
			reg e2;
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

	void logical_and(reg& e1) {
		binary_shift(e1);
		while(p && p[0] == '&' && p[1] == '&') {
			reg e2;
			next(p + 2);
			binary_shift(e2);
			if(!stop) {
				dereference(e1);
				dereference(e2);
				e1.value = e1.value && e2.value;
			}
		}
	}

	void logical_or(reg& e1) {
		logical_and(e1);
		while(p && p[0] == '|' && p[1] == '|') {
			reg e2;
			next(p + 2);
			logical_and(e2);
			if(!stop) {
				dereference(e1);
				dereference(e2);
				e1.value = e1.value || e2.value;
			}
		}
	}

	void expression(reg& e1) {
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

	bseval(const char* p) : p(p), stop(false), local_base(0), parameters(0) {
	}

	void set(bsval value) {
		base = value;
	}

};

int bsdata::evalute(const char* code) {
	bseval::reg result;
	bseval interpreter(code);
	interpreter.expression(result);
	return result.value;
}

int bsdata::evalute(const char* code, bsval context, bsfunc* functions) {
	bseval::reg result;
	bseval interpreter(code);
	interpreter.set(context);
	interpreter.expression(result);
	return result.value;
}