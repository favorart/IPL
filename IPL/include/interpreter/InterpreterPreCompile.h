#ifndef _IPL_PRE_COMPILE_H
#define _IPL_PRE_COMPILE_H

typedef struct _names_table names_table;
typedef struct  _hash_table  hash_table;
//-----------------------------------------------------------------------------------------
//---*** DUMP ***--------------------------------------------------------------------------
#define SHA256LEN 32
int     checkSHA256file (char *filename, unsigned char *oldSHA256Line,
	                                        unsigned char *newSHA256Line);
//-------------------------------------------------------------------------------
#define FILENAME_VARS     ".ipc.variables.dat"
#define FILENAME_CONS     ".ipc.constants.dat"
#define FILENAME_REWO     ".ipc.reg_words.dat"

#define FILE_EXT_LEN      4
typedef enum { HT_NO, HT_VARS, HT_CONS, HT_REWO } vhtenum;
typedef enum { FL_VTYPE=0, FL_OFFSET, FL_NONODE }  flenum;
//------------------------------------
int    checkHashConfFiles  (names_table *Names_table);
int    checkHashConfigFile (names_table *Names_table, FILE *f_inc);
int    writeHashConfigFile (names_table *Names_table, FILE *f_out);
//------------------------------------
void   myvar_fwrite (myvar *var, FILE* f_out);
void   myvar_fread  (myvar *var, FILE* f_inc);
//------------------------------------
int    hash_fwrite ( hash_table *Hash, FILE* f_out );
int    hash_fread  ( hash_table *Hash, FILE* f_inc );
//------------------------------------
int    bytecode_fwrite (names_table *Names_table, list_pmv *Sentence, FILE* f_out);
int    bytecode_fread  (names_table *Names_table, list_pmv *Sentence, FILE* f_inc);
//------------------------------------
int    program_fwrite ( names_table *Names_table, list_pmv *Sentence );
int    program_fread  ( names_table *Names_table, list_pmv *Sentence );
//------------------------------------
void   ipc_files_remove ( names_table *Names_table );
//------------------------------------
int    writeHashScriptFile (char *filename, FILE *f_out);
int    checkHashScriptFile (char *filename, FILE *f_inc);
//------------------------------------
void   fprintf_list ( FILE* f_out, list_pmv *Sentence, names_table *Names_table );
void    fscanf_list ( FILE* f_inc, list_pmv *Sentence, names_table *Names_table );
//-----------------------------------------------------------------------------------------
#endif
