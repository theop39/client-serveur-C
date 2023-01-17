#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <stdbool.h> 
#include <limits.h>
#include "../headers/exchangeStructure.h"
#include "../headers/manipulationFichier.h"

#define PORT_SERVER 14255

#define UNUSED(x) (void)(x) /* Ca sert à éviter les warning si on utilise pas le paramètre */

void handlerSIGCHLD(int receivedSignal) {
    UNUSED(receivedSignal);
    fflush(stdout);
    int status;
    wait(&status);

    int firstByte = (status & 0xFF);

    if (firstByte == 0) { //terminaison du fils par un exit 

        int secondByte = ((status >> 8) & 0xFF);//code de retour du fils

        if (secondByte == ERROR_OPEN) {//erreur ouverture d'un fichier
            exit(EXIT_FAILURE);
        }
    }
}

void handlerSIGINT(int receivedSignal) {//on redefinie le comportement du signal SIGINT
    UNUSED(receivedSignal);
    exit(EXIT_SUCCESS);
}

void chomp(char *s) { /* vire le caractère '/n' de la ligne récupérer dans le fichier */
    while (*s != '\n' && *s != '\0')
        ++s;
    *s = '\0';
}

int main(int argc, char* argv[]) {
    UNUSED(argv);
  
    if (argc != 3) { //on doit avoir 3 arguments
        fprintf(stderr , "Erreur, nombre d'arguments invalide\n");
        fprintf(stderr , "Usage: testServeur numero_port fichier_fond_documentaire\n");
        exit(EXIT_FAILURE);
    } 
    int port = atoi(argv[1]);
    struct sigaction handlerChildEnd;
    handlerChildEnd.sa_handler = handlerSIGCHLD;
    handlerChildEnd.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &handlerChildEnd, NULL);

    struct sigaction handlerCtrlC;
    handlerCtrlC.sa_handler = handlerSIGINT;
    handlerCtrlC.sa_flags = SA_RESTART;
    sigaction(SIGINT, &handlerCtrlC, NULL); //redefinie le comportement de SIGINT (pour les processus fils et pere)
    
    
    int listeningSocket, serviceSocket, valRes; 
    struct sockaddr_in adServer;
    struct sockaddr_in adClient;
    socklen_t adClientSize;


    listeningSocket = socket(AF_INET, SOCK_STREAM, 0); /* Création du socket d'écoute */
    if ( listeningSocket == -1 ) { 
        perror("socket");
        exit(EXIT_FAILURE); 
    }
    puts("Création du socket d'écoute réussie");
    
    /* remplissage de la structure de l'adresse du serveur */
    adServer.sin_family = AF_INET;
    adServer.sin_addr.s_addr = htonl(INADDR_ANY);
    adServer.sin_port = htons(port);
    memset(adServer.sin_zero, 0, sizeof(adServer.sin_zero)); /* Remplit le tableau de 0 */

    valRes = bind(listeningSocket, (struct sockaddr*) &adServer, sizeof(adServer)); /* attachement de la socket au serveur */
    if ( valRes == -1 ) { 
        perror("bind");
        exit(EXIT_FAILURE); 
    }
    puts("Bind réussi");

    valRes = listen(listeningSocket, 5); /* initialisation de l'écoute du serveur sur le socket */
    if ( valRes == -1 ) { 
        perror("listen");
        exit(EXIT_FAILURE); 
    }
    puts("Le serveur écoute");

    while(1) {
        serviceSocket = accept(listeningSocket, (struct sockaddr*) &adClient, &adClientSize);
        puts("Création d'un socket service");
        if ( serviceSocket == -1 ) { 
            perror("accept()");
            close(listeningSocket);
            exit(EXIT_FAILURE);
        } else { 
            /* Création d'un processus filsaprès acceptation d'une demande de connexion client */
            switch (fork()) {
                case -1:
                    close(listeningSocket);
                    close(serviceSocket);
                    perror("fork");
                    exit(EXIT_FAILURE);
                case 0: ;    /* processus fils */
                    close(listeningSocket);
                    Client_Request client_request;
                    Server_Answer server_answer;
                    int refAChercher, length;
                    char *requestTab;

                    read(serviceSocket, &client_request, sizeof(client_request));

                    /* Pour la requête 1 on envoie juste un int, donc on a pas besoin de malloc un tableau */
                    if (client_request.requestChoice != REQUEST1) {
                        length = client_request.userInputLength * sizeof(char);
                        requestTab = (char *) malloc(length); 
                        read(serviceSocket, requestTab, length); // tableau requestTab contient maintenant la référence à chercher dans le fichier
                    } else {    
                        read(serviceSocket, &refAChercher, sizeof(refAChercher));
                    }
                    
                    /* les pointeurs que prendra la fonction de parsing du fichier représentant la banque de données */
                    char *pRefInFile = NULL;
                    char *pNameInFile = NULL; 
                    char *pTitleInFile = NULL; 
                    char *pGenreInFile = NULL; 
                    char *pPageInFile = NULL; 
                    char *pAppreInFile = NULL; 

                    char buf[BUFFER_SIZE];
                    bool found = false;

                    char *pNameInRequestTab, *pGenreInRequestTab, *pCriteriaInRequestTab;
                    int incrRequestTab = 0;

                    int nbResult = 0;
                    int nbDiesefound = 0;

                    char *tabMotCle[4]; /* On accepte au max 4 mots clés */
                    int nbMotCle = 0;

                    char path[75];

                    strcpy(path, "../doc/");
                    strcat(path, argv[2]);

                    FILE *f = fopen(path, "r"); // tentative d'ouverture du fichier en mode lecture
                    
                    if (f == NULL) {
                        fprintf(stderr, "Echec lors de l'ouverture fichier : %s\n", argv[2]);
                        close(serviceSocket);
                        exit(ERROR_OPEN);
                    }

                    /********************************** Partie traitement des requêtes **********************************/
                    switch(client_request.requestChoice) {
                        case REQUEST1: 
                            /* Pour la requête 1, l'envoie se passe en 2 temps:
                                    - On envoie tout d'abord un int: 1 si la référence existe, 0 sinon
                                    - Si la référence existe on envoie les infos dans la structure server_answer, sinon on ne fait rien
                            */
                            while (fgets(buf, BUFFER_SIZE, f) != NULL && !found) {
                                /* On parse une fois la ligne en remplacant le # par des \0 et en placent des 'pointeurs' sur chaque début de données */
                                parseFichier(buf, &pRefInFile , &pNameInFile, &pTitleInFile, &pGenreInFile, &pPageInFile, &pAppreInFile);                        
                                
                                if (atoi(pRefInFile) == refAChercher) {
                                    nbResult = 1;
                                    write(serviceSocket, &nbResult, sizeof(nbResult)); 

                                    strcpy(server_answer.authorName, pNameInFile);
                                    strcpy(server_answer.title, pTitleInFile);
                                    strcpy(server_answer.genre, pGenreInFile);
                                    server_answer.nbPages = atoi(pPageInFile);
                                    write(serviceSocket, &server_answer, sizeof(server_answer));
                                    found = true;
                                }
                            }
                            /* dans le cas ou on a pas trouvé la référence, le boolean found est toujours = false, on envoie juste 0 */
                            if (!found) {
                                write(serviceSocket, &nbResult, sizeof(nbResult));
                            } 
                            break;
                        case REQUEST2:
                            /* on commence par parse le tableau de mot clés et placer les pointeurs */
                            tabMotCle[nbMotCle] = &requestTab[incrRequestTab];
                            nbMotCle++;
                            while(incrRequestTab < length - 1) {
                                if (requestTab[incrRequestTab] == '#' && nbDiesefound <= 2) {
                                    requestTab[incrRequestTab] = '\0';
                                    tabMotCle[nbMotCle] = &requestTab[incrRequestTab + 1];
                                    nbMotCle++;
                                    nbDiesefound++;
                                }
                                incrRequestTab++;
                            }

                            char *pFirstOcc = NULL;
                            
                            /* On commence par compter le nombre de résultat */
                            while (fgets(buf, BUFFER_SIZE, f) != NULL) {
                                
                                parseFichier(buf, &pRefInFile , &pNameInFile, &pTitleInFile, &pGenreInFile, &pPageInFile, &pAppreInFile);                        
                                int i = 0;
                                bool isIn = false;
                                while (i < nbMotCle && !isIn) {
                                    /* strstr renvoie un pointeur vers la première occurence de son 2 ème paramètre dans le 1er paramètre, sinon null */
                                    pFirstOcc = strstr(pTitleInFile , tabMotCle[i]);
                                    if (pFirstOcc != NULL) {
                                        nbResult++;
                                        isIn = true;
                                    }
                                    i++;
                                }
                            }

                            fseek(f, 0, SEEK_SET);
                            /* On envoie le nombre de résultat (= nombre de read) que le client doit récupérer */
                            write(serviceSocket, &nbResult, sizeof(nbResult));

                            if (nbResult != 0) {
                                char copyBuf[BUFFER_SIZE];
                                int i, lengthCopy;
                                bool isIn;
                                /* Cette fois on parse le fichier pour renvoyer la/les ligne(s) complète(s) contenant au moins 1 mot clé */
                                while (fgets(buf, BUFFER_SIZE, f) != NULL) {
                                    strcpy(copyBuf, buf);

                                    parseFichier(buf, &pRefInFile , &pNameInFile, &pTitleInFile, &pGenreInFile, &pPageInFile, &pAppreInFile);                        
                                    i = 0;
                                    isIn = false;
                                    while (i < nbMotCle && !isIn) {
                                        pFirstOcc = strstr(pTitleInFile , tabMotCle[i]);
                                        if (pFirstOcc != NULL) {
                                            nbResult++;
                                            isIn = true;
                                        }
                                        i++;
                                    }
                                    
                                    if (isIn) {
                                        chomp(copyBuf);

                                        lengthCopy = strlen(copyBuf) + 1; 
                                        write(serviceSocket, &lengthCopy, sizeof(int)); /* envoie la taille au client */

                                        write(serviceSocket, copyBuf, lengthCopy); /* envoie du tableau */
                                    }
                                }
                            }                                       
                            break;                         
                        case REQUEST3:
                            /* 
                                tableau requestTab qui contient nomAuteur#genre\0
                                On effectue un mini parse pour remplacer les # par de \0 et placer les pointeurs  
                            */
                            pNameInRequestTab = &requestTab[incrRequestTab];
                            while(incrRequestTab < length - 1 && requestTab[incrRequestTab] != '#') {
                                incrRequestTab++;
                                if (requestTab[incrRequestTab] == '#') {
                                    nbDiesefound++;
                                }
                            }
                                                        
                            if (nbDiesefound == 1) {
                                requestTab[incrRequestTab] = '\0';
                                pGenreInRequestTab = &requestTab[incrRequestTab + 1];
                            } else { /* Ca c'est au cas ou la requête reçue est corrompu. On place juste le pointeur au début du tableau */
                                pGenreInRequestTab = &requestTab[0]; // ici on pourrai traiter en mettant nbresult à -1 et tout fermer
                            }
                            
                            /* 
                                On commence pas parcourir une fois entièrement le fichier pour connaître le nombre de résultats dont le 
                                nom de l'auteur et le genre sont égaux à ceux donnés par le client
                            */
                            while (fgets(buf, BUFFER_SIZE, f) != NULL) {
                                
                                parseFichier(buf, &pRefInFile , &pNameInFile, &pTitleInFile, &pGenreInFile, &pPageInFile, &pAppreInFile);                        
                                if (strcmp(pNameInFile, pNameInRequestTab) == 0) {
                                    if (strcmp(pGenreInFile, pGenreInRequestTab) == 0) {
                                        nbResult++;
                                    }
                                }
                            }
        
                            /* On envoie le nombre de résultat (= nombre de read) que le client doit récupérer */
                            write(serviceSocket, &nbResult, sizeof(nbResult));

                            if (nbResult != 0) {
                                fseek(f, 0, SEEK_SET);    
                            /* Cette fois on parse le fichier pour renvoyer la ref et le titre */
                                while (fgets(buf, BUFFER_SIZE, f) != NULL) {
        
                                    parseFichier(buf, &pRefInFile , &pNameInFile, &pTitleInFile, &pGenreInFile, &pPageInFile, &pAppreInFile);                        
                                    
                                    if (strcmp(pNameInFile, pNameInRequestTab) == 0) {
                                        if (strcmp(pGenreInFile, pGenreInRequestTab) == 0) {
                                            server_answer.ref = atoi(pRefInFile);
                                            strcpy(server_answer.title, pTitleInFile);
                                            write(serviceSocket, &server_answer, sizeof(server_answer));
                                        }
                                    }
                                } 
                            }          
                            break;
                        case REQUEST4:
                            /* 
                                tableau requestTab qui contient nomAuteur#typeDeCritère\0
                                On effectue un mini parse pour remplacer les # par de \0 et placer les pointeurs 
                            */
                            pNameInRequestTab = &requestTab[incrRequestTab];
                            while(incrRequestTab < length - 1 && requestTab[incrRequestTab] != '#') {
                                incrRequestTab++;
                                if (requestTab[incrRequestTab] == '#') {
                                    requestTab[incrRequestTab] = '\0';
                                    pCriteriaInRequestTab = &requestTab[incrRequestTab + 1];
                                    nbDiesefound++;
                                } 
                            }
                            /* 
                                Si on a pas rencontré 2 dièses c'est que l'envoie a été corrompu. On place donc les pointeurs au début du
                                du tableau pour éviter tout crash du serveur 
                            */
                            if (nbDiesefound != 1) { // meme chose ici on peut mettre nbresut = -1 et tout fermer
                                pCriteriaInRequestTab = &requestTab[incrRequestTab + 1];                                                               
                            }

                            int minNbPages = INT_MAX;
                            char maxAppr = 'Z'; 

                            while (fgets(buf, BUFFER_SIZE, f) != NULL) {
                                
                                parseFichier(buf, &pRefInFile , &pNameInFile, &pTitleInFile, &pGenreInFile, &pPageInFile, &pAppreInFile);                        
                                
                                if (strcmp(pNameInFile, pNameInRequestTab) == 0) {
                                    if (strcmp(pCriteriaInRequestTab, "parPage") == 0) {
                                        if (atoi(pPageInFile) < minNbPages) {
                                            minNbPages = atoi(pPageInFile);
                                            nbResult++;
                                        }
                                    } else if (strcmp(pCriteriaInRequestTab, "parAppr") == 0) {
                                        if (*pAppreInFile < maxAppr) {
                                            maxAppr = *pAppreInFile;
                                            nbResult++;
                                        }  
                                    }
                                }
                            }
                            
                            if (nbResult != 0) {
                                nbResult = 0;
                                fseek(f, 0, SEEK_SET); /* on replace la tête de lecture au début */                         

                                while (fgets(buf, BUFFER_SIZE, f) != NULL && !found) {
                                    /* On parse une fois la ligne en remplacant le # par des \0 et en placent des 'pointeurs' sur chaque début de données */
                                    parseFichier(buf, &pRefInFile , &pNameInFile, &pTitleInFile, &pGenreInFile, &pPageInFile, &pAppreInFile);                        
                                    
                                    if (strcmp(pNameInFile, pNameInRequestTab) == 0) {
                                        if (strcmp(pCriteriaInRequestTab, "parPage") == 0) {
                                            if (atoi(pPageInFile) == minNbPages) {
                                                nbResult = 1;
                                            }
                                        } else if (strcmp(pCriteriaInRequestTab, "parAppr") == 0) {
                                            if (*pAppreInFile == maxAppr) {
                                                nbResult = 1;
                                            }
                                        }

                                        if (nbResult == 1) {
                                            write(serviceSocket, &nbResult, sizeof(nbResult)); 

                                            server_answer.ref = atoi(pRefInFile);
                                            strcpy(server_answer.authorName, pNameInFile);
                                            strcpy(server_answer.title, pTitleInFile);
                                            strcpy(server_answer.genre, pGenreInFile);
                                            server_answer.nbPages = atoi(pPageInFile);
                                            server_answer.appreciation = *pAppreInFile;

                                            write(serviceSocket, &server_answer, sizeof(server_answer)); 
                                            found = true;
                                        }
                                    }
                                }
                            }
                            fclose(f);
                            break;
                        default:
                            printf("Erreur dans le switch des requêtes\n");
                            break;

                    }

                
                    /*if (fclose(f) == EOF) {
                        printf("Echec fermeture fichier\n");
                        return EXIT_FAILURE;        
                    }*/
                    if (client_request.requestChoice != REQUEST1) {
                        free(requestTab);
                    }
                    close(serviceSocket);
                    exit(EXIT_SUCCESS);
                default:    /* processus père */
                    close(serviceSocket);
                    break;   
            }
        }
    }

    close(listeningSocket);
    return EXIT_SUCCESS;
}