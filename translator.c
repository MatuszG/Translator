#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "translator.h"
#include "itoa.h"

struct Items {
	char* item;
	struct Items* next;
};

struct Items *Head = NULL, *Wsk = NULL;		/* elements of a list */
char type = '\0';							/* type of a variable */
extern int LineCount;						/* line counter from file */
sem_rec latter[10];							/* latter labels for nested while */
int counter = 0;							/* counter for while (max. 10 nested while) */
FILE* src3AC;								/* handle to file with 3AC code */
int tempnum;								/* counter for temp variables */
int labelnum;								/* counter for labels */

void emit1(char* str)
{
    fprintf(src3AC, "\t%s\n", str);
}

void emit2(char* str1, char* str2)
{
    fprintf(src3AC, "\t%s %s\n", str1, str2);
}

void emit3(char* str1, char* str2, char* str3)
{
    fprintf(src3AC, "\t%s %s %s\n", str1, str2, str3);
}

void emit4(char* str1, char* str2, char* str3, char* str4)
{
    fprintf(src3AC, "\t%s %s %s %s %s\n", str1, str2, str3, str4);
}

void emit5(char* str1, char* str2, char* str3, char* str4, char* str5)
{
    fprintf(src3AC, "\t%s %s %s %s %s\n", str1, str2, str3, str4, str5);
}

void SemanticError(const char* message, const char* name) {//
	DrawArrow();
	printf("###### Semantic Error: %s - %s\n", message, name);
	printf("###### Semantic Error: line %d or above\n", LineCount);
	if (strstr(message, "different") != NULL) {
		printf("See a symbol table below:\n");
		ListSymbolTable();
	}
	exit(1);
}

void SemanticWarning(const char* message, const char* name) {//
	DrawArrow();
	printf("###### Semantic Warning: %s - %s\n", message, name);
	printf("###### Semantic Warning: line %d or above\n", LineCount);
}

void gettemp(char* tempname) {
	IdString s;

	itoa_(++tempnum, s);
	strcpy(tempname, "temp");
	strcat(tempname, s);
	Enter(tempname, Id, "");
}

void getlabel(char* labelname) {
	IdString s;

	itoa_(++labelnum, s);
	strcpy(labelname, "Label");
	strcat(labelname, s);
}

void GenInfix(sem_rec expr1, sem_rec op, sem_rec expr2, sem_rec* res) {
	sem_rec expr0;
	IdString temp, type, type2;

	gettemp(temp);
	strcpy(expr0.Lexeme, temp);
	if (isdigit(expr1.Lexeme[0])) {
		if (strstr(expr1.Lexeme, ".") != NULL) strcpy(type, "real");
		else strcpy(type, "integer");
	}
	else {
		IdVal = LookUp(expr1.Lexeme);
		HashItem* head = SymbolTable[IdVal];
		while (strcmp(head->Name, expr1.Lexeme) != 0) head = head->Next;
		if (strcmp(head->Type, "") == 0) SemanticError("variable not declared", head->Name);
		else if (strcmp(head->Type, "ProgramName") == 0 || strcmp(head->Type, "keyword") == 0)
			SemanticError("cannot be used as a variable", head->Name);
		strcpy(type, head->Type);
	}
	if (isdigit(expr2.Lexeme[0])) {
		if (strstr(expr2.Lexeme, ".") != NULL) strcpy(type2, "real");
		else strcpy(type2, "integer");
	}
	else {
		IdVal = LookUp(expr2.Lexeme);
		HashItem* head = SymbolTable[IdVal];
		while (strcmp(head->Name, expr2.Lexeme) != 0) head = head->Next;
		if (strcmp(head->Type, "") == 0) SemanticError("variable not declared", head->Name);
		else if (strcmp(head->Type, "ProgramName") == 0 || strcmp(head->Type, "keyword") == 0)
			SemanticError("cannot be used as a variable", head->Name);
		strcpy(type2, head->Type);
	}
	if (strcmp(type, type2) != 0) {
		IdString message;
		strcpy(message, expr1.Lexeme);
		strcat(message, " and ");
		strcat(message, expr2.Lexeme);
		SemanticError("different types of variables", message);
	}
	if (strcmp(temp, "")) {
		IdVal = LookUp(temp);
		HashItem* head = SymbolTable[IdVal];
		while (strcmp(head->Name, temp) != 0) head = head->Next;
		head->Type = (char*)malloc(strlen(temp) + 1);
		strcpy(head->Type, type);
	}
	emit5(expr0.Lexeme, ":=", expr1.Lexeme, op.Lexeme, expr2.Lexeme);
	*res = expr0;
}

