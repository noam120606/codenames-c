#ifndef MESSAGE_H
#define MESSAGE_H

// Headers
#define MSG_CREATELOBBY 0
#define MSG_JOINLOBBY 1
#define MSG_STARTGAME 2

#include "codenames.h"
int on_message(Codenames* codenames, const char* message);

#endif