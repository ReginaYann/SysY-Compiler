#include<stdio.h>
#include "ast.h"
#include<string.h>
#include<math.h>

#define false 0
#define true 1
#define unknown 2
#define EQ(a,b) (strcmp(a,b)==0)

int VarCnt, TempCnt, ParamCnt;
int LabelCnt,FuncCnt;
Variable* VarList[2000];
Variable* ParamList[20];
Func* FuncList[1000];
int LayerMark[100];
int currLayer;
int indent;
static int whilelabel1,whilelabel2; //在break和continue的时候可以用到，用来记录最后一个while的标记

void scanNode(Node* node);
Variable* scanEXPR(Node* node);

void initEnv(){
	VarCnt=TempCnt=ParamCnt=0;
	LabelCnt=FuncCnt=0;
	currLayer=indent=0;
	LayerMark[0]=0;
}

Node* rightMost(Node* node){
	while (node->sib)
		node=node->sib;
	return node;
}

Node* makeNode(const char* attr,Node* son,enum _node_type type){
	Node* newNode=(Node*)malloc(sizeof(Node));
	newNode->type=type;
	newNode->son=son;
	newNode->sib=NULL;
	newNode->attr=attr;
	newNode->linenum=lineno;
	return newNode;
}

Variable* makeVar(const char* oriname,int isarray, enum _var_type type, int linenum, int dim, int* arraysize){
	Variable* newVar=(Variable*)malloc(sizeof(Variable));
	newVar->type=type;
	newVar->isarray=isarray;
	newVar->linenum=linenum;
	newVar->valid=true;
	newVar->oriName=(oriname==NULL?NULL:strdup(oriname));
	newVar->dim=dim;
	newVar->arraysize=arraysize;
	if (type==_inst){
		if (isarray||oriname==NULL){
			fprintf(stderr,"Line %d: type error: %s\n",linenum,oriname);
			exit(1);
		}
		newVar->yoreName=strdup(oriname);
		return newVar;
	}
	else{
		char buffer[10]={'\0'};
		if (type==_vari){
			sprintf(buffer,"T%d",VarCnt);
			VarList[VarCnt++]=newVar;
		}
		else if (type==_temp)
			sprintf(buffer,"t%d",TempCnt++);
		else{
			sprintf(buffer,"p%d",ParamCnt);
			ParamList[ParamCnt++]=newVar;
		}
		newVar->yoreName=strdup(buffer);
		if (type!=_para){
			if (indent)
				fprintf(stdout,"	");
			if (!isarray)
				fprintf(stdout,"var %s\n",newVar->yoreName);
			else{
				int index=0;
				int mul=1;
				for (index=0;index<dim;index++)
					mul=mul*arraysize[index];
				fprintf(stdout,"var %d %s\n",4*mul,newVar->yoreName);
			}
		}
		return newVar;
	}
}

Variable* matchVar(const char* oriName, int isarray, enum _var_type type, int linenum){
	if (type==_vari){
		int i;
		for (i=0;i<ParamCnt;++i)
			if (EQ(ParamList[i]->oriName,oriName)){
				if (isarray != unknown && isarray != ParamList[i]->isarray){
					fprintf(stderr,"Line %d: type error: %s\n",linenum,oriName);
					exit(1);
				}
				return ParamList[i];
			}
		for (i=VarCnt-1;i>=0;--i)
			if(EQ(VarList[i]->oriName,oriName)&&VarList[i]->valid){
				if (isarray != unknown && isarray != ParamList[i]->isarray){
					fprintf(stderr,"Line %d: type error: %s\n",linenum,oriName);
					exit(1);
				}
				return VarList[i];
			}
		fprintf(stderr,"Line %d: variable not defined: %s\n",linenum,oriName);
		exit(1);
	}
	else{
		fprintf(stderr,"Line %d:type error: %s\n",linenum,oriName);
		exit(1);
	}
}


void checkParameter(Node* node, parameter* param, const char* funcName, int undefined)
{
	while (node && (undefined || param))
	{
		Variable* temp = scanEXPR(node);
		if ((!undefined) && temp->isarray != param->isarray)
		{
			fprintf(stderr, "Line %d: function parameter type error: %s(...)\n",
				node->linenum, funcName);
			exit(1);
		}
		/*if ((!undefined) && (temp->isarray && param->arraysize && param->arraysize != temp->arraysize))
		{
			fprintf(stderr, "Line %d: function parameter type error: %s(...)\n",
				node->linenum, funcName);
			exit(1);
		}*/
		if (indent) fprintf(stdout, "	");
		fprintf(stdout, "param %s\n", temp->yoreName);
		node = node->sib;
		if (!undefined) param = param->next;
	}
	if ((!undefined) && (node && node->type == _expr) || param)
	{
		fprintf(stderr, "Line %d: function parameter more or less: %s\n", node->linenum, funcName);
		exit(1);
	}
}

