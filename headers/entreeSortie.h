#ifndef _ENTREE_SORTIE_H
#define _ENTREE_SORTIE_H

#include "../headers/exchangeStructure.h"

#define MAX_REF_LENGTH 10
#define MAX_AUTHOR_LENGTH 50
#define MAX_GENRE_LENGTH 25
#define MAX_CRITERIA_LENGTH 8
#define DISPLAY_CRITERIA_LENGTH 110
#define NB_KEY_WORDS_MAX 4
#define MAX_KEY_WORDS_LENGTH 25
#define CONTINUE_INPUT 1
#define STOP_INPUT 0


void clearStdin (void);
int readInteger(int min, int max, const char *msg);
void displayRequest(Request_Type requestType, Server_Answer serverAnswer);
char *getDisplayCriteria(void);
char **getKeyWords(size_t *nbKeyWord);

#endif