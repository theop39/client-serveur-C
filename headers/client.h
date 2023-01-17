#ifndef _CLIENT_H
#define _CLIENT_H

/* 
    Gère la création du socket client, récupère les informations du serveur dont le nom est donnée
    par le paramètre 2 sur le port donnée par le paramètre 1. Enfin établit la connection vers le serveur.
    Retour: le descripteur du socket crée et connecté */
int connection_client_to_server(int numeroPort, char *hostname);

void exec_echange_client_serveur(int socketClient, int requestNum);

#endif