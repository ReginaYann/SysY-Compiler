#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>
#define true 1
#define false 0
//#define testing
extern char* strdup(const char*);

void printFunc(const char* op, const char* src1, const char* src2);
void printDecl(const char* op, const char* src1);
void printOpe2(const char* op, const char* src1, const char* src2, const char* dst);
void printOpe1(const char* op, const char* src1, const char* dst);
void printAssi(const char* op, const char* src1, const char* src2, const char* dst);
void printIfbr(const char* op, const char* src1, const char* src2, const char* label);
void printGoto(const char* op);
void printPlab(const char* op);
void printCall(const char* op);
void printStor(const char* op, const char* src1);
void printLoad(const char* var, const char* reg);
void printLdad(const char* var, const char* reg);
void printRetu();
void printEndf(const char* op);

#define YYSTYPE const char*
