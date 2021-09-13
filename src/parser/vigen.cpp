#include "vigen.h"

#include "../core/error.h"
#include "../objects/stringobject.h"
#include "../objects/listobject.h"
#include "../objects/intobject.h"
#include "../objects/floatobject.h"
#include "../objects/complexobject.h"
#include "stringparser.h"

/* Helper functions */

static int tokenizer_error(Parser *p)
{
	if (ViError_Occurred())
	{
		return -1;
	}

	const char *msg = NULL;
	ViObject *errtype = ViExc_SyntaxError;
	switch (p->tok->done)
	{
	case E_TOKEN:
		msg = "invalid token";
		break;
	case E_EOFS:
		msg = "EOF while scanning triple-quoted string literal";
		break;
	case E_EOLS:
		msg = "EOL while scanning string literal";
		break;
	case E_EOF:
		msg = "unexpected EOF while parsing";
		break;
	case E_DEDENT:
		errtype = ViExc_IndentationError;
		msg = "unindent does not match any outer indentation level";
		break;
	case E_INTR:
		if (!ViError_Occurred())
		{
			ViError_SetNone(ViExc_KeyboardInterrupt);
		}
		return -1;
	case E_NOMEM:
		ViError_NoMemory();
		return -1;
	case E_TABSPACE:
		errtype = ViExc_TabError;
		msg = "inconsistent use of tabs and spaces in indentation";
		break;
	case E_TOODEEP:
		errtype = ViExc_IndentationError;
		msg = "too many levels of indentation";
		break;
	case E_LINECONT:
		msg = "unexpected character after line continuation character";
		break;
	default:
		msg = "unknown parsing error";
		break;
	}

	ViError_SetString(errtype, msg);

	return TOK_UNKNOWN;
}

static int get_keyword_or_name_type(Parser *p, const char *name, int name_len)
{
	assert(name_len > 0);
	if (name_len >= p->keywordListSize ||
		p->keywords[name_len] == NULL ||
		p->keywords[name_len]->type == TOK_UNKNOWN)
	{
		return TOK_NAME;
	}
	for (KeywordToken *k = p->keywords[name_len]; k != NULL && k->type != -1; k++)
	{
		if (strncmp(k->name, name, name_len) == 0)
		{
			return k->type;
		}
	}
	return TOK_NAME;
}

static mod_type gen_run_parser_from_file_pointer(std::ifstream *fp, ViObject *filename, int mode, const char *ps1, const char *ps2, int *error_code, ViArena *arena)
{
	TokState *tok = ViTokenizer_FromFile(fp, ps1, ps2);
	if (tok == NULL)
	{
		ViError_SetString(ViExc_SystemError, "failed to initialize tokenizer");
		return NULL;
	}

	tok->filename = filename;
	ViObject_INCREF(filename);

	mod_type result = NULL;

	Parser *p = ViParser_New(tok, mode, error_code, arena);
	if (p == NULL)
		goto error;

	result = (mod_type)ViParser_Parse(p);
	ViParser_Free(p);
error:
	ViTokenizer_Free(tok);
	return result;
}

/* Memoization functions */

// Instrumentation to count the effectiveness of memoization.
// The array counts the number of tokens skipped by memoization,
// indexed by type.

#define NSTATISTICS 2000
static long memo_statistics[NSTATISTICS];

void ViGen_Memo_ClearStats()
{
	for (int i = 0; i < NSTATISTICS; i++)
	{
		memo_statistics[i] = 0;
	}
}

ViObject *ViGen_Memo_GetStats()
{
	ViObject *ret = ViListObject_New(NSTATISTICS);
	if (ret == NULL)
	{
		return NULL;
	}
	for (int i = 0; i < NSTATISTICS; i++)
	{
		ViObject *value = ViIntObject_FromInt(memo_statistics[i]);
		if (value == NULL)
		{
			ViObject_DECREF(ret);
			return NULL;
		}
		// ViList_SetItem borrows a reference to value.
		if (ViList_SetItem(ret, i, value) < 0)
		{
			ViObject_DECREF(ret);
			return NULL;
		}
	}
	return ret;
}