void AssignProc(sem_rec* term, sem_rec* expr) {
	IdVal = LookUp(term->Lexeme);
	HashItem* head = SymbolTable[IdVal];
	while (strcmp(head->Name, term->Lexeme) != 0) head = head->Next;
	if (strcmp(head->Type, "") == 0) SemanticError("variable not declared", head->Name);
	else if (strcmp(head->Type, "ProgramName") == 0 || strcmp(head->Type, "keyword") == 0)
		SemanticError("cannot be used as a variable", head->Name);
	if (isdigit(expr->Lexeme[0])) {
		IdString type;
		if (strstr(expr->Lexeme, ".") != NULL) strcpy(type, "real");
		else strcpy(type, "integer");
		if (strcmp(head->Type, type) != 0) {
			IdString message;
			strcpy(message, head->Name);
			strcat(message, " and ");
			strcat(message, expr->Lexeme);
			SemanticError("different types of variables", message);
		}
	}
	else {
		IdVal = LookUp(expr->Lexeme);
		HashItem* head2 = SymbolTable[IdVal];
		while (strcmp(head2->Name, expr->Lexeme) != 0) head2 = head2->Next;
		if (strcmp(head2->Type, "") == 0) SemanticError("variable not declared", head2->Name);
		else if (strcmp(head2->Type, "ProgramName") == 0 || strcmp(head2->Type, "keyword") == 0)
			SemanticError("cannot be used as a variable", head2->Name);
		if (strcmp(head->Type, head2->Type) != 0) {
			IdString message;
			strcpy(message, head->Name);
			strcat(message, " and ");
			strcat(message, head2->Name);
			SemanticError("different types of variables", message);
		}
	}
	emit3(term->Lexeme, ":=", expr->Lexeme);
}

void ReadProc() {
	Wsk = Head;
	while (Wsk != NULL) {
		IdVal = LookUp(Wsk->item);
		HashItem* head = SymbolTable[IdVal];
		while (strcmp(head->Name, Wsk->item) != 0) head = head->Next;
		if (strcmp(head->Type, "") == 0) SemanticError("variable not declared", head->Name);
		else if (strcmp(head->Type, "ProgramName") == 0 || strcmp(head->Type, "keyword") == 0)
			SemanticError("cannot be used as a variable", head->Name);
		emit2("read", Wsk->item);
		Wsk = Wsk->next;
	}
	Wsk = Head;
	while (Wsk != NULL) {
		Head = Wsk->next;
		free(Wsk->item);
		free(Wsk);
		Wsk = Head;
	}
	Wsk = Head = NULL;
}

void WriteEqualProc(sem_rec* expr) {
	IdVal = LookUp(expr->Lexeme);
	HashItem* head = SymbolTable[IdVal];
	while (strcmp(head->Name, expr->Lexeme) != 0) head = head->Next;
	if (strcmp(head->Type, "") == 0) SemanticError("variable not declared", head->Name);
	else if (strcmp(head->Type, "ProgramName") == 0 || strcmp(head->Type, "keyword") == 0)
		SemanticError("cannot be used as a variable", head->Name);
	emit2("write", expr->Lexeme);
}

void PlusProc(sem_rec* operator) {
	strcpy(operator->Lexeme, "+");
}

void MinusProc(sem_rec* operator) {
	strcpy(operator->Lexeme, "-");
}

void MultiProc(sem_rec* operator) {
	strcpy(operator->Lexeme, "*");
}

