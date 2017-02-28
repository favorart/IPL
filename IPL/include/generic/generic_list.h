#define _CAT(x,y)    x##y
#define  CAT(x,y)  _CAT(x,y)

#ifndef  GENERIC_LIST_POSTFIX 
#error ("GENERIC_LIST_POSTFIX")
#endif
#ifndef  GENERIC_LIST_TYPE
#error ("GENERIC_LIST_TYPE")
#endif


//-------------------------------------------------------------------------------
typedef struct CAT(CAT(_list_,GENERIC_LIST_POSTFIX),_node) CAT(CAT(list_,GENERIC_LIST_POSTFIX),_node);
struct CAT(CAT(_list_,GENERIC_LIST_POSTFIX),_node)
{ GENERIC_LIST_TYPE value;
  struct CAT(CAT(_list_,GENERIC_LIST_POSTFIX),_node) *prev;
  struct CAT(CAT(_list_,GENERIC_LIST_POSTFIX),_node) *next;
};

typedef struct CAT(_list_,GENERIC_LIST_POSTFIX) CAT(list_,GENERIC_LIST_POSTFIX);
struct CAT(_list_,GENERIC_LIST_POSTFIX)                       // double directions list  | список
{ size_t len;                                                 // number of elements      | число элементов
  struct CAT(CAT(_list_,GENERIC_LIST_POSTFIX),_node) *first;  // link to the first       | ссылка на первый
  struct CAT(CAT(_list_,GENERIC_LIST_POSTFIX),_node) *last;   // link to the last        | ссылка на последний
};


//-------------------------------------------------------------------------------
/* List_Create   function

   > Initialize the List.
*/
static void   CAT(CAT(list_,GENERIC_LIST_POSTFIX),_create)
(struct CAT(_list_,GENERIC_LIST_POSTFIX) *List)
{ List->len   = 0U; 
  List->first = NULL;
  List->last  = NULL;
}


//-------------------------------------------------------------------------------
/* List_Free   function

   > Free all of the allocated memory in the List.
*/
static void   CAT(CAT(list_,GENERIC_LIST_POSTFIX),_free)
(struct CAT(_list_,GENERIC_LIST_POSTFIX) *List)
{ struct CAT(CAT(_list_,GENERIC_LIST_POSTFIX),_node)* cur;

  while( List->first )
  { cur = List->first;
    List->first = List->first->next;
    
    if(cur)
    { free(cur);
      cur = NULL;
    }
  }

  List->len   = 0U;
  List->first = NULL;
  List->last  = NULL;
}


//-------------------------------------------------------------------------------
/* List_Clear   function

   > Forget all of the inserted Elements in the List and free their memory.
*/
static void   CAT(CAT(list_,GENERIC_LIST_POSTFIX),_clear)
(struct CAT(_list_,GENERIC_LIST_POSTFIX) *List)
{ CAT(CAT(list_,GENERIC_LIST_POSTFIX),_free)(List); }


//-------------------------------------------------------------------------------
/* List_Insert   function

   > Inserting the Element into the Array by the Index.
   > The found value will be saved by pointer P_elem.
   >
   > if the given Index greater then the Array Length,
   > the new element will be added at the end of the Array.
   
   Return:
     pointer to a newly inserted Element and
     NULL  if it could not be inserted

*/
static GENERIC_LIST_TYPE* CAT(CAT(list_,GENERIC_LIST_POSTFIX),_insert)
(struct CAT(_list_,GENERIC_LIST_POSTFIX) *List, size_t index, GENERIC_LIST_TYPE *p_elem)
{ 
  struct CAT(CAT(_list_,GENERIC_LIST_POSTFIX),_node) *new_node;
  if( p_elem == NULL )
  { SetError(ERR_PTR);
    return NULL;
  }

  new_node = (struct CAT(CAT(_list_,GENERIC_LIST_POSTFIX),_node)*)
       malloc( sizeof(struct CAT(CAT(_list_,GENERIC_LIST_POSTFIX),_node)) );
  if( !new_node )
  { SetError(ERR_MEM);
    return NULL;
  }
  new_node->value = *p_elem;

  if( !List->first ) // cлучай пустого списка
  { List->first = new_node;
    List->first->next = List->first->prev = NULL;
    List->last = List->first;
  }
  else if( !index ) // cлучай вставки в начало списка
  { List->first->prev = new_node;
    List->first->prev->next = List->first;
    List->first = List->first->prev;
    List->first->prev = NULL;
  }
  else if( index >= List->len ) // (int) index < 0 ||  !!!!!!!!!!!!
  { 
    // случай вставки в конец списка
    List->last->next = new_node;
    List->last->next->prev = List->last;
    List->last = List->last->next; 
    List->last->next = NULL;
  }
  else // вставка в середину
  { struct CAT(CAT(_list_,GENERIC_LIST_POSTFIX),_node) *cur;

    if( (List->len - index) < index )
    { cur = List->last;
      index = (List->len - 1U) - index;
      while(index--)
       cur = cur->prev;
    }
    else
    { cur = List->first;
      while(index--)
       cur = cur->next;
    }

    new_node->next = cur;
    new_node->prev = cur->prev;

    cur->prev->next = new_node;
    cur->prev = new_node;
  }

  List->len++;
  return &(new_node->value);
}