int ViGen_IsMemoized(Parser *p, int type, void *pres)
{
	if (p->mark == p->fill)
	{
		if (ViGen_FillToken(p) < 0)
		{
			p->error_indicator = 1;
			return -1;
		}
	}

	Token *t = p->tokens[p->mark];

	for (Memo *m = t->memo; m != NULL; m = m->next)
	{
		if (m->type == type)
		{
			if (0 <= type && type < NSTATISTICS)
			{
				long count = m->mark - p->mark;
				// A memoized negative result counts for one.
				if (count <= 0)
				{
					count = 1;
				}
				memo_statistics[type] += count;
			}
			p->mark = m->mark;
			*(void **)(pres) = m->node;
			return 1;
		}
	}
	return 0;
}

// Here, mark is the start of the node, while p->mark is the end.
// If node==NULL, they should be the same.
int ViGen_Memo_Insert(Parser *p, int mark, int type, void *node)
{
	// Insert in front
	Memo *m = (Memo *)Mem_Alloc(sizeof(Memo));
	if (m == NULL)
		return -1;
	m->type = type;
	m->node = node;
	m->mark = p->mark;
	m->next = p->tokens[mark]->memo;
	p->tokens[mark]->memo = m;
	return 0;
}

// Like ViGen_Memo_Insert(), but updates an existing node if found.
int ViGen_Memo_Update(Parser *p, int mark, int type, void *node)
{
	for (Memo *m = p->tokens[mark]->memo; m != NULL; m = m->next)
	{
		if (m->type == type)
		{
			// Update existing node
			m->node = node;
			m->mark = p->mark;
			return 0;
		}
	}
	// Insert new node
	return ViGen_Memo_Insert(p, mark, type, node);
}

/* Tokenizer functions */

int ViGen_FillToken(Parser *p)
{
	const char *start;
	const char *end;
	int type = ViTokenizer_Get(p->tok, &start, &end);

	// Skip '# type: ignore' comments
	while (type == TOK_TYPE_IGNORE)
	{
		type = ViTokenizer_Get(p->tok, &start, &end);
	}

	if (type == TOK_ENDMARKER && p->mode == PARSER_MODE_SINGLE_INPUT && p->parsing_started)
	{
		type = TOK_NEWLINE; /* Add an extra newline */
		p->parsing_started = 0;

		if (p->tok->indent && !(p->flags & PARSE_DONT_IMPLY_DEDENT))
		{
			p->tok->pendin = -p->tok->indent;
			p->tok->indent = 0;
		}
	}
	else
	{
		p->parsing_started = 1;
	}

	if (p->fill == p->size)
	{
		int newsize = p->size * 2;
		Token **new_tokens = (Token **)Mem_Realloc(p->tokens, newsize * sizeof(Token *));
		if (new_tokens == NULL)
		{
			ViError_NoMemory();
			return TOK_UNKNOWN;
		}
		p->tokens = new_tokens;

		for (int i = p->size; i < newsize; i++)
		{
			p->tokens[i] = (Token *)Mem_Alloc(sizeof(Token));
			if (p->tokens[i] == NULL)
			{
				p->size = i; // Needed, in order to cleanup correctly after parser fails
				ViError_NoMemory();
				return TOK_UNKNOWN;
			}
			memset(p->tokens[i], '\0', sizeof(Token));
		}
		p->size = newsize;
	}

	Token *t = p->tokens[p->fill];
	t->type = static_cast<token_type>((type == TOK_NAME) ? get_keyword_or_name_type(p, start, (int)(end - start)) : type);
	t->value = ViStringObject_FromStringAndSize(start, end - start);
	if (t->value == NULL)
		return TOK_UNKNOWN;

	int lineno = type == TOK_STRING ? p->tok->first_lineno : p->tok->lineno;
	const char *line_start = type == TOK_STRING ? p->tok->multi_line_start : p->tok->line_start;
	int end_lineno = p->tok->lineno;
	int col_offset = -1;
	int end_col_offset = -1;
	if (start != NULL && start >= line_start)
	{
		col_offset = (int)(start - line_start);
	}
	if (end != NULL && end >= p->tok->line_start)
	{
		end_col_offset = (int)(end - p->tok->line_start);
	}

	t->lineno = p->starting_lineno + lineno;
	t->col_offset = p->tok->lineno == 1 ? p->starting_col_offset + col_offset : col_offset;
	t->end_lineno = p->starting_lineno + end_lineno;
	t->end_col_offset = p->tok->lineno == 1 ? p->starting_col_offset + end_col_offset : end_col_offset;

	p->fill += 1;

	if (type == TOK_ERRORTOKEN)
	{
		if (p->tok->done == E_DECODE)
		{
			ViError_SetString(ViExc_SystemError, "unknown error");
			return TOK_UNKNOWN;
		}
		return tokenizer_error(p);
	}

	return TOK_ENDMARKER;
}

