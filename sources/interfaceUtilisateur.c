#include <stdio.h>
#include "../headers/interfaceUtilisateur.h"
#include "../headers/client.h"
#include "../headers/entreeSortie.h"

void exec_interface_application(int numeroPort, char *hostname) {

    int socketClient, requestNum; 

    socketClient = connection_client_to_server(numeroPort, hostname); /* initialise la connection au serveur */

    Etat_Automate_Interface etat = E_MENU_BIENVENUE;

    while (etat != E_FIN) {

        switch (etat) {
            case E_MENU_BIENVENUE:
                puts("Bienvenue");
                puts("");
                etat = E_MENU_SELECTION_PRINCIPALE;
                break;
            case E_MENU_SELECTION_PRINCIPALE:
                puts("1) Requete 1");
                puts("2) Requete 2");
                puts("3) Requete 3");
                puts("4) Requete 4");
                puts("5) Aide");
                puts("6) Quitter");
                puts("");
                requestNum = readInteger(1, 6, "Saisir le numéro de la requête >>");

                switch (requestNum) {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        exec_echange_client_serveur(socketClient, requestNum);                    
                        etat = E_FIN;
                        break;
                    case 5:
                        etat = E_MAIN_AIDE;
                        break;
                    case 6:
                        etat = E_FIN;                    
                        break;
                    default:
                        puts("Fatal error. Application closing");
                        etat = E_FIN;
                        break;                        
                }
                break;
            
            case E_MAIN_AIDE:
                puts("");
                puts("Bonjour et bienvenue à la bibliothèque 'Les belles petites pages'");
                puts("  Pour utiliser notre application, vous pouvez selectionner un numero de requete pour vous tenir");
                puts("  informer des livres disponibles dans notre bibliothèque.");
                puts("");
                puts("  En appuyant sur la requête numéro 1 :");
                puts("      -vous pouvez nous envoyer la reference du livre que vous souhaité et nous vous donnerons l'auteur,");
                puts("      le titre et le genre littéraire associé.");
                puts("  En choisissant la requête numero 2 :");
                puts("      -vous devez nous donner un ou plusieurs mots clès (maximum 4) et on vous donnera les informations de ");
                puts("      tous les livres dont leur titre contient au moins un des mots clè.");
                puts("  Avec la requête numero 3 :");
                puts("      -renseigner le nom de l'auteur sous la forme 'Prénom Nom' et le genre littéraire recherché et");
                puts("      vous pourrez trouver les références ainis que les titres des livres associé à la demmande.");
                puts("  Et en se servant de la requête numero 4 :");
                puts("      -vous pouvez nous fournir le nom de l'auteur et un critère et on vous donnera un livre de cet auteur ");
                puts("      Vous devez choisir un critère parmis le nombre minimum de page du livre ou la meilleur appréciation.");
                puts("--------------------------------------------------------------------------------------------------------------");
                puts("  Pour quitter l'application, choisissez l'option numero 6 du menu principal");
                puts("");
                puts("Merci d'avoir utiliser notre application et à bientot chez Les belles petites pages");
                
                readInteger(1, 1, "Saisir 1 pour retourner au menu principal >>");
                etat = E_MENU_SELECTION_PRINCIPALE;                    
                break;
            default:
                puts("Fatal error. Application closing");
                etat = E_FIN;
                break;
        }

    }
    puts("Fermeture de l'application");

}
