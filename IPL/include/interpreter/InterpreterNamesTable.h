#ifndef _NAMES_TABLE_H
#define _NAMES_TABLE_H

//------------------------------------------------------------------------------
typedef struct _myvar myvar;
struct _myvar;
typedef struct _list_mv list_mv;
struct _list_mv;
typedef struct _list_nm list_nm;
struct _list_nm;
typedef struct _list_pmv list_pmv;
struct _list_pmv;
typedef struct _hash_table  hash_table;
struct _hash_table;
typedef struct _IPL_interpreter interpreter;
struct _IPL_interpreter;
//------------------------------------------------------------------------------
typedef struct _lexical_data lexical_data;
struct _lexical_data
{ char          **Alphabet; //---Массив всех разрешенных знаков
  size_t          AlphSize; //---Кол-во классов в алфавите 
//------------------------------------------------------------------------------
  char           *word_buf; //---Буфер хранения слова в начале Лекс.Анализа
  size_t          word_len; //---Выделенное количество памяти под буффер слова
  char           *comp_buf; //---Буфер хранения композиции слов при Лекс.Анализе
  size_t          comp_len; //---Выделенное количество памяти под буфер композиции
//------------------------------------------------------------------------------
  size_t        max_rw_spc; //---Максимальное число пробелов в рег.слове
  size_t        max_rw_len; //---Максимальная длина имени регулярного слова
  size_t        cur_rw_len; //---Длина текущей лексической конструкции
//------------------------------------------------------------------------------
  list_nm  *nwords_storage; //---Место для хранения конструктора рег.слов при сборке
  list_nm  *nwords_garbage; //---Место для хранения 
//------------------------------------------------------------------------------
  list_pmv    *ccomm_names; //---Список имен слов, закрывающих многострочный\
                                 комментарий, используется в функции лексич.\
                                 анализатора пропуска комментариев для\
                                 остановки, в соотв с синхронным словом.
};
typedef struct  _syntax_data  syntax_data;
struct  _syntax_data
{ unsigned int   ProgramWord; //---Найдена точка входа в программу
  unsigned int   GlobalComma; //---Если <exp> внутри параметров функции   // flag_in_func_params
  unsigned int   saVarsComma; //---vars C func decl 
  unsigned int   DepthOfStat; //---Глубина включенных блоков
  unsigned int   DepthRecurs; //---Глубина рекурсии                       // flag_in_func
  unsigned int   DepthCycles; //---Количество циклов, которые выполняются
  unsigned int   GlobalEnDot; //---было ли пред.словом закрывающая точка.
  char          *DepthOfItem; //---Пред.строка при нумерованиии инструкций
  list_pmv      *FncDefNames; //---Список всех рег.слов, обозначающих функцию языка
  myvar        **ConstInCont; //---Если была индексация

  unsigned int   cEnumerator; //---Нумерователь констант-контейнеров
  unsigned int   jEnumerator; //---Нумерователь констант-структур
};
//------------------------------------------------------------------------------
typedef struct _names_table names_table;
struct _names_table
{ FILE          *pfScript; //---Файловый поток скрипта
  interpreter   *pUsrConf; //---Указательна структуру интерпретатора
//------------------------------------------------------------------------------
   variables  *pVariables; //---Список всех переменных программы
   constants  *pConstants; //---Список всех констант в программе
  hash_table  *pReg_Words; //---Список всех регулярных слов программы
//------------------------------------------------------------------------------
   variables  *pBsValVars; //---Начальные значения переменных
  list_mv     *pSentences; //---Свалка ненужных аллокированных выражений
  list_pmv    *pUserFuncs; //---Имена пользовательских функций, имеющихся на данный момент
//------------------------------------------------------------------------------
  lexical_data        Lex; //---Буферы лексического анализатора
   syntax_data        Flg; //---Глобальные авто-выставляющиеся флаги
};
//-------------------------------------------------------------------------------
int  names_table_create (names_table *Names_table, interpreter *Interpreter);
void names_table_free   (names_table *Names_table);
//-------------------------------------------------------------------------------
names_table**  _Names_table (void);
//------------------------------------------------------------------------------
#define  REWO_CONF_EXT   ".txt"
#define  ALPH_CONF_EXT   "_character_set.txt"

#endif