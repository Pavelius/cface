#include "aref.h"
#include "string.h"

#pragma once

enum lexer_flags {
	LexerRemoveSpace	= 0x00000001,
	LexerRemoveLine		= 0x00000002,
	LexerAddLine		= 0x00000004,
	LexerAddSpace		= 0x00000008,
};
enum lexer_tokens	{
	LexerWhiteSpace, LexerIllegal, LexerLineFeed, LexerEof,
	LexerKeyword, LexerPreprocessor,
	LexerNumber, LexerString, LexerStringMultiline,
	LexerComment, LexerCommentMultiline,
	LexerIdentifier, LexerType, LexerNamespace,
	LexerOperator, LexerBegin, LexerEnd, LexerBeginIndex, LexerEndIndex, LexerBeginExpression, LexerEndExpression,
	// Analize
	LexerEndOp, LexerPointer, LexerPoint, LexerFunction,
};

struct lexer
{
	struct keyword
	{
		const char*			name;
		unsigned			token;
		int					count; // cashed token
		operator bool() { return name != 0; }
	};
	struct info
	{
		virtual void		adderror(string name, int token) = 0;
		virtual void		addmember(string name, string type, int token) = 0;
		void				addkeywords(lexer* scheme);
		void				parse(const char* source, lexer* lex);
		void				parseimport(const char* source, lexer* lex);
	};
	const char*				name;
	const char**			extensions;
	static lexer*			first;
	lexer*					next;
	aref<keyword>			keywords;
	//
	lexer(const char* name, const char** extensions, keyword* keywords);
	//
	const keyword*			find(lexer_tokens t) const;
	static bool				issym(char e);
	const keyword*			match(const char* text) const;
	static bool				match(const char* text, const char* name);
	const char*				prev(const char* p0, const char* p, lexer_tokens& t) const;
	static const char*		word(const char* string, lexer_tokens& token, const lexer* scheme);
};