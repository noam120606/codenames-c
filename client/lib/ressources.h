/**
 * @file ressources.h
 * @brief Gestionnaire de ressources et libération mémoire automatique.
 */

#ifndef RESSOURCES_H
#define RESSOURCES_H

#include "sdl.h"

#define MAX_RESOURCES 16

/**
 * Typedef pour la fonction de destruction de ressource.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
typedef int(*DestroyFunction)(void);

/**
 * Typedef pour la fonction de destruction du contexte SDL.
 * @param context Le contexte SDL à détruire.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
typedef int(*DestroyContext)(AppContext*);

/**
 * Structure pour gérer les ressources.
 * @param context Le contexte SDL associé aux ressources.
 * @param destroy_context La fonction de destruction du contexte SDL.
 * @param destroy_functions Les fonctions de destruction des ressources.
 * @param nb_resources Le nombre de ressources gérées.
 */
typedef struct {
    AppContext* context;
    DestroyContext destroy_context;
    DestroyFunction destroy_functions[MAX_RESOURCES];
    int nb_resources;
} Resources;

/**
 * Initialise les ressources.
 * @return Un pointeur vers la structure de ressources initialisée.
 */
Resources* init_resources();

/**
 * Définit le contexte SDL dans les ressources.
 * @param res La structure de ressources à mettre à jour.
 * @param context Le contexte SDL à définir.
 * @param destroy_context La fonction de destruction du contexte SDL.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int define_app_context(Resources* res, AppContext* context, DestroyContext destroy_context);

/**
 * Ajoute une fonction de destruction à la liste des ressources.
 * @param res La structure de ressources à mettre à jour.
 * @param destroy La fonction de destruction à ajouter.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int add_destroy_resource(Resources* res, DestroyFunction destroy);

/**
 * Libère les ressources allouées.
 * @param res Le pointeur vers la structure de ressources à libérer.
 */
void cleanup_resources(Resources* res);

#endif // RESSOURCES_H