#ifndef _GENERIC_STACK_H
#define _GENERIC_STACK_H

#define _CAT(x,y)    x##y
#define  CAT(x,y)  _CAT(x,y)

#if !defined GENERIC_STACK_POSTFIX 
#    error("GENERIC_STACK_POSTFIX")
#endif
#if !defined GENERIC_STACK_TYPE
#    error("GENERIC_STACK_TYPE")
#endif

//-----------------------------------------------------------------------------------------
typedef struct CAT(_stack_,GENERIC_STACK_POSTFIX) CAT(stack_,GENERIC_STACK_POSTFIX);
struct CAT(_stack_,GENERIC_STACK_POSTFIX)
{ GENERIC_STACK_TYPE *content;
  size_t len;
  size_t allocated;
};
//-----------------------------------------------------------------------------------------


static void   CAT(CAT(stack_,GENERIC_STACK_POSTFIX),_alloc)  
 (struct CAT(_stack_,GENERIC_STACK_POSTFIX) *Stack, size_t num) 
{ if ( !num )
   return;
  Stack->allocated += num; 
  
  if ( !Stack->content )
    Stack->content = (GENERIC_STACK_TYPE*) malloc( Stack->allocated*sizeof(GENERIC_STACK_TYPE) );
  else
  { Stack->content = (GENERIC_STACK_TYPE*)realloc( Stack->content,  sizeof(GENERIC_STACK_TYPE)*Stack->allocated );
  }

  if ( !Stack->content )
    SetError(ERR_MEM);
}

static void   CAT(CAT(stack_,GENERIC_STACK_POSTFIX),_create)
 (struct CAT(_stack_,GENERIC_STACK_POSTFIX) *Stack, size_t num)
{ Stack->content   = NULL; 
  Stack->len       = 0; 
  Stack->allocated = 0;

  CAT(CAT(stack_,GENERIC_STACK_POSTFIX),_alloc)(Stack,num);
}

static void   CAT(CAT(stack_,GENERIC_STACK_POSTFIX),_free)
 (struct CAT(_stack_,GENERIC_STACK_POSTFIX) *Stack) 
{ Stack->len  = 0; 
  Stack->allocated = 0;
  
  if( Stack->content )
  { free(Stack->content); 
    Stack->content = NULL;
  }
}

static GENERIC_STACK_TYPE* CAT(CAT(stack_,GENERIC_STACK_POSTFIX),_push)
 (struct CAT(_stack_,GENERIC_STACK_POSTFIX) *Stack, GENERIC_STACK_TYPE* elem)
{ if( !elem )
  { SetError(ERR_PTR);
    return NULL;
  }
 
  if( Stack->len >= Stack->allocated )
   CAT(CAT(stack_, GENERIC_STACK_POSTFIX),_alloc)(Stack,Stack->allocated * 2);

  Stack->content[Stack->len] = *elem;
  return &(Stack->content[Stack->len++]);
}

static GENERIC_STACK_TYPE  CAT(CAT(stack_,GENERIC_STACK_POSTFIX),_pop)
 (struct CAT(_stack_, GENERIC_STACK_POSTFIX) *Stack) 
{ GENERIC_STACK_TYPE elem = Stack->content[--Stack->len];
  return elem;
}
//-----------------------------------------------------------------------------------------
#endif