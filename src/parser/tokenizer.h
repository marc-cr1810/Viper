#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include "../port.h"
#include "../objects/object.h"

typedef struct _tokstate TokState;

#include "token.h"

#define MAX_INDENT 100 // Maximum amount of indentations
#define MAX_PAREN 200 // Maximum amount of parentheses

#define TABSIZE 8 // DO NOT CHANGE THIS EVER
#define ALTTABSIZE 1 // Alternate tab spacing

/* Checks to see if a char could be a potential identifier */
#define is_potential_identifier_start(c) (\
              (c >= 'a' && c <= 'z')\
               || (c >= 'A' && c <= 'Z')\
               || c == '_'\
               || (c >= 128))

#define is_potential_identifier_char(c) (\
              (c >= 'a' && c <= 'z')\
               || (c >= 'A' && c <= 'Z')\
               || (c >= '0' && c <= '9')\
               || c == '_'\
               || (c >= 128))

enum class decoding_state
{
    STATE_INIT,
    STATE_RAW,
    STATE_NORMAL        /* have a codec associated with input */
};

typedef struct _tokstate
{
    /* Input state; buf <= cur <= inp <= end */
    /* NB an entire line is held in the buffer */
    char *buf;          /* Input buffer, or NULL; malloc'ed if fp != NULL */
    char *cur;          /* Next character in buffer */
    char *inp;          /* End of data in buffer */
    const char *end;    /* End of input buffer if buf != NULL */
    const char *start;  /* Start of current token if not NULL */
    int done;           /* E_OK normally, E_EOF at EOF, otherwise error code */
    /* NB If done != E_OK, cur must be == inp!!! */
    std::ifstream *fp;           /* Rest of input; NULL if tokenizing a string */
    int tabsize;        /* Tab spacing */
    int indent;         /* Current indentation index */
    int indstack[MAX_INDENT];            /* Stack of indents */
    int atbol;          /* Nonzero if at begin of new line */
    int pendin;         /* Pending indents (if > 0) or dedents (if < 0) */
    const char *prompt, *nextprompt;          /* For interactive prompting */
    char *stdin_content;
    int lineno;         /* Current line number */
    int first_lineno;   /* First line of a single line or multi line string
                           expression (cf. issue 16806) */
    int level;          /* () [] {} Parentheses nesting level */
            /* Used to allow free continuations inside them */
    char parenstack[MAX_PAREN];
    int parenlinenostack[MAX_PAREN];
    ViObject *filename;
    /* Stuff for checking on different tab sizes */
    int altindstack[MAX_INDENT];         /* Stack of alternate indents */
    /* Stuff for PEP 0263 */
    enum decoding_state decoding_state;
    int decoding_erred;         /* whether erred in decoding  */
    int read_coding_spec;       /* whether 'coding:...' has been read  */
    char *encoding;         /* Source encoding. */
    int cont_line;          /* whether we are in a continuation line. */
    const char *line_start;     /* pointer to start of current line */
    const char *multi_line_start; /* pointer to start of first line of
                                     a single line or multi line string
                                     expression (cf. issue 16806) */
    ViObject *decoding_readline; /* open(...).readline */
    ViObject *decoding_buffer;
    const char *enc;        /* Encoding for the current str. */
    char *str;
    char *input;       /* Tokenizer's newline translated copy of the string. */

    int type_comments;      /* Whether to look for type comments */

    /* async/await related fields (still needed depending on feature_version) */
    int async_hacks;     /* =1 if async/await aren't always keywords */
    int async_def;        /* =1 if tokens are inside an 'async def' body. */
    int async_def_indent; /* Indentation level of the outermost 'async def'. */
    int async_def_nl;     /* =1 if the outermost 'async def' had at least one
                             NEWLINE token after it. */
} TokState;

extern TokState* ViTokenizer_FromFile(std::ifstream* fp, const char* ps1, const char* ps2);

extern int ViTokenizer_Get(TokState* tok, const char **p_start, const char **p_end);
extern void ViTokenizer_Free(TokState *tok);

#endif // __TOKENIZER_H__