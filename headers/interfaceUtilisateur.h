#ifndef _INTERFACE_UTILISATEUR_H
#define _INTERFACE_UTILISATEUR_H

/* Etat de l'automate gérant l'interface IHM */
typedef enum {E_MENU_BIENVENUE, E_MENU_SELECTION_PRINCIPALE, E_MAIN_REQU1,
                E_MAIN_REQU2, E_MAIN_REQU3, E_MAIN_REQU4, E_MAIN_AIDE, E_FIN} Etat_Automate_Interface;

/*  Mini automate de gestion de l'interface IHM */
void exec_interface_application(int numeroPort, char *hostname);

#endif