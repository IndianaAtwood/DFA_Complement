#ifndef __CHARLIST_H__

#define __CHARLIST_H__

typedef struct charList CharList;
struct charList {
  char *val;
  CharList* next;
};

CharList* makeCharListNode(char* val, CharList* next);
void printCharList(CharList* list);
void deleteValue(CharList **listPtr, char *key);
CharList* charListReverse(CharList *listPtr);
void charListCompare(CharList *, CharList *);

#endif
