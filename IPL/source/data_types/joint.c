#include "header.h"

// simple numbers content
unsigned int  get_hash_size (unsigned int size)
{ unsigned int simple_arr[] = {    797,   1531,   2999,   6011, 
                                 12391,  25013,  54983, 100019, 
                                204983, 410029, 820037, 999983 };
  unsigned int i,simple_len = sizeof(simple_arr) / sizeof(*simple_arr);

  for(i=0; i<simple_len; i++)
   if( size < simple_arr[i] ) return simple_arr[i];
  return simple_arr[simple_len-1];
}
//------------------------------------------------------------------------------------
 #define JOINT_HASH_MULTIPLIER 27
// вычисление хэша 
unsigned int  joint_hash (joint *Joint, myvar *key)
{ ///* **************************
  unsigned int h=0; unsigned char *p;
  for(p = (unsigned char*)key->name; *p!=0; p++)
   h = JOINT_HASH_MULTIPLIER * h + *p;
  return (h % Joint->size);
  //************************** */
  
  /*if( (key->vtype != Variable) &&
      (key->vtype != Constant) )
   SetCorrectError (myvar_name (key),ERR_NAME);
  if( !isSimpleType (key->type) )
   SetCorrectError (myvar_name (key),ERR_TYPE);

  if( key->vtype == Variable )
   key = myvar_create_value (key->value,key->type);

  return ( (unsigned int)key % Joint->size );*/
}
//-----------------------------------------------
int    joint_mvcmp (myvar* a, myvar *b)
{ //return (a == b);
  return !strcmp (myvar_name (a),myvar_name (b));
}
//-----------------------------------------------
void   joint_create (joint *Joint, unsigned int size)
{ if( !Joint ) { SetError (ERR_PAR); return; }
  memset (Joint,0,sizeof(joint));

  Joint->hash = joint_hash;
  Joint->size = 797; //(size) ? (size) : (get_hash_size (0));
  Joint->len  = 0;
#ifdef JOINT_PTR_NODE
  Joint->content = (joint_node**)calloc( Joint->size,sizeof(joint_node*) );
#else // NODE
  Joint->content = (joint_node* )calloc( Joint->size,sizeof(joint_node ) );
#endif
  if( !Joint->content ) SetError (ERR_MEM);
}
void   joint_free   (joint *Joint)
{ joint_clear (Joint);
  if( Joint->content ) free (Joint->content);
  memset (Joint,0,sizeof(joint));
}
void   joint_clear  (joint *Joint)
{ if( !Joint ) { SetError (ERR_PAR); return; }
#ifdef JOINT_PTR_NODE
  if( Joint->len )
  { joint_node *node = joint_next (Joint,JOINT_ITER_BASE);
    while( node ) { free (node); node = joint_next (Joint,JOINT_ITER_NEXT); }
  }
#else // NODE
  if( Joint->len ) memset (Joint->content,0,sizeof(joint_node)*Joint->size);
#endif
  Joint->len = 0;
}
void   joint_copy   (joint *scr, joint *dest)
{ if( !scr || !dest ) { SetError (ERR_PAR); return; }

  dest->hash = scr->hash;
  if( scr->size != dest->size )
  { joint_free (dest);
  
    dest->size = scr->size;
    dest->len  = 0;
#ifdef JOINT_PTR_NODE
  dest->content = (joint_node**)calloc( scr->size,sizeof(joint_node*) );
#else // NODE
  dest->content = (joint_node* )calloc( scr->size,sizeof(joint_node ) );
#endif
  if( !dest->content ) { SetError (ERR_MEM); return; }
  }
  else joint_clear (dest);

  if( scr->len )
  { joint_node *node; joint_iter iter = {0};
    while( (node = joint_next (scr,&iter)) )
     joint_insert (dest,node->key,node->val);
  } // end if
}
//-----------------------------------------------
#define JOINT_SIZE_RATIO  0.7
int    joint_extend (joint *Joint)
{ float ratio = (float)Joint->len / Joint->size;
  if( ratio > JOINT_SIZE_RATIO )
  { joint J = {0};
    J.size = get_hash_size (Joint->size);
    J.hash = Joint->hash;
#ifdef JOINT_PTR_NODE
    J.content = (joint_node**)calloc( J.size,sizeof(joint_node*) );
#else // NODE
    J.content = (joint_node* )calloc( J.size,sizeof(joint_node ) );
#endif
    if( !J.content ) { SetError (ERR_MEM); return 1; }
     
    { joint_node *node; joint_iter iter = {0};
      while( node = joint_next (Joint,&iter) )
       joint_insert (&J,node->key,node->val);
    }
    joint_free (Joint); *Joint = J;
    return 1;
  } // if ratio
  return 0;
}
//-----------------------------------------------
#define JOINT_PROBING_STEP  2
// просто записываем указатели
myvar*  joint_insert  (joint *Joint, myvar *key, myvar *val)
{ joint_node* node = NULL; unsigned int h;
  if( !Joint || !key || !val ) { SetError (ERR_PAR); return NULL; }

  h = Joint->hash (Joint,key);
#ifdef JOINT_PTR_NODE
  for(node = Joint->content[h]; node; node = node->next)
   if( joint_mvcmp (key,node->key) )
    return (node->val = val);

  node = (joint_node*)malloc( sizeof(joint_node) );
  if( !node ) { SetError(ERR_MEM); return NULL; }
  // добавляется в начало списка
  node->next = Joint->content[h]; 
  Joint->content[h] = node;
#else // NODE
  { unsigned int d = JOINT_PROBING_STEP;
    node = &Joint->content[h];
    while( node->free )
    { if( joint_mvcmp (key,node->key) ) return (node->val = val);
      if( (h+=d) >= Joint->size ) h=0;
      node = &Joint->content[h];
    }     
  } node->free = 1;
#endif
  node->val = myvar_create_value (val->value,val->type); // val;
  node->key = key;
  Joint->len++;

  //if( joint_extend (Joint) )
  // return *joint_exact (Joint,key);
  return node->val;
}
myvar** joint_exact   (joint *Joint, myvar *key)
{ joint_node* node = NULL; unsigned int h;
  if( !Joint || !key ) { SetError (ERR_PAR); return NULL; }

  h = Joint->hash (Joint,key);
#ifdef JOINT_PTR_NODE
  for(node = Joint->content[h]; node; node = node->next)
   if( joint_mvcmp (key,node->key) ) return &node->val;
#else
  { unsigned int d = JOINT_PROBING_STEP;
    node = &Joint->content[h];
    while( node->free )
    { if( joint_mvcmp (key,node->key) )
       return &node->val;
      if( (h+=d) >= Joint->size ) h=0;
      node = &Joint->content[h];
    }     
  }
#endif
  return NULL;
}
myvar*  joint_extract (joint *Joint, myvar *key)
{ joint_node *node; myvar *val = NULL; unsigned int h;
  if( !Joint || !Joint->len || !key ) SetError (ERR_PAR);

  h = Joint->hash (Joint,key);
#ifdef JOINT_PTR_NODE
  { joint_node *prev = NULL;

    for(node = Joint->content[h]; node; prev = node, node = node->next)
     if( joint_mvcmp (key,node->key) )
     { val = node->val;
       // переставить ссылки
       if( prev ) prev->next  = node->next;
       else Joint->content[h] = node->next;
       free (node); goto END;
     }
  }
#else
  { unsigned int d = JOINT_PROBING_STEP;
    joint_node *tnode, *pnode = NULL;

    node = &Joint->content[h];
    while( node->free )
    { if( joint_mvcmp (key,node->key) )
      { val = node->val; tnode = node;
        memset (node,0,sizeof(joint_node));
      }
      if( (h+=d) >= Joint->size ) h=0;
      node = &Joint->content[h];

      if( val )
      { unsigned int hs =  Joint->hash (Joint,key);
        while( hs == Joint->hash (Joint,node->key) )
        { if( (h+=d) >= Joint->size ) h=0;
          pnode = node; node = &Joint->content[h];      
        }

        if( pnode )
        { tnode = pnode; memset (pnode,0,sizeof(joint_node)); }
        goto END;
      }
    }     
  }
#endif
END:;
  return val;
}
// joint_exact == joint_find только поиск не по key, а по val (exactly the same)
myvar*  joint_find    (joint *Joint, myvar *val, int (*compar)(myvar**,myvar**))
{ if( !Joint || !val ) { SetError (ERR_PAR); return NULL; }

  if( Joint->len )
  { joint_node* node; joint_iter iter = {0};
    while( node = joint_next (Joint,&iter) )
     if( (*compar) (&node->val,&val) ) return node->key;
  }
  return NULL;
}
//------------------------------------------------------------------------------------
joint_node* joint_next (joint *Joint, joint_iter* iter)
{ 
#ifdef JOINT_PTR_NODE
  { if( !iter->i && !iter->node )
   iter->node = Joint->content[0];
  else if( iter->node )
   iter->node = iter->node->next;

  while( !iter->node )
   if( iter->i < (Joint->size-1) )
    iter->node = Joint->content[++iter->i];
   else { iter->node = NULL; iter->i = 0; break; }
  return iter->node;
#else
  { joint_node* node;
    while( !Joint->content[iter->i++].free ) 
    { if(iter->i == Joint->size) break; }

    if( iter->i < Joint->size )
     node = &Joint->content[iter->i-1];
    else { node = NULL; iter->i=0; }

    return node;
  }
#endif
}
//------------------------------------------------------------------------------------
