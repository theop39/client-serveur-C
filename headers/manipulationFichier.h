#ifndef _PARSING_FICHIER_H_
#define _PARSING_FICHIER_H_

#define BUFFER_SIZE 255
#define ERROR_OPEN 3

void parseFichier(char *buf, char **pRefInFile , char **pNameInFile, char **pTitleInFile, 
	char **pGenreInFile, char **pPageInFile, char **pAppreInFile);


#endif