#include "header.h"
#include "interpreter.h"
#include "lexicalCheck.h"

//-------------------------------------------------------------------------------
names_table** _Names_table(void)
{ static names_table *Names_table; 
  return &Names_table;
}
//-------------------------------------------------------------------------------
int  isNLine (char  sgn) { return (sgn == SYMB_NLINE); }
int  isSlash (char  sgn) { return (sgn == SYMB_SLASH); }
//-------------------------------------------------------------------------------
int isLetter   (char sgn)
{ unsigned int i; char *str = (*_Names_table())->Lex.Alphabet[ALPH_LETTERS];
  for(i=0; str[i]; i++)
   if(str[i] == sgn)
    return 1;
  return 0;
}
int isSign     (char sgn)
{ unsigned int i;
  for(i=0; (*_Names_table())->Lex.Alphabet[ALPH_SIGNS][i]!=0;i++)
   if((*_Names_table())->Lex.Alphabet[ALPH_SIGNS][i] == sgn)
    return 1;
  return 0;
}
int isDigit    (char sgn)
{ unsigned int i;
  for(i=0; (*_Names_table())->Lex.Alphabet[ALPH_DIGITS][i]!=0;i++)
   if((*_Names_table())->Lex.Alphabet[ALPH_DIGITS][i] == sgn)
    return 1;
  return 0;
}
int isQuote    (char sgn)
{ return ( (*_Names_table())->Lex.Alphabet[ALPH_QUOTE    ][0] == sgn ) ? (1) : (0); }
int isDecPoint (char sgn)
{ return ( (*_Names_table())->Lex.Alphabet[ALPH_DEC_POINT][0] == sgn ) ? (1) : (0); }
//-------------------------------------------------------------------------------
int     isInteger    (char* buf)
{ if( !buf || !(*buf) ) return 0;
  while( *buf ) if( !isDigit (*(buf++)) ) return 0;
  return 1;
}
int     isFloatPoint (char* buf)
{ int flag_dot = 0;
  if( !buf || !(*buf) || !isDigit(*buf) ) return 0;

  while( *(++buf) )
   if(  isDecPoint (*buf) ) flag_dot++;
   else if( !isDigit    (*buf) ) return 0;

  if( !isDigit (*(buf++)) || (flag_dot != 1) ) return 0;
  return 1;
}
myvar*  isRegWord    (char* buf)
{ return  hash_lookup ((*_Names_table())->pReg_Words,buf,NULL,HASH_ADD_DISABLE,0); }
myvar*  isIdentifier (char* buf)
{ return  variables_lookup (buf,NULL,HASH_ADD_DISABLE); }
//-------------------------------------------------------------------------------
// обработка escape-последовательностей
char*    EscapeString (char* buf)
{ unsigned int i,k;
  for(i=0, k=0; i<strlen(buf)+1; i++, k++)
  { if(buf[i]=='\\')
    { switch(buf[++i])
      { case 'a': buf[k] = 0x07;   break;
        case 'b': buf[k] = 0x08;   break;
        case 't': buf[k] = 0x09;   break;
        case 'n': buf[k] = 0x0A;   break;
        case 'v': buf[k] = 0x0B;   break;
        case 'f': buf[k] = 0x0C;   break;
        case 'r': buf[k] = 0x0D;   break;
        default : buf[k] = buf[i]; break;
      }
    }
    else buf[k] = buf[i];
  }
  return buf;
}
void    ToLowerString (char* buf)
{ if( (*_Names_table())->pUsrConf->Syntax.no_mean_reg_of_letter ) return;
  while( *buf = tolower (*buf) ) buf++;
}
//-------------------------------------------------------------------------------
