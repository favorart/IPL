#ifndef _CONTAINER_H
#define _CONTAINER_H

//#include "header.h"
//#pragma warning (disable: 4018) // signed/unsigned mismatch

#include "pmvArray.h"
#include "pmvList.h"
//-------------------------------------------------------------------------------
typedef enum { CONT_TYPE_ARRAY = 1, CONT_TYPE_DLIST = 2 } conts_enum;

extern const size_t CONT_ARRAY_DEFAULT_SIZE;
//-------------------------------------------------------------------------------
struct _container                 // контейнер хранения данных
{
  conts_enum type;                    // тип контейнера
  
  size_t* len;
  union covalue
  { array_pmv avalue;  
     list_pmv lvalue; 
  } value;

  size_t  adressing_last_series;  // продолжительность последней серии адресации
  size_t  inserting_last_series;  // продолжительность последней серии вставок

  size_t  adressing_almost;       // число раз обращений по индексам
  size_t  inserting_almost;       // число раз вставления в середину
};
//-------------------------------------------------------------------------------
void   container_create (container* Container, conts_enum type, size_t size);
void   container_clear  (container* Container);
void   container_free   (container* Container);

void   container_copy (container* scr, container* dest);

myvar** container_exact   (container* Container, size_t    index);
myvar*  container_extract (container* Container, size_t    index);
myvar*  container_insert  (container* Container, size_t    index, myvar *p_elem);
myvar*  container_find    (container* Container, size_t *p_index, myvar *p_elem, int (*compare)(myvar**, myvar**));
//-------------------------------------------------------------------------------
#endif
