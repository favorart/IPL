#ifndef _LEX_CHECK_H
#define _LEX_CHECK_H

//-------------------------------------------------------------------------------
#define  RW_ID_FILENAME   "reg.word.id.txt"

#define      SYMB_LCOMM  '#'
#define      SYMB_NLINE  '\n'
#define      SYMB_SLASH  '\\'
#define      SYMB_EQUAL  '='
#define      SYMB_MINUS  '-'
#define      SYMB_SPACE  ' '
#define      SYMB_QUOTE  '"'

#define  STR_SYMB_ENDOT  "."
#define  STR_SYMB_SEPAR  ","
#define  STR_SYMB_SPACE  " "
//-----------------------------------------------------------------------------------------
typedef enum { ALPH_LETTERS, ALPH_SIGNS, ALPH_DIGITS, ALPH_QUOTE, ALPH_DEC_POINT } alph_enum;
//-----------------------------------------------------------------------------------------
#define MIN_RW_LEN 3

int isNLine    (char sgn);
int isSlash    (char sgn);
//-------------------------------------------------------------------------------
int isLetter   (char sgn);
int isSign     (char sgn);
int isDigit    (char sgn);
int isQuote    (char sgn);
int isDecPoint (char sgn);
//-------------------------------------------------------------------------------
rwenum  isComment    (char* buf);
int     isInteger    (char* buf);
int     isFloatPoint (char* buf);

myvar*  isRegWord    (char* buf);
myvar*  isOperator   (char* buf);
myvar*  isIdentifier (char* buf);
//-------------------------------------------------------------------------------
char*   EscapeString (char* buf);
void   ToLowerString (char* buf);
//-------------------------------------------------------------------------------
#endif
