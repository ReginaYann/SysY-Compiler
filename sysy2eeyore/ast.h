#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum _node_type{
	_resv=0,	//reserved node
	_vdcl=1,	//variable declare
	_fdcl=2,	//function declare
	_cdcl=3,	//const variable declare
	_vdef=4,	//variable define
	_fdef=5,	//function define
	_cdef=6,	//const variable define
	_stat=7,	//statement
	//_bloc=8,	//block
	_expr=9,	//expression
	_iden=10,	//identifier
	_imme=11,	//immediate number
};

enum _var_type{
	_vari=0,	//vatiable
	_para=1,	//parameter
	_temp=2,	//temporary use
	_inst=3,	//immediate number
};


struct AstNode;
typedef struct AstNode{
	enum _node_type type;
	struct AstNode* son;
	struct AstNode* sib;
	const char* attr;
	int linenum; 
}Node;

typedef struct{
	enum _var_type type;
	char* oriName;
	char* yoreName;
	int isarray;
	int isconst;
	int valid;		//check if valid in this environment
	int linenum;
	int dim;
	int* arraysize;
	int totalsize;
}Variable;

struct PARAM;
typedef struct PARAM{
	int isarray;
	int dim;
	int* arraysize;
	struct PARAM* next;
}parameter;

typedef struct{
	char* funcName;
	int linenum;
	int declared;
	int isreturn;
	parameter* first;
}Func;


Node* makeNode(const char*,Node*,enum _node_type);
Node* rightMost(Node*);
void scanTree(Node* root,FILE* fd);
void printTree(Node*);

static int lineno;

#define YYSTYPE Node*
