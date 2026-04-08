#include "../lib/all.h"

void load_version(AppContext* context) {

    char version[16];
    
    FILE* v = fopen("../client/src/VERSION", "r");
    if (v) {
        if (fgets(version, sizeof(version), v)) {
            /* Retirer le '\n' final */
            size_t len = strlen(version);
            if (len > 0 && version[len - 1] == '\n') version[len - 1] = '\0';
        } else {
            context->version[0] = '\0';
        }
        fclose(v);
    } else {
        /* Fallback : 0.0.0 si le fichier est absent */
        snprintf(version, sizeof(version), "%s", "0.0.0");
    }

    snprintf(context->version, sizeof(context->version), "V%s", version);

    // Demande de comparaison serveur
    char trame[64];
    format_to(trame, sizeof(trame), "%d %s", MSG_COMPAREVERSION, context->version);
    send_tcp(context->sock, trame);
}