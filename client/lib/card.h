/**
 * @file card.h
 * @brief Définitions et structures pour la gestion des cartes dans Codenames côté client.
 */

#ifndef CARD_H
#define CARD_H

#include "../lib/game.h"
#include "../lib/utils.h"

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
 * @param revealed 0 si caché, 1 si révélé.
 * @param selected 0 si non sélectionné, 1 si sélectionné.
 * @param is_pressed 0 si non pressé, 1 si pressé.
 * @param is_hovered 0 si non survolé, 1 si survolé.
 */
typedef struct Card {
    char word[32];
    Team team;
    CardType type;
    Booleen revealed;
    Booleen selected;
    Booleen is_pressed;
    Booleen is_hovered;
} Card;

int init_cards(AppContext * context);
void game_render_cards(AppContext * context);
int card_free();

#endif // CARD_H