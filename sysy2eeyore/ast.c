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
Variable* VarList[5000];
Variable* ParamList[50];
Func* FuncList[1000];
int LayerMark[100];
int currLayer;
int indent;
static int whilelabel1,whilelabel2; //��break��continue��ʱ������õ���������¼���һ��while�ı�ǣ�Ȼ��goto������
char code[5000][50];
int codeline=0;

//���⣺Ҫ�ѽ��print������ȥ��Ŀǰ�ǣ�sprintf(code)

void scanNode(Node* node);
Variable* scanEXPR(Node* node);
int scanInit(Node* node, int curlayer, int* arraysize, int dim, int pointer, char** initv);
int getval(Node* node);

void initEnv(){
	VarCnt=TempCnt=ParamCnt=0;
	LabelCnt=FuncCnt=0;
	currLayer=indent=0;
	LayerMark[0]=0;
	codeline=0;
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
	//newVar->oriName=(oriname==NULL?NULL:strdup(oriname));
	if (oriname == NULL)
		newVar->oriName = NULL;
	else{
		newVar->oriName=malloc(sizeof(char)*strlen(oriname));
		strcpy(newVar->oriName, oriname);
	}
	newVar->dim=dim;
	newVar->arraysize=arraysize;
	newVar->totalsize = 0;
	if (type==_inst){
		if (isarray||oriname==NULL){
			fprintf(stderr,"Line %d: type error: %s\n",linenum,oriname);
			exit(1);
		}
		//newVar->yoreName=strdup(oriname);
		newVar->yoreName=malloc(sizeof(char)*strlen(oriname));
		strcpy(newVar->yoreName, oriname);
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
		//newVar->yoreName=strdup(buffer);
		newVar->yoreName=malloc(sizeof(char)*strlen(buffer));
		strcpy(newVar->yoreName, buffer);
		int index = 0;
		int mul = 1;
		for (index = 0; index < dim; index++)
			mul = mul * arraysize[index];
		newVar->totalsize = mul;
		if (type!=_para){
			if (indent)
				sprintf(code[codeline++],"	");
			if (!isarray)
				sprintf(code[codeline++],"var %s\n",newVar->yoreName);
			else{
				sprintf(code[codeline++],"var %d %s\n",4*mul,newVar->yoreName);
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
				/*if (isarray != unknown && isarray != ParamList[i]->isarray){
					sprintf(stderr,"Line %d: type error: %s\n",linenum,oriName);
					exit(1);
				}*/
				return ParamList[i];
			}
		for (i=VarCnt-1;i>=0;--i)
			if(EQ(VarList[i]->oriName,oriName)&&VarList[i]->valid){
				/*if (isarray != unknown && isarray != VarList[i]->isarray){
					sprintf(stderr,"Line %d: type error: %s\n",linenum,oriName);
					exit(1);
				}*/
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

//������ʵû��check��ֻ����Ĭ�ϴ������Ĳ������ǺϷ��ģ���ʵ��Ҫ���þ��Ǵ�ӡparam
void checkParameter(Node* node, parameter* param, const char* funcName, int undefined)
{
	//while (node && (undefined || param))
	while (node)
	{
		//Variable* temp = scanEXPR(node);
		/*if ((!undefined) && temp->isarray != param->isarray)
		{
			sprintf(stderr, "Line %d: function parameter type error: %s(...)\n",
				node->linenum, funcName);
			exit(1);
		}*/
		/*if ((!undefined) && (temp->isarray && param->arraysize && param->arraysize != temp->arraysize))
		{
			sprintf(stderr, "Line %d: function parameter type error: %s(...)\n",
				node->linenum, funcName);
			exit(1);
		}*/
		if (node->type == _iden) {
			if (indent) sprintf(code[codeline++], "	");
			Variable* temp = matchVar(node->attr, true, _vari, node->linenum);
			sprintf(code[codeline++], "param %s\n", temp->yoreName);
		}
		else {
			/*Variable* id = matchVar(node->son->attr, true, _vari, node->linenum);
			int totalsize = id->totalsize;
			int offline = 0;
			int* arraysize = id->arraysize;
			int tmpline = id->linenum;
			node = node->son;
			int cnt = 0;
			while (node->sib) {
				node = node->sib;
				totalsize = totalsize / arraysize[cnt];
				offline = offline + atoi(node->attr)*totalsize;
				cnt++;
			}
			offline *= 4;
			Variable* temp = makeVar(NULL, false, _temp, tmpline, 0, NULL);
			if (indent) sprintf(code[codeline++], "	");
			sprintf(code[codeline++], "%s = %s + %d\n", temp->yoreName, id->yoreName, offline);*/
			Variable* temp=scanEXPR(node);
			if (indent) sprintf(code[codeline++], "	");
			sprintf(code[codeline++], "param %s\n", temp->yoreName);
		}
		node = node->sib;
		//if (!undefined) param = param->next;
	}
	/*if ((!undefined) && (node && node->type == _expr) || param)
	{
		sprintf(stderr, "Line %d: function parameter more or less: %s\n", node->linenum, funcName);
		exit(1);
	}*/
}

//��parameter�ڵ�ĳ���expr��ģ�attr��param��param[]���
void addFunc(const char* funcName, Node* node, int declaring,int isreturn)
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
		//newFunc->funcName = strdup(funcName);
		newFunc->funcName=malloc(sizeof(char)*strlen(funcName));
		strcpy(newFunc->funcName, funcName);
		newFunc->linenum = node->linenum;
		newFunc->declared = false;
		newFunc->isreturn = isreturn;
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

//��û��дinit{}
Variable* scanEXPR(Node* node){
	if (node->type==_imme)
		return makeVar(node->attr,false,_inst,node->linenum,0,NULL);
	else if (node->type==_iden)
		return matchVar(node->attr,false,_vari,node->linenum);
	else{
		//for lval
		if (EQ(node->attr,"lint[]")||EQ(node->attr,"rint[]")){
			Variable* id=matchVar(node->son->attr,true,_vari,node->linenum);
			int* arraysize = id->arraysize;
			int tmpline = node->linenum;
			Node* indexnode=node->son->sib;
			int cnt = 1;
			Variable* index=scanEXPR(indexnode);
			Variable* index4=makeVar(NULL,false,_temp,tmpline,0,NULL);
			if (indent) sprintf(code[codeline++], "	");
			sprintf(code[codeline++], "%s = %s\n", index4->yoreName, index->yoreName);
			for (cnt=1;cnt<id->dim;cnt++){
				if (indent) sprintf(code[codeline++], "	");
				sprintf(code[codeline++], "%s = %s * %d\n", index4->yoreName, index4->yoreName, arraysize[cnt]);
				if (indexnode->sib==NULL)
					break;
				indexnode=indexnode->sib;
				index=scanEXPR(indexnode);
				if (indent) sprintf(code[codeline++], "	");
				sprintf(code[codeline++], "%s = %s + %s\n", index4->yoreName, index4->yoreName, index->yoreName);
				//cnt++;
			}
			if (indent) sprintf(code[codeline++], "	");
			sprintf(code[codeline++], "%s = %s * 4\n", index4->yoreName, index4->yoreName);
			if (EQ(node->attr, "lint[]")) {
				char buffer[20] = { '\0' };
				sprintf(buffer, "%s [%s]", id->yoreName, index4->yoreName);
				Variable* temp = makeVar(buffer, false, _temp, tmpline, 0, NULL);
				return temp;
			}
			if (EQ(node->attr, "rint[]")) {
				Variable* temp = makeVar(NULL, false, _temp, tmpline, 0, NULL);
				if (indent) sprintf(code[codeline++], "	");
				sprintf(code[codeline++], "%s = %s [%s]\n", temp->yoreName, id->yoreName, index4->yoreName);
				return temp;
			}
		}
		else if (EQ(node->attr,"call")){
			const char* funcName = node->son->attr;
			int i;
			for (i = 0; i < FuncCnt; i++) {
				if (EQ(funcName, FuncList[i]->funcName)) 
					break;
			}
			parameter* tmppara = malloc(sizeof(parameter));
			if (i < FuncCnt)
				checkParameter(node->son->sib, FuncList[i]->first, funcName, 0);
			else
				//�������ʹ�õ������Լ��ȶ���õĿ⺯��
				checkParameter(node->son->sib, NULL, funcName, 0);
			//�ӵ�һ��������ʼ����
			//checkParameter(node->son->sib, FuncList[i]->first, funcName, 0);
			if (EQ(funcName, "getint") || EQ(funcName, "getch") || EQ(funcName, "getarray") || ((i < FuncCnt) && (FuncList[i]->isreturn == true))) {
				Variable* temp = makeVar(NULL, false, _temp, node->linenum, 0, NULL);
				if (indent) sprintf(code[codeline++],"	");
				sprintf(code[codeline++], "%s = call f_%s\n", temp->yoreName, funcName);
				return temp;
			}
			else {
				if (indent) sprintf(code[codeline++],"	");
				sprintf(code[codeline++], "call f_%s\n", funcName);
				return NULL;
			}

			/*checkParameter(node->son->sib, NULL, funcName, 1);
			Variable* temp = makeVar(NULL, false, _temp, node->linenum, 0);
			if (indent) sprintf(code, "	");
			sprintf(code, "%s = call f_%s\n", temp->yoreName, funcName);
			return temp;*/
		}
		else if (EQ(node->attr,"pos") || EQ(node->attr,"neg") ||EQ(node->attr,"not") ){
			const char* op=node->attr;
			Variable *operand =scanEXPR(node->son);
			Variable *temp=makeVar(NULL,false,_temp,node->linenum,0,NULL);
			if (indent)	sprintf(code[codeline++],"	");
			if (EQ(op,"neg"))
				sprintf(code[codeline++],"%s = -%s\n",temp->yoreName,operand->yoreName);
			else if (EQ(op,"not"))
				sprintf(code[codeline++],"%s = !%s\n",temp->yoreName,operand->yoreName);
			else
				sprintf(code[codeline++],"%s = %s\n",temp->yoreName,operand->yoreName);
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
		else if (EQ(node->attr, "param[]")) {
			if (node->son->sib == NULL) {
				int* arraysize = malloc(sizeof(int));
				arraysize[0] = 1;
				return makeVar(node->son->attr, true, _para, node->linenum, 1, arraysize);
			}
			else {
				const char* paramname = node->son->attr;
				int paraline = node->linenum;
				node = node->son;
				int dims = 1;
				int buffer[10];
				buffer[0] = 1;
				while (node->sib) {
					buffer[dims++] = atoi(node->sib->attr);
					node = node->sib;
				}
				int* arraysize = malloc(sizeof(int)*dims);
				int i;
				for (i = 0; i < dims; i++)
					arraysize[i] = buffer[i];
				return makeVar(paramname, true, _para, paraline, dims, arraysize);
			}
		}
		else{
			const char* op=node->attr;
			Variable* operand1=scanEXPR(node->son);
			Variable* operand2=scanEXPR(node->son->sib);
			Variable* temp=makeVar(NULL,false,_temp,node->linenum,0,NULL);
			char buffer[20]={'\0'};
			sprintf(buffer,"%s = %s %s %s",temp->yoreName,operand1->yoreName,op,operand2->yoreName);
			if (indent)	sprintf(code[codeline++],"	");
			sprintf(code[codeline++],"%s\n",buffer);
			return temp;
		}
	}
}

//finished����
void scanSTAT(Node* node){
	const char* op=node->attr;
	if (EQ(op, "{}")) {
		LayerMark[++currLayer] = VarCnt;
		node = node->son;
		while (node)
		{
			scanNode(node);
			node = node->sib;
		}
		int i;
		for (i = LayerMark[currLayer--]; i < VarCnt; ++i)
			VarList[i]->valid = false;
		return;
	}
	else if (EQ(op,"semi")){
		//do nothing
		return;
	}
	else if (EQ(op,"if")){
		int label=LabelCnt++;
		if (EQ(node->son->attr,"&&")){
			Variable* cond1=scanEXPR(node->son->son);
			if (indent) sprintf(code[codeline++],"	");
			sprintf(code[codeline++],"if %s == 0 goto l%d\n",cond1->yoreName,label);
			Variable* cond2=scanEXPR(node->son->son->sib);
			if (indent) sprintf(code[codeline++],"	");
			sprintf(code[codeline++],"if %s == 0 goto l%d\n",cond2->yoreName,label);
		}
		else{
			Variable* cond=scanEXPR(node->son);
			if (indent) sprintf(code[codeline++],"	");
			sprintf(code[codeline++],"if %s == 0 goto l%d\n",cond->yoreName,label);
		}
		scanNode(node->son->sib);
		sprintf(code[codeline++],"l%d:\n",label);
		return;
	}
	else if (EQ(op,"ifelse")){
		int label1=LabelCnt++;
		int label2=LabelCnt++;
		if (EQ(node->son->attr,"&&")){
			Variable* cond1=scanEXPR(node->son->son);
			if (indent) sprintf(code[codeline++],"	");
			sprintf(code[codeline++],"if %s == 0 goto l%d\n",cond1->yoreName,label1);
			Variable* cond2=scanEXPR(node->son->son->sib);
			if (indent) sprintf(code[codeline++],"	");
			sprintf(code[codeline++],"if %s == 0 goto l%d\n",cond2->yoreName,label1);
		}
		else{
			Variable* cond=scanEXPR(node->son);
			if (indent) sprintf(code[codeline++],"	");
			sprintf(code[codeline++],"if %s == 0 goto l%d\n",cond->yoreName,label1);
		}
		//Variable* cond=scanEXPR(node->son);
		//if (indent) sprintf(code[codeline++], "	");
		//sprintf(code[codeline++],"if %s == 0 goto l%d\n",cond->yoreName,label1);
		scanNode(node->son->sib);
		if (indent) sprintf(code[codeline++],"	");
		sprintf(code[codeline++],"goto l%d\n",label2);
		sprintf(code[codeline++],"l%d:\n",label1);
		scanNode(node->son->sib->sib);
		sprintf(code[codeline++],"l%d:\n",label2);
		return;
	}
	else if (EQ(op,"while")){
		int label1=LabelCnt++;
		int label2=LabelCnt++;
		whilelabel1=label1;
		whilelabel2=label2;
		sprintf(code[codeline++],"l%d:\n",label1);
		if (EQ(node->son->attr,"&&")){
			Variable* cond1=scanEXPR(node->son->son);
			if (indent) sprintf(code[codeline++],"	");
			sprintf(code[codeline++],"if %s == 0 goto l%d\n",cond1->yoreName,label2);
			Variable* cond2=scanEXPR(node->son->son->sib);
			if (indent) sprintf(code[codeline++],"	");
			sprintf(code[codeline++],"if %s == 0 goto l%d\n",cond2->yoreName,label2);
		}
		else{
			Variable* cond=scanEXPR(node->son);
			if (indent) sprintf(code[codeline++],"	");
			sprintf(code[codeline++],"if %s == 0 goto l%d\n",cond->yoreName,label2);
		}
		scanNode(node->son->sib);
		if (indent) sprintf(code[codeline++],"	");
		sprintf(code[codeline++],"goto l%d\n",label1);
		sprintf(code[codeline++],"l%d:\n",label2);
		return;
	}
	else if (EQ(op,"break")){
		if (indent) sprintf(code[codeline++],"	");
		sprintf(code[codeline++],"goto l%d\n",whilelabel2);
		return;
	}
	else if (EQ(op,"continue")){
		if (indent) sprintf(code[codeline++],"	");
		sprintf(code[codeline++],"goto l%d\n",whilelabel1);
		return;
	}
	else if (EQ(op,"ret")){
		if (node->son){
			Variable* expr=scanEXPR(node->son);
			if (indent) sprintf(code[codeline++],"	");
			sprintf(code[codeline++],"return %s\n",expr->yoreName);
			return;
		}
		else{
			if (indent) sprintf(code[codeline++],"	");
			sprintf(code[codeline++],"return\n");
			return;
		}
	}
	else if (EQ(op,"lval=")){
		Variable* expr=scanEXPR(node->son->sib);
		Variable* lval=scanEXPR(node->son);
		if (indent) sprintf(code[codeline++], "	");
		if (node->son->type==_iden)
			sprintf(code[codeline++], "%s = %s\n",lval->yoreName,expr->yoreName);
		else
			sprintf(code[codeline++], "%s = %s\n",lval->oriName,expr->yoreName);
		return;
	}
}

//finished?
void scanFDEF(Node* node) {
	if (indent) {
		fprintf(stderr, "Line %d: form error\n", node->linenum);
		exit(1);
	}
	int isreturn;
	if (EQ(node->attr, "func="))
		isreturn = true;
	else
		isreturn = false;
	node = node->son;
	const char* funcName = node->attr;
	//�����node���Ѿ���paramter��node��
	addFunc(funcName, node, true, isreturn);
	sprintf(code[codeline++], "f_%s [%d]\n", funcName, ParamCnt);
	indent = true;
	while (node->sib && node->sib->type == _expr) 
		node = node->sib;
	while (node->sib){
		node = node->sib;
		scanNode(node);
	}
	indent = false;
	sprintf(code[codeline++], "end f_%s\n", funcName);
	return;
}

//���鲿�ֻ�û��ȫŪ��
void scanVDEF(Node* node) {
	if (EQ(node->attr, "int=")) {
		Variable* newnode = makeVar(node->son->attr, false, _vari, node->linenum, 0,NULL);
		Variable* expr = scanEXPR(node->son->sib);
		if (indent) sprintf(code[codeline++], "	");
		sprintf(code[codeline++], "%s = %s\n", newnode->yoreName, expr->yoreName);
		return;
	}
	else if (EQ(node->attr, "int")) {
		Variable* newnode = makeVar(node->son->attr, false, _vari, node->linenum, 0, NULL);
		return;
	}
	//ֻ�Ƕ��������飬û�и���ֵ
	else if (EQ(node->attr, "int[]")) {
		int dimcnt = 0;
		const char* varname = node->son->attr;
		node = node->son;
		int dimRecord[10];
		while (node->sib) {
			dimRecord[dimcnt++] = atoi(node->sib->attr);
			node = node->sib;
		}
		int* arraysize = malloc(sizeof(int)*dimcnt);
		int i;
		for (i = 0; i < dimcnt; i++)
			arraysize[i] = dimRecord[i];
		Variable* newnode = makeVar(varname, true, _vari, node->linenum, dimcnt, arraysize);
		return;
	}
	else {
		//����������ͱ�����������Ӧ�ø�������һ���ģ�����Ҫ��ôȥ�����Ǹ���ֵȻ��һ����������أ���ͷ��
		int dimcnt = 0;
		const char* varname=node->son->attr;
		node = node->son;
		int dimRecord[10];
		//ֻҪ�������һ���ڵ㣬���һ���ڵ���initval
		while (node->sib && node->sib->sib) {
			dimRecord[dimcnt++] = atoi(node->sib->attr);
			node = node->sib;
		}
		int* arraysize = malloc(sizeof(int)*dimcnt);
		int i;
		for (i = 0; i < dimcnt; i++)
			arraysize[i] = dimRecord[i];
		Variable* newnode = makeVar(varname, true, _vari, node->linenum, dimcnt, arraysize);
		int totalsize = newnode->totalsize;
		//sprintf(code[codeline++],"totalsize=%d\n",totalsize);
		char** initv = malloc(sizeof(char*)*totalsize);
		node = node->sib;
		int ret = scanInit(node,0,arraysize,dimcnt,0,initv);
		for (i = 0; i < totalsize; i++) {
			if (indent) sprintf(code[codeline++],"	");
			sprintf(code[codeline++], "%s [%d] = %s\n", newnode->yoreName, i * 4, initv[i]);
		}
		return;
	}
}

//finished��
void scanVDCL(Node* node) {
	node = node->son;
	while (node) {
		scanVDEF(node);
		node = node->sib;
	}
}

//�й�����ĵط���û��д��
//const define
void scanCDEF(Node* node){
	//int= ��int������
	if (EQ(node->attr,"int=")){
		//int* arraysize=malloc(sizeof(int));
		//arraysize[0]=0;
		Variable* newnode=makeVar(node->son->attr,false,_vari,node->linenum,0,NULL);
		Variable* expr=scanEXPR(node->son->sib);
		if (indent) sprintf(code[codeline++],"	");
		sprintf(code[codeline++], "%s = %s\n",newnode->yoreName,expr->yoreName);
		return;
	}
	//int[]= : ��������
	else{
		int dimcnt = 0;
		const char* varname=node->son->attr;
		node = node->son;
		int dimRecord[10];
		//ֻҪ�������һ���ڵ㣬���һ���ڵ���initval
		while (node->sib && node->sib->sib) {
			if (node->sib->type==_imme)			
				dimRecord[dimcnt++] = atoi(node->sib->attr);
			else 
				dimRecord[dimcnt++]=getval(node->sib);
			node = node->sib;
		}
		int* arraysize = malloc(sizeof(int)*dimcnt);
		int i;
		for (i = 0; i < dimcnt; i++)
			arraysize[i] = dimRecord[i];
		Variable* newnode = makeVar(varname, true, _vari, node->linenum, dimcnt, arraysize);
		int totalsize = newnode->totalsize;
		//sprintf(code[codeline++],"totalsize=%d\n",totalsize);
		char** initv = malloc(sizeof(char*)*totalsize);
		node = node->sib;
		int ret = scanInit(node,0,arraysize,dimcnt,0,initv);
		for (i = 0; i < totalsize; i++) {
			if (indent) sprintf(code[codeline++],"	");
			sprintf(code[codeline++], "%s [%d] = %s\n", newnode->yoreName, i * 4, initv[i]);
		}
		return;
	}	
}

int scanInit(Node* node, int curlayer, int* arraysize, int dim, int pointer,char** initv) {
	if (EQ(node->attr,"init")) {
		Variable* temp = scanEXPR(node->son);
		initv[pointer] = malloc(sizeof(char)*strlen(temp->yoreName));
		strcpy(initv[pointer], temp->yoreName);
		return 1;
	}
	else if (EQ(node->attr, "empty{}")) {
		int need = 1;
		int i;
		for (i = curlayer; i < dim; i++)
			need *= arraysize[i];
		for (i = pointer; i < pointer + need; i++) {
			initv[i] = malloc(sizeof(char)*2);
			strcpy(initv[i], "0");
		}
		return need;
	}
	else {
		int need = 1;
		int i;
		int res = 0;
		for (i = curlayer; i < dim; i++)
			need *= arraysize[i];
		int r=need;
		Node* newnode = node->son;
		res = scanInit(newnode, curlayer + 1, arraysize, dim, pointer, initv);
		need -= res;
		pointer += res;
		while (newnode->sib) {
			newnode=newnode->sib;
			res = scanInit(newnode, curlayer + 1, arraysize, dim, pointer, initv);
			need -= res;
			pointer += res;
		}
		if (need != 0) {
			int j;
			for (j = pointer; j < pointer+need; j++) {
				initv[j] = malloc(sizeof(char)*2);
				strcpy(initv[j], "0");
			}
		}
		return r;
	}
}

//finished��
void scanCDCL(Node* node){
	node=node->son;	
	while(node){
		scanCDEF(node);
		node=node->sib;
	}
}

//finished
void scanNode(Node* node) {
	switch (node->type) {
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
		fprintf(stderr, "Line %d: statement error: %s %d\n", node->linenum, node->attr, node->type);
		exit(1);
	}
}

void changeseq();
//finished
void scanTree(Node* root, FILE* fd) {
	initEnv();
	//predefine();
	while (root) {
		scanNode(root);
		root = root->sib;
	}
	changeseq();
	rewind(fd);
	int i = 0;
	while (i < codeline) {
		fprintf(fd, "%s", code[i]);
		i++;
	}
}

/*
��������ĵط��м�����̫���׵ĵط���
	1�����������ʱ��ά����exp/constexp��Ϊ�˹���������Ӧ������Ҫ�õ��������������ľ����ֵ�ɣ������ֵ��ô�ó����أ�����˵�ڲ���ʵ������ά�ȶ���ֱ�ӵ����ֲ����Ǳ��ʽ��
	2����̫����InitVal��һ�����������Ԫ�صĳ�ʼ������Щ{}��������ô���£���
	3�����ڶ�ά����ı�ʾ������˵x[1][2][3],����eeyore����������ô��ʾ���أ���
	4�����βε�ʱ��x[][2][3]��eeyore��������ô��ʾ���أ�

Ŀǰ�Ҷ����鶨��Ĵ���
	Variable�� int arraysize��Ϊint *arraysize,����һ��int dim����¼����ά��
*/

/*
������ЩҪ����
	1��Initval��scanCDEF��scanVDEF���桰int[]=��������Ҫ�����꣬Ȼ��scanEXPR�������init���ͽڵ�ҲҪ������
	2��scanFDEF�����βεĲ��֣����ǲ�����Ҫ����ʲô
	3��lval��scanExpr���int[]��û�ĺ�
ʣ�µ�Ӧ���ǶԵİɣ�ʵ���벻�����������ô��������ȶ�һ������Ĳ���
*/


void changeseq() {
	int varpointer[20];
	int brac = 0;
	char record[50];
	char tab[50];
	varpointer[0] = 0;
	int i;
	for (i = 0; i < codeline; i++) {
		if (strlen(code[i]) >= 3 && code[i][0] == 'v'&&code[i][1] == 'a'&&code[i][2] == 'r') {
			if ((i > 0 && !EQ(code[i - 1], "	")) || i == 0) {
				if (i != varpointer[brac]) {
					//record=strdup(code[i]);
					strcpy(record, code[i]);
					int j;
					for (j = i - 1; j >= varpointer[brac]; j--)
						//code[j+1]=strdup(code[j]);
						strcpy(code[j + 1], code[j]);
					//code[varpointer[brac]]=strdup(record);
					strcpy(code[varpointer[brac]], record);
				}
				varpointer[brac]++;
			}
			else {
				if (i != varpointer[brac]) {
					//record=strdup(code[i]);
					strcpy(record, code[i]);
					strcpy(tab, code[i - 1]);
					int j;
					for (j = i - 2; j >= varpointer[brac]; j--)
						//code[j+1]=strdup(code[j]);
						strcpy(code[j + 2], code[j]);
					//code[varpointer[brac]]=strdup(record);
					strcpy(code[varpointer[brac] + 1], record);
					strcpy(code[varpointer[brac]], tab);
				}
				varpointer[brac] += 2;
			}
		}
		/*else if (strlen(code[i])>=4 && code[i][1]=='v'&&code[i][2]=='a'&&code[i][3]=='r'){
			if (i!=varpointer[brac]){
				//record=strdup(code[i]);
				strcpy(record,code[i]);
				strcpy(tab,code[i-1]);
				int j;
				for (j=i-2;j>=varpointer[brac];j--)
					//code[j+1]=strdup(code[j]);
					strcpy(code[j+2],code[j]);
				//code[varpointer[brac]]=strdup(record);
				strcpy(code[varpointer[brac]+1],record);
				strcpy(code[varpointer[brac]],tab);
			}
			varpointer[brac]+=2;
		}*/
		else if (strlen(code[i]) >= 3 && code[i][0] == 'f'&&code[i][1] == '_')
			varpointer[++brac] = i + 1;
		else if (strlen(code[i]) >= 3 && code[i][0] == 'e'&&code[i][1] == 'n'&&code[i][2] == 'd')
			brac--;
	}
}


int getval(Node* node){
	if (node->type==_imme)
		return atoi(node->attr);
	const char* op=node->attr;
	if (EQ(op,"+")){
		return getval(node->son)+getval(node->son->sib);
	}
	else if (EQ(op,"-")){
		return getval(node->son)-getval(node->son->sib);
	} 
	else if (EQ(op,"*")){
		return getval(node->son)*getval(node->son->sib);
	}
	else if (EQ(op,"/")){
		return getval(node->son)/getval(node->son->sib);
	}
	else
		return 0;
}
