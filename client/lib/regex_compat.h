#ifndef REGEX_COMPAT_H
#define REGEX_COMPAT_H

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

typedef struct {
    char* pattern;
    int anchored_start;
    int anchored_end;
    void* tokens;
    int token_count;
} CnRegex;

#else

#include <regex.h>

typedef struct {
    regex_t impl;
} CnRegex;

#endif

/**
 * Compile une expression régulière simple. Supporte les métacaractères suivants :
 * - `^` : ancre de début de ligne
 * - `$` : ancre de fin de ligne
 * `.` : correspond à n'importe quel caractère
 * - `[...]` : classe de caractères, avec support des plages (ex: `[a-z]`) et de la négation (ex: `[^0-9]`)
 * - `\` : caractère d'échappement pour les métacaractères
 * - `*` : correspond à zéro ou plusieurs occurrences du token précédent
 * - `+` : correspond à une ou plusieurs occurrences du token précédent
 * - `?` : correspond à zéro ou une occurrence du token précédent
 * - `{m,n}` : correspond à au moins m et au plus n occurrences du token précédent
 * @param re Pointeur vers une structure CnRegex à initialiser
 * @param pattern Chaîne de caractères représentant l'expression régulière à compiler
 */
int cn_regex_compile(CnRegex* re, const char* pattern);

/**
 * Teste si une chaîne de caractères correspond à une expression régulière compilée. Retourne 0 si la chaîne correspond, 1 si elle ne correspond pas, et -1 en cas d'erreur (par exemple, si les arguments sont invalides).
 * @param re Pointeur vers une structure CnRegex compilée
 * @param text Chaîne de caractères à tester
 * @return 0 si la chaîne correspond à l'expression régulière, 1 si elle ne correspond pas, -1 en cas d'erreur
 */
int cn_regex_match(const CnRegex* re, const char* text);

/**
 * Libère les ressources associées à une expression régulière compilée. Après l'appel de cette fonction, la structure CnRegex doit être considérée comme invalide et ne doit plus être utilisée.
 * @param re Pointeur vers une structure CnRegex à libérer
 */
void cn_regex_free(CnRegex* re);

#endif
