#ifndef _PROCEDURE_H
#define _PROCEDURE_H

typedef struct _hash_table hash_table;
typedef struct _list_pmv     list_pmv;

#define  FUNC_ARGS_NO_NAME   "args"
//-----------------------------------------------------------------------------------------
//---*** STRUCT PROCEDURE ***--------------------------------------------------------------
typedef struct _procedure procedure;
struct _procedure // функция
{ myvar       *ResultType; // тип возвращаемого значения
  list_pmv     FormalPars; // список формальных параметров в порядке задания,\
                              число параметров – длина списка
                           // список фактических параметров получается отдельно\
                              при вызове функции
  list_pmv      Sentences; // код тела: список указателей на данные в правильном порядке
  variables    *BsValVars; // начальные значения переменных функции для предкомпиляции
  variables    *Variables; // переменных функции для выполнения
	//-------------------------
	void        *fScriptTmp;
	lexical_data   *pLexTmp;
	char         *pFilename;
	//-------------------------
};
//-----------------------------------------------------------------------------------------
procedure*  procedure_create (void);
procedure*  procedure_init   (myvar *rtype, char *filename, names_table *Names_table);
void        procedure_free   (procedure *Procedure);
void        procedure_copy   (procedure *scr, procedure *dest);
//-----------------------------------------------------------------------------------------
int    procedure_fread   (procedure *Procedure, names_table *Names_table, FILE *f_inc);
int    procedure_fwrite  (procedure *Procedure, names_table *Names_table, FILE *f_out);
//-----------------------------------------------------------------------------------------
#endif