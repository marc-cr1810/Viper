#include "ast.h"

#include "../core/vimem.h"
#include "../core/error.h"

ast_generic_seq *ViAST_NewGenericSeq(Vi_size_t size)
{	
	ast_generic_seq *seq = NULL;
	size_t n = 0;

	// Check size is ok
	if (size < 0 ||
		(size && (((size_t)size - 1) > (SIZE_MAX / sizeof(void *)))))
	{
		ViError_NoMemory();
		return NULL;
	}
	n += sizeof(ast_generic_seq);
	seq = (ast_generic_seq *)Mem_Alloc(n);
	if (!seq)
	{
		ViError_NoMemory();
		return NULL;
	}
	memset(seq, 0, n);
	seq->size = size;
	seq->elements = (void **)seq->typed_elements;
	return seq;
}

ast_identifier_seq *ViAST_NewIdentifierSeq(Vi_size_t size)
{
	ast_identifier_seq *seq = NULL;
	size_t n;

	// Check size is ok
	if (size < 0 ||
		(size && (((size_t)size - 1) > (SIZE_MAX / sizeof(void *)))))
	{
		ViError_NoMemory();
		return NULL;
	}
	n += sizeof(ast_identifier_seq);
	seq = (ast_identifier_seq *)Mem_Alloc(n);
	if (!seq)
	{
		ViError_NoMemory();
		return NULL;
	}
	memset(seq, 0, n);
	seq->size = size;
	seq->elements = (void **)seq->typed_elements;
	return seq;
}

ast_int_seq *ViAST_NewIntSeq(Vi_size_t size)
{
	ast_int_seq *seq = NULL;
	size_t n = 0;

	// Check size is ok
	if (size < 0 ||
		(size && (((size_t)size - 1) > (SIZE_MAX / sizeof(void *)))))
	{
		ViError_NoMemory();
		return NULL;
	}
	n += sizeof(ast_int_seq);
	seq = (ast_int_seq *)Mem_Alloc(n);
	if (!seq)
	{
		ViError_NoMemory();
		return NULL;
	}
	memset(seq, 0, n);
	seq->size = size;
	seq->elements = (void **)seq->typed_elements;
	return seq;
}

//
//
//		Create node definitions
//
//

mod_type ViAST_Interactive(ast_stmt_seq *body, ViArena *arena)
{
	mod_type mod;
	///TODO: Fix memory allocation (body has null objects after setting module kind)
	mod = (mod_type)Mem_Alloc(sizeof(*mod)); // Should call ViArena_Alloc here
	if (!mod)
		return NULL;
	mod->kind = module_kind::Interactive_Kind;
	mod->v.Interactive.body = body;
	return mod;
}

stmt_type ViAST_Expr(expr_type value, int lineno, int col_offset, int end_lineno, int end_col_offset, ViArena *arena)
{
	if (!value)
	{
		ViError_SetString(ViExc_ValueError, "field 'value' is required for ViAST_Expr");
		return NULL;
	}
	stmt_type stmt;
	stmt = (stmt_type)ViArena_Alloc(arena, sizeof(stmt));
	if (!stmt)
		return NULL;
	stmt->kind = stmt_kind::Expr_kind;
	stmt->v.Expr.value = value;
	stmt->lineno = lineno;
	stmt->col_offset = col_offset;
	stmt->end_lineno = end_lineno;
	stmt->end_col_offset = end_col_offset;
	return stmt;
}

stmt_type ViAST_Pass(int lineno, int col_offset, int end_lineno, int end_col_offset, ViArena *arena)
{
	stmt_type stmt;
	stmt = (stmt_type)ViArena_Alloc(arena, sizeof(stmt));
	if (!stmt)
		return NULL;
	stmt->kind = stmt_kind::Pass_kind;
	stmt->lineno = lineno;
	stmt->col_offset = col_offset;
	stmt->end_lineno = end_lineno;
	stmt->end_col_offset = end_col_offset;
	return stmt;
}

expr_type ViAST_Constant(constant value, string kind, int lineno, int col_offset, int end_lineno, int end_col_offset, ViArena *arena)
{
	if (!value)
	{
		ViError_SetString(ViExc_ValueError, "field 'value' is required for ViAST_Constant");
		return NULL;
	}
	expr_type expr;
	expr = (expr_type)ViArena_Alloc(arena, sizeof(*expr));
	if (!expr)
		return NULL;
	expr->kind = expr_kind::Constant_kind;
	expr->v.Constant.value = value;
	expr->v.Constant.kind = kind;
	expr->lineno = lineno;
	expr->col_offset = col_offset;
	expr->end_lineno = end_lineno;
	expr->end_col_offset = end_col_offset;
	return expr;
}

expr_type ViAST_BinOp(expr_type left, operator_type op, expr_type right, int lineno, int col_offset, int end_lineno, int end_col_offset, ViArena *arena)
{
	if (!left)
	{
		ViError_SetString(ViExc_ValueError, "field 'left' is required for ViAST_BinOp");
		return NULL;
	}
	if (!static_cast<int>(op))
	{
		ViError_SetString(ViExc_ValueError, "field 'op' is required for ViAST_BinOp");
		return NULL;
	}
	if (!right)
	{
		ViError_SetString(ViExc_ValueError, "field 'right' is required for ViAST_BinOp");
		return NULL;
	}
	expr_type p;
	p = (expr_type)ViArena_Alloc(arena, sizeof(*p));
	if (!p)
		return NULL;
	p->kind = expr_kind::BinOp_kind;
	p->v.BinaryOp.left = left;
	p->v.BinaryOp.op = op;
	p->v.BinaryOp.right = right;
	p->lineno = lineno;
	p->col_offset = col_offset;
	p->end_lineno = end_lineno;
	p->end_col_offset = end_col_offset;
	return p;
}

expr_type ViAST_UnaryOp(unaryop_type op, expr_type operand, int lineno, int col_offset, int end_lineno, int end_col_offset, ViArena *arena)
{
	if (!static_cast<int>(op))
	{
		ViError_SetString(ViExc_ValueError, "field 'op' is required for ViAST_UnaryOp");
		return NULL;
	}
	if (!operand)
	{
		ViError_SetString(ViExc_ValueError, "field 'operand' is required for ViAST_UnaryOp");
		return NULL;
	}
	expr_type p;
	p = (expr_type)ViArena_Alloc(arena, sizeof(*p));
	if (!p)
		return NULL;
	p->kind = expr_kind::UnaryOp_kind;
	p->v.UnaryOp.op = op;
	p->v.UnaryOp.operand = operand;
	p->lineno = lineno;
	p->col_offset = col_offset;
	p->end_lineno = end_lineno;
	p->end_col_offset = end_col_offset;
	return p;
}
