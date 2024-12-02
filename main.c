#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "h_files/cJSON.h"
#include "h_files/main.h"
#include "h_files/charList.h"

int main(int argc, char* argv[]) {
    // Prompt user for .json file
    printf("%s", "Enter .json file name: ");
    char file[20];
    scanf("%s", file);
    printf("\n");

    // Open DFA.json, the original 5-tuple
    FILE *fp = fopen(file, "r");
    if(fp == NULL) {
	printf("Unable to open file...\n");
    	return 1;
    }

    // Pulled from internet, reads DFA into cJSON object
    char buffer[1024];
    int len = fread(buffer, 1, sizeof(buffer), fp);
    fclose(fp);

    cJSON *DFA = cJSON_Parse(buffer);
    if(DFA == NULL) {
	const char *error_ptr = cJSON_GetErrorPtr();
	if(error_ptr != NULL) {
		printf("Ptr Error: %s\n", error_ptr);
	}
	cJSON_Delete(DFA);
	return 1;
    }


    ////////////////////////////////////
    //				      //
    //    MY OWN CODE FROM HERE ON    //
    //				      //
    ////////////////////////////////////


    // 0 is all states, 1 is alpha, 2 is transitions, 3 is start, 4 is accept
    cJSON **tuple = malloc(5 * sizeof(cJSON*));
    char *strings[] = {"all_states", "alphabet", "state_transitions", "start_state", "accept_states"};

    // populates the 5-tuple from the input file
    readDFA(DFA, tuple, strings);
   	
    // write the JSON string to a file 
    FILE *newDFA = fopen("newDFA.json", "w"); 
    if (newDFA == NULL) { 
    	printf("Error: Unable to open the file.\n");
    }

    // checks if DFA is in correct format, returns the error message
    //	   if DFA is in correct format, it returns list of all_states for use in complement()
    CharList *allStates = verify(tuple, strings);

    // Finds the complement of DFA, saves new Accept States into original DFA 5-tuple
    complement(allStates, &tuple[4]);

    // Prints the new DFA using the 5-tuple and the names of each tuple
    printDFA(newDFA, tuple, strings);

    fclose(newDFA);
    cJSON_Delete(DFA);

    return 0;
}

/*
 *  Reads the 5-tuple into an array
 */
void readDFA(cJSON *DFA, cJSON **tuple, char *titles[]) {
    // reads the five-tuple from the original DFA
    for(int i = 0; i < 5; i++) {
	tuple[i] = cJSON_GetObjectItem(DFA, titles[i]);
    }
}

/*
 *  Checks the format of the DFA, using the 5-tuple and the titles of the tuples.
 */
CharList* verify(cJSON **tuple, char *titles[]) {

    // creates and compiles the regex expressions for states and the alphabet
    const char *statePattern = "^q[0-9]$";
    const char *alphaPattern = "a|b";
    regex_t stateReg;
    regex_t alphaReg;

    regcomp(&stateReg, statePattern, REG_EXTENDED);
    regcomp(&alphaReg, alphaPattern, REG_EXTENDED);

    // creates LL of all the states, used later for complement()
    CharList *list = NULL;
    CharList *transStates = NULL;
    int size = cJSON_GetArraySize(tuple[0]);

    for(int i = 0; i < size; i++) {
 	cJSON *value = cJSON_GetArrayItem(tuple[0], i);
	list = makeCharListNode(value->valuestring, list);
    }

    // iterates through all tuples, handling each one separately
    for(int i = 0; i < 5; i++) {
	// all tuples but 3 are arrays, so they are handled together
	if(i != 3) {
   	    if(!cJSON_IsArray(tuple[i])) {
		printf("%s should be an array.\n", titles[i]);
		exit(1);
	    }
	    
	    // For all_states & accept_states (0,4): checks if each state is in correct regex
	    // For alphabet (1): checks if the alphabet is "a, b"
	    // For transitions (2): checks if states are in correct regex, adds states to a CharList
	    // For start_state (3): checks if it is a string
	    cJSON *element = NULL;
	    cJSON_ArrayForEach(element, tuple[i]) {
	    	if(i == 0 || i == 4) {
	            regexCheck(&stateReg, element->valuestring);
	        } else if(i == 1) {
		    regexCheck(&alphaReg, element->valuestring);
		} else if(i == 2) {
		    // transitions are iterated through, isolates each states
		    cJSON *state = cJSON_GetObjectItem(element, "state");
		    cJSON *a = cJSON_GetObjectItem(element, "a");
		    cJSON *b = cJSON_GetObjectItem(element, "b");

		    // if "states" are in correct format, they are added to a CharList
		    regexCheck(&stateReg, state->valuestring);
		    transStates = makeCharListNode(state->valuestring, transStates);

		    // check if states in "a" and "b" have correct format
		    regexCheck(&stateReg, a->valuestring);
		    regexCheck(&stateReg, b->valuestring);
		}
	    }
	} else {
	    if(!cJSON_IsString(tuple[i])) {
	    	printf("%s should be a string.\n", titles[i]);
	    }

	    regexCheck(&stateReg, tuple[i]->valuestring);
	}
    }
	
    // reverses the all_states and transitions CharLists (both created backwards)
    CharList *reversedTransStates = charListReverse(transStates);
    CharList *reversedAllStates = charListReverse(list);

    // compares the two, checking if each state is reflected in the transition table (should be equal)
    charListCompare(reversedAllStates, reversedTransStates);
   
    // returns the all_states array for the complement() function to use
    return reversedAllStates;
}

/*
 *  Checks if the states in the original DFA are in the correct regex form (used by verify() )
 */
void regexCheck(regex_t *regex, char *str) {
    if(regexec(regex, str, 0, NULL, 0)) {
	printf("IMPROPER FORMAT: %s\n", str);
	printf("(states should be in form 'q#', alphabet using 'a, b')");
	exit(1);
    }
}

/*
 *  Deletes the original accept_states from the all_states CharList, leaving Reject States
 */
void complement(CharList *allStates, cJSON **acceptStates) {
    // creates new array for the new Accept States tuple
    cJSON *array = cJSON_CreateArray();

    // iterates through all accept states, removing it from the list
    //    of all states (left with only non-accept states)
    int num = cJSON_GetArraySize(*acceptStates);
    for(int i = 0; i < num; i++) {
	cJSON *val = cJSON_GetArrayItem(*acceptStates, i);
	deleteValue(&allStates, val->valuestring);
    }

    // adds all original non-accept states to new Accept States tuple
    for(CharList *p = allStates; p != NULL; p = p->next) {
	cJSON *str = cJSON_CreateString(p->val);
	cJSON_AddItemToArray(array, str);
    }

    // replaces the original tuple with the new one
    *acceptStates = array;
}

/*
 *  Writes the new DFA to the newDFA.json file
 */
void printDFA(FILE *newFile, cJSON **tuple, char *titles[]) { 
    cJSON *DFA = cJSON_CreateObject();

    for(int i = 0; i < 5; i++) {
        // copy the input data to an output file
	//    (arrays and strings use different add() function
        if(cJSON_IsArray(tuple[i])) {
	    cJSON_AddItemToObject(DFA, titles[i], tuple[i]);
        } else {
	    // if not an array (aka the Start State tuple), just add the string
            cJSON_AddStringToObject(DFA, titles[i], tuple[i]->valuestring);
        }
    }
    
    // print the new DFA to the console, also to file newDFA.json
    char *output = cJSON_Print(DFA);
    fprintf(newFile, output);
    printf("SUCCESS! Type 'cat newDFA.json' to print the complement of DFA.");

    // free the JSON string and cJSON object 
    cJSON_free(output);
}
