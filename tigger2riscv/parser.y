%{
	#include "tigger2riscv.h"
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	int global;
	int yylex();
	int yyerror(char*);
	#define EQ(a,b) (strcmp(a,b)==0)
	#define CPY(a, b, c) memcpy((void*)a, (const void*)b, (size_t)c)
	#define CMP(a, b, c) memcmp((const void*)a, (const void*)b, (unsigned int)c)
	extern char* strdup(const char*);
	int stk = 0;
%}
%token MALLOC END IF GOTO CALL STORE LOAD LOADADDR RETURN
%token REGNAME FUNCNAME LABNAME IMMNAME 
%token LMB RMB NOT MIN ASS OP2 COL

%%
Goal: 
	  Statement Goal 	{}
	| Statement 		{}
	;
Statement:
	  FUNCNAME LMB IMMNAME RMB LMB IMMNAME RMB	{printFunc($1, $3, $6);}
	| REGNAME ASS MALLOC IMMNAME				{printDecl($1, $4);}
	| REGNAME ASS REGNAME OP2 REGNAME			{printOpe2($4, $3, $5, $1);}
	| REGNAME ASS REGNAME MIN REGNAME			{printOpe2("-", $3, $5, $1);}
	| REGNAME ASS REGNAME OP2 IMMNAME			{printOpe2($4, $3, $5, $1);}
	| REGNAME ASS REGNAME MIN IMMNAME			{printOpe2( "-", $3, $5, $1);}
	| REGNAME ASS MIN REGNAME 					{printOpe1("-", $4, $1);}
	| REGNAME ASS NOT REGNAME					{printOpe1( "!", $4,  $1);}
	| REGNAME ASS REGNAME 						{printAssi($1, $3, NULL, NULL);}
	| REGNAME ASS IMMNAME 						{printAssi($1, $3, NULL, NULL);}
	| REGNAME LMB IMMNAME RMB ASS REGNAME 		{printAssi($1, $6, $3, NULL);}
	| REGNAME ASS REGNAME LMB IMMNAME RMB  		{printAssi($1, $3, NULL, $5);}
	| IF REGNAME OP2 REGNAME GOTO LABNAME 		{printIfbr($3, $2, $4, $6);}
	| GOTO LABNAME 								{printGoto( $2);}
	| LABNAME COL 								{printPlab( $1);}
	| CALL FUNCNAME 							{printCall( $2);}
	| STORE REGNAME IMMNAME 					{printStor($2, $3);}
	| LOAD IMMNAME REGNAME 						{printLoad( $2, $3);}
	| LOAD REGNAME REGNAME 						{printLoad( $2, $3);}
	| LOADADDR IMMNAME REGNAME 					{printLdad( $2, $3);}
	| LOADADDR REGNAME REGNAME 					{printLdad( $2, $3);}
	| RETURN 									{printRetu();}
	| END FUNCNAME 								{printEndf($2);}
	;
%%


int main(int argc, char** argv)
{

	const char* infile=argv[2];
	const char* outfile=argv[4];
	freopen(infile,"r",stdin);
	freopen(outfile,"w",stdout);
	rewind(stdout);
	yyparse();
	return 0;
}

int yyerror(char* error)
{
	fprintf(stderr, "Syntax error: %s\n", error);
	exit(1);
}

