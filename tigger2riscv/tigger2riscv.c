#include "tigger2riscv.h"
#define EQ(a,b) (strcmp(a,b)==0)
#define CPY(a, b, c) memcpy((void*)a, (const void*)b, (size_t)c)
#define CMP(a, b, c) memcmp((const void*)a, (const void*)b, (unsigned int)c)
extern char* strdup(const char*);

extern int stk;


void printFunc(const char* op, const char* src1, const char* src2)
{
	//add 1 to the stack size, position 0 is used to deal with the long integer
	stk = ((atoi(src2)+1)/4+1)*16;
	//fprintf(stdout, "    .size    f,.-f\n");
	fprintf(stdout, "    .text\n");
	fprintf(stdout, "    .align   2\n");
	fprintf(stdout, "    .global  %s\n", op);
	fprintf(stdout, "    .type    %s,@function\n",op);
	fprintf(stdout, "%s:\n", op);
	fprintf(stdout, "    add      sp,sp,-%d\n", stk);
	fprintf(stdout, "    sw       ra,%d(sp)\n", stk-4);
	return;
}

void printDecl(const char* op,const char* src1)
{
	int size = atoi(src1);
	const char * name = op;
	fprintf(stdout, "    .comm    %s,%d,4\n", name, size*4);
	return;
}

void printOpe2(const char* op, const char* src1, const char* src2,const char* dst)
{
	if(EQ(op,"+"))
		fprintf(stdout, "    add      %s,%s,%s\n", dst, src1, src2);
	else if(EQ(op,"-"))
		fprintf(stdout, "    sub      %s,%s,%s\n", dst, src1, src2);
	else if(EQ(op,"*"))
		fprintf(stdout, "    mul      %s,%s,%s\n", dst, src1, src2);
	else if(EQ(op,"/"))
		fprintf(stdout, "    div      %s,%s,%s\n", dst, src1, src2);
	else if(EQ(op,"%"))
		fprintf(stdout, "    rem      %s,%s,%s\n", dst, src1, src2);
	else if(EQ(op,"<"))
		fprintf(stdout, "    slt      %s,%s,%s\n", dst, src1, src2);
	else if(EQ(op,">"))
		fprintf(stdout, "    sgt      %s,%s,%s\n", dst, src1, src2);
	else if(EQ(op,"&&"))
	{
		fprintf(stdout, "    and      %s,%s,%s\n", dst, src1, src2);
		fprintf(stdout, "    snez     %s,%s\n", dst, dst);
	}
	else if(EQ(op,"||"))
	{
		fprintf(stdout, "    or       %s,%s,%s\n", dst, src1, src2);
		fprintf(stdout, "    snez     %s,%s\n", dst, dst);
	}
	else if(EQ(op,"!="))
	{
		fprintf(stdout, "    xor      %s,%s,%s\n", dst, src1, src2);
		fprintf(stdout, "    snez     %s,%s\n", dst, dst);
	}
	else if(EQ(op,"=="))
	{
		fprintf(stdout, "    xor      %s,%s,%s\n", dst, src1, src2);
		fprintf(stdout, "    seqz     %s,%s\n", dst, dst);
	}
	return;
}

void printOpe1(const char* op,const char* src, const char* dst)
{
	if(EQ(op,"!"))
		fprintf(stdout, "    seqz     %s,%s\n", dst, src);
	else fprintf(stdout, "    neg      %s,%s\n", dst, src);
	return;
}

void printAssi(const char* op, const char* src1, const char* src2, const char* dst)
{
	const char* left = op;
	const char* right = src1;
	const char* larray = src2;
	const char* rarray = dst;
  
	if(rarray != NULL)
		fprintf(stdout, "    lw       %s,%s(%s)\n", left, rarray, right);
	else if(larray != NULL)
		fprintf(stdout, "    sw       %s,%s(%s)\n", right, larray, left);
	else if(left[0] == 'v')
	{
		fprintf(stdout, "    .global  %s\n", left);
		fprintf(stdout, "    .section .sdata\n");
		fprintf(stdout, "    .align   2\n");
		fprintf(stdout, "    .type    %s,@object\n", left);
		fprintf(stdout, "    .size    %s,4\n", left);
		fprintf(stdout, "%s:\n", left);
		fprintf(stdout, "    .word    0\n");
	}
	else if(isdigit(right[0]) || right[0]=='-')
		fprintf(stdout, "    li       %s,%s\n", left, right);
	else fprintf(stdout, "    mv       %s,%s\n", left, right);
	return;
}

