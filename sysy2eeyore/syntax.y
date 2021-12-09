%{
//annouce the global identifiers
//head files
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include<string.h>
#include "ast.h"

static Node* root;
int yyerror(char*);
int yylex();

%}


//declarations: teminate tokens,unterminate tokens,priorities
%token INT VOID RET WHILE IF ELSE BREAK CONT
%token ADD SUB MUL DIV MOD ASG NOT LT GT
%token LE GE EQ NE AND OR CONS ID IMM
%token LBS LBM LBL RBS RBM RBL COMMA SEMI


//priority (according to baidu pedia)
%left ASG
%left OR
%left AND
%left EQ NE
%left GE GT LE LT
%left ADD SUB
%left MUL DIV MOD
%left NOT


%%
// grammar rules

Start		:	CompUnit					{$$=$1;root=$1;}
			;

CompUnit	:	Decl					{$$=$1;}
			|	FuncDef					{$$=$1;}
			|	CompUnit Decl			{$$=$1;rightMost($1)->sib=$2;}
			|	CompUnit FuncDef		{$$=$1;rightMost($1)->sib=$2;}
			;

Decl		:	ConstDecl							{$$=$1;}
			|	VarDecl								{$$=$1;}
			;

/*here : Btype->INT*/
ConstDecl	:	CONS INT ConstDefList SEMI	
									{$$=makeNode("int;",$3,_cdcl);}
			;

ConstDefList:	ConstDef			{$$=$1;}
			|	ConstDef COMMA ConstDefList			{$$=$1;$1->sib=$3;}
			;

/*Btype		:	INT									{}
			;*/

ConstDef	:	ID Tmp2 ASG ConstInitVal		{$$=makeNode("int[]=",$1,_cdef);
											$1->sib=$2;
											rightMost($2)->sib=$4;}
			|	ID ASG ConstInitVal				{$$=makeNode("int=",$1,_cdef);
												$1->sib=$3;}
			;

Tmp2		:	LBM	ConstExp RBM 					{$$=$2;}
			|	LBM	ConstExp RBM Tmp2				{$$=$2; $2->sib=$4;}
			;
			
ConstInitVal:	ConstExp					{$$=makeNode("init",$1,_expr);}
			|	LBL RBL						{$$=makeNode("empty{}",NULL,_expr);}
			|	LBL ConstInitValList RBL	{$$=makeNode("init{}",$2,_expr);}
			;

ConstInitValList	
			:	ConstInitVal						{$$=$1;}
			|	ConstInitVal COMMA ConstInitValList	{$$=$1; rightMost($1)->sib=$3;}
			;
/*here : Btype->INT*/
VarDecl		:	INT VarDefList SEMI		{$$=makeNode("int;",$2,_vdcl);}
			;

VarDefList	:	VarDef 							{$$=$1;}
			|	VarDef COMMA VarDefList			{$$=$1; $1->sib=$3;}
			;

VarDef		:	ID VarDefPos				{$$=makeNode("int[]",$1,_vdef);
											$1->sib=$2;}
			|	ID							{$$=makeNode("int",$1,_vdef);}
			|	ID VarDefPos ASG InitVal	{$$=makeNode("int[]=",$1,_vdef);
											$1->sib=$2;
											rightMost($2)->sib=$4;}
			|	ID ASG InitVal				{$$=makeNode("int=",$1,_vdef);
											$1->sib=$3;}
			;

VarDefPos	:	LBM	Exp RBM 				{$$=$2;}
			|	LBM	Exp RBM VarDefPos		{$$=$2; $2->sib=$4;}
			;

InitVal		:	Exp							{$$=makeNode("init",$1,_expr);}
			|	LBL RBL						{$$=makeNode("empty{}",NULL,_expr);}
			|	LBL InitValList RBL			{$$=makeNode("init{}",$2,_expr);}
			;

InitValList	:	InitVal						{$$=$1;}
			|	InitVal COMMA InitValList	{$$=$1; rightMost($1)->sib=$3;}
			;



/*FuncDef		:	FuncType ID '('	')' Block			{}
			|	FuncType ID '(' FuncFParams ')' Block	{}
			;

FuncType	:	VOID								{}
			|	INT									{}
			;*/

FuncDef		:	VOID ID LBS	RBS Block			{$$=makeNode("func",$2,_fdef);
												$2->sib=$5;}
			|	VOID ID LBS FuncFParams RBS Block	
												{$$=makeNode("func",$2,_fdef);
												$2->sib=$4;
												rightMost($2)->sib=$6;}
			|	INT ID LBS RBS Block			{$$=makeNode("func=",$2,_fdef);
												$2->sib=$5;}
			|	INT ID LBS FuncFParams RBS Block	
												{$$=makeNode("func=",$2,_fdef);
												$2->sib=$4;
												rightMost($2)->sib=$6;}
			;

FuncFParams	:	FuncFParam COMMA FuncFParams	{$$=$1; $1->sib=$3;}
			|	FuncFParam						{$$=$1;}
			;


Tmp10		: 								{$$ = NULL;}
			| LBM ConstExp RBM Tmp10		{$$ = $2; $2->sib=$4;}
			;

FuncFParam	: INT ID						{$$ = makeNode("param",$2,_expr);}
			| INT ID LBM RBM Tmp10			{$$ = makeNode("param[]",$2,_expr);
											$2->sib=$5;}
			;


