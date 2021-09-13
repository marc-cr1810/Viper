#ifndef __STRINGPARSER_H__
#define __STRINGPARSER_H__

#include "../port.h"
#include "ast.h"
#include "parser.h"
#include "../objects/object.h"

#define EXPRLIST_N_CACHED  64

typedef struct
{
    /* Incrementally build an array of expr_ty, so be used in an
       asdl_seq. Cache some small but reasonably sized number of
       expr_ty's, and then after that start dynamically allocating,
       doubling the number allocated each time. Note that the f-string
       f'{0}a{1}' contains 3 expr_ty's: 2 FormattedValue's, and one
       Constant for the literal 'a'. So you add expr_ty's about twice as
       fast as you add expressions in an f-string. */

    Vi_size_t allocated;  /* Number we've allocated. */
    Vi_size_t size;       /* Number we've used. */
    expr_type *p;         /* Pointer to the memory we're actually
                              using. Will point to 'data' until we
                              start dynamically allocating. */
    expr_type    data[EXPRLIST_N_CACHED];
} ExprList;

/* The FstringParser is designed to add a mix of strings and
   f-strings, and concat them together as needed. Ultimately, it
   generates an expr_ty. */
typedef struct
{
    ViObject *last_str;
    ExprList expr_list;
    int fmode;
} FstringParser;

void ViStringParser_Init(FstringParser *state);
int ViStringParser_ParseStr(Parser *p, int *bytesmode, int *rawmode, ViObject **result, const char **fstr, Vi_size_t *fstrlen, Token *t);
int ViStringParser_ConcatFstring(Parser *p, FstringParser *state, const char **str, const char *end, int raw, int recurse_lvl, Token *first_token, Token *t, Token *last_token);
int ViStringParser_ConcatAndDel(FstringParser *state, ViObject *str);
expr_type ViStringParser_Finish(Parser *p, FstringParser *state, Token *first_token, Token *last_token);
void ViStringParser_Dealloc(FstringParser *state);

#endif // __STRINGPARSER_H__