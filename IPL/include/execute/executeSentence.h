#ifndef _EX_SENTENCE_H
#define _EX_SENTENCE_H

//----------------------------
#define ORDER_CORRECT  0
#define ORDER_REVERSE -1

#define EX_ONE_WORD  0
#define EX_SENTENCE  1
//----------------------------
typedef struct _names_table names_table;
struct _names_table;
typedef struct _list_pmv list_pmv;
struct _list_pmv;
typedef struct _myvar myvar;
struct _myvar;
//----------------------------
typedef struct _exSentence exSentence;
struct _exSentence
{ names_table *Names_table;
 
  size_t   *len;
  list_pmv *exec_sentence;
  list_pmv *make_sentence;
};
//-----------------------------------------------------------------------------------------
void  exSentenceConvert (exSentence *exS, list_pmv *ExecSentence, names_table *Names_table);
void  exSentenceCollect (exSentence *exS, list_pmv *CopySentence, names_table *Names_table);

myvar*  exSentenceExact      (exSentence *exS, int order);
myvar*  exSentenceExtract    (exSentence *exS, int order);
myvar*  exSentenceInsertList (exSentence *exS, list_pmv *List);
myvar*  exSentenceSub        (exSentence *exS, list_pmv *Parameters);
myvar*  exSentenceUndo       (exSentence *exS);
//-----------------------------------------------------------------------------------------
#endif