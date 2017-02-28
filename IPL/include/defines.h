#ifndef _DEFINES_H
#define _DEFINES_H
//--------------------------------------
#define _STR(x)			 #x
#define  STR(x)	 	 	_STR(x)
#define _CAT(x,y)    x##y
#define  CAT(x,y)  _CAT(x,y)

#define OUTLINE(x)  CAT(STR(%),STR(x ))

#define   STRLENZ	  1024
#define   STRLEN		1025
#define   BUFLEN	 	257
#define  NAMELEN	  BUFLEN
//--------------------------------------
#define  NONE_NOT_NAME  "None"

#define  TRUE_INT_NAME  "1"
#define  TRUE_STR_NAME  "1"
#define  TRUE_FLT_NAME  "1.000000"

#define  ZERO_INT_NAME  "0"
#define  ZERO_FLT_NAME  "0.000000"
#define  ZERO_STR_NAME  ""

#define  ZERO_CNT_NAME  " 0"
#define  ZERO_JNT_NAME  " 0"
//--------------------------------------
#endif
