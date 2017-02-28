#ifndef _VARIABLES_H
#define _VARIABLES_H

typedef struct _hash_table variables;
typedef struct _names_table names_table;

names_table**  _Names_table (void);
//-------------------------------------------------------------------------------
#define  variables_create()                       hash_create((*_Names_table())->pVariables)
#define  variables_free()                         hash_free((*_Names_table())->pVariables)
#define  variables_lookup(name,elem,flag)         hash_lookup((*_Names_table())->pVariables,(name),(elem),(flag),(tenum)0)
#define  variables_extract(name)                  hash_extract((*_Names_table())->pVariables,(name),(tenum)0)
//-------------------------------------------------------------------------------
#endif