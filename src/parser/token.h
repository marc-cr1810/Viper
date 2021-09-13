#ifndef __TOKEN_H__
#define __TOKEN_H__

#include "../port.h"
#include "../objects/object.h"

enum token_type
{
	TOK_UNKNOWN = -1,	// Unknown token type; for errors
	TOK_ENDMARKER,		// Endmarker token
	TOK_NEWLINE,		// Newline token
	TOK_NAME,			// Variable names
	TOK_NUMBER,			// Number token type
	TOK_STRING,			// String token type

	/* Keyword tokens */
	TOK_IF,				// if
	TOK_DO,				// do
	TOK_FOR,			// for
	TOK_ELSE,			// else
	TOK_FUNC,			// func
	TOK_NULL,			// Null
	TOK_TRUE,			// True
	TOK_WHILE,			// while
	TOK_CLASS,			// class
	TOK_ASYNC,			// async
	TOK_AWAIT,			// await
	TOK_FALSE,			// False
	TOK_EXTENSION,		// extension

	/* Operator tokens */
	TOK_OP,				// Blank operator value, often means unknown operator
	TOK_ADD,			// +
	TOK_MINUS,			// -
	TOK_STAR,			// *
	TOK_FSLASH,			// /
	TOK_BSLASH,			// "\"
	TOK_COMMA,			// ,
	TOK_DOT,			// .
	TOK_EQUAL,			// =
	TOK_GREATER,		// >
	TOK_LESS,			// <
	TOK_AT,				// @
	TOK_PERCENT,		// %
	TOK_AMPER,			// &
	TOK_COLON,			// :
	TOK_SEMI,			// ;
	TOK_CIRCUMFLEX,		// ^
	TOK_TILDE,			// ~
	TOK_VBAR,			// |
	TOK_LPAREN,			// (
	TOK_RPAREN,			// )
	TOK_LSQB,			// [
	TOK_RSQB,			// ]
	TOK_LBRACE,			// {
	TOK_RBRACE,			// }

	/* Double char operator tokens */
	TOK_NOTEQUAL,		// "!=", <>
	TOK_ADDEQUAL,		// +=
	TOK_MINUSEQUAL,		// -=
	TOK_STAREQUAL,		// *=
	TOK_FSLASHEQUAL,	// /=
	TOK_GREATEREQUAL,	// >=
	TOK_LESSEQUAL,		// <=
	TOK_DOUBLEADD,		// ++
	TOK_DOUBLEMINUS,	// --
	TOK_ELLIPSIS,		// ..
	TOK_DOUBLESTAR,		// **
	TOK_DOUBLEFSLASH,	// //
	TOK_LEFTSHIFT,		// <<
	TOK_RIGHTSHIFT,		// >>

	/* Miscellaneous tokens */
	TOK_INDENT,
	TOK_DEDENT,
	TOK_TYPE_IGNORE,
	TOK_TYPE_COMMENT,
	TOK_ERRORTOKEN,
	TOK_N_TOKENS,
	TOK_NT_OFFSET = 256
};

typedef struct _memo
{
	int type;
	void *node;
	int mark;
	struct _memo *next;
} Memo;

typedef struct _token
{
	token_type type;	// What the token is
	ViObject *value;	// ViStringObject value of the token
	int lineno, col_offset, end_lineno, end_col_offset;	// Line and column position of the token
	Memo *memo;
} Token;

token_type ViToken_OneChar(char c1);
token_type ViToken_TwoChars(char c1, char c2);
token_type ViToken_ThreeChars(char c1, char c2, char c3);

#endif // __TOKEN_H__