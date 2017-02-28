#include "header.h"


const size_t CONT_ARRAY_DEFAULT_SIZE = 50U;

static const size_t  insersions_threshold = 25U;
static const size_t  addressing_threshold = 25U;
static const size_t  default_array_size   = 35U;
static const float   convert_coefficient  = 1.5F;
//-------------------------------------------------------------------------------
void container_create (container* Container, conts_enum type, size_t size)
{ 
  Container->type = type;
  
  Container->adressing_last_series = 0U;
  Container->inserting_last_series = 0U;

  Container->adressing_almost = 0U;
  Container->inserting_almost = 0U;

  switch( Container->type )
  {
    case CONT_TYPE_ARRAY:
    { array_pmv_create (&Container->value.avalue, (!size) ? (default_array_size) : (size));
      Container->len = &Container->value.avalue.len;
      break;
    }

    case CONT_TYPE_DLIST:
    { list_pmv_create (&Container->value.lvalue);
      Container->len = &Container->value.lvalue.len;
      break;
    }

    default: break;
  }
}

void container_copy   (container* scr, container* dest)
{ 
  if( !scr || !dest )
    return;

  container_free   (dest);
  container_create (dest,scr->type,*scr->len);

  switch( scr->type )
  {
    case CONT_TYPE_ARRAY: array_pmv_copy (&scr->value.avalue, &dest->value.avalue); break;
    case CONT_TYPE_DLIST:  list_pmv_copy (&scr->value.lvalue, &dest->value.lvalue); break;
    default: break;
  }

  dest->adressing_last_series = scr->adressing_last_series;
  dest->inserting_last_series = scr->inserting_last_series;

  dest->adressing_almost = scr->adressing_almost;
  dest->inserting_almost = scr->inserting_almost;
}

void container_clear  (container* Container)
{ 
  switch( Container->type )
  {
    case CONT_TYPE_ARRAY: array_pmv_clear (&Container->value.avalue); break;
    case CONT_TYPE_DLIST:  list_pmv_clear (&Container->value.lvalue); break;
    default: break;
  } 
  Container->adressing_last_series = 0U;
  Container->inserting_last_series = 0U;

  Container->adressing_almost = 0U;
  Container->inserting_almost = 0U;
}

void container_free   (container* Container)
{
  switch( Container->type )
  {
    case CONT_TYPE_ARRAY: array_pmv_free (&Container->value.avalue); break;
    case CONT_TYPE_DLIST:  list_pmv_free (&Container->value.lvalue); break;
    default: break;
  } 
  Container->adressing_last_series = 0U;
  Container->inserting_last_series = 0U;

  Container->adressing_almost = 0U;
  Container->inserting_almost = 0U;
}
//-------------------------------------------------------------------------------
void container_convert_to_array (container* Container)
{ array_pmv Array;
  array_pmv_create(&Array, Container->value.lvalue.len + default_array_size);
  
  while( *Container->len )
  {
    myvar *pvar;
    if ( !list_pmv_extract (&Container->value.lvalue, -1, &pvar) )
      array_pmv_insert (&Array, -1, &pvar);
  }
  list_pmv_free (&Container->value.lvalue);

  Container->type = CONT_TYPE_ARRAY;
  Container->value.avalue = Array;
  Container->len = &Container->value.avalue.len;
}
void container_convert_to_list  (container* Container)
{ list_pmv List;
  list_pmv_create(&List);

  while( *Container->len )
  {
    myvar *pvar;
    if ( !array_pmv_extract (&Container->value.avalue, -1, &pvar) )
      list_pmv_insert (&List, -1, &pvar);
  }
  array_pmv_free (&Container->value.avalue);

  Container->type = CONT_TYPE_DLIST;
  Container->value.lvalue = List;
  Container->len = &Container->value.lvalue.len;
}
//-------------------------------------------------------------------------------
typedef enum { CONT_INSERTSION = 1, CONT_ADDRESSING = 2 } cont_opers;
/* Converting array to list and vise versa.. */
void container_convert    (container* Container, cont_opers flag)
{
       if ( flag == CONT_INSERTSION )
  { Container->inserting_almost++;
    Container->inserting_last_series++;
    Container->adressing_last_series = 0U;
  }
  else if ( flag == CONT_ADDRESSING )
  { Container->adressing_almost++;
    Container->adressing_last_series++;
    Container->inserting_last_series = 0U;
  }
  else
  { SetError(ERR_PAR);
    return;
  }

  //----------------------------------------
  if ( Container->inserting_last_series >= insersions_threshold &&
       Container->type == CONT_TYPE_ARRAY &&
       Container->inserting_almost > convert_coefficient*Container->adressing_almost
    )
  { container_convert_to_list  (Container);
    Container->adressing_almost = Container->adressing_last_series = 0U;
    Container->inserting_almost = Container->inserting_last_series = 1U;
  }
  //----------------------------------------
  else if ( Container->inserting_last_series >= addressing_threshold &&
            Container->type == CONT_TYPE_DLIST &&
            Container->adressing_almost > convert_coefficient*Container->inserting_almost
         )
  { container_convert_to_array (Container);
    Container->adressing_almost = Container->adressing_last_series = 1U;
    Container->inserting_almost = Container->inserting_last_series = 0U;
  }
  //----------------------------------------
}
//-------------------------------------------------------------------------------
myvar*  container_extract (container* Container, size_t    index)
{
  int result = 0;
  myvar *p_result = NULL;
  
  if( index < (*Container->len) )
    container_convert (Container, CONT_INSERTSION);

  switch( Container->type )
  {
    case CONT_TYPE_ARRAY: result = array_pmv_extract (&Container->value.avalue, index, &p_result); break;
    case CONT_TYPE_DLIST: result =  list_pmv_extract (&Container->value.lvalue, index, &p_result); break;
    default: break;
  }
  return  (!result) ? (p_result) : (NULL);
}
myvar** container_exact   (container* Container, size_t    index)
{ 
  myvar** result = NULL;
  
  if( index <= (*Container->len) )
    container_convert (Container, CONT_ADDRESSING);

  switch( Container->type )
  {
    case CONT_TYPE_ARRAY: result = array_pmv_exact (&Container->value.avalue, index); break;
    case CONT_TYPE_DLIST: result =  list_pmv_exact (&Container->value.lvalue, index); break;
    default: break;
  }
  return result;
}
myvar*  container_insert  (container* Container, size_t    index, myvar *p_elem)
{
  int result = 0;
  myvar *p_result = NULL;

  if ( index < (*Container->len) )
    container_convert (Container, CONT_INSERTSION);

  switch ( Container->type )
  {
    case CONT_TYPE_ARRAY: result = *array_pmv_insert (&Container->value.avalue, index, &p_elem); break;
    case CONT_TYPE_DLIST: result = *list_pmv_insert (&Container->value.lvalue, index, &p_elem); break;
    default: break;
  }
  return result;
}
myvar*  container_find    (container* Container, size_t *p_index, myvar *p_elem, int (*compare)(myvar**, myvar**))
{
  int result = 0;
  myvar **p_result = NULL;

  container_convert (Container, CONT_ADDRESSING);

  switch (Container->type)
  { 
    case CONT_TYPE_ARRAY :result = array_pmv_find (&Container->value.avalue, p_index, &p_elem, &p_result, compare); break;
    case CONT_TYPE_DLIST: result =  list_pmv_find (&Container->value.lvalue, p_index, &p_elem, &p_result, compare); break;
    default: break;
  }
  return  (!result) ? (*p_result) : (NULL);
}
//-------------------------------------------------------------------------------