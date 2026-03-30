/**
 * @file word.h
 * @brief Définitions et structures pour la gestion des cartes dans Codenames côté client.
 */

#ifndef WORD_H
#define WORD_H

#include "../lib/game.h"
#include "../lib/utils.h"

typedef enum WordType {
    WT_MALE,
    WT_FEMALE,
    WT_CAT,
    WT_DOG,
} WordType;

/**
 * Représente un mot dans la grille de Codenames.
 *
 * @param word texte du mot (terminé par \0).
 * @param team équipe à laquelle le mot appartient (TEAM_*).
 * @param type type de mot (masculin, féminin, etc.) pour l'affichage de la carte.
 * @param revealed 0 si caché, 1 si révélé.
 * @param selected 0 si non sélectionné, 1 si sélectionné.
 */
typedef struct Word {
    char word[32];
    Team team;
    WordType type;
    Booleen revealed;
    Booleen selected;
} Word;

int init_words(AppContext * context);
void game_render_cards(AppContext * context);
int word_free();

#endif // WORD_H