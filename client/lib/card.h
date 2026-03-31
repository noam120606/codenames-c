/**
 * @file card.h
 * @brief Définitions et structures pour la gestion des cartes dans Codenames côté client.
 */

#ifndef CARD_H
#define CARD_H

#include "../lib/game.h"
#include "../lib/utils.h"

/**
 * Types de cartes pour l'affichage (masculin, féminin, etc.). Ne correspond pas à une logique de jeu, mais uniquement à l'affichage.
 * Permet d'avoir des cartes avec des formes différentes selon le type, pour une meilleure lisibilité.
 */
typedef enum CardType {
    CT_MALE,
    CT_FEMALE,
    CT_CAT,
    CT_DOG,
} CardType;

/**
 * Représente un mot dans la grille de Codenames.
 * @param word texte du mot (terminé par \0).
 * @param team équipe à laquelle le mot appartient (TEAM_*).
 * @param type type de mot (masculin, féminin, etc.) pour l'affichage de la carte.
 * @param rect position et taille de la carte à l'écran (calculé lors du rendu). 
 * @param revealed 0 si caché, 1 si révélé.
 * @param selected 0 si non sélectionné, 1 si sélectionné.
 * @param is_pressed 0 si non pressé, 1 si pressé.
 * @param is_hovered 0 si non survolé, 1 si survolé.
 */
typedef struct Card {
    char word[32];
    Team team;
    SDL_Rect rect;
    CardType type;
    Booleen revealed;
    Booleen selected;
    Booleen is_pressed;
    Booleen is_hovered;
} Card;

int init_cards(AppContext * context);
int cards_handle_event(AppContext* context, SDL_Event* event);
void game_render_cards(AppContext * context);
int card_free();

#endif // CARD_H