void printIfbr(const char* op, const char* src1, const char* src2, const char* label)
{
	if(EQ(op,"!="))
		fprintf(stdout, "    bne");
	else if(EQ(op,"=="))
		fprintf(stdout, "    beq");
	else if(EQ(op,"<"))
		fprintf(stdout, "    blt");
	else fprintf(stdout, "    bgt");
	fprintf(stdout, "      %s,%s,.l%s\n", src1, src2, label);
	return;
}

void printGoto(const char* op)
{
	fprintf(stdout, "    j        .l%s\n", op);
	return;
}

void printPlab(const char* op)
{
	fprintf(stdout, ".l%s:\n", op);
	return;
}

void printCall(const char* op)
{
	fprintf(stdout, "    call     %s\n", op);
	return;
}

void printStor(const char* op,const char* src1)
{
	int var = atoi(src1);
	if (var < 511) {
		const char* reg = op;
		fprintf(stdout, "    sw       %s,%d(sp)\n", reg, (var+1) * 4);
	}
	else {
		const char* reg = op;
		char* tmpreg[5] = { 0 };
		if (reg[0] == "t"&&reg[1] == "0")
			sprintf(tmpreg, "t1");
		else
			sprintf(tmpreg, "t0");
		fprintf(stdout, "    sw       %s,%d(sp)\n", tmpreg, 0);
		fprintf(stdout, "    li       %s,%d\n", tmpreg, (var + 1) * 4);
		fprintf(stdout, "    sw       %s,%s(sp)\n", reg, tmpreg);
		fprintf(stdout, "    lw       %s,%d(sp)\n", tmpreg, 0);
	}
	return;
}

void printLoad(const char* var,const char* reg)
{
	if(isdigit(var[0])&&atoi(var)<511)
		fprintf(stdout, "    lw       %s,%d(sp)\n", reg, atoi(var)*4+4);
	else if (isdigit(var[0]) && atoi(var) >= 511) {
		char* tmpreg[5] = { 0 };
		if (reg[0] == "t"&&reg[1] == "0")
			sprintf(tmpreg, "t1");
		else
			sprintf(tmpreg, "t0");
		fprintf(stdout, "    sw       %s,%d(sp)\n", tmpreg, 0);
		fprintf(stdout, "    li       %s,%d\n", tmpreg, (atoi(var) + 1) * 4);
		fprintf(stdout, "    lw       %s,%s(sp)\n", reg, tmpreg);
		fprintf(stdout, "    lw       %s,%d(sp)\n", tmpreg, 0);
	}
	else
	{
		fprintf(stdout, "    lui      %s,%%hi(%s)\n", reg, var);
		fprintf(stdout, "    lw       %s,%%lo(%s)(%s)\n", reg, var, reg);
	}
	return;
}

void printLdad(const char* var, const char* reg)
{
	if(isdigit(var[0]) && atoi(var) < 511)
		fprintf(stdout, "    addi      %s,sp,%d\n", reg, atoi(var)*4+4);
	else if (isdigit(var[0]) && atoi(var) >= 511) {
		char* tmpreg[5] = { 0 };
		if (reg[0] == "t"&& reg[1] == "0")
			sprintf(tmpreg, "t1");
		else
			sprintf(tmpreg, "t0");
		fprintf(stdout, "    sw       %s,%d(sp)\n", tmpreg, 0);
		fprintf(stdout, "    li       %s,%d\n", tmpreg, (atoi(var) + 1) * 4);
		fprintf(stdout, "    addi      %s,sp,%s\n", reg, tmpreg);
		fprintf(stdout, "    lw       %s,%d(sp)\n", tmpreg, 0);
	}
	else
	{
		fprintf(stdout, "    lui      %s,%%hi(%s)\n", reg, var);
		fprintf(stdout, "    add      %s,%s,%%lo(%s)\n", reg, reg, var);
	}
	return;
}

void printRetu()
{
	fprintf(stdout, "    lw       ra,%d(sp)\n", stk-4);
	fprintf(stdout, "    add      sp,sp,%d\n", stk);
	fprintf(stdout, "    jr       ra\n");
	return;
}

void printEndf(const char* op)
{
	fprintf(stdout, "    .size    %s,.-%s\n", op, op);
	return;
}