void DivProc(sem_rec* operator) {
	strcpy(operator->Lexeme, "/");
}

void IntProc(sem_rec* term) {
	strcpy(term->Lexeme, IntLexeme);
}

void FloatProc(sem_rec* term) {
	strcpy(term->Lexeme, RealLexeme);
}

void IdProc(sem_rec* term) {
	strcpy(term->Lexeme, NameVal);
}

void BooLessProc(sem_rec* exp1, sem_rec* op, sem_rec* exp2, sem_rec* cmd, sem_rec* lab) {
	sem_rec expr0;
	IdString label;
	IdString type, type2;

	if (isdigit(exp1->Lexeme[0])) {
		if (strstr(exp1->Lexeme, ".") != NULL) strcpy(type, "real");
		else strcpy(type, "integer");
	}
	else {
		IdVal = LookUp(exp1->Lexeme);
		HashItem* head = SymbolTable[IdVal];
		while (strcmp(head->Name, exp1->Lexeme) != 0) head = head->Next;
		if (strcmp(head->Type, "") == 0) SemanticError("variable not declared", head->Name);
		else if (strcmp(head->Type, "ProgramName") == 0 || strcmp(head->Type, "keyword") == 0)
			SemanticError("cannot be used as a variable", head->Name);
		strcpy(type, head->Type);
	}
	if (isdigit(exp2->Lexeme[0])) {
		if (strstr(exp2->Lexeme, ".") != NULL) strcpy(type2, "real");
		else strcpy(type2, "integer");
	}
	else {
		IdVal = LookUp(exp2->Lexeme);
		HashItem* head = SymbolTable[IdVal];
		while (strcmp(head->Name, exp2->Lexeme) != 0) head = head->Next;
		if (strcmp(head->Type, "") == 0) SemanticError("variable not declared", head->Name);
		else if (strcmp(head->Type, "ProgramName") == 0 || strcmp(head->Type, "keyword") == 0)
			SemanticError("cannot be used as a variable", head->Name);
		strcpy(type2, head->Type);
	}
	if (strcmp(type, type2) != 0) {
		IdString message;
		strcpy(message, exp1->Lexeme);
		strcat(message, " and ");
		strcat(message, exp2->Lexeme);
		SemanticError("different types of variables", message);
	}
	if (strcmp(cmd->Lexeme, "if") == 0) {
		getlabel(label);
		strcpy(expr0.Lexeme, label);
		if (strcmp(op->Lexeme, "<") == 0) emit4("JGE", exp1->Lexeme, exp2->Lexeme, expr0.Lexeme);
		else if (strcmp(op->Lexeme, "=") == 0) emit4("JNE", exp1->Lexeme, exp2->Lexeme, expr0.Lexeme);
		else if (strcmp(op->Lexeme, ">") == 0) emit4("JLE", exp1->Lexeme, exp2->Lexeme, expr0.Lexeme);
		else if (strcmp(op->Lexeme, "<=") == 0) emit4("JG", exp1->Lexeme, exp2->Lexeme, expr0.Lexeme);
		else if (strcmp(op->Lexeme, "<>") == 0) emit4("JE", exp1->Lexeme, exp2->Lexeme, expr0.Lexeme);
		else if (strcmp(op->Lexeme, ">=") == 0) emit4("JL", exp1->Lexeme, exp2->Lexeme, expr0.Lexeme);
		strcat(expr0.Lexeme, ":");
		strcpy(lab->Lexeme, expr0.Lexeme);
	}
	else {
		getlabel(label);
		strcpy(expr0.Lexeme, label);
		strcpy(latter[counter].Lexeme, label);
		counter++;
		IdString temp;
		strcpy(temp, expr0.Lexeme);
		strcat(temp, ":");
		emit1(temp);
		getlabel(label);
		strcpy(expr0.Lexeme, label);
		if (strcmp(op->Lexeme, "<") == 0) emit4("JGE", exp1->Lexeme, exp2->Lexeme, expr0.Lexeme);
		else if (strcmp(op->Lexeme, "=") == 0) emit4("JNE", exp1->Lexeme, exp2->Lexeme, expr0.Lexeme);
		else if (strcmp(op->Lexeme, ">") == 0) emit4("JLE", exp1->Lexeme, exp2->Lexeme, expr0.Lexeme);
		else if (strcmp(op->Lexeme, "<=") == 0) emit4("JG", exp1->Lexeme, exp2->Lexeme, expr0.Lexeme);
		else if (strcmp(op->Lexeme, "<>") == 0) emit4("JE", exp1->Lexeme, exp2->Lexeme, expr0.Lexeme);
		else if (strcmp(op->Lexeme, ">=") == 0) emit4("JL", exp1->Lexeme, exp2->Lexeme, expr0.Lexeme);
		strcat(expr0.Lexeme, ":");
		strcpy(lab->Lexeme, expr0.Lexeme);
	}
}

