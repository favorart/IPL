#ifndef _CONSTANTS_H
#define _CONSTANTS_H

typedef struct _hash_table constants;
typedef struct _names_table names_table;

names_table**  _Names_table (void);
//-------------------------------------------------------------------------------
#define  constants_create()                              hash_create((*_Names_table())->pConstants)
#define  constants_free()                                hash_free((*_Names_table())->pConstants)
#define  constants_lookup(name,elem,flag,type)           hash_lookup((*_Names_table())->pConstants,(name),(elem),(flag),(type))
#define  constants_extract(name,type)                    hash_extract((*_Names_table())->pConstants,(name),(type))
//-------------------------------------------------------------------------------
#endif