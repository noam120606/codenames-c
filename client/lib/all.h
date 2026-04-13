/**
 * @file all.h
 * @brief En-tête principal regroupant tous les includes du client.
 */

#ifndef ALL_H
#define ALL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <direct.h>
#include <io.h>
#else
#include <getopt.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#endif

#include "../SDL2/include/SDL2/SDL.h"
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"
#include "../SDL2/include/SDL2/SDL_mixer.h"

#ifdef _WIN32
#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#define CLOSESOCKET(s) closesocket((SOCKET)(s))
#define MKDIR_DATA(path) _mkdir(path)
#else
#define CLOSESOCKET(s) close(s)
#define MKDIR_DATA(path) mkdir((path), 0755)
#endif

#include "../lib/tcp.h"
#include "../lib/sdl.h"
#include "../lib/menu.h"
#include "../lib/tuto.h"
#include "../lib/credits.h"
#include "../lib/button.h"
#include "../lib/background.h"
#include "../lib/infos.h"
#include "../lib/text.h"
#include "../lib/input.h"
#include "../lib/window.h"
#include "../lib/audio.h"
#include "../lib/crossfader.h"
#include "../lib/message.h"
#include "../lib/utils.h"
#include "../lib/list.h"
#include "../lib/game.h"
#include "../lib/user.h"
#include "../lib/lobby.h"
#include "../lib/chat.h"
#include "../lib/history.h"
#include "../lib/ressources.h"
#include "../lib/save.h"
#include "../lib/version.h"
#include "../lib/card.h"

#include "../lib/regex_compat.h"

#endif // ALL_H
