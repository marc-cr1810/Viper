#include "parser.h"

#include "../core/error.h"
#include "vigen.h"
#include "../objects/boolobject.h"

#define ViParser_PrintTest(func, check) D(fprintf(stderr, "%*c> " func "[%d-%d]: %s?\n", p->level, ' ', mark, p->mark, check))
#define ViParser_PrintSuccess(func, check) D(fprintf(stderr, "%*c+ " func "[%d-%d]: %s succeeded!\n", p->level, ' ', mark, p->mark, check))
#define ViParser_PrintFail(func, check) D(fprintf(stderr, "%*c- " func "[%d-%d]: %s failed!\n", p->level, ' ', mark, p->mark, check))

#define RULE_HEAD()			\
D(p->level++);				\
if (p->error_indicator)	\
{							\
	D(p->level--);			\
	return NULL;			\
}

static inline void *CHECK_CALL(Parser *p, void *result)
{
	if (result == NULL)
	{
		assert(ViError_Occurred());
		p->error_indicator = 1;
	}
	return result;
}

#define CHECK(type, result) ((type) CHECK_CALL(p, result))

#define EXTRA start_lineno, start_col_offset, end_lineno, end_col_offset, p->arena

static const int keywordListSize = KEYWORD_COUNT;
static KeywordToken keywords[][KEYWORDS_MAX] = {
	{ // 0
		{NULL, TOK_UNKNOWN}
	},
	{ // 1
		{NULL, TOK_UNKNOWN}
	},
	{ // 2
		{"if", TOK_IF},
		{"do", TOK_DO},
		{NULL, TOK_UNKNOWN}
	},
	{ // 3
		{"for", TOK_FOR},
		{NULL, TOK_UNKNOWN}},
	{ // 4
		{"else", TOK_ELSE},
		{"func", TOK_FUNC},
		{"Null", TOK_NULL},
		{"True", TOK_TRUE},
		{NULL, TOK_UNKNOWN}
	},
	{ // 5
		{"while", TOK_WHILE},
		{"class", TOK_CLASS},
		{"async", TOK_ASYNC},
		{"await", TOK_AWAIT},
		{"False", TOK_FALSE},
		{NULL, TOK_UNKNOWN}
	},
	{ // 6
		{NULL, TOK_UNKNOWN}
	},
	{ // 7
		{NULL, TOK_UNKNOWN}
	},
	{ // 8
		{NULL, TOK_UNKNOWN}
	},
	{ // 9
		{"extension", TOK_EXTENSION},
		{NULL, TOK_UNKNOWN}
	} };

/* Helper functions */

//
// RULE DEFINITIONS
//

/* Rule types */

#define type_file_mode 1000
#define type_interactive_mode 1001
#define type_eval_mode 1002
#define type_string_mode 1003
#define type_statement_newline 1004
#define type_simple_statements 1005
#define type_simple_statement 1006
#define type_assignment 1007
#define type_star_expressions 1008
#define type_star_expression 1009
#define type_expression 1010
#define type_disjunction 1011
#define type_conjunction 1012
#define type_inversion 1013
#define type_comparison 1014
#define type_bitwise_or 1015
#define type_bitwise_xor 1016
#define type_bitwise_and 1017
#define type_shift_expr 1018
#define type_sum 1019
#define type_term 1020
#define type_factor 1021
#define type_power 1022
#define type_await_primary 1023
#define type_primary 1024
#define type_atom 1025
#define type_strings 1026
#define type_loop_strings 1027

// Forward declarations

static mod_type rule_file_mode(Parser *p);
static mod_type rule_interactive_mode(Parser *p);
static mod_type rule_eval_mode(Parser *p);
static expr_type rule_string_mode(Parser *p);

static ast_stmt_seq *rule_statement_newline(Parser *p);
static ast_stmt_seq *rule_simple_statements(Parser *p);

static stmt_type rule_simple_statement(Parser *p);
static stmt_type rule_assignment(Parser *p);
static expr_type rule_star_expressions(Parser *p);
static expr_type rule_star_expression(Parser *p);
static expr_type rule_expression(Parser *p);
static expr_type rule_disjunction(Parser *p);
static expr_type rule_conjunction(Parser *p);
static expr_type rule_inversion(Parser *p);
static expr_type rule_comparison(Parser *p);
static expr_type rule_bitwise_or(Parser *p);
static expr_type rule_bitwise_xor(Parser *p);
static expr_type rule_bitwise_and(Parser *p);
static expr_type rule_shift_expr(Parser *p);
static expr_type rule_sum(Parser *p);
static expr_type rule_term(Parser *p);
static expr_type rule_factor(Parser *p);
static expr_type rule_power(Parser *p);
static expr_type rule_await_primary(Parser *p);
static expr_type rule_primary(Parser *p);
static expr_type rule_atom(Parser *p);
static expr_type rule_strings(Parser *p);
static ast_seq *rule_loop_string(Parser *p);

// Definitions

// file: statements? $
static mod_type rule_file_mode(Parser *p)
{
	ViError_SetString(ViExc_SystemError, "parser rule not implemented");
	return NULL;
}

