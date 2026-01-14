#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#define ROLE_ESPION 1
#define ROLE_AGENT 2

#define TEAM_NEUTRAL 0
#define TEAM_RED 1
#define TEAM_BLUE 2
#define TEAM_ASSASSIN 3 // mot noir

typedef struct {
    int socket_fd;
    char player_name[32];
    int role;
    int team;
} Player;

typedef struct {
    char word[32];
    int team;
    int revealed;
} Word;

#define GAME_STATUS_WAITING 0
#define GAME_STATUS_TURN_RED 1
#define GAME_STATUS_TURN_BLUE 2
#define GAME_STATUS_ENDED 3

typedef struct {
    int game_id;
    Word *words;
    int word_count;
    int status;
    int red_words_remaining;
    int blue_words_remaining;
} Game;

typedef struct {
    int lobby_id;
    Player *players;
    Game game;
    int current_players;
    int max_players;
} Lobby;

typedef struct {
    Lobby *lobbies;
    int lobby_count;
} GameManager;

Word* generateWords(int count);
void shuffleWords(Word* words, int count);
Game* create_game(int game_id, int word_count);
int destroy_game(Game* game);
int reveal_word(Game* game, int word_id);
int check_game_end(Game* game);
char* game_to_json(Game* game);
void print_game(Game* game);
GameManager* create_game_manager();
int destroy_game_manager(GameManager* manager);
int add_game_manager_lobby(GameManager* manager, Lobby new_lobby);

// fonctions publiques
Lobby* new_lobby(GameManager* manager, int max_players);
void new_game(GameManager* manager, int lobby_id);

#endif // GAME_MANAGER_H