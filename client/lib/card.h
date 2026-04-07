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
 * @param guess_rect position et taille de l'icône de devinette à l'écran.
 * @param revealed 0 si caché, 1 si révélé.
 * @param selected 0 si non sélectionné, 1 si sélectionné.
 * @param is_pressed 0 si non pressé, 1 si pressé.
 * @param is_hovered 0 si non survolé, 1 si survolé.
 */
typedef struct Card {
    char word[32];
    Team team;
    SDL_Rect rect;
    SDL_Rect guess_rect;
    CardType type;
    Booleen revealed;
    Booleen selected;
    Booleen is_pressed;
    Booleen is_hovered;
} Card;

/**
 * Initialise les textures des cartes et les textes associés.
 * @param context contexte de l'application, utilisé pour le rendu et la création de textures.
 * @return le nombre de chargements ayant échoué (0 si tout a réussi).
 */
int init_cards(AppContext * context);

/**
 * Gère les événements liés aux cartes (clics, survols, etc.).
 * @param context contexte de l'application, utilisé pour accéder à l'état du jeu et des cartes.
 * @param event événement SDL à traiter.
 * @return EXIT_SUCCESS si l'événement a été traité avec succès, EXIT_FAILURE sinon.
 */
int cards_handle_event(AppContext* context, SDL_Event* event);

/**
 * Rend les cartes à l'écran en fonction de leur état (révélées, sélectionnées, etc.).
 * @param context contexte de l'application, utilisé pour le rendu et l'accès à l'état
 */
void game_render_cards(AppContext * context);

/**
 * Libère les ressources associées aux cartes (textures, textes, etc.).
 * @return EXIT_SUCCESS si les ressources ont été libérées avec succès, EXIT_FAILURE sinon.
 */
int card_free();

#endif // CARD_H