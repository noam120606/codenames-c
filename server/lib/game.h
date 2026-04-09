/**
 * @file game.h
 * @brief Logique de la partie de Codenames côté serveur.
 */

#ifndef GAME_H
#define GAME_H

#include "../lib/message.h"

/**
 * Niveaux de difficulté du jeu.
 * @param WORDS_DIFFICULTY_NORMAL Difficulté normale (wordlist.txt).
 * @param WORDS_DIFFICULTY_HARD Difficulté difficile (wordlist_hard.txt).
 * @param WORDS_DIFFICULTY_INFO Difficulté informatique (wordlist_info.txt).
 * @param WORDS_DIFFICULTY_FREAKY Difficulté décalée (wordlist_freaky.txt).
 */
typedef enum WordsDifficulty {
    WORDS_DIFFICULTY_NORMAL,
    WORDS_DIFFICULTY_HARD,
    WORDS_DIFFICULTY_INFO,
    WORDS_DIFFICULTY_FREAKY
} WordsDifficulty;

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
 * @param can_guess Nombre de mots devinables par les agents dans le tour en cours (déterminé par l'indice donné par l'espion).
 * @param state état courant de la partie (GAMESTATE_*).
 */
typedef struct {
    Word* words;
    int nb_words;
    int can_guess;
    GameState state;
} Game;

/**
 * Initialise le gestionnaire de parties (structures internes, RNG, etc.).
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int init_game_manager();

/**
 * Récupère tous les mots selon la difficulté choisie.
 * @param difficulty Niveau de difficulté (DIFFICULTY_EASY ou DIFFICULTY_HARD).
 * @return Un tableau de chaînes (char*) contenant les mots,
 *         ou NULL en cas d'erreur. La gestion mémoire est à la
 *         charge de l'appelant.
 */
char** fetchWords(WordsDifficulty difficulty);

/**
 * Génère un tableau de mots pour une partie.
 * Les mots sont sélectionnés aléatoirement et associés à une équipe.
 * @param count Le nombre de mots à générer.
 * @param start_team L'équipe qui commence la partie.
 * @param difficulty Niveau de difficulté de la partie.
 * @return Un tableau de Word contenant les mots générés,
 *         ou NULL en cas d'erreur. La gestion mémoire est à la
 *         charge de l'appelant.
 */
Word* generateWords(int count, Team start_team, WordsDifficulty difficulty, int nb_assassins);

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

/**
 * Traite la pré-sélection d'une carte par un agent.
 * Le serveur vérifie que c'est bien le tour de l'agent et redistribue la sélection à tous les autres joueurs du lobby.
 * @param codenames Contexte principal du serveur.
 * @param client Client TCP ayant envoyé la demande (l'agent).
 * @param message Message brut reçu du client.
 * @param args Arguments extraits du message (card_index, selected).
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int request_preguess(Codenames* codenames, TcpClient* client, char* message, Arguments args);

/**
 * Traite la soumission d'une carte devinée par un agent.
 * Le serveur vérifie la validité de la carte, met à jour l'état du jeu et diffuse les changements à tous les joueurs du lobby.
 * @param codenames Contexte principal du serveur.
 * @param client Client TCP ayant envoyé la demande (l'agent).
 * @param message Message brut reçu du client.
 * @param args Arguments extraits du message (card_index).
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int request_guess_card(Codenames* codenames, TcpClient* client, char* message, Arguments args);

/**
 * Traite la demande de changement de difficulté d'un lobby.
 * Seul le propriétaire du lobby peut changer la difficulté.
 * @param codenames Contexte principal du serveur.
 * @param client Client TCP ayant envoyé la demande.
 * @param message Message brut reçu du client.
 * @param args Arguments extraits du message (difficulty: 0=facile, 1=difficile).
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int request_set_words_difficulty(Codenames* codenames, TcpClient* client, char* message, Arguments args);
int request_set_nb_assassins(Codenames* codenames, TcpClient* client, char* message, Arguments args);

#endif // GAME_H