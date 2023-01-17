#ifndef _EXCHANGE_STRUCTURE_H_
#define _EXCHANGE_STRUCTURE_H_
#include <stdbool.h>
#define MAX_SIZE 100

typedef enum { REQUEST1, REQUEST2, REQUEST3, REQUEST4 } Request_Type;

typedef struct {
    Request_Type requestChoice;
    int userInputLength;
} Client_Request;


typedef struct {
    int ref;
    char authorName[MAX_SIZE];
    char title[MAX_SIZE];
    char genre[MAX_SIZE];
    int nbPages;
    char appreciation;
} Server_Answer;

/*  Description : Permet de tuer le processus fils et de ne pas le laisser zombie
    Param : Signal
void handlerSIGCHL(int receivedSignal); */

#endif