//-------------------------------------------------------------------------------
/* List_Extract   function

   > Extraction an Element from the List by the Index.
   > The extracted value will be saved by pointer P_elem.
   >
   > if the given Index greater then the List Length,
   > the last element will be extracted.
   
   Return:
      0 = SUCCESS
     -1 = FAILURE

*/
static int  CAT(CAT(list_,GENERIC_LIST_POSTFIX),_extract)
(struct CAT (_list_, GENERIC_LIST_POSTFIX) *List, size_t  index, GENERIC_LIST_TYPE *p_elem)
{
  if ( !List->len )
    return -1;

  if ( List->len == 1U )
  {
    *p_elem = List->first->value;

    free (List->first);
    List->first = List->last = NULL;
  }
  else if ( index == 0U ) // в случае - извлечь из начала списка
  {
    *p_elem = List->first->value;
    List->first = List->first->next;

    free (List->first->prev);
    List->first->prev = NULL;
  }
  else if ( index >= (List->len - 1U) ) // (int) index < 0 ||  !!!!!!!!!!!!!!
  {
    // в случае - извлечь из конца списка
    *p_elem = List->last->value;
    List->last = List->last->prev;

    free (List->last->next);
    List->last->next = NULL;
  }
  else  // в случае - извлечь из середины списка
  {
    struct CAT (CAT (_list_, GENERIC_LIST_POSTFIX), _node) *cur;

    if ( (List->len - index) < index )
    {
      cur = List->last;
      index = List->len - index;
      while ( index-- )
        cur = cur->prev;
    }
    else
    {
      cur = List->first;
      while ( index-- )
        cur = cur->next;
    }
    *p_elem = cur->value;
    cur->next->prev = cur->prev;
    cur->prev->next = cur->next;

    free (cur);
  }

  --List->len;
  return 0;
}


//-------------------------------------------------------------------------------
/* List_Extracts   function

   > Extraction an Element from the List by the Index.
   > The extracted value will be saved by pointer P_elem.
   >
   > if the given Index greater then the List Length,
   > the last element will be extracted.
   
   Return: the extracted Element  or
           the Element filled zeroes, 
           if the index is incorrect

*/
static GENERIC_LIST_TYPE  CAT(CAT(list_,GENERIC_LIST_POSTFIX),_extracts)
(struct CAT (_list_, GENERIC_LIST_POSTFIX) *List, size_t  index)
{ 
  GENERIC_LIST_TYPE elem = {0};
  if( !List->len )
    return elem;
  
  if( List->len == 1U )
  { elem = List->first->value;

    free(List->first);
    List->first = List->last = NULL;
  }
  else if( !index ) // в случае - извлечь из начала списка
  { elem = List->first->value;
    List->first = List->first->next;

    free(List->first->prev);
    List->first->prev = NULL;
  }
  else if( index >= (List->len - 1U) ) // (int) index < 0 ||  !!!!!!!!!!!!!!
  { 
    // в случае - извлечь из конца списка
    elem = List->last->value;
    List->last = List->last->prev;

    free(List->last->next);
    List->last->next = NULL;
  }
  else  // в случае - извлечь из середины списка
  { struct CAT(CAT(_list_,GENERIC_LIST_POSTFIX),_node) *cur;

    if( (List->len - index) < index )
    { cur = List->last;
      index = List->len - index;
      while(index--)
       cur = cur->prev;
    }
    else
    { cur = List->first;
      while(index--)
       cur = cur->next;
    }
    elem = cur->value;
    cur->next->prev = cur->prev;
    cur->prev->next = cur->next;

    free(cur);
  }

  List->len--;
  return elem;
}


