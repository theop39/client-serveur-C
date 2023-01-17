#include <stdio.h>
#include "../headers/manipulationFichier.h"

void parseFichier(char *buf, char **pRefInFile , char **pNameInFile, char **pTitleInFile, 
    char **pGenreInFile, char **pPageInFile, char **pAppreInFile) {
    
    int incr = 0;
    int nbDieseRencontre = 0;
    while (buf[incr] != '\n') {
        if (incr == 0) {
            *pRefInFile = &buf[incr];
        } else if (buf[incr] == '#') {
            buf[incr] = '\0';
            nbDieseRencontre++;
            switch (nbDieseRencontre) {
                case 1:
                    *pNameInFile = &buf[incr + 1];
                    break;
                case 2:
                    *pTitleInFile = &buf[incr + 1];
                    break;
                case 3:
                    *pGenreInFile = &buf[incr + 1];
                    break;
                case 4:
                    *pPageInFile = &buf[incr + 1];
                    break;
                case 5:
                    *pAppreInFile = &buf[incr + 1];
                    break;
                default:   
                    printf("Une erreur lors du premier parse de la ligne\n");         
                    break;                                    
            }
        }
        incr++;
    }
}
