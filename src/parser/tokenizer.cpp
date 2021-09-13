#include "tokenizer.h"

#include "../core/victype.h"
#include "../core/vimem.h"
#include "../core/errorcode.h"
#include "../core/error.h"
#include "../core/visys.h"

// Spaces in this constant are treated as "zero or more spaces or tabs" when tokenizing.
static const char *type_comment_prefix = "# type: ";

static TokState *tokenizer_new()
{
	TokState *tok = (TokState *)Mem_Alloc(sizeof(TokState));
	if (tok == NULL)
		return NULL;
	tok->buf = tok->cur = tok->inp = NULL;
	tok->start = NULL;
	tok->end = NULL;
	tok->done = E_OK;
	tok->fp = NULL;
	tok->input = NULL;
	tok->tabsize = TABSIZE;
	tok->indent = 0;
	tok->indstack[0] = 0;

	tok->atbol = 1;
	tok->pendin = 0;
	tok->prompt = tok->nextprompt = NULL;
	tok->lineno = 0;
	tok->level = 0;
	tok->altindstack[0] = 0;
	tok->decoding_state = decoding_state::STATE_INIT;
	tok->decoding_erred = 0;
	tok->read_coding_spec = 0;
	tok->enc = NULL;
	tok->encoding = NULL;
	tok->cont_line = 0;
	tok->filename = NULL;
	tok->decoding_readline = NULL;
	tok->decoding_buffer = NULL;
	tok->type_comments = 0;
	tok->stdin_content = NULL;

	return tok;
}

/* Error helpers */
static int indenterror(TokState *tok)
{
	tok->done = E_TABSPACE;
	tok->cur = tok->inp;
	return TOK_ERRORTOKEN;
}

static int syntaxerror(TokState *tok, const char *msg)
{
	ViError_SetString(ViExc_SyntaxError, msg);
	tok->done = E_ERROR;
	return TOK_ERRORTOKEN;
}

/* File pointer helpers */

// Get the next byte from tokenizer
static int file_get_char(TokState *tok)
{
	return tok->fp->get();
}

// Peek at the next byte in tokenizer
static int file_peek_char(TokState *tok)
{
	return tok->fp->peek();
}

// Put back the last byte into tokenizer
static void file_unget_char(TokState *tok)
{
	tok->fp->unget();
}

/* String helpers */

// Get a byte from tokenizer using the input buffer
static int string_get_char(TokState *tok)
{
	return VI_CHARMASK(*tok->input++);
}

// Put back a byte from tokenizer using the input buffer
static void string_unget_char(TokState *tok)
{
	tok->input--;
}

/* Tokenizer helpers */

static char *translate_newlines(const char *s, int exec_input, TokState *tok)
{
	int skip_next_lf = 0;
	size_t needed_length = strlen(s) + 2, final_length;
	char *buf, *current;
	char c = '\0';
	buf = (char *)Mem_Alloc(needed_length);
	if (buf == NULL)
	{
		tok->done = E_NOMEM;
		return NULL;
	}
	for (current = buf; *s; s++, current++)
	{
		c = *s;
		if (skip_next_lf)
		{
			skip_next_lf = 0;
			if (c == '\n')
			{
				c = *++s;
				if (!c)
					break;
			}
		}
		if (c == '\r')
		{
			skip_next_lf = 1;
			c = '\n';
		}
		*current = c;
	}
	/* If this is exec input, add a newline to the end of the string if
	   there isn't one already. */
	if (exec_input && c != '\n')
	{
		*current = '\n';
		current++;
	}
	*current = '\0';
	final_length = current - buf + 1;
	if (final_length < needed_length && final_length)
	{
		/* should never fail */
		char *result = (char *)Mem_Realloc(buf, final_length);
		if (result == NULL)
		{
			Mem_Free(buf);
		}
		buf = result;
	}
	return buf;
}