Token *ViGen_ExpectToken(Parser *p, int type)
{
	if (p->mark == p->fill)
	{
		if (ViGen_FillToken(p) < 0)
		{
			p->error_indicator = 1;
			return NULL;
		}
	}
	Token *t = p->tokens[p->mark];
	if (t->type != type)
		return NULL;
	p->mark += 1;
	return t;
}

int ViGen_LookaheadWithInt(int positive, Token *(func)(Parser *, int), Parser *p, int arg)
{
	int mark = p->mark;
	void *result = func(p, arg);
	p->mark = mark;
	return (result != NULL) == positive;
}

int ViGen_Lookahead(int positive, void* (func)(Parser*), Parser* p)
{
	int mark = p->mark;
	void* res = (void*)func(p);
	p->mark = mark;
	return (res != NULL) == positive;
}

Token *ViGen_GetLastNonWhitespaceToken(Parser *p)
{
	assert(p->mark >= 0);
	Token *token = NULL;
	for (int m = p->mark - 1; m >= 0; m--)
	{
		token = p->tokens[m];
		if (token->type != TOK_ENDMARKER && (token->type < TOK_NEWLINE || token->type > TOK_DEDENT))
		{
			break;
		}
	}
	return token;
}

/* AST functions */

ast_stmt_seq *ViGen_InteractiveExit(Parser *p)
{
	if (p->error_code)
	{
		*(p->error_code) = E_EOF;
	}
	return NULL;
}

static ViObject *parse_number_raw(const char *s)
{
	const char *end;
	long x;
	double dx;
	ViComplex compl;
	int imflag;

	assert(s != NULL);
	errno = 0;
	end = s + strlen(s) - 1;
	imflag = *end == 'j' || *end == 'J';
	if (s[0] == '0')
	{
		
		x = (long)strtoul(s, (char **)&end, 0);
		if (x < 0 && errno == 0)
		{
			return ViIntObject_FromString(s, 0);
		}
	}
	else
	{
		x = strtol(s, (char **)&end, 0);
	}
	if (*end == '\0')
	{
		if (errno != 0)
		{
			return ViIntObject_FromString(s,  0);
		}
		return ViIntObject_FromInt(x);
	}
	// XXX Huge floats may silently fail
	if (imflag)
	{
		compl.real = 0.;
		compl.imag = strtod(s, (char **)&end);
		if (compl.imag == -1.0 && ViError_Occurred())
		{
			return NULL;
		}
		return ViComplexObject_FromComplex(compl);
	}
	dx = strtod(s, NULL);
	if (dx == -1.0 && ViError_Occurred())
	{
		return NULL;
	}
	return ViFloatObject_FromDouble(dx);
}

