#include "header.h"
#include "joint.h"

//-------------------------------------------------------------------------------
char*   myvar_name  (myvar *cur)
{ return ( (cur->vtype == Constant) && (cur->type == Str) ) ? (cur->value.svalue) : (cur->name); }
// создание myvar по имени и по значению
myvar*  myvar_create       (char* buf,    tenum type)
{ myvar var = {0}, *cur = NULL;
  if( !buf ) SetError(ERR_PTR);

  if( cur = constants_lookup (buf,NULL,HASH_ADD_DISABLE,type) )
   return cur;

  var. type = type;
  var.vtype = Constant;

  switch( type )
  { case Not:
     strcpy (var.name,buf);
     break;
  
    case Int:
     sscanf(buf,I_OUTLINE,&var.value.ivalue);
     strcpy(var.name,buf);
     break;

    case Str:
     var.value.svalue = (char*)malloc( sizeof(char)*(strlen(buf)+1) );
     if( !var.value.svalue ) SetError(ERR_MEM);
     strcpy(var.value.svalue,buf);
     break;

    case Flt:
     sscanf(buf,F_OUTLINE,&var.value.fvalue);
     strcpy(var.name,buf);
     break;

    case Fnc:
     SetError (ERR_PAR);
     break;

    case Cnt:
     var.value.cvalue = (container*)calloc( 1,sizeof(container) );
     if( !var.value.cvalue )
       SetError(ERR_MEM);
     container_create ( var.value.cvalue, CONT_TYPE_DLIST, 0 );
     strcpy(var.name,buf);
     break;

    case Jnt:
     var.value.jvalue = (joint*)calloc( 1,sizeof(joint) );
     if( !var.value.jvalue ) SetError(ERR_MEM);
     joint_create (var.value.jvalue,0);
     strcpy(var.name,buf);
     break;

    default: SetCorrectError (buf,ERR_TYPE); break;
  }
  return constants_lookup (myvar_name(&var),&var,HASH_ADD_ENABLE,var.type);
}
myvar*  myvar_create_value (tvalue value, tenum type)
{ myvar var = {0}; char *name = var.name;
  var.  type = type;
  var. vtype = Constant;

  switch( type )
  { case Int:
     if( value.ivalue )
     { var.value.ivalue = value.ivalue; 
       sprintf (var.name,I_OUTLINE,var.value.ivalue);
     } else name = ZERO_INT_NAME;
     break;

    case Str:
     if( value.svalue && *(value.svalue) )
     { var.value.svalue = (char*)malloc( sizeof(char)*(strlen(value.svalue)+1) );
       if( !var.value.svalue ) SetError(ERR_MEM);
       strcpy(var.value.svalue,value.svalue);
       name = var.value.svalue;
     } else name = ZERO_STR_NAME;
     break;

    case Flt:
     if( value.fvalue )
     { var.value.fvalue = value.fvalue;
       sprintf (var.name,F_OUTLINE,var.value.fvalue);
     } else name = ZERO_FLT_NAME;
     break;

    case Fnc:
     SetError (ERR_PAR);
     break;

    case Cnt:
     if( value.cvalue && *(value.cvalue->len) )
     { var.value.cvalue = (container*)calloc( 1,sizeof(container) );
       if( !var.value.cvalue ) SetError(ERR_MEM);
       container_copy ( value.cvalue,var.value.cvalue );
       container_const_name (*_Names_table(),var.name);
     } else name = ZERO_CNT_NAME;
     break;

    case Jnt:
     if( value.jvalue && value.jvalue->len )
     { var.value.jvalue = (joint*)calloc( 1,sizeof(joint) );
       if( !var.value.jvalue ) SetError(ERR_MEM);
       joint_copy (value.jvalue,var.value.jvalue);
       joint_const_name (*_Names_table(),var.name);
     } else name = ZERO_JNT_NAME;
     break;

    default: SetError (ERR_TYPE); break;
  }
  return constants_lookup (name,&var,HASH_ADD_ENABLE,var.type);
}
// очистить myvar
void    myvar_free       (myvar* var)
{ if( !var ) SetError (ERR_PTR);
  myvar_free_value (var);
  memset (var,0,sizeof(myvar));
}
void    myvar_free_value (myvar* var)
{ switch( var->type )
  { case Not: case Int: case Flt: case Sgn:
       break;

    case Str:
     if( var->value.svalue ) 
     { free (var->value.svalue); var->value.svalue = NULL;
     } break;

    case Fnc:  
     if(var->value.pvalue)
     { procedure_free (var->value.pvalue);
       var->value.pvalue = NULL;
     } break;  

    case Cnt: 
     if( var->value.cvalue )
     { container_free (var->value.cvalue);
       free (var->value.cvalue); var->value.cvalue = NULL;
     } break;

    case Jnt: 
     if( var->value.jvalue )
     { joint_free (var->value.jvalue); 
       free (var->value.jvalue); var->value.jvalue = NULL;
     } break;

    default: SetError(ERR_PAR); break;
  } // end switch
}
void    myvar_free_count (myvar* var, int flag)
{ if( !isEnumerable (var->type) ) return;
  else if( var->type == Cnt )
  { size_t i, n = *var->value.cvalue->len;

    for(i = 0; i < n; ++i)
     if( isEnumerable ((*container_exact (var->value.cvalue,i))->type) )
      myvar_free_count (container_extract (var->value.cvalue,i), MV_FREE_SELF_Y);

    container_clear (var->value.cvalue);
  } // end else if
  else if( var->type == Jnt )
  { joint_node *cur, jnode = {0}; joint_iter iter = {0};
    joint *Joint = var->value.jvalue;
    myvar *key, *val; 
    
    jnode.free = 1;
    while( cur = joint_next (Joint,&iter) )
     if( isEnumerable (cur->key->type) ||
         isEnumerable (cur->val->type) )
     { key = cur->key; val = joint_extract (Joint,key);
       if( isEnumerable (key->type) ) myvar_free_count (key, MV_FREE_SELF_Y);
       if( isEnumerable (val->type) ) myvar_free_count (val, MV_FREE_SELF_Y);
       cur = &jnode;
     } // end if

    joint_clear (var->value.jvalue);
  } // end else if
  if( flag == MV_FREE_SELF_Y )
  { myvar_free_value  (var);
    constants_extract (var->name,var->type);
  }
}
// копировать структуру myvar
void    myvar_copy       (myvar *scr, myvar *dest)
{ myvar_copy_value (scr,dest);

  dest-> vtype = scr-> vtype;
  dest->sytype = scr->sytype;
  dest->rwtype = scr->rwtype;
  strcpy (dest->name,scr->name);
}
void    myvar_copy_value (myvar *scr, myvar *dest)
{ if( !scr || !dest ) SetError (ERR_PTR);

  if( dest->type != scr->type )
  { myvar_free_value (dest);
    dest->type = scr->type;
  }

  switch( scr->type )
  { case Not: case Sgn: break;
  
    case Int:
     dest->value.ivalue = scr->value.ivalue;
     break;

    case Flt:
     dest->value.fvalue = scr->value.fvalue;
     break;

    case Str:
     if( (dest->type != scr->type) || !dest->value.svalue || 
         (strlen (dest->value.svalue) < strlen (scr->value.svalue)) )
     { if( dest->value.svalue ) free (dest->value.svalue);
       dest->value.svalue = (char*)malloc( sizeof(char)*(strlen(scr->value.svalue)+1) );
       if( !dest->value.svalue ) SetError (ERR_MEM);
     }
     strcpy (dest->value.svalue,scr->value.svalue);
     break;

    case Fnc:
     if( (dest->type != scr->type) || !dest->value.pvalue )
      dest->value.pvalue = procedure_create ();
     procedure_copy (scr->value.pvalue,dest->value.pvalue);
     break;  

    case Cnt:
     if( (dest->type != scr->type) || !dest->value.cvalue )
      if( !(dest->value.cvalue = (container*)calloc( 1,sizeof(container) )) )
       SetError (ERR_MEM);
     container_copy (scr->value.cvalue,dest->value.cvalue);
     break;

    case Jnt:
     if( (dest->type != scr->type) || !dest->value.jvalue )
      if( !(dest->value.jvalue = (joint*)calloc( 1,sizeof(joint) )) )
       SetError (ERR_MEM);
     joint_copy (scr->value.jvalue,dest->value.jvalue); 
     break;

    default : SetError(ERR_PAR); break;
  } // end switch
}
//-------------------------------------------------------------------------------
int     myvar_compare_equal (myvar **a, myvar **b)
{ if( (*a)->type != (*b)->type ) return 1;

  switch ((*a)->type)
  { case Int: return (          (*a)->value.ivalue == (*b)->value.ivalue  ) ? (0) : (1);
    case Flt: return (          (*a)->value.fvalue == (*b)->value.fvalue  ) ? (0) : (1);
    case Str: return ( !strcmp ((*a)->value.svalue,   (*b)->value.svalue) ) ? (0) : (1);
    case Cnt: 
     if( *(*a)->value.cvalue->len != *(*b)->value.cvalue->len ) return 1;
     else
     { unsigned int i; myvar *par1, *par2;
       for(i=0; i<*(*a)->value.cvalue->len; i++)
       { par1 = *container_exact ((*a)->value.cvalue,i);
         par2 = *container_exact ((*b)->value.cvalue,i);
         if( myvar_compare_equal (&par1,&par2) ) return 1;
       }
       return 0;
     }
    case Jnt:
     if( (*a)->value.jvalue->len != (*b)->value.jvalue->len ) return 1;
     else
     { joint_node *acur, *bcur; joint_iter aiter = {0}, biter = {0};
       while( (acur = joint_next ((*a)->value.jvalue,&aiter)) &&
              (bcur = joint_next ((*b)->value.jvalue,&biter)) )
        if( myvar_compare_equal (&acur->val,&bcur->val) ||
            myvar_compare_equal (&acur->key,&bcur->key) ) return 1;
       return 0;
     }
    default : SetCorrectError ((*a)->name,ERR_TYPE); return 1;
  }
}
//-------------------------------------------------------------------------------
INTEGER  toTypeInt (myvar* from);
char*    toTypeStr (myvar* from);
FLOAT    toTypeFlt (myvar* from);
//-------------------------------------------------------------------------------
int   sign (int x)
{ return (x<0) ? (-1) : ( (x>0)?(1):(0) ); }
/*
                             StrInt  2-1 = 1
         IntInt  1-1 = 0     StrFlt  2-1 = 1
         IntFlt  1-1 = 0     CntInt  5-1 = 1
         FltInt  1-1 = 0     CntFlt  5-1 = 1
         FltFlt  1-1 = 0     CntStr  5-2 = 1
         StrStr  2-2 = 0     JntInt  6-1 = 1
         CntCnt  5-5 = 0     JntFlt  6-1 = 1
         JntJnt  6-6 = 0     JntStr  6-2 = 1
                             JntCnt  6-5 = 1
*/
int   myvar_compare_precedence (tenum atype,tenum btype)
{ int at, bt;
  at = (int)(atype == Flt)?(Int):(atype);
  bt = (int)(btype == Flt)?(Int):(btype);
  return sign (at - bt); 
}
//-------------------------------------------------------------------------------
int   myvar_compare        (myvar **a, myvar **b)
{ int result = myvar_compare_precedence ((*a)->type,(*b)->type);

  if( !(*a)->type ) SetCorrectError (myvar_name (*a),ERR_TYPE);
  if( !(*b)->type ) SetCorrectError (myvar_name (*b),ERR_TYPE);

  if( !result )
  { switch( (*a)->type )
    { case Int: case Flt:
       if( ((*b)->type == Int) && ((*a)->type == Int) )
       { INTEGER ai = (*a)->value.ivalue;
         INTEGER bi = (*b)->value.ivalue;
         result = (ai == bi)?(0):( (ai > bi)?(1):(-1) );
       }
       else
       { FLOAT   af = ( (*a)->type == Flt ) ? ( (*a)->value.fvalue ) : ( toTypeFlt (*a) );
         FLOAT   bf = ( (*b)->type == Flt ) ? ( (*b)->value.fvalue ) : ( toTypeFlt (*b) );
         result = (af == bf)?(0):( (af > bf)?(1):(-1) );
       }
       break;

      case Str:
       result = strcmp ((*a)->value.svalue,(*b)->value.svalue);
       break;

      case Cnt: 
            if( *(*a)->value.cvalue->len > *(*b)->value.cvalue->len ) result =  1;
       else if( *(*a)->value.cvalue->len < *(*b)->value.cvalue->len ) result = -1;
       else
       { unsigned int i;
         for(i=0; i<*((*a)->value.cvalue->len); i++)
          if( result = myvar_compare (container_exact ((*a)->value.cvalue,i),
                                      container_exact ((*b)->value.cvalue,i)) )
           break;
       }
       break;

      case Jnt:
            if( (*a)->value.jvalue->len > (*b)->value.jvalue->len ) result =  1;
       else if( (*a)->value.jvalue->len < (*b)->value.jvalue->len ) result = -1;
       else
       { joint_node *acur, *bcur; joint_iter aiter = {0}, biter = {0};
         while( (acur = joint_next ((*a)->value.jvalue,&aiter)) &&
                (bcur = joint_next ((*b)->value.jvalue,&biter)) )
         { if( result = myvar_compare (&acur->val,&bcur->val) ) break;
           if( result = myvar_compare (&acur->key,&bcur->key) ) break;
         }
       }
       break;

       default: SetCorrectError ((*a)->name,ERR_TYPE); break;
    }
  }
  return result;
}
int   myvar_compare_revr   (myvar **a, myvar **b)
{ return ( -myvar_compare(a,b) ); }
//-------------------------------------------------------------------------------
