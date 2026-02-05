#ifndef GAME_H
#define GAME_H

#define TEAM_NEUTRAL 0
#define TEAM_RED 1
#define TEAM_BLUE 2
#define TEAM_BLACK 3

typedef struct {
    char word[32];
    int team;
    int revealed;
} Word;

#define GAMESTATE_WAITING 0
#define GAMESTATE_TURN_RED 1
#define GAMESTATE_TURN_BLUE 2
#define GAMESTATE_ENDED 3

typedef struct {
    int id;
    Word* words;
    int nb_words;
    int state;
} Game;

// Fonctions
int init_game_manager(void);
char** fetchWords();
Word* generateWords(int count);
void shuffleWords(Word* words, int count);

#endif // GAME_H