//这里的关于parameter的部分还得再改，parameter节点应该不是_vdcl类的
//把parameter节点改成了expr类的，param和param[]
void addFunc(const char* funcName, Node* node, int declaring)
{
	int declaredBefore = false;
	int index;
	int i;
	for (i = 0; i < FuncCnt; ++i)
		if (EQ(funcName, FuncList[i]->funcName))
		{
			if (declaring || ((!declaring) && FuncList[i]->declared == false))
			{
				fprintf(stderr, "Line %d: function redefine or redeclare: %s\n", node->linenum, funcName);
				exit(1);
			}
			declaredBefore = true;
			index = i;
			break;
		}

	if (!declaredBefore)
	{
		Func* newFunc = (Func*)malloc(sizeof(Func));
		newFunc->funcName = strdup(funcName);
		newFunc->linenum = node->linenum;
		newFunc->declared = false;
		ParamCnt = 0;
		if (!(node->sib && node->sib->type == _expr))
			newFunc->first = NULL;
		else
		{
			newFunc->first = (parameter*)malloc(sizeof(parameter));
			node = node->sib;
			Variable* par = scanEXPR(node);
			parameter* tmp = newFunc->first;
			tmp->isarray = par->isarray;
			tmp->dim = par->dim;
			tmp->arraysize = par->arraysize;
			tmp->next = NULL;
			while (node->sib && node->sib->type == _expr)
			{
				tmp->next = (parameter*)malloc(sizeof(parameter));
				tmp = tmp->next;
				node = node->sib;
				Variable* par = scanEXPR(node);
				tmp->isarray = par->isarray;
				tmp->dim = par->dim;
				tmp->arraysize = par->arraysize;
				tmp->next = NULL;
			}
		}
		FuncList[FuncCnt++] = newFunc;
	}
	else
	{
		parameter* ppointer = FuncList[index]->first;
		FuncList[index]->declared = false;
		while (node->sib && node->sib->type == _expr)
		{
			node = node->sib;
			Variable* par = scanEXPR(node);
			if (ppointer == NULL || ppointer->isarray != par->isarray || ppointer->arraysize != par->arraysize)
			{
				fprintf(stderr, "Line %d: function parameter type error: %s\n", node->linenum, funcName);
				exit(1);
			}
			ppointer = ppointer->next;
		}
	}
	return;
}

