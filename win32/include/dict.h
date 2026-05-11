/************************ :encoding=UTF-8:tabSize=8: *************************\
*                                                                             *
*   File name	    dict.h						      *
*									      *
*   Description	    A general purpose dictionary "class" for C programs       *
*									      *
*   Notes:	    A dictionary is an associative array, where the key is    *
*		    a string, and the value is arbitrary.             	      *
*		    Also known as a map in the C++ standard template library. *
*									      *
*		    Implemented as a self-balancing binary tree.              *
*		    							      *
*   Usage:	    #define DICT_DEFINE_PROCS // Define in one of the modules.*
*		    #include "dict.h"	      // Include in every module.     *
*		    main() {                                                  *
*		      dict_t *dict = NewDict();				      *
*		      NewDictValue(dict, "one", "Number 1 definition");       *
*		      NewDictValue(dict, "two", "Number 2 definition");       *
*		      printf(DictValue(&dict, "one");			      *
*		    }							      *
*		    							      *
*		    DICT_DEFINE_PROCS forces dict.h to generate procedures.   *
*		    This makes dict.h usable without any external dependency. *
*		    An alternative is to link with public routines in SysLib. *
*		    To do that, instead of predefining DICT_DEFINE_PROCS, use:*
*		    #include "SysLib.h"	      // Force linking with SysLib    *
*		    							      *
*   History:								      *
*    2010-07-09 JFL Created this module. 				      *
*    2016-12-31 JFL Renamed AddDictValue as NewDictValue, as documented above.*
*    2017-01-02 JFL Adapted to use SysToolsLib's debugm.h definitions.	      *
*    2017-01-04 JFL Added case-independant NewIDict().			      *
*    2017-01-06 JFL Added an optional data compatison routine for multimaps.  *
*		    Create with NewMMap(datacmp) or NewIMMap(datacmp).	      *
*    2020-04-20 JFL Added support for MacOS.				      *
*    2020-07-29 JFL MsvcLibX now supports a standard snprintf().	      *
*    2021-02-16 JFL Make sure the debug macros also have unique names, to     *
*                   allow having multiple kinds of trees in the same program. *
*    2024-06-22 JFL Fixed the declaration of inline routines.                 *
*    2024-07-21 JFL Improved the fix for compatibility with C99 compilers.    *
*    2025-08-10 JFL Declare all keys as const.				      *
*                   Changed the procedures definition mechanism, to minimize  *
*                   the macro space used.                                     *
*    2025-08-15 JFL Fixed SetDictValue() if the node already exists.	      *
*                   Include SysLib.h if HAS_SYSLIB is defined.                *
*    2025-08-20 JFL Fixed static alternatives for all inline functions        *
*                   changed on 08-15.                                         *
*                   Fixed the case of SysLib.h.                               *
*    2026-02-09 JFL Output debug information only in eXtra debug mode.	      *
*    2026-02-10 JFL Fixed a debug output indentation error.		      *
*                                                                             *
*         © Copyright 2016 Hewlett Packard Enterprise Development LP          *
* Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 *
\*****************************************************************************/

#ifndef _DICT_H
#define _DICT_H

/* Implemented as a self-balancing binary tree */
#include "tree.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
/* These are standard routines, but Microsoft thinks not */
#define strdup _strdup
#endif /* _WIN32 */

#if defined(__unix__) || defined(__MACH__) /* Automatically defined when targeting Unix or Mach apps. */
#include <strings.h>
#define _stricmp strcasecmp
#endif /* __unix__ */

/* Private structure used to implement the dictionary nodes */
typedef struct _dictnode {
  NODE_FIELDS(struct _dict_t, struct _dictnode);
  const char *pszKey;
  void *pData;
} dictnode;
/* And the dictionary object */
typedef struct _dict_t {
  TREE_FIELDS(struct _dict_t, struct _dictnode);
  int (*keycmp)(const char *s1, const char *s2); /* Key comparison routine */
  int (*datacmp)(void *p1, void *p2); /* Data comparison routine for multimaps */
  void (*freedata)(void *p); /* Optional data destructor routine */
} dict_t;

/* Define types and declare functions for handling a tree of such structures */
TREE_DEFINE_TYPES(dict_t, dictnode);

/* Declare private static routines defined in the DICT_DEFINE_PROCS() macro. */
int TREE_CMP(dictnode)(dictnode *pn1, dictnode *pn2);
void *dictnode_tree_callback(dictnode *node, void *ref);

