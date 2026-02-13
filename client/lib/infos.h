#ifndef INFOS_H
#define INFOS_H

#include "sdl.h"

// INIT INFOS

/**
 * Initialise les ressources pour l'affichage des informations.
 * @param context Contexte SDL contenant le renderer.
 * @return Nombre d'erreurs de chargement (0 si succès).
 */
int init_infos(SDL_Context* context);


// OTHER INFOS

/** 
 * Calcule les FPS (Frames Per Second) et met à jour le champ `fps` du contexte SDL.
 * @param context Contexte SDL contenant le temps écoulé depuis la dernière mise à jour.
 * @param current_time Temps actuel en millisecondes (généralement obtenu via `SDL_GetTicks()`).
 * Note : Cette fonction doit être appelée à chaque frame pour mettre à jour les FPS.
 */
void calculate_fps(SDL_Context* context, Uint32 current_time);


// DISPLAY INFOS

/**
 * Affiche les informations à l'écran.
 * @param context Contexte SDL contenant le renderer et les données nécessaires pour l'affichage.
 */
void infos_display(SDL_Context* context);

/**
 * Affiche les FPS (Frames Per Second)
 * @param context Contexte SDL contenant le temps écoulé depuis la dernière mise à jour.
 * @param display_x La position x courante du placeholder pour que les FPS le suivent.
 */
void fps_display(SDL_Context* context, int display_x);

/**
 * Libère les ressources utilisées pour l'affichage des informations.
 */
void infos_free();



#endif /* INFOS_H */