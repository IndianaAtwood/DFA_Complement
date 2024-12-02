#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "h_files/charList.h"

// creates a new node at the start
CharList* makeCharListNode(char* val, CharList* next) {
  CharList* rtnVal = malloc(sizeof(CharList));
  rtnVal->val = val;
  rtnVal->next = next;
  return rtnVal;
}

// prints all values in order
void printCharList(CharList* list) {
  for (CharList* p = list; p != NULL; p = p->next) {
    printf("%s\n", p->val);
  }
}

// delete first occurence of a given value
void deleteValue(CharList **listPtr, char *key) {
  for (CharList** pp = listPtr; *pp != NULL; pp = &(*pp)->next) {
    if (strcmp((*pp)->val, key) == 0) {
      CharList* temp = *pp;
      *pp = temp->next;
      free(temp);
     
      return;
    } 
  } 
}

// reverses a CharList, as they are often in backwards order upon creation
CharList* charListReverse(CharList *listPtr) {
  CharList *new = NULL;

  for (CharList *p = listPtr; p != NULL; p = p->next) {
    new = makeCharListNode(p->val, new);
  }
  
  return new;
}

// checks if two CharLists are equal, or one is shorter than the other
void charListCompare(CharList *one, CharList *two) {
    while (one != NULL && two != NULL) {
        if (strcmp(one->val, two->val) != 0) {
            printf("MISMATCHED STATES:\nall_states: %s\ntransition states: %s\n", one->val, two->val);
            exit(1);
        }
        // Advance both pointers
        one = one->next;
        two = two->next;
    }

    // Check if the lists are of different lengths
    if (one != NULL || two != NULL) {
        printf("all_states and transitions states have different lengths (mismatched).\n");
        exit(1);
    }
}
