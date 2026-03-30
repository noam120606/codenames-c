#ifndef GAME_H
#define GAME_H

#include "../lib/message.h"

/**
 * TEAM est utilisé à la fois pour catégoriser les mots dans la grille et pour assigner les joueurs à une équipe.
 * Catégories de mots dans la grille de Codenames.
 * Les mots sont classés en 4 catégories :
 * @param TEAM_NONE mot neutre (aucune équipe).
 * @param TEAM_RED mot appartenant à l'équipe rouge.
 * @param TEAM_BLUE mot appartenant à l'équipe bleue.
 * @param TEAM_BLACK mot assassin (met fin à la partie si révélé).
 */
typedef enum Team {
    TEAM_NONE,
    TEAM_RED,
    TEAM_BLUE,
    TEAM_BLACK
} Team;

/**
 * Représente un mot dans la grille de Codenames.
 *
 * @param word texte du mot (terminé par \0).
 * @param team équipe à laquelle le mot appartient (TEAM_*).
 * @param revealed 0 si caché, 1 si révélé.
 */
typedef struct {
    char word[32];
    Team team;
    int revealed;
} Word;

/**
 * États possibles d'une partie.
 * @param GAMESTATE_WAITING en attente de joueurs / démarrage.
 * @param GAMESTATE_TURN_RED_SPY tour de l'espion rouge.
 * @param GAMESTATE_TURN_RED_AGENT tour de l'agent rouge.
 * @param GAMESTATE_TURN_BLUE_SPY tour de l'espion bleu.
 * @param GAMESTATE_TURN_BLUE_AGENT tour de l'agent bleu.
 * @param GAMESTATE_ENDED partie terminée.
 */
typedef enum GameState {
    GAMESTATE_WAITING,
    GAMESTATE_TURN_RED_SPY,
    GAMESTATE_TURN_RED_AGENT,
    GAMESTATE_TURN_BLUE_SPY,
    GAMESTATE_TURN_BLUE_AGENT,
    GAMESTATE_ENDED
} GameState;

/**
 * Représente une partie de Codenames.
 * @param words tableau dynamique de Word (taille nb_words).
 * @param nb_words nombre de mots dans la grille.
 * @param state état courant de la partie (GAMESTATE_*).
 */
typedef struct {
    Word* words;
    int nb_words;
    GameState state;
} Game;

/**
 * Initialise le gestionnaire de parties (structures internes, RNG, etc.).
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int init_game_manager();

/**
 * Récupère tous les mots dans assets/wordlist.txt.
 * Chaque ligne du fichier est traitée comme un mot.
 * @return Un tableau de chaînes (char*) contenant les mots,
 *         ou NULL en cas d'erreur. La gestion mémoire est à la
 *         charge de l'appelant.
 */
char** fetchWords();

/**
 * Génère un tableau de mots pour une partie.
 * Les mots sont sélectionnés aléatoirement et associés à une équipe.
 * @param count Le nombre de mots à générer.
 * @return Un tableau de Word contenant les mots générés,
 *         ou NULL en cas d'erreur. La gestion mémoire est à la
 *         charge de l'appelant.
 */
Word* generateWords(int count, Team start_team);

/**
 * Mélange un tableau de mots in-place (Fisher-Yates).
 * @param words Tableau de Word à mélanger.
 * @param count Nombre d'éléments dans le tableau.
 */
void shuffleWords(Word* words, int count);

/**
 * Traite la demande de démarrage de partie.
 * @param codenames Contexte principal du serveur.
 * @param client Client TCP ayant envoyé la demande.
 * @param message Message brut reçu du client.
 * @param args Arguments extraits du message.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int request_start_game(Codenames* codenames, TcpClient* client, char* message, Arguments args);

/**
 * Traite la soumission d'un indice par un espion.
 * Le serveur redistribue l'indice à tous les autres joueurs du lobby.
 * @param codenames Contexte principal du serveur.
 * @param client Client TCP ayant envoyé la demande (l'espion).
 * @param message Message brut reçu du client.
 * @param args Arguments extraits du message (nb_hint, hint_word).
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int request_submit_hint(Codenames* codenames, TcpClient* client, char* message, Arguments args);

#endif // GAME_H