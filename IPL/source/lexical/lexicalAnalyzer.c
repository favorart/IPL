#include "header.h"
#include "lexicalCheck.h"
#include "lexicalConstruct.h"
#include "interpreter.h"
#include "interpreterLexical.h"

#include "nmList.h"
//-------------------------------------------------------------------------------
// считать следующую строку в выражение
myvar* laNextWord (list_pmv *Sentence)
{ myvar *cur;
  if ( !list_pmv_extract (Sentence, 0U, &cur) ) 
  { if( laLexicalAnalyzer (*_Names_table(),Sentence) && !Sentence->len )
      SetError(ERR_TERM);
    list_pmv_extract (Sentence, 0, &cur);
  }
  return cur;
}
// посмотреть следующее слово
myvar* laNextExact (list_pmv *Sentence)
{ myvar *cur;

  if( Sentence->len )
   cur = *list_pmv_exact (Sentence,0);
  else if( !laLexicalAnalyzer (*_Names_table(),Sentence) || Sentence->len )
   cur = *list_pmv_exact (Sentence,0);
  else
   cur = NULL;

  return cur;
}
//-------------------------------------------------------------------------------
myvar*  laReWoConstruct (lexical_data *Lex, int flag);
myvar*  laSignConstruct (lexical_data *Lex);

int     laParcerProgram (names_table *Names_table, char *buf, lenum *type);
int     laParcerComment (names_table *Names_table, int   sytype);
//-------------------------------------------------------------------------------
//---*** Lexical Analyzer ***----------------------------------------------------