//-------------------------------------------------------------------------------
/* List_Exact   function

   Return:
     pointer to the Element by Index  or
     pointer of the last Element if index is incorrect.

*/
static GENERIC_LIST_TYPE* CAT(CAT(list_,GENERIC_LIST_POSTFIX),_exact   )
(struct CAT(_list_,GENERIC_LIST_POSTFIX) *List, size_t  index)
{
  if( !index )
   return &(List->first->value);
  else if( index >= List->len ) // (int) index < 0 || !!!!
   return &(List->last->value);
  else
  { struct CAT(CAT(_list_,GENERIC_LIST_POSTFIX),_node) *cur;

    if( (List->len - index) < index )
    { cur = List->last;
      index = (List->len - 1U) - index;
      while(index--)
       cur = cur->prev;
    }
    else
    { cur = List->first;
      while(index--)
       cur = cur->next;
    }
    return &(cur->value);
  }
}


//-------------------------------------------------------------------------------
/* List_Find   function
  
   > Linear search for the first Element (of the given value by pointer p_elem)
   > in the List using Compare function.
   > The pointer to found Element in the List will be saved by pointer pp_found.
   > Index of this Element will be saved by the p_index.
   
   Return:
      0 = SUCCESS
     -1 = FAILURE

*/
static int  CAT(CAT(list_,GENERIC_LIST_POSTFIX),_find    )
(struct CAT(_list_,GENERIC_LIST_POSTFIX) *List,
 size_t *p_index,
 GENERIC_LIST_TYPE   *p_elem,
 GENERIC_LIST_TYPE **pp_found,
 size_t (*compar)(GENERIC_LIST_TYPE*,GENERIC_LIST_TYPE*) )
{ 
  size_t i;
  struct CAT(CAT(_list_,GENERIC_LIST_POSTFIX),_node) *cur;
  
  if( p_elem == NULL || compar == NULL )
  { SetError(ERR_PTR);
    return -1;
  } 
  
  if( p_index )
  { *p_index = -1; }

  for(cur = List->first, i=0; cur; cur = cur->next, i++)
   if( !(*compar)(&(cur->value), p_elem) )
   { if( p_index != NULL )
     { *p_index = i; }
     if ( pp_found != NULL )
     { *pp_found = &(cur->value); }
     return 0;
   }

  return -1;
}


//-------------------------------------------------------------------------------
/* List_Copy   function

   > Make dest the exact copy of src.
   > 
   > If dest and src are valid:
   > Free all the data in the dest.
   > Reinitialize it like the src.
   > Copy all data from the src into dest.
*/
static void   CAT(CAT(list_,GENERIC_LIST_POSTFIX),_copy)
(struct CAT(_list_,GENERIC_LIST_POSTFIX) *scr, struct CAT(_list_,GENERIC_LIST_POSTFIX) *dest)
{
  struct CAT(CAT(_list_,GENERIC_LIST_POSTFIX),_node) *dcur, *scur;
  
  if( !scr || !dest )
    return;
  // CAT(CAT(list_,GENERIC_LIST_POSTFIX),_free)(dest);

  scur =  scr->first;
  dcur = dest->first;

  while( dcur && scur )
  { dcur->value = scur->value;
    dcur = dcur->next;
    scur = scur->next;
  }

  while( scur )
  { CAT(CAT(list_,GENERIC_LIST_POSTFIX),_insert)(dest, -1, &scur->value);
    scur = scur->next;
  }
  while( dest->len > scr->len )
   CAT(CAT(list_,GENERIC_LIST_POSTFIX),_extract)(dest, -1, NULL);
}


