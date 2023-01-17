#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <limits.h>
#include "../headers/exchangeStructure.h"
#include "../headers/entreeSortie.h"
#include "../headers/routines.h"
#include "../headers/manipulationFichier.h"
#include "../headers/interfaceUtilisateur.h"

#define UNUSED(x) (void)(x)

/* 
    Gère la création du socket client, récupère les informations du serveur dont le nom est donnée
    par le paramètre 2 sur le port donnée par le paramètre 1. Enfin établit la connection vers le serveur.
    Retour: le descripteur du socket crée et connecté */
int connection_client_to_server(int numeroPort, char *hostname) {

    int socketClient, valRes;
    struct sockaddr_in adServer;
    struct hostent *infoServer;

    socketClient = socket(AF_INET, SOCK_STREAM, 0); /* Création du socket client */
    if (socketClient == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    puts("Création du socket client réussie");

    infoServer = gethostbyname(hostname); /* Récupère les infos du serveur */
    if ( infoServer == NULL ) {
        fprintf(stderr, "Hostname inconnu\n");
        exit(EXIT_FAILURE);
    }

    puts("Récupération des infos serveur réussie");

    adServer.sin_family = AF_INET;
    adServer.sin_port = htons(numeroPort);
    memcpy(&adServer.sin_addr, infoServer->h_addr, infoServer->h_length);
    memset(adServer.sin_zero, 0, sizeof(adServer.sin_zero)); /* Remplit le tableau de 0 */
    
    valRes = connect(socketClient, (struct sockaddr *)&adServer, sizeof(adServer)); /* Connection au serveur */
    
    if (valRes == -1) {
        perror("connect():");
        exit(EXIT_FAILURE);
    }
    puts("Connection au serveur réussie");

    return socketClient;
}

void exec_echange_client_serveur(int socketClient, int requestNum) {

    Server_Answer server_answer; //informations recu par le serveur
    Client_Request client_request; //informations transmisent au serveur
    size_t authorLength;
    size_t genreLength;
    size_t inputLength = 0; //taille de l'input transmis au serveur (paramètres des requetes, ex : reference pour requete 1 ...)
    char *requestTab = NULL;
    int nbResult = 0; //nombre de resultats que le serveur va nous envoyer (resultat de type Server_Answer)
    char author[MAX_AUTHOR_LENGTH];

    switch (requestNum) {
        case 1: ;
            int ref;
            client_request.requestChoice = REQUEST1;
            
            ref = readInteger(1, INT_MAX, "Saisir la référence >>"); /* récupération de la référence depuis l'entrée standard */

            client_request.userInputLength = sizeof(int); /* récupération la taille de la référence */

            write(socketClient, &client_request, sizeof(client_request)); /* envoie au serveur du numero de requête + taille du message */

            write(socketClient, &ref, client_request.userInputLength); /* envoie au serveur de la ref */

            read(socketClient, &nbResult, sizeof(int)); /* récupération du nombre de résultat (0 ou 1) */

            if (nbResult == 1) { /* récupération et affichage du résultat */
                read(socketClient, &server_answer, sizeof(server_answer)); 
                displayRequest(REQUEST1, server_answer);
            } else {
                fprintf(stderr, "Aucun livre n'existe avec la reference %u\n", ref);
            }
            break;
        case 2: ;        
            char **keyWords = NULL;
            client_request.requestChoice = REQUEST2;
            size_t nbKeyWords = 0;

            keyWords = getKeyWords(&nbKeyWords); //saisie des mots clés par l'utilisateur
            inputLength = getInputLengthForKeyWords(keyWords, nbKeyWords); //tailles des mots clés concacténés et séparés par des #

            client_request.userInputLength = inputLength; //on renseigne la taille des paramètres de la requete, pour le que le serveur sache combien d'octets il doit lire

            write(socketClient, &client_request, sizeof(client_request)); /* envoie au serveur du numero de requête + taille du message */

            requestTab = getInputKeyWords(keyWords, nbKeyWords);

            /* 
                envoie au serveur des mots clés à comparer dans les titres des livres (4 maximum)
                format envoyé: choix1#choix2#choix3#choix4\0
            */
            write(socketClient, requestTab, inputLength); 

            read(socketClient, &nbResult, sizeof(int)); /* récupération du nombre de résultat trouvé */

            if (nbResult >= 1) { /* cas ou le serveur à trouvé au moins 1 résultat */
                FILE *fileResults = NULL;
                fileResults = fopen("../doc/fileResults.txt", "w"); //fichier qui contiendra les resultats envoyés par le serveur

                for (int i = 0; i < nbResult; i++) {
                    int sizeOfLine;
                    char *currentLine = NULL;

                    read(socketClient, &sizeOfLine, sizeof(int)); /* récupère la taille du tableau */

                    currentLine = (char *) malloc(sizeof(char) * sizeOfLine);

                    read(socketClient, currentLine, sizeOfLine); /* recopie ce que contient le tableau dans le fichier fileTesults.txt */

                    fprintf(fileResults, "%s\n", currentLine); //on sauvegarde chaque resultat dans le fichier

                    free(currentLine);
                }
                
                fclose(fileResults);

                /* 
                    création d'un processus fils pour executer le script shell qui va trier le fichier fileTesults.txt
                    renvoie un fichier fileSorted.txt avec les lignes triées par ordre alphabétique des noms d'auteurs 
                */
                switch (fork()) { 
                    case -1:
                        perror("fork()");
                        exit(EXIT_FAILURE);
                    case 0: ;
                        const char *cmdName = "./../sources/test.sh";
                        char *cmdArgs[2] = { "test.sh",  (char*)0 };

                        if (execvp(cmdName, cmdArgs) == -1) {
                            perror("execvp");
                            exit(EXIT_FAILURE);
                        }
                        exit(0);
                    default:
                        wait(NULL);
                }
            
                fileResults = fopen("../doc/fileSorted.txt", "r"); // tentative d'ouverture du fichier en mode lecture
                if (fileResults == NULL) {
                    printf("Echec ouverture fichier\n");
                    exit(EXIT_FAILURE);
                }
            
                char buf[BUFFER_SIZE];
                char *pRefInFile = NULL;
                char *pNameInFile = NULL; 
                char *pTitleInFile = NULL; 
                char *pGenreInFile = NULL; 
                char *pPageInFile = NULL; 
                char *pAppreInFile = NULL; 

                /* parse du fichier fileSorted et affichage des résultats */
                while (fgets(buf, BUFFER_SIZE, fileResults) != NULL) {
                                    
                    parseFichier(buf, &pRefInFile , &pNameInFile, &pTitleInFile, &pGenreInFile, &pPageInFile, &pAppreInFile);                        

                    server_answer.ref = atoi(pRefInFile); 
                    strcpy(server_answer.authorName, pNameInFile);
                    strcpy(server_answer.title, pTitleInFile);
                    strcpy(server_answer.genre, pGenreInFile);
                    server_answer.nbPages = atoi(pPageInFile);

                    displayRequest(REQUEST2, server_answer);  /* affichage des résultats */             
                }
                fclose(fileResults);
            } else { /* cas ou nbResult = 0 */
                fprintf(stderr, "Aucun des livres ne contient les mots clés suivants : [ ");
                
                for (size_t i = 0; i < nbKeyWords; i++) {
                    if (i < nbKeyWords - 1) {
                        fprintf(stderr, "%s, ", keyWords[i]);
                    } else {
                        fprintf(stderr, "%s ", keyWords[i]);
                    }
                }
                fprintf(stderr, "]\n");
            }

            for (size_t i = 0; i < nbKeyWords; i++) {
                free(keyWords[i]);
            }

            free(keyWords);
            free(requestTab);
            break;
        case 3:
            client_request.requestChoice = REQUEST3;
            char genre[MAX_GENRE_LENGTH];

            //saisie des paramètres de la requete

            printf("Saisir le nom de l'auteur >>");
            fgets(author, MAX_AUTHOR_LENGTH, stdin);

            clearStdin();

            author[strlen(author) - 1] = '\0';

            printf("Saisir le genre >>");
            fgets(genre, MAX_GENRE_LENGTH, stdin);

            clearStdin();

            genre[strlen(genre) - 1] = '\0';

            /* calcule de la longueur de la chaine à envoyer, format nomAuteur#genre  */
            authorLength = strlen(author) * sizeof(char);
            genreLength = strlen(genre) * sizeof(char);
            inputLength = authorLength + genreLength + 2; /* +2 car on comptabilise le \0 et le # entre les 2 paramètres */

            requestTab = (char *)malloc(sizeof(char) * inputLength);

            strcpy(requestTab, author);
            strcat(requestTab, "#");
            strcat(requestTab, genre);

            client_request.userInputLength = inputLength;

            write(socketClient, &client_request, sizeof(client_request)); /* envoi des infos de l'input au serveur (taille chaine + type requete) */

            write(socketClient, requestTab, inputLength); /* envoie de la chaine de caractère nomAuteur#genre\0 au serveur */

            read(socketClient, &nbResult, sizeof(int)); /* récupération du nombre de résultat trouvé par le serveur */

            if (nbResult == 0) {
                fprintf(stderr, "Nous n'avons trouvé aucun livre pour l'auteur %s pour le genre %s\n", author, genre);
            }
            else {
                for (int i = 0; i < nbResult; i++) { /* boucle qui récupère les résultats et les affiches */
                    read(socketClient, &server_answer, sizeof(server_answer));
                    displayRequest(REQUEST3, server_answer);
                }
            }
            free(requestTab);
            break;
        case 4:
            client_request.requestChoice = REQUEST4;

            int crit;
            char criteria[MAX_CRITERIA_LENGTH];

            printf("Saisir le nom de l'auteur >>");
            fgets(author, MAX_AUTHOR_LENGTH, stdin);

            author[strlen(author) - 1] = '\0';

            char *displayCriteria = getDisplayCriteria();//informations affichées a l'utilisateur 
        
            crit = readInteger(1, 2, displayCriteria);
            free(displayCriteria);

            setCriteria(criteria, crit);
            
            size_t criteriaLength = strlen(criteria) * sizeof(char);
            
            authorLength = strlen(author) * sizeof(char);
            inputLength = authorLength + criteriaLength + 2; /* +2 car \0 et le # */
               
            client_request.userInputLength = inputLength; /* calcule de la longueur de la chaine à envoyer, format nomAuteur#critère\0 */

            write(socketClient, &client_request, sizeof(client_request)); /* envoi des infos de l'input au serveur (taille chaine + type requete) */

            requestTab = (char *) malloc(sizeof(char) * inputLength); 

            //on set le paramètre de la requete
            strcpy(requestTab, author);
            strcat(requestTab, "#");
            strcat(requestTab, criteria);
            
            write(socketClient, requestTab, inputLength); /* envoie de la chaine de caractère nomAuteur#critère\0 au serveur */
                      
            read(socketClient, &nbResult, sizeof(int)); /* récupération du nombre de résultat trouvé par le serveur */

            if (nbResult == 0) {
                fprintf(stderr, "Nous n'avons trouvé aucun livre pour l'auteur %s\n", author);      
            } else {
                read(socketClient, &server_answer, sizeof(server_answer));
        
                displayRequest(REQUEST4, server_answer);
                
            }
            free(requestTab);
            break;
        default:
            printf("Erreur");
            break;
        }

    close(socketClient);
}

int main(int argc, char *argv[]) {
    UNUSED(argv);

    if (argc != 3) { //on doit avoir 3 arguments
        fprintf(stderr, "Erreur, nombre d'arguments invalide\n");
        fprintf(stderr, "Usage: TestClient numero_port hostname\n");
        exit(EXIT_FAILURE);
    }

    exec_interface_application(atoi(argv[1]), argv[2]);

    return EXIT_SUCCESS;
}