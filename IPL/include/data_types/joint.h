#ifndef _JOINT_H
#define _JOINT_H

typedef struct _joint joint; struct _joint;
typedef struct _myvar myvar; struct _myvar;

typedef unsigned int (*ptr_func_hash)(joint*,myvar*);
//-----------------------------------------------
typedef struct _joint_hash_node joint_node;
struct _joint_hash_node
{ myvar         *key;
  myvar         *val;
#ifdef JOINT_PTR_NODE
  joint_node   *next;
#else
  unsigned char free;
#endif
};

struct _joint
{ ptr_func_hash   hash;
  unsigned int     len;
  unsigned int    size;
#ifdef JOINT_PTR_NODE
  joint_node **content;
#else
  joint_node  *content;
#endif
};
//-----------------------------------------------
void    joint_create  (joint *Joint, unsigned int size);
void    joint_free    (joint *Joint);
void    joint_clear   (joint *Joint);
void    joint_copy    (joint *scr, joint *dest);
//-----------------------------------------------
myvar*  joint_insert  (joint *Joint, myvar *key, myvar *val);
myvar*  joint_extract (joint *Joint, myvar *key);
myvar** joint_exact   (joint *Joint, myvar *key);
myvar*  joint_find    (joint *Joint, myvar *val, int (*compar)(myvar**,myvar**));
//-----------------------------------------------
typedef struct _joint_iter joint_iter;
struct _joint_iter
{ unsigned int  i;
#ifdef JOINT_PTR_NODE
  joint_node *cur;
#endif
};

joint_node* joint_next (joint *Joint, joint_iter* iter);
//-----------------------------------------------
#endif