//还没有写initval,param[]
Variable* scanEXPR(Node* node){
	if (node->type==_imme)
		return makeVar(node->attr,false,_inst,node->linenum,0,NULL);
	else if (node->type==_iden)
		return matchVar(node->attr,false,_vari,node->linenum);
	else{
		/*if (EQ(node->attr,"int")){
			Variable* id=matchVar(node->son->attr,false,_vari,node->linenum);
			//fprintf("%s",id->yoreName);
			return id;
		}*/
		if (EQ(node->attr,"int[]")){
			char buffer[40]="\0";
			Variable* id=matchVar(node->son->attr,true,_vari,node->linenum);
			sprintf(buffer,"%s",id->yoreName);
			int tmpline = node->linenum;
			node=node->son;
			while(node->sib){
				node=node->sib;
				Variable* expr=scanEXPR(node);
				Variable* expr4=makeVar(NULL,false,_temp,node->linenum,0,NULL);
				if (indent) fprintf(stdout,"	");
				fprintf(stdout,"%s = 4 * %s\n",expr4->yoreName,expr->yoreName);
				sprintf(buffer,"[%s]",expr4->yoreName);
			}
			Variable* temp = makeVar(NULL, false, _temp, tmpline, 0, NULL);
			fprintf(stdout, "%s = %s\n", temp->yoreName, buffer);
			return temp;
		}
		else if (EQ(node->attr,"call")){
			const char* funcName = node->son->attr;
			int i;
			for (i = 0; i < FuncCnt; i++) {
				if (EQ(funcName, FuncList[i]->funcName)) {
					checkParameter(node->son->sib, FuncList[i]->first, funcName, 0);
					if (indent) printf("	");
					if (FuncList[i]->isreturn == true) {
						Variable* temp = makeVar(NULL, false, _temp, node->linenum, 0, NULL);
						fprintf(stdout, "%s = call f_%s\n", temp->yoreName, funcName);
						return temp;
					}
					else {
						fprintf(stdout, "call f_%s\n", funcName);
						return NULL;
					}
				}
			}
			fprintf(stderr, "Warning: Line %d: function not defined: %s\n", node->linenum, funcName);
			exit(1);
			/*checkParameter(node->son->sib, NULL, funcName, 1);
			Variable* temp = makeVar(NULL, false, _temp, node->linenum, 0);
			if (indent) fprintf(stdout, "	");
			fprintf(stdout, "%s = call f_%s\n", temp->yoreName, funcName);
			return temp;*/
		}
		else if (EQ(node->attr,"pos") || EQ(node->attr,"neg") ||EQ(node->attr,"not") ){
			const char* op=node->attr;
			Variable *operand =scanEXPR(node->son);
			Variable *temp=makeVar(NULL,false,_temp,node->linenum,0,NULL);
			if (indent)	fprintf(stdout,"	");
			if (EQ(op,"neg"))
				fprintf(stdout,"%s = -%s\n",temp->yoreName,operand->yoreName);
			else if (EQ(op,"not"))
				fprintf(stdout,"%s = !%s\n",temp->yoreName,operand->yoreName);
			else
				fprintf(stdout,"%s = +%s\n",temp->yoreName,operand->yoreName);
			return temp;
		}
		else if (EQ(node->attr, "init")) {
			Variable* expr = scanEXPR(node->son);
			return expr;
		}
		/*else if (EQ(node->attr, "init{}")) {

		}*/
		else if (EQ(node->attr, "param")) {
			return makeVar(node->son->attr, false, _para, node->linenum, 0, NULL);
		}
		/*else if (EQ(node->attr, "param[]")) {
			if (node->son->sib) {

			}
			else {

			}
		}*/
		else{
			const char* op=node->attr;
			Variable* operand1=scanEXPR(node->son);
			Variable* operand2=scanEXPR(node->son->sib);
			Variable* temp=makeVar(NULL,false,_temp,node->linenum,0,NULL);
			char buffer[20]={'\0'};
			sprintf(buffer,"%s = %s %s %s",temp->yoreName,operand1->yoreName,op,operand2->yoreName);
			if (indent)	fprintf(stdout,"	");
			fprintf(stdout,"%s\n",buffer);
			return temp;
		}
	}
}

//stat应该写完了？？
void scanSTAT(Node* node){
	const char* op=node->attr;
	if (EQ(op,"if")){
		int label=LabelCnt++;
		Variable* cond=scanEXPR(node->son);
		char buffer[30]={'\0'};
		sprintf(buffer,"if %s == 0 goto l%d\n",cond->yoreName,label);
		if (indent) fprintf(stdout,"	");
		fprintf(stdout,"%s\n",buffer);
		scanNode(node->son->sib);
		fprintf(stdout,"l%d:\n",label);
		return;
	}
	else if (EQ(op,"ifelse")){
		int label1=LabelCnt++;
		int label2=LabelCnt++;
		Variable* cond=scanEXPR(node->son);
		if (indent) fprintf(stdout, "	");
		fprintf(stdout,"if %s == 0 goto l%d\n",cond->yoreName,label1);
		scanNode(node->son->sib);
		if (indent) fprintf(stdout,"	");
		fprintf(stdout,"goto l%d\n",label2);
		fprintf(stdout,"l%d:\n",label1);
		scanNode(node->son->sib->sib);
		fprintf(stdout,"l%d:\n",label2);
		return;
	}
	else if (EQ(op,"while")){
		int label1=LabelCnt++;
		int label2=LabelCnt++;
		whilelabel1=label1;
		whilelabel2=label2;
		fprintf(stdout,"l%d:\n",label1);
		Variable* cond=scanEXPR(node->son);
		if (indent) fprintf(stdout,"	");
		fprintf(stdout,"if %s == 0 goto l%d\n",cond->yoreName,label2);
		scanNode(node->son->sib);
		if (indent) fprintf(stdout,"	");
		fprintf(stdout,"goto l%d\n",label1);
		fprintf(stdout,"l%d:\n",label2);
		return;
	}
	else if (EQ(op,"break")){
		if (indent) fprintf(stdout,"	");
		fprintf(stdout,"goto l%d\n",whilelabel2);
		return;
	}
	else if (EQ(op,"continue")){
		if (indent) fprintf(stdout,"	");
		fprintf(stdout,"goto l%d\n",whilelabel1);
		return;
	}
	else if (EQ(op,"ret")){
		if (node->son){
			Variable* expr=scanEXPR(node->son);
			if (indent) fprintf(stdout,"	");
			fprintf(stdout,"return %s\n",expr->yoreName);
			return;
		}
		else{
			if (indent) fprintf(stdout,"	");
			fprintf(stdout,"return\n");
			return;
		}
	}
	else if (EQ(op,"lval=")){
		Variable* expr=scanEXPR(node->son->sib);
		if (indent) fprintf(stdout,"	");
		Variable* lval=scanEXPR(node->son);
		fprintf(stdout, "%s = %s\n",lval->yoreName,expr->yoreName);
		return;
	}
}