void IfProc(sem_rec* label) {
	emit1(label->Lexeme);
}

void WhileLessEquaLessProc(sem_rec* label) {
	counter--;
	emit2("J", latter[counter].Lexeme);
	emit1(label->Lexeme);
}

void LessProc(sem_rec* op) {
	strcpy(op->Lexeme, "<");
}

void EqualProc(sem_rec* op) {
	strcpy(op->Lexeme, "=");
}

void GreaterProc(sem_rec* op) {
	strcpy(op->Lexeme, ">");
}

void LessEquaLessProc(sem_rec* op) {
	strcpy(op->Lexeme, "<=");
}

void NotEqualProc(sem_rec* op) {
	strcpy(op->Lexeme, "<>");
}

void GreaterEqualProc(sem_rec* op) {
	strcpy(op->Lexeme, ">=");
}

void AddId(sem_rec* term) {
	if (Head == NULL) {
		Head = Wsk = (struct Items*)malloc(sizeof(struct Items));
		Wsk->item = (char*)malloc(strlen(term->Lexeme) + 1);
		strcpy(Wsk->item, term->Lexeme);
		Wsk->next = NULL;
	}
	else if (Wsk->next == NULL) {
		Wsk->next = (struct Items*)malloc(sizeof(struct Items));
		Wsk = Wsk->next;
		Wsk->item = (char*)malloc(strlen(term->Lexeme) + 1);
		strcpy(Wsk->item, term->Lexeme);
		Wsk->next = NULL;
	}
}

void IntegerProc(sem_rec* term) {
	strcpy(term->Lexeme, "integer");
}

void RealProc(sem_rec* term) {
	strcpy(term->Lexeme, "real");
}

void DeclProc(sem_rec* term) {
	Wsk = Head;
	while (Wsk != NULL) {
		IdVal = LookUp(Wsk->item);
		HashItem* head = SymbolTable[IdVal];
		while (strcmp(head->Name, Wsk->item) != 0) head = head->Next;
		if (strcmp(head->Type, "") != 0) SemanticError("variable redeclared", head->Name);
		free(head->Type);
		head->Type = (char*)malloc(strlen(term->Lexeme) + 1);
		strcpy(head->Type, term->Lexeme);
		emit2(term->Lexeme, head->Name);
		Wsk = Wsk->next;
	}
	Wsk = Head;
	while (Wsk != NULL) {
		Head = Wsk->next;
		free(Wsk->item);
		free(Wsk);
		Wsk = Head;
	}
	Wsk = Head = NULL;
}

void NameProc(sem_rec* term) {
	IdVal = LookUp(term->Lexeme);
	HashItem* head = SymbolTable[IdVal];
	while (strcmp(head->Name, term->Lexeme) != 0) head = head->Next;
	free(head->Type);
	head->Type = (char*)malloc(strlen("ProgramName") + 1);
	strcpy(head->Type, "ProgramName");
}

void Start() {
	src3AC = fopen("prog.3AC", "w");
	tempnum = 0;
	labelnum = 0;
}

void Finish() {
	emit1("halt");
	fclose(src3AC);
}