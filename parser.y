%{
#ifdef NGDEBUG
#include <assert.h>
#endif

#include <stdio.h>
#include <string.h>
#include "global.h"
#include "lexer.h"
#include "env.h"
#include "syntree.h"
#include "stab.h"

%}

%output "parser.c"
%defines "parser.h"

%initial-action {
	global_env = curr_env = env_new(NULL);
}

%token COMMENT
%token STRING
%token CHARACTER
%token SEMI
%token COLON
%token DOT
%token COMMA
%token UL
%token SHAPE
%token STROKE
%token QUEST
%token SQUOTE
%token DQUOTE
%token LBRACE
%token RBRACE
%token LSBRAC
%token RSBRAC
%token LPAREN
%token RPAREN
%token INC
%token DEC
%token PINC
%token PDEC
%token PLUS
%token MINUS
%token UPLUS
%token UMINUS
%token MULTIPLY
%token DIVIDE
%token MOD
%token LAND
%token LOR
%token LNOT
%token AND
%token OR
%token XOR
%token NOT
%token SHL
%token SHR
%token EQ
%token NE
%token LE
%token GE
%token LT
%token GT
%token ASSIGN
%token PLUSASN
%token MINUSASN
%token MULASN
%token DIVASN
%token MODASN
%token SHLASN
%token SHRASN
%token ANDASN
%token XORASN
%token ORASN
%token MEMBER
%token SYM
%token FLOATPNT
%token INTEGER
%token TYPE
%token SIGNED
%token UNSIGNED
%token SHORT
%token LONG
%token STRUCT
%token IF
%token ELSE
%token WHILE
%token DO
%token FOR
%token SWITCH
%token CASE
%token GOTO
%token RETURN

%left COMMA
%right ASSIGN PLUSASN MINUSASN MULASN DIVASN MODASN SHLASN SHRASN ANDASN XORASN ORASN
%right IFF
%left LOR
%left LAND
%left OR
%left XOR
%left AND
%left EQ NE
%left LT LE GT GE
%left SHL SHR
%left PLUS MINUS
%left MULTIPLY DIVIDE MOD
%right PINC PDEC UPLUS UMINUS LNOT NOT PTR REFR
%left INC DEC LSBRAC DOT MEMBER

%nonassoc NOELSE
%nonassoc ELSE

%%

program		: code				{
							fmsg = fopen("msg.out", "w");
							fprintf(fmsg, "node id(env id):\tDescription\tChildren\n");
							syntree_translate($1);
							fclose(fmsg);
#ifdef NGDEBUG
							assert(sym_top == 0);
							assert(type_top == 0);
#endif
						}
		/* TODO: error handling */
		/* TODO: memory free */
		;

code		: code global			{ $$ = syntree_insert_last($1, $2); }
		| global			{ $$ = $1; }
		;

global		: var_def			{ $$ = $1; }
		| func_def			{ $$ = $1; }
		| struct_def			{ $$ = $1; }
		;

env_enter	:				{ curr_env = env_new(curr_env); }
		;

env_leave	:				{ curr_env = curr_env->prev; }
		;

sym_insert	: SYM				{
							struct Stab *symbol = env_insert(curr_env, lastsym, yyget_lineno());
							STACK_PUSH(sym_stack, sym_top, symbol);
						}

sym_lookup	: SYM				{
							struct Stab *symbol = env_lookup(curr_env, lastsym);
							STACK_PUSH(sym_stack, sym_top, symbol);
						}

s_sym_lookup	: SYM				{
							struct Stab *symbol = env_lookup(curr_env, prevsym);
							STACK_PUSH(sym_stack, sym_top, symbol);
						}

type		: TYPE
		| STRUCT s_sym_lookup		{
							struct Type_info *ti = type_new(T_STRUCT, STACK_POP(sym_stack, sym_top));
							STACK_PUSH(type_stack, type_top, ti);
						}
		;

pointer		: MULTIPLY sym_insert		{
							struct Stab *symbol = STACK_TOP(sym_stack, sym_top);
							symbol->type = STACK_TOP(type_stack, type_top);
							symbol->ptrcount = 1;
						}
		| MULTIPLY pointer		{
							struct Stab *symbol = STACK_TOP(sym_stack, sym_top);
							symbol->ptrcount++;
						}
		;

id_noary	: sym_insert			{
							struct Stab *symbol = STACK_TOP(sym_stack, sym_top);
							symbol->type = STACK_TOP(type_stack, type_top);
						}
		| pointer
		;

array		: array LSBRAC INTEGER RSBRAC	{
							struct Stab *symbol = STACK_TOP(sym_stack, sym_top);
							struct Arysize_entry *ae = arysize_new(lastval);
							symbol->arysize_cnt++;
							LIST_INSERT(symbol->arysize_list, ae);
						}
		| id_noary LSBRAC INTEGER RSBRAC{
							struct Stab *symbol = STACK_TOP(sym_stack, sym_top);
							symbol->arysize_cnt = 1;
							symbol->arysize_list = arysize_new(lastval);
						}

identifier	: id_noary
		| array
		;