//-------------------------------------------------------------------------------
/* List_Sort   function

   > Merge sorting of the List.
*/
static void    CAT(CAT(list_,GENERIC_LIST_POSTFIX),_merge_sort )
( struct CAT(_list_,GENERIC_LIST_POSTFIX) *source,
  int (*compar)(GENERIC_LIST_TYPE*,GENERIC_LIST_TYPE*) )
{ struct CAT(_list_,GENERIC_LIST_POSTFIX) a,b;
  // вырожденный случай – длина списка равно 0 или 1
  if( source->len < 2 ) return;

  //CAT(CAT(list_,GENERIC_LIST_POSTFIX),_split ) (source,&a,&b);
  //static void  CAT(CAT(list_,GENERIC_LIST_POSTFIX),_split )
  //( struct CAT(_list_,GENERIC_LIST_POSTFIX) *source /*  INPUT */ , 
  //  struct CAT(_list_,GENERIC_LIST_POSTFIX) *front  /* OUTPUT */ ,
  //  struct CAT(_list_,GENERIC_LIST_POSTFIX) *back   /* OUTPUT */ ) 
  { 
    //---delete if function
    struct CAT(_list_,GENERIC_LIST_POSTFIX) *front = &a;
    struct CAT(_list_,GENERIC_LIST_POSTFIX) *back  = &b;

    if (source->len < 2) 
    { front->first = source->first;
      front->last  = source->last;
      front->len   = source->len;
    
      back->first = NULL;
      back->last  = NULL;
      back->len   = 0;
    }
    else 
    { struct CAT(CAT(_list_,GENERIC_LIST_POSTFIX),_node)
      *current = source->first;
      size_t i,front_len = (source->len+1)/2;
    
      for(i=0; i<front_len-1; i++)
       current = current->next;

      // исходный список разбивается на два подсписка
      front->first = source->first;
      front->last  = current;
      front->len   = front_len;

      back->first = current->next;
      back->last  = source->last;
      back->len   = source->len - front_len;

      back->first->prev = NULL;
      front->last->next = NULL;
    }
  }
  // end of _split function

  // рекурсивная сортировка подсписков
  CAT(CAT(list_,GENERIC_LIST_POSTFIX),_merge_sort ) (&a,compar); 
  CAT(CAT(list_,GENERIC_LIST_POSTFIX),_merge_sort ) (&b,compar);

  // CAT(CAT(list_,GENERIC_LIST_POSTFIX),_sorted_create ) (source,&a,&b,compar);
  // static void  CAT(CAT(list_,GENERIC_LIST_POSTFIX),_sorted_create )
  // ( struct CAT(_list_,GENERIC_LIST_POSTFIX) *source,
  //   struct CAT(_list_,GENERIC_LIST_POSTFIX) *pa,
  //   struct CAT(_list_,GENERIC_LIST_POSTFIX) *pb,
  //   size_t (*compar)(GENERIC_LIST_TYPE*,GENERIC_LIST_TYPE*) ) 
  {
    //---delete if function
    struct CAT(_list_,GENERIC_LIST_POSTFIX) *pa = &a;
    struct CAT(_list_,GENERIC_LIST_POSTFIX) *pb = &b;
   
         if( !pa->len ) *source = *pb;
    else if( !pb->len ) *source = *pa;
    else
    { struct CAT(CAT(_list_,GENERIC_LIST_POSTFIX),_node)
      *scur, *acur = pa->first, *bcur = pb->first;

      if( (*compar) (&acur->value,&bcur->value) <= 0 )
      { scur = source->first = acur; acur = acur->next; }
      else
      { scur = source->first = bcur; bcur = bcur->next; }

      while( acur && bcur )
      { if( (*compar) (&acur->value,&bcur->value) <= 0 )
        { scur->next = acur; acur = acur->next; }
        else
        { scur->next = bcur; bcur = bcur->next; }
        scur->next->prev = scur; scur = scur->next;      
      }

      while( acur )
      { scur->next = acur; acur = acur->next;
        scur->next->prev = scur; scur = scur->next;
      }
      while( bcur )
      { scur->next = bcur; bcur = bcur->next;
        scur->next->prev = scur; scur = scur->next;
      }
      source->last = scur; source->len  = (pa->len + pb->len);
    } // end else
  }
  // end of _sorted_create function
}
//-------------------------------------------------------------------------------
