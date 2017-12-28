#include "io_text.h"

using namespace io;

text::text(io::stream& parent, codepages cp) : cp(cp),
parent(parent), cashed_pos(0), cashed_count(0), end_of_file(false) {
}

void text::shift() {
	auto c = cashed_count - cashed_pos;
	if(c)
		memcpy(cashed, cashed + cashed_pos, c);
	cashed_count = c;
	cashed_pos = 0;
}

bool text::makecashe(unsigned count) {
	if(cashed_count - cashed_pos >= count)
		return true;
	if(cp == CPNONE) {
		cashed_count += parent.read(cashed, 3);
		if(cashed_count >= 3 && ((unsigned char)cashed[0]) == 0xEF && ((unsigned char)cashed[1]) == 0xBB && ((unsigned char)cashed[2]) == 0xBF) {
			cp = CPUTF8;
			cashed_pos += 3;
			shift();
			return makecashe(count);
		} else
			cp = CP1251;
	}
	if(cashed_pos)
		shift();
	auto size = sizeof(cashed) / sizeof(cashed[0]) - cashed_count;
	if(cp == CP1251)
		cashed_count += parent.read(cashed + cashed_count, size);
	else {
		while(size != 0) {
			auto value = getu();
			if(value >= 0x410 && value <= 0x44F)
				value = value - 0x410 + 0xC0;
			else switch(value) {
			case 0x406: value = 0xB2; break; // I
			case 0x407: value = 0xAF; break; // ¯
			case 0x456: value = 0xB3; break;
			case 0x457: value = 0xBF; break;
			}
			cashed[cashed_count++] = (unsigned char)value;
			size--;
		}
	}
	return count <= cashed_count;
}

void text::next(unsigned count) {
	while(true) {
		if(makecashe(count)) {
			cashed_pos += count;
			return;
		}
		count -= cashed_count;
	}
}

bool text::islinefeed() {
	return is("\n\r");
}

int	text::read(void* result, int count) {
	auto r = 0;
	auto c = imin(cashed_count - cashed_pos, (unsigned)count);
	if(c) {
		memcpy(result, cashed + cashed_pos, c);
		cashed_pos += c;
		r += c;
	}
	count -= c;
	if(count)
		r += parent.read((char*)result + c, count);
	return r;
}

int	text::write(const void* result, int count) {
	return parent.write(result, count);
}

int text::seek(int count, int rel) {
	cashed_count = 0;
	cashed_pos = 0;
	return parent.seek(count, rel);
}

bool text::is(const char* value) {
	if(!makecashe(1))
		return false;
	if(zchr(value, cashed[cashed_pos]))
		return true;
	return false;
}

bool text::skipws() {
	if(is(" \t")) {
		next(1);
		return true;
	}
	return false;
}

bool text::ispair(const char* pairs) {
	if(!makecashe(1))
		return false;
	auto sym = (unsigned char)cashed[cashed_pos];
	for(int i = 0; pairs[i]; i += 2) {
		if(sym >= (unsigned char)pairs[i] && sym <= (unsigned char)pairs[i + 1])
			return true;
	}
	return false;
}

bool text::skipcr() {
	if(is('\n')) {
		if(is('\r'))
			next(2);
		else
			next(1);
		return true;
	} else if(is('\r')) {
		if(is('\n'))
			next(2);
		else
			next(1);
		return true;
	}
	return false;
}

bool text::next(const char* value) {
	unsigned count = zlen(value);
	if(!makecashe(count))
		return false;
	if(memcmp(cashed + cashed_pos, value, count) != 0)
		return false;
	cashed_pos += count;
	return true;
}

bool text::identifier(char* result, unsigned max_count) {
	if(!ispair("AZazÀßàÿ") && !is('_'))
		return false;
	auto pb = result;
	auto pe = result + max_count;
	while(ispair("AZaz09Àßàÿ") || is('_')) {
		if(pb < pe)
			*pb++ = cashed[cashed_pos];
		cashed_pos++;
	}
	*pb = 0;
	return true;
}

bool text::is(const char symbol) {
	if(!makecashe(1))
		return false;
	return cashed[cashed_pos] == symbol;
}

bool text::is(const char symbol, unsigned index) {
	if(!makecashe(index + 1))
		return false;
	return cashed[cashed_pos + index] == symbol;
}

unsigned char text::getone() {
	unsigned char sym;
	if(!parent.read(&sym, 1))
		return 0;
	return sym;
}

char text::get() {
	makecashe(1);
	if(cashed_pos < cashed_count)
		return cashed[cashed_pos++];
	return 0;
}

char text::getnr() {
	makecashe(1);
	if(cashed_pos < cashed_count)
		return cashed[cashed_pos];
	return 0;
}

void text::putu(unsigned value) {
	switch(cp) {
	case CPUTF8:
		if(((unsigned short)value) < 128)
			put((unsigned char)value);
		else if(((unsigned short)value) < 2047) {
			put((unsigned char)(192 + (((unsigned short)value) / 64)));
			put((unsigned char)(128 + (((unsigned short)value) % 64)));
		} else {
			put((unsigned char)(224 + (((unsigned short)value) / 4096)));
			put((unsigned char)(128 + ((((unsigned short)value) / 64) % 64)));
			put((unsigned char)(224 + (((unsigned short)value) % 64)));
		}
		break;
	case CP1251:
		if(value >= 0x410 && value <= 0x44F)
			value = value - 0x410 + 0xC0;
		else switch(value) {
		case 0x406: value = 0xB2; break; // I
		case 0x407: value = 0xAF; break; // ¯
		case 0x456: value = 0xB3; break;
		case 0x457: value = 0xBF; break;
		}
		put((unsigned char)value);
		break;
	case CPU16LE:
		put((unsigned char)(value & 0xFF));
		put((unsigned char)(((unsigned)value >> 8)));
		break;
	case CPU16BE:
		put((unsigned char)(((unsigned)value >> 8)));
		put((unsigned char)(value & 0xFF));
		break;
	default:
		put((unsigned char)value);
		break;
	}
}

unsigned text::getu() {
	unsigned result = getone();
	switch(cp) {
	case CPUTF8:
		if(result >= 192 && result <= 223)
			result = (result - 192) * 64 + (getone() - 128);
		else if(result >= 224 && result <= 239) {
			auto p0 = getone();
			auto p1 = getone();
			result = (result - 224) * 4096 + (p0 - 128) * 64 + (p1 - 128);
		}
		return result;
	case CPU16LE:
		result |= getone() << 8;
		return result;
	case CP1251:
		if(((unsigned char)result >= 0xC0))
			return result - 0xC0 + 0x410;
		else switch(result) {
		case 0xB2: return 0x406;
		case 0xAF: return 0x407;
		case 0xB3: return 0x456;
		case 0xBF: return 0x457;
		}
		return result;
	default:
		return result;
	}
}