/* Declare public static routines */
extern void *NewDictValue(dict_t *dict, const char *key, void *value); /* maps & multimaps. Preserves existing values */
extern void *SetDictValue(dict_t *dict, const char *key, void *value); /* maps only. Overwrites existing values */
extern void DeleteDictValue(dict_t *dict, const char *key); /* maps only */
extern void *DictValue(dict_t *dict, const char *key); /* maps only */

/* Define private types used by the Foreach routine */
typedef void *DICT_CALLBACK_PROC(const char *key, void *data, void *ref);
typedef struct {
  DICT_CALLBACK_PROC *cb;
  void *ref;
} DICT_CB_STRUCT;

/* Declare inline functions. C99 requires to provide callable alternatives.
   This is done in the DICT_INLINE_FUNCTIONS_ALTERNATIVES macro below */
inline dict_t *NewDict(void (*freedata)(void *p)) {
  dict_t *dict = new_dictnode_tree();
  if (dict) {
    dict->keycmp = strcmp;		/* Case-dependant key comparison */
    dict->freedata = freedata;		/* Optional data destructor routine */
  }
  return dict;
}

inline dict_t *NewIDict(void (*freedata)(void *p)) {
  dict_t *dict = new_dictnode_tree();
  if (dict) {
    dict->keycmp = _stricmp;		/* Case-independant key comparison */
    dict->freedata = freedata;		/* Optional data destructor routine */
  }
  return dict;
}

inline dict_t *NewMMap(int (*datacmp)(void *p1, void *p2), void (*freedata)(void *p)) {
  dict_t *dict = new_dictnode_tree();
  if (dict) {
    dict->keycmp = strcmp;		/* Case-dependant key comparison */
    dict->datacmp = datacmp;		/* Data comparison */
    dict->freedata = freedata;		/* Optional data destructor routine */
  }
  return dict;
}

inline dict_t *NewIMMap(int (*datacmp)(void *p1, void *p2), void (*freedata)(void *p)) {
  dict_t *dict = new_dictnode_tree();
  if (dict) {
    dict->keycmp = _stricmp;		/* Case-independant key comparison */
    dict->datacmp = datacmp;		/* Data comparison */
    dict->freedata = freedata;		/* Optional data destructor routine */
  }
  return dict;
}

inline void *ForeachDictValue(dict_t *dict, DICT_CALLBACK_PROC cb, void *ref) {
  DICT_CB_STRUCT s;
  s.cb = cb;
  s.ref = ref;
  return foreach_dictnode(dict, dictnode_tree_callback, &s);
}

inline dictnode *FirstDictValue(dict_t *dict) {
  return first_dictnode(dict);
}

inline dictnode *NextDictValue(dict_t *dict, dictnode *pn) {
  return next_dictnode(dict, pn);
}

inline dictnode *LastDictValue(dict_t *dict) {
  return last_dictnode(dict);
}

inline dictnode *PrevDictValue(dict_t *dict, dictnode *pn) {
  return prev_dictnode(dict, pn);
}

inline int GetDictSize(dict_t *dict) {
  return num_dictnode(dict);
}

#endif /* !defined(_DICT_H) */

/********************** Optionally define the procedures **********************/

#if defined(DICT_DEFINE_PROCS)

#if !defined(_DICT_PROCS_DEFINED)
#define _DICT_PROCS_DEFINED

#if __STDC_VERSION__ >= 199901L

/* Provide static alternatives for all inline functions, as required by C99 */
dict_t *NewDict(void (*freedata)(void *p));
dict_t *NewIDict(void (*freedata)(void *p));
dict_t *NewMMap(int (*datacmp)(void *p1, void *p2), void (*freedata)(void *p));
dict_t *NewIMMap(int (*datacmp)(void *p1, void *p2), void (*freedata)(void *p));
void *ForeachDictValue(dict_t *dict, DICT_CALLBACK_PROC cb, void *ref);
dictnode *FirstDictValue(dict_t *dict);
dictnode *NextDictValue(dict_t *dict, dictnode *pn);
dictnode *LastDictValue(dict_t *dict);
dictnode *PrevDictValue(dict_t *dict, dictnode *pn);
int GetDictSize(dict_t *dict);

#else

/* Older compilers that implement inline functions don't need these alternatives.
   Furthermore, the MSVC 1.5 compilers for DOS choke on them due to macro size limitations. */

#endif /* __STDC_VERSION__ >= 199901L */

/* Static routines that need to be defined once somewhere. */

TREE_DEFINE_PROCS(dict_t, dictnode);

