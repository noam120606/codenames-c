/**
 * @file uuid.h
 * @brief Gestion des identifiants uniques (UUID) des joueurs.
 */

#ifndef UUID_H
#define UUID_H

#include "../lib/codenames.h"
#include "../lib/message.h"

/**
 * Traite une requête d'UUID d'un client.
 * @param codenames Instance du serveur Codenames.
 * @param client Client demandant l'UUID.
 * @param message Message contenant la requête.
 * @param args Arguments de la requête.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int request_uuid(Codenames* codenames, TcpClient* client, char* message, Arguments args);

#endif // UUID_H