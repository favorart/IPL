#include "header.h"
#include "lexicalCheck.h"

typedef    myvar* (*pf2type) (myvar*,myvar*);
//-------------------------------------------------------------------------------
/*inline*/ myvar* _toTypeInt (myvar*,myvar*);
/*inline*/ myvar* _toTypeStr (myvar*,myvar*);
/*inline*/ myvar* _toTypeFlt (myvar*,myvar*);
/*inline*/ myvar* _toTypeCnt (myvar*,myvar*);
/*inline*/ myvar* _toTypeJnt (myvar*,myvar*);
//-------------------------------------------------------------------------------
myvar*  toTypeAny (names_table *Names_table, myvar *to, myvar *from)
{ pf2type  arr_pfunc[] = {NULL, _toTypeInt, _toTypeStr, _toTypeFlt,
                          NULL, _toTypeCnt, _toTypeJnt };

		if( to->type == from->type )
		{ myvar_copy_value (from,to); return to; }
		if( (to->type == Not) || (from->type == Not) || (to->vtype != Variable) ||
			   (to->type == Fnc) || (from->type == Fnc) || (to-> type  > Jnt) )
			SetCorrectError (myvar_name (to),ERR_TYPE);

		return (*arr_pfunc[to->type])(to,from);
}
//-------------------------------------------------------------------------------
INTEGER StrToInt (char* str)
{ INTEGER inumber=0;
		if( sscanf (str,I_OUTLINE,&inumber) != 1 )
		{ SetWarning (str,WRG_CNVRT); inumber=0; }
		return inumber;
}
FLOAT   StrToFlt (char* str)
{ FLOAT fnumber=0.;
		if( sscanf (str,F_OUTLINE,&fnumber) != 1 )
		{ SetWarning(str,WRG_CNVRT); fnumber=0.; }
		return fnumber;
}
//-------------------------------------------------------------------------------
myvar*  _toTypeInt (myvar *to, myvar *from)
{ switch (from->type)
		{ case Int: to->value.ivalue =          (from->value.ivalue); break;
				case Str: to->value.ivalue = StrToInt (from->value.svalue); break;
				case Flt: to->value.ivalue = (INTEGER)(from->value.fvalue); break;
				case Cnt: 
					if( *from->value.cvalue->len == 1 ) _toTypeInt (to,container_extract (from->value.cvalue,0));
					else     { SetWarning (myvar_name (to),WRG_CNVRT); to->value.ivalue = 0; } break;
				case Jnt: { SetWarning (myvar_name (to),WRG_CNVRT); to->value.ivalue = 0; } break;
				default : SetCorrectError (myvar_name (to),ERR_CAST); break;
		}
		return to;
}
myvar*  _toTypeStr (myvar *to, myvar *from)
{ switch (from->type)
		{ case Int: case Flt:
					{ char* sh = (char*)malloc( sizeof(char)*BUFLEN );
							if( !sh ) SetError (ERR_MEM);
							if( from->type == Int ) sprintf (sh,I_OUTLINE,from->value.ivalue);
							else                    sprintf (sh,F_OUTLINE,from->value.fvalue);
							to->value.svalue = sh;
					}
					break;
				case Str: myvar_copy_value (from,to);
					break;
				case Cnt: case Jnt:
					{ myvar *zstr = myvar_create (ZERO_STR_NAME,Str);
							SetWarning (myvar_name (to),WRG_CNVRT);
							myvar_copy_value (zstr,to);
					}
					break;
				default : SetCorrectError (myvar_name (to),ERR_CAST);
					break;
		}
		return to;
}
myvar*  _toTypeFlt (myvar *to, myvar *from)
{ switch (from->type)
		{ case Int: to->value.fvalue =   (FLOAT)(from->value.ivalue); break;
				case Str: to->value.fvalue = StrToFlt (from->value.svalue); break;
				case Flt: to->value.fvalue =          (from->value.fvalue); break;
				case Cnt: 
					if( *from->value.cvalue->len == 1 ) _toTypeFlt (to,container_extract (from->value.cvalue,0));
					else     { SetWarning (myvar_name (to),WRG_CNVRT); to->value.fvalue = 0.; } break;
				case Jnt: { SetWarning (myvar_name (to),WRG_CNVRT); to->value.fvalue = 0.; } break;
				default :   SetCorrectError (myvar_name (to),ERR_CAST); break;
		}
		return to;
}
myvar*  _toTypeCnt (myvar *to, myvar *from)
{ if( from->type != Cnt )
		{ container *cont;
				if( !(cont = (container*)malloc( sizeof(container) )) )
					SetCorrectError (myvar_name (to),ERR_MEM);
				container_create (cont,CONT_TYPE_DLIST, 0);
				container_insert (cont,0,myvar_create_value (from->value,from->type));
				to->value.cvalue = cont;
		}
		else myvar_copy_value (from,to);
		return to;
}
myvar*  _toTypeJnt (myvar *to, myvar *from)
{      if( from->type == Jnt )
			myvar_copy_value (from,to);
		else if( from->type == Cnt )
		{ if( *from->value.cvalue->len%2 == 1 )
			 { myvar *zjnt = myvar_create (ZERO_JNT_NAME,Jnt);
						SetWarning (myvar_name (to),WRG_CNVRT);
						myvar_copy_value (zjnt,to);
				}
				else
				{ myvar *key, *val, *cur = myvar_create (ZERO_JNT_NAME,Jnt);

						while( *from->value.cvalue->len )
						{ key = container_extract (from->value.cvalue,0);
								val = container_extract (from->value.cvalue,0);
								joint_insert (cur->value.jvalue,key,val);
						}

						to->value.jvalue = cur->value.jvalue;
						constants_extract (myvar_name (cur),cur->type);
				}
		}
		else if( isSimpleType (from->type) )
		{ myvar *zjnt = myvar_create (ZERO_JNT_NAME,Jnt);
				SetWarning (myvar_name (to),WRG_CNVRT);
				myvar_copy_value (zjnt,to);
		}
		else SetCorrectError (myvar_name (to),ERR_CAST);
		return to;
}
//-------------------------------------------------------------------------------
