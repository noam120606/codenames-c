#ifndef UUID_H
#define UUID_H

#include "../lib/codenames.h"
#include "../lib/message.h"

/**
 * Handle a request for a UUID.
 * @param codenames The Codenames instance.
 * @param client The client requesting the UUID.
 * @param message The message containing the request.
 * @param args The arguments for the request.
 * @return EXIT_SUCCESS on success, or EXIT_FAILURE on error.
 */
int request_uuid(Codenames* codenames, TcpClient* client, char* message, Arguments args);

#endif // UUID_H