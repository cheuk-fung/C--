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
#ifdef NGDEBUG
							assert(sym_top == 0);
							assert(type_top == 0);
#endif
							fmsg = fopen("msg.out", "w");
							fprintf(fmsg, "node id(env id):\tDescription\tChildren\n");
							syntree_translate($1);
							fclose(fmsg);

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
							symbol->ptr_cnt = 1;
						}
		| MULTIPLY pointer		{
							struct Stab *symbol = STACK_TOP(sym_stack, sym_top);
							symbol->ptr_cnt++;
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
							struct Stab *symbol = STACK_POP(sym_stack, sym_top);
							struct Syntree_node *t = syntree_new_node(0, K_DEF, ti->kind, NULL, (void *)symbol, 0, 0, 0, 0);
							$$ = syntree_insert_last($1, t);
						}
		| identifier			{
							struct Type_info *ti = STACK_TOP(type_stack, type_top);
							struct Stab *symbol = STACK_POP(sym_stack, sym_top);
							$$ = syntree_new_node(0, K_DEF, ti->kind, NULL, (void *)symbol, 0, 0, 0, 0);
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
							struct Stab *symbol = STACK_POP(sym_stack, sym_top);
							symbol->isfunc = TRUE;
							symbol->type = STACK_POP(type_stack, type_top);
							$$ = syntree_new_node(1, K_FUNC, T_VOID, NULL, (void *)symbol, $7, 0, 0, 0);
						}
		;

struct_def	: STRUCT sym_insert env_enter LBRACE var_def_list RBRACE env_leave SEMI	{
	  	/* does not support defining struct right after struct def or without any var definition */
							struct Stab *symbol = STACK_POP(sym_stack, sym_top);
							symbol->type = type_new(T_STRUCT, symbol);
							if ($5) {
								symbol->member_cnt = $5->env->symbol_cnt;
								symbol->member_env = $5->env;
							}
							$$ = syntree_new_node(1, K_STRUCT, T_STRUCT, NULL, (void *)symbol, $5, 0, 0, 0);
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
							$$ = syntree_new_node(3, K_STMT, T_VOID, (void *)K_IFELSE, NULL, $3, $5, $7, 0);
						}
		| IF LPAREN expr RPAREN sentence %prec NOELSE	{
							$$ = syntree_new_node(2, K_STMT, T_VOID, (void *)K_IF, NULL, $3, $5, 0, 0);
						}

		| WHILE LPAREN expr RPAREN sentence	{
							$$ = syntree_new_node(2, K_STMT, T_VOID, (void *)K_WHILE, NULL, $3, $5, 0, 0);
						}
		| DO env_enter block env_leave WHILE LPAREN expr RPAREN SEMI	{
							$$ = syntree_new_node(2, K_STMT, T_VOID, (void *)K_DO, NULL, $3, $7, 0, 0);
						}
		| FOR LPAREN exprz SEMI exprz SEMI exprz RPAREN sentence	{
							$$ = syntree_new_node(4, K_STMT, T_VOID, (void *)K_FOR, NULL, $3, $5, $7, $9);
						}
		| RETURN expr SEMI		{
							struct Stab *symbol = STACK_TOP(sym_stack, sym_top);
							$$ = syntree_new_node(1, K_STMT, T_VOID, (void *)K_RET, (void *)symbol, $2, 0, 0, 0);
						}
		| RETURN SEMI			{
							struct Stab *symbol = STACK_TOP(sym_stack, sym_top);
							$$ = syntree_new_node(0, K_STMT, T_VOID, (void *)K_RET, (void *)symbol, 0, 0, 0, 0);
						}
		| env_enter block env_leave	{ $$ = $2; }
		/* TODO: SWEITCH, GOTO */
		;