// interactive: statement_newline
static mod_type rule_interactive_mode(Parser *p)
{
	RULE_HEAD();

	mod_type result = NULL;
	int mark = p->mark;
	{ // statement_newline
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("interactive_mode", "statement_newline");
		ast_stmt_seq *stmt_sq;

		if (
			(stmt_sq = rule_statement_newline(p)) // statement_newline
			)
		{
			ViParser_PrintSuccess("interactive_mode", "statement_newline");
			result = ViAST_Interactive(stmt_sq, p->arena);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("interactive_mode", "statement_newline");
	}
	result = NULL;
done:
	D(p->level--);
	return result;
}

// eval: expressions NEWLINE* $
static mod_type rule_eval_mode(Parser *p)
{
	ViError_SetString(ViExc_SystemError, "parser rule not implemented");
	return NULL;
}

// string: star_expressions
static expr_type rule_string_mode(Parser *p)
{
	ViError_SetString(ViExc_SystemError, "parser rule not implemented");
	return NULL;
}

// statement_newline: compound_stmt NEWLINE | simple_stmts | NEWLINE | $
static ast_stmt_seq *rule_statement_newline(Parser *p)
{
	RULE_HEAD();

	ast_stmt_seq *result = NULL;
	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return NULL;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;

	{ // simple_statements
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("statement_newline", "simple_statements");
		ast_stmt_seq *simple_stmts_var;
		if (
			(simple_stmts_var = rule_simple_statements(p)) // simple_statements
			)
		{
			ViParser_PrintSuccess("statement_newline", "simple_statements");
			result = simple_stmts_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("statement_newline", "simple_statements");
	}
	{ // NEWLINE
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("statement_newline", "NEWLINE");
		Token *newline_var;
		if (
			(newline_var = ViGen_ExpectToken(p, TOK_NEWLINE)) // token = 'NEWLINE'
			)
		{
			ViParser_PrintSuccess("statement_newline", "NEWLINE");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = (ast_stmt_seq *)ViGen_SingletonSeq(p, CHECK(stmt_type, ViAST_Pass(EXTRA)));
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("statement_newline", "NEWLINE");
	}
	{ // $
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("statement_newline", "$");
		Token *endmarker_var;
		if (
			(endmarker_var = ViGen_ExpectToken(p, TOK_ENDMARKER)) // token = 'ENDMARKER'
			)
		{
			ViParser_PrintSuccess("statement_newline", "$");
			result = ViGen_InteractiveExit(p);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("statement_newline", "$");
	}
	result = NULL;
done:
	D(p->level--);
	return result;
}

// simple_statements: simple_statement NEWLINE
static ast_stmt_seq *rule_simple_statements(Parser *p)
{
	RULE_HEAD();

	ast_stmt_seq *result = NULL;
	int mark = p->mark;
	{ // simple_statement NEWLINE
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("simple_statements", "simple_statement NEWLINE");
		stmt_type stmt;
		Token *newline_var;
		if (
			(stmt = rule_simple_statement(p)) // simple_statement
			&&
			(newline_var = ViGen_ExpectToken(p, TOK_NEWLINE)) // token = 'NEWLINE'
			)
		{
			ViParser_PrintSuccess("simple_statements", "simple_statement NEWLINE");
			result = (ast_stmt_seq *)ViGen_SingletonSeq(p, stmt);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("simple_statements", "simple_statement NEWLINE");
	}
done:
	D(p->level--);
	return result;
}

// simple_statement:
//     | assignment
//     | star_expressions
//     | &'return' return_statement
//     | &('import' | 'from') import_statement
//     | &'raise' raise_statement
//     | 'pass'
//     | &'del' del_statement
//     | &'yield' yield_statement
//     | &'assert' assert_statement
//     | 'break'
//     | 'continue'
//     | &'global' global_statement
//     | &'nonlocal' nonlocal_statement
static stmt_type rule_simple_statement(Parser *p)
{
	RULE_HEAD();

	stmt_type result = NULL;
	if (ViGen_IsMemoized(p, type_simple_statement, &result))
	{
		D(p->level--);
		return result;
	}
	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return NULL;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // assignment
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("simple_statement", "assignment");
		stmt_type assignment_var;
		if (
			(assignment_var = rule_assignment(p)) // assignment
			)
		{
			ViParser_PrintSuccess("simple_statement", "assignment");
			result = assignment_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("simple_statement", "assignment");
	}
	{ // star_expressions
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("simple_statement", "star_expressions");
		expr_type expr;
		if (
			(expr = rule_star_expressions(p)) // star_expressions
			)
		{
			ViParser_PrintSuccess("simple_statement", "star_expressions");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_Expr(expr, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("simple_statement", "star_expressions");
	}
	result = NULL;
done:
	ViGen_Memo_Insert(p, mark, type_simple_statement, result);
	D(p->level--);
	return result;
}

// assignment:
//     | NAME ':' expression ['=' annotated_rhs]
//     | ('(' single_target ')' | single_subscript_attribute_target) ':' expression ['=' annotated_rhs]
//     | ((star_targets '='))+ (yield_expr | star_expressions) !'=' TYPE_COMMENT?
//     | single_target augassign ~ (yield_expr | star_expressions)
//     | invalid_assignment
static stmt_type rule_assignment(Parser *p)
{
	RULE_HEAD();

	stmt_type result = NULL;
	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return NULL;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;

	result = NULL;
done:
	D(p->level--);
	return result;
}

// star_expressions:
//     | star_expression ((',' star_expression))+ ','?
//     | star_expression ','
//     | star_expression
static expr_type rule_star_expressions(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return NULL;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // star_expression
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("star_expressions", "star_expression");
		expr_type star_expression_var;
		if (
			(star_expression_var = rule_star_expression(p)) // star_expression
			)
		{
			ViParser_PrintSuccess("star_expressions", "star_expression");
			result = star_expression_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("star_expressions", "star_expression");
	}
	result = NULL;
done:
	D(p->level--);
	return result;
}

// star_expression: '*' bitwise_or | expression
static expr_type rule_star_expression(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	if (ViGen_IsMemoized(p, type_star_expression, &result))
	{
		D(p->level--);
		return result;
	}

	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return result;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // expression
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("star_expression", "expression");
		expr_type expression_var;
		if (
			(expression_var = rule_expression(p)) // expression
			)
		{
			ViParser_PrintSuccess("star_expression", "expression");
			result = expression_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("star_expression", "expression");
	}
	result = NULL;
done:
	ViGen_Memo_Insert(p, mark, type_star_expression, result);
	D(p->level--);
	return result;
}

// expression: disjunction 'if' disjunction 'else' expression | disjunction | lambdef
static expr_type rule_expression(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	if (ViGen_IsMemoized(p, type_expression, &result))
	{
		D(p->level--);
		return result;
	}

	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return result;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // disjunction
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("expression", "disjunction");
		expr_type disjunction_var;
		if (
			(disjunction_var = rule_disjunction(p)) // disjunction
			)
		{
			ViParser_PrintSuccess("expression", "disjunction");
			result = disjunction_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("expression", "disjunction");
	}
	result = NULL;
done:
	ViGen_Memo_Insert(p, mark, type_expression, result);
	D(p->level--);
	return result;
}

// disjunction: conjunction (('or' conjunction))+ | conjunction
static expr_type rule_disjunction(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	if (ViGen_IsMemoized(p, type_disjunction, &result))
	{
		D(p->level--);
		return result;
	}

	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return result;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // conjunction
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("disjunction", "conjunction");
		expr_type conjunction_var;
		if (
			(conjunction_var = rule_conjunction(p)) // conjunction
			)
		{
			ViParser_PrintSuccess("disjunction", "conjunction");
			result = conjunction_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("disjunction", "conjunction");
	}
	result = NULL;
done:
	ViGen_Memo_Insert(p, mark, type_disjunction, result);
	D(p->level--);
	return result;
}

// conjunction: inversion (('and' inversion))+ | inversion
static expr_type rule_conjunction(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	if (ViGen_IsMemoized(p, type_conjunction, &result))
	{
		D(p->level--);
		return result;
	}

	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return result;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // inversion
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("conjunction", "inversion");
		expr_type inversion_var;
		if (
			(inversion_var = rule_inversion(p))
			)
		{
			ViParser_PrintSuccess("conjunction", "inversion");
			result = inversion_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("conjunction", "inversion");
	}
	result = NULL;
done:
	ViGen_Memo_Insert(p, mark, type_conjunction, result);
	D(p->level--);
	return result;
}

// inversion: 'not' inversion | comparison
static expr_type rule_inversion(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	if (ViGen_IsMemoized(p, type_inversion, &result))
	{
		D(p->level--);
		return result;
	}

	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return result;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // comparison
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("inversion", "comparison");
		expr_type comparison_var;
		if (
			(comparison_var = rule_comparison(p)) // comparison
			)
		{
			ViParser_PrintSuccess("inversion", "comparison");
			result = comparison_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("inversion", "comparison");
	}
	result = NULL;
done:
	ViGen_Memo_Insert(p, mark, type_inversion, result);
	D(p->level--);
	return result;
}

// comparison: bitwise_or compare_op_bitwise_or_pair+ | bitwise_or
static expr_type rule_comparison(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return result;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // bitwise_or
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("comparison", "bitwise_or");
		expr_type bitwise_or_var;
		if (
			(bitwise_or_var = rule_bitwise_or(p)) // bitwise_or
			)
		{
			ViParser_PrintSuccess("comparison", "bitwise_or");
			result = bitwise_or_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("comparison", "bitwise_or");
	}
	result = NULL;
done:
	D(p->level--);
	return result;
}

// Left-recursive
// bitwise_or: bitwise_or '|' bitwise_xor | bitwise_xor
static expr_type raw_bitwise_or(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return result;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // bitwise_or '|' bitwise_xor
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("bitwise_xor", "bitwise_or '|' bitwise_xor");
		Token *_literal;
		expr_type a;
		expr_type b;
		if (
			(a = rule_bitwise_or(p)) // bitwise_or
			&&
			(_literal = ViGen_ExpectToken(p, TOK_VBAR)) // token = '|'
			&&
			(b = rule_bitwise_xor(p)) // bitwise_xor
			)
		{
			ViParser_PrintSuccess("bitwise_xor", "bitwise_or '|' bitwise_xor");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_BinOp(a, operator_type::BitOr, b, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("bitwise_xor", "bitwise_or '|' bitwise_xor");
	}
	{ // bitwise_xor
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("bitwise_or", "bitwise_xor");
		expr_type bitwise_xor_var;
		if (
			(bitwise_xor_var = rule_bitwise_xor(p)) // bitwise_xor
			)
		{
			ViParser_PrintSuccess("bitwise_or", "bitwise_xor");
			result = bitwise_xor_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("bitwise_or", "bitwise_xor");
	}
	result = NULL;
done:
	D(p->level--);
	return result;
}
static expr_type rule_bitwise_or(Parser *p)
{
	D(p->level++);
	expr_type result = NULL;
	if (ViGen_IsMemoized(p, type_bitwise_or, &result))
	{
		D(p->level--);
		return result;
	}
	int mark = p->mark;
	int resmark = p->mark;
	while (1)
	{
		int tmpvar = ViGen_Memo_Update(p, mark, type_bitwise_or, result);
		if (tmpvar)
		{
			D(p->level--);
			return result;
		}
		p->mark = mark;
		void *raw = raw_bitwise_or(p);
		if (p->error_indicator)
			return NULL;
		if (raw == NULL || p->mark <= resmark)
			break;
		resmark = p->mark;
		result = (expr_type)raw;
	}
	p->mark = resmark;
	D(p->level--);
	return result;
}

// Left-recursive
// bitwise_xor: bitwise_xor '^' bitwise_and | bitwise_and
static expr_type raw_bitwise_xor(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return result;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // bitwise_xor '^' bitwise_and
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("bitwise_xor", "bitwise_xor '^' bitwise_and");
		Token *_literal;
		expr_type a;
		expr_type b;
		if (
			(a = rule_bitwise_xor(p)) // bitwise_xor
			&&
			(_literal = ViGen_ExpectToken(p, TOK_CIRCUMFLEX)) // token = '^'
			&&
			(b = rule_bitwise_and(p)) // bitwise_and
			)
		{
			ViParser_PrintSuccess("bitwise_xor", "bitwise_xor '^' bitwise_and");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_BinOp(a, operator_type::BitXor, b, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("bitwise_xor", "bitwise_xor '^' bitwise_and");
	}
	{ // bitwise_and
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("bitwise_xor", "bitwise_and");
		expr_type bitwise_and_var;
		if (
			(bitwise_and_var = rule_bitwise_and(p)) // bitwise_and
			)
		{
			ViParser_PrintSuccess("bitwise_xor", "bitwise_and");
			result = bitwise_and_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("bitwise_xor", "bitwise_and");
	}
	result = NULL;
done:
	D(p->level--);
	return result;
}
static expr_type rule_bitwise_xor(Parser *p)
{
	D(p->level++);
	expr_type result = NULL;
	if (ViGen_IsMemoized(p, type_bitwise_xor, &result))
	{
		D(p->level--);
		return result;
	}
	int mark = p->mark;
	int resmark = p->mark;
	while (1)
	{
		int tmpvar = ViGen_Memo_Update(p, mark, type_bitwise_xor, result);
		if (tmpvar)
		{
			D(p->level--);
			return result;
		}
		p->mark = mark;
		void *raw = raw_bitwise_xor(p);
		if (p->error_indicator)
			return NULL;
		if (raw == NULL || p->mark <= resmark)
			break;
		resmark = p->mark;
		result = (expr_type)raw;
	}
	p->mark = resmark;
	D(p->level--);
	return result;
}

// Left-recursive
// bitwise_and: bitwise_and '&' shift_expr | shift_expr
static expr_type raw_bitwise_and(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return result;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // bitwise_and '&' shift_expr
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("bitwise_and", "bitwise_and '&' shift_expr");
		Token *_literal;
		expr_type a;
		expr_type b;
		if (
			(a = rule_bitwise_and(p)) // bitwise_and
			&&
			(_literal = ViGen_ExpectToken(p, TOK_AMPER)) // token = '&'
			&&
			(b = rule_shift_expr(p)) // shift_expr
			)
		{
			ViParser_PrintSuccess("bitwise_and", "bitwise_and '&' shift_expr");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_BinOp(a, operator_type::BitAnd, b, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("bitwise_and", "bitwise_and '&' shift_expr");
	}
	{ // shift_expr
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("bitwise_and", "shift_expr");
		expr_type shift_expr_var;
		if (
			(shift_expr_var = rule_shift_expr(p)) // shift_expr
			)
		{
			ViParser_PrintSuccess("bitwise_and", "shift_expr");
			result = shift_expr_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("bitwise_and", "shift_expr");
	}
	result = NULL;
done:
	D(p->level--);
	return result;
}
static expr_type rule_bitwise_and(Parser *p)
{
	D(p->level++);
	expr_type result = NULL;
	if (ViGen_IsMemoized(p, type_bitwise_and, &result))
	{
		D(p->level--);
		return result;
	}
	int mark = p->mark;
	int resmark = p->mark;
	while (1)
	{
		int tmpvar = ViGen_Memo_Update(p, mark, type_bitwise_and, result);
		if (tmpvar)
		{
			D(p->level--);
			return result;
		}
		p->mark = mark;
		void *raw = raw_bitwise_and(p);
		if (p->error_indicator)
			return NULL;
		if (raw == NULL || p->mark <= resmark)
			break;
		resmark = p->mark;
		result = (expr_type)raw;
	}
	p->mark = resmark;
	D(p->level--);
	return result;
}

// Left-recursive
// shift_expr: shift_expr '<<' sum | shift_expr '>>' sum | sum
static expr_type raw_shift_expr(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return result;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // shift_expr '<<' sum
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("shift_expr", "shift_expr '<<' sum");
		Token *_literal;
		expr_type a;
		expr_type b;
		if (
			(a = rule_shift_expr(p)) // shift_expr
			&&
			(_literal = ViGen_ExpectToken(p, TOK_LEFTSHIFT)) // token = '<<'
			&&
			(b = rule_sum(p)) // sum
			)
		{
			ViParser_PrintSuccess("shift_expr", "shift_expr '<<' sum");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_BinOp(a, operator_type::LShift, b, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("shift_expr", "shift_expr '<<' sum");
	}
	{ // shift_expr '>>' sum
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("shift_expr", "shift_expr '>>' sum");
		Token *_literal;
		expr_type a;
		expr_type b;
		if (
			(a = rule_shift_expr(p)) // shift_expr
			&&
			(_literal = ViGen_ExpectToken(p, TOK_RIGHTSHIFT)) // token = '>>'
			&&
			(b = rule_sum(p)) // sum
			)
		{
			ViParser_PrintSuccess("shift_expr", "shift_expr '>>' sum");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_BinOp(a, operator_type::RShift, b, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("shift_expr", "shift_expr '>>' sum");
	}
	{ // sum
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("shift_expr", "sum");
		expr_type sum_var;
		if (
			(sum_var = rule_sum(p)) // sum
			)
		{
			ViParser_PrintSuccess("shift_expr", "sum");
			result = sum_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("shift_expr", "sum");
	}
	result = NULL;
done:
	D(p->level--);
	return result;
}
static expr_type rule_shift_expr(Parser *p)
{
	D(p->level++);
	expr_type result = NULL;
	if (ViGen_IsMemoized(p, type_shift_expr, &result))
	{
		D(p->level--);
		return result;
	}
	int mark = p->mark;
	int resmark = p->mark;
	while (1)
	{
		int tmpvar = ViGen_Memo_Update(p, mark, type_shift_expr, result);
		if (tmpvar)
		{
			D(p->level--);
			return result;
		}
		p->mark = mark;
		void *raw = raw_shift_expr(p);
		if (p->error_indicator)
			return NULL;
		if (raw == NULL || p->mark <= resmark)
			break;
		resmark = p->mark;
		result = (expr_type)raw;
	}
	p->mark = resmark;
	D(p->level--);
	return result;
}

// Left-recursive
// sum: sum '+' term | sum '-' term | term
static expr_type raw_sum(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return result;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // sum '+' term 
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("sum", "sum '+' term");
		Token *_literal;
		expr_type a;
		expr_type b;
		if (
			(a = rule_sum(p)) // sum
			&&
			(_literal = ViGen_ExpectToken(p, TOK_ADD)) // token = '+'
			&&
			(b = rule_term(p)) // term
			)
		{
			ViParser_PrintSuccess("sum", "sum '+' term");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_BinOp(a, operator_type::Add, b, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("sum", "sum '+' term");
	}
	{ // sum '-' term 
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("sum", "sum '-' term");
		Token *_literal;
		expr_type a;
		expr_type b;
		if (
			(a = rule_sum(p)) // sum
			&&
			(_literal = ViGen_ExpectToken(p, TOK_MINUS)) // token = '-'
			&&
			(b = rule_term(p)) // term
			)
		{
			ViParser_PrintSuccess("sum", "sum '-' term");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_BinOp(a, operator_type::Sub, b, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("sum", "sum '-' term");
	}
	{ // term
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("sum", "term");
		expr_type term_var;
		if (
			(term_var = rule_term(p)) // term
			)
		{
			ViParser_PrintSuccess("sum", "term");
			result = term_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("sum", "term");
	}
	result = NULL;
done:
	D(p->level--);
	return result;
}
static expr_type rule_sum(Parser *p)
{
	D(p->level++);
	expr_type result = NULL;
	if (ViGen_IsMemoized(p, type_sum, &result))
	{
		D(p->level--);
		return result;
	}
	int mark = p->mark;
	int resmark = p->mark;
	while (1)
	{
		int tmpvar = ViGen_Memo_Update(p, mark, type_sum, result);
		if (tmpvar)
		{
			D(p->level--);
			return result;
		}
		p->mark = mark;
		void *raw = raw_sum(p);
		if (p->error_indicator)
			return NULL;
		if (raw == NULL || p->mark <= resmark)
			break;
		resmark = p->mark;
		result = (expr_type)raw;
	}
	p->mark = resmark;
	D(p->level--);
	return result;
}
// Left-recursive
// term:
//     | term '*' factor
//     | term '/' factor
//     | term '//' factor
//     | term '%' factor
//     | term '@' factor
//     | factor
static expr_type raw_term(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return result;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // term '*' factor 
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("term", "term '*' factor");
		Token *_literal;
		expr_type a;
		expr_type b;
		if (
			(a = rule_term(p)) // term
			&&
			(_literal = ViGen_ExpectToken(p, TOK_STAR)) // token = '*'
			&&
			(b = rule_factor(p)) // factor
			)
		{
			ViParser_PrintSuccess("term", "term '*' factor");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_BinOp(a, operator_type::Mult, b, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("term", "term '*' factor");
	}
	{ // term '/' factor 
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("term", "term '/' factor");
		Token *_literal;
		expr_type a;
		expr_type b;
		if (
			(a = rule_term(p)) // term
			&&
			(_literal = ViGen_ExpectToken(p, TOK_FSLASH)) // token = '/'
			&&
			(b = rule_factor(p)) // factor
			)
		{
			ViParser_PrintSuccess("term", "term '/' factor");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_BinOp(a, operator_type::Div, b, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("term", "term '/' factor");
	}
	{ // term '//' factor 
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("term", "term '//' factor");
		Token *_literal;
		expr_type a;
		expr_type b;
		if (
			(a = rule_term(p)) // term
			&&
			(_literal = ViGen_ExpectToken(p, TOK_DOUBLEFSLASH)) // token = '//'
			&&
			(b = rule_factor(p)) // factor
			)
		{
			ViParser_PrintSuccess("term", "term '//' factor");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_BinOp(a, operator_type::FloorDiv, b, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("term", "term '//' factor");
	}
	{ // term '%' factor 
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("term", "term '%' factor");
		Token *_literal;
		expr_type a;
		expr_type b;
		if (
			(a = rule_term(p)) // term
			&&
			(_literal = ViGen_ExpectToken(p, TOK_PERCENT)) // token = '%'
			&&
			(b = rule_factor(p)) // factor
			)
		{
			ViParser_PrintSuccess("term", "term '%' factor");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_BinOp(a, operator_type::Mod, b, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("term", "term '%' factor");
	}
	{ // term '@' factor 
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("term", "term '@' factor");
		Token *_literal;
		expr_type a;
		expr_type b;
		if (
			(a = rule_term(p)) // term
			&&
			(_literal = ViGen_ExpectToken(p, TOK_AT)) // token = '@'
			&&
			(b = rule_factor(p)) // factor
			)
		{
			ViParser_PrintSuccess("term", "term '@' factor");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_BinOp(a, operator_type::MatMult, b, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("term", "term '@' factor");
	}
	{ // factor
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("term", "term");
		expr_type factor_var;
		if (
			(factor_var = rule_factor(p)) // factor
			)
		{
			ViParser_PrintSuccess("term", "factor");
			result = factor_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("term", "factor");
	}
	result = NULL;
done:
	D(p->level--);
	return result;
}
static expr_type rule_term(Parser *p)
{
	D(p->level++);
	expr_type result = NULL;
	if (ViGen_IsMemoized(p, type_term, &result))
	{
		D(p->level--);
		return result;
	}
	int mark = p->mark;
	int resmark = p->mark;
	while (1)
	{
		int tmpvar = ViGen_Memo_Update(p, mark, type_term, result);
		if (tmpvar)
		{
			D(p->level--);
			return result;
		}
		p->mark = mark;
		void *raw = raw_term(p);
		if (p->error_indicator)
			return NULL;
		if (raw == NULL || p->mark <= resmark)
			break;
		resmark = p->mark;
		result = (expr_type)raw;
	}
	p->mark = resmark;
	D(p->level--);
	return result;
}

// factor: '+' factor | '-' factor | '~' factor | power
expr_type rule_factor(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	if (ViGen_IsMemoized(p, type_factor, &result))
	{
		D(p->level--);
		return result;
	}

	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return result;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // '+' factor
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("factor", "'+' factor");
		Token *_literal;
		expr_type a;
		if (
			(_literal = ViGen_ExpectToken(p, TOK_ADD)) // token = '+'
			&&
			(a = rule_factor(p)) // factor
			)
		{
			ViParser_PrintSuccess("factor", "'+' factor");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_UnaryOp(unaryop_type::UAdd, a, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("factor", "'+' factor");
	}
	{ // '-' factor
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("factor", "'-' factor");
		Token *_literal;
		expr_type a;
		if (
			(_literal = ViGen_ExpectToken(p, TOK_MINUS)) // token = '-'
			&&
			(a = rule_factor(p)) // factor
			)
		{
			ViParser_PrintSuccess("factor", "'-' factor");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_UnaryOp(unaryop_type::USub, a, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("factor", "'-' factor");
	}
	{ // '~' factor
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("factor", "'~' factor");
		Token *_literal;
		expr_type a;
		if (
			(_literal = ViGen_ExpectToken(p, TOK_TILDE)) // token = '~'
			&&
			(a = rule_factor(p)) // factor
			)
		{
			ViParser_PrintSuccess("factor", "'~' factor");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_UnaryOp(unaryop_type::Invert, a, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("factor", "'~' factor");
	}
	{ // power
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("factor", "power");
		expr_type power_var;
		if (
			(power_var = rule_power(p)) // power
			)
		{
			ViParser_PrintSuccess("factor", "power");
			result = power_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("factor", "power");
	}
	result = NULL;
done:
	ViGen_Memo_Insert(p, mark, type_factor, result);
	D(p->level--);
	return result;
}

// power: await_primary '**' factor | await_primary
static expr_type rule_power(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return result;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // await_primary '**' factor 
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("power", "await_primary '**' factor");
		Token *_literal;
		expr_type a;
		expr_type b;
		if (
			(a = rule_await_primary(p)) // await_primary
			&&
			(_literal = ViGen_ExpectToken(p, TOK_DOUBLESTAR)) // token = '**'
			&&
			(b = rule_factor(p)) // factor
			)
		{
			ViParser_PrintSuccess("power", "await_primary '**' factor");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_BinOp(a, operator_type::Pow, b, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("power", "await_primary '**' factor");
	}
	{ // await_primary
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("power", "await_primary");
		expr_type await_primary_var;
		if (
			(await_primary_var = rule_await_primary(p)) // await_primary
			)
		{
			ViParser_PrintSuccess("power", "await_primary");
			result = await_primary_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("power", "await_primary");
	}
	result = NULL;
done:
	D(p->level--);
	return result;
}

// await_primary: AWAIT primary | primary
static expr_type rule_await_primary(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	if (ViGen_IsMemoized(p, type_await_primary, &result))
	{
		D(p->level--);
		return result;
	}

	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return result;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // primary
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("await_primary", "primary");
		expr_type primary_var;
		if (
			(primary_var = rule_primary(p)) // primary
			)
		{
			ViParser_PrintSuccess("await_primary", "primary");
			result = primary_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("await_primary", "primary");
	}
	result = NULL;
done:
	ViGen_Memo_Insert(p, mark, type_await_primary, result);
	D(p->level--);
	return result;
}

// Left-recursive
// primary:
//     | invalid_primary
//     | primary '.' NAME
//     | primary genexp
//     | primary '(' arguments? ')'
//     | primary '[' slices ']'
//     | atom
static expr_type raw_primary(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return result;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // atom
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("primary", "atom");
		expr_type atom_var;
		if (
			(atom_var = rule_atom(p)) // atom
			)
		{
			ViParser_PrintSuccess("primary", "atom");
			result = atom_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("primary", "atom");
	}
	result = NULL;
done:
	D(p->level--);
	return result;
}
static expr_type rule_primary(Parser *p)
{
	D(p->level++);
	expr_type result = NULL;
	if (ViGen_IsMemoized(p, type_primary, &result))
	{
		D(p->level--);
		return result;
	}
	int mark = p->mark;
	int resmark = p->mark;
	while (1)
	{
		int tmpvar = ViGen_Memo_Update(p, mark, type_primary, result);
		if (tmpvar)
		{
			D(p->level--);
			return result;
		}
		p->mark = mark;
		void *raw = raw_primary(p);
		if (p->error_indicator)
			return NULL;
		if (raw == NULL || p->mark <= resmark)
			break;
		resmark = p->mark;
		result = (expr_type)raw;
	}
	p->mark = resmark;
	D(p->level--);
	return result;
}

// atom:
//     | NAME
//     | 'True'
//     | 'False'
//     | 'Null'
//     | &STRING strings
//     | NUMBER
//     | &'(' (tuple | group | genexp)
//     | &'[' (list | listcomp)
//     | &'{' (dict | set | dictcomp | setcomp)
//     | '...'
static expr_type rule_atom(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	int mark = p->mark;
	if (p->mark == p->fill && ViGen_FillToken(p) < 0)
	{
		p->error_indicator = 1;
		D(p->level--);
		return result;
	}

	int start_lineno = p->tokens[mark]->lineno;
	int start_col_offset = p->tokens[mark]->col_offset;
	{ // 'True'
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("atom", "'True'");
		Token *keyword;
		if (
			(keyword = ViGen_ExpectToken(p, TOK_TRUE)) // 'True'
			)
		{
			ViParser_PrintSuccess("atom", "'True'");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_Constant((constant)Vi_True, NULL, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("atom", "'True'");
	}
	{ // 'False'
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("atom", "'False'");
		Token *keyword;
		if (
			(keyword = ViGen_ExpectToken(p, TOK_FALSE)) // 'False'
			)
		{
			ViParser_PrintSuccess("atom", "'False'");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_Constant((constant)Vi_False, NULL, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("atom", "'False'");
	}
	{ // 'Null'
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("atom", "'Null'");
		Token *keyword;
		if (
			(keyword = ViGen_ExpectToken(p, TOK_NULL)) // 'Null'
			)
		{
			ViParser_PrintSuccess("atom", "'Null'");
			Token *token = ViGen_GetLastNonWhitespaceToken(p);
			if (token == NULL)
			{
				D(p->level--);
				return NULL;
			}
			int end_lineno = token->end_lineno;
			int end_col_offset = token->end_col_offset;
			result = ViAST_Constant((constant)Vi_Null, NULL, EXTRA);
			if (result == NULL && ViError_Occurred())
			{
				p->error_indicator = 1;
				D(p->level--);
				return NULL;
			}
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("atom", "'Null'");
	}
	{ // &STRING strings
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("atom", "&STRING strings");
		expr_type strings_var;
		if (
			ViGen_Lookahead(1, ViGen_StringToken, p)
			&&
			(strings_var = rule_strings(p)) // strings
			)
		{
			ViParser_PrintSuccess("atom", "&STRING strings");
			result = strings_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("atom", "&STRING strings");
	}
	{ // NUMBER
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("atom", "NUMBER");
		expr_type number_var;
		if (
			(number_var = ViGen_NumberToken(p)) // NUMBER
			)
		{
			ViParser_PrintSuccess("atom", "NUMBER");
			result = number_var;
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("atom", "NUMBER");
	}
	result = NULL;
done:
	D(p->level--);
	return result;
}

// strings: STRING+
expr_type rule_strings(Parser *p)
{
	RULE_HEAD();

	expr_type result = NULL;
	if (ViGen_IsMemoized(p, type_strings, &result))
	{
		D(p->level--);
		return result;
	}

	int mark = p->mark;
	{ // STRING+
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("strings", "STRING+");
		ast_seq *a;
		if (
			(a = rule_loop_string(p)) // STRING+
			)
		{
			ViParser_PrintSuccess("strings", "STRING+");
			goto done;
		}
		p->mark = mark;
		ViParser_PrintFail("strings", "STRING+");
	}
	result = NULL;
done:
	ViGen_Memo_Insert(p, mark, type_strings, result);
	D(p->level--);
	return result;
}

// loop_string: STRING
ast_seq *rule_loop_string(Parser *p)
{
	RULE_HEAD();

	void *result = NULL;
	int mark = p->mark;
	int start_mark = p->mark;
	void **children = (void **)Mem_Alloc(sizeof(void *));
	if (!children)
	{
		p->error_indicator = 1;
		ViError_NoMemory();
		D(p->level--);
		return NULL;
	}
	size_t children_capacity = 1;
	size_t n = 0;
	{ // STRING
		if (p->error_indicator)
		{
			D(p->level--);
			return NULL;
		}
		ViParser_PrintTest("loop_string", "STRING");
		expr_type string_var;
		while (
			(string_var = (expr_type)ViGen_StringToken(p)) // STRING
			)
		{
			result = string_var;
			if (n == children_capacity)
			{
				children_capacity *= 2;
				void **new_children = (void **)Mem_Realloc(children, children_capacity * sizeof(void *));
				if (!new_children)
				{
					p->error_indicator = 1;
					ViError_NoMemory();
					D(p->level--);
					return NULL;
				}
				children = new_children;
			}
			children[n++] = result;
			mark = p->mark;
		}
		p->mark = mark;
		ViParser_PrintFail("loop_string", "STRING");
	}
	if (n == 0 || p->error_indicator)
	{
		Mem_Free(children);
		D(p->level--);
		return NULL;
	}
	ast_seq *seq = (ast_seq *)ViAST_NewGenericSeq(n);
	if (!seq)
	{
		Mem_Free(children);
		p->error_indicator = 1;
		ViError_NoMemory();
		D(p->level--);
		return NULL;
	}
	for (int i = 0; i < n; i++)
		ViAST_SEQ_SET_UNTYPED(seq, i, children[i]);
	Mem_Free(children);
	ViGen_Memo_Insert(p, start_mark, type_loop_strings, seq);
	D(p->level--);
	return seq;
}

static int newline_in_string(Parser *p, const char *cur)
{
	for (const char *c = cur; c >= p->tok->buf; c--)
	{
		if (*c == '\'' || *c == '"')
		{
			return 1;
		}
	}
	return 0;
}

/* Check that the source for a single input statement really is a single
   statement by looking at what is left in the buffer after parsing.
   Trailing whitespace and comments are OK. */
static int bad_single_statement(Parser *p)
{
	const char *cur = strchr(p->tok->buf, '\n');

	/* Newlines are allowed if preceded by a line continuation character
	   or if they appear inside a string. */
	if (!cur || (cur != p->tok->buf && *(cur - 1) == '\\') || newline_in_string(p, cur))
	{
		return 0;
	}
	char c = *cur;

	for (;;)
	{
		while (c == ' ' || c == '\t' || c == '\n' || c == '\014')
		{
			c = *++cur;
		}

		if (!c)
		{
			return 0;
		}

		if (c != '#')
		{
			return 1;
		}

		// Suck up comment.
		while (c && c != '\n')
		{
			c = *++cur;
		}
	}
}

static void parser_reset_state(Parser *p)
{
	for (int i = 0; i < p->fill; i++)
		p->tokens[i]->memo = NULL;
	p->mark = 0;
}

static void *parser_parse(Parser *p)
{
	void *result = NULL;
	switch (p->mode)
	{
		case PARSER_MODE_FILE_INPUT:
			result = rule_file_mode(p);
			break;
		case PARSER_MODE_SINGLE_INPUT:
			result = rule_interactive_mode(p);
			break;
		case PARSER_MODE_EVAL_INPUT:
			result = rule_eval_mode(p);
			break;
		case PARSER_MODE_STRING_INPUT:
			result = rule_string_mode(p);
			break;
	}
	return result;
}

// API function definitions

Parser *ViParser_New(TokState *tok, int mode, int *error_code, ViArena *arena)
{
	Parser *p = (Parser *)Mem_Alloc(sizeof(Parser));
	if (p == NULL)
	{
		ViError_SetString(ViExc_SystemError, "out of memory");
		return NULL;
	}

	assert(tok != NULL);
	p->mode = mode;
	p->error_indicator = 0;
	p->error_code = error_code;
	p->parsing_started = 0;
	p->flags = 0;
	p->starting_lineno = 0;
	p->starting_col_offset = 0;

	p->arena = arena;

	p->tok = tok;
	p->tokens = (Token **)Mem_Alloc(sizeof(Token *));
	if (!p->tokens)
	{
		Mem_Free(p->tokens);
		ViError_SetString(ViExc_MemoryError, "out of memory");
		return NULL;
	}
	p->tokens[0] = (Token *)Mem_Calloc(1, sizeof(Token));
	if (!p->tokens)
	{
		Mem_Free(p->tokens[0]);
		Mem_Free(p->tokens);
		ViError_SetString(ViExc_MemoryError, "out of memory");
		return NULL;
	}
	p->size = 1;
	p->fill = 0;

	p->keywordListSize = keywordListSize;
	p->keywords = keywords;

	p->mark = 0;
	p->level = 0;

	return p;
}

void *ViParser_Parse(Parser *p)
{
	void *result = parser_parse(p);
	if (result == NULL)
	{
		parser_reset_state(p);
		parser_parse(p);
		if (ViError_Occurred())
			return NULL;

		if (p->fill == 0)
			ViError_SetString(ViExc_SyntaxError, "error at start before reading any input");
		else if (p->tok->done == E_EOF)
			ViError_SetString(ViExc_SyntaxError, "unexpected EOF while parsing");
		else
		{
			if (p->tokens[p->fill - 1]->type == TOK_INDENT)
				ViError_SetString(ViExc_IndentationError, "unexpected indent");
			else if (p->tokens[p->fill - 1]->type == TOK_DEDENT)
				ViError_SetString(ViExc_IndentationError, "unexpected unindent");
			else
				ViError_SetString(ViExc_SyntaxError, "invalid syntax");
		}
		return NULL;
	}
	if (p->mode == PARSER_MODE_SINGLE_INPUT && bad_single_statement(p))
	{
		p->tok->done = E_BADSINGLE;
		ViError_SetString(ViExc_SyntaxError, "multiple statements found while compiling a single statement");
		return NULL;
	}
	return result;
}

void ViParser_Free(Parser *p)
{
	for (int i = 0; i < p->size; i++)
	{
		Mem_Free(p->tokens[i]);
	}
	Mem_Free(p->tokens);
	Mem_Free(p);
}