// если конец файла - вернет 1, иначе 0;
int     laLexicalAnalyzer (names_table *Names_table, list_pmv *List_atoms)
{ int result = 0; char *wbuf = Names_table->Lex.word_buf, *cbuf;
  lenum type = LA_NO; wenum wait = WAIT_NO; myvar *pvar = NULL;
  //----------------------------
  while( !result && (wait || (type != LA_LINE) || !List_atoms->len) )
  { type = LA_NO;

    if( laParcerProgram (Names_table,wbuf,&type) ) result = 1;
    wbuf = Names_table->Lex.word_buf;
    cbuf = Names_table->Lex.comp_buf;
    //----------------------------
    if( (wait == WAIT_NWORD) && (type != LA_WORD) && (type != LA_LINE) )
    { wait = WAIT_NO;
      while( Names_table->Lex.nwords_storage->len )
       if( pvar = laReWoConstruct (&Names_table->Lex,(result)?(REWO_ENDF):(REWO_STOP)) )
       { laGrbgToVarName (Names_table,List_atoms);

         wait = laReWoComm_Wait (pvar,wait);
         if( (wait == WAIT_CCOMM) || (wait == WAIT_NLINE) ) break;
         list_pmv_insert (List_atoms,-1,&pvar);
       }
      laGrbgToVarName (Names_table,List_atoms);
    }
    //----------------------------
    switch (type)
    { default: break;
    
      case LA_LINE:
       CurError()->line++;
       if( wait == WAIT_NLINE ) wait = WAIT_NO;
       break;

      case LA_STRG:
       if( !wait )
       { pvar = myvar_create (EscapeString (wbuf),Str);
         list_pmv_insert (List_atoms,-1,&pvar);
       }
       break;

      case LA_NUMB:
       if( !wait )
       {      if( isFloatPoint (wbuf) )  pvar = myvar_create (wbuf,Flt);
         else if( isInteger    (wbuf) )  pvar = myvar_create (wbuf,Int);
         else if( isDecPoint (wbuf[strlen(wbuf)-1]) && isDigit (*wbuf) )
         { pvar = myvar_create (wbuf,Not); }
         else if( isDecPoint (wbuf[strlen(wbuf)-1]) && (strlen (wbuf) == 1) )
         { pvar = isRegWord (wbuf); }
         else { SetCorrectError (wbuf,ERR_NAME); result = 2; goto END; }
         list_pmv_insert (List_atoms,-1,&pvar);
       }
       break;

      case LA_SIGN:
       if( !wait )
        while( strlen (wbuf) )
        { if( !(pvar = laSignConstruct (&Names_table->Lex) ))
          { SetCorrectError (wbuf,ERR_NAME); result = 2; goto END; }

          if( wait = laReWoComm_Wait (pvar,wait) ) break;
          list_pmv_insert (List_atoms,-1,&pvar);
        }
       break;

      case LA_WORD:
       ToLowerString (wbuf);
       if( strlen(wbuf) >= NAMELEN )
       { SetCorrectError (wbuf,ERR_NAME); result = 2; goto END; }

       if( pvar = laReWoConstruct (&Names_table->Lex,REWO_MORE) )
       { laGrbgToVarName (Names_table,List_atoms);

         wait = laReWoComm_Wait (pvar,wait);
         if( (wait == WAIT_CCOMM) || (wait == WAIT_NLINE) ) break;
         list_pmv_insert (List_atoms,-1,&pvar);
       }
       wait = ( !Names_table->Lex.nwords_storage->len ) ? (WAIT_NO) : (WAIT_NWORD);
       break;
    } // end switch

    if( (wait == WAIT_CCOMM) || (wait == WAIT_NLINE) )
    { if( laParcerComment (Names_table,pvar->sytype) )
      { result = 1; break; }
      wait = WAIT_NO;
    }
  } // end while
  //----------------------------
END:
  return result;
}
//-------------------------------------------------------------------------------
int     laParcerProgram   (names_table *Names_table, char *buf, lenum *type)
{ int result = 0; unsigned int i=0; char c; lenum flag = LA_NO;
  //----------------------------
  while( fscanf (Names_table->pfScript,"%c",&c) == 1 )
  {      if( (flag == LA_STRG) || ((flag == LA_NO) && (isQuote (c))) ) 
    { buf = laLexIncWordBuf (&Names_table->Lex,i);
      if( flag == LA_STRG )
       if( isQuote (c) && !isSlash(buf[i-1]) )
       { buf[i] = flag = 0; *type = LA_STRG; }
       else buf[i++] = c;
      else flag = LA_STRG;
    }
    else if( (flag == LA_NUMB) || ((flag == LA_NO) && (isDigit (c) || isDecPoint (c))) )
    { if( !isDigit (c) && !isDecPoint (c) )
      { buf[i] = flag = 0; *type = LA_NUMB; ungetc (c,Names_table->pfScript); }
      else
      { if( flag != LA_NUMB ) flag = LA_NUMB; buf[i++] = c; }    
    }
    else if( (flag == LA_SIGN) || ((flag == LA_NO) && (isSign  (c))) )
    { if( !isSign (c) )
      { buf[i] = flag = 0; *type = LA_SIGN; ungetc (c,Names_table->pfScript); }
      else
      { if( flag != LA_SIGN ) flag = LA_SIGN; buf[i++] = c; }  
    }
    else if( (flag == LA_WORD) || ((flag == LA_NO) && (isLetter(c))) )
    { if( !isLetter (c) && !isDigit (c) )
      { buf[i] = flag = 0; *type = LA_WORD; ungetc (c,Names_table->pfScript); }
      else
      { if( flag != LA_WORD ) flag = LA_WORD; buf[i++] = c; }
    }
    else if( isNLine (c) )
    { buf[i]=0; *type = LA_LINE; }

    if( !flag && *type ) goto END;
  }

  if( feof (Names_table->pfScript) )
  { if(i && flag) { buf[i]=0; *type = flag; flag = 0; result = 1; goto END; }
    SetCorrectError (Names_table->pUsrConf->script,ERR_FINC);
  }
  // else result = 1;
  //----------------------------
END:;
  return result;
}
int     laParcerComment   (names_table *Names_table, int sytype)
{ int result = 0; char c; unsigned int i = 0;
  int fl_low = Names_table->pUsrConf->Syntax.no_mean_reg_of_letter;

  char    *cbuf = Names_table->Lex.comp_buf;
  list_nm *nwrd = Names_table->Lex.nwords_storage;
  int flag=0; char *end_word = NULL;
  //----------------------------
  if( sytype )
   end_word = (*list_pmv_exact (Names_table->Lex.ccomm_names,sytype-1))->name;

  if( nwrd->len ) compose_nword (cbuf,nwrd); else *cbuf = 0;
  while( (*cbuf && (c=*(cbuf++))) || (fscanf (Names_table->pfScript,"%c",&c) == 1) )
  { if( !end_word && isNLine (c) )  goto END;
    else if( end_word )
    { if( fl_low ) c = tolower (c);
     
      if( !isLetter(c) && !isSign(c) )
       if( flag ) c=SYMB_SPACE; else continue;
      flag = ( isLetter(c) || isSign(c) );

      ( c==end_word[i] )?(i++):(i=0);
      if( i && !end_word[i] ) goto END;
    }
  }
  if( !feof (Names_table->pfScript) )
   SetCorrectError (Names_table->pUsrConf->script,ERR_FINC);
  else result = 1;
  //----------------------------
END:;
  return result;
}
//-------------------------------------------------------------------------------
myvar*  laReWoConstruct   (lexical_data *Lex, int flag)
{ char    *wbuf    = Lex->word_buf;
  char    *cbuf    = Lex->comp_buf;
  list_nm *nwrd    = Lex->nwords_storage;
  list_nm *grbg    = Lex->nwords_garbage;
  size_t   cur_len = Lex->cur_rw_len;
  size_t   max_spc = Lex->max_rw_spc;
  size_t   max_len = Lex->max_rw_len;
  myvar*   pvar    = NULL; // name Name;
  //-------------------------------------
  if( flag == REWO_STOP )
  { cur_len = 0; }
  else if( strlen (wbuf) )
  { cur_len += strlen (wbuf);
    if( cur_len <= (max_len - max_spc) )                   // если это слово не длиннее самого длинного
     list_nm_insert (nwrd,-1,(name*)wbuf);                 //   добавляем еще одно словосостовляющее
  }
  if( ((flag != REWO_MORE) && nwrd->len) ||
      (nwrd->len == (max_spc+1)) )                         // чтобы получилось слово из максимального числа
  {        compose_nword (cbuf,nwrd);                      //   словосотавляющих, которое имеется в базе
    pvar = lookfor_nword (cbuf);                           //   теперь ищем есть ли регулярное слово или идентификатор в группе 
                                                           //   составленное, составленное без последнего, ..., первая составляющая
    if( pvar ) extract_nword (pvar->name,nwrd);            //   если да -> то вытаскиваем входящие словосоставляющие из конструктора
    else                                                   //   иначе
    {
      name Name;
      list_nm_extract (nwrd, 0U, &Name);                   //     достаем первое словосоставляющее из конструктора
      list_nm_insert  (grbg, -1, &Name);                   //     вставляем его в список составляющих будущего идентификатора
    } // end else
  }   // end if
  else *cbuf = 0;

  if(flag != REWO_STOP)
  { if( cur_len > (max_len - max_spc) )                    // если тот раз мы не добавили из-за длины новое слово
     list_nm_insert (nwrd,-1,(name*)wbuf);                 //   то добавляем сейчас
    Lex->cur_rw_len = cur_len - strlen (cbuf);             // корректируем потенциальную длину
  }
  else Lex->cur_rw_len = 0;
  if(flag == REWO_ENDF) *wbuf = 0;
  //-------------------------------------
  return pvar;
}
myvar*  laSignConstruct   (lexical_data *Lex)
{ char *wbuf = Lex->word_buf;
  char *cbuf = Lex->comp_buf;
  unsigned int len=strlen(wbuf); strcpy(cbuf,wbuf);
  while( !isRegWord(cbuf) && *cbuf ) cbuf[--len]=0;
  if(len) memmove (wbuf,wbuf+len,strlen(wbuf)+1-len);
  return isRegWord (cbuf);
}
//-------------------------------------------------------------------------------


