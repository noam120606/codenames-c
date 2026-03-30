/**
 * @file infos.h
 * @brief Affichage des informations à l'écran (FPS, ping, volume).
 */

#ifndef INFOS_H
#define INFOS_H

#include "sdl.h"

/** Identifiants des crossfaders du bandeau infos. */
#define CROSSFADER_ID_MUSIC_VOLUME    1
#define CROSSFADER_ID_SFX_VOLUME      2

// INIT INFOS

/**
 * Initialise les ressources pour l'affichage des informations.
 * @param context Contexte SDL contenant le renderer.
 * @return Nombre d'erreurs de chargement (0 si succès).
 */
int init_infos(AppContext* context);


// OTHER INFOS

/** 
 * Calcule les FPS (Frames Per Second) et met à jour le champ `fps` du contexte SDL.
 * @param context Contexte SDL contenant le temps écoulé depuis la dernière mise à jour.
 * @param current_time Temps actuel en millisecondes (généralement obtenu via `SDL_GetTicks()`).
 * Note : Cette fonction doit être appelée à chaque frame pour mettre à jour les FPS.
 */
void calculate_fps(AppContext* context, Uint32 current_time);


// DISPLAY INFOS

/**
 * Affiche les informations à l'écran.
 * @param context Contexte SDL contenant le renderer et les données nécessaires pour l'affichage.
 */
void infos_display(AppContext* context);

/**
 * Gère les événements SDL pour le bandeau infos (crossfaders, etc.).
 * @param context Contexte SDL.
 * @param event Événement SDL à traiter.
 */
void infos_handle_event(AppContext* context, SDL_Event* event);

/**
 * Affiche les FPS (Frames Per Second)
 * @param context Contexte SDL contenant le temps écoulé depuis la dernière mise à jour.
 * @param display_x La position x courante du placeholder pour que les FPS le suivent.
 */
void fps_ping_display(AppContext* context, int display_x);

/**
 * Libère les ressources utilisées pour l'affichage des informations.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int infos_free();



#endif /* INFOS_H */