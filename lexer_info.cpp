#include "crt.h"
#include "string.h"
#include "io.h"
#include "point.h"
#include "lexer.h"

// Часть контекста
struct codeinfo_block_part : public string
{
	lexer_tokens	token;
	lexer*			scheme;

	void next_no_space()
	{
		while(true)
		{
			text = end;
			end = lexer::word(text, token, scheme);
			switch(token)
			{
			case LexerLineFeed:
			case LexerCommentMultiline:
			case LexerStringMultiline:
			case LexerWhiteSpace:
			case LexerComment:
				continue;
			default:
				return;
			}
		}
	}

};

// Постоянный контекст
struct codeinfo_block : public codeinfo_block_part
{

	struct status : public codeinfo_block_part
	{
		codeinfo_block_part&	e;
		bool					rollback;

		status(codeinfo_block_part& e) : codeinfo_block_part(e), e(e), rollback(true)
		{
		}
		~status()
		{
			if(rollback)
				e = reinterpret_cast<codeinfo_block_part&>(*this);
		}
		void clear()
		{
			rollback = false;
		}
	};
	struct type
	{
		string		name;
		string		result;
		unsigned	flags;
	};
	
	aref<type>		members;
	lexer::info&	pg;

	codeinfo_block(lexer::info& pg) : pg(pg)
	{
		members.data = 0;
		members.count = 0;
	}

	~codeinfo_block()
	{
		members.clear();
	}

	void create(const char* source, lexer* lex)
	{
		this->text = end = source;
		this->token = LexerEof;
		this->scheme = lex;
		next_no_space();
	}

	void next()
	{
		const char* p = text;
		lexer_tokens t = token;
		next_no_space();
	}

	bool match(const char* name)
	{
		int s1 = zlen(name);
		int s2 = end - text;
		if(s1!=s2 || memcmp(text, name, s1)!=0)
			return false;
		next();
		return true;
	}

	type* addlocale(string name, string result, unsigned flags)
	{
		members.reserve();
		auto& e = members.add();
		e.name = name;
		e.result = result;
		e.flags = flags;
		return &e;
	}

	string findtype(string name)
	{
		for(auto& e : members)
		{
			if(e.name == name)
				return e.result;
		}
		return string::empthy;
	}

	bool declaration(string* type, lexer::info& e)
	{
		if(token!=LexerType && !(token==LexerIdentifier && findtype(*this)))
			return false;
		status push(*this);
		// Пропустим стандартные типы
		while(token==LexerType)
		{
			*type = *this;
			next();
		}
		// Пропустим нестандартный тип, который должен быть один
		if(token==LexerIdentifier)
		{
			if(findtype(*this))
			{
				*type = *this;
				next();
			}
		}
		if(token != LexerPointer && token!=LexerIdentifier)
			return false;
		push.clear();
		return true;
	}

	void skip(char sym)
	{
		if(*text==sym)
			next();
	}

	string identifier()
	{
		string result("");
		if(token==LexerIdentifier || token==LexerKeyword)
		{
			result = *this;
			next();
		}
		return result;
	}

	int number()
	{
		if(token!=LexerNumber)
			return 0;
		int result = sz2num(text);
		next();
		return result;
	}

	void statement()
	{
		int current_count = members.count;
		switch(token)
		{
		case LexerEof:
		case LexerEnd:
		case LexerEndIndex:
		case LexerEndExpression:
			break;
		case LexerEndOp:
			next();
			break;
		case LexerIdentifier:
		case LexerType:
			if(!local_declaration(false, false, true))
				next();
			break;
		case LexerPoint:
			next();
			break;
		case LexerOperator:
		case LexerKeyword:
		case LexerNumber:
		case LexerString:
		case LexerPointer:
			next();
			break;
		case LexerBegin:
			next();
			while(token!=LexerEnd && token!=LexerEof)
			{
				statement();
				if(token==LexerEndExpression || token==LexerEndIndex)
					next();
			}
			next();
			members.count = current_count;
			break;
		case LexerBeginIndex:
			next();
			while(token!=LexerEndIndex && token!=LexerEof)
			{
				statement();
				if(token==LexerEndExpression || token==LexerEnd)
					next();
			}
			next();
			break;
		case LexerBeginExpression:
			next();
			while(token!=LexerEndExpression && token!=LexerEof)
			{
				statement();
				if(token==LexerEndIndex || token==LexerEnd)
					next();
			}
			next();
			break;
		default:
			pg.adderror(*this, token);
			next();
			break;
		}
	}

	string add_type(string id, string result)
	{
		id.end = result.end;
		return id;
	}

	void block_import()
	{
		while(match("import"))
		{
			string name;
			string t = *this;
			while(token!=LexerEof)
			{
				name = identifier();
				t = add_type(t, name);
				if(*text=='.')
				{
					next();
					continue;
				}
				break;
			}
			if(match("as"))
				name = identifier();
			addlocale(name, t, LexerType);
			pg.addmember(name, t, LexerType);
			skip(';');
		}
	}

	void block_enums()
	{
		while(match("enum"))
		{
			string name = identifier();
			addlocale(name, "enum", LexerType);
			pg.addmember(name, "int", LexerType);
			statement();
			skip(';');
		}
	}

	string declare_pointer(string t)
	{
		while(token==LexerPointer)
			next();
		return t;
	}

	bool local_declaration(bool global, bool functions, bool variables)
	{
		string declared_type;
		string content;
		if(!declaration(&declared_type, pg))
			return false;
		while(true)
		{
			string type = declare_pointer(declared_type);
			string name = identifier();
			// Функция
			if(token==LexerBeginExpression)
			{
				// Параметры функции
				statement();
				content.text = text;
				statement();
				content.end = text;
				if(functions && global)
				{
					addlocale(name, type, LexerFunction);
					pg.addmember(name, type, LexerFunction);
				}
				break;
			}
			// Массивы
			while(token==LexerBeginIndex)
				statement();
			// Присвоение значений
			if(token=='=')
			{
				next();
				statement();
			}
			addlocale(name, type, LexerIdentifier);
			if(variables && global)
				pg.addmember(name, type, LexerIdentifier);
			if(text[0]==',')
			{
				next();
				continue;
			}
			skip(';');
			break;
		}
		return true;
	}

	void block_declaration()
	{
		while(local_declaration(true, true, true));
	}

};

void lexer::info::parse(const char* source, lexer* lex)
{
	if(!lex)
		return;
	codeinfo_block e(*this);
	e.create(source, lex);
	e.block_import();
	e.block_enums();
	e.block_declaration();
}

void lexer::info::parseimport(const char* source, lexer* lex)
{
	if(!lex)
		return;
	codeinfo_block e(*this);
	e.create(source, lex);
	e.block_import();
}

void lexer::info::addkeywords(lexer* scheme)
{
	if(!scheme)
		return;
	string st;
	for(int i = 0; i<scheme->keywords.count; i++)
	{
		switch(scheme->keywords.data[i].token)
		{
		case LexerKeyword:
		case LexerType:
			addmember(scheme->keywords.data[i].name, "",
				scheme->keywords.data[i].token);
			break;
		default:
			break;
		}
	}
}