#define _CRT_SECURE_NO_WARNINGS
#ifndef TRANSLATOR
#define TRANSLATOR
#include "global.h"

void emit1(const char*);
void emit2(const char*, const char*);
void emit3(const char*, const char*, const char*);
void emit4(const char*, const char*, const char*, const char*);
void emit5(const char*, const char*, const char*, const char*, const char*);
void gettemp(char*);
void AssignProc(sem_rec*, sem_rec*);
void ReadProc();
void WriteEqualProc(sem_rec*);
void IfProc(sem_rec*);
void WhileLessEquaLessProc(sem_rec*);
void GenInfix(sem_rec, sem_rec, sem_rec, sem_rec*);
void PlusProc(sem_rec*);
void MinusProc(sem_rec*);
void MultiProc(sem_rec*);
void DivProc(sem_rec*);
void IntProc(sem_rec*);
void FloatProc(sem_rec*);
void IdProc(sem_rec*);
void BooLessProc(sem_rec*, sem_rec*, sem_rec*, sem_rec*, sem_rec*);
void LessProc(sem_rec*);
void EqualProc(sem_rec*);
void GreaterProc(sem_rec*);
void LessEquaLessProc(sem_rec*);
void NotEqualProc(sem_rec*);
void GreaterEqualProc(sem_rec*);
void AddId(sem_rec*);
void IntegerProc(sem_rec*);
void RealProc(sem_rec*);
void DeclProc(sem_rec*);
void NameProc(sem_rec*);
void Start();
void Finish();

#endif