
#ifndef __PARSER_H__
#define __PARSER_H__

#include "ast.h"
#include "tokenizer.h"
#include "../core/viarena.h"

#define KEYWORD_COUNT 10
#define KEYWORDS_MAX 10

/*
 *	Parser
*/

typedef struct _keywordtoken
{
	const char *name;
	token_type type;
} KeywordToken;

#if 0
#define PARSE_YIELD_IS_KEYWORD        0x0001
#endif

#define PARSE_DONT_IMPLY_DEDENT       0x0002

#if 0
#define PARSE_WITH_IS_KEYWORD         0x0003
#define PARSE_PRINT_IS_FUNCTION       0x0004
#define PARSE_UNICODE_LITERALS        0x0008
#endif

#define PARSE_IGNORE_COOKIE 0x0010
#define PARSE_BARRY_AS_BDFL 0x0020
#define PARSE_TYPE_COMMENTS 0x0040
#define PARSE_ASYNC_HACKS   0x0080

#define PARSER_MODE_SINGLE_INPUT 0
#define PARSER_MODE_FILE_INPUT 1
#define PARSER_MODE_EVAL_INPUT 2
#define PARSER_MODE_STRING_INPUT 3

typedef struct _parser
{
	int mode;
	int error_indicator;
	int* error_code;
	int parsing_started;
	int flags;
	int starting_lineno;
	int starting_col_offset;

	ViArena *arena;

	TokState *tok;
	Token **tokens;
	int fill, size;

	int keywordListSize;
	KeywordToken (*keywords)[KEYWORD_COUNT];

	int mark;
	int level;
} Parser;

Parser *ViParser_New(TokState *tokState, int startRule, int *error_code, ViArena *arena);

void *ViParser_Parse(Parser *p);

void ViParser_Free(Parser *p);

#endif // __PARSER_H__