//finished?
void scanFDEF(Node* node) {
	if (indent) {
		fprintf(stderr, "Line %d: form error\n", node->linenum);
		exit(1);
	}
	node = node->son;
	const char* funcName = node->attr;
	addFunc(funcName, node, true);
	fprintf(stdout, "f_%s [%d]\n", funcName, ParamCnt);
	indent = true;
	while (node->sib && node->sib->type == _expr) 
		node = node->sib;
	while (node->sib){
		node = node->sib;
		scanNode(node);
	}
	indent = false;
	fprintf(stdout, "end f_%s\n", funcName);
	return;
}

//数组部分还没写
void scanVDEF(Node* node) {
	if (EQ(node->attr, "int=")) {
		Variable* newnode = makeVar(node->son->attr, false, _vari, node->linenum, 0,NULL);
		Variable* expr = scanEXPR(node->son->sib);
		if (indent) fprintf(stdout, "	");
		fprintf(stdout, "%s = %s\n", newnode->yoreName, expr->yoreName);
		return;
	}
	else if (EQ(node->attr, "int")) {
		Variable* newnode = makeVar(node->son->attr, false, _vari, node->linenum, 0, NULL);
		return;
	}
	//先不管数组的事
	/*else if (EQ(node->attr, "int[]")) {
		Node* thisnode = node->son;
		node = node->son;
		int dim = 0;
		int buffer[10];
		while (node->sib != NULL && node->sib->sib != NULL) {
			
		}
		return;
	}
	else {

	}*/
}

//finished？
void scanVDCL(Node* node) {
	node = node->son;
	while (node) {
		scanVDEF(node);
		node = node->sib;
	}
}

//有关数组的地方还没有管
void scanCDEF(Node* node){
	if (EQ(node->attr,"int")){
		int* arraysize=malloc(sizeof(int));
		arraysize[0]=0;
		Variable* newnode=makeVar(node->son->attr,false,_vari,node->linenum,0,arraysize);
		Variable* expr=scanEXPR(node->son->sib);
		if (indent) fprintf(stdout,"	");
		fprintf(stdout, "%s = %s\n",newnode->yoreName,expr->yoreName);
		return;
	}
	else{
		//先不管数组的事
		return;
	}	
}

//finished？
void scanCDCL(Node* node){
	node=node->son;	
	while(node){
		scanCDEF(node);
		node=node->sib;
	}
}

//finished？
void scanNode(Node* node){
	switch(node->type){
		case _vdcl: scanVDCL(node); break;
		//case _fdcl: scanFDCL(node); break;
		case _cdcl: scanCDCL(node); break;
		case _vdef: scanVDEF(node); break;
		case _fdef: scanFDEF(node); break;
		case _cdef: scanCDEF(node);	break;
		case _stat: scanSTAT(node);	break;
		case _expr:
		case _iden:
		case _imme: scanEXPR(node); break;
		default:
			fprintf(stderr,"Line %d: statement error: %s %d\n",node->linenum,node->attr,node->type);
			exit(1);
	}
}

//finished？
void scanTree(Node* root){
	initEnv();
	while (root){
		scanNode(root);
		root=root->sib;
	}
}

/*
关于数组的地方有几个不太明白的地方：
	1，定义数组的时候，维度是exp/constexp，为了构建数组我应该是需要得到它们最后算出来的具体的值吧？那这个值怎么得出来呢？还是说在测试实例里面维度都是直接的数字不会是表达式？
	2，不太明白InitVal那一块儿对于数组元素的初始化，那些{}符号是怎么回事？？
	3，对于多维数组的表示，比如说x[1][2][3],这在eeyore语言中是怎么表示的呢？？
	4，传形参的时候，x[][2][3]在eeyore里面是怎么表示的呢？

目前我对数组定义的处理：
	Variable的 int arraysize改为int *arraysize,加上一个int dim来记录数组维度
*/
