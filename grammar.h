#pragma once

namespace grammar
{
    namespace adjective
    {
		const char*	by(char* r, const char* s);
    }
    namespace noun
    {
		namespace pluar
		{
			char* by(char* r, const char* s);
			char* of(char* r, const char* s);
		}
		char* by(char* r, const char* s);
		char* of(char* r, const char* s);
    }
}
