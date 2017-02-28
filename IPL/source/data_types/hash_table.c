#include "header.h"
#include "hash_table.h"

//------------------------------------------------------------------------------------
unsigned int  get_hash_size (unsigned int size);
int      hash_extend (hash_table* Hash);
//------------------------------------------------------------------------------------
#define HASH_MULTIPLIER 31
// вычисление хэша строки
unsigned int hash (hash_table* Hash, char *key)
{ unsigned int h = 0;
  unsigned char* p;

  for(p = (unsigned char*)key; *p!=0; p++)
   h = HASH_MULTIPLIER * h + *p;
  
  return (h % Hash->size);
}
//------------------------------------------------------------------------------------
#define HASH_SIZE_RATIO  0.7
int      hash_extend (hash_table* Hash)
{ float ratio = (float)Hash->len / Hash->size;

  if( ratio > HASH_SIZE_RATIO )
  { hash_table H; 
    H.len  = 0;
    H.size = get_hash_size (Hash->size);
    H.content = (hash_node**)calloc( H.size,sizeof(hash_node*) );
    if( !H.content ) SetError (ERR_MEM);

    { myvar *val; hash_iter iter = {0};
      while( val = hash_next (Hash,&iter) )
       hash_lookup (&H,val->name,val,HASH_ADD_ENABLE,val->type);
    }

    hash_free (Hash); *Hash = H;
    return 1;
  } // if ratio
  return 0;
}
//------------------------------------------------------------------------------------
// если есть или теперь есть в таблице возвращает указатель, нет - ноль
myvar*   hash_lookup  (hash_table* Hash, char* name, myvar* elem, int flag_add, tenum flag_var)
{ hash_node* node = NULL; unsigned int h;
  if( !name || (flag_add && !elem) )
  { SetError(ERR_PTR); return NULL; }

  h = hash (Hash,name);
  for(node = Hash->content[h]; node; node = node->next)
   if( !strcmp(name,node->value.name) &&
      (!flag_var || (node->value.type == flag_var)) )
    return &(node->value);

//#define _DEBUG_
#ifdef  _DEBUG_ //-------------------------------
  if( elem )
  { unsigned int i,j;
    FILE *f = fopen ((elem->vtype == Variable)?("vtmp.txt"):("ctmp.txt"),"w");

    for(j=0, i=0; i<Hash->size; i++)
    { node = Hash->content[i];
      if(node) 
       fprintf (f,"%u,%u,%s,%u\n",j,i,
                        node->value.name,
                        (int)flag_var);
    }
    fclose (f);
  }
#endif //----------------------------------------
  
  if( flag_add )
  { node = (hash_node*)malloc( sizeof(hash_node) );
    if(!node) { SetError(ERR_MEM); return NULL; }

    node->value = *elem;
    Hash->len++;

    // добавляется в начало списка
    node->next = Hash->content[h]; 
    Hash->content[h] = node;

    // добавляется в конец списка
    //{ hash_node *nnode = Hash->content[h];
    //  node->next = NULL;
    //  if( nnode )
    //  { while( nnode->next )
    //     nnode = nnode->next;
    //    nnode->next = node;
    //  }
    //  else Hash->content[h] = node;
    //} 

    // рост хэш-таблицы
    //if( hash_extend (Hash) )
    // return hash_lookup (Hash,name,NULL,0,flag_var);
    return &(node->value);
  }
  return NULL;
}
// в случае неудачи - myvar = {0};
myvar    hash_extract (hash_table* Hash, char *name, tenum flag_var)
{ myvar elem = {0};
  hash_node *curr, *prev = NULL;
  unsigned int h;

  if( !Hash->len || !name ) { SetError(ERR_PTR); goto END; }
  h = hash(Hash, name);

  for(curr = Hash->content[h]; curr; prev = curr, curr = curr->next)
   if( !strcmp(name, curr->value.name) &&
      (!flag_var || (curr->value.type == flag_var)) )
    { elem = curr->value;
      (prev) ? (prev->next = curr->next) : (Hash->content[h] = NULL); // переставить ссылки
      free(curr);

      Hash->len--;
      goto END;
    }

END:
  return elem;
}
//------------------------------------------------------------------------------------
#define HASH_SIZE_MINIMUM        65537 // 65536
#define HASH_SIZE_SIMPLE_NUMBER  86453

