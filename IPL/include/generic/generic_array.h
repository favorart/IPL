#define _CAT(x,y)    x##y
#define  CAT(x,y)  _CAT(x,y)

#ifndef  GENERIC_ARRAY_POSTFIX 
#error ("GENERIC_ARRAY_POSTFIX")
#endif
 
#ifndef  GENERIC_ARRAY_TYPE
#error ("GENERIC_ARRAY_TYPE")
#endif


typedef int (*qsort_comp)(const void*, const void*);
//-------------------------------------------------------------------------------
typedef struct CAT(_array_,GENERIC_ARRAY_POSTFIX) CAT(array_,GENERIC_ARRAY_POSTFIX);
struct CAT(_array_,GENERIC_ARRAY_POSTFIX)   // Array                                       | массив
{ size_t len;                               // Number of elements inside                   | число элементов
  size_t allocated;                         // Quantity of allocated memory (in elements)  | кол-во выделенной памяти (в элементах)
  GENERIC_ARRAY_TYPE *content;              // Pointer to begin of allocated memory        | указатель на начало массива
};


//-------------------------------------------------------------------------------
/* Array_Alloc   function

   > Allocate the given number n of the Array Elements.
*/
static void CAT(CAT(array_,GENERIC_ARRAY_POSTFIX),_alloc)
(struct CAT(_array_,GENERIC_ARRAY_POSTFIX) *Array, size_t n)
{ if( !n )
   return;
  
  Array->allocated += n;
  if( !Array->content )
    Array->content = (GENERIC_ARRAY_TYPE*)  calloc (Array->allocated, sizeof (GENERIC_ARRAY_TYPE));
  else   
    Array->content = (GENERIC_ARRAY_TYPE*) realloc (Array->content,   sizeof (GENERIC_ARRAY_TYPE) * Array->allocated);

  if( !Array->content )
    SetError(ERR_MEM);
}


//-------------------------------------------------------------------------------
/* Array_Free   function

   > Free all of the allocated memory for Array.
*/
static void CAT(CAT(array_,GENERIC_ARRAY_POSTFIX),_free)
(struct CAT(_array_,GENERIC_ARRAY_POSTFIX) *Array)
{ Array->len       = 0U; 
  Array->allocated = 0U;

  if( Array->content )
  { free(Array->content);
    Array->content = NULL;
  }
}


//-------------------------------------------------------------------------------
/* Array_Create   function

   > Initialize the Array and allocate some memory.
*/
static void CAT(CAT(array_,GENERIC_ARRAY_POSTFIX),_create)
(struct CAT(_array_,GENERIC_ARRAY_POSTFIX) *Array, size_t n)
{ Array->len       = 0U;
  Array->allocated = 0U;
  Array->content   = NULL;
  CAT(CAT(array_,GENERIC_ARRAY_POSTFIX),_alloc)(Array, n);
}


//-------------------------------------------------------------------------------
/* Array_Clear   function

   > Forget all of the inserted Elements in the Array.
*/
static void CAT(CAT(array_,GENERIC_ARRAY_POSTFIX),_clear)
(struct CAT(_array_,GENERIC_ARRAY_POSTFIX) *Array)
{ Array->len = 0; }


//-------------------------------------------------------------------------------
/* Array_Copy   function

   > Make dest the exact copy of src.
   > 
   > If dest and src are valid:
   > Free all the data in the dest.
   > Reinitialize it like the src.
   > Copy all data from the src into dest.

*/
static void CAT(CAT(array_,GENERIC_ARRAY_POSTFIX),_copy)
(struct CAT(_array_,GENERIC_ARRAY_POSTFIX) *scr, 
 struct CAT(_array_,GENERIC_ARRAY_POSTFIX) *dest)
{ 
  if( !scr || !dest )
    return;

  if( dest->allocated < scr->len )
  { CAT(CAT(array_,GENERIC_ARRAY_POSTFIX),_free)(dest);
    CAT(CAT(array_,GENERIC_ARRAY_POSTFIX),_create)(dest,scr->allocated);
  }
  else
  { CAT(CAT(array_,GENERIC_ARRAY_POSTFIX),_clear)(dest); }

  for(dest->len = 0U; dest->len < scr->len; ++dest->len)
  { dest->content[dest->len] = scr->content[dest->len]; }
}

 
 //-------------------------------------------------------------------------------
 /* Array_Exact   function
  
    Return:
      pointer to the Element by Index  or
      pointer of the last Element if index is incorrect.

 */
static GENERIC_ARRAY_TYPE* CAT(CAT(array_,GENERIC_ARRAY_POSTFIX),_exact)
( struct CAT(_array_,GENERIC_ARRAY_POSTFIX) *Array, size_t index)
{ 
  if( index >= Array->len )
  { index = Array->len - 1U; }
  return &Array->content[index];
}


