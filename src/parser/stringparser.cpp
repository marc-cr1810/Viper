#include "stringparser.h"

void ViStringParser_Init(FstringParser *state)
{}

int ViStringParser_ParseStr(Parser * p, int *bytesmode, int *rawmode, ViObject * *result, const char **fstr, Vi_size_t * fstrlen, Token * t)
{
	return 0;
}

int ViStringParser_ConcatFstring(Parser *p, FstringParser *state, const char **str, const char *end, int raw, int recurse_lvl, Token *first_token, Token *t, Token *last_token)
{
	return 0;
}

int ViStringParser_ConcatAndDel(FstringParser *state, ViObject *str)
{
	return 0;
}

expr_type ViStringParser_Finish(Parser *p, FstringParser *state, Token *first_token, Token *last_token)
{
	return expr_type();
}

void ViStringParser_Dealloc(FstringParser *state)
{}
