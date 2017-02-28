#ifndef _HASH_TABLE_H
#define _HASH_TABLE_H

typedef struct _myvar myvar; struct _myvar;
typedef struct _hash_node   hash_node;
typedef struct _hash_table  hash_table;
//------------------------------------------------------------------------------------
struct _hash_node
{ myvar     value;
  hash_node* next;
};
struct _hash_table
{ unsigned int    len; // кол-во элементов
  unsigned int   size; // кол-во ячеек хэш-таблицы 
  hash_node** content; // начало списков
};
//-----------------------------------------------------------------------------
void   hash_create (hash_table *Hash);
void   hash_free   (hash_table *Hash);
void   hash_clear  (hash_table *Hash);
void   hash_copy   (hash_table *scr, hash_table *dest);
//-----------------------------------------------------------------------------
#define  HASH_ADD_DISABLE   0
#define  HASH_ADD_ENABLE    1

#define  HASH_VAR           0

// ( hash_exact, hash_find, hash_insert )
myvar*    hash_lookup  (hash_table *Hash, char *name, myvar *elem, int flag_add, tenum flag_var);
myvar     hash_extract (hash_table *Hash, char *name, tenum flag_var);
//-----------------------------------------------------------------------------
typedef struct _hash_iter hash_iter;
struct _hash_iter
{ unsigned int i;
  hash_node *cur;
};

// когда все элементы закончены - возвращает NULL
myvar*  hash_next (hash_table* Hash, hash_iter* iter);

// использование:
//   myvar* cur; hash_iter iter = {0};
//   while( cur = hash_next (Hash,&iter) )
//   { // cur ... }
//-----------------------------------------------------------------------------
#endif