//-------------------------------------------------------------------------------
/* Array_Find   function

   > Linear search for the first Element (of the given value by pointer p_elem)
   > in the Array using Compare function.
   > The pointer to found Element in the Array will be saved by pointer pp_found.
   > Index of this Element will be saved by the p_index.
   
   Return:
     0 = SUCCESS
    -1 = FAILURE

*/
static int  CAT(CAT(array_,GENERIC_ARRAY_POSTFIX),_find)
(struct CAT (_array_, GENERIC_ARRAY_POSTFIX) *Array,
 size_t *p_index, 
 GENERIC_ARRAY_TYPE   *p_elem, 
 GENERIC_ARRAY_TYPE **pp_found,
 int (*p_compar)(GENERIC_ARRAY_TYPE*, GENERIC_ARRAY_TYPE*))
{ 
  size_t i;

  if( p_elem == NULL || p_compar == NULL )
  { SetError(ERR_PTR);
    return -1;
  } 
  
  if ( p_index != NULL )
  { *p_index = 0U; }

  for (i = 0U; i < Array->len; ++i)
   if ( !(*p_compar)( &(Array->content[i]), p_elem) )
   { 
     if( p_index !=NULL )
     { *p_index = i; }
     
     if ( pp_found != NULL )
     { *pp_found = &(Array->content[i]); }

     return 0;
   }
 
  return -1;
}


//-------------------------------------------------------------------------------
/* Array_Shift   function

   > Shifting Elements in the Array to keep it uninterrupted.
   > Filling the holes in the Array.
   > 
   > Step is a number of positions to shift an Elements from the "Begin".
   > Len is the number of Elements from the "Begin" to the end of the Array
   > of from the begin of the Array to the Hole position.
   
   Return: no

*/
static void CAT(CAT(array_,GENERIC_ARRAY_POSTFIX),_shift)
(GENERIC_ARRAY_TYPE *begin, size_t len, int step)
{ 
  int i;
  if( step == 0 )
    return;
  // сдвиг элементов для заполнения дырок
  if( step>0 ) { for(i = (int) len; i > 0; --i) begin[i] = begin[i-step]; }
  else         { for(i = 0; i < (int) len; ++i) begin[i] = begin[i-step]; }
}


//-------------------------------------------------------------------------------
/* Array_Insert   function

   > Inserting the Element into the Array by the Index.
   > The found value will be saved by pointer P_elem.
   > 
   > if the given Index greater then the Array Length,
   > the new element will be added at the end of the Array.
   
   Return:
     pointer to a newly inserted Element and
     NULL  if it could not be inserted

*/
static GENERIC_ARRAY_TYPE* CAT(CAT(array_,GENERIC_ARRAY_POSTFIX),_insert)
(struct CAT (_array_, GENERIC_ARRAY_POSTFIX) *Array, size_t index, GENERIC_ARRAY_TYPE *p_elem)
{
  if ( p_elem != NULL )
  { SetError(ERR_PTR);
    return NULL;
  }
 
  if( Array->len >= Array->allocated )
   CAT(CAT(array_,GENERIC_ARRAY_POSTFIX),_alloc)(Array,Array->allocated*2);

  if( index >= (Array->len - 1U) )   // add at the end
  { index = Array->len; }
  else
  { CAT(CAT(array_, GENERIC_ARRAY_POSTFIX), _shift)((Array->content + index), (Array->len - index), 1); }

  Array->content[index] = *p_elem;

  ++Array->len;
  return &(Array->content[index]);
}


//-------------------------------------------------------------------------------
/* Array_Extract   function

   > Extraction an Element from the Array by the Index.
   > The extracted value will be saved by pointer P_elem.
   > 
   > if the given Index greater then the Array Length,
   > the last element will be extracted.

   Return:
     0 = SUCCESS
    -1 = FAILURE

*/
static int  CAT(CAT(array_,GENERIC_ARRAY_POSTFIX),_extract)
(struct CAT (_array_, GENERIC_ARRAY_POSTFIX) *Array, size_t index, GENERIC_ARRAY_TYPE *p_elem)
{ 
  if ( Array->len == 0U )
  {
    memset (p_elem, 0, sizeof (GENERIC_ARRAY_TYPE));
    return -1;
  }

  if( index >= (Array->len - 1U) )
  { *p_elem = Array->content[Array->len - 1U]; }
  else
  { *p_elem = Array->content[index];
    CAT(CAT(array_,GENERIC_ARRAY_POSTFIX),_shift)((Array->content + index),(Array->len - index), -1);
  }

  --Array->len;
  return 0;
}
//-------------------------------------------------------------------------------
