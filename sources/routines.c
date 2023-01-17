#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void setCriteria(char *criteria, int crit) {
        if (crit == 1) {
            strcpy(criteria, "parPage"); 
        }
        else if (crit == 2) {
            strcpy(criteria, "parAppr");
        }
        else {
            fprintf(stderr, "Erreur, fonction setCriteria() : mauvaise valeur du critere.");
            exit(EXIT_FAILURE);
        }
}


size_t getInputLengthForKeyWords(char **keyWords, size_t nbKeyWords) {
    size_t inputLengthForKeyWords = 0;

    for (size_t i = 0; i < nbKeyWords; i++) {
        inputLengthForKeyWords += strlen(keyWords[i]);

        if ((i < 3) && (nbKeyWords > 1) && (i != nbKeyWords - 1)) {//(du premier au troisieme mot) && (si il y a plus de un mot) && (si c'est pas le dernier mot)
            inputLengthForKeyWords++; //on compte les #
        }
    }

    inputLengthForKeyWords++; //on compte le caractere de fin '\0'

    return inputLengthForKeyWords;
}

//retourne les mots clés concacténés dans une chaine avec des #
char *getInputKeyWords(char **keyWords, size_t nbKeyWords) {
    
    if (nbKeyWords < 1) {
        fprintf(stderr, "Erreur, fonction getInputKeyWords() : aucun mot clés.");
        exit(EXIT_FAILURE);
    }

    size_t inputKeyWordsLength = getInputLengthForKeyWords(keyWords, nbKeyWords);
    char *inputKeyWords = NULL;

    inputKeyWords = (char *) malloc(sizeof(char) * inputKeyWordsLength);


    strcpy(inputKeyWords, keyWords[0]);


    for (size_t i = 1; i < nbKeyWords; i++) {
        
        strcat(inputKeyWords, "#");
        strcat(inputKeyWords, keyWords[i]);
    }
    
    return inputKeyWords;

}
