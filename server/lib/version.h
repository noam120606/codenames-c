/**
 * @file version.h
 * @brief Gestion de la version du serveur et comparaison avec les clients.
 */

#ifndef VERSION_H
#define VERSION_H

#include "../lib/codenames.h"

/**
 * Charge la version du serveur dans la structure Codenames.
 * @param codenames Pointeur vers la structure Codenames.
 */
void load_version(Codenames* codenames);

/**
 * Gère la comparaison de version entre le client et le serveur.
 * @param codenames Pointeur vers la structure Codenames.
 * @param client Pointeur vers le client TCP.
 * @param message Message reçu du client.
 * @param args Arguments extraits du message.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int on_version_compare(Codenames* codenames, TcpClient* client, char* message, Arguments args);

#endif // VERSION_H