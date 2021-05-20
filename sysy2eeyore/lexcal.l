%{
#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "y.tab.h"

char* oct2dec(char *num);
char* hex2dec(char *num);
char* num2char(int num);
//void yyerror(char*);
%}


/* definitions */
/*here the signed int can be dec,oct and hex*/
decInt		([1-9][0-9]*)
hexInt		(0[xX][0-9a-fA-F]+)
octInt		(0[0-7]*)
Identifier	([_a-zA-Z][_a-zA-Z0-9]*)
/*comment should also be taken into account*/
comment1	("//"[^\n]*)
comment2	("/*"((.|(\n))*)"*/")





/* rules */
%%

[ \t]			{}
[\n]			{lineno++;}
{comment1}		{}
{comment2}		{}
"int"			{return INT;}
"void"			{return VOID;}
"if"			{return IF;}
"else"			{return ELSE;}
"while"			{return WHILE;}
"continue"		{return CONT;}
"break"			{return BREAK;}
"return"		{return RET;}
"constant"		{return CONS;}
{decInt}		{yylval=makeNode(strdup(yytext),NULL,_imme);return IMM;}
{octInt}		{yylval=makeNode(oct2dec(strdup(yytext)),NULL,_imme);return IMM;}
{hexInt}		{yylval=makeNode(hex2dec(strdup(yytext)),NULL,_imme);return IMM;}
{Identifier}	{yylval=makeNode(strdup(yytext),NULL,_iden);return ID;}
"+"				{return ADD;}
"-"				{return SUB;}
"*"				{return MUL;}
"/"				{return DIV;}
"%"				{return MOD;}
"="				{return ASG;}
"!"				{return NOT;}
"<"				{return LT;}
">"				{return GT;}
"<="			   			{return LE;}
">="			{return GE;}
"=="			{return EQ;}
"!="			{return NE;}
"&&"			{return AND;}
"||"			{return OR;}
"("				{return LBS;}
")"				{return RBS;}
"["				{return LBM;}
"]"				{return RBM;}
"{"				{return LBL;}
"}"				{return RBL;}
","					{return COMMA;}
";"					{return SEMI;}



%%

/*int main(){
	//for printing the result in the cmd line	
	while(yylex()){
		puts(yytext);
	}
	return 0;
	//yylex();
	//return 1;
}*/

char* num2char(int num){
	int len=1;
	int tmp=num;
	while (tmp/10!=0){
		tmp/=10;
		len++;
	}
	char* string=malloc(sizeof(char)*(len+1));
	int i;
	for (i=0;i<len;i++){
		string[i]= num/pow(10,len-i-1)+48;
		num=num % (int)pow(10,len-i-1);
	}
	string[len] ='\0';
	//printf("string= %s\n",string);
	return string;
}


char* hex2dec(char *num){
	int n=0;
	int len=strlen(num);
	int i; 
	for (i=len-1;i>1;i--){
		int digit;
		//printf("%d\n",num[i]);
		if ((num[i]>='A') && (num[i]<='F'))
			digit=num[i]-(int)'A'+10;
		else if ((num[i]>='a') && (num[i]<='f'))
			digit=num[i]-(int)'a'+10;
		else
			digit=num[i]-48;
		//printf("char= %c, digit= %d\n",num[i],digit);
		n+=(digit*pow(16,len-i-1));
		//printf(i);
	}
	char* string;
	string=num2char(n);
	//printf("string= %s\n",string);
	return string;
}

char* oct2dec(char *num){
	int n=0;
	//printf("enter here\n");
	int len=strlen(num);
	//printf("len=%d",len); 
	int i;
	for (i=len-1;i>0;i--){
		//printf("digit= %c\n",num[i]); 
		n+=((num[i]-48)*pow(8,len-i-1));
		//printf("num= %d\n",n);
	}
	//printf("enter 2\n");
	//printf("n= %d\n",n);
	char* string;
	string=num2char(n);
	return string;
}

int yywrap(){
	return 1;
}
