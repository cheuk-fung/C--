%{
#include "global.h"
#include "parser.h"
#include "env.h"
#include "stab.h"

enum { BLKCMTBEGIN, BLKCMTEND, LINECMT, NEWLINE, _NULL };

static int inblkcmt;
static int inlinecmt;

static int proc(int);
%}

%option outfile="lexer.c" header-file="lexer.h"
%option noyywrap

whitespace	[ \t]+
floatnum	[0-9]+\.|[0-9]*\.[0-9]+
integer		[0-9]+
symbol		[_a-zA-Z]+
blkcmtbegin	\/\*
blkcmtend	\*\/
linecmt		\/\/
string		\"[^\"]*\"
character	'.'|'\\.'
type		void|char|int|float|double

%%

{whitespace}	{ }
"\n"		{ proc(NEWLINE); }
{blkcmtbegin}	{ int r =  proc(BLKCMTBEGIN); if (r != COMMENT) return r; }
{blkcmtend}	{ int r =  proc(BLKCMTEND); if (r != COMMENT) return r; }
{linecmt}	{ int r =  proc(LINECMT); if (r != COMMENT) return r; }
{string}	{ int r =  proc(STRING); if (r != COMMENT) return r; }
{character}	{ int r =  proc(CHARACTER); if (r != COMMENT) return r; }
";"		{ int r =  proc(SEMI); if (r != COMMENT) return r; }
":"		{ int r =  proc(COLON); if (r != COMMENT) return r; }
"."		{ int r =  proc(DOT); if (r != COMMENT) return r; }
","		{ int r =  proc(COMMA); if (r != COMMENT) return r; }
"_"		{ int r =  proc(UL); if (r != COMMENT) return r; }
"#"		{ int r =  proc(SHAPE); if (r != COMMENT) return r; }
"\\"		{ int r =  proc(STROKE); if (r != COMMENT) return r; }
"?"		{ int r =  proc(QUEST); if (r != COMMENT) return r; }
"'"		{ int r =  proc(SQUOTE); if (r != COMMENT) return r; }
"\""		{ int r =  proc(DQUOTE); if (r != COMMENT) return r; }
"{"		{ int r =  proc(LBRACE); if (r != COMMENT) return r; }
"}"		{ int r =  proc(RBRACE); if (r != COMMENT) return r; }
"["		{ int r =  proc(LSBRAC); if (r != COMMENT) return r; }
"]"		{ int r =  proc(RSBRAC); if (r != COMMENT) return r; }
"("		{ int r =  proc(LPAREN); if (r != COMMENT) return r; }
")"		{ int r =  proc(RPAREN); if (r != COMMENT) return r; }
"->"		{ int r =  proc(MEMBER); if (r != COMMENT) return r; }
"++"		{ int r =  proc(INC); if (r != COMMENT) return r; }
"--"		{ int r =  proc(DEC); if (r != COMMENT) return r; }
"+="		{ int r =  proc(PLUSASN); if (r != COMMENT) return r; }
"-+"		{ int r =  proc(MINUSASN); if (r != COMMENT) return r; }
"*="		{ int r =  proc(MULASN); if (r != COMMENT) return r; }
"/="		{ int r =  proc(DIVASN); if (r != COMMENT) return r; }
"%="		{ int r =  proc(MODASN); if (r != COMMENT) return r; }
"<<="		{ int r =  proc(SHLASN); if (r != COMMENT) return r; }
">>="		{ int r =  proc(SHRASN); if (r != COMMENT) return r; }
"&="		{ int r =  proc(ANDASN); if (r != COMMENT) return r; }
"^="		{ int r =  proc(XORASN); if (r != COMMENT) return r; }
"|="		{ int r =  proc(ORASN); if (r != COMMENT) return r; }
"+"		{ int r =  proc(PLUS); if (r != COMMENT) return r; }
"-"		{ int r =  proc(MINUS); if (r != COMMENT) return r; }
"*"		{ int r =  proc(MULTIPLY); if (r != COMMENT) return r; }
"/"		{ int r =  proc(DIVIDE); if (r != COMMENT) return r; }
"%"		{ int r =  proc(MOD); if (r != COMMENT) return r; }
"&&"		{ int r =  proc(LAND); if (r != COMMENT) return r; }
"||"		{ int r =  proc(LOR); if (r != COMMENT) return r; }
"!"		{ int r =  proc(LNOT); if (r != COMMENT) return r; }
"&"		{ int r =  proc(AND); if (r != COMMENT) return r; }
"|"		{ int r =  proc(OR); if (r != COMMENT) return r; }
"^"		{ int r =  proc(XOR); if (r != COMMENT) return r; }
"~"		{ int r =  proc(NOT); if (r != COMMENT) return r; }
"<<"		{ int r =  proc(SHL); if (r != COMMENT) return r; }
">>"		{ int r =  proc(SHR); if (r != COMMENT) return r; }
"=="		{ int r =  proc(EQ); if (r != COMMENT) return r; }
"!="		{ int r =  proc(NE); if (r != COMMENT) return r; }
"<="		{ int r =  proc(LE); if (r != COMMENT) return r; }
">="		{ int r =  proc(GE); if (r != COMMENT) return r; }
"<"		{ int r =  proc(LT); if (r != COMMENT) return r; }
">"		{ int r =  proc(GT); if (r != COMMENT) return r; }
"="		{ int r =  proc(ASSIGN); if (r != COMMENT) return r; }
"NULL"		{ int r =  proc(_NULL); if (r != COMMENT) return r; }
{type}		{ int r =  proc(TYPE); if (r != COMMENT) return r; }
"signed"	{ int r =  proc(SIGNED); if (r != COMMENT) return r; }
"unsigned"	{ int r =  proc(UNSIGNED); if (r != COMMENT) return r; }
"short"		{ int r =  proc(SHORT); if (r != COMMENT) return r; }
"long"		{ int r =  proc(LONG); if (r != COMMENT) return r; }
"struct"	{ int r =  proc(STRUCT); if (r != COMMENT) return r; }
"if"		{ int r =  proc(IF); if (r != COMMENT) return r; }
"else"		{ int r =  proc(ELSE); if (r != COMMENT) return r; }
"while"		{ int r =  proc(WHILE); if (r != COMMENT) return r; }
"do"		{ int r =  proc(DO); if (r != COMMENT) return r; }
"for"		{ int r =  proc(FOR); if (r != COMMENT) return r; }
"switch"	{ int r =  proc(SWITCH); if (r != COMMENT) return r; }
"case"		{ int r =  proc(CASE); if (r != COMMENT) return r; }
"goto"		{ int r =  proc(GOTO); if (r != COMMENT) return r; }
"return"	{ int r =  proc(RETURN); if (r != COMMENT) return r; }
{symbol}	{ int r =  proc(SYM); if (r != COMMENT) return r; }
{floatnum}	{ int r =  proc(FLOATPNT); if (r != COMMENT) return r; }
{integer}	{ int r =  proc(INTEGER); if (r != COMMENT) return r; }

