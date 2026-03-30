/**
 * @file save.h
 * @brief Gestion de la sauvegarde et lecture des propriétés du joueur.
 */

#ifndef SAVE_H
#define SAVE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Ouvre le fichier de propriétés avec le mode spécifié.
 * @param mode Mode d'ouverture du fichier (ex: "r" pour lecture, "w" pour écriture).
 * @return Pointeur vers le fichier ouvert, ou NULL en cas d'échec.
 */
FILE* open_properties(const char* mode);

/**
 * Lit une propriété depuis le fichier de propriétés.
 * @param buf Buffer pour stocker la valeur de la propriété.
 * @param property Nom de la propriété à lire.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int read_property(char* buf, const char* property);

/**
 * Écrit une propriété dans le fichier de propriétés.
 * @param property Nom de la propriété à écrire.
 * @param value Valeur de la propriété à écrire.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int write_property(const char* property, const char* value);

#endif // SAVE_H