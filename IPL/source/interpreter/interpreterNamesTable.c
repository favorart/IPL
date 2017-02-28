#include "Header.h"
#include "Interpreter.h"
#include "mvList.h"
#include "nmList.h"

#include "InterpreterLexical.h"
#include "InterpreterPreCompile.h"

names_table** _Names_table (void);
int  check (names_table *Names_table);
//-------------------------------------------------------------------------------
#include <locale.h>

int   set_script_locale   (char* lang)
{ int result = 0;
 
       if( !strcmp(lang, "eng") ) { }
  else if( !strcmp(lang, "rus") ) { setlocale(LC_ALL,"rus"); setlocale(LC_NUMERIC,"eng"); }
  else if( !strcmp(lang, "uzb") ) { }
  //else if( !strcmp(lang, "xxx")) {}
  else { SetCorrectError(lang,ERR_CONF); result = 1; }

  return result;
}
//-------------------------------------------------------------------------------
int   alphabet_create     (char* filename, char ***arr, unsigned int *alph_size)
{ unsigned int i,j; int result = 0;
  FILE *f = NULL; long file_size;
  char *buf = NULL;  char c;
  char    group_id [BUFLEN];
  char  characters [BUFLEN];
  char *correcting_words[] = {"letters=","signs=","digits=","quote=","dec.point="};
  *alph_size = sizeof( correcting_words ) / sizeof( char* );

  // open file stream
  if( !(f = fopen (filename,"rt")) )
  { SetCorrectError(filename,ERR_FILE); result = 1; goto END; }

  // get file size
  fseek (f,0L,SEEK_END);
  file_size = ftell (f);
  fseek (f,0L,SEEK_SET);

  // alloc memory
  if( !( buf = (char*)malloc( sizeof(char)*(file_size+1) )) ) 
  { SetCorrectError(NULL,ERR_MEM); result = 1; goto END; }
  if( !((*arr) = (char**)calloc( *alph_size,sizeof(char*) )) ) 
  { SetCorrectError(NULL,ERR_MEM); result = 1; goto END; }

  // read text from file
  fread (buf,sizeof(char),file_size,f); buf[file_size] = '\0';

  for(j=0, i=0, c=buf[0]; c!=0; c=buf[++i])
  { // пропускаем комментарии и лишние строки
    if( (c == '\n') || (c == ' ') ) continue;
    if(  c == '#'  ) { while( c != '\n' ) c=buf[++i]; continue; }

    if( sscanf(buf+i,"%s %s",group_id,characters) != 2 )
    { SetCorrectError(filename,ERR_FILE); result = 1; goto END; }
    i += strlen(group_id) + strlen(characters);

    // заполнить массивы доступных знаков
    if( !strcmp (group_id,correcting_words[j]) )
    { if( !((*arr)[j] = (char*)malloc( sizeof(char)*(strlen(characters)+1) )) ) 
      { SetCorrectError(NULL,ERR_MEM); result = 1; goto END; }
      strcpy ((*arr)[j],characters); j++;
      if( j == *alph_size ) break;
    }
    else { SetCorrectError(filename,ERR_CONF); result = 1; goto END; }
  } // end for

#ifdef  _DEBUG_ //---DEBUG---------------------------------

  for(i=0; i<(*alph_size); i++)
  { for(j=0, c=(*arr)[i][j]; c!=0; c=(*arr)[i][++j]) 
     printf("%c",c);
    printf("\n");
  }

#endif //--------------------------------------------------

END:
  // free temp resources
  if(buf) { free (buf); buf = NULL; }
  if(f)   { fclose (f);   f = NULL; }

  return result;
}
//-------------------------------------------------------------------------------
int           lex_create  (lexical_data *Lex)
{ int result = 0;

  if( !(Lex->comp_buf = (char*)malloc( sizeof(char)*STRLEN ) ))
  { SetError (ERR_MEM); result = 1; goto END; }
  Lex->comp_len = STRLEN;
  if( !(Lex->word_buf = (char*)malloc( sizeof(char)*STRLEN ) ))
  { SetError (ERR_MEM); result = 1; goto END; }
  Lex->word_len = STRLEN;

  if( !(Lex->nwords_garbage = (list_nm*)malloc( sizeof(list_nm) )) )
  { SetError (ERR_MEM); result = 1; goto END; }
  if( !(Lex->nwords_storage = (list_nm*)malloc( sizeof(list_nm) )) )
  { SetError (ERR_MEM); result = 1; goto END; }
  list_nm_create (Lex->nwords_garbage);
  list_nm_create (Lex->nwords_storage);

  if( !(Lex->ccomm_names = (list_pmv*)malloc( sizeof(list_pmv) )) )
  { SetError (ERR_MEM); result = 1; goto END; }
  list_pmv_create (Lex->ccomm_names);
END:;
  return result;
}
void          lex_free    (lexical_data *Lex)
{ if( Lex->comp_buf ) { free (Lex->comp_buf); Lex->comp_buf = NULL; }
  if( Lex->word_buf ) { free (Lex->word_buf); Lex->word_buf = NULL; }
  if( Lex->ccomm_names )
  { list_pmv_free (Lex->ccomm_names);
    free (Lex->ccomm_names); Lex->ccomm_names = NULL;
  }
  if( Lex->nwords_garbage )
  { list_nm_free (Lex->nwords_garbage);
    free (Lex->nwords_garbage); Lex->nwords_garbage = NULL;
  }
  if( Lex->nwords_storage )
  { list_nm_free (Lex->nwords_storage);
    free (Lex->nwords_storage); Lex->nwords_storage = NULL;
  }

  if( Lex->Alphabet ) 
  { unsigned int i;
    for(i=0; i<Lex->AlphSize; i++)
      if( Lex->Alphabet[i] ) free (Lex->Alphabet[i]);
    free (Lex->Alphabet); Lex->Alphabet = NULL;
  }
}
int           lex_copy    (lexical_data *scr, lexical_data *dest)
{ int result = 0; unsigned int i,n;

  if( !scr || !dest )
  { SetError (ERR_PAR);  result = 1; goto END; }
  lex_free (dest);
  if(lex_create(dest)) { result = 1; goto END; }
  
  dest->max_rw_len = scr->max_rw_len;
  dest->max_rw_spc = scr->max_rw_spc;
  
  dest->comp_len = scr->comp_len;
  dest->word_len = scr->word_len;
  dest->AlphSize = scr->AlphSize;

  dest->Alphabet = (char**)malloc( sizeof(char*)*dest->AlphSize );
  if( !dest->Alphabet ) { SetError (ERR_MEM); result = 1; goto END; }

  for(i=0; i<dest->AlphSize; i++)
  { n = strlen (scr->Alphabet[i]) + 1;
   
    dest->Alphabet[i] = (char*)malloc( sizeof(char)*n );
    if( !dest->Alphabet[i] )
    { SetError (ERR_MEM); result = 1; goto END; }

    strcpy (dest->Alphabet[i],scr->Alphabet[i]);
  }
  list_pmv_copy (scr->ccomm_names,dest->ccomm_names);

END:;
  return result;
}
//-------------------------------------------------------------------------------
int   names_table_create  (names_table *Names_table, interpreter *Interpreter)
{ int  result = 0;

  memset (Names_table,0,sizeof(names_table)); 
  //memset (&Names_table->Flg,0,sizeof(Names_table->Flg));
  //memset (&Names_table->Lex,0,sizeof(Names_table->Lex));
  //Names_table->pfScript = NULL;
  Names_table->Flg.DepthOfItem = ZERO_STR_NAME;
  
  if( !(Names_table->pVariables = ( variables*)malloc( sizeof( variables) )) )
  { SetError (ERR_MEM); result = 1; goto END; }
  if( !(Names_table->pConstants = ( constants*)malloc( sizeof( constants) )) )
  { SetError (ERR_MEM); result = 1; goto END; }
  if( !(Names_table->pReg_Words = (hash_table*)malloc( sizeof(hash_table) )) )
  { SetError (ERR_MEM); result = 1; goto END; }
  if( !(Names_table->pBsValVars = (hash_table*)malloc( sizeof(hash_table) )) )
  { SetError (ERR_MEM); result = 1; goto END; }
  if( !(Names_table->pSentences = (   list_mv*)malloc( sizeof( list_mv  ) )) )
  { SetError (ERR_MEM); result = 1; goto END; }

   variables_create ();
   constants_create ();
        hash_create (Names_table->pReg_Words);
        hash_create (Names_table->pBsValVars);
     list_mv_create (Names_table->pSentences);

  //--Установка соответствующей языковой поддержки
  if( set_script_locale (Interpreter->config) )
  { result = 1; goto END; }
  //----------------------------------------------

  Names_table->pUsrConf = Interpreter;
  if( 1 )//check (Names_table) )
  { char filename[BUFLEN];
    //---Создание структуры лесического анализатора
    if( lex_create (&Names_table->Lex) )
    { result = 1; goto END; }

    if( !(Names_table->Flg.FncDefNames = (list_pmv*)malloc( sizeof(list_pmv) )) )
    { SetError (ERR_MEM); result = 1; goto END; }
    list_pmv_create (Names_table->Flg.FncDefNames);
    //----------------------------------------------

    //---Заполнение алфавитов разрешенными буквами
    strcpy(filename,Interpreter->config);
    strcat(filename,ALPH_CONF_EXT);
    if ( alphabet_create (filename, &Names_table->Lex.Alphabet, &Names_table->Lex.AlphSize) )
    { result = 1; goto END; }
    //----------------------------------------------

    //---Заполнение списка регулярных слов
    strcpy(filename,Interpreter->config);
    strcat(filename,REWO_CONF_EXT);
    if( regwords_config (filename,Names_table->pReg_Words) )
    { result = 1; goto END; }
    //----------------------------------------------

    //---Открытие основного файлового потока выполняемого скрипта
    if (!( Names_table->pfScript = fopen(Interpreter->script,"rt") )) //---читает SHARED
    { SetCorrectError(Interpreter->script,ERR_FILE); result = 1; goto END; }
    //----------------------------------------------
  }

  if( !(Names_table->pUserFuncs = (list_pmv*)malloc( sizeof(list_pmv) )) )
  { SetError (ERR_MEM); result = 1; goto END; }
  list_pmv_create (Names_table->pUserFuncs);
  //----------------------------------------------
END:
  return result;
}
void  names_table_free    (names_table *Names_table)
{ Names_table->pUsrConf = NULL;
 
   variables_free ();
   constants_free ();

        hash_free (Names_table->pReg_Words);
        hash_free (Names_table->pBsValVars);

     list_mv_free (Names_table->pSentences);
    list_pmv_free (Names_table->pUserFuncs);

  if( Names_table->pVariables) { free (Names_table->pVariables); Names_table->pVariables = NULL; }
  if( Names_table->pConstants) { free (Names_table->pConstants); Names_table->pConstants = NULL; }
  if( Names_table->pReg_Words) { free (Names_table->pReg_Words); Names_table->pReg_Words = NULL; }
  if( Names_table->pBsValVars) { free (Names_table->pBsValVars); Names_table->pBsValVars = NULL; }
  if( Names_table->pSentences) { free (Names_table->pSentences); Names_table->pSentences = NULL; }

  lex_free (&Names_table->Lex);
  if( Names_table->Flg.FncDefNames )
  { list_pmv_free (Names_table->Flg.FncDefNames);
    free (Names_table->Flg.FncDefNames); Names_table->Flg.FncDefNames = NULL;
  }
  if( Names_table->pfScript ) { fclose(Names_table->pfScript); Names_table->pfScript = NULL; }
}
//-------------------------------------------------------------------------------
myvar* saNewSentence (void)
{ myvar var = {0};
  var.vtype = Pre_Comp;
  if( !(var.value.lvalue = (list_pmv*)calloc( 1,sizeof(list_pmv) )) )
   SetError(ERR_MEM);

  return list_mv_insert ( (*_Names_table())->pSentences,-1,&var);
}
//-------------------------------------------------------------------------------
