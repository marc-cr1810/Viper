#ifndef __VIGEN_H__
#define __VIGEN_H__

#include "../port.h"

#include "ast.h"
#include "tokenizer.h"
#include "parser.h"

#include "../objects/object.h"

/* Memoization functions */

void ViGen_Memo_ClearStats();
ViObject *ViGen_Memo_GetStats();
int ViGen_IsMemoized(Parser *p, int type, void *pres);

// Here, mark is the start of the node, while p->mark is the end.
// If node==NULL, they should be the same.
int ViGen_Memo_Insert(Parser *p, int mark, int type, void *node);
// Like ViGen_Memo_Insert(), but updates an existing node if found.
int ViGen_Memo_Update(Parser *p, int mark, int type, void *node);

/* Tokenizer functions */

int ViGen_FillToken(Parser *p);
Token *ViGen_ExpectToken(Parser *p, int type);

int ViGen_LookaheadWithInt(int positive, Token *(func)(Parser *, int), Parser *p, int arg);
int ViGen_Lookahead(int positive, void* (func)(Parser*), Parser* p);

Token *ViGen_GetLastNonWhitespaceToken(Parser *p);

/* AST functions */

ast_stmt_seq *ViGen_InteractiveExit(Parser *p);

expr_type ViGen_NumberToken(Parser *p);
void* ViGen_StringToken(Parser* p);

expr_type ViGen_ConcatStrings(Parser *p, ast_seq *strings);

// Creates a single-element ast_seq* that contains a
ast_seq *ViGen_SingletonSeq(Parser *p, void *a);

mod_type ViParser_ASTFromFileObject(std::ifstream *fp, ViObject *filename, int mode, const char *ps1, const char *ps2, int *error_code, ViArena *arena);

#endif // __VIGEN_H__