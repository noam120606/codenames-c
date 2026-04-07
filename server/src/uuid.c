#include "../lib/all.h"

int request_uuid(Codenames* codenames, TcpClient* client, char* message, Arguments args) {
    (void)message;
    (void)args;
    
    // Générer un UUID unique et l'envoyer au client
    char* uuid = generate_uuid("data/uuids");
    if (uuid == NULL) {
        printf("Failed to generate UUID for client %d\n", client->id);
        return EXIT_FAILURE;
    }

    char reponse[128];
    format_to(reponse, sizeof(reponse), "%d %s", MSG_REQUESTUUID, uuid);
    tcp_send_to_client(codenames, client->id, reponse);
    printf("Generated UUID %s for client %d\n", uuid, client->id);
    free(uuid);

    return EXIT_SUCCESS;
}
