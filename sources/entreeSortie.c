#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/exchangeStructure.h"
#include "../headers/entreeSortie.h"

void clearStdin (void) {    
   int c;
   while ((c = getchar ()) != '\n' && c != EOF);
}

int readInteger(int min, int max, const char *msg) {
    char input[50];
    int integer;

    while (1)
    {
        printf("%s", msg);
        fgets(input, 50, stdin);


        if (sscanf(input, "%d", &integer) == 1 && (integer >= min && integer <= max)) {
            break;
        }
        printf("Erreur, la saisie est incorrecte\n"
               "Votre saisie doit être comprise entre %d et %d\n", min, max); 
    }

    return integer;
}

void displayRequest(Request_Type requestType, Server_Answer serverAnswer){
    fflush(stdout);
    
    switch(requestType){
        case REQUEST1: 
            printf("Auteur: %s \n", serverAnswer.authorName);
            printf("Titre: %s \n", serverAnswer.title);
            printf("Genre: %s \n", serverAnswer.genre);
            
            if(serverAnswer.nbPages > 300) {
                printf("Plus de 300 pages\n");
            }
            else if(serverAnswer.nbPages < 300) {
                printf("Moins de 300 pages\n");
            }
            else {
                 printf("Il y a 300 pages\n");
            }   
            break;
        case REQUEST2:
            printf("Ref: %u\n", serverAnswer.ref);
            printf("Auteur: %s\n", serverAnswer.authorName);
            printf("Titre: %s\n", serverAnswer.title);
            printf("Genre: %s\n", serverAnswer.genre);
            printf("Nombre de pages : %u\n", serverAnswer.nbPages);
            break;
        case REQUEST3: 
            printf("Reference: %u \n", serverAnswer.ref);
            printf("Titre: %s \n", serverAnswer.title);
            break;
        case REQUEST4: 
            printf("Ref: %u\n", serverAnswer.ref);
            printf("Auteur: %s\n", serverAnswer.authorName);
            printf("Titre: %s\n", serverAnswer.title);
            printf("Genre: %s\n", serverAnswer.genre);
            printf("Nombre de pages : %u\n", serverAnswer.nbPages);
            printf("Appréciation: %c\n", serverAnswer.appreciation);
            break;
        default:
            puts("Erreur dans la fonction displayRequest()");
            break;
    }
}

char *getDisplayCriteria(void) {
    char *displayCriteria = (char *) malloc(sizeof(char) *  DISPLAY_CRITERIA_LENGTH);

    strcpy(displayCriteria, "Choisir le critere :\n");
    strcat(displayCriteria, "1 recherche par nombre de pages\n");
    strcat(displayCriteria, "2 recherche par appréciation\n");
    strcat(displayCriteria, "Saisir le critere >>");

    return displayCriteria;
}

char **getKeyWords(size_t *nbKeyWord) {
    
    char **keyWords = (char **) malloc(sizeof(char*) *  NB_KEY_WORDS_MAX);
    size_t i = 0;
    int choice = CONTINUE_INPUT;

    puts("Saisir les mots clés : (4 maximum)");

    while (i < 4 && choice != STOP_INPUT) {
        
        printf("Saisir un mot clé >>");

        keyWords[i] = (char*) malloc(sizeof(char) * MAX_KEY_WORDS_LENGTH);

        scanf("%25s", keyWords[i]);

        clearStdin(); 


        keyWords[i][strlen(keyWords[i]) - 1] = '\0';

        if (i != 3) {
         
            puts("Voulez-vous continuer ?");
            choice = readInteger(0, 1, "Tapez 0 pour arreter ou 1 pour entrer un autre mot clé\nSaisir votre choix >>");
    
        }
        i++;

    }
    
    *nbKeyWord = i;

    return keyWords;
} 