int TREE_CMP(dictnode)(dictnode *pn1, dictnode *pn2) {
  dict_t *tree = (pn1 && pn1->sbbt_tree) ? pn1->sbbt_tree : (pn2 ? pn2->sbbt_tree : NULL);
  int dif = 0;
  TREE_ENTRY(dictnode,(TREE_V(TREE_CMP(dictnode)) "(%p, %p)\n", pn1, pn2));
  TREE_IF_DEBUG({
    char key1[80] = "NULL";
    char key2[80] = "NULL";
    char *pszKeyCmp = "NULL";
    if (TREE_DEBUG_IS_ON()) {
      if (pn1) snprintf(key1, sizeof(key1), "\"%s\"", pn1->pszKey);
      if (pn2) snprintf(key2, sizeof(key2), "\"%s\"", pn2->pszKey);
      if (tree) {
      	if (tree->keycmp == strcmp) pszKeyCmp = "strcmp";
	else if (tree->keycmp == _stricmp) pszKeyCmp = "stricmp";
	else pszKeyCmp = "???";
      }
      printf("%*s%s(%s, %s)\n", iIndent, "", pszKeyCmp, key1, key2);
    }
  })
  if (tree) {
    dif = tree->keycmp(pn1->pszKey, pn2->pszKey);
    if (!dif && tree->datacmp) dif = tree->datacmp(pn1->pData, pn2->pData);
  }
  TREE_RETURN_INT(dictnode,dif);
}

TREE_IF_DEBUG(
int TREE_SPRINT(dictnode)(char *buf, int len, dictnode *n) {
  if (!n) return snprintf(buf, len, "NULL");
  return snprintf(buf, len, "\"%s\"", n->pszKey);
}
)

void *dictnode_tree_callback(dictnode *node, void *ref) {
  DICT_CB_STRUCT *s = ref;
  return (s->cb)(node->pszKey, node->pData, s->ref);
}

/* For both maps and multimaps. Do not change the tree is a node already exists. */
void *NewDictValue(dict_t *dict, const char *key, void *value) {
  dictnode *node = calloc(1, sizeof(dictnode));
  TREE_ENTRY(dictnode,("NewDictValue(%p, \"%s\", %p)\n", dict, key, value));
  if (node) {
    dictnode *oldNode;
    node->pszKey = strdup(key);
    node->pData = value;   /* Necessary for both maps & multimaps */
    oldNode = get_dictnode(dict, node);
    if (oldNode) {	/* This is a duplicate of an existing node */
      free((void *)(node->pszKey));
      free(node);
      node = oldNode;		/* Refer to the old node */
      /* The old value is preserved. So map values are NOT updated. */
    } else {
      add_dictnode(dict, node); /* Register the new node in the tree */
    }
  }
  TREE_RETURN(dictnode,node);
}

/* For maps only. Change the value if a node with that key already exists. */
void *SetDictValue(dict_t *dict, const char *key, void *value) {
  dictnode *node;
  dictnode refNode = {0};
  TREE_ENTRY(dictnode,("SetDictValue(%p, \"%s\", %p)\n", dict, key, value));
  refNode.pszKey = key;
  node = get_dictnode(dict, &refNode);
  if (node) {
    if (dict->freedata) (dict->freedata)(node->pData); /* Free the old value */
    node->pData = value;
  } else {
    node = NewDictValue(dict, key, value);
  }
  TREE_RETURN(dictnode,node);
}

/* For maps only */
void DeleteDictValue(dict_t *dict, const char *key) {
  dictnode *pNodeFound;
  dictnode refNode = {0};
  refNode.pszKey = key;
  pNodeFound = get_dictnode(dict, &refNode);
  if (pNodeFound) {
    const char *pszKey = pNodeFound->pszKey;
    if (dict->freedata) { /* If defined, call the destructor for the value. */
      (dict->freedata)(pNodeFound->pData);
    }
    remove_dictnode(dict, pNodeFound);
    free((void *)pszKey); /* Must be freed afterwards, because it's used by remove()! */
  }
}

/* For maps only */
void *DictValue(dict_t *dict, const char *key) {
  dictnode *pNodeFound;
  dictnode refNode = {0};
  refNode.pszKey = key;
  pNodeFound = get_dictnode(dict, &refNode);
  if (!pNodeFound) {
    return 0;
  }
  return pNodeFound->pData;
}

#endif /* !defined(_DICT_PROCS_DEFINED) */

#else /* !defined(DICT_DEFINE_PROCS) */

#if HAS_SYSLIB

#include "SysLib.h"	/* Automatically link with SysLib dictionary routines */

#endif /* HAS_SYSLIB */

#endif /* defined(DICT_DEFINE_PROCS) */

