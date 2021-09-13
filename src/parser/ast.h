#ifndef __AST_H__
#define __AST_H__

#include "../port.h"
#include "../core/viarena.h"
#include "../objects/object.h"

typedef ViObject *identifier;
typedef ViObject *object;
typedef ViObject *string;
typedef ViObject *constant;

typedef struct _mod *mod_type;
typedef struct _stmt *stmt_type;
typedef struct _expr *expr_type;

typedef struct _type_ignore *type_ignore_ty;

/* Operator types */

typedef enum class _operator
{
	Add = 1,
	Sub,
	Mult,
	MatMult,
	Div,
	Mod,
	Pow,
	LShift,
	RShift,
	BitOr,
	BitXor,
	BitAnd,
	FloorDiv
} operator_type;

typedef enum class _unary_operator
{
	Invert = 1,
	Not,
	UAdd,
	USub
} unaryop_type;

/* AST node sequences */

#define AST_SEQ_HEAD \
	Vi_size_t size;     \
	void **elements;

typedef struct
{
	AST_SEQ_HEAD;
} ast_seq;

typedef struct
{
	AST_SEQ_HEAD;
	void *typed_elements[1];
} ast_generic_seq;

typedef struct
{
	AST_SEQ_HEAD;
	ViObject *typed_elements[1];
} ast_identifier_seq;

typedef struct
{
	AST_SEQ_HEAD;
	int typed_elements[1];
} ast_int_seq;

ast_generic_seq *ViAST_NewGenericSeq(Vi_size_t size);
ast_identifier_seq *ViAST_NewIdentifierSeq(Vi_size_t size);
ast_int_seq *ViAST_NewIntSeq(Vi_size_t size);

#define ViAST_SEQ_GET_UNTYPED(s, i) (s)->elements[(i)]
#define ViAST_SEQ_GET(s, i) (s)->typed_elements[(i)]
#define ViAST_SEQ_LENGTH(s) ((s) == NULL ? 0 : (s)->size)

#ifdef Vi_DEBUG
#define ViAST_SEQ_SET_UNTYPED(s, i, v) \
	do { \
		Vi_size_t ast_i = (i); \
		assert((s) != NULL); \
		assert(0 <= ast_i && ast_i < (s)->size); \
		(s)->elements[i] = (v); \
	} while(0)
#else
#define ViAST_SEQ_SET_UNTYPED(s, i, v) (s)->elements[i] = (v)
#endif

#ifdef Vi_DEBUG
#define ViAST_SEQ_SET(s, i, v) \
	do { \
		Vi_size_t ast_i = (i) \
		assert((s) != NULL); \
		assert(0 <= ast_i && ast_i < (s)->size) \
		(s)->typed_elements[i] = (v); \
	} while(0)
#else
#define ViAST_SEQ_SET(s, i, v) (s)->typed_elements[i] = (v)
#endif

typedef struct
{
	AST_SEQ_HEAD;
	mod_type typed_elements[1];
} ast_mod_seq;

typedef struct
{
	AST_SEQ_HEAD;
	stmt_type typed_elements[1];
} ast_stmt_seq;

typedef struct
{
	AST_SEQ_HEAD;
	expr_type typed_elements[1];
} ast_expr_seq;

typedef struct
{
	AST_SEQ_HEAD;
	type_ignore_ty typed_elements[1];
} ast_type_ignore_seq;

/* AST node types */

enum class module_kind
{
	Unknown_Kind = -1,
	Module_Kind,
	Interactive_Kind,
	Expression_Kind
};

struct _mod
{
	enum module_kind kind;
	union
	{
		struct
		{
			ast_stmt_seq *body;
			ast_type_ignore_seq *type_ignores;
		} Module;

		struct
		{
			ast_stmt_seq *body;
		} Interactive;

		struct
		{
			expr_type body;
		} Expression;

		struct
		{
			ast_expr_seq *argtypes;
			expr_type returns;
		} FunctionType;
	} v;
};

enum class stmt_kind
{
	FunctionDef_kind = 1, AsyncFunctionDef_kind = 2, ClassDef_kind = 3,
	Return_kind = 4, Delete_kind = 5, Assign_kind = 6,
	AugAssign_kind = 7, AnnAssign_kind = 8, For_kind = 9,
	AsyncFor_kind = 10, While_kind = 11, If_kind = 12, With_kind = 13,
	AsyncWith_kind = 14, Raise_kind = 15, Try_kind = 16,
	Assert_kind = 17, Import_kind = 18, ImportFrom_kind = 19,
	Global_kind = 20, Nonlocal_kind = 21, Expr_kind = 22, Pass_kind = 23,
	Break_kind = 24, Continue_kind = 25
};

struct _stmt
{
	enum stmt_kind kind;
	union
	{
		struct
		{
			expr_type target;
			expr_type expr;
		} Assign;

		struct
		{
			expr_type value;
		} Expr;
	} v;
	int lineno;
	int col_offset;
	int end_lineno;
	int end_col_offset;
};

enum class expr_kind
{
	BoolOp_kind = 1, NamedExpr_kind = 2, BinOp_kind = 3, UnaryOp_kind = 4,
	Lambda_kind = 5, IfExp_kind = 6, Dict_kind = 7, Set_kind = 8,
	ListComp_kind = 9, SetComp_kind = 10, DictComp_kind = 11,
	GeneratorExp_kind = 12, Await_kind = 13, Yield_kind = 14,
	YieldFrom_kind = 15, Compare_kind = 16, Call_kind = 17,
	FormattedValue_kind = 18, JoinedStr_kind = 19, Constant_kind = 20,
	Attribute_kind = 21, Subscript_kind = 22, Starred_kind = 23,
	Name_kind = 24, List_kind = 25, Tuple_kind = 26, Slice_kind = 27
};

struct _expr
{
	enum expr_kind kind;
	union
	{
		struct
		{
			expr_type left;
			operator_type op;
			expr_type right;
		} BinaryOp;

		struct
		{
			expr_type operand;
			unaryop_type op;
		} UnaryOp;

		struct
		{
			constant value;
			string kind;
		} Constant;

		struct
		{
			identifier id;
		} Name;
	} v;
	int lineno;
	int col_offset;
	int end_lineno;
	int end_col_offset;
};

enum _type_ignore_kind { TypeIgnore_kind = 1 };
struct _type_ignore
{
	enum _type_ignore_kind kind;
	union
	{
		struct
		{
			int lineno;
			string tag;
		} TypeIgnore;
	} v;
};

/* Create nodes */

mod_type ViAST_Interactive(ast_stmt_seq *body, ViArena *arena);

stmt_type ViAST_Expr(expr_type value, int lineno, int col_offset, int end_lineno, int end_col_offset, ViArena *arena);
stmt_type ViAST_Pass(int lineno, int col_offset, int end_lineno, int end_col_offset, ViArena *arena);
expr_type ViAST_Constant(constant value, string kind, int lineno, int col_offset, int end_lineno, int end_col_offset, ViArena *arena);
expr_type ViAST_BinOp(expr_type left, operator_type op, expr_type right, int lineno, int col_offset, int end_lineno, int end_col_offset, ViArena *arena);
expr_type ViAST_UnaryOp(unaryop_type op, expr_type operand, int lineno, int col_offset, int end_lineno, int end_col_offset, ViArena *arena);

#endif // __AST_H__