expr		: expr INC			{
							$$ = syntree_new_node(1, K_EXPR, T_VOID, (void *)K_OPR, (void *)INC, $1, 0, 0, 0);
						}
		| expr DEC			{
							$$ = syntree_new_node(1, K_EXPR, T_VOID, (void *)K_OPR, (void *)DEC, $1, 0, 0, 0);
						}
		| LPAREN expr RPAREN		{ $$ = $2; }
		| expr LSBRAC expr RSBRAC	{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_ARY, (void *)($1->info.symbol), $1, $3, 0, 0);
						}
		| expr DOT SYM			{
							if ($1->ntype.kind != T_STRUCT) {
								fprintf(stderr, "Not a struct on line %d.\n", $1->lineno);
								exit(1);
							}
							struct Stab *symbol = env_lookup($1->ntype.struct_sym->member_env, lastsym);
							struct Syntree_node *node = syntree_new_node(0, K_EXPR, symbol->type->kind, (void *)K_SYM, (void *)symbol, 0, 0, 0, 0);
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)DOT, $1, node, 0, 0);
						}
		| expr MEMBER SYM		{
		/* TODO */
						}
		| INC expr %prec PINC		{
							$$ = syntree_new_node(1, K_EXPR, T_VOID, (void *)K_OPR, (void *)PINC, $2, 0, 0, 0);
						}
		| DEC expr %prec PDEC		{
							$$ = syntree_new_node(1, K_EXPR, T_VOID, (void *)K_OPR, (void *)PDEC, $2, 0, 0, 0);
						}
		| PLUS expr %prec UPLUS		{
							$$ = syntree_new_node(1, K_EXPR, T_VOID, (void *)K_OPR, (void *)UPLUS, $2, 0, 0, 0);
						}
		| MINUS expr %prec UMINUS	{
							$$ = syntree_new_node(1, K_EXPR, T_VOID, (void *)K_OPR, (void *)UMINUS, $2, 0, 0, 0);
						}
		| LNOT expr			{
							$$ = syntree_new_node(1, K_EXPR, T_VOID, (void *)K_OPR, (void *)LNOT, $2, 0, 0, 0);
						}
		| NOT expr			{
							$$ = syntree_new_node(1, K_EXPR, T_VOID, (void *)K_OPR, (void *)NOT, $2, 0, 0, 0);
						}
		| MULTIPLY expr %prec PTR	{
							$$ = syntree_new_node(1, K_EXPR, T_VOID, (void *)K_OPR, (void *)PTR, $2, 0, 0, 0);
						}
		| AND expr %prec REFR		{
							$$ = syntree_new_node(1, K_EXPR, T_VOID, (void *)K_OPR, (void *)REFR, $2, 0, 0, 0);
						}
		| expr MULTIPLY expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)MULTIPLY, $1, $3, 0, 0);
						}
		| expr DIVIDE expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)DIVIDE, $1, $3, 0, 0);
						}
		| expr MOD expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)MOD, $1, $3, 0, 0);
						}
		| expr PLUS expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)PLUS, $1, $3, 0, 0);
						}
		| expr MINUS expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)MINUS, $1, $3, 0, 0);
						}
		| expr SHL expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)SHL, $1, $3, 0, 0);
						}
		| expr SHR expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)SHR, $1, $3, 0, 0);
						}
		| expr LT expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)LT, $1, $3, 0, 0);
						}
		| expr LE expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)LE, $1, $3, 0, 0);
						}
		| expr GT expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)GT, $1, $3, 0, 0);
						}
		| expr GE expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)GE, $1, $3, 0, 0);
						}
		| expr EQ expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)EQ, $1, $3, 0, 0);
						}
		| expr NE expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)NE, $1, $3, 0, 0);
						}
		| expr AND expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)AND, $1, $3, 0, 0);
						}
		| expr XOR expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)XOR, $1, $3, 0, 0);
						}
		| expr OR expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)OR, $1, $3, 0, 0);
						}
		| expr LAND expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)LAND, $1, $3, 0, 0);
						}
		| expr LOR expr			{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)LOR, $1, $3, 0, 0);
						}
		| expr ASSIGN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)ASSIGN, $1, $3, 0, 0);
						}
		| expr PLUSASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)PLUSASN, $1, $3, 0, 0);
						}
		| expr MINUSASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)MINUSASN, $1, $3, 0, 0);
						}
		| expr MULASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)MULASN, $1, $3, 0, 0);
						}
		| expr DIVASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)DIVASN, $1, $3, 0, 0);
						}
		| expr MODASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)MODASN, $1, $3, 0, 0);
						}
		| expr SHLASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)SHLASN, $1, $3, 0, 0);
						}
		| expr SHRASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)SHRASN, $1, $3, 0, 0);
						}
		| expr ANDASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)ANDASN, $1, $3, 0, 0);
						}
		| expr XORASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)XORASN, $1, $3, 0, 0);
						}
		| expr ORASN expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)ORASN, $1, $3, 0, 0);
						}
		| expr COMMA expr		{
							$$ = syntree_new_node(2, K_EXPR, T_VOID, (void *)K_OPR, (void *)COMMA, $1, $3, 0, 0);
						}
		| STRING			{
							strings[str_cnt] = strdup(yytext);
							$$ = syntree_new_node(0, K_EXPR, T_STR, (void *)K_STR, (void *)str_cnt++, 0, 0, 0, 0);
						}
		| CHARACTER			{
							$$ = syntree_new_node(0, K_EXPR, T_CHAR, (void *)K_CHAR, (void *)lastval, 0, 0, 0, 0);
						}
		| FLOATPNT			{
							$$ = syntree_new_node(0, K_EXPR, T_DOUBLE, (void *)K_DOUBLE, NULL, 0, 0, 0, 0);
						}
		| INTEGER			{
							$$ = syntree_new_node(0, K_EXPR, T_INT, (void *)K_INT, (void *)lastval, 0, 0, 0, 0);
						}
		| sym_lookup			{
		/* TODO: add hash operation to c existence */
							struct Stab *symbol = STACK_POP(sym_stack, sym_top);
							$$ = syntree_new_node(0, K_EXPR, symbol->type->kind, (void *)K_SYM, (void *)symbol, 0, 0, 0, 0);
						}
		| sym_lookup LPAREN exprz RPAREN{
		/* call function */
							struct Stab *symbol = STACK_POP(sym_stack, sym_top);
							$$ = syntree_new_node(1, K_EXPR, T_VOID, (void *)K_CALL, (void *)symbol, $3, 0, 0, 0);
						}
		/* TODO: IFF */
		;

exprz		: expr				{ $$ = $1; }
		|				{ $$ = NULL; }
		;

%%