idlist		: idlist COMMA identifier	{
							struct Type_info *ti = STACK_TOP(type_stack, type_top);
							struct Syntree_node *t = syntree_new_node(0, K_DEF, ti->kind);
							t->symbol = STACK_POP(sym_stack, sym_top);
							$$ = syntree_insert_last($1, t);
						}
		| identifier			{
							struct Type_info *ti = STACK_TOP(type_stack, type_top);
							$$ = syntree_new_node(0, K_DEF, ti->kind);
							$$->symbol = STACK_POP(sym_stack, sym_top);
						}
		;

var_def		: type idlist SEMI		{
	 	/* does not support define with initialization */
							$$ = $2;
							STACK_POP(type_stack, type_top);
						}
		;

var_defs	: var_defs var_def		{ $$ = syntree_insert_last($1, $2); }
		| var_def			{ $$ = $1; }
		;

var_def_list	: var_defs			{ $$ = $1; }
		|				{ $$ = NULL; }
		;

func_def	: type id_noary env_enter LPAREN param_list RPAREN block env_leave	{
							$$ = syntree_new_node(1, K_FUNC, T_FUNC);
							$$->symbol = STACK_POP(sym_stack, sym_top);
							$$->symbol->isfunc = TRUE;
							$$->symbol->type = STACK_POP(type_stack, type_top);	
							$$->child[0] = $7;
						}
		;

struct_def	: STRUCT sym_insert env_enter LBRACE var_def_list RBRACE env_leave SEMI	{
	  	/* does not support defining struct right after struct def or without any var definition */
							$$ = syntree_new_node(1, K_STRUCT, T_STRUCT);
							$$->symbol = STACK_POP(sym_stack, sym_top);
							$$->symbol->type = type_new(T_STRUCT, $$->symbol);
							if ($5) {
								$$->symbol->member_cnt = $5->env->symbol_cnt;
								$$->symbol->member_env = $5->env;
							}
							$$->child[0] = $5;
						}
		;

param_list	: params
		|
		;

params		: params COMMA param		{
							struct Param_entry *pe = param_new(STACK_POP(sym_stack, sym_top));
							struct Stab *symbol = STACK_TOP(sym_stack, sym_top);
							symbol->param_cnt++;
							LIST_INSERT(symbol->param_list, pe);
						}
		| param				{
							struct Param_entry *pe = param_new(STACK_POP(sym_stack, sym_top));
							struct Stab *symbol = STACK_TOP(sym_stack, sym_top);
							symbol->param_cnt = 1;
							symbol->param_list = pe;
						}
		;

param		: type identifier		{
							STACK_POP(type_stack, type_top);
						}
		;

sentence	: var_def			{ $$ = $1; }
		| struct_def			{ $$ = $1; }
		| stmt				{ $$ = $1; }
		| expr SEMI			{ $$ = $1; }
		| SEMI
		;

sentence_list	: sentence_list sentence	{
							if ($1 == NULL) {
								$$ = $2;
							} else {
								syntree_insert_last($1, $2);
							}
						}
		|				{ $$ = NULL; }
		;

block		: LBRACE sentence_list RBRACE	{ $$ = $2; }
		;

stmt		: IF LPAREN expr RPAREN sentence ELSE sentence %prec ELSE	{
							$$ = syntree_new_node(3, K_STMT, T_VOID);
							$$->child[0] = $3;
							$$->child[1] = $5;
							$$->child[2] = $7;
							$$->stmt = K_IFELSE;
						}
		| IF LPAREN expr RPAREN sentence %prec NOELSE	{
							$$ = syntree_new_node(2, K_STMT, T_VOID);
							$$->child[0] = $3;
							$$->child[1] = $5;
							$$->stmt = K_IF;
						}

		| WHILE LPAREN expr RPAREN sentence	{
							$$ = syntree_new_node(2, K_STMT, T_VOID);
							$$->child[0] = $3;
							$$->child[1] = $5;
							$$->stmt = K_WHILE;
						}
		| DO env_enter block env_leave WHILE LPAREN expr RPAREN SEMI	{
							$$ = syntree_new_node(2, K_STMT, T_VOID);
							$$->child[0] = $3;
							$$->child[1] = $7;
							$$->stmt = K_DO;
						}
		| FOR LPAREN exprz SEMI exprz SEMI exprz RPAREN sentence	{
							$$ = syntree_new_node(4, K_STMT, T_VOID);
							$$->child[0] = $3;
							$$->child[1] = $5;
							$$->child[2] = $7;
							$$->child[3] = $9;
							$$->stmt = K_FOR;
						}
		| RETURN expr SEMI		{
							$$ = syntree_new_node(1, K_STMT, T_VOID);
							$$->child[0] = $2;
							$$->stmt = K_RET;
							$$->symbol = STACK_TOP(sym_stack, sym_top);
						}
		| env_enter block env_leave	{ $$ = $2; }
		/* TODO: SWEITCH, GOTO */
		;

