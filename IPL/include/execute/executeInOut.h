#ifndef _EX_IN_OUT_H
#define _EX_IN_OUT_H

typedef struct _names_table   names_table;
typedef struct _myvar               myvar;
//-------------------------------------------------------------------------------
void   joint_fscanf      (FILE*  fin, names_table *Names_table, myvar *jnt);
void   joint_fprintf     (FILE* fout, names_table *Names_table, myvar *jnt);

void   container_fscanf  (FILE*  fin, names_table *Names_table, myvar *cnt);
void   container_fprintf (FILE* fout, names_table *Names_table, myvar *cnt);
//-------------------------------------------------------------------------------
myvar* fscanf_constants  (FILE*  fin, names_table *Names_table, myvar *cur);
//-------------------------------------------------------------------------------
#endif