static ViObject *parse_number(const char *s)
{
	char *dup;
	char *end;
	ViObject *result = NULL;

	assert(s != NULL);

	if (strchr(s, '_') == NULL)
	{
		return parse_number_raw(s);
	}
	/* Create a duplicate without underscores. */
	dup = (char *)Mem_Alloc(strlen(s) + 1);
	if (dup == NULL)
	{
		ViError_NoMemory();
		return NULL;
	}
	end = dup;
	for (; *s; s++)
	{
		if (*s != '_')
		{
			*end++ = *s;
		}
	}
	*end = '\0';
	result = parse_number_raw(dup);
	Mem_Free(dup);
	return result;
}

expr_type ViGen_NumberToken(Parser *p)
{
	Token *t = ViGen_ExpectToken(p, TOK_NUMBER);
	if (t == NULL)
		return NULL;

	char *raw_num = ViString_ToString(t->value);
	if (raw_num == NULL)
	{
		p->error_indicator = 1;
		return NULL;
	}

	ViObject *num = parse_number(raw_num);

	if (num == NULL)
	{
		p->error_indicator = 1;
		return NULL;
	}

	if (ViArena_AddViObject(p->arena, num) < 0)
	{
		ViObject_DECREF(num);
		p->error_indicator = 1;
		return NULL;
	}

	return ViAST_Constant(num, NULL, t->lineno, t->col_offset, t->end_lineno, t->end_col_offset, p->arena);
}

void* ViGen_StringToken(Parser* p)
{
	return ViGen_ExpectToken(p, TOK_STRING);
}

expr_type ViGen_ConcatStrings(Parser *p, ast_seq *strings)
{
	Vi_size_t len = ViAST_SEQ_LENGTH(strings);
	assert(len > 0);

	Token *first = (Token *)ViAST_SEQ_GET_UNTYPED(strings, 0);
	Token *last = (Token *)ViAST_SEQ_GET_UNTYPED(strings, len - 1);

	int bytesmode = 0;
	ViObject *bytes_str = NULL;

	FstringParser state;
	ViStringParser_Init(&state);

	for (Vi_size_t i = 0; i < len; i++)
	{
		Token *t = (Token *)ViAST_SEQ_GET_UNTYPED(strings, i);

		int this_bytesmode;
		int this_rawmode;
		ViObject *s;
		const char *fstr;
		Vi_size_t fstrlen = -1;

		if (ViStringParser_ParseStr(p, &this_bytesmode, &this_rawmode, &s, &fstr, &fstrlen, t) != 0)
			goto error;

		/* Check that we are not mixing bytes with unicode. */
		if (i != 0 && bytesmode != this_bytesmode)
		{
			ViError_SetString(ViExc_SyntaxError, "cannot mix bytes and nonbytes literals");
			ViObject_XDECREF(s);
			goto error;
		}
		bytesmode = this_bytesmode;

		if (fstr != NULL)
		{
			assert(s == NULL && !bytesmode);
			int result = ViStringParser_ConcatFstring(p, &state, &fstr, fstr + fstrlen, this_rawmode, 0, first, t, last);
			if (result < 0)
				goto error;
		}
		else
		{
			/* String or byte string */
		}
	}

	return expr_type();
}

ast_seq *ViGen_SingletonSeq(Parser *p, void *a)
{
	assert(a != NULL);
	ast_seq *seq = (ast_seq *)ViAST_NewGenericSeq(1);
	if (!seq)
		return NULL;
	ViAST_SEQ_SET_UNTYPED(seq, 0, a);
	return seq;
}

mod_type ViParser_ASTFromFileObject(std::ifstream *fp, ViObject *filename, int mode, const char *ps1, const char *ps2, int *error_code, ViArena *arena)
{
	return gen_run_parser_from_file_pointer(fp, filename, mode, ps1, ps2, error_code, arena);
}