Block		:	LBL Tmp6 RBL				{$$=makeNode("{}",$2,_stat);}
			;

Tmp6		:								{$$=NULL;}
			|	BlockItem Tmp6			{$$=$1; rightMost($1)->sib=$2;}
			;

BlockItem	:	Decl								{$$=$1;}
			|	Stmt								{$$=$1;}
			;

Stmt		:	LVal ASG Exp SEMI		{$$=makeNode("lval=",$1,_stat);
										$1->sib=$3;}
			|	SEMI					{$$=makeNode("semi",$1,_stat);}
			|	Exp SEMI				{$$=$1;}
			|	Block					{$$=$1;}
			/* here the if-else causes shift/reduce conflict 
			   but temporarily do nothing to fix it*/
			|	IF LBS Cond RBS Stmt	{$$=makeNode("if",$3,_stat);
										$3->sib=$5;}
			|	IF LBS Cond RBS Stmt ELSE Stmt	{$$=makeNode("ifelse",$3,_stat);
												$3->sib=$5;
												$5->sib=$7;}
			|	WHILE LBS Cond RBS Stmt			{$$=makeNode("while",$3,_stat);
												$3->sib=$5;}
			| 	BREAK SEMI					{$$=makeNode("break",NULL,_stat);}
			|	CONT SEMI				{$$=makeNode("continue",NULL,_stat);}
			|	RET SEMI						{$$=makeNode("ret",NULL,_stat);}
			|	RET Exp SEMI					{$$=makeNode("ret",$2,_stat);}
			;

Exp			:	AddExp								{$$=$1;}
			;

Cond		:	LOrExp								{$$=$1;}
			;

LVal		:	ID Tmp7						{$$=makeNode("lint[]",$1,_expr);
											$1->sib=$2;}
			|	ID							{$$=$1;}
			;

RVal		:	ID Tmp7						{$$=makeNode("rint[]",$1,_expr);
											$1->sib=$2;}
			|	ID							{$$=$1;}
			;


Tmp7		:	LBM Exp RBM							{$$=$2;}
			|	LBM Exp RBM Tmp7					{$$=$2; $2->sib=$4;}
			; 

PrimaryExp	:	LBS Exp RBS							{$$=$2;}
			|	RVal								{$$=$1;}
			|	Number								{$$=$1;}
			;

Number		:	IMM								{$$=$1;}
			;

UnaryExp	:	PrimaryExp						{$$=$1;}
			|	ID LBS RBS					{$$=makeNode("call",$1,_expr);}
			|	ID LBS FuncRParams RBS		{$$=makeNode("call",$1,_expr);
											$1->sib=$3;}
			|	ADD UnaryExp			{$$=makeNode("pos",$2,_expr);}
			|	SUB UnaryExp			{$$=makeNode("neg",$2,_expr);}
			|	NOT UnaryExp			{$$=makeNode("not",$2,_expr);}
			;



FuncRParams	:	Exp 						{$$=$1;}
			|	Exp	COMMA FuncRParams		{$$=$1; $1->sib=$3;}
			;


MulExp		:	UnaryExp					{$$=$1;}
			|	MulExp MUL UnaryExp			{$$=makeNode("*",$1,_expr);
											$1->sib=$3;}
			|	MulExp DIV UnaryExp			{$$=makeNode("/",$1,_expr);
											$1->sib=$3;}
			|	MulExp MOD UnaryExp			{$$=makeNode("%",$1,_expr);
											$1->sib=$3;}
			;

AddExp		:	MulExp						{$$=$1;}
			|	AddExp ADD MulExp			{$$=makeNode("+",$1,_expr);
											$1->sib=$3;}
			|	AddExp SUB MulExp			{$$=makeNode("-",$1,_expr);
											$1->sib=$3;}
			;

RelExp		:	AddExp						{$$=$1;}
			|	RelExp LT AddExp			{$$=makeNode("<",$1,_expr);
											$1->sib=$3;}
			|	RelExp GT AddExp			{$$=makeNode(">",$1,_expr);
											$1->sib=$3;}
			|	RelExp GE AddExp			{$$=makeNode(">=",$1,_expr);
											$1->sib=$3;}
			|	RelExp LE AddExp			{$$=makeNode("<=",$1,_expr);
											$1->sib=$3;}
			;

EqExp		:	RelExp						{$$=$1;}
			|	EqExp EQ RelExp				{$$=makeNode("==",$1,_expr);
											$1->sib=$3;}
			|	EqExp NE RelExp				{$$=makeNode("!=",$1,_expr);
											$1->sib=$3;}
			;

LAndExp		:	EqExp						{$$=$1;}
			|	LAndExp AND EqExp			{$$=makeNode("&&",$1,_expr);
											$1->sib=$3;}
			;

LOrExp		:	LAndExp						{$$=$1;}
			|	LOrExp OR LAndExp			{$$=makeNode("||",$1,_expr);
											$1->sib=$3;}
			;

ConstExp	:	AddExp								{$$=$1;}
			;
	

%%

int main(int argc,char** argv){
	const char* infile=argv[3];
	const char* outfile=argv[5];
	freopen(infile, "r", stdin);
	FILE* fd=freopen(outfile, "w", stdout);
	//freopen(outfile, "w", stdout);
	lineno=0;
	yyparse();
	scanTree(root,fd);
	return 0;
}

int yyerror(char* error){
	fprintf(stderr,"Line %d: syntax error: %s\n",lineno,error);
	exit(1);
}
