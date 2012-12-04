%{
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
%token ID
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
%left INC DEC DOT MEMBER

%nonassoc NOELSE
%nonassoc ELSE

%%

program		: code				{
							printf("node id(env id):\tDescription\tChildren\n");
							syntree_translate($$);
						}
		/* TODO: error handling */
		;

code		: code global			{ $$ = syntree_insert_last($1, $2); }
		| global			{ $$ = $1; }
		;

global		: var_def			{ $$ = $1; }
		| func_def			{ $$ = $1; }
		| struct_def			{ $$ = $1; }
		;

identifier	: ID				{
							lastid_top()->type = lasttype;
						}
		| MULTIPLY ID			{
		/* pointer, does not support pointer to pointer */
							lastid_top()->type = lasttype;
							lastid_top()->ptrcount = 1;
							lastid_top()->arysize = -1;
						}
		| ID LSBRAC INTEGER RSBRAC	{
		/* one-dimensional array */
							lastid_top()->type = lasttype;
							lastid_top()->ptrcount = 1;	/* array is pointer essentially */
							lastid_top()->arysize = lastval;
						}
		| MULTIPLY ID LSBRAC INTEGER RSBRAC	{
		/* one-dimensional array of pointer */	
							lastid_top()->type = lasttype;
							lastid_top()->ptrcount = 2;	/* array is pointer essentially */
							lastid_top()->arysize = lastval;
						}
		;

idlist		: idlist COMMA identifier	{
							id_insert_last(lastid_list, id_new_node(lastid_pop()));
						}
		| identifier			{
							lastid_list = id_new_node(lastid_pop());
						}
		;

var_def		: TYPE idlist SEMI		{
	 	/* does not support define with initialization */
							$$ = syntree_new_node(0, K_DEF);
							$$->idlist = lastid_list;
						}
		;

var_def_list	: var_def_list var_def		{
							$$ = syntree_insert_last($1, $2);
						}
		| var_def			{
							$$ = $1;
						}
		;

func_head	: TYPE ID			{
							$$ = syntree_new_node(2, K_FUNC);
							lastid_top()->type = lasttype;
							lastid_top()->isfunc = TRUE;
							$$->id = lastid_pop();
						}
		| TYPE MULTIPLY ID		{
							$$ = syntree_new_node(2, K_FUNC);
							lastid_top()->type = lasttype;
							lastid_top()->isfunc = TRUE;
							lastid_top()->ptrcount = 1;
							lastid_top()->arysize = -1;
							$$->id = lastid_pop();
						}
		;

func_def	: func_head LPAREN params RPAREN block	{
								$1->child[0] = $3;
								$1->child[1] = $5;
								$$ = $1;
							}
		;

struct_def	: STRUCT ID LBRACE var_def_list RBRACE SEMI	{
		/* TODO */
	  	/* does not support defining struct right after struct def or without any var definition */
							}
		;

params		: param_list				{
								$$ = 0;
		/* TODO */
							}
		|					{ $$ = 0; }
		;

param_list	: param_list COMMA param		{
		/* TODO */
							}
		| param
		;

param		: TYPE ID				{
		/* TODO */
							}
		| TYPE MULTIPLY ID	/* support pointer parameter but does not support array parameter */
		;

sentence	: var_def				{ $$ = $1; }
		| stmt					{ $$ = $1; }
		| expr SEMI				{ $$ = $1; }
		;

sentence_list	: sentence_list sentence		{
								if ($1 == NULL) {
									$$ = $2;
								} else {
									syntree_insert_last($1, $2);
								}
							}
		|					{ $$ = 0; }
		;

block		: LBRACE sentence_list RBRACE		{ $$ = $2; }
		;

stmt		: IF LPAREN expr RPAREN sentence ELSE sentence %prec ELSE	{
								$$ = syntree_new_node(3, K_STMT);
								$$->child[0] = $3;
								$$->child[1] = $5;
								$$->child[2] = $7;
								$$->stmt = K_IFELSE;
							}
		| IF LPAREN expr RPAREN sentence %prec NOELSE	{
								$$ = syntree_new_node(2, K_STMT);
								$$->child[0] = $3;
								$$->child[1] = $5;
								$$->stmt = K_IF;
							}

		| WHILE LPAREN expr RPAREN sentence	{
								$$ = syntree_new_node(2, K_STMT);
								$$->child[0] = $3;
								$$->child[1] = $5;
								$$->stmt = K_WHILE;
							}
		| DO block WHILE LPAREN expr RPAREN SEMI	{
								$$ = syntree_new_node(2, K_STMT);
								$$->child[0] = $2;
								$$->child[1] = $5;
								$$->stmt = K_DO;
							}
		| FOR LPAREN exprz SEMI exprz SEMI exprz RPAREN sentence	{
								$$ = syntree_new_node(4, K_STMT);
								$$->child[0] = $3;
								$$->child[1] = $5;
								$$->child[2] = $7;
								$$->child[3] = $9;
								$$->stmt = K_FOR;
							}
		| RETURN expr SEMI			{
								$$ = syntree_new_node(1, K_STMT);
								$$->child[0] = $2;
								$$->stmt = K_RET;
							}
		| block					{ $$ = $1; }
		/* TODO: SWEITCH, GOTO */
		;

expr		: expr INC				{
								$$ = syntree_new_node(1, K_EXPR);
								$$->child[0] = $1;
								$$->expr = K_OPR;
								$$->token = INC;
							}
		| expr DEC				{
								$$ = syntree_new_node(1, K_EXPR);
								$$->child[0] = $1;
								$$->expr = K_OPR;
								$$->token = DEC;
							}
		| expr DOT ID				{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = syntree_new_node(0, K_EXPR);
								$$->child[1]->expr = K_ID;
								$$->child[1]->id = lastid_pop();
								$$->expr = K_OPR;
								$$->token = DOT;
							}
		| expr MEMBER ID			{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = syntree_new_node(0, K_EXPR);
								$$->child[1]->expr = K_ID;
								$$->child[1]->id = lastid_pop();
								$$->expr = K_OPR;
								$$->token = MEMBER;
							}
		| INC expr %prec PINC			{
								$$ = syntree_new_node(1, K_EXPR);
								$$->child[0] = $2;
								$$->expr = K_OPR;
								$$->token = PINC;
							}
		| DEC expr %prec PDEC			{
								$$ = syntree_new_node(1, K_EXPR);
								$$->child[0] = $2;
								$$->expr = K_OPR;
								$$->token = PDEC;
							}
		| PLUS expr %prec UPLUS			{
								$$ = syntree_new_node(1, K_EXPR);
								$$->child[0] = $2;
								$$->expr = K_OPR;
								$$->token = UPLUS;
							}
		| MINUS expr %prec UMINUS		{
								$$ = syntree_new_node(1, K_EXPR);
								$$->child[0] = $2;
								$$->expr = K_OPR;
								$$->token = UMINUS;
							}
		| LNOT expr				{
								$$ = syntree_new_node(1, K_EXPR);
								$$->child[0] = $2;
								$$->expr = K_OPR;
								$$->token = LNOT;
							}
		| NOT expr				{
								$$ = syntree_new_node(1, K_EXPR);
								$$->child[0] = $2;
								$$->expr = K_OPR;
								$$->token = NOT;
							}
		| MULTIPLY expr %prec PTR		{
								$$ = syntree_new_node(1, K_EXPR);
								$$->child[0] = $2;
								$$->expr = K_OPR;
								$$->token = PTR;
							}
		| AND expr %prec REFR			{
								$$ = syntree_new_node(1, K_EXPR);
								$$->child[0] = $2;
								$$->expr = K_OPR;
								$$->token = REFR;
							}
		| expr MULTIPLY expr			{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = MULTIPLY;
							}
		| expr DIVIDE expr			{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = DIVIDE;
							}
		| expr MOD expr				{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = MOD;
							}
		| expr PLUS expr			{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = PLUS;
							}
		| expr MINUS expr			{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = MINUS;
							}
		| expr SHL expr				{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = SHL;
							}
		| expr SHR expr				{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = SHR;
							}
		| expr LT expr				{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = LT;
							}
		| expr LE expr				{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = LE;
							}
		| expr GT expr				{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = GT;
							}
		| expr GE expr				{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = GE;
							}
		| expr EQ expr				{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = EQ;
							}
		| expr NE expr				{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = NE;
							}
		| expr AND expr				{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = AND;
							}
		| expr XOR expr				{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = XOR;
							}
		| expr OR expr				{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = OR;
							}
		| expr LAND expr			{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = LAND;
							}
		| expr LOR expr				{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = LOR;
							}
		| expr ASSIGN expr			{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = ASSIGN;
							}
		| expr PLUSASN expr			{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = PLUSASN;
							}
		| expr MINUSASN expr			{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = MINUSASN;
							}
		| expr MULASN expr			{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = MULASN;
							}
		| expr DIVASN expr			{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = DIVASN;
							}
		| expr MODASN expr			{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = MODASN;
							}
		| expr SHLASN expr			{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = SHLASN;
							}
		| expr SHRASN expr			{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = SHRASN;
							}
		| expr ANDASN expr			{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = ANDASN;
							}
		| expr XORASN expr			{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = XORASN;
							}
		| expr ORASN expr			{
								$$ = syntree_new_node(2, K_EXPR);
								$$->child[0] = $1;
								$$->child[1] = $3;
								$$->expr = K_OPR;
								$$->token = ORASN;
							}
		| LPAREN expr RPAREN			{ $$ = $2; }
		| STRING				{
								$$ = syntree_new_node(0, K_EXPR);
								$$->expr = K_STR;
								$$->str = strdup(yytext);
							}
		| CHARACTER				{
								$$ = syntree_new_node(0, K_EXPR);
								$$->expr = K_CHAR;
								$$->c = lastval;
							}
		| FLOATPNT				{
								$$ = syntree_new_node(0, K_EXPR);
								$$->expr = K_FLOAT;
								$$->dval = lastdval;
							}
		| INTEGER				{
								$$ = syntree_new_node(0, K_EXPR);
								$$->expr = K_INT;
								$$->val = lastval;
							}
		| ID					{
		/* TODO: add hash operation to check ID existence */
								$$ = syntree_new_node(0, K_EXPR);
								$$->expr = K_ID;
								$$->id = lastid_pop();
							}
		| ID LSBRAC expr RSBRAC			{
								$$ = syntree_new_node(1, K_EXPR);
								$$->child[0] = $3;
								$$->expr = K_ARY;
								$$->id = lastid_pop();
							}
		| call_func				{ $$ = $1; }
		/* TODO: COMMA, IFF */
		;

exprz		: expr					{ $$ = $1; }
		|					{ $$ = 0; }
		;

call_func	: ID LPAREN args RPAREN			{
		/* TODO */
							}
		;

args		: arg_list				{
		/* TODO */
							}
		|
		;

arg_list	: arg_list COMMA expr			{
		/* TODO */
							}
		| expr
		;

%%