expr		: expr INC			{
							$$ = syntree_new_node(1, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->expr = K_OPR;
							$$->token = INC;
						}
		| expr DEC			{
							$$ = syntree_new_node(1, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->expr = K_OPR;
							$$->token = DEC;
						}
		| LPAREN expr RPAREN		{ $$ = $2; }
		| expr LSBRAC expr RSBRAC	{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_ARY;
							$$->symbol = $1->symbol;
						}
		| expr DOT expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $2;
							$$->expr = K_OPR;
							$$->token = DOT;
						}
		| expr MEMBER expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $2;
							$$->expr = K_OPR;
							$$->token = MEMBER;
						}
		| INC expr %prec PINC		{
							$$ = syntree_new_node(1, K_EXPR, T_VOID);
							$$->child[0] = $2;
							$$->expr = K_OPR;
							$$->token = PINC;
						}
		| DEC expr %prec PDEC		{
							$$ = syntree_new_node(1, K_EXPR, T_VOID);
							$$->child[0] = $2;
							$$->expr = K_OPR;
							$$->token = PDEC;
						}
		| PLUS expr %prec UPLUS		{
							$$ = syntree_new_node(1, K_EXPR, T_VOID);
							$$->child[0] = $2;
							$$->expr = K_OPR;
							$$->token = UPLUS;
						}
		| MINUS expr %prec UMINUS	{
							$$ = syntree_new_node(1, K_EXPR, T_VOID);
							$$->child[0] = $2;
							$$->expr = K_OPR;
							$$->token = UMINUS;
						}
		| LNOT expr			{
							$$ = syntree_new_node(1, K_EXPR, T_VOID);
							$$->child[0] = $2;
							$$->expr = K_OPR;
							$$->token = LNOT;
						}
		| NOT expr			{
							$$ = syntree_new_node(1, K_EXPR, T_VOID);
							$$->child[0] = $2;
							$$->expr = K_OPR;
							$$->token = NOT;
						}
		| MULTIPLY expr %prec PTR	{
							$$ = syntree_new_node(1, K_EXPR, T_VOID);
							$$->child[0] = $2;
							$$->expr = K_OPR;
							$$->token = PTR;
						}
		| AND expr %prec REFR		{
							$$ = syntree_new_node(1, K_EXPR, T_VOID);
							$$->child[0] = $2;
							$$->expr = K_OPR;
							$$->token = REFR;
						}
		| expr MULTIPLY expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = MULTIPLY;
						}
		| expr DIVIDE expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = DIVIDE;
						}
		| expr MOD expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = MOD;
						}
		| expr PLUS expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = PLUS;
						}
		| expr MINUS expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = MINUS;
						}
		| expr SHL expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = SHL;
						}
		| expr SHR expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = SHR;
						}
		| expr LT expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = LT;
						}
		| expr LE expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = LE;
						}
		| expr GT expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = GT;
						}
		| expr GE expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = GE;
						}
		| expr EQ expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = EQ;
						}
		| expr NE expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = NE;
						}
		| expr AND expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = AND;
						}
		| expr XOR expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = XOR;
						}
		| expr OR expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = OR;
						}
		| expr LAND expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = LAND;
						}
		| expr LOR expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = LOR;
						}
		| expr ASSIGN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = ASSIGN;
						}
		| expr PLUSASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = PLUSASN;
						}
		| expr MINUSASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = MINUSASN;
						}
		| expr MULASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = MULASN;
						}
		| expr DIVASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = DIVASN;
						}
		| expr MODASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = MODASN;
						}
		| expr SHLASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = SHLASN;
						}
		| expr SHRASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = SHRASN;
						}
		| expr ANDASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = ANDASN;
						}
		| expr XORASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = XORASN;
						}
		| expr ORASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = ORASN;
						}
		| expr COMMA expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->expr = K_OPR;
							$$->token = COMMA;
						}
/* TODO
		| STRING			{
							$$ = syntree_new_node(0, K_EXPR, T_STR);
							$$->expr = K_STR;
							$$->str = strdup(yytext);
						}
*/
		| CHARACTER			{
							$$ = syntree_new_node(0, K_EXPR, T_CHAR);
							$$->expr = K_CHAR;
							$$->c = lastval;
						}
		| FLOATPNT			{
							$$ = syntree_new_node(0, K_EXPR, T_DOUBLE);
							$$->expr = K_DOUBLE;
							$$->dval = lastdval;
						}
		| INTEGER			{
							$$ = syntree_new_node(0, K_EXPR, T_INT);
							$$->expr = K_INT;
							$$->val = lastval;
						}
		| sym_lookup			{
		/* TODO: add hash operation to c existence */
							$$ = syntree_new_node(0, K_EXPR, T_VOID);
							$$->expr = K_SYM;
							$$->symbol = STACK_POP(sym_stack, sym_top);
							$$->ntype = $$->symbol->type->kind;
						}
		| sym_lookup LPAREN exprz RPAREN{
		/* call function */
							$$ = syntree_new_node(1, K_EXPR, T_CALL);
							$$->expr = K_CALL;
							$$->symbol = STACK_POP(sym_stack, sym_top);
							$$->child[0] = $3;
						}
		/* TODO: IFF */
		;

exprz		: expr				{ $$ = $1; }
		|				{ $$ = NULL; }
		;

%%

