# Codenames

Jeu codenames multijoueur en langage C

## Transmition client/serveur

Tunnel TCP initié par le client.

Format de donnée :
```
HEADER:TYPE:PAYLOAD
```

Headers disponibles : `JOIN, LEAVE, MESSAGE, REVEAL, HINT`
Types disponibles : `JSON, MSG, NULL`
Payload : `Données styucturé (json), un message textuel ou rien, en fonction du type`