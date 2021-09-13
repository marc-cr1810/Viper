#include "token.h"

// Tests for single char operator types
token_type ViToken_OneChar(char c1)
{
	switch (c1)
	{
	case '+':
		return TOK_ADD;
	case '-':
		return TOK_MINUS;
	case '*':
		return TOK_STAR;
	case '/':
		return TOK_FSLASH;
	case '\\':
		return TOK_BSLASH;
	case ',':
		return TOK_COMMA;
	case '.':
		return TOK_DOT;
	case '=':
		return TOK_EQUAL;
	case '>':
		return TOK_GREATER;
	case '<':
		return TOK_LESS;
	case '@':
		return TOK_AT;
	case '%':
		return TOK_PERCENT;
	case '&':
		return TOK_AMPER;
	case ':':
		return TOK_COLON;
	case ';':
		return TOK_SEMI;
	case '^':
		return TOK_CIRCUMFLEX;
	case '~':
		return TOK_TILDE;
	case '|':
		return TOK_VBAR;
	case '(':
		return TOK_LPAREN;
	case ')':
		return TOK_RPAREN;
	case '[':
		return TOK_LSQB;
	case ']':
		return TOK_RSQB;
	case '{':
		return TOK_LBRACE;
	case '}':
		return TOK_RBRACE;
	}
	return TOK_OP;
}

token_type ViToken_TwoChars(char c1, char c2)
{
	switch (c1)
	{
	case '+':
		switch (c2)
		{
		case '=':
			return TOK_ADDEQUAL;
		case '+':
			return TOK_DOUBLEADD;
		}
		break;
	case '-':
		switch (c2)
		{
		case '=':
			return TOK_MINUSEQUAL;
		case '-':
			return TOK_DOUBLEMINUS;
		}
		break;
	case '*':
		switch (c2)
		{
		case '*':
			return TOK_DOUBLESTAR;
		case '=':
			return TOK_STAREQUAL;
		}
		break;
	case '/':
		switch (c2)
		{
		case '/':
			return TOK_DOUBLEFSLASH;
		case '=':
			return TOK_FSLASHEQUAL;
		}
		break;
	case '!':
		switch (c2)
		{
		case '=':
			return TOK_NOTEQUAL;
		}
		break;
	case '>':
		switch (c2)
		{
		case '=':
			return TOK_GREATEREQUAL;
		case '>':
			return TOK_RIGHTSHIFT;
		}
		break;
	case '<':
		switch (c2)
		{
		case '=':
			return TOK_LESSEQUAL;
		case '<':
			return TOK_LEFTSHIFT;
		case '>':
			return TOK_NOTEQUAL;
		}
		break;
	case '.':
		switch (c2)
		{
		case '.':
			return TOK_ELLIPSIS;
		}
		break;
	}
	return TOK_OP;
}

token_type ViToken_ThreeChars(char c1, char c2, char c3)
{
	return TOK_OP;
}