%%

int yyerror(const char *msg)
{
    fprintf(stderr, "Error: %s\n", msg);
    return 0;
}

static int proc(int token)
{
    if (inblkcmt && token != BLKCMTEND) {
        if (token == NEWLINE) yyset_lineno(yyget_lineno() + 1);
        return COMMENT;
    }
    if (inlinecmt && token != NEWLINE) return COMMENT;
    switch (token) {
        case BLKCMTBEGIN: inblkcmt = TRUE; return COMMENT;
        case BLKCMTEND: inblkcmt = FALSE; return COMMENT;
        case LINECMT: inlinecmt = TRUE; return COMMENT;
        case NEWLINE:
            yyset_lineno(yyget_lineno() + 1);
            if (inlinecmt) {
                inlinecmt = FALSE;
                return COMMENT;
            }
            return NEWLINE;
        case TYPE:
            switch (yytext[0]) {
                case 'v': STACK_PUSH(type_stack, type_top, type_new(T_VOID, NULL)); break;
                case 'c': STACK_PUSH(type_stack, type_top, type_new(T_CHAR, NULL)); break;
                case 'i': STACK_PUSH(type_stack, type_top, type_new(T_INT, NULL)); break;
                case 'f': STACK_PUSH(type_stack, type_top, type_new(T_FLOAT, NULL)); break;
                case 'd': STACK_PUSH(type_stack, type_top, type_new(T_DOUBLE, NULL));
            }
            return TYPE;
        case SYM:
            if (prevsym) free(prevsym);
            prevsym = lastsym;
            lastsym = strdup(yytext);
            return SYM;
        case STRING:
            return STRING;
        case CHARACTER:
            lastval = yytext[1];	/* TODO: ESC support */
            return CHARACTER;
        case FLOATPNT:
            sscanf(yytext, "%lf", &lastdval);
            return FLOATPNT;
        case INTEGER:
            sscanf(yytext, "%d", &lastval);
            return INTEGER;
        case _NULL:
	    lastval = 0;
            return INTEGER;
        default:
            return token;
    }
}