void   hash_create (hash_table* Hash)
{ Hash->size = 25013; //get_hash_size (0);
  Hash->len  = 0;
  /* to allocate memory for a table */
  Hash->content = (hash_node**)calloc( Hash->size,sizeof(hash_node*) );
  if( !Hash->content ) SetError(ERR_MEM);
}
void   hash_free   (hash_table* Hash)
{ if( Hash->content ) 
  { unsigned int i;
    hash_node *curr, *next;
    
    for(i=0; i<Hash->size; i++)
     for(curr = Hash->content[i]; curr; curr = next)
      { next = curr->next;
        myvar_free (&curr->value);
        free (curr);
      }

    free (Hash->content);
    Hash->content = NULL;
  }
  Hash->size = 0;
  Hash->len  = 0;
}
void   hash_clear  (hash_table* Hash)
{ if( Hash->len ) 
  { unsigned int i;
    hash_node *curr, *next;
    
    for(i=0; i<Hash->size; i++)
    { for(curr = Hash->content[i]; curr; curr = next)
      { next = curr->next;
        myvar_free (&curr->value);
        free (curr); curr = NULL;
      }
      Hash->content[i] = NULL;
    }
    Hash->len  = 0;
  }
}
void   hash_copy   (hash_table* scr, hash_table* dest)
{ if( !scr || !dest ) { SetError(ERR_PTR); return; }
  hash_clear  (dest);
  
  { unsigned int i;
    hash_node *curr, *new_node, *node = NULL;

    for(i=0; i<scr->size; i++)
     for(curr = scr->content[i]; curr; curr = curr->next)
     { new_node = (hash_node*)calloc( 1,sizeof(hash_node) );
       if( !new_node ) { SetError (ERR_MEM); return; NULL; }
      
       myvar_copy (&curr->value, &new_node->value);
       
       // добавляется в конец списка
       if(!dest->content[i])
         node = dest->content[i] = new_node;
       else
         node = node->next = new_node;
       new_node->next = NULL;
     }
  }
  dest->len = scr->len;
}
//------------------------------------------------------------------------------------
myvar* hash_next   (hash_table* Hash, hash_iter* iter)
{ if( !iter->i && !iter->cur )
   iter->cur = Hash->content[0];
  else if( iter->cur )
   iter->cur = iter->cur->next;

  while( !iter->cur )
   if( iter->i<(Hash->size-1) )
    iter->cur = Hash->content[++iter->i];
   else { iter->cur = NULL; iter->i = 0; return NULL; }
  return &iter->cur->value;
}
//------------------------------------------------------------------------------------
// example.c
/*
#pragma warning (disable: 4996)

#include <stdio.h>
#include <vld.h>

#include "hash_table.h"

int main()
{ hash_table myhash;
  hash_table myhash2;
  myvar var, var1, var2, var_ = {0};

  strcpy(var.name,"blah");
  var.value = 235;

  strcpy(var1.name,"fuch");
  var1.value = 20;

  strcpy(var2.name,"is it works?");
  var2.value = 25;

  hash_create(&myhash,HASH_SIZE);
  hash_create(&myhash2,HASH_SIZE);

  hash_lookup(&myhash,&var, HASH_ADD__ENABLE);
  hash_lookup(&myhash,&var1,HASH_ADD__ENABLE);
  hash_lookup(&myhash,&var2,HASH_ADD__ENABLE);

  var = hash_extract(&myhash,"fuch");
  if( var.name[0]==0 ) printf("Error\n");

  var = hash_extract(&myhash,"123");
  if( var.name[0]==0 ) printf("Error\n");

  strcpy(var.name,"123");
  if( !hash_lookup(&myhash,&var,HASH_ADD__ENABLE) ) printf("Error\n");

  hash_copy(&myhash,&myhash2);

  { myvar* curr;
    for( curr = hash_next(&myhash2,HASH_BASE); curr; curr=hash_next(&myhash2,HASH_NEXT) )
     printf("%s %d\n",curr->name,curr->value);
  }
  hash_free(&myhash);
  hash_free(&myhash2);
  system("pause");
  return 0;
}
*/
//-------------------------------------------------------------------------------