// Get next char
static char tokenizer_next(TokState *tok)
{
	for (;;)
	{
		if (tok->cur != tok->inp)
			return VI_CHARMASK(*tok->cur++);
		if (tok->done != E_OK)
			return EOF;
		if (tok->fp == NULL)
		{
			char *end = strchr(tok->inp, '\n');
			if (end != NULL)
				end++;
			else
			{
				end = strchr(tok->inp, '\0');
				if (end == tok->inp)
				{
					tok->done = E_EOF;
					return EOF;
				}
			}
			if (tok->start == NULL)
				tok->buf = tok->cur;
			tok->line_start = tok->cur;
			tok->lineno++;
			tok->inp = end;
			return VI_CHARMASK(*tok->cur++);
		}
		if (tok->prompt != NULL)
		{
			char *newtok = ViSys_ReadLine(tok->prompt);
			if (newtok != NULL)
			{
				char *translated = translate_newlines(newtok, 0, tok);
				Mem_Free(newtok);
				if (translated == NULL)
					return EOF;
				newtok = translated;
				if (tok->stdin_content == NULL)
				{
					tok->stdin_content = (char *)Mem_Alloc(strlen(translated) + 1);
					if (tok->stdin_content == NULL)
					{
						tok->done = E_NOMEM;
						return EOF;
					}
					sprintf(tok->stdin_content, "%s", translated);
				}
				else
				{
					char *new_str = (char *)Mem_Alloc(strlen(tok->stdin_content) + strlen(translated) + 1);
					if (new_str == NULL)
					{
						tok->done = E_NOMEM;
						return EOF;
					}
					sprintf(new_str, "%s%s", tok->stdin_content, translated);
					Mem_Free(tok->stdin_content);
					tok->stdin_content = new_str;
				}
			}
			if (tok->nextprompt != NULL)
				tok->prompt = tok->nextprompt;
			if (newtok == NULL)
				tok->done = E_INTR;
			else if (*newtok == '\0')
			{
				Mem_Free(newtok);
				tok->done = E_EOF;
			}
			else if (tok->start != NULL)
			{
				size_t start = tok->start - tok->buf;
				size_t oldlen = tok->cur - tok->buf;
				size_t newlen = oldlen + strlen(newtok);
				Vi_size_t cur_multi_line_start = tok->multi_line_start - tok->buf;
				char *buf = tok->buf;
				buf = (char *)Mem_Realloc(buf, newlen + 1);
				tok->lineno++;
				if (buf == NULL)
				{
					Mem_Free(tok->buf);
					tok->buf = NULL;
					Mem_Free(newtok);
					tok->done = E_NOMEM;
					return EOF;
				}
				tok->buf = buf;
				tok->cur = tok->buf + oldlen;
				tok->multi_line_start = tok->buf + cur_multi_line_start;
				tok->line_start = tok->cur;
				strcpy(tok->buf + oldlen, newtok);
				Mem_Free(newtok);
				tok->inp = tok->buf + newlen;
				tok->end = tok->inp + 1;
				tok->start = tok->buf + start;
			}
			else
			{
				tok->lineno++;
				if (tok->buf != NULL)
					Mem_Free(tok->buf);
				tok->buf = newtok;
				tok->cur = tok->buf;
				tok->line_start = tok->buf;
				tok->inp = strchr(tok->buf, '\0');
				tok->end = tok->inp + 1;
			}
		}
		///TODO: File pointer readline support
	}
}

static void tokenizer_back(TokState *tok, char c)
{
	if (c != EOF)
	{
		if (--tok->cur < tok->buf)
		{
			ViError_SetString(ViExc_SystemError, "tokenizer at beginning of buffer");
			ViError_Print();
		}
		if (*tok->cur != c)
			*tok->cur = c;
	}
}

static int decimal_tail(TokState *tok)
{
	int c;

	while (1)
	{
		do
		{
			c = tokenizer_next(tok);
		}
		while (isdigit(c));
		if (c != '_')
		{
			break;
		}
		c = tokenizer_next(tok);
		if (!isdigit(c))
		{
			tokenizer_back(tok, c);
			syntaxerror(tok, "invalid decimal literal");
			return 0;
		}
	}
	return c;
}

