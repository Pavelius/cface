#include "adat.h"
#include "aref.h"
#include "bsdata.h"
#include "crt.h"

struct bseval {

	struct reg {

		const bsreq* type;
		int		count, size, value;
		bool	islvalue;

		constexpr reg() : value(0), type(number_type), size(sizeof(int)), count(1), islvalue(false) {}
		operator bool() const { return type != 0; }
		constexpr bool isnumber() const { return type == number_type; }
		constexpr bool istext() const { return type == text_type; }
		constexpr bool isscalar() const { return isnumber() || istext(); }

		void set(int value) {
			this->value = value;
			type = number_type;
		}

		void set(const char* value) {
			this->value = (int)value;
			type = text_type;
		}

	};

	static const int identifier_size = 64;

	const char*		p;
	bool			stop;
	const char**	parameters;
	bsval			base;
	aref<reg>		locals;
	aref<bsfunc>	functions;

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
			auto param_count = locals.count;
			auto pe = parameters + param_count;
			for(auto p = parameters; p < pe && *p; p++) {
				if(strcmp(*p, id) == 0) {
					result = *(locals.data + (p - parameters));
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
					reg locals_data[8];
					auto locals_index = 0;
					next(p + 1);
					while(p && p[0] && p[0] != ')') {
						expression(locals_data[(locals_index<lenghtof(locals_data)) ? locals_index++ : 0]);
						if(p[0] == ')')
							break;
						skip(',');
					}
					skip(')');
					if(!stop) {
						auto pf = getfunction(identifier);
						if(!pf)
							error(ErrorNotFoundFunction1p, identifier);
						else {
							auto locals_push = locals; locals.data = locals_data; locals.count = locals_index;
							auto push_parameters = parameters; parameters = pf->parameters;
							auto push_p = p; p = pf->code;
							expression(e1);
							parameters = push_parameters;
							p = push_p;
							locals = locals_push;
						}
					}
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

	void postfix(reg& e1) {
		unary(e1);
		char identifier[identifier_size];
		while(p) {
			switch(p[0]) {
			case '.':
				p = psidn(p + 1, identifier, identifier + sizeof(identifier) - 1);
				next(p);
				if(!stop) {
					dereference(e1);
					setindirect(e1, identifier, (void*)e1.value, e1.type);
				}
				continue;
			case '[':
				next(p + 1);
				{
					reg e2; expression(e2);
					if(!stop) {
						if(e2.type != number_type)
							error(ErrorExpected1p, "number");
						if(!e1.islvalue && !e1.isscalar())
							error(ErrorExpected1p, "array or pointer");
						if(e2.value >= e1.count)
							e2.value = e1.count - 1;
						if(e2.value >= 0)
							e1.value += e2.value * e1.size;
					}
				}
				skip(']');
				continue;
			default:
				return;
			}
		}
	}

	void multiplication(reg& e1) {
		postfix(e1);
		while(p && (p[0] == '*' || p[0] == '/' || p[0] == '%') && p[1] != '=') {
			reg e2;
			char t1 = p[0];
			next(p + 1);
			postfix(e2);
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

	bseval(const char* p) : p(p), stop(false), parameters(0) {
	}

};

int bsdata::evalute(const char* code) {
	bseval::reg result;
	bseval interpreter(code);
	interpreter.expression(result);
	return result.value;
}

int bsdata::evalute(const char* code, bsval context, bsfunc* functions, unsigned functions_count) {
	bseval::reg result;
	bseval interpreter(code);
	interpreter.base = context;
	interpreter.functions.data = functions;
	interpreter.functions.count = functions_count;
	interpreter.expression(result);
	return result.value;
}