int tokenizer_get(TokState *tok, const char **p_start, const char **p_end)
{
	int c;
	int blankline, nonascii;

	*p_start = *p_end = NULL;
nextline:
	tok->start = NULL;
	blankline = 0;

	/* Get indentation level */
	if (tok->atbol)
	{
		int col = 0;
		int altcol = 0;
		tok->atbol = 0;
		for (;;)
		{
			c = tokenizer_next(tok);
			if (c == ' ')
			{
				col++, altcol++;
			}
			else if (c == '\t')
			{
				col = (col / tok->tabsize + 1) * tok->tabsize;
				altcol = (altcol / ALTTABSIZE + 1) * ALTTABSIZE;
			}
			else if (c == '\014')
			{/* Control-L (formfeed) */
				col = altcol = 0; /* For Emacs users */
			}
			else
			{
				break;
			}
		}
		tokenizer_back(tok, c);
		if (c == '#' || c == '\n' || c == '\\')
		{
			/* Lines with only whitespace and/or comments
			   and/or a line continuation character
			   shouldn't affect the indentation and are
			   not passed to the parser as NEWLINE tokens,
			   except *totally* empty lines in interactive
			   mode, which signal the end of a command group. */
			if (col == 0 && c == '\n' && tok->prompt != NULL)
			{
				blankline = 0; /* Let it through */
			}
			else if (tok->prompt != NULL && tok->lineno == 1)
			{
				/* In interactive mode, if the first line contains
				   only spaces and/or a comment, let it through. */
				blankline = 0;
				col = altcol = 0;
			}
			else
			{
				blankline = 1; /* Ignore completely */
			}
			/* We can't jump back right here since we still
			   may need to skip to the end of a comment */
		}
		if (!blankline && tok->level == 0)
		{
			if (col == tok->indstack[tok->indent])
			{
				/* No change */
				if (altcol != tok->altindstack[tok->indent])
				{
					return indenterror(tok);
				}
			}
			else if (col > tok->indstack[tok->indent])
			{
				/* Indent -- always one */
				if (tok->indent + 1 >= MAX_INDENT)
				{
					tok->done = E_TOODEEP;
					tok->cur = tok->inp;
					return TOK_ERRORTOKEN;
				}
				if (altcol <= tok->altindstack[tok->indent])
				{
					return indenterror(tok);
				}
				tok->pendin++;
				tok->indstack[++tok->indent] = col;
				tok->altindstack[tok->indent] = altcol;
			}
			else /* col < tok->indstack[tok->indent] */
			{
				/* Dedent -- any number, must be consistent */
				while (tok->indent > 0 &&
					   col < tok->indstack[tok->indent])
				{
					tok->pendin--;
					tok->indent--;
				}
				if (col != tok->indstack[tok->indent])
				{
					tok->done = E_DEDENT;
					tok->cur = tok->inp;
					return TOK_ERRORTOKEN;
				}
				if (altcol != tok->altindstack[tok->indent])
				{
					return indenterror(tok);
				}
			}
		}
	}

	tok->start = tok->cur;

	/* Return pending indents/dedents */
	if (tok->pendin != 0)
	{
		if (tok->pendin < 0)
		{
			tok->pendin++;
			return TOK_DEDENT;
		}
		else
		{
			tok->pendin--;
			return TOK_INDENT;
		}
	}

	/* Peek ahead at the next character */
	c = tokenizer_next(tok);
	tokenizer_back(tok, c);
	/* Check if we are closing an async function */
	if (tok->async_def
		&& !blankline
		/* Due to some implementation artifacts of type comments,
		 * a TYPE_COMMENT at the start of a function won't set an
		 * indentation level and it will produce a NEWLINE after it.
		 * To avoid spuriously ending an async function due to this,
		 * wait until we have some non-newline char in front of us. */
		&& c != '\n'
		&& tok->level == 0
		/* There was a NEWLINE after ASYNC DEF,
		   so we're past the signature. */
		&& tok->async_def_nl
		/* Current indentation level is less than where
		   the async function was defined */
		&& tok->async_def_indent >= tok->indent)
	{
		tok->async_def = 0;
		tok->async_def_indent = 0;
		tok->async_def_nl = 0;
	}

again:
	tok->start = NULL;
	/* Skip spaces */
	do
	{
		c = tokenizer_next(tok);
	}
	while (c == ' ' || c == '\t' || c == '\014');

	/* Set start of current token */
	tok->start = tok->cur - 1;

	/* Skip comment, unless it's a type comment */
	if (c == '#')
	{
		const char *prefix, *p, *type_start;

		while (c != EOF && c != '\n')
		{
			c = tokenizer_next(tok);
		}

		if (tok->type_comments)
		{
			p = tok->start;
			prefix = type_comment_prefix;
			while (*prefix && p < tok->cur)
			{
				if (*prefix == ' ')
				{
					while (*p == ' ' || *p == '\t')
					{
						p++;
					}
				}
				else if (*prefix == *p)
				{
					p++;
				}
				else
				{
					break;
				}

				prefix++;
			}

			/* This is a type comment if we matched all of type_comment_prefix. */
			if (!*prefix)
			{
				int is_type_ignore = 1;
				const char *ignore_end = p + 6;
				tokenizer_back(tok, c);  /* don't eat the newline or EOF */

				type_start = p;

				/* A TYPE_IGNORE is "type: ignore" followed by the end of the token
				 * or anything ASCII and non-alphanumeric. */
				is_type_ignore = (
					tok->cur >= ignore_end && memcmp(p, "ignore", 6) == 0
					&& !(tok->cur > ignore_end
						 && ((unsigned char)ignore_end[0] >= 128 || Vi_ISALNUM(ignore_end[0]))));

				if (is_type_ignore)
				{
					*p_start = ignore_end;
					*p_end = tok->cur;

					/* If this type ignore is the only thing on the line, consume the newline also. */
					if (blankline)
					{
						tokenizer_next(tok);
						tok->atbol = 1;
					}
					return TOK_TYPE_IGNORE;
				}
				else
				{
					*p_start = type_start;  /* after type_comment_prefix */
					*p_end = tok->cur;
					return TOK_TYPE_COMMENT;
				}
			}
		}
	}

	/* Check for EOF and errors now */
	if (c == EOF)
	{
		return tok->done == E_EOF ? TOK_ENDMARKER : TOK_ERRORTOKEN;
	}

	/* Identifier (most frequent token!) */
	nonascii = 0;
	if (is_potential_identifier_start(c))
	{
		/* Process the various legal combinations of b"", r"", u"", and f"". */
		int saw_b = 0, saw_r = 0, saw_u = 0, saw_f = 0;
		while (1)
		{
			if (!(saw_b || saw_u || saw_f) && (c == 'b' || c == 'B'))
				saw_b = 1;
			/* Since this is a backwards compatibility support literal we don't
			   want to support it in arbitrary order like byte literals. */
			else if (!(saw_b || saw_u || saw_r || saw_f)
					 && (c == 'u' || c == 'U'))
			{
				saw_u = 1;
			}
			/* ur"" and ru"" are not supported */
			else if (!(saw_r || saw_u) && (c == 'r' || c == 'R'))
			{
				saw_r = 1;
			}
			else if (!(saw_f || saw_b || saw_u) && (c == 'f' || c == 'F'))
			{
				saw_f = 1;
			}
			else
			{
				break;
			}
			c = tokenizer_next(tok);
			if (c == '"' || c == '\'')
			{
				goto letter_quote;
			}
		}
		while (is_potential_identifier_char(c))
		{
			if (c >= 128)
			{
				nonascii = 1;
			}
			c = tokenizer_next(tok);
		}
		tokenizer_back(tok, c);
		if (nonascii)
		{
			return TOK_ERRORTOKEN;
		}

		*p_start = tok->start;
		*p_end = tok->cur;

		/* async/await parsing block. */
		if (tok->cur - tok->start == 5 && tok->start[0] == 'a')
		{
			/* May be an 'async' or 'await' token.  For Python 3.7 or
			   later we recognize them unconditionally.  For Python
			   3.5 or 3.6 we recognize 'async' in front of 'def', and
			   either one inside of 'async def'.  (Technically we
			   shouldn't recognize these at all for 3.4 or earlier,
			   but there's no *valid* Python 3.4 code that would be
			   rejected, and async functions will be rejected in a
			   later phase.) */
			if (!tok->async_hacks || tok->async_def)
			{
				/* Always recognize the keywords. */
				if (memcmp(tok->start, "async", 5) == 0)
				{
					return TOK_ASYNC;
				}
				if (memcmp(tok->start, "await", 5) == 0)
				{
					return TOK_AWAIT;
				}
			}
			else if (memcmp(tok->start, "async", 5) == 0)
			{
				/* The current token is 'async'.
				   Look ahead one token to see if that is 'def'. */

				TokState ahead_tok;
				const char *ahead_tok_start = NULL;
				const char *ahead_tok_end = NULL;
				int ahead_tok_kind;

				memcpy(&ahead_tok, tok, sizeof(ahead_tok));
				ahead_tok_kind = ViTokenizer_Get(&ahead_tok, &ahead_tok_start,
												 &ahead_tok_end);

				if (ahead_tok_kind == TOK_NAME
					&& ahead_tok.cur - ahead_tok.start == 3
					&& memcmp(ahead_tok.start, "def", 3) == 0)
				{
					/* The next token is going to be 'def', so instead of
					   returning a plain NAME token, return ASYNC. */
					tok->async_def_indent = tok->indent;
					tok->async_def = 1;
					return TOK_ASYNC;
				}
			}
		}

		return TOK_NAME;
	}

	/* Newline */
	if (c == '\n')
	{
		tok->atbol = 1;
		if (blankline || tok->level > 0)
		{
			goto nextline;
		}
		*p_start = tok->start;
		*p_end = tok->cur - 1; /* Leave '\n' out of the string */
		tok->cont_line = 0;
		if (tok->async_def)
		{
			/* We're somewhere inside an 'async def' function, and
			   we've encountered a NEWLINE after its signature. */
			tok->async_def_nl = 1;
		}
		return TOK_NEWLINE;
	}

	/* Period or number starting with period? */
	if (c == '.')
	{
		c = tokenizer_next(tok);
		if (isdigit(c))
		{
			goto fraction;
		}
		else if (c == '.')
		{
			c = tokenizer_next(tok);
			if (c == '.')
			{
				*p_start = tok->start;
				*p_end = tok->cur;
				return TOK_ELLIPSIS;
			}
			else
			{
				tokenizer_back(tok, c);
			}
			tokenizer_back(tok, '.');
		}
		else
		{
			tokenizer_back(tok, c);
		}
		*p_start = tok->start;
		*p_end = tok->cur;
		return TOK_DOT;
	}

	/* Number */
	if (isdigit(c))
	{
		if (c == '0')
		{
			/* Hex, octal or binary -- maybe. */
			c = tokenizer_next(tok);
			if (c == 'x' || c == 'X')
			{
				/* Hex */
				c = tokenizer_next(tok);
				do
				{
					if (c == '_')
					{
						c = tokenizer_next(tok);
					}
					if (!isxdigit(c))
					{
						tokenizer_back(tok, c);
						return syntaxerror(tok, "invalid hexadecimal literal");
					}
					do
					{
						c = tokenizer_next(tok);
					}
					while (isxdigit(c));
				}
				while (c == '_');
			}
			else if (c == 'o' || c == 'O')
			{
				/* Octal */
				c = tokenizer_next(tok);
				do
				{
					if (c == '_')
					{
						c = tokenizer_next(tok);
					}
					if (c < '0' || c >= '8')
					{
						tokenizer_back(tok, c);
						if (isdigit(c))
						{
							return syntaxerror(tok,
											   "invalid digit in octal literal");
						}
						else
						{
							return syntaxerror(tok, "invalid octal literal");
						}
					}
					do
					{
						c = tokenizer_next(tok);
					}
					while ('0' <= c && c < '8');
				}
				while (c == '_');
				if (isdigit(c))
				{
					return syntaxerror(tok,
									   "invalid digit in octal literal");
				}
			}
			else if (c == 'b' || c == 'B')
			{
				/* Binary */
				c = tokenizer_next(tok);
				do
				{
					if (c == '_')
					{
						c = tokenizer_next(tok);
					}
					if (c != '0' && c != '1')
					{
						tokenizer_back(tok, c);
						if (isdigit(c))
						{
							return syntaxerror(tok,
											   "invalid digit in binary literal");
						}
						else
						{
							return syntaxerror(tok, "invalid binary literal");
						}
					}
					do
					{
						c = tokenizer_next(tok);
					}
					while (c == '0' || c == '1');
				}
				while (c == '_');
				if (isdigit(c))
				{
					return syntaxerror(tok,
									   "invalid digit in binary literal");
				}
			}
			else
			{
				int nonzero = 0;
				/* maybe old-style octal; c is first char of it */
				/* in any case, allow '0' as a literal */
				while (1)
				{
					if (c == '_')
					{
						c = tokenizer_next(tok);
						if (!isdigit(c))
						{
							tokenizer_back(tok, c);
							return syntaxerror(tok, "invalid decimal literal");
						}
					}
					if (c != '0')
					{
						break;
					}
					c = tokenizer_next(tok);
				}
				if (isdigit(c))
				{
					nonzero = 1;
					c = decimal_tail(tok);
					if (c == 0)
					{
						return TOK_ERRORTOKEN;
					}
				}
				if (c == '.')
				{
					c = tokenizer_next(tok);
					goto fraction;
				}
				else if (c == 'e' || c == 'E')
				{
					goto exponent;
				}
				else if (c == 'j' || c == 'J')
				{
					goto imaginary;
				}
				else if (nonzero)
				{
					/* Old-style octal: now disallowed. */
					tokenizer_back(tok, c);
					return syntaxerror(tok,
									   "leading zeros in decimal integer "
									   "literals are not permitted; "
									   "use an 0o prefix for octal integers");
				}
			}
		}
		else
		{
			/* Decimal */
			c = decimal_tail(tok);
			if (c == 0)
			{
				return TOK_ERRORTOKEN;
			}
			{
				/* Accept floating point numbers. */
				if (c == '.')
				{
					c = tokenizer_next(tok);
				fraction:
					/* Fraction */
					if (isdigit(c))
					{
						c = decimal_tail(tok);
						if (c == 0)
						{
							return TOK_ERRORTOKEN;
						}
					}
				}
				if (c == 'e' || c == 'E')
				{
					int e;
				exponent:
					e = c;
					/* Exponent part */
					c = tokenizer_next(tok);
					if (c == '+' || c == '-')
					{
						c = tokenizer_next(tok);
						if (!isdigit(c))
						{
							tokenizer_back(tok, c);
							return syntaxerror(tok, "invalid decimal literal");
						}
					}
					else if (!isdigit(c))
					{
						tokenizer_back(tok, c);
						tokenizer_back(tok, e);
						*p_start = tok->start;
						*p_end = tok->cur;
						return TOK_NUMBER;
					}
					c = decimal_tail(tok);
					if (c == 0)
					{
						return TOK_ERRORTOKEN;
					}
				}
				if (c == 'j' || c == 'J')
				{
					/* Imaginary part */
				imaginary:
					c = tokenizer_next(tok);
				}
			}
		}
		tokenizer_back(tok, c);
		*p_start = tok->start;
		*p_end = tok->cur;
		return TOK_NUMBER;
	}

letter_quote:
	/* String */
	if (c == '\'' || c == '"')
	{
		int quote = c;
		int quote_size = 1;             /* 1 or 3 */
		int end_quote_size = 0;

		/* Nodes of type STRING, especially multi line strings
		   must be handled differently in order to get both
		   the starting line number and the column offset right.
		   (cf. issue 16806) */
		tok->first_lineno = tok->lineno;
		tok->multi_line_start = tok->line_start;

		/* Find the quote size and start of string */
		c = tokenizer_next(tok);
		if (c == quote)
		{
			c = tokenizer_next(tok);
			if (c == quote)
			{
				quote_size = 3;
			}
			else
			{
				end_quote_size = 1;     /* empty string found */
			}
		}
		if (c != quote)
		{
			tokenizer_back(tok, c);
		}

		/* Get rest of string */
		while (end_quote_size != quote_size)
		{
			c = tokenizer_next(tok);
			if (c == EOF)
			{
				if (quote_size == 3)
				{
					tok->done = E_EOFS;
				}
				else
				{
					tok->done = E_EOLS;
				}
				tok->cur = tok->inp;
				return TOK_ERRORTOKEN;
			}
			if (quote_size == 1 && c == '\n')
			{
				tok->done = E_EOLS;
				tok->cur = tok->inp;
				return TOK_ERRORTOKEN;
			}
			if (c == quote)
			{
				end_quote_size += 1;
			}
			else
			{
				end_quote_size = 0;
				if (c == '\\')
				{
					tokenizer_next(tok);  /* skip escaped char */
				}
			}
		}

		*p_start = tok->start;
		*p_end = tok->cur;
		return TOK_STRING;
	}

	/* Line continuation */
	if (c == '\\')
	{
		c = tokenizer_next(tok);
		if (c != '\n')
		{
			tok->done = E_LINECONT;
			tok->cur = tok->inp;
			return TOK_ERRORTOKEN;
		}
		c = tokenizer_next(tok);
		if (c == EOF)
		{
			tok->done = E_EOF;
			tok->cur = tok->inp;
			return TOK_ERRORTOKEN;
		}
		else
		{
			tokenizer_back(tok, c);
		}
		tok->cont_line = 1;
		goto again; /* Read next line */
	}

	/* Check for two-character token */
	{
		int c2 = tokenizer_next(tok);
		int token = ViToken_TwoChars(c, c2);
		if (token != TOK_OP)
		{
			int c3 = tokenizer_next(tok);
			int token3 = ViToken_ThreeChars(c, c2, c3);
			if (token3 != TOK_OP)
			{
				token = token3;
			}
			else
			{
				tokenizer_back(tok, c3);
			}
			*p_start = tok->start;
			*p_end = tok->cur;
			return token;
		}
		tokenizer_back(tok, c2);
	}

	/* Keep track of parentheses nesting level */
	switch (c)
	{
	case '(':
	case '[':
	case '{':
		if (tok->level >= MAX_PAREN)
		{
			return syntaxerror(tok, "too many nested parentheses");
		}
		tok->parenstack[tok->level] = c;
		tok->parenlinenostack[tok->level] = tok->lineno;
		tok->level++;
		break;
	case ')':
	case ']':
	case '}':
		if (!tok->level)
		{
			return syntaxerror(tok, "unmatched paren");
		}
		tok->level--;
		int opening = tok->parenstack[tok->level];
		if (!((opening == '(' && c == ')') ||
			  (opening == '[' && c == ']') ||
			  (opening == '{' && c == '}')))
		{
			if (tok->parenlinenostack[tok->level] != tok->lineno)
			{
				return syntaxerror(tok,
								   "closing parenthesis does not match "
								   "opening parenthesis");
			}
			else
			{
				return syntaxerror(tok,
								   "closing parenthesis does not match "
								   "opening parenthesis");
			}
		}
		break;
	}

	/* Punctuation character */
	*p_start = tok->start;
	*p_end = tok->cur;
	return ViToken_OneChar(c);
}

TokState *ViTokenizer_FromFile(std::ifstream *fp, const char *ps1, const char *ps2)
{
	TokState *tok = tokenizer_new(); if (tok == NULL)
		return NULL;
	if ((tok->buf = (char *)Mem_Alloc(BUFSIZ)) == NULL)
	{
		ViTokenizer_Free(tok);
		return NULL;
	}
	tok->cur = tok->inp = tok->buf;
	tok->end = tok->buf + BUFSIZ;
	tok->fp = fp;
	tok->prompt = ps1;
	tok->nextprompt = ps2;

	return tok;
}

int ViTokenizer_Get(TokState *tok, const char **p_start, const char **p_end)
{
	int result = tokenizer_get(tok, p_start, p_end);
	if (tok->decoding_erred)
	{
		result = TOK_ERRORTOKEN;
		tok->done = E_DECODE;
	}
	return result;
}

void ViTokenizer_Free(TokState *tok)
{
	if (tok->encoding != NULL)
		Mem_Free(tok->encoding);
	ViObject_XDECREF(tok->decoding_readline);
	ViObject_XDECREF(tok->decoding_buffer);
	ViObject_XDECREF(tok->filename);
	if (tok->fp != NULL && tok->buf != NULL)
		Mem_Free(tok->buf);
	if (tok->input)
		Mem_Free(tok->input);
	if (tok->stdin_content)
		Mem_Free(tok->stdin_content);
	Mem_